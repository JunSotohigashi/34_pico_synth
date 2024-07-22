#include "filter.hpp"

Filter::Filter()
    : cutoff_freq(Fixed_16_16::from_float(5000.0f)),
      resonance(Fixed_16_16::one)
{
    set_filter(Fixed_16_16::from_float(5000.0f), Fixed_16_16::one);
}

void Filter::set_filter(Fixed_16_16 cutoff_freq, Fixed_16_16 resonance)
{
    // omega = 2 * pi * cuttoff_freq / sampling_rate
    // omega = Fixed_16_16::from_float(10.29437081f) * cutoff_freq;

    omega = Fixed_16_16::from_float(0.628318531f);
    alpha = Fixed_16_16::from_float(0.293892626f);
    a_0 = Fixed_16_16::from_float(1.293892626f);
    a_1 = Fixed_16_16::from_float(-1.618033989f);
    a_2 = Fixed_16_16::from_float(0.706107374f);
    b_0 = Fixed_16_16::from_float(0.095491503f);
    b_1 = Fixed_16_16::from_float(0.190983006f);
    b_2 = Fixed_16_16::from_float(0.095491503f);
    b_0_a_0 = b_0 / a_0;
    b_1_a_0 = b_1 / a_0;
    b_2_a_0 = b_2 / a_0;
    a_1_a_0 = a_1 / a_0;
    a_2_a_0 = a_2 / a_0;
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