/**
 * @file keyboard_scanner.cpp
 * @brief Keyboard matrix scanner implementation
 */

#include "keyboard_scanner.hpp"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

namespace keyboard {

KeyboardScanner::KeyboardScanner()
    : velocity_{}, gate_on_{}
{
    for (uint8_t i = 0; i < N_KEYS; ++i) {
        velocity_[i] = 127;
        gate_on_[i] = false;
    }
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

size_t KeyboardScanner::process_scan(const uint16_t* results, ScanEvent* events, size_t max_events) {
    size_t count = 0;

    // KEY_INDEX is mapped to 6 matrix rows, each composed of 2 scan phases.
    for (uint8_t i = 0; i < N_KEYS; ++i) {
        const uint8_t row = KEY_INDEX[i] / N_PIN_LOW;
        const uint8_t col = KEY_INDEX[i] % N_PIN_LOW;

        const bool key1 = (results[row * 2] >> col) & 1u;
        const bool key2 = (results[row * 2 + 1] >> col) & 1u;

        if (!gate_on_[i] && key1 && key2) {
            gate_on_[i] = true;
            if (count < max_events) {
                events[count++] = ScanEvent{KeyEvent::Press, i, velocity_[i]};
            }
            continue;
        }

        if (!gate_on_[i] && ((key1 && !key2) || (!key1 && key2)) && velocity_[i] > 1) {
            --velocity_[i];
            continue;
        }

        if (gate_on_[i] && !key1 && !key2) {
            gate_on_[i] = false;
            velocity_[i] = 127;
            if (count < max_events) {
                events[count++] = ScanEvent{KeyEvent::Release, i, 0};
            }
        }
    }

    return count;
}

} // namespace keyboard
