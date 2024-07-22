#include "filter.hpp"
#include <cmath>

Filter::Filter()
    : cutoff_freq(Fixed_16_16::from_float(5000.0f)),
      resonance(Fixed_16_16::one)
{
    set_filter(Fixed_16_16::from_float(5000.0f), Fixed_16_16::one);
}

void Filter::set_filter(Fixed_16_16 cutoff_freq, Fixed_16_16 resonance)
{
    b_0_a_0 = Fixed_16_16::from_float(0.022186415f);
    b_1_a_0 = Fixed_16_16::from_float(0.044372831f);
    b_2_a_0 = Fixed_16_16::from_float(0.022186415f);
    a_1_a_0 = Fixed_16_16::from_float(-1.724481653f);
    a_2_a_0 = Fixed_16_16::from_float(0.813227315f);
}

void Filter::set_filter(float cutoff_freq, float resonance)
{
    float omega = 2.0f * M_PI * cutoff_freq / 40000.0f;
    float alpha = sinf(omega) / (2.0f * resonance);
    float a_0 = 1.0f + alpha;
    float a_1 = -2.0f * cosf(omega);
    float a_2 = 1.0f - alpha;
    float b_0 = (1.0f - cosf(omega)) / 2.0f;
    float b_1 = 1.0f - cosf(omega);
    float b_2 = (1.0f - cosf(omega)) / 2.0f;
    b_0_a_0 = Fixed_16_16::from_float(b_0 / a_0);
    b_1_a_0 = Fixed_16_16::from_float(b_1 / a_0);
    b_2_a_0 = Fixed_16_16::from_float(b_2 / a_0);
    a_1_a_0 = Fixed_16_16::from_float(a_1 / a_0);
    a_2_a_0 = Fixed_16_16::from_float(a_2 / a_0);
}

Fixed_16_16 Filter::get_value(Fixed_16_16 x)
{
    Fixed_16_16 y = b_0_a_0 * x + b_1_a_0 * x_1 + b_2_a_0 * x_2 - a_1_a_0 * y_1 - a_2_a_0 * y_2;
    if (y > Fixed_16_16::one) y = Fixed_16_16::one;
    if (y < -Fixed_16_16::one) y = -Fixed_16_16::one;
    x_2 = x_1;
    x_1 = x;
    y_2 = y_1;
    y_1 = y;
    return y;
}