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
#include "fixed_point.hpp"

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
     * \return Fixed_16_16
     */
    Fixed_16_16 get_value();

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

    void set_adsr(Fixed_16_16 attack, Fixed_16_16 decay, Fixed_16_16 sustain, Fixed_16_16 release);

private:
    Fixed_16_16 value;   // Current gain
    EGState state;       // Current EG state
    EGState state_old;   // Previous EG state
    Fixed_16_16 attack;  // Attack time
    Fixed_16_16 decay;   // Decay time
    Fixed_16_16 sustain; // Sustin level
    Fixed_16_16 release; // Release time
    Fixed_16_16 tau;
    uint16_t cycle;
};

#endif