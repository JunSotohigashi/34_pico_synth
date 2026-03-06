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
    float get_cutoff_freq() const { return cutoff_freq_; }
    float get_resonance() const { return resonance_; }
    void serialize(uint16_t *buf) const;
};
