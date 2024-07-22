#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/interp.h"
#include "hardware/adc.h"
#include "voice.hpp"
#include "fixed_point.hpp"

// constants
#define PIN_OUT_L 14
#define PIN_OUT_R 15
#define PIN_BTN_1 12
#define PIN_BTN_2 13
#define PIN_VR_1 26

// global variablea
queue_t sound_buffer;

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

int main()
{
    stdio_init_all();

    // オンボードLED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // ボタン
    gpio_init(PIN_BTN_1);
    gpio_init(PIN_BTN_2);
    gpio_set_dir(PIN_BTN_1, GPIO_IN);
    gpio_set_dir(PIN_BTN_2, GPIO_IN);
    gpio_set_pulls(PIN_BTN_1, true, false);
    gpio_set_pulls(PIN_BTN_2, true, false);
    adc_init();
    adc_gpio_init(PIN_VR_1);
    adc_select_input(0);

    // 出力音声のバッファー
    queue_init(&sound_buffer, sizeof(uint32_t), 64);

    // PWM音声出力 約61kHz
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

    // 処理開始
    multicore_launch_core1(main_core1);
    main_core0();
}

void main_core0()
{
    uint8_t n_voice = 2;

    Voice voice[n_voice];
    for (uint8_t i = 0; i < n_voice; i++)
    {
        voice[i].set_vco1_wave_type(WaveType::Saw);
        voice[i].set_vco2_wave_type(WaveType::Saw);
    }

    voice[0].set_vco_freq_note_number(36);
    voice[1].set_vco_freq_note_number(52);

    bool btn1_old = false;
    bool btn2_old = false;

    uint16_t input_cycle = 0;
    Fixed_16_16 gain_unit = Fixed_16_16::from_float(0.5);

    while (true)
    {
        if (input_cycle % 400 == 0)
        {
            bool btn1 = !gpio_get(PIN_BTN_1);
            bool btn2 = !gpio_get(PIN_BTN_2);
            if (!btn1_old && btn1)
            {
                voice[0].gate_on();
            }
            if (!btn2_old && btn2)
            {
                voice[1].gate_on();
            }
            if (btn1_old && !btn1)
            {
                voice[0].gate_off();
            }
            if (btn2_old && !btn2)
            {
                voice[1].gate_off();
            }

            btn1_old = btn1;
            btn2_old = btn2;

            uint16_t vr1 = adc_read();
            voice[0].set_vcf_freq_res(20.0f + 10000.0f * (float)vr1 / 4095.0f, 2.0f);
            voice[1].set_vcf_freq_res(20.0f + 10000.0f * (float)vr1 / 4095.0f, 2.0f);
            // voice[0].set_vco_duty(vr1 << 3);
            // voice[1].set_vco_duty(vr1 << 3);
        }

        if (input_cycle == 0)
        {
            printf("Hello world\n");
        }

        input_cycle = (input_cycle + 1) % 40000;

        // get sound value
        Fixed_16_16 voice_value = Fixed_16_16::zero;
        for (uint8_t i = 0; i < n_voice; i++)
        {
            voice_value += voice[i].get_value() * gain_unit;
        }

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
