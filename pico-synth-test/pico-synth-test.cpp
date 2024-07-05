#include <stdio.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
// #include "hardware/spi.h"
// #include "hardware/timer.h"

// // SPI Defines
// // We are going to use SPI 0, and allocate it to the following GPIO pins
// // Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// #define SPI_PORT spi0
// #define PIN_MISO 16
// #define PIN_CS   17
// #define PIN_SCK  18
// #define PIN_MOSI 19

// int64_t alarm_callback(alarm_id_t id, void *user_data) {
//     // Put your timeout handler code in here
//     return 0;
// }

#define PIN_LED 25
#define PIN_OUT 15

bool timer_callback(repeating_timer_t *rt)
{
    static double phase1 = 0.0f;
    // static double phase2 = 0.0f;
    static uint16_t count = 0;
    static uint8_t i = 0;
    double freq1[8] = {261.626, 293.665, 329.628, 349.228, 391.995, 440.0, 493.883, 523.251};
    double phase_delta1 = 2048*freq1[i]/40000.0;
    // double phase_delta2 = 2048*(freq1[i]+2.0)/40000.0;
    phase1 = fmod(phase1 + phase_delta1, 2048.0);
    // phase2 = fmod(phase2 + phase_delta2, 2048.0);
    // uint16_t out_level = (((uint16_t)phase1>>1) + ((uint16_t)phase2>>1)) & 0b11111111111;
    uint16_t out_level = ((uint16_t)phase1>>1) & 0b11111111111;
    pwm_set_gpio_level(PIN_OUT, out_level);

    count++;
    if(count > 40000){
        count = 0;
        i = (i+1) % 8;
    }

    return true;
}

void main_core1()
{
    gpio_init(PIN_OUT);
    gpio_set_dir(PIN_OUT, GPIO_OUT);
    gpio_set_function(PIN_OUT, GPIO_FUNC_PWM);
    uint pwm_slice_num = pwm_gpio_to_slice_num(PIN_OUT);
    uint pwm_chan_num = pwm_gpio_to_channel(PIN_OUT);
    pwm_set_clkdiv_int_frac(pwm_slice_num, 0b11, 0b0);
    pwm_set_wrap(pwm_slice_num, 2048);
    pwm_set_enabled(pwm_slice_num, true);

    static repeating_timer_t timer;
    add_repeating_timer_us(-25, &timer_callback, NULL, &timer);

    multicore_fifo_pop_blocking();
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    // spi_init(SPI_PORT, 1000*1000);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    // gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    // gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    // gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    // gpio_set_dir(PIN_CS, GPIO_OUT);
    // gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // Timer example code - This example fires off the callback after 2000ms
    // add_alarm_in_ms(2000, alarm_callback, NULL, false);
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    multicore_launch_core1(main_core1);

    while (true)
    {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
