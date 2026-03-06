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
        unit_state_prev_[i] = SoundUnitState::IDLE;
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

    // Always transition to SUSTAIN state
    // Whether gate_off() is triggered depends on sustain_pedal_ state in update()
    unit_state_[unit] = SoundUnitState::SUSTAIN;
}

void VoiceAllocator::handle_sustain_on()
{
    sustain_pedal_ = true;
}

void VoiceAllocator::handle_sustain_off()
{
    sustain_pedal_ = false;
    // For all units in SUSTAIN state, trigger RELEASE phase immediately
    // by forcing state transition detection in next update() cycle
    for (uint8_t u = 0; u < 16; u++)
    {
        if (unit_state_[u] == SoundUnitState::SUSTAIN)
        {
            unit_state_prev_[u] = SoundUnitState::IDLE;  // Force state transition detection
        }
    }
}

void VoiceAllocator::update(SoundUnit *units[16])
{
    for (uint8_t u = 0; u < 16; u++)
    {
        // Only send gate commands on state transitions
        const SoundUnitState current = unit_state_[u];
        const SoundUnitState previous = unit_state_prev_[u];

        if (current != previous)
        {
            if (current == SoundUnitState::ACTIVE)
            {
                units[u]->gate_on(unit_to_note_[u], unit_velocity_[u]);
            }
            else if (current == SoundUnitState::SUSTAIN)
            {
                // Trigger RELEASE only if sustain pedal is not pressed
                // AND EG is not already in RELEASE state
                if (!sustain_pedal_ && units[u]->get_vca_eg()->get_state() != EGState::RELEASE)
                {
                    units[u]->gate_off();
                }
            }
            else if (current == SoundUnitState::IDLE)
            {
                // Already released, ensure gate is off
                units[u]->gate_off();
            }
            unit_state_prev_[u] = current;
        }

        // For SUSTAIN units, transition to IDLE when EG reaches IDLE state
        if (current == SoundUnitState::SUSTAIN)
        {
            // Only check after state has been stable (wait at least one cycle)
            if (previous == SoundUnitState::SUSTAIN && units[u]->get_vca_eg()->get_state() == EGState::IDLE)
            {
                unit_state_[u] = SoundUnitState::IDLE;
                uint8_t note = unit_to_note_[u];
                note_to_unit_[note] = 255;
            }
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
