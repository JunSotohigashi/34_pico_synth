/**
 * \file envelope_generator.hpp
 * \author JunSotohigashi (c6h4ohcooc6h5@gmail.com)
 * \brief
 * \version 0.1
 * \date 2024-07-20
 *
 *
 */

#ifndef ENVELOPE_GENERATOR_HPP
#define ENVELOPE_GENERATOR_HPP

#include "pico/stdlib.h"
#include "include/fpm/fixed.hpp"

/**
 * \brief Envelope Generator state expression
 *
 */
enum class EGState
{
    Ready,
    Attack,
    Decay,
    Sustain,
    Release
};

/**
 * \brief Envelope Generator Class
 *
 */
class EG
{
public:
    EG();

    /**
     * \brief Get the gain value in Q12
     *
     * \return fpm::fixed_16_16
     */
    fpm::fixed_16_16 get_value();

    /**
     * \brief Trigger the gate on, set \a state to Attack
     *
     */
    void gate_on();

    /**
     * \brief Trigger the gate off, set \a state to Release
     *
     */
    void gate_off();

private:
    fpm::fixed_16_16 value{0}; // Current gain
    EGState state;    // Current EG state
    fpm::fixed_16_16 attack{0.1};  // Attack time
    fpm::fixed_16_16 decay{0.1};   // Decay time
    fpm::fixed_16_16 sustain{0.4}; // Sustin level
    fpm::fixed_16_16 release{0.1}; // Release time
    fpm::fixed_16_16 tau{0.01};     // Time constant for rate of change
};

#endif