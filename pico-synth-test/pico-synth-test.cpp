#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "table_wave.h"
#include "table_freq.h"

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
    queue_init(&sound_buffer, sizeof(int16_t), 8);

    // PWM音声出力 約60kHz
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
                wave = (wave + 1) % 3;
            }
        }

        static const int16_t *wave_table[3] = {wave_saw, wave_sine, wave_triangle};

        static uint16_t phase16_1 = 0;
        phase16_1 += note_phase16_delta[note];
        uint8_t phase8_1 = phase16_1 >> 8;
        int16_t out_level_1 = wave_table[wave][phase8_1] / 8;

        static uint16_t phase16_2 = 0;
        phase16_2 += note_phase16_delta[(note + 3) % 128];
        uint8_t phase8_2 = phase16_2 >> 8;
        int16_t out_level_2 = wave_table[wave][phase8_2] / 8;

        static uint16_t phase16_3 = 0;
        phase16_3 += note_phase16_delta[(note + 5) % 128];
        uint8_t phase8_3 = phase16_3 >> 8;
        int16_t out_level_3 = wave_table[wave][phase8_3] / 8;

        static uint16_t phase16_4 = 0;
        phase16_4 += note_phase16_delta[(note + 7) % 128];
        uint8_t phase8_4 = phase16_4 >> 8;
        int16_t out_level_4 = wave_table[wave][phase8_4] / 8;

        static uint16_t phase16_5 = 0;
        phase16_5 += note_phase16_delta[(note + 10) % 128];
        uint8_t phase8_5 = phase16_5 >> 8;
        int16_t out_level_5 = wave_table[wave][phase8_5] / 8;

        static uint16_t phase16_6 = 0;
        phase16_6 += note_phase16_delta[(note + 14) % 128];
        uint8_t phase8_6 = phase16_6 >> 8;
        int16_t out_level_6 = wave_table[wave][phase8_6] / 8;

        static uint16_t phase16_7 = 0;
        phase16_7 += note_phase16_delta[(note + 17) % 128];
        uint8_t phase8_7 = phase16_7 >> 8;
        int16_t out_level_7 = wave_table[wave][phase8_7] / 8;

        static uint16_t phase16_8 = 0;
        phase16_8 += note_phase16_delta[(note + 22) % 128];
        uint8_t phase8_8 = phase16_8 >> 8;
        int16_t out_level_8 = wave_table[wave][phase8_8] / 8;

        int16_t out_level = out_level_1 + out_level_2 + out_level_3 + out_level_4 + out_level_5 + out_level_6 + out_level_7 + out_level_8;

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