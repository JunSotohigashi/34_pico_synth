#include "voice.hpp"
#include <stdio.h>

#define HEAVY_CYCLE 100


Voice::Voice()
    : cycle(0),
      vco1(Oscillator()),
      vco2(Oscillator()),
      vco2_tune(1.0),
      vco_mix(Fixed_16_16::from_float(0.5f)),
      vcf(Filter()),
      vcf_eg(EG()),
      vcf_eg_value(Fixed_16_16::zero),
      vcf_cutoff(16384),
      vcf_resonance(Fixed_16_16::one),
      vca_eg(EG()),
      vca_eg_value(Fixed_16_16::zero),
      vca_gain(Fixed_16_16::one),
      lfo(Oscillator()),
      lfo_taget(LFOTarget::VCO2),
      lfo_freq(0.0),
      lfo_depth(0.0),
      lfo_gain(Fixed_16_16::one),
      lfo_pan(Fixed_16_16::from_float(0.5f)),
      value_l(Fixed_16_16::zero),
      value_r(Fixed_16_16::zero)
{
}

void Voice::update()
{
    cycle++;
    if (cycle == HEAVY_CYCLE)
    {
        vca_eg_value = vca_eg.get_value();
        vcf_eg_value = vcf_eg.get_value() * vcf_eg_amount + Fixed_16_16::one;
        vcf.set_filter(static_cast<uint16_t>((Fixed_16_16::from_raw_value(vcf_cutoff) * vcf_eg_value).raw_value), vcf_resonance);

        if (lfo_taget == LFOTarget::Gain)
        {
            lfo_gain = ((lfo.get_value() - Fixed_16_16::one) * Fixed_16_16::from_raw_value(lfo_depth)) * Fixed_16_16::from_float(0.5f) + Fixed_16_16::one;
        }
        else
        {
            lfo_gain = Fixed_16_16::one;
        }

        if (lfo_taget == LFOTarget::Pan)
        {
            lfo_pan = ((lfo.get_value() * Fixed_16_16::from_raw_value(lfo_depth)) + Fixed_16_16::one) * Fixed_16_16::from_float(0.5f);
        }
        else
        {
            lfo_pan = Fixed_16_16::from_float(0.5f);
        }

        cycle = 0;
    }

    Fixed_16_16 value1 = vco1.get_value() * (Fixed_16_16::one - vco_mix * vco_mix);                                           // y = 1-x^2
    Fixed_16_16 value2 = vco2.get_value() * (Fixed_16_16::one - (Fixed_16_16::one - vco_mix) * (Fixed_16_16::one - vco_mix)); // y = 1-(1-x)^2

    Fixed_16_16 y = vcf.get_value(value1 + value2) * vca_eg_value * vca_gain;

    value_l = y * lfo_pan * lfo_gain;
    value_r = y * (Fixed_16_16::one - lfo_pan) * lfo_gain;

    if (value_l > Fixed_16_16::one)
        value_l = Fixed_16_16::one;
    if (value_l < -Fixed_16_16::one)
        value_l = -Fixed_16_16::one;

    if (value_r > Fixed_16_16::one)
        value_r = Fixed_16_16::one;
    if (value_r < -Fixed_16_16::one)
        value_r = -Fixed_16_16::one;
}

Fixed_16_16 Voice::get_value_L()
{
    return value_l;
}

Fixed_16_16 Voice::get_value_R()
{
    return value_r;
}

void Voice::gate_on()
{
    vca_eg.gate_on();
    vcf_eg.gate_on();
}

void Voice::gate_off()
{
    vca_eg.gate_off();
    vcf_eg.gate_off();
}

void Voice::set_vco1_wave_type(WaveType wave_type)
{
    vco1.set_wave_type(wave_type);
}

void Voice::set_vco2_wave_type(WaveType wave_type)
{
    vco2.set_wave_type(wave_type);
}

void Voice::set_vco_freq(float freq)
{
    vco_freq = freq;
    vco1.set_phase16_delta(static_cast<uint16_t>(vco_freq * 65536.0f / 40000.0f));
    vco2.set_phase16_delta(static_cast<uint16_t>((vco_freq * vco2_tune) * 65536.0f / 40000.0f));
}

void Voice::set_vco_freq_note_number(uint8_t note)
{
    set_vco_freq(note_freq[note & 0b1111111]);
}

void Voice::set_vco2_tune(float tune)
{
    vco2_tune = tune;
    vco2.set_phase16_delta(static_cast<uint16_t>((vco_freq * vco2_tune) * 65536.0f / 40000.0f));
}

void Voice::set_vco_duty(uint16_t duty)
{
    vco1.set_duty(duty);
    vco2.set_duty(duty);
}

void Voice::set_vco_mix(uint16_t mix)
{
    vco_mix = Fixed_16_16::from_raw_value(mix);
}

void Voice::set_vcf_freq_res(bool is_hpf, uint16_t cutoff, Fixed_16_16 resonance)
{
    vcf.set_filter_type(is_hpf);
    vcf_cutoff = cutoff;
    vcf_resonance = resonance;
}

void Voice::set_vcf_eg(uint16_t attack, uint16_t decay, uint16_t amount)
{
    Fixed_16_16 a = Fixed_16_16::from_raw_value(attack);
    Fixed_16_16 d = Fixed_16_16::from_raw_value(decay);
    vcf_eg_amount = Fixed_16_16::from_raw_value(amount << 1);
    vcf_eg.set_adsr(a * a, d * d, Fixed_16_16::zero, Fixed_16_16::zero);
}

void Voice::set_vca_eg(uint16_t attack, uint16_t decay, uint16_t sustain, uint16_t release)
{
    Fixed_16_16 a = Fixed_16_16::from_raw_value(attack);
    Fixed_16_16 d = Fixed_16_16::from_raw_value(decay);
    Fixed_16_16 s = Fixed_16_16::from_raw_value(sustain);
    Fixed_16_16 r = Fixed_16_16::from_raw_value(release);
    vca_eg.set_adsr(a * a, d * d, s, r * r);
}

void Voice::set_vca_gain(uint16_t gain)
{
    vca_gain = Fixed_16_16::from_raw_value(gain);
}

void Voice::set_lfo(LFOTarget target, WaveType type, float freq, uint16_t depth)
{
    lfo_taget = target;
    lfo_freq = freq;
    lfo_depth = depth;
    lfo.set_phase16_delta(lfo_freq * 65536.0f / 400.0f);
    lfo.set_wave_type(type);
}
