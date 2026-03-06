#include "sound_module_manager.hpp"

#include <cmath>
#include <cstring>

namespace
{
constexpr float kSampleRate = 40000.0f;
constexpr float kMinCutoffHz = 20.0f;
constexpr float kMaxCutoffHz = 18000.0f;
constexpr float kMinQ = 0.70710678f;
constexpr float kMaxQ = 4.0f;
constexpr float kTwoPi = 6.28318530718f;

inline float clampf(float v, float lo, float hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

inline uint16_t phase_delta_from_hz(float hz)
{
    const float value = hz * 65536.0f / kSampleRate;
    return static_cast<uint16_t>(clampf(value, 0.0f, 65535.0f));
}

inline void encode_float(float value, uint16_t &hi, uint16_t &lo)
{
    uint32_t raw = 0;
    static_assert(sizeof(float) == sizeof(uint32_t), "Unexpected float size");
    std::memcpy(&raw, &value, sizeof(raw));
    hi = static_cast<uint16_t>(raw >> 16);
    lo = static_cast<uint16_t>(raw & 0xFFFF);
}

void calc_biquad(bool is_hpf, float cutoff_hz, float q, float &b0, float &b1, float &b2, float &a1, float &a2)
{
    const float omega = kTwoPi * cutoff_hz / kSampleRate;
    const float sn = std::sinf(omega);
    const float cs = std::cosf(omega);
    const float alpha = sn / (2.0f * q);

    float tb0 = 0.0f;
    float tb1 = 0.0f;
    float tb2 = 0.0f;
    const float a0 = 1.0f + alpha;
    const float ta1 = -2.0f * cs;
    const float ta2 = 1.0f - alpha;

    if (is_hpf)
    {
        tb0 = (1.0f + cs) * 0.5f;
        tb1 = -(1.0f + cs);
        tb2 = (1.0f + cs) * 0.5f;
    }
    else
    {
        tb0 = (1.0f - cs) * 0.5f;
        tb1 = 1.0f - cs;
        tb2 = (1.0f - cs) * 0.5f;
    }

    b0 = tb0 / a0;
    b1 = tb1 / a0;
    b2 = tb2 / a0;
    a1 = ta1 / a0;
    a2 = ta2 / a0;
}
} // namespace

SoundModuleManager::SoundModuleManager(VoiceAllocator *va, PanelManager *pm)
    : voice_allocator_(va), panel_manager_(pm)
{
    for (uint8_t i = 0; i < 16; i++)
    {
        sound_units_[i] = new SoundUnit();
    }
}

SoundModuleManager::~SoundModuleManager()
{
    for (uint8_t i = 0; i < 16; i++)
    {
        delete sound_units_[i];
    }
}

void SoundModuleManager::update()
{
    panel_manager_->update();
    panel_manager_->set_sound_unit_status(voice_allocator_->get_unit_state_bitmap());

    for (uint8_t i = 0; i < 16; i++)
    {
        sound_units_[i]->update();

        sound_units_[i]->get_vco()->set_wavetype1(panel_manager_->get_vco1_wavetype());
        sound_units_[i]->get_vco()->set_wavetype2(panel_manager_->get_vco2_wavetype());
        sound_units_[i]->get_vco()->set_vco1_duty(panel_manager_->get_vco1_duty() / 1023.0f);
        sound_units_[i]->get_vco()->set_vco2_offset(panel_manager_->get_vco2_offset() / 1023.0f);
        sound_units_[i]->get_vco()->set_mix(panel_manager_->get_vco_mix() / 1023.0f);

        sound_units_[i]->get_vcf()->set_cutoff_freq(panel_manager_->get_vcf_cutoff());
        sound_units_[i]->get_vcf()->set_resonance(panel_manager_->get_vcf_resonance() / 1023.0f);

        float vcf_a = panel_manager_->get_vcf_attack() / 1023.0f;
        float vcf_d = panel_manager_->get_vcf_decay() / 1023.0f;
        float vcf_s = panel_manager_->get_vcf_sustain() / 1023.0f;
        float vcf_r = 0.0f;
        float vcf_int = panel_manager_->get_vcf_eg_int() / 1023.0f;
        sound_units_[i]->get_vcf_eg()->set_adsr(vcf_a, vcf_d, vcf_s, vcf_r, vcf_int);

        float vca_a = panel_manager_->get_vca_attack() / 1023.0f;
        float vca_d = panel_manager_->get_vca_decay() / 1023.0f;
        float vca_s = panel_manager_->get_vca_sustain() / 1023.0f;
        float vca_r = panel_manager_->get_vca_release() / 1023.0f;
        sound_units_[i]->get_vca_eg()->set_adsr(vca_a, vca_d, vca_s, vca_r, 1.0f);

        sound_units_[i]->get_lfo()->set_wavetype(panel_manager_->get_lfo_wavetype());
        sound_units_[i]->get_lfo()->set_target(panel_manager_->get_lfo_target());
        sound_units_[i]->get_lfo()->set_freq(panel_manager_->get_lfo_freq() / 1023.0f * 10.0f);
        sound_units_[i]->get_lfo()->set_depth(panel_manager_->get_lfo_depth() / 1023.0f);
    }

    voice_allocator_->update(sound_units_);
}

void SoundModuleManager::serialize(uint8_t unit_id, uint16_t *buf)
{
    buf[IDX_UNIT_ID] = unit_id;

    const bool active = (voice_allocator_->get_unit_state(unit_id) != SoundUnitState::IDLE);
    const uint8_t note = voice_allocator_->get_unit_note(unit_id);
    float base_freq = 440.0f * std::pow(2.0f, (static_cast<int>(note) - 69) / 12.0f);
    if (!active)
    {
        base_freq = 0.0f;
    }

    const float tune_pos = (static_cast<float>(panel_manager_->get_vco2_offset()) - 512.0f) / 512.0f;
    const float vco2_ratio = std::pow(2.0f, std::pow(tune_pos, 3.0f));

    buf[IDX_VCO1_PHASE_DELTA] = phase_delta_from_hz(base_freq);
    buf[IDX_VCO2_PHASE_DELTA] = phase_delta_from_hz(base_freq * vco2_ratio);
    buf[IDX_VCO1_WAVE] = static_cast<uint16_t>(panel_manager_->get_vco1_wavetype());
    buf[IDX_VCO2_WAVE] = static_cast<uint16_t>(panel_manager_->get_vco2_wavetype());
    buf[IDX_VCO1_DUTY] = static_cast<uint16_t>(panel_manager_->get_vco1_duty() << 6);
    buf[IDX_VCO_MIX] = static_cast<uint16_t>(panel_manager_->get_vco_mix() << 6);

    const bool is_hpf = panel_manager_->get_vcf_type() == VCFFilterType::HPF;
    buf[IDX_VCF_TYPE] = is_hpf ? 1 : 0;

    const float cutoff_norm = panel_manager_->get_vcf_cutoff() / 1023.0f;
    float cutoff_hz = kMinCutoffHz * std::pow(kMaxCutoffHz / kMinCutoffHz, cutoff_norm);
    cutoff_hz = clampf(cutoff_hz, kMinCutoffHz, kMaxCutoffHz);

    const float q_norm = panel_manager_->get_vcf_resonance() / 1023.0f;
    const float q = kMinQ + q_norm * (kMaxQ - kMinQ);

    float b0 = 0.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
    calc_biquad(is_hpf, cutoff_hz, q, b0, b1, b2, a1, a2);

    encode_float(b0, buf[IDX_VCF_B0_HI], buf[IDX_VCF_B0_LO]);
    encode_float(b1, buf[IDX_VCF_B1_HI], buf[IDX_VCF_B1_LO]);
    encode_float(b2, buf[IDX_VCF_B2_HI], buf[IDX_VCF_B2_LO]);
    encode_float(a1, buf[IDX_VCF_A1_HI], buf[IDX_VCF_A1_LO]);
    encode_float(a2, buf[IDX_VCF_A2_HI], buf[IDX_VCF_A2_LO]);

    buf[IDX_VCA_GAIN_L] = active ? 65535 : 0;
    buf[IDX_VCA_GAIN_R] = active ? 65535 : 0;
}
