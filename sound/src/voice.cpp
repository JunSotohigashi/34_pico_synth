#include "voice.hpp"

namespace
{
    Fixed_16_16 clamp(Fixed_16_16 value, Fixed_16_16 low, Fixed_16_16 high)
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
    : vco_mix_(Fixed_16_16::from_raw_value(32768)),
      vca_gain_l_(Fixed_16_16::zero),
      vca_gain_r_(Fixed_16_16::zero),
      value_l_(Fixed_16_16::zero),
      value_r_(Fixed_16_16::zero)
{
}

void Voice::update()
{
    Fixed_16_16 value1 = vco1_.get_value();
    Fixed_16_16 value2 = vco2_.get_value();
    
    Fixed_16_16 mix_sq = vco_mix_ * vco_mix_;
    Fixed_16_16 one_minus_mix = Fixed_16_16::one - vco_mix_;
    Fixed_16_16 one_minus_mix_sq = one_minus_mix * one_minus_mix;
    
    value1 = value1 * (Fixed_16_16::one - mix_sq);
    value2 = value2 * (Fixed_16_16::one - one_minus_mix_sq);

    Fixed_16_16 filtered = vcf_.process(value1 + value2);
    value_l_ = clamp(filtered * vca_gain_l_, Fixed_16_16::from_int32(-1), Fixed_16_16::one);
    value_r_ = clamp(filtered * vca_gain_r_, Fixed_16_16::from_int32(-1), Fixed_16_16::one);
}

Fixed_16_16 Voice::get_value_l() const
{
    return value_l_;
}

Fixed_16_16 Voice::get_value_r() const
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
    vco_mix_ = Fixed_16_16::from_raw_value(static_cast<int32_t>(mix));
}

void Voice::set_vcf_type(bool is_hpf)
{
    vcf_.set_filter_type(is_hpf);
}

void Voice::set_vcf_coefficient_by_index(uint8_t index, Fixed_16_16 value)
{
    vcf_.set_coefficient_by_index(index, value);
}

void Voice::set_vca_gain_lr(uint16_t gain_l, uint16_t gain_r)
{
    vca_gain_l_ = Fixed_16_16::from_raw_value(static_cast<int32_t>(gain_l));
    vca_gain_r_ = Fixed_16_16::from_raw_value(static_cast<int32_t>(gain_r));
}
