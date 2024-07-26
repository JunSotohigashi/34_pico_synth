/**
 * \file voice.hpp
 * \author JunSotohigashi (c6h4ohcooc6h5@gmail.com)
 * \brief Synthesis module for Pico-Synth project
 * \version 0.1
 * \date 2024-07-20
 *
 *
 */

#ifndef VOICE_HPP
#define VOICE_HPP

#include "pico/stdlib.h"
#include "fixed_point.hpp"
#include "oscillator.hpp"
#include "envelope_generator.hpp"
#include "filter.hpp"

// VCO,VCF,VCA,EGなどを内蔵した単一発音モジュール

/**
 * \brief a synthesis unit, contains 2 VCOs, VCF, VCA, EG for VCF and EG for VCA
 *
 */
class Voice
{
public:
    Voice();

    /**
     * \brief Get current value
     *
     * \return Fixed_16_16 output
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

    /**
     * \brief Set the waveform type for vco1
     *
     * \param wave_type
     */
    void set_vco1_wave_type(WaveType wave_type);

    /**
     * \brief Set the waveform type for vco2
     *
     * \param wave_type
     */
    void set_vco2_wave_type(WaveType wave_type);

    /**
     * \brief Set the frequency for VCO 1 & 2
     *
     * \param freq
     */
    void set_vco_freq(float freq);

    /**
     * \brief Set the frequency for VCO 1 & 2, using a MIDI note number
     *
     * \param note MIDI note number, 60 for C4
     */
    void set_vco_freq_note_number(uint8_t note);

    void set_vco2_tune(float tune);

    /**
     * \brief Set the duty ratio for Square wave
     *
     * \param duty 0 - 65535
     *
     * \note Works for Square wave only
     */
    void set_vco_duty(uint16_t duty);

    void set_vco_mix(uint16_t mix);

    void set_vcf_freq_res(bool is_hpf, uint16_t cutoff, Fixed_16_16 resonance);

    void set_vca_eg(uint16_t attack, uint16_t decay, uint16_t sustain, uint16_t release);

    void set_vca_gain(uint16_t gain);

private:
    Oscillator vco1;
    Oscillator vco2;
    EG vca_eg;
    Filter vcf;
    float vco_freq;
    float vco2_tune;
    Fixed_16_16 vco_mix;
    Fixed_16_16 vca_gain;
};

#endif