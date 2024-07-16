#include <stdio.h>
#include "pico/stdlib.h"

#define N_PIN_HIGH 12
#define N_PIN_LOW 14
#define N_KEYS 76

// high-side GPIO: 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
// low-side GPIO: 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
const uint PIN_HIGH_MASK = 0b11111111111100;
const uint PIN_LOW_MASK = 0b111111111111110000000000000000;

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
        sleep_us(100);
        // read GPIO 16-29
        results[i] = (gpio_get_all() >> 16) & 0b11111111111111;
    }
}

int main()
{
    stdio_init_all();
    pin_init();

    while (true)
    {
        // read key-matrix
        uint16_t keys_raw[N_PIN_HIGH];
        scan_keyboard(keys_raw);

        const uint8_t keys_index[N_KEYS] = {80, 66, 52, 37, 23, 9, 79, 65, 51, 36, 22, 8, 78, 64, 50, 39, 25, 11, 81, 67, 53, 40, 26, 12, 82, 68, 54, 41, 27, 13, 83, 69, 55, 28, 14, 0, 70, 56, 42, 29, 15, 1, 71, 57, 43, 30, 16, 2, 72, 58, 44, 35, 21, 7, 77, 63, 49, 34, 20, 6, 76, 62, 48, 33, 19, 5, 75, 61, 47, 32, 18, 4, 74, 60, 46, 31};
        bool keys_xor[N_KEYS];
        bool keys_and[N_KEYS];

        for (uint8_t i = 0; i < N_KEYS; i++)
        {
            uint8_t row = keys_index[i] / N_PIN_LOW;
            uint8_t col = keys_index[i] % N_PIN_LOW;
            bool key1 = (keys_raw[row * 2]) >> col & 1;
            bool key2 = (keys_raw[row * 2 + 1]) >> col & 1;
            keys_xor[i] = (key1 && !key2) || (!key1 && key2);
            keys_and[i] = key1 && key2;
        }

        for (uint8_t i = 0; i < N_KEYS; i++)
        {
            printf("%d", keys_and[i] ? 1 : 0);
        }

        // for (uint8_t i = 0; i < N_PIN_HIGH; i++)
        // {
        //     for (uint8_t j = 0; j < N_PIN_LOW; j++)
        //     {
        //         printf("%d", keys_raw[i] & (1 << (13 - j)) ? 1 : 0);
        //     }
        //     printf("\n");
        //     // printf("%04x\n", keys[i]);
        // }
        printf("\n");
        sleep_ms(20);
    }
}
