#include "oscillator.hpp"

#include "table_wave_auto.h"

Oscillator::Oscillator()
    : wave_type_now_(WaveType::Saw),
      wave_type_next_(WaveType::Saw),
      phase16_(0),
      phase16_delta_(0),
      duty_(32767)
{
}

void Oscillator::set_wave_type(WaveType wave_type)
{
    wave_type_next_ = wave_type;
}

void Oscillator::set_phase16_delta(uint16_t phase16_delta)
{
    phase16_delta_ = phase16_delta;
}

void Oscillator::set_duty(uint16_t duty)
{
    duty_ = duty;
}

float Oscillator::get_value()
{
    phase16_ += phase16_delta_;
    wave_type_now_ = wave_type_next_;

    switch (wave_type_now_)
    {
    case WaveType::Saw:
        return get_wave_saw(phase16_, phase16_delta_);
    case WaveType::Sine:
        return get_wave_sine(phase16_);
    case WaveType::Triangle:
        return get_wave_tri(phase16_, phase16_delta_);
    case WaveType::Square:
        return get_wave_saw(phase16_, phase16_delta_) - get_wave_saw(static_cast<uint16_t>(phase16_ + duty_), phase16_delta_);
    default:
        return 0.0f;
    }
}
