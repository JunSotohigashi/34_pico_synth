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
#define PIN_SW_VCO1_R 29
#define PIN_SW_VCO1_L 28
#define PIN_SW_VCO2_R 27
#define PIN_SW_VCO2_L 26
#define PIN_SW_VCF_R 25
#define PIN_SW_VCF_L 24
#define PIN_SW_VCA_WAVE_R 23
#define PIN_SW_VCA_WAVE_L 22
#define PIN_SW_VCA_TARGET_R 21
#define PIN_SW_VCA_TARGET_L 20

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
    printf(">PIN_SW_VCA_WAVE_L: %d\n", !gpio_get(PIN_SW_VCA_WAVE_L));
    printf(">PIN_SW_VCA_WAVE_R: %d\n", !gpio_get(PIN_SW_VCA_WAVE_R));
    printf(">PIN_SW_VCA_TARGET_L: %d\n", !gpio_get(PIN_SW_VCA_TARGET_L));
    printf(">PIN_SW_VCA_TARGET_R: %d\n", !gpio_get(PIN_SW_VCA_TARGET_R));
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_SW_VCO1_L);
    gpio_init(PIN_SW_VCO1_R);
    gpio_init(PIN_SW_VCO2_L);
    gpio_init(PIN_SW_VCO2_R);
    gpio_init(PIN_SW_VCF_L);
    gpio_init(PIN_SW_VCF_R);
    gpio_init(PIN_SW_VCA_WAVE_L);
    gpio_init(PIN_SW_VCA_WAVE_R);
    gpio_init(PIN_SW_VCA_TARGET_L);
    gpio_init(PIN_SW_VCA_TARGET_R);
    gpio_set_dir(PIN_SW_VCO1_L, GPIO_IN);
    gpio_set_dir(PIN_SW_VCO1_R, GPIO_IN);
    gpio_set_dir(PIN_SW_VCO2_L, GPIO_IN);
    gpio_set_dir(PIN_SW_VCO2_R, GPIO_IN);
    gpio_set_dir(PIN_SW_VCF_L, GPIO_IN);
    gpio_set_dir(PIN_SW_VCF_R, GPIO_IN);
    gpio_set_dir(PIN_SW_VCA_WAVE_L, GPIO_IN);
    gpio_set_dir(PIN_SW_VCA_WAVE_R, GPIO_IN);
    gpio_set_dir(PIN_SW_VCA_TARGET_L, GPIO_IN);
    gpio_set_dir(PIN_SW_VCA_TARGET_R, GPIO_IN);
    gpio_set_pulls(PIN_SW_VCO1_L, true, false);
    gpio_set_pulls(PIN_SW_VCO1_R, true, false);
    gpio_set_pulls(PIN_SW_VCO2_L, true, false);
    gpio_set_pulls(PIN_SW_VCO2_R, true, false);
    gpio_set_pulls(PIN_SW_VCF_L, true, false);
    gpio_set_pulls(PIN_SW_VCF_R, true, false);
    gpio_set_pulls(PIN_SW_VCA_WAVE_L, true, false);
    gpio_set_pulls(PIN_SW_VCA_WAVE_R, true, false);
    gpio_set_pulls(PIN_SW_VCA_TARGET_L, true, false);
    gpio_set_pulls(PIN_SW_VCA_TARGET_R, true, false);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCO1_L, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCO1_R, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCO2_L, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCO2_R, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCF_L, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCF_R, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCA_WAVE_L, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCA_WAVE_R, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCA_TARGET_L, true);
    gpio_set_input_hysteresis_enabled(PIN_SW_VCA_TARGET_R, true);

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
    // LFO:target [5, 4, 3, 2, 1, 0]
    SPI74HC595 led_lfo_low(SPI_PORT, PIN_CS_SR_LFO_LOW);
    // VCO:wave: [2:2, 2:1, 2:0, 1:3, 1:2, 1:1, 1:0, 2:3]
    SPI74HC595 led_vco(SPI_PORT, PIN_CS_SR_VCO);

    uint8_t vco1_wave = 0;
    bool sw_vco1_l_old = false;
    bool sw_vco1_r_old = false;
    led_vco.put(1, true);
    led_vco.put(2, false);
    led_vco.put(3, false);
    led_vco.put(4, false);

    while (true)
    {
        print_adc(adc1, adc2);
        print_sw();

        bool sw_vco1_l = !gpio_get(PIN_SW_VCO1_L);
        bool sw_vco1_r = !gpio_get(PIN_SW_VCO1_R);
        if (sw_vco1_l && !sw_vco1_l_old)
        {
            vco1_wave = (vco1_wave + 3) % 4;
        }
        if (sw_vco1_r && !sw_vco1_r_old)
        {
            vco1_wave = (vco1_wave + 1) % 4;
        }
        switch (vco1_wave)
        {
        case 0:
            led_vco.put(1, true);
            led_vco.put(2, false);
            led_vco.put(3, false);
            led_vco.put(4, false);
            break;
        case 1:
            led_vco.put(1, false);
            led_vco.put(2, true);
            led_vco.put(3, false);
            led_vco.put(4, false);
            break;
        case 2:
            led_vco.put(1, false);
            led_vco.put(2, false);
            led_vco.put(3, true);
            led_vco.put(4, false);
            break;
        case 3:
            led_vco.put(1, false);
            led_vco.put(2, false);
            led_vco.put(3, false);
            led_vco.put(4, true);
            break;

        default:
            break;
        }
        sw_vco1_l_old = sw_vco1_l;
        sw_vco1_r_old = sw_vco1_r;

        sleep_ms(10);
    }
}
