#pragma once

#include "pico/stdlib.h"
#include "fixed_point.hpp"

enum class WaveType : uint8_t
{
    Saw = 0,
    Sine = 1,
    Triangle = 2,
    Square = 3
};

class Oscillator
{
public:
    Oscillator();

    void set_wave_type(WaveType wave_type);
    void set_phase16_delta(uint16_t phase16_delta);
    void set_duty(uint16_t duty);

    Fixed_16_16 get_value();

private:
    WaveType wave_type_now_;
    WaveType wave_type_next_;
    uint16_t phase16_;
    uint16_t phase16_delta_;
    uint16_t duty_;
};
