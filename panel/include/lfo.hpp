#pragma once

#include "types.hpp"

class LFO
{
private:
    LFOWaveType wavetype_;
    LFOTarget target_;
    float freq_;
    float depth_;
    float time_;

public:
    LFO();
    void set_wavetype(LFOWaveType wt) { wavetype_ = wt; }
    void set_target(LFOTarget target) { target_ = target; }
    void set_freq(float freq) { freq_ = freq; }
    void set_depth(float depth) { depth_ = depth; }
    void update();
    float get_value(LFOTarget target) const;
};
