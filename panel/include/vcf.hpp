#pragma once

#include "types.hpp"

class VCF
{
private:
    VCFFilterType filter_;
    float cutoff_freq_;
    float resonance_;

public:
    VCF();
    void set_filter(VCFFilterType filter) { filter_ = filter; }
    void set_cutoff_freq(float cutoff) { cutoff_freq_ = cutoff; }
    void set_resonance(float resonance) { resonance_ = resonance; }
    void serialize(uint16_t *buf) const;
};
