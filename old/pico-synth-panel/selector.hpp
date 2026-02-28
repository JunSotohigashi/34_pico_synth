#ifndef SELECTOR_HPP
#define SELECTOR_HPP

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "spi_74hc595.hpp"

const uint32_t FLASH_TARGET_OFFSET = PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;

class Selector
{
public:
    Selector(uint pin_sw_r, uint pin_sw_l, uint8_t n_state, SPI74HC595 **led_sr, uint8_t *led_pin)
        : pin_sw_r(pin_sw_r),
          pin_sw_l(pin_sw_l),
          sw_r_old(false),
          sw_l_old(false),
          n_state(n_state),
          state(0),
          led_sr(led_sr),
          led_pin(led_pin)
    {
        // read previous state from flash
        // const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
        // state = flash_data[flash_save_id];
        // gpio
        gpio_init(pin_sw_r);
        gpio_init(pin_sw_l);
        gpio_set_dir(pin_sw_r, GPIO_IN);
        gpio_set_dir(pin_sw_l, GPIO_IN);
        gpio_set_pulls(pin_sw_r, true, false);
        gpio_set_pulls(pin_sw_l, true, false);
        gpio_set_input_hysteresis_enabled(pin_sw_r, true);
        gpio_set_input_hysteresis_enabled(pin_sw_l, true);
        show_leds();
    };

    void update()
    {
        bool sw_r = !gpio_get(pin_sw_r);
        bool sw_l = !gpio_get(pin_sw_l);
        if ((sw_r && !sw_r_old) || (sw_l && !sw_l_old))
        {
            if (sw_r && !sw_r_old)
                state = (state + 1) % n_state;
            else
                state = (state + n_state - 1) % n_state;
            show_leds();
        }
        sw_r_old = sw_r;
        sw_l_old = sw_l;
    }

    uint8_t get_state()
    {
        return state;
    }

private:
    uint pin_sw_r;
    uint pin_sw_l;
    bool sw_r_old;
    bool sw_l_old;
    uint8_t n_state;
    uint8_t state;
    SPI74HC595 **led_sr;
    uint8_t *led_pin;
    void show_leds()
    {
        for (uint8_t i = 0; i < n_state; i++)
        {
            led_sr[i]->put(led_pin[i], state == i);
        }
    }
};

#endif