#pragma once

#include <array>
#include "pico/stdlib.h"
#include "fixed_point.hpp"

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
    void set_coefficients(Fixed_16_16 b0, Fixed_16_16 b1, Fixed_16_16 b2, Fixed_16_16 a1, Fixed_16_16 a2);
    void set_coefficient_by_index(uint8_t index, Fixed_16_16 value);

    Fixed_16_16 process(Fixed_16_16 x);

private:
    FilterType filter_type_;
    std::array<Fixed_16_16, 5> coeff_; // b0, b1, b2, a1, a2
    Fixed_16_16 x1_;
    Fixed_16_16 x2_;
    Fixed_16_16 y1_;
    Fixed_16_16 y2_;
};
