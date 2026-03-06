#include "voice.hpp"

namespace
{
    float clamp(float value, float low, float high)
    {
        if (value < low)
        {
            return low;
        }
        if (value > high)
        {
            return high;
        }
        return value;
    }
}

Voice::Voice()
    : vco_mix_(0.5f),
      vca_gain_l_(0.0f),
      vca_gain_r_(0.0f),
      value_l_(0.0f),
      value_r_(0.0f)
{
}

void Voice::update()
{
    const float value1 = vco1_.get_value() * (1.0f - vco_mix_ * vco_mix_);
    const float one_minus_mix = 1.0f - vco_mix_;
    const float value2 = vco2_.get_value() * (1.0f - one_minus_mix * one_minus_mix);

    const float filtered = vcf_.process(value1 + value2);
    value_l_ = clamp(filtered * vca_gain_l_, -1.0f, 1.0f);
    value_r_ = clamp(filtered * vca_gain_r_, -1.0f, 1.0f);
}

float Voice::get_value_l() const
{
    return value_l_;
}

float Voice::get_value_r() const
{
    return value_r_;
}

void Voice::set_vco1_wave_type(WaveType wave_type)
{
    vco1_.set_wave_type(wave_type);
}

void Voice::set_vco2_wave_type(WaveType wave_type)
{
    vco2_.set_wave_type(wave_type);
}

void Voice::set_vco1_phase_delta(uint16_t phase_delta)
{
    vco1_.set_phase16_delta(phase_delta);
}

void Voice::set_vco2_phase_delta(uint16_t phase_delta)
{
    vco2_.set_phase16_delta(phase_delta);
}

void Voice::set_vco_duty(uint16_t duty)
{
    vco1_.set_duty(duty);
    vco2_.set_duty(duty);
}

void Voice::set_vco_mix(uint16_t mix)
{
    vco_mix_ = static_cast<float>(mix) / 65535.0f;
}

void Voice::set_vcf_type(bool is_hpf)
{
    vcf_.set_filter_type(is_hpf);
}

void Voice::set_vcf_coefficient_by_index(uint8_t index, float value)
{
    vcf_.set_coefficient_by_index(index, value);
}

void Voice::set_vca_gain_lr(uint16_t gain_l, uint16_t gain_r)
{
    vca_gain_l_ = static_cast<float>(gain_l) / 65535.0f;
    vca_gain_r_ = static_cast<float>(gain_r) / 65535.0f;
}
