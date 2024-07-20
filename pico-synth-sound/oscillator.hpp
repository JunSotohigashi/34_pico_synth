/**
 * \file oscillator.hpp
 * \author JunSotohigashi (c6h4ohcooc6h5@gmail.com)
 * \brief Oscillator Class for Pico-Synth
 * \version 0.1
 * \date 2024-07-20
 * 
 * 
 */

#ifndef OSCILLATOR_HPP
#define OSCILLATOR_HPP

#include "pico/stdlib.h"

/**
 * \brief Waveform type expression
 * 
 */
enum class WaveType
{
    Saw,
    Sine,
    Triangle,
    Square
};

/**
 * \brief wave table based oscillator
 * 
 */
class Oscillator
{
public:
    Oscillator();
    /**
     * \brief Get current value
     * 
     * \return int16_t output range: -32767 ~ 32767
     */
    int16_t get_value();

    /**
     * \brief Set the wave type
     * 
     * \param wave_type waveform type that you want to change to
     */
    void set_wave_type(WaveType wave_type);

    /**
     * \brief Change frequancy
     * 
     * \param phase16_delta Increment value for each cycle
     * 
     * \note Larger phase16_delta, lower frequency
     */
    void set_phase16_delta(uint16_t phase16_delta);

    /**
     * \brief Set the duty ratio for Square wave
     * 
     * \param duty 0 - 65535
     * 
     * \note Works for Square wave only
     */
    void set_duty(uint16_t duty);

private:
    WaveType wave_type_now;     // Current waveform type
    WaveType wave_type_next;    // Next waveform type, used for noise-less waveform changing
    uint16_t phase16;           // Current phase, used for looking up wave table
    uint16_t phase16_delta;     // Increment of phase16 every cycle
    uint16_t duty;              // Duty ratio of Square wave
};

#endif