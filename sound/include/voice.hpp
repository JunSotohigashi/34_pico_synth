#pragma once

#include "pico/stdlib.h"

#include "oscillator.hpp"
#include "filter.hpp"

class Voice
{
public:
    Voice();

    void update();

    float get_value_l() const;
    float get_value_r() const;

    void set_vco1_wave_type(WaveType wave_type);
    void set_vco2_wave_type(WaveType wave_type);
    void set_vco1_phase_delta(uint16_t phase_delta);
    void set_vco2_phase_delta(uint16_t phase_delta);
    void set_vco_duty(uint16_t duty);
    void set_vco_mix(uint16_t mix);

    void set_vcf_type(bool is_hpf);
    void set_vcf_coefficient_by_index(uint8_t index, float value);

    void set_vca_gain_lr(uint16_t gain_l, uint16_t gain_r);

private:
    Oscillator vco1_;
    Oscillator vco2_;
    float vco_mix_;

    Filter vcf_;

    float vca_gain_l_;
    float vca_gain_r_;

    float value_l_;
    float value_r_;
};
