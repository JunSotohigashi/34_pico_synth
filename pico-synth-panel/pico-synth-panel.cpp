#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "selector.hpp"
#include "spi_mcp3008.hpp"

// SPI to ADC and LED
#define SPI_PORT_PANEL spi0
#define PIN_SCK_PANEL 18
#define PIN_MOSI_PANEL 19
#define PIN_MISO_PANEL 16
#define PIN_CS_ADC1 7
#define PIN_CS_ADC2 8
#define PIN_CS_SR_UNIT_LOW 2
#define PIN_CS_SR_LFO_HIGH_VCF 3
#define PIN_CS_SR_LFO_LOW 4
#define PIN_CS_SR_UNIT_HIGH 5
#define PIN_CS_SR_VCO 6
// SPI to sound unit
#define SPI_PORT_SOUND spi1
#define PIN_SCK_SOUND 14
#define PIN_MOSI_SOUND 15
#define PIN_MISO_SOUND 12
#define PIN_CS_SOUND 13
#define STREAM_LENGTH 34
// UART
#define UART_PORT uart0
#define PIN_UART_TX 0
#define PIN_UART_RX 1
// switches
#define PIN_SW_VCO1_R 29
#define PIN_SW_VCO1_L 28
#define PIN_SW_VCO2_R 27
#define PIN_SW_VCO2_L 26
#define PIN_SW_VCF_R 25
#define PIN_SW_VCF_L 24
#define PIN_SW_LFO_WAVE_R 23
#define PIN_SW_LFO_WAVE_L 22
#define PIN_SW_LFO_TARGET_R 21
#define PIN_SW_LFO_TARGET_L 20

semaphore_t sem;
uint16_t unit_state = 0;
uint8_t unit_note[16] = {0};
uint8_t unit_velocity[16] = {0};

void main_core1()
{
    char buf[256] = {0};
    char buf_msg[12] = {0};
    uint8_t buf_index = 0;
    uint8_t unit_now = 0;
    while (true)
    {
        if (uart_is_readable(UART_PORT))
        {
            buf[buf_index] = uart_getc(UART_PORT);
            // message end
            if (buf[buf_index] == '\n')
            {
                for (uint16_t i = 0; i <= 8; i++)
                {
                    buf_msg[i] = buf[(buf_index + i - 8) & 0xFF];
                }
                buf_msg[10] = '\0';
                uint8_t msg[3];
                sscanf(buf_msg, "%hhx %hhx %hhx", &msg[0], &msg[1], &msg[2]);
                // assign unit
                if (msg[0] == 0x90)
                {
                    sem_acquire_blocking(&sem);
                    for (uint8_t i = 0; i < 16; i++)
                    {
                        uint8_t i_offset = (i + unit_now) & 0x0F;
                        if (msg[2] == 0) // note off
                        {
                            if ((unit_state & 1 << i) && unit_note[i] == msg[1])
                            {
                                unit_note[i] = 0;
                                unit_velocity[i] = 0;
                                unit_state &= ~(1 << i);
                                break;
                            }
                        }
                        else // note on
                        {
                            if (!(unit_state & 1 << i_offset))
                            {
                                unit_state |= 1 << i_offset;
                                unit_note[i_offset] = msg[1];
                                unit_velocity[i_offset] = msg[2];
                                unit_now = (i_offset + 1) & 0x0F;
                                break;
                            }
                        }
                    }
                    sem_release(&sem);
                }
            }
            buf_index++;
        }
    }
}

