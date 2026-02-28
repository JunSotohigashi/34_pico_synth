/**
 * @file usb_midi.cpp
 * @brief USB MIDI interface implementation
 */

#include "usb_midi.hpp"
#include "bsp/board.h"
#include "tusb.h"

namespace keyboard {

UsbMidi::UsbMidi()
    : packet_buffer_{}
{
}

void UsbMidi::init() {
    board_init();
    tusb_init();
}

void UsbMidi::task() {
    tud_task();
}

bool UsbMidi::send(const MidiMessage& msg) {
    if (!tud_midi_mounted()) {
        return false;
    }
    
    uint8_t packet[3] = {msg.status, msg.data1, msg.data2};
    uint32_t written = tud_midi_stream_write(0, packet, 3);
    return written == 3;
}

bool UsbMidi::available() const {
    return tud_midi_available() > 0;
}

bool UsbMidi::read(MidiMessage& msg) {
    if (!tud_midi_available()) {
        return false;
    }
    
    if (tud_midi_packet_read(packet_buffer_)) {
        // packet_buffer_[0] = Cable Number + Code Index Number
        // packet_buffer_[1] = MIDI status
        // packet_buffer_[2] = MIDI data1
        // packet_buffer_[3] = MIDI data2
        
        // Convert Note Off (0x80) to Note On with velocity 0
        if (packet_buffer_[1] == 0x80) {
            msg.status = 0x90;  // Note On
            msg.data1 = packet_buffer_[2];
            msg.data2 = 0;  // Velocity 0 = Note Off
        } else {
            msg.status = packet_buffer_[1];
            msg.data1 = packet_buffer_[2];
            msg.data2 = packet_buffer_[3];
        }
        
        return true;
    }
    
    return false;
}

} // namespace keyboard
