#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "spi_mcp3008.hpp"
#include "spi_74hc595.hpp"

// SPI Defines
#define SPI_PORT spi0
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS_ADC1 7
#define PIN_CS_ADC2 8
#define PIN_CS_SR_UNIT_LOW 2
#define PIN_CS_SR_LFO_HIGH_VCF 3
#define PIN_CS_SR_LFO_LOW 4
#define PIN_CS_SR_UNIT_HIGH 5
#define PIN_CS_SR_VCO 6
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

void print_adc(SPIMCP3008 adc1, SPIMCP3008 adc2)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        printf(">CHa%u: %4u\n", i, adc1.read(i));
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        printf(">CHb%u: %4u\n", i, adc2.read(i));
    }
}

void print_sw()
{
    printf(">PIN_SW_VCO1_L: %d\n", !gpio_get(PIN_SW_VCO1_L));
    printf(">PIN_SW_VCO1_R: %d\n", !gpio_get(PIN_SW_VCO1_R));
    printf(">PIN_SW_VCO2_L: %d\n", !gpio_get(PIN_SW_VCO2_L));
    printf(">PIN_SW_VCO2_R: %d\n", !gpio_get(PIN_SW_VCO2_R));
    printf(">PIN_SW_VCF_L: %d\n", !gpio_get(PIN_SW_VCF_L));
    printf(">PIN_SW_VCF_R: %d\n", !gpio_get(PIN_SW_VCF_R));
    printf(">PIN_SW_LFO_WAVE_L: %d\n", !gpio_get(PIN_SW_LFO_WAVE_L));
    printf(">PIN_SW_LFO_WAVE_R: %d\n", !gpio_get(PIN_SW_LFO_WAVE_R));
    printf(">PIN_SW_LFO_TARGET_L: %d\n", !gpio_get(PIN_SW_LFO_TARGET_L));
    printf(">PIN_SW_LFO_TARGET_R: %d\n", !gpio_get(PIN_SW_LFO_TARGET_R));
}

class Selector
{
public:
    Selector(uint pin_sw_r, uint pin_sw_l, uint8_t n_state, SPI74HC595 **led_sr, uint8_t *led_pin)
        : pin_sw_r(pin_sw_r),
          pin_sw_l(pin_sw_l),
          sw_r_old(false),
          sw_l_old(false),
          n_state(n_state),
          state(0),
          led_sr(led_sr),
          led_pin(led_pin)
    {
        gpio_init(pin_sw_r);
        gpio_init(pin_sw_l);
        gpio_set_dir(pin_sw_r, GPIO_IN);
        gpio_set_dir(pin_sw_l, GPIO_IN);
        gpio_set_pulls(pin_sw_r, true, false);
        gpio_set_pulls(pin_sw_l, true, false);
        gpio_set_input_hysteresis_enabled(pin_sw_r, true);
        gpio_set_input_hysteresis_enabled(pin_sw_l, true);
        show_leds();
    };

    void update()
    {
        bool sw_r = !gpio_get(pin_sw_r);
        bool sw_l = !gpio_get(pin_sw_l);
        if ((sw_r && !sw_r_old) || (sw_l && !sw_l_old))
        {
            if (sw_r && !sw_r_old)
                state = (state + 1) % n_state;
            else
                state = (state + n_state - 1) % n_state;
            show_leds();
        }
        sw_r_old = sw_r;
        sw_l_old = sw_l;
    }

    uint8_t get_state()
    {
        return state;
    }

private:
    uint pin_sw_r;
    uint pin_sw_l;
    bool sw_r_old;
    bool sw_l_old;
    uint8_t n_state;
    uint8_t state;
    SPI74HC595 **led_sr;
    uint8_t *led_pin;
    void show_leds()
    {
        for (uint8_t i = 0; i < n_state; i++)
        {
            led_sr[i]->put(led_pin[i], state == i);
        }
    }
};

