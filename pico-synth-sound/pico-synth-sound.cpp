#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/interp.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "voice.hpp"
#include "fixed_point.hpp"

// constants
#define PIN_OUT_L 14
#define PIN_OUT_R 15

#define SPI_PORT_SOUND spi0
#define PIN_SCK_SOUND 18
#define PIN_RX_SOUND 16
#define PIN_TX_SOUND 19
#define PIN_CS_SOUND 17

#define STREAM_LENGTH 34

#define UNIT_1 1
#define UNIT_2 2

// global variablea
queue_t sound_buffer;
uint16_t stream[STREAM_LENGTH];
volatile uint8_t stream_index = 0;

// prototypes
/**
 * \brief main function
 *
 * \return int
 */
int main();

/**
 * \brief main function for core 0
 *
 * \note Core 0 generates sound data and stream to Core 1
 */
void main_core0();

/**
 * \brief main function for core 1
 *
 * \note Core 1 receives sound data and output using PWM
 * \note Repeating timer is used for precise intervals
 */
void main_core1();

/**
 * \brief called 40kHz cycle, output sound using PWM
 *
 * \param rt handler of repeating timer
 * \return Always return true for enabling next callback
 */
bool timer_callback(repeating_timer_t *rt);

void spi_callback()
{
    uint16_t received;
    spi_read16_blocking(SPI_PORT_SOUND, 0, &received, 1);
    if (received == 0xffff)
        stream_index = 0;
    stream[stream_index] = received;
    stream_index = (stream_index + 1) % STREAM_LENGTH;
}

int main()
{
    // overclock
    set_sys_clock_khz(200000, true);

    stdio_init_all();

    // オンボードLED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // 出力音声のバッファー
    queue_init(&sound_buffer, sizeof(uint32_t), 64);

    // PWM音声出力 125MHzの時 約61kHz
    gpio_init(PIN_OUT_L);
    gpio_init(PIN_OUT_R);
    gpio_set_dir(PIN_OUT_L, GPIO_OUT);
    gpio_set_dir(PIN_OUT_R, GPIO_OUT);
    gpio_set_function(PIN_OUT_L, GPIO_FUNC_PWM);
    gpio_set_function(PIN_OUT_R, GPIO_FUNC_PWM);
    uint pwm_slice_L = pwm_gpio_to_slice_num(PIN_OUT_L);
    uint pwm_slice_R = pwm_gpio_to_slice_num(PIN_OUT_R);
    uint pwm_chan_L = pwm_gpio_to_channel(PIN_OUT_L);
    uint pwm_chan_R = pwm_gpio_to_channel(PIN_OUT_R);
    pwm_set_clkdiv_int_frac(pwm_slice_L, 1, 0);
    pwm_set_clkdiv_int_frac(pwm_slice_R, 1, 0);
    pwm_set_wrap(pwm_slice_L, 2048);
    pwm_set_wrap(pwm_slice_R, 2048);
    pwm_set_mask_enabled((1 << pwm_slice_L) | (1 << pwm_slice_R));

    // interpolatorによる線形補間
    interp_config interp_cfg = interp_default_config();
    interp_config_set_blend(&interp_cfg, true);
    interp_set_config(interp0, 0, &interp_cfg);
    interp_cfg = interp_default_config();
    interp_config_set_signed(&interp_cfg, true);
    interp_set_config(interp0, 1, &interp_cfg);

    // SPI
    spi_init(SPI_PORT_SOUND, 1000 * 1000);
    spi_set_slave(SPI_PORT_SOUND, true);
    spi_set_format(SPI_PORT_SOUND, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_SCK_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_TX_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_RX_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_SOUND, GPIO_FUNC_SPI);
    spi0_hw->imsc = SPI_SSPIMSC_RXIM_BITS; // Enable the RX FIFO interrupt (RXIM)
    irq_set_enabled(SPI0_IRQ, 1);
    irq_set_exclusive_handler(SPI0_IRQ, spi_callback);

    // 処理開始
    multicore_launch_core1(main_core1);
    main_core0();
}

