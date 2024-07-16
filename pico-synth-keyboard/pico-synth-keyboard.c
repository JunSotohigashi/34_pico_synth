#include <stdio.h>
#include "pico/stdlib.h"

#define N_PIN_HIGH 12
#define N_PIN_LOW 14

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

void scan_keyboard(uint16_t results[N_PIN_HIGH])
{
    for (uint8_t i = 0; i < N_PIN_HIGH; i++)
    {
        // set high-side
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
        uint16_t keys[N_PIN_HIGH];
        scan_keyboard(keys);
        for (uint8_t i = 0; i < N_PIN_HIGH; i++)
        {
            for (uint8_t j = 0; j < N_PIN_LOW; j++)
            {
                printf("%d", keys[i] & (1 << (13-j)) ? 1 : 0);
            }
            printf("\n");
            // printf("%04x\n", keys[i]);
        }
        printf("\n");
        sleep_ms(200);
    }
}
