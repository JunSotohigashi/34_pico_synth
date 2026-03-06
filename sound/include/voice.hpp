#pragma once

#include "pico/stdlib.h"
#include "fixed_point.hpp"

#include "oscillator.hpp"
#include "filter.hpp"

class Voice
{
public:
    Voice();

    void update();

    Fixed_16_16 get_value_l() const;
    Fixed_16_16 get_value_r() const;

    void set_vco1_wave_type(WaveType wave_type);
    void set_vco2_wave_type(WaveType wave_type);
    void set_vco1_phase_delta(uint16_t phase_delta);
    void set_vco2_phase_delta(uint16_t phase_delta);
    void set_vco_duty(uint16_t duty);
    void set_vco_mix(uint16_t mix);

    void set_vcf_type(bool is_hpf);
    void set_vcf_coefficient_by_index(uint8_t index, Fixed_16_16 value);

    void set_vca_gain_lr(uint16_t gain_l, uint16_t gain_r);

private:
    Oscillator vco1_;
    Oscillator vco2_;
    Fixed_16_16 vco_mix_;

    Filter vcf_;

    Fixed_16_16 vca_gain_l_;
    Fixed_16_16 vca_gain_r_;

    Fixed_16_16 value_l_;
    Fixed_16_16 value_r_;
};