void main_core0()
{
    Voice voice1;
    Voice voice2;

    // Voice voice[n_voice];
    // for (uint8_t i = 0; i < n_voice; i++)
    // {
    //     voice[i].set_vco1_wave_type(WaveType::Saw);
    //     voice[i].set_vco2_wave_type(WaveType::Saw);
    // }

    bool gate1_old = false;
    bool gate2_old = false;

    uint16_t input_cycle = 0;
    Fixed_16_16 gain_unit = Fixed_16_16::from_float(0.5);

    while (true)
    {
        // 100Hz cycle
        if (input_cycle % 400 == 0)
        {
            // gate and note control
            bool gate1 = stream[UNIT_1] >> 15;
            bool gate2 = stream[UNIT_2] >> 15;
            if (!gate1_old && gate1)
            {
                voice1.set_vco_freq_note_number(stream[UNIT_1] >> 8 & 127);
                voice1.gate_on();
            }
            if (!gate2_old && gate2)
            {
                voice2.set_vco_freq_note_number(stream[UNIT_2] >> 8 & 127);
                voice2.gate_on();
            }
            if (gate1_old && !gate1)
            {
                voice1.gate_off();
            }
            if (gate2_old && !gate2)
            {
                voice2.gate_off();
            }
            gate1_old = gate1;
            gate2_old = gate2;

            // VCO wavetype select
            WaveType vco1_wave = static_cast<WaveType>(stream[17] >> 10 & 3);
            WaveType vco2_wave = static_cast<WaveType>(stream[17] >> 8 & 3);
            voice1.set_vco1_wave_type(vco1_wave);
            voice1.set_vco2_wave_type(vco2_wave);
            voice2.set_vco1_wave_type(vco1_wave);
            voice2.set_vco2_wave_type(vco2_wave);

            // VCO square wave duty control
            uint16_t vco_duty = (stream[18] << 5) + 200;
            voice1.set_vco_duty(vco_duty);
            voice2.set_vco_duty(vco_duty);

            // VCO2 tune control
            float vco2_tune = powf(2.0f, powf((static_cast<float>(stream[19]) - 512.0f) / 512.0f, 3.0f)); // 0->0.5, 512->1.0, 1024->2.0, fine tuning near center
            voice1.set_vco2_tune(vco2_tune);
            voice2.set_vco2_tune(vco2_tune);

            // VCO mixture control
            uint16_t vco_mix = stream[20] << 6;
            voice1.set_vco_mix(vco_mix);
            voice2.set_vco_mix(vco_mix);

            // VCF type, cutoff frequency and resonance control
            bool vcf_is_hpf = (stream[17] >> 7) & 1;
            uint16_t vcf_freq = stream[21] << 5;
            Fixed_16_16 vcf_res = Fixed_16_16::from_raw_value((static_cast<int32_t>(stream[22]) << 9) + 46341); // Q>=1/sqrt(2)
            voice1.set_vcf_freq_res(vcf_is_hpf, vcf_freq, vcf_res);
            voice2.set_vcf_freq_res(vcf_is_hpf, vcf_freq, vcf_res);

            // VCF EG
            uint16_t vcf_attack = stream[23] >> 2 << 8; // forces 0~3 to 0
            uint16_t vcf_decay = stream[24] >> 2 << 8;
            uint16_t vcf_amount = stream[25] << 6;
            voice1.set_vcf_eg(vcf_attack, vcf_decay, vcf_amount);
            voice2.set_vcf_eg(vcf_attack, vcf_decay, vcf_amount);

            // VCA EG
            uint16_t vca_attack = stream[27] >> 2 << 8; // forces 0~3 to 0
            uint16_t vca_decay = stream[28] >> 2 << 8;
            uint16_t vca_sustain = stream[29] << 6;
            uint16_t vca_release = stream[30] >> 2 << 8;
            voice1.set_vca_eg(vca_attack, vca_decay, vca_sustain, vca_release);
            voice2.set_vca_eg(vca_attack, vca_decay, vca_sustain, vca_release);

            // VCA Gain
            uint16_t vca_gain = stream[31] << 6;
            voice1.set_vca_gain(vca_gain);
            voice2.set_vca_gain(vca_gain);
        }

        // 1Hz cycle
        if (input_cycle == 0)
        {
            printf("UNIT: %d, %d\n", UNIT_1, UNIT_2);
        }

        input_cycle = (input_cycle + 1) % 40000;

        // get sound value
        Fixed_16_16 voice_value = (voice1.get_value() + voice2.get_value()) * gain_unit;

        uint32_t out_level = voice_value.raw_value + 0x10000; // remove sign
        uint16_t out_level16 = out_level >> 2;

        uint16_t out_level_L = out_level16;
        uint16_t out_level_R = out_level16;
        // write to sound buffer
        uint32_t out_level_LR = ((uint32_t)out_level_L << 16) | out_level_R;
        queue_add_blocking(&sound_buffer, &out_level_LR);
    }
}

void main_core1()
{
    // launch 40kHz(25us) timer for sound output
    static repeating_timer_t timer;
    add_repeating_timer_us(-25, &timer_callback, NULL, &timer);
}

bool timer_callback(repeating_timer_t *rt)
{
    // In case of no stream, do nothing
    if (queue_is_empty(&sound_buffer))
    {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        return true;
    }
    // Pop value from the queue
    uint32_t out_level;
    queue_try_remove(&sound_buffer, &out_level);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    // Convert 0~65535(uint16) to 0~2047(uint11)
    pwm_set_gpio_level(PIN_OUT_R, (out_level >> 5) & 0x7FF);
    pwm_set_gpio_level(PIN_OUT_L, (out_level >> 16 >> 5) & 0x7FF);

    return true;
}
