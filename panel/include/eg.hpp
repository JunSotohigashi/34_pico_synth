#pragma once

#include "types.hpp"

class EG
{
private:
    float value_;
    EGState state_;
    float attack_;
    float decay_;
    float sustain_;
    float release_;
    float eg_int_;

public:
    EG();
    void set_adsr(float a, float d, float s, float r, float eg_int);
    void gate_on();
    void gate_off();
    void update();
    float get_value() const { return value_ * eg_int_; }
    EGState get_state() const { return state_; }
};
