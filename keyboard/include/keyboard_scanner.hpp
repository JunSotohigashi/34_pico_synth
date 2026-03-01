/**
 * @file keyboard_scanner.hpp
 * @brief Keyboard matrix scanner
 */

#pragma once

#include "config.hpp"
#include <cstddef>
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

struct ScanEvent {
    KeyEvent type;
    uint8_t key_index;
    uint8_t velocity;
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
     * @param events Output buffer for detected events
     * @param max_events Maximum number of events that can be written
     * @return Number of events written to events[]
     */
    size_t process_scan(const uint16_t* results, ScanEvent* events, size_t max_events);
    
private:
    std::array<uint8_t, N_KEYS> velocity_;
    std::array<bool, N_KEYS> gate_on_;
};

} // namespace keyboard
