#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/interp.h"
#include "voice.hpp"
#include "fixed_point.hpp"

// 定数宣言
#define PIN_OUT_L 14
#define PIN_OUT_R 15

// グローバル変数
queue_t sound_buffer_L;
queue_t sound_buffer_R;

// 関数プロトタイプ宣言
int main();
void main_core0();
void main_core1();
bool timer_callback(repeating_timer_t *rt);

// メイン関数
int main()
{
    stdio_init_all();

    // オンボードLED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // 出力音声のバッファー
    queue_init(&sound_buffer_L, sizeof(uint16_t), 16);
    queue_init(&sound_buffer_R, sizeof(uint16_t), 16);

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
    pwm_set_clkdiv_int_frac(pwm_slice_L, 0b1, 0b0);
    pwm_set_clkdiv_int_frac(pwm_slice_R, 0b1, 0b0);
    pwm_set_wrap(pwm_slice_L, 2048);
    pwm_set_wrap(pwm_slice_R, 2048);
    pwm_set_mask_enabled((1 << pwm_slice_L) | (1 << pwm_slice_R));

    // interpolatorによる線形補間
    interp_config interp_cfg = interp_default_config();
    interp_config_set_blend(&interp_cfg, true);
    interp_set_config(interp0, 0, &interp_cfg);
    interp_cfg = interp_default_config();
    interp_set_config(interp0, 1, &interp_cfg);

    // 処理開始
    multicore_launch_core1(main_core1);
    main_core0();
}

// Core0のメイン関数
void main_core0()
{
    Voice voice1;
    Voice voice2;
    voice1.set_vco1_wave_type(WaveType::Saw);
    voice1.set_vco2_wave_type(WaveType::Saw);
    voice2.set_vco1_wave_type(WaveType::Saw);
    voice2.set_vco2_wave_type(WaveType::Saw);
    voice1.set_vco_freq_note_number(48); // C3
    voice2.set_vco_freq_note_number(55); // G3

    while (true)
    {
        // get sound value
        int16_t out_level_1 = mul_i16_q12(voice1.get_value(), 0x0400); // ×0.25
        int16_t out_level_2 = mul_i16_q12(voice2.get_value(), 0x0400);
        // convert int16 to uint11
        uint16_t out_level_L = ((out_level_1 + out_level_2) >> 5) + 1024;
        uint16_t out_level_R = ((out_level_1 + out_level_2) >> 5) + 1024;
        queue_add_blocking(&sound_buffer_L, &out_level_L);
        queue_add_blocking(&sound_buffer_R, &out_level_R);
    }
}

// Core1のメイン関数
void main_core1()
{
    // 音声出力割り込み 40kHz(25us)周期
    static repeating_timer_t timer;
    add_repeating_timer_us(-25, &timer_callback, NULL, &timer);
}

// 40kHz周期で呼び出され, PWM音声出力を行う
bool timer_callback(repeating_timer_t *rt)
{
    // 音声出力バッファが空
    if (queue_is_empty(&sound_buffer_L) || queue_is_empty(&sound_buffer_R))
    {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        return true;
    }

    // バッファからの取り出し
    uint16_t out_level_L;
    uint16_t out_level_R;
    queue_try_remove(&sound_buffer_L, &out_level_L);
    queue_try_remove(&sound_buffer_R, &out_level_R);
    gpio_put(PICO_DEFAULT_LED_PIN, false);

    // -32768~32767(int16) から PWMのデューティー 0~2047(uint11) へ変換
    pwm_set_gpio_level(PIN_OUT_L, out_level_L);
    pwm_set_gpio_level(PIN_OUT_R, out_level_R);

    return true;
}
