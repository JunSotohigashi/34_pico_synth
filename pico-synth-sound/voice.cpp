#include "voice.hpp"
#include <stdio.h>

#define HEAVY_CYCLE 100

const float note_freq[128] = {8.175798916, 8.661957218, 9.177023997, 9.722718241, 10.30086115, 10.91338223, 11.56232571, 12.24985737, 12.9782718, 13.75, 14.56761755, 15.43385316, 16.35159783, 17.32391444, 18.35404799, 19.44543648, 20.60172231, 21.82676446, 23.12465142, 24.49971475, 25.9565436, 27.5, 29.13523509, 30.86770633, 32.70319566, 34.64782887, 36.70809599, 38.89087297, 41.20344461, 43.65352893, 46.24930284, 48.9994295, 51.9130872, 55, 58.27047019, 61.73541266, 65.40639133, 69.29565774, 73.41619198, 77.78174593, 82.40688923, 87.30705786, 92.49860568, 97.998859, 103.8261744, 110, 116.5409404, 123.4708253, 130.8127827, 138.5913155, 146.832384, 155.5634919, 164.8137785, 174.6141157, 184.9972114, 195.997718, 207.6523488, 220, 233.0818808, 246.9416506, 261.6255653, 277.182631, 293.6647679, 311.1269837, 329.6275569, 349.2282314, 369.9944227, 391.995436, 415.3046976, 440, 466.1637615, 493.8833013, 523.2511306, 554.365262, 587.3295358, 622.2539674, 659.2551138, 698.4564629, 739.9888454, 783.990872, 830.6093952, 880, 932.327523, 987.7666025, 1046.502261, 1108.730524, 1174.659072, 1244.507935, 1318.510228, 1396.912926, 1479.977691, 1567.981744, 1661.21879, 1760, 1864.655046, 1975.533205, 2093.004522, 2217.461048, 2349.318143, 2489.01587, 2637.020455, 2793.825851, 2959.955382, 3135.963488, 3322.437581, 3520, 3729.310092, 3951.06641, 4186.009045, 4434.922096, 4698.636287, 4978.03174, 5274.040911, 5587.651703, 5919.910763, 6271.926976, 6644.875161, 7040, 7458.620184, 7902.13282, 8372.01809, 8869.844191, 9397.272573, 9956.063479, 10548.08182, 11175.30341, 11839.82153, 12543.85395};

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
      vca_gain(Fixed_16_16::one)
{
}

Fixed_16_16 Voice::get_value()
{
    cycle++;
    if (cycle == HEAVY_CYCLE)
    {
        vca_eg_value = vca_eg.get_value();
        vcf_eg_value = vcf_eg.get_value() * vcf_eg_amount + Fixed_16_16::one;
        vcf.set_filter(static_cast<uint16_t>((Fixed_16_16::from_raw_value(vcf_cutoff) * vcf_eg_value).raw_value), vcf_resonance);
        cycle = 0;
    }

    Fixed_16_16 value1 = vco1.get_value() * (Fixed_16_16::one - vco_mix * vco_mix);                                           // y = 1-x^2
    Fixed_16_16 value2 = vco2.get_value() * (Fixed_16_16::one - (Fixed_16_16::one - vco_mix) * (Fixed_16_16::one - vco_mix)); // y = 1-(1-x)^2

    Fixed_16_16 y = vcf.get_value(value1 + value2) * vca_eg_value * vca_gain;
    if (y > Fixed_16_16::one)
        y = Fixed_16_16::one;
    if (y < -Fixed_16_16::one)
        y = -Fixed_16_16::one;
    return y;
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
    vco1.set_phase16_delta(vco_freq * 65536.0f / 40000.0f);
    vco2.set_phase16_delta((vco_freq * vco2_tune) * 65536.0f / 40000.0f);
}

void Voice::set_vco_freq_note_number(uint8_t note)
{
    vco_freq = note_freq[note & 0b1111111];
    vco1.set_phase16_delta(vco_freq * 65536.0f / 40000.0f);
    vco2.set_phase16_delta((vco_freq * vco2_tune) * 65536.0f / 40000.0f);
}

void Voice::set_vco2_tune(float tune)
{
    vco2_tune = tune;
    vco2.set_phase16_delta((vco_freq * vco2_tune) * 65536.0f / 40000.0f);
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
