#include "filter.hpp"
#include "table_wave.h"

Filter::Filter()
    : filter_type(FilterType::LPF)
{
    set_filter(16384, Fixed_16_16::one);
}

void Filter::set_filter(uint16_t cutoff, Fixed_16_16 resonance)
{
    if (filter_type == FilterType::LPF)
    {
        uint16_t omega = cutoff >> 1;
        Fixed_16_16 two = Fixed_16_16::one + Fixed_16_16::one;
        Fixed_16_16 sin_omega = get_wave(wave_sine, omega);
        Fixed_16_16 cos_omega = get_wave(wave_sine, omega + (uint16_t)16384);
        Fixed_16_16 alpha = sin_omega / (two * resonance);
        Fixed_16_16 a0 = Fixed_16_16::one + alpha;
        Fixed_16_16 a1 = -two * cos_omega;
        Fixed_16_16 a2 = Fixed_16_16::one - alpha;
        Fixed_16_16 b1 = Fixed_16_16::one - cos_omega;
        Fixed_16_16 b0 = b1 / two;
        Fixed_16_16 b2 = b0;
        b0_a0 = b0 / a0;
        b1_a0 = b1 / a0;
        b2_a0 = b2 / a0;
        a1_a0 = a1 / a0;
        a2_a0 = a2 / a0;
    }
    else if (filter_type == FilterType::HPF)
    {
        uint16_t omega = cutoff >> 1;
        Fixed_16_16 two = Fixed_16_16::one + Fixed_16_16::one;
        Fixed_16_16 sin_omega = get_wave(wave_sine, omega);
        Fixed_16_16 cos_omega = get_wave(wave_sine, omega + (uint16_t)16384);
        Fixed_16_16 alpha = sin_omega / (two * resonance);
        Fixed_16_16 a0 = Fixed_16_16::one + alpha;
        Fixed_16_16 a1 = -two * cos_omega;
        Fixed_16_16 a2 = Fixed_16_16::one - alpha;
        Fixed_16_16 b1 = Fixed_16_16::one + cos_omega;
        Fixed_16_16 b0 = b1 / two;
        Fixed_16_16 b2 = b0;
        b0_a0 = b0 / a0;
        b1_a0 = -b1 / a0;
        b2_a0 = b2 / a0;
        a1_a0 = a1 / a0;
        a2_a0 = a2 / a0;
    }
}

void Filter::set_filter_type(bool is_hpf)
{
    if (is_hpf)
        filter_type = FilterType::HPF;
    else
        filter_type = FilterType::LPF;
}

Fixed_16_16 Filter::get_value(Fixed_16_16 x)
{
    Fixed_16_16 y = b0_a0 * x + b1_a0 * x1 + b2_a0 * x2 - a1_a0 * y1 - a2_a0 * y2;
    if (y > Fixed_16_16::one)
        y = Fixed_16_16::one;
    if (y < -Fixed_16_16::one)
        y = -Fixed_16_16::one;
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = y;
    return y;
}