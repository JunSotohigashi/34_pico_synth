/**
 * @file uart_midi.hpp
 * @brief UART MIDI transmitter
 */

#pragma once

#include "midi_protocol.hpp"
#include "hardware/uart.h"
#include <cstdint>

namespace keyboard {

/**
 * @brief UART MIDI transmitter
 */
class UartMidi {
public:
    UartMidi(uart_inst_t* uart, uint tx_pin, uint rx_pin, uint32_t baud_rate);
    
    /**
     * @brief Send MIDI message via UART
     * @param msg MIDI message to send
     */
    void send(const MidiMessage& msg);
    
    /**
     * @brief Check if UART is ready to transmit
     */
    bool is_writable() const;
    
private:
    uart_inst_t* uart_;
    char tx_buffer_[9];  // Pre-allocated buffer for formatting
};

} // namespace keyboard
