#pragma once

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "spi_74hc595.hpp"
#include <cstdint>

class Selector {
private:
    uint pin_sw_r_;
    uint pin_sw_l_;
    bool sw_r_old_;
    bool sw_l_old_;
    uint8_t n_state_;
    uint8_t state_;
    SPI74HC595** led_sr_;
    uint8_t* led_pin_;
    
    void show_leds() {
        for (uint8_t i = 0; i < n_state_; i++) {
            led_sr_[i]->put(led_pin_[i], state_ == i);
        }
    }
    
public:
    Selector(uint pin_sw_r, uint pin_sw_l, uint8_t n_state, 
             SPI74HC595** led_sr, uint8_t* led_pin)
        : pin_sw_r_(pin_sw_r),
          pin_sw_l_(pin_sw_l),
          sw_r_old_(false),
          sw_l_old_(false),
          n_state_(n_state),
          state_(0),
          led_sr_(led_sr),
          led_pin_(led_pin) {
        
        // Initialize GPIO
        gpio_init(pin_sw_r_);
        gpio_init(pin_sw_l_);
        gpio_set_dir(pin_sw_r_, GPIO_IN);
        gpio_set_dir(pin_sw_l_, GPIO_IN);
        gpio_set_pulls(pin_sw_r_, true, false);  // Pull-up
        gpio_set_pulls(pin_sw_l_, true, false);
        gpio_set_input_hysteresis_enabled(pin_sw_r_, true);
        gpio_set_input_hysteresis_enabled(pin_sw_l_, true);
        
        show_leds();
    }
    
    void update() {
        bool sw_r = !gpio_get(pin_sw_r_);  // Active low
        bool sw_l = !gpio_get(pin_sw_l_);
        
        // Detect rising edge
        if ((sw_r && !sw_r_old_) || (sw_l && !sw_l_old_)) {
            if (sw_r && !sw_r_old_) {
                state_ = (state_ + 1) % n_state_;  // Right: increment
            } else {
                state_ = (state_ + n_state_ - 1) % n_state_;  // Left: decrement
            }
            show_leds();
        }
        
        sw_r_old_ = sw_r;
        sw_l_old_ = sw_l;
    }
    
    uint8_t get_state() const {
        return state_;
    }
};
