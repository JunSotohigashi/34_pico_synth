#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <hardware/sync.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>
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

enum class MidiEventType : uint8_t
{
    NOTE_ON = 0,
    NOTE_OFF = 1,
    SUSTAIN_ON = 2,
    SUSTAIN_OFF = 3
};

struct MidiEvent
{
    MidiEventType type;
    uint8_t data1;  // note number for NOTE_ON/OFF
    uint8_t data2;  // velocity for NOTE_ON
};

static SoundModuleManager *g_sound_mgr = nullptr;
constexpr uint64_t TIMER_PERIOD_US = 20000; // 20ms timer interval

// MIDI event queue (Core 1 -> Core 0)
static queue_t midi_event_queue;

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

void core1_entry()
{
    while (g_sound_mgr == nullptr)
    {
        tight_loop_contents();
    }
    
    static char uart_line[16] = {0};
    static uint8_t uart_line_index = 0;
    
    while (true)
    {
        // Process UART MIDI on Core1 and forward events via queue
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

            MidiEvent event;
            if ((status == 0x80) || (status == 0x90 && data2 == 0))
            {
                event.type = MidiEventType::NOTE_OFF;
                event.data1 = data1;
                event.data2 = 0;
                queue_try_add(&midi_event_queue, &event);
            }
            else if (status == 0x90 && data2 != 0)
            {
                event.type = MidiEventType::NOTE_ON;
                event.data1 = data1;
                event.data2 = data2;
                queue_try_add(&midi_event_queue, &event);
            }
            else if (status == 0xB0 && data1 == 0x40)
            {
                if (data2 >= 64)
                {
                    event.type = MidiEventType::SUSTAIN_ON;
                }
                else
                {
                    event.type = MidiEventType::SUSTAIN_OFF;
                }
                event.data1 = 0;
                event.data2 = 0;
                queue_try_add(&midi_event_queue, &event);
            }
        }
        tight_loop_contents();
    }
}

void process_midi_events(SoundModuleManager *sound_mgr)
{
    MidiEvent event;
    VoiceAllocator *va = sound_mgr->get_voice_allocator();
    
    // Process all pending MIDI events from queue
    while (queue_try_remove(&midi_event_queue, &event))
    {
        switch (event.type)
        {
        case MidiEventType::NOTE_ON:
            va->handle_note_on(event.data1, event.data2);
            break;
        case MidiEventType::NOTE_OFF:
            va->handle_note_off(event.data1);
            break;
        case MidiEventType::SUSTAIN_ON:
            va->handle_sustain_on();
            break;
        case MidiEventType::SUSTAIN_OFF:
            va->handle_sustain_off();
            break;
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

    // Initialize MIDI event queue
    queue_init(&midi_event_queue, sizeof(MidiEvent), 128);

    g_sound_mgr = &sound_mgr;
    multicore_launch_core1(core1_entry);

    uint16_t stream[STREAM_LENGTH];
    absolute_time_t next_time = get_absolute_time();
    
    while (true)
    {
        // Process MIDI events from Core1 queue
        process_midi_events(&sound_mgr);

        // Update and transmit at fixed 20ms intervals
        next_time = delayed_by_us(next_time, TIMER_PERIOD_US);
        sleep_until(next_time);
        
        sound_mgr.update();
        
        // Send parameters for each unit sequentially with MIDI processing between units
        for (uint8_t unit = 0; unit < 16; unit++)
        {
            sound_mgr.serialize(unit, stream);
            transmit_stream(stream);
            process_midi_events(&sound_mgr);  // Process MIDI events between transmissions
        }
    }

    return 0;
}