int main()
{
    stdio_init_all();

    // UART
    uart_init(UART_PORT, 115200);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // VCF:Cut-Off, VCO:mix, VCO:Duty, VCO2:Tune, VCF:Release, VCF:Decay, VCF:Attack, VCF:Resonance
    SPIMCP3008 adc1(SPI_PORT, PIN_CS_ADC1);
    // LFO:Depth, LFO:Speed, VCA:Gain, VCA:Release, VCA:Sustain, VCA:Decay, VCA:Attack, VCF:Sustain
    SPIMCP3008 adc2(SPI_PORT, PIN_CS_ADC2);

    // Unit: [6, 5, 4, 3, 2, 1, 0, 7]
    SPI74HC595 led_unit_low(SPI_PORT, PIN_CS_SR_UNIT_LOW);
    // Unit: [6, 5, 4, 3, 2, 1, 0, 7]
    SPI74HC595 led_unit_high(SPI_PORT, PIN_CS_SR_UNIT_HIGH);
    // [VCF:LPF, LFO:wave5, LFO:wave4, LFO:wave3, LFO:wave2, LFO:wave1, LFO:wave0, VCF:HPF]
    SPI74HC595 led_lfo_high_vcf(SPI_PORT, PIN_CS_SR_LFO_HIGH_VCF);
    // LFO:target [None, 5, 4, 3, 2, 1, 0, None]
    SPI74HC595 led_lfo_low(SPI_PORT, PIN_CS_SR_LFO_LOW);
    // VCO:wave: [2:2, 2:1, 2:0, 1:3, 1:2, 1:1, 1:0, 2:3]
    SPI74HC595 led_vco(SPI_PORT, PIN_CS_SR_VCO);

    SPI74HC595 *vco1_sr[4] = {&led_vco, &led_vco, &led_vco, &led_vco};
    uint8_t vco1_pin[4] = {1, 2, 3, 4};
    Selector vco1(PIN_SW_VCO1_R, PIN_SW_VCO1_L, 4, vco1_sr, vco1_pin);

    SPI74HC595 *vco2_sr[4] = {&led_vco, &led_vco, &led_vco, &led_vco};
    uint8_t vco2_pin[4] = {5, 6, 7, 0};
    Selector vco2(PIN_SW_VCO2_R, PIN_SW_VCO2_L, 4, vco2_sr, vco2_pin);

    SPI74HC595 *vcf_sr[2] = {&led_lfo_high_vcf, &led_lfo_high_vcf};
    uint8_t vcf_pin[2] = {7, 0};
    Selector vcf(PIN_SW_VCF_R, PIN_SW_VCF_L, 2, vcf_sr, vcf_pin);

    SPI74HC595 *lfo_wave_sr[6] = {&led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf};
    uint8_t lfo_wave_pin[6] = {1, 2, 3, 4, 5, 6};
    Selector lfo_wave(PIN_SW_LFO_WAVE_R, PIN_SW_LFO_WAVE_L, 6, lfo_wave_sr, lfo_wave_pin);

    SPI74HC595 *lfo_target_sr[6] = {&led_lfo_low, &led_lfo_low, &led_lfo_low, &led_lfo_low, &led_lfo_low, &led_lfo_low};
    uint8_t lfo_target_pin[6] = {1, 2, 3, 4, 5, 6};
    Selector lfo_target(PIN_SW_LFO_TARGET_R, PIN_SW_LFO_TARGET_L, 6, lfo_target_sr, lfo_target_pin);

    uint8_t unit_now = 0;
    uint16_t unit_state = 0;
    uint8_t unit_note[16] = {0};

    char buf[256] = {0};
    char buf_msg[12] = {0};
    uint8_t buf_index = 0;

    while (true)
    {
        vco1.update();
        vco2.update();
        vcf.update();
        lfo_wave.update();
        lfo_target.update();

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
                printf("%02x %02x %02x\n", msg[0], msg[1], msg[2]);
                // assign unit
                if (msg[0] == 0x90)
                {
                    for (uint8_t i = 0; i < 16; i++)
                    {
                        uint8_t i_offset = (i + unit_now) & 0x0F;
                        if (msg[2] == 0) // note off
                        {
                            if ((unit_state & 1 << i) && unit_note[i] == msg[1])
                            {
                                unit_note[i] = 0;
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
                                unit_now = (i_offset + 1) & 0x0F;
                                break;
                            }
                        }
                    }
                    led_unit_high.put_8bit((unit_state & 0xFF) << 1 | (unit_state & 0xFF) >> 7);
                    led_unit_low.put_8bit((unit_state >> 8 & 0xFF) << 1 | (unit_state >> 8 & 0xFF) >> 7);
                }
            }
            buf_index++;
        }
    }
}
