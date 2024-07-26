#include "oscillator.hpp"
#include "table_wave_auto.h"
#include "hardware/interp.h"

Oscillator::Oscillator()
    : wave_type_now(WaveType::Saw),
      wave_type_next(WaveType::Saw),
      phase16(0),
      phase16_delta(0),
      duty(32767)
{
}

Fixed_16_16 Oscillator::get_value()
{
    phase16 += phase16_delta;
    wave_type_now = wave_type_next; // TODO: 0交差時に切り替え

    switch (wave_type_now)
    {
    case WaveType::Saw:
        return get_wave_saw(phase16, phase16_delta);

    case WaveType::Sine:
        return get_wave_sine(phase16);

    case WaveType::Triangle:
        return get_wave_tri(phase16, phase16_delta);

    case WaveType::Square:
        return get_wave_saw(phase16, phase16_delta) - get_wave_saw(phase16 + duty, phase16_delta);

    default:
        break;
    }
    return Fixed_16_16::zero;
}

void Oscillator::set_wave_type(WaveType wave_type)
{
    this->wave_type_next = wave_type;
}

void Oscillator::set_phase16_delta(uint16_t phase16_delta)
{
    this->phase16_delta = phase16_delta;
}

void Oscillator::set_duty(uint16_t duty)
{
    this->duty = duty;
}
