/**
 * @file midi_protocol.hpp
 * @brief MIDI protocol definitions and utilities
 */

#pragma once

#include <cstdint>
#include <array>

namespace keyboard {

// MIDI Status Bytes
enum class MidiStatus : uint8_t {
    NoteOff = 0x80,
    NoteOn = 0x90,
    ControlChange = 0xB0,
};

// MIDI Control Change Numbers
enum class MidiCC : uint8_t {
    Sustain = 0x40,
};

/**
 * @brief MIDI message structure
 */
struct MidiMessage {
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
    
    constexpr MidiMessage() : status(0), data1(0), data2(0) {}
    
    constexpr MidiMessage(uint8_t s, uint8_t d1, uint8_t d2)
        : status(s), data1(d1), data2(d2) {}
    
    static constexpr MidiMessage note_on(uint8_t note, uint8_t velocity) {
        return MidiMessage(
            static_cast<uint8_t>(MidiStatus::NoteOn),
            note,
            velocity
        );
    }
    
    static constexpr MidiMessage note_off(uint8_t note) {
        return MidiMessage(
            static_cast<uint8_t>(MidiStatus::NoteOn),
            note,
            0  // Velocity 0 = Note Off
        );
    }
    
    static constexpr MidiMessage sustain_on() {
        return MidiMessage(
            static_cast<uint8_t>(MidiStatus::ControlChange),
            static_cast<uint8_t>(MidiCC::Sustain),
            127
        );
    }
    
    static constexpr MidiMessage sustain_off() {
        return MidiMessage(
            static_cast<uint8_t>(MidiStatus::ControlChange),
            static_cast<uint8_t>(MidiCC::Sustain),
            0
        );
    }
};

/**
 * @brief Format MIDI message as hex string for UART
 * @param msg MIDI message
 * @param buffer Output buffer (at least 9 bytes)
 * @return Number of bytes written
 */
inline uint8_t format_midi_hex(const MidiMessage& msg, char* buffer) {
    static constexpr char hex[] = "0123456789abcdef";
    
    buffer[0] = hex[msg.status >> 4];
    buffer[1] = hex[msg.status & 0x0F];
    buffer[2] = ' ';
    buffer[3] = hex[msg.data1 >> 4];
    buffer[4] = hex[msg.data1 & 0x0F];
    buffer[5] = ' ';
    buffer[6] = hex[msg.data2 >> 4];
    buffer[7] = hex[msg.data2 & 0x0F];
    buffer[8] = '\n';
    
    return 9;
}

} // namespace keyboard
