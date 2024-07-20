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
     * \return uint16_t gain(Q12), 0x10000 is 1.0
     */
    uint16_t get_value();

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
    uint32_t value28; // Current gain value in Q28
    EGState state;    // Current EG state
    uint16_t attack;  // Attack time, 0 - 65535
    uint16_t decay;   // Decay time, 0 - 65535
    uint16_t sustain; // Sustin level, 0 - 65535
    uint16_t release; // Release time, 0 - 65535
    uint16_t tau;     // Time constant for rate of change
};

#endif