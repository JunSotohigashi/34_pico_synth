#include "filter.hpp"
#include "table_wave.h"

Filter::Filter()
{
    set_filter(16384, Fixed_16_16::one);
}

// void Filter::set_filter(Fixed_16_16 cutoff_freq, Fixed_16_16 resonance)
// {
//     b_0_a_0 = Fixed_16_16::from_float(0.022186415f);
//     b_1_a_0 = Fixed_16_16::from_float(0.044372831f);
//     b_2_a_0 = Fixed_16_16::from_float(0.022186415f);
//     a_1_a_0 = Fixed_16_16::from_float(-1.724481653f);
//     a_2_a_0 = Fixed_16_16::from_float(0.813227315f);
// }

// void Filter::set_filter(float cutoff_freq, float resonance)
// {
//     float omega = 2.0f * M_PI * cutoff_freq / 40000.0f;
//     float alpha = sinf(omega) / (2.0f * resonance);
//     float a_0 = 1.0f + alpha;
//     float a_1 = -2.0f * cosf(omega);
//     float a_2 = 1.0f - alpha;
//     float b_0 = (1.0f - cosf(omega)) / 2.0f;
//     float b_1 = 1.0f - cosf(omega);
//     float b_2 = (1.0f - cosf(omega)) / 2.0f;
//     b_0_a_0 = Fixed_16_16::from_float(b_0 / a_0);
//     b_1_a_0 = Fixed_16_16::from_float(b_1 / a_0);
//     b_2_a_0 = Fixed_16_16::from_float(b_2 / a_0);
//     a_1_a_0 = Fixed_16_16::from_float(a_1 / a_0);
//     a_2_a_0 = Fixed_16_16::from_float(a_2 / a_0);
// }

void Filter::set_filter(uint16_t cutoff, Fixed_16_16 resonance)
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