int main()
{
    stdio_init_all();
    sem_init(&sem, 1, 1);

    // UART
    uart_init(UART_PORT, 115200);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT_PANEL, 1000 * 1000);
    gpio_set_function(PIN_MISO_PANEL, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_PANEL, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_PANEL, GPIO_FUNC_SPI);

    spi_init(SPI_PORT_SOUND, 1000 * 1000);
    spi_set_format(SPI_PORT_SOUND, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_SOUND, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS_SOUND, GPIO_OUT);
    gpio_put(PIN_CS_SOUND, true);

    // VCF:Cut-Off, VCO:mix, VCO:Duty, VCO2:Tune, VCF:Release, VCF:Decay, VCF:Attack, VCF:Resonance
    SPIMCP3008 adc1(SPI_PORT_PANEL, PIN_CS_ADC1);
    // LFO:Depth, LFO:Speed, VCA:Gain, VCA:Release, VCA:Sustain, VCA:Decay, VCA:Attack, VCF:Sustain
    SPIMCP3008 adc2(SPI_PORT_PANEL, PIN_CS_ADC2);

    // Unit: [6, 5, 4, 3, 2, 1, 0, 7]
    SPI74HC595 led_unit_low(SPI_PORT_PANEL, PIN_CS_SR_UNIT_LOW);
    // Unit: [6, 5, 4, 3, 2, 1, 0, 7]
    SPI74HC595 led_unit_high(SPI_PORT_PANEL, PIN_CS_SR_UNIT_HIGH);
    // [VCF:LPF, LFO:wave5, LFO:wave4, LFO:wave3, LFO:wave2, LFO:wave1, LFO:wave0, VCF:HPF]
    SPI74HC595 led_lfo_high_vcf(SPI_PORT_PANEL, PIN_CS_SR_LFO_HIGH_VCF);
    // LFO:target [None, 5, 4, 3, 2, 1, 0, None]
    SPI74HC595 led_lfo_low(SPI_PORT_PANEL, PIN_CS_SR_LFO_LOW);
    // VCO:wave: [2:2, 2:1, 2:0, 1:3, 1:2, 1:1, 1:0, 2:3]
    SPI74HC595 led_vco(SPI_PORT_PANEL, PIN_CS_SR_VCO);

    SPI74HC595 *vco1_sr[4] = {&led_vco, &led_vco, &led_vco, &led_vco};
    uint8_t vco1_pin[4] = {1, 2, 3, 4};
    Selector sel_vco1(PIN_SW_VCO1_R, PIN_SW_VCO1_L, 4, vco1_sr, vco1_pin);

    SPI74HC595 *vco2_sr[4] = {&led_vco, &led_vco, &led_vco, &led_vco};
    uint8_t vco2_pin[4] = {5, 6, 7, 0};
    Selector sel_vco2(PIN_SW_VCO2_R, PIN_SW_VCO2_L, 4, vco2_sr, vco2_pin);

    SPI74HC595 *vcf_sr[2] = {&led_lfo_high_vcf, &led_lfo_high_vcf};
    uint8_t vcf_pin[2] = {7, 0};
    Selector sel_vcf(PIN_SW_VCF_R, PIN_SW_VCF_L, 2, vcf_sr, vcf_pin);

    SPI74HC595 *lfo_wave_sr[6] = {&led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf};
    uint8_t lfo_wave_pin[6] = {1, 2, 3, 4, 5, 6};
    Selector sel_lfo_wave(PIN_SW_LFO_WAVE_R, PIN_SW_LFO_WAVE_L, 6, lfo_wave_sr, lfo_wave_pin);

    SPI74HC595 *lfo_target_sr[6] = {&led_lfo_low, &led_lfo_low, &led_lfo_low, &led_lfo_low, &led_lfo_low, &led_lfo_low};
    uint8_t lfo_target_pin[6] = {1, 2, 3, 4, 5, 6};
    Selector sel_lfo_target(PIN_SW_LFO_TARGET_R, PIN_SW_LFO_TARGET_L, 6, lfo_target_sr, lfo_target_pin);

    multicore_launch_core1(main_core1);

    while (true)
    {
        // read switches and switch each mode
        sel_vco1.update();
        sel_vco2.update();
        sel_vcf.update();
        sel_lfo_wave.update();
        sel_lfo_target.update();

        // apply unit led
        led_unit_high.put_8bit((unit_state & 0xFF) << 1 | (unit_state & 0xFF) >> 7);
        led_unit_low.put_8bit((unit_state >> 8 & 0xFF) << 1 | (unit_state >> 8 & 0xFF) >> 7);

        // create message to sound unit
        uint16_t stream[STREAM_LENGTH];
        stream[0] = 0xFFFF; // header
        sem_acquire_blocking(&sem);
        for (uint8_t ch = 0; ch < 16; ch++)
        {
            stream[ch + 1] = ((unit_state >> ch) & 1) << 15 | unit_note[ch] << 8 | unit_velocity[ch];
        }
        sem_release(&sem);
        stream[17] = sel_vco1.get_state() << 10 | sel_vco2.get_state() << 8 | sel_vcf.get_state() << 7 | sel_lfo_wave.get_state() << 3 | sel_lfo_target.get_state();
        stream[18] = adc1.read(2); // vco duty
        stream[19] = adc1.read(3); // vco2 tune
        stream[20] = adc1.read(1); // vco mix
        stream[21] = adc1.read(0); // vcf cutoff
        stream[22] = adc1.read(7); // vcf resonanse
        stream[23] = adc1.read(6); // vcf attack
        stream[24] = adc1.read(5); // vcf decay
        stream[25] = adc2.read(7); // vcf sustain
        stream[26] = adc1.read(4); // vcf release
        stream[27] = adc2.read(6); // vca attack
        stream[28] = adc2.read(5); // vca decay
        stream[29] = adc2.read(4); // vca sustain
        stream[30] = adc2.read(3); // vca release
        stream[31] = adc2.read(2); // vca gain
        stream[32] = adc2.read(1); // lfo speed
        stream[33] = adc2.read(0); // lfo depth
        
        for (uint8_t i = 0; i < STREAM_LENGTH; i++)
        {
            gpio_put(PIN_CS_SOUND, false);
            sleep_us(10);
            spi_write16_blocking(SPI_PORT_SOUND, &stream[i], 1);
            sleep_us(10);
            gpio_put(PIN_CS_SOUND, true);
        }
        for (uint8_t i = 0; i < STREAM_LENGTH; i++)
        {
            printf("%04x", stream[i]);
        }
        printf("\n");
    }
}
