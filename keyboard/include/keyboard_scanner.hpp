/**
 * @file keyboard_scanner.hpp
 * @brief Keyboard matrix scanner
 */

#pragma once

#include "config.hpp"
#include <cstdint>
#include <array>

namespace keyboard {

/**
 * @brief Keyboard event type
 */
enum class KeyEvent : uint8_t {
    None,
    Press,
    Release,
};

/**
 * @brief Keyboard scanner for matrix scanning
 */
class KeyboardScanner {
public:
    KeyboardScanner();
    
    /**
     * @brief Initialize GPIO for keyboard matrix
     */
    void init();
    
    /**
     * @brief Scan keyboard matrix once
     * @param results Output array for scan results (N_PIN_HIGH elements)
     */
    void scan(uint16_t* results);
    
    /**
     * @brief Process scan results and detect key events
     * @param results Scan results from scan()
     * @param key_index Output: index of changed key (if any)
     * @param velocity Output: velocity value (fixed at 127 for now)
     * @return Key event type
     */
    KeyEvent process_scan(const uint16_t* results, uint8_t& key_index, uint8_t& velocity);
    
private:
    std::array<uint16_t, N_PIN_HIGH> previous_state_;
    std::array<bool, N_KEYS> key_pressed_;
};

} // namespace keyboard
