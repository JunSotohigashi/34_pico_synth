#ifndef VOICE_HPP
#define VOICE_HPP

#include "pico/stdlib.h"
#include "oscillator.hpp"
#include "envelope_generator.hpp"

// VCO,VCF,VCA,EGなどを内蔵した単一発音モジュール
class Voice
{
public:
    Voice();
    int16_t get_value();
    void set_vco1_wave_type(WaveType wave_type);
    void set_vco2_wave_type(WaveType wave_type);
    void set_vco_freq(float freq);
    void set_vco_freq_note_number(uint8_t note);
    void set_vco_duty(uint16_t duty);

private:
    Oscillator vco1;
    Oscillator vco2;
    EG vca_eg;
    float vco2_tune;

};

#endif