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

enum class LFOTarget
{
    VCO2,
    VCO12,
    Duty,
    Cutoff,
    Gain,
    Pan
};

enum class LFOType
{
    Saw_up,
    Saw_down,
    Triangle,
    Sine,
    Square,
    Random
};

/**
 * \brief a synthesis unit, contains 2 VCOs, VCF, VCA, EG for VCF and EG for VCA
 *
 */
class Voice
{
public:
    Voice();

    void update();

    Fixed_16_16 get_value_L();
    Fixed_16_16 get_value_R();

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

    void set_vcf_eg(uint16_t attack, uint16_t decay, uint16_t amount);

    void set_vca_eg(uint16_t attack, uint16_t decay, uint16_t sustain, uint16_t release);

    void set_vca_gain(uint16_t gain);

private:
    uint16_t cycle;
    Oscillator vco1;
    Oscillator vco2;
    float vco_freq;
    float vco2_tune;
    Fixed_16_16 vco_mix;
    Filter vcf;
    EG vcf_eg;
    Fixed_16_16 vcf_eg_amount;
    Fixed_16_16 vcf_eg_value;
    uint16_t vcf_cutoff;
    Fixed_16_16 vcf_resonance;
    EG vca_eg;
    Fixed_16_16 vca_eg_value;
    Fixed_16_16 vca_gain;

    Fixed_16_16 value_l;
    Fixed_16_16 value_r;
};

const float note_freq[128] = {8.175798916, 8.661957218, 9.177023997, 9.722718241, 10.30086115, 10.91338223, 11.56232571, 12.24985737, 12.9782718, 13.75, 14.56761755, 15.43385316, 16.35159783, 17.32391444, 18.35404799, 19.44543648, 20.60172231, 21.82676446, 23.12465142, 24.49971475, 25.9565436, 27.5, 29.13523509, 30.86770633, 32.70319566, 34.64782887, 36.70809599, 38.89087297, 41.20344461, 43.65352893, 46.24930284, 48.9994295, 51.9130872, 55, 58.27047019, 61.73541266, 65.40639133, 69.29565774, 73.41619198, 77.78174593, 82.40688923, 87.30705786, 92.49860568, 97.998859, 103.8261744, 110, 116.5409404, 123.4708253, 130.8127827, 138.5913155, 146.832384, 155.5634919, 164.8137785, 174.6141157, 184.9972114, 195.997718, 207.6523488, 220, 233.0818808, 246.9416506, 261.6255653, 277.182631, 293.6647679, 311.1269837, 329.6275569, 349.2282314, 369.9944227, 391.995436, 415.3046976, 440, 466.1637615, 493.8833013, 523.2511306, 554.365262, 587.3295358, 622.2539674, 659.2551138, 698.4564629, 739.9888454, 783.990872, 830.6093952, 880, 932.327523, 987.7666025, 1046.502261, 1108.730524, 1174.659072, 1244.507935, 1318.510228, 1396.912926, 1479.977691, 1567.981744, 1661.21879, 1760, 1864.655046, 1975.533205, 2093.004522, 2217.461048, 2349.318143, 2489.01587, 2637.020455, 2793.825851, 2959.955382, 3135.963488, 3322.437581, 3520, 3729.310092, 3951.06641, 4186.009045, 4434.922096, 4698.636287, 4978.03174, 5274.040911, 5587.651703, 5919.910763, 6271.926976, 6644.875161, 7040, 7458.620184, 7902.13282, 8372.01809, 8869.844191, 9397.272573, 9956.063479, 10548.08182, 11175.30341, 11839.82153, 12543.85395};

#endif