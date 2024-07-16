#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "bsp/board.h"
#include "tusb.h"

// 定数宣言
#define N_PIN_HIGH 12
#define N_PIN_LOW 14
#define N_KEYS 76

// high-side GPIO: 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
// low-side GPIO: 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
const uint PIN_HIGH_MASK = 0b11111111111100;
const uint PIN_LOW_MASK = 0b111111111111110000000000000000;
const uint8_t KEY_INDEX[N_KEYS] = {80, 66, 52, 37, 23, 9, 79, 65, 51, 36, 22, 8, 78, 64, 50, 39, 25, 11, 81, 67, 53, 40, 26, 12, 82, 68, 54, 41, 27, 13, 83, 69, 55, 28, 14, 0, 70, 56, 42, 29, 15, 1, 71, 57, 43, 30, 16, 2, 72, 58, 44, 35, 21, 7, 77, 63, 49, 34, 20, 6, 76, 62, 48, 33, 19, 5, 75, 61, 47, 32, 18, 4, 74, 60, 46, 31};

// グローバル変数
queue_t key_event;
uint8_t velocity[N_KEYS];
bool gate_on[N_KEYS];

// 関数プロトタイプ宣言
int main();
void main_core0();
void main_core1();
void pin_init();
void scan_keyboard(uint16_t results[N_PIN_HIGH]);
void task_keyboard();

// メイン関数
int main()
{
    // 初期化処理
    stdio_init_all();
    board_init();
    tusb_init();
    pin_init();

    // キーボードの押下情報を格納するキュー
    queue_init(&key_event, sizeof(uint32_t), 32);

    // 処理開始
    multicore_launch_core1(main_core1);
    main_core0();
}

// Core0のメイン関数
void main_core0()
{
    while (true)
    {
        tud_task();
        while (queue_get_level(&key_event))
        {
            uint16_t event;
            queue_remove_blocking(&key_event, &event);
            uint8_t note_number = event >> 8;
            uint8_t note_velocity = event & 0xFF;
            
            uint8_t msg[3];
            msg[0] = 0x90; // Note on - CH1
            msg[1] = note_number;
            msg[2] = note_velocity;
            tud_midi_stream_write(0, msg, 3);
        }
        sleep_ms(1);
    }
}

// Core1のメイン関数
void main_core1()
{
    for (uint8_t i = 0; i < N_KEYS; i++)
    {
        velocity[i] = 127;
        gate_on[i] = false;
    }
    while (true)
    {
        task_keyboard();
        sleep_ms(2);
    }
}

// Initialize GPIO for keyboard-matrix
void pin_init()
{
    gpio_init_mask(PIN_HIGH_MASK | PIN_LOW_MASK);
    gpio_set_dir_out_masked(PIN_HIGH_MASK);
    gpio_set_dir_in_masked(PIN_LOW_MASK);

    for (uint8_t i = 0; i < 32; i++)
    {
        if ((PIN_LOW_MASK >> i) & 1)
        {
            gpio_pull_down(i);
            gpio_is_input_hysteresis_enabled(i);
        }
    }
}

// scan keyboards as matrix
void scan_keyboard(uint16_t results[N_PIN_HIGH])
{
    for (uint8_t i = 0; i < N_PIN_HIGH; i++)
    {
        // shift and set high-side
        gpio_put_masked(0b11111111111100, 1 << (i + 2));
        sleep_us(10);
        // read GPIO 16-29
        results[i] = (gpio_get_all() >> 16) & 0b11111111111111;
    }
}

void task_keyboard()
{

    // read key-matrix
    uint16_t keys_raw[N_PIN_HIGH];
    scan_keyboard(keys_raw);

    for (uint8_t i = 0; i < N_KEYS; i++)
    {
        uint8_t row = KEY_INDEX[i] / N_PIN_LOW;
        uint8_t col = KEY_INDEX[i] % N_PIN_LOW;
        bool key1 = (keys_raw[row * 2]) >> col & 1;
        bool key2 = (keys_raw[row * 2 + 1]) >> col & 1;
        if (!gate_on[i] && key1 && key2)
        {
            gate_on[i] = true;
            uint16_t event = (i + 28) << 8 | velocity[i];
            queue_add_blocking(&key_event, &event);
        }
        else if (!gate_on[i] && ((key1 && !key2) || (!key1 && key2)) && velocity[i] > 1)
        {
            velocity[i]--;
        }
        else if (gate_on[i] && !key1 && !key2)
        {
            gate_on[i] = false;
            velocity[i] = 127;
            uint16_t event = (i + 28) << 8;
            queue_add_blocking(&key_event, &event);
        }
    }
}