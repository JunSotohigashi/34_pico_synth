#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <hardware/sync.h>
#include <pico/multicore.h>
#include <cstdio>
#include <cstring>

#include "config.hpp"
#include "spi_mcp3008.hpp"
#include "spi_74hc595.hpp"
#include "selector.hpp"
#include "types.hpp"
#include "sound_unit.hpp"
#include "voice_allocator.hpp"
#include "panel_manager.hpp"
#include "sound_module_manager.hpp"

volatile bool g_update_flag = false; // Core 1 -> Core 0: panel update request
volatile bool g_send_flag = false;   // Core 1 -> Core 0: stream send request
static SoundModuleManager *g_sound_mgr = nullptr;
constexpr uint64_t TIMER_PERIOD_US = 20000; // 20ms timer interval

// UART RX ring buffer (256 bytes)
static char uart_rx_buffer[256];
static volatile uint16_t uart_rx_write_idx = 0;
static uint16_t uart_rx_read_idx = 0;

// UART RX interrupt handler
void uart0_irq_handler()
{
    while (uart_is_readable(uart0))
    {
        const char ch = static_cast<char>(uart_getc(uart0));
        const uint16_t next_write = (uart_rx_write_idx + 1) % sizeof(uart_rx_buffer);
        
        // Only store if not overflowing
        if (next_write != uart_rx_read_idx)
        {
            uart_rx_buffer[uart_rx_write_idx] = ch;
            uart_rx_write_idx = next_write;
        }
    }
}

int64_t timer_callback(alarm_id_t id, void *user_data)
{
    (void)id;
    (void)user_data;
    g_update_flag = true;
    return TIMER_PERIOD_US;
}

void core1_entry()
{
    while (g_sound_mgr == nullptr)
    {
        tight_loop_contents();
    }
    add_alarm_in_us(TIMER_PERIOD_US, timer_callback, NULL, false);
    while (true)
    {
        tight_loop_contents();
    }
}

void handle_uart_midi(SoundModuleManager *sound_mgr)
{
    static char uart_line[16] = {0};
    static uint8_t uart_line_index = 0;
    
    // Process all available characters from ring buffer
    while (uart_rx_read_idx != uart_rx_write_idx)
    {
        const char ch = uart_rx_buffer[uart_rx_read_idx];
        uart_rx_read_idx = (uart_rx_read_idx + 1) % sizeof(uart_rx_buffer);

        if (ch == '\r')
        {
            continue;
        }

        if (ch != '\n')
        {
            if (uart_line_index < sizeof(uart_line) - 1)
            {
                uart_line[uart_line_index++] = ch;
            }
            else
            {
                // Drop malformed long lines and resync at next newline.
                uart_line_index = 0;
            }
            continue;
        }

        uart_line[uart_line_index] = '\0';
        uart_line_index = 0;

        uint8_t status = 0;
        uint8_t data1 = 0;
        uint8_t data2 = 0;
        if (sscanf(uart_line, "%hhx %hhx %hhx", &status, &data1, &data2) != 3)
        {
            continue;
        }

        VoiceAllocator *va = sound_mgr->get_voice_allocator();

        if ((status == 0x80) || (status == 0x90 && data2 == 0))
        {
            va->handle_note_off(data1);
        }
        else if (status == 0x90 && data2 != 0)
        {
            va->handle_note_on(data1, data2);
        }
        else if (status == 0xB0 && data1 == 0x40)
        {
            if (data2 >= 64)
            {
                va->handle_sustain_on();
            }
            else
            {
                va->handle_sustain_off();
            }
        }
    }
}

void transmit_stream(uint16_t stream[STREAM_LENGTH])
{
    for (uint8_t i = 0; i < STREAM_LENGTH; i++)
    {
        gpio_put(PIN_CS_SOUND, false);
        sleep_us(10);
        spi_write16_blocking(spi1, &stream[i], 1);
        sleep_us(10);
        gpio_put(PIN_CS_SOUND, true);
    }
}

