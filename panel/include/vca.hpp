#pragma once

#include "types.hpp"

class VCA
{
private:
    float gain_;
    float pan_;

public:
    VCA();
    void set_gain(float gain) { gain_ = gain; }
    void set_pan(float pan) { pan_ = pan; }
    float get_gain() const { return gain_; }
    float get_pan() const { return pan_; }
    void serialize(uint16_t *buf) const;
};
