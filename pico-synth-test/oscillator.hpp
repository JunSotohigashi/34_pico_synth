#ifndef OSCILLATOR_HPP
#define OSCILLATOR_HPP

#include "pico/stdlib.h"

enum class WaveType
{
    Saw,
    Sine,
    Triangle,
    Square
};

class Oscillator
{
public:
    Oscillator();
    int16_t get_value();
    void set_wave_type(WaveType wave_type);
    void set_phase16_delta(uint16_t phase16_delta);
    void set_duty(uint16_t duty);

private:
    WaveType wave_type_now;
    WaveType wave_type_next;
    uint16_t phase16;
    uint16_t phase16_delta;
    uint16_t duty;
};

#endif