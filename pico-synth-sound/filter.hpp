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

enum class FilterType
{
    LPF,
    HPF
};


/**
 * \brief Digital BiQuad Filter
 *
 */
class Filter
{
public:
    Filter();
    // void set_filter(Fixed_16_16 cutoff_freq, Fixed_16_16 resonance);
    // void set_filter(float cutoff_freq, float resonance);
    void set_filter(uint16_t cutoff, Fixed_16_16 resonance);
    void set_filter_type(bool is_hpf);
    Fixed_16_16 get_value(Fixed_16_16 x);

private:
    FilterType filter_type;
    Fixed_16_16 b0_a0;
    Fixed_16_16 b1_a0;
    Fixed_16_16 b2_a0;
    Fixed_16_16 a1_a0;
    Fixed_16_16 a2_a0;
    Fixed_16_16 x1;
    Fixed_16_16 x2;
    Fixed_16_16 y1;
    Fixed_16_16 y2;
};

#endif