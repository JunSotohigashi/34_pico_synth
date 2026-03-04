#pragma once

#include "types.hpp"

class VCO
{
private:
    float freq_;
    VCOWaveType wavetype1_;
    VCOWaveType wavetype2_;
    float vco1_duty_;
    float vco2_offset_;
    float mix_;

public:
    VCO();
    void set_freq(float freq) { freq_ = freq; }
    void set_wavetype1(VCOWaveType wt) { wavetype1_ = wt; }
    void set_wavetype2(VCOWaveType wt) { wavetype2_ = wt; }
    void set_vco1_duty(float duty) { vco1_duty_ = duty; }
    void set_vco2_offset(float offset) { vco2_offset_ = offset; }
    void set_mix(float mix) { mix_ = mix; }
    void serialize(uint16_t *buf) const;
};
