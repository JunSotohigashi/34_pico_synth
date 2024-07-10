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
    int16_t get_out_level();
    void set_wave_type(WaveType wave_type);
    void set_freq(float freq);
    void set_freq_note_number(uint8_t note);
    void set_duty(uint16_t duty);

private:
    WaveType wave_type_now = WaveType::Saw;
    WaveType wave_type_next = WaveType::Saw;
    uint16_t phase16 = 0;
    uint16_t phase16_delta = 0;
    uint16_t duty = 32767;
};

#endif