#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "oscillator.hpp"

// 定数宣言
#define PIN_LED 25
#define PIN_OUT 15

// グローバル変数
queue_t sound_buffer;

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
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);

    // 出力音声のバッファー
    queue_init(&sound_buffer, sizeof(int16_t), 256);

    // PWM音声出力 約61kHz
    gpio_init(PIN_OUT);
    gpio_set_dir(PIN_OUT, GPIO_OUT);
    gpio_set_function(PIN_OUT, GPIO_FUNC_PWM);
    uint pwm_slice_num = pwm_gpio_to_slice_num(PIN_OUT);
    uint pwm_chan_num = pwm_gpio_to_channel(PIN_OUT);
    pwm_set_clkdiv_int_frac(pwm_slice_num, 0b1, 0b0);
    pwm_set_wrap(pwm_slice_num, 2048);
    pwm_set_enabled(pwm_slice_num, true);

    // 処理開始
    multicore_launch_core1(main_core1);
    main_core0();
}

// Core0のメイン関数
void main_core0()
{
    Oscillator osc1;
    Oscillator osc2;

    while (true)
    {
        static uint8_t note = 0;
        static uint16_t count = 0;
        static uint8_t wave = 0;
        count++;
        if (count == 0x4000)
        {
            count = 0;
            note++;
            note &= 0b1111111;
            if (note == 0)
            {
                wave = (wave + 1) % 5;
                switch (wave)
                {
                case 0:
                    osc1.set_wave_type(WaveType::Saw);
                    osc2.set_wave_type(WaveType::Saw);
                    break;
                case 1:
                    osc1.set_wave_type(WaveType::Sine);
                    osc2.set_wave_type(WaveType::Sine);
                    break;
                case 2:
                    osc1.set_wave_type(WaveType::Triangle);
                    osc2.set_wave_type(WaveType::Triangle);
                    break;
                case 3:
                    osc1.set_wave_type(WaveType::Square);
                    osc2.set_wave_type(WaveType::Square);
                    osc1.set_duty(32768);
                    osc2.set_duty(32768);
                    break;
                case 4:
                    osc1.set_wave_type(WaveType::Square);
                    osc2.set_wave_type(WaveType::Square);
                    osc1.set_duty(1024);
                    osc2.set_duty(1024);
                    break;
                default:
                    break;
                }
            }
            osc1.set_freq_note_number(note);
            osc2.set_freq_note_number((note+4)%128);
        }
        int16_t out_level_1 = osc1.get_out_level() / 8;
        int16_t out_level_2 = osc2.get_out_level() / 8;
        int16_t out_level = out_level_1 + out_level_2;
        queue_add_blocking(&sound_buffer, &out_level);
    }
}

// Core1のメイン関数
void main_core1()
{
    // 音声出力割り込み 40kHz(25us)周期
    static repeating_timer_t timer;
    add_repeating_timer_us(-25, &timer_callback, NULL, &timer);
}

bool timer_callback(repeating_timer_t *rt)
{
    // 音声出力バッファが空
    if (queue_is_empty(&sound_buffer))
    {
        gpio_put(PIN_LED, true);
        return true;
    }

    // バッファからの取り出し
    int16_t out_level;
    queue_try_remove(&sound_buffer, &out_level);
    gpio_put(PIN_LED, false);

    // -32768~32767(int16) から PWMのデューティー 0~2047(uint11) へ変換
    uint16_t out_duty = (out_level >> 5) + 1024;
    pwm_set_gpio_level(PIN_OUT, out_duty);

    return true;
}