#include "voice_allocator.hpp"
#include "sound_unit.hpp"

uint8_t VoiceAllocator::find_free_unit()
{
    uint8_t start = (last_unit_used_ == 255) ? 0 : (last_unit_used_ + 1) % 16;
    for (uint8_t i = 0; i < 16; i++)
    {
        uint8_t u = (start + i) % 16;
        if (unit_state_[u] == SoundUnitState::IDLE)
        {
            return u;
        }
    }
    return 255;
}

uint8_t VoiceAllocator::find_oldest_unit()
{
    uint32_t min_history = UINT32_MAX;
    uint8_t oldest = 0;
    for (uint8_t u = 0; u < 16; u++)
    {
        if (unit_history_[u] < min_history)
        {
            min_history = unit_history_[u];
            oldest = u;
        }
    }
    return oldest;
}

VoiceAllocator::VoiceAllocator() : last_unit_used_(255), history_counter_(1), sustain_pedal_(false)
{
    for (uint8_t i = 0; i < 128; i++)
    {
        note_to_unit_[i] = 255;
    }
    for (uint8_t i = 0; i < 16; i++)
    {
        unit_to_note_[i] = 0;
        unit_velocity_[i] = 0;
        unit_state_[i] = SoundUnitState::IDLE;
        unit_history_[i] = 0;
    }
}

void VoiceAllocator::handle_note_on(uint8_t note, uint8_t velocity)
{
    uint8_t unit = note_to_unit_[note];

    if (unit != 255 && unit_state_[unit] != SoundUnitState::IDLE)
    {
        unit_velocity_[unit] = velocity;
        unit_state_[unit] = SoundUnitState::ACTIVE;
        unit_history_[unit] = history_counter_++;
        return;
    }

    unit = find_free_unit();
    if (unit == 255)
    {
        unit = find_oldest_unit();
        uint8_t old_note = unit_to_note_[unit];
        note_to_unit_[old_note] = 255;
    }

    note_to_unit_[note] = unit;
    unit_to_note_[unit] = note;
    unit_velocity_[unit] = velocity;
    unit_state_[unit] = SoundUnitState::ACTIVE;
    unit_history_[unit] = history_counter_++;
    last_unit_used_ = unit;
}

void VoiceAllocator::handle_note_off(uint8_t note)
{
    uint8_t unit = note_to_unit_[note];
    if (unit == 255)
        return;

    if (sustain_pedal_)
    {
        unit_state_[unit] = SoundUnitState::SUSTAIN;
    }
    else
    {
        unit_state_[unit] = SoundUnitState::IDLE;
        note_to_unit_[note] = 255;
    }
}

void VoiceAllocator::handle_sustain_on()
{
    sustain_pedal_ = true;
}

void VoiceAllocator::handle_sustain_off()
{
    sustain_pedal_ = false;
    for (uint8_t u = 0; u < 16; u++)
    {
        if (unit_state_[u] == SoundUnitState::SUSTAIN)
        {
            unit_state_[u] = SoundUnitState::IDLE;
            uint8_t note = unit_to_note_[u];
            note_to_unit_[note] = 255;
        }
    }
}

void VoiceAllocator::update(SoundUnit *units[16])
{
    for (uint8_t u = 0; u < 16; u++)
    {
        if (unit_state_[u] == SoundUnitState::ACTIVE)
        {
            units[u]->gate_on(unit_to_note_[u], unit_velocity_[u]);
        }
        else if (unit_state_[u] == SoundUnitState::IDLE)
        {
            units[u]->gate_off();
        }
    }
}

uint16_t VoiceAllocator::get_unit_state_bitmap() const
{
    uint16_t bitmap = 0;
    for (uint8_t u = 0; u < 16; u++)
    {
        if (unit_state_[u] != SoundUnitState::IDLE)
        {
            bitmap |= (1 << u);
        }
    }
    return bitmap;
}
