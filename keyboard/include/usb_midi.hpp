/**
 * @file usb_midi.hpp
 * @brief USB MIDI interface wrapper
 */

#pragma once

#include "midi_protocol.hpp"
#include <cstdint>

namespace keyboard {

/**
 * @brief USB MIDI device interface
 */
class UsbMidi {
public:
    UsbMidi();
    
    /**
     * @brief Initialize USB MIDI
     */
    void init();
    
    /**
     * @brief Process USB tasks (must be called frequently)
     */
    void task();
    
    /**
     * @brief Send MIDI message via USB
     * @param msg MIDI message to send
     * @return true if sent successfully
     */
    bool send(const MidiMessage& msg);
    
    /**
     * @brief Check if MIDI data is available from host
     */
    bool available() const;
    
    /**
     * @brief Read MIDI message from host
     * @param msg Output MIDI message
     * @return true if message was read
     */
    bool read(MidiMessage& msg);
    
private:
    uint8_t packet_buffer_[4];
};

} // namespace keyboard
