#pragma once

#include <cstdint>
#include "types.hpp"

class SoundUnit;

class VoiceAllocator
{
private:
    uint8_t note_to_unit_[128];
    uint8_t unit_to_note_[16];
    uint8_t unit_velocity_[16];
    SoundUnitState unit_state_[16];
    SoundUnitState unit_state_prev_[16];  // Track previous state for transitions
    uint32_t unit_history_[16];
    uint8_t last_unit_used_;
    uint32_t history_counter_;
    bool sustain_pedal_;

    uint8_t find_free_unit();
    uint8_t find_oldest_unit();

public:
    VoiceAllocator();
    void handle_note_on(uint8_t note, uint8_t velocity);
    void handle_note_off(uint8_t note);
    void handle_sustain_on();
    void handle_sustain_off();
    void update(SoundUnit *units[16]);

    uint8_t get_unit_note(uint8_t unit) const { return unit_to_note_[unit]; }
    uint8_t get_unit_velocity(uint8_t unit) const { return unit_velocity_[unit]; }
    SoundUnitState get_unit_state(uint8_t unit) const { return unit_state_[unit]; }
    uint16_t get_unit_state_bitmap() const;
};
