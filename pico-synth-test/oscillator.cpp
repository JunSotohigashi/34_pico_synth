#include "oscillator.hpp"
#include "table_wave.h"
#include "table_freq.h"

int16_t Oscillator::get_out_level()
{
    // uint16_t phase16_old = this->phase16;
    this->phase16 += this->phase16_delta;

    // if (this->phase16 < phase16_old)
    //     this->wave_type_now = this->wave_type_next;
    this->wave_type_now = this->wave_type_next;
    
    switch (this->wave_type_now)
    {
    case WaveType::Saw:
        return wave_saw[this->phase16 >> 8];

    case WaveType::Sine:
        return wave_sine[this->phase16 >> 8];

    case WaveType::Triangle:
        return wave_triangle[this->phase16 >> 8];

    case WaveType::Square:
        if (this->phase16 < this->duty)
            return 32767;
        else
            return -32767;

    default:
        break;
    }
    return 0;
}

void Oscillator::set_wave_type(WaveType wave_type)
{
    this->wave_type_next = wave_type;
}

void Oscillator::set_freq(float freq)
{
    this->phase16_delta = freq * 65536.0f / 40000.0f;
}

void Oscillator::set_freq_note_number(uint8_t note)
{
    this->phase16_delta = note_phase16_delta[note & 0b1111111];
}

void Oscillator::set_duty(uint16_t duty)
{
    this->duty = duty;
}
