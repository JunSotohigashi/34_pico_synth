/**
 * \file filter.hpp
 * \author JunSotohigashi (c6h4ohcooc6h5@gmail.com)
 * \brief Digital Filter Class for Pico-Synth project
 * \version 0.1
 * \date 2024-07-20
 *
 *
 */

#ifndef FILTER_HPP
#define FILTER_HPP

#include "pico/stdlib.h"
#include "fixed_point.hpp"

/**
 * \brief Digital BiQuad Filter
 *
 */
class Filter
{
public:
    Filter();
    void set_filter(Fixed_16_16 cutoff_freq, Fixed_16_16 resonance);
    Fixed_16_16 get_value(Fixed_16_16 x);

private:
    Fixed_16_16 cutoff_freq;
    Fixed_16_16 resonance;
    Fixed_16_16 omega;
    Fixed_16_16 alpha;
    Fixed_16_16 a_0;
    Fixed_16_16 a_1;
    Fixed_16_16 a_2;
    Fixed_16_16 b_0;
    Fixed_16_16 b_1;
    Fixed_16_16 b_2;
    Fixed_16_16 b_0_a_0;
    Fixed_16_16 b_1_a_0;
    Fixed_16_16 b_2_a_0;
    Fixed_16_16 a_1_a_0;
    Fixed_16_16 a_2_a_0;
    Fixed_16_16 x_1;
    Fixed_16_16 x_2;
    Fixed_16_16 y_1;
    Fixed_16_16 y_2;
};

#endif