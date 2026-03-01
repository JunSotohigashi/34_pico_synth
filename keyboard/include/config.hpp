/**
 * @file config.hpp
 * @brief Keyboard module configuration
 */

#pragma once

#include <cstdint>

namespace keyboard {

// Hardware pins
constexpr uint8_t PIN_UART_TX = 0;
constexpr uint8_t PIN_UART_RX = 1;
constexpr uint32_t UART_BAUD_RATE = 115200;

constexpr uint8_t PIN_PEDAL = 15;

// Keyboard matrix configuration
constexpr uint8_t N_PIN_HIGH = 12;  // High-side pins: GPIO 2-13
constexpr uint8_t N_PIN_LOW = 14;   // Low-side pins: GPIO 16-29
constexpr uint8_t N_KEYS = 76;

// High-side GPIO: 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
constexpr uint32_t PIN_HIGH_MASK = 0b11111111111100;

// Low-side GPIO: 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
constexpr uint32_t PIN_LOW_MASK = 0b111111111111110000000000000000;

// Timing
constexpr uint32_t SCAN_INTERVAL_MS = 2;
constexpr uint32_t SCAN_DELAY_US = 10;

// Key mapping (76 keys)
constexpr uint8_t MIDI_BASE_NOTE = 28;

constexpr uint8_t KEY_INDEX[N_KEYS] = {
    80, 66, 52, 37, 23, 9, 79, 65, 51, 36, 22, 8, 78, 64, 50, 39, 25, 11,
    81, 67, 53, 40, 26, 12, 82, 68, 54, 41, 27, 13, 83, 69, 55, 28, 14, 0,
    70, 56, 42, 29, 15, 1, 71, 57, 43, 30, 16, 2, 72, 58, 44, 35, 21, 7,
    77, 63, 49, 34, 20, 6, 76, 62, 48, 33, 19, 5, 75, 61, 47, 32, 18, 4,
    74, 60, 46, 31
};

} // namespace keyboard