int main()
{
    stdio_init_all();

    uart_init(uart0, UART_BAUDRATE);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);
    
    // Enable UART RX interrupt
    irq_set_exclusive_handler(UART0_IRQ, uart0_irq_handler);
    uart_set_irq_enables(uart0, true, false);  // RX interrupt only
    irq_set_enabled(UART0_IRQ, true);

    spi_init(spi0, SPI_BAUDRATE_PANEL);
    gpio_set_function(PIN_MISO_PANEL, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_PANEL, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_PANEL, GPIO_FUNC_SPI);

    spi_init(spi1, SPI_BAUDRATE_SOUND);
    spi_set_format(spi1, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_SOUND, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS_SOUND, GPIO_OUT);
    gpio_put(PIN_CS_SOUND, true);

    SPIMCP3008 adc1(spi0, PIN_CS_ADC1);
    SPIMCP3008 adc2(spi0, PIN_CS_ADC2);

    SPI74HC595 led_unit_low(spi0, PIN_CS_SR_UNIT_LOW);
    SPI74HC595 led_unit_high(spi0, PIN_CS_SR_UNIT_HIGH);
    SPI74HC595 led_lfo_high_vcf(spi0, PIN_CS_SR_LFO_HIGH_VCF);
    SPI74HC595 led_lfo_low(spi0, PIN_CS_SR_LFO_LOW);
    SPI74HC595 led_vco(spi0, PIN_CS_SR_VCO);

    SPI74HC595 *vco1_sr[4] = {&led_vco, &led_vco, &led_vco, &led_vco};
    uint8_t vco1_pin[4] = {1, 2, 3, 4};
    Selector sel_vco1(PIN_SW_VCO1_R, PIN_SW_VCO1_L, 4, vco1_sr, vco1_pin);

    SPI74HC595 *vco2_sr[4] = {&led_vco, &led_vco, &led_vco, &led_vco};
    uint8_t vco2_pin[4] = {5, 6, 7, 0};
    Selector sel_vco2(PIN_SW_VCO2_R, PIN_SW_VCO2_L, 4, vco2_sr, vco2_pin);

    SPI74HC595 *vcf_sr[2] = {&led_lfo_high_vcf, &led_lfo_high_vcf};
    uint8_t vcf_pin[2] = {7, 0};
    Selector sel_vcf(PIN_SW_VCF_R, PIN_SW_VCF_L, 2, vcf_sr, vcf_pin);

    SPI74HC595 *lfo_wave_sr[6] = {&led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf,
                                  &led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf};
    uint8_t lfo_wave_pin[6] = {1, 2, 3, 4, 5, 6};
    Selector sel_lfo_wave(PIN_SW_LFO_WAVE_R, PIN_SW_LFO_WAVE_L, 6, lfo_wave_sr, lfo_wave_pin);

    SPI74HC595 *lfo_target_sr[6] = {&led_lfo_low, &led_lfo_low, &led_lfo_low,
                                    &led_lfo_low, &led_lfo_low, &led_lfo_low};
    uint8_t lfo_target_pin[6] = {1, 2, 3, 4, 5, 6};
    Selector sel_lfo_target(PIN_SW_LFO_TARGET_R, PIN_SW_LFO_TARGET_L, 6, lfo_target_sr, lfo_target_pin);

    VoiceAllocator voice_allocator;
    PanelManager panel_manager(&sel_vco1, &sel_vco2, &sel_vcf, &sel_lfo_wave, &sel_lfo_target,
                               &adc1, &adc2,
                               &led_unit_low, &led_unit_high,
                               &led_lfo_high_vcf, &led_lfo_low, &led_vco);
    SoundModuleManager sound_mgr(&voice_allocator, &panel_manager);

    g_sound_mgr = &sound_mgr;
    multicore_launch_core1(core1_entry);

    uint16_t stream[STREAM_LENGTH];
    while (true)
    {
        handle_uart_midi(&sound_mgr);

        if (g_update_flag)
        {
            // Keep heavy SPI/ADC work out of alarm IRQ context.
            g_update_flag = false;
            sound_mgr.update();
            g_send_flag = true;
        }

        if (g_send_flag)
        {
            // Send parameters for each unit sequentially
            for (uint8_t unit = 0; unit < 16; unit++)
            {
                sound_mgr.serialize(unit, stream);
                transmit_stream(stream);
            }
            g_send_flag = false;
        }
    }

    return 0;
}
