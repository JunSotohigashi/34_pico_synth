#pragma once

#include <array>
#include "pico/stdlib.h"

enum class FilterType : uint8_t
{
    LPF = 0,
    HPF = 1
};

class Filter
{
public:
    Filter();

    void set_filter_type(bool is_hpf);
    void set_coefficients(float b0, float b1, float b2, float a1, float a2);
    void set_coefficient_by_index(uint8_t index, float value);

    float process(float x);

private:
    FilterType filter_type_;
    std::array<float, 5> coeff_; // b0, b1, b2, a1, a2
    float x1_;
    float x2_;
    float y1_;
    float y2_;
};
