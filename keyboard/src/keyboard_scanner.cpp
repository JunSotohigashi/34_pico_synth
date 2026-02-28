/**
 * @file keyboard_scanner.cpp
 * @brief Keyboard matrix scanner implementation
 */

#include "keyboard_scanner.hpp"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

namespace keyboard {

KeyboardScanner::KeyboardScanner()
    : previous_state_{}, key_pressed_{}
{
}

void KeyboardScanner::init() {
    // Initialize keyboard matrix pins
    gpio_init_mask(PIN_HIGH_MASK | PIN_LOW_MASK);
    gpio_set_dir_out_masked(PIN_HIGH_MASK);
    gpio_set_dir_in_masked(PIN_LOW_MASK);
    
    // Configure pull-down and hysteresis for low-side pins
    for (uint8_t i = 0; i < 32; i++) {
        if ((PIN_LOW_MASK >> i) & 1) {
            gpio_pull_down(i);
            gpio_set_input_hysteresis_enabled(i, true);
        }
    }
}

void KeyboardScanner::scan(uint16_t* results) {
    for (uint8_t i = 0; i < N_PIN_HIGH; i++) {
        // Set high-side pin
        gpio_put_masked(PIN_HIGH_MASK, 1u << (i + 2));
        sleep_us(SCAN_DELAY_US);
        
        // Read low-side pins (GPIO 16-29)
        results[i] = gpio_get_all() >> 16;
    }
    
    // Clear all high-side pins
    gpio_put_masked(PIN_HIGH_MASK, 0);
}

KeyEvent KeyboardScanner::process_scan(const uint16_t* results, uint8_t& key_index, uint8_t& velocity) {
    // Compare with previous state to detect changes
    for (uint8_t i = 0; i < N_PIN_HIGH; i++) {
        if (results[i] != previous_state_[i]) {
            uint16_t changed = results[i] ^ previous_state_[i];
            
            // Find first changed bit
            for (uint8_t j = 0; j < N_PIN_LOW; j++) {
                if (changed & (1 << j)) {
                    uint8_t key_pos = i * N_PIN_LOW + j;
                    
                    // Check if this is a valid key
                    for (uint8_t k = 0; k < N_KEYS; k++) {
                        if (KEY_INDEX[k] == key_pos) {
                            key_index = k;
                            velocity = 127;  // Fixed velocity for now
                            
                            bool is_pressed = results[i] & (1 << j);
                            key_pressed_[k] = is_pressed;
                            previous_state_[i] = results[i];
                            
                            return is_pressed ? KeyEvent::Press : KeyEvent::Release;
                        }
                    }
                }
            }
            
            previous_state_[i] = results[i];
        }
    }
    
    return KeyEvent::None;
}

} // namespace keyboard
