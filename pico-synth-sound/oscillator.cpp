#include "oscillator.hpp"
#include "table_wave.h"
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

    Fixed_16_16 one = Fixed_16_16::from_int32(1);
    Fixed_16_16 zero = Fixed_16_16::from_int32(0);
    Fixed_16_16 value;

    switch (wave_type_now)
    {
    case WaveType::Saw:
        interp0->base[0] = wave_saw[phase16 >> 8];
        interp0->base[1] = wave_saw[(phase16 + 1) >> 8];
        interp0->accum[1] = phase16 & 0xFF;
        return value.from_raw_value(interp0->peek[1]);

    case WaveType::Sine:
        interp0->base[0] = wave_sine[phase16 >> 8];
        interp0->base[1] = wave_sine[(phase16 + 1) >> 8];
        interp0->accum[1] = phase16 & 0xFF;
        return value.from_raw_value(interp0->peek[1]);

    case WaveType::Triangle:
        interp0->base[0] = wave_triangle[phase16 >> 8];
        interp0->base[1] = wave_triangle[(phase16 + 1) >> 8];
        interp0->accum[1] = phase16 & 0xFF;
        return value.from_raw_value(interp0->peek[1]);

    case WaveType::Square:
        if (phase16 < duty)
            return one;
        else
            return -one;

    default:
        break;
    }
    return zero;
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
