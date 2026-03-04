#include "sound_module_manager.hpp"

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
    // Send 12 params for a single unit
    // Format per unit:
    // [0]: Destination Unit (0-15)
    // [1]: VCO Freq (0-65535 Hz)
    // [2]: VCO1 WaveType (0-3)
    // [3]: VCO2 WaveType (0-3)
    // [4]: VCO1 Duty (0-65535)
    // [5]: VCO2 Offset (-32768~32767 Hz)
    // [6]: VCO Mix (0-65535)
    // [7]: VCF Type (0-1)
    // [8]: VCF Cutoff (0-65535 Hz)
    // [9]: VCF Resonance (0-65535)
    // [10]: VCA Gain Left (0-65535 = 0.0-1.0)
    // [11]: VCA Gain Right (0-65535 = 0.0-1.0)

    // [0] Destination Unit
    buf[0] = unit_id;

    // [1] VCO Frequency from voice note
    uint8_t note = voice_allocator_->get_unit_note(unit_id);
    // Simplified: use note directly; Sound module will convert to frequency
    buf[1] = note & 0xFFFF;

    // [2-3] VCO WaveTypes
    buf[2] = static_cast<uint16_t>(panel_manager_->get_vco1_wavetype());
    buf[3] = static_cast<uint16_t>(panel_manager_->get_vco2_wavetype());

    // [4] VCO1 Duty (0-65535)
    buf[4] = panel_manager_->get_vco1_duty();

    // [5] VCO2 Offset (-32768~32767)
    int16_t vco2_offset = static_cast<int16_t>(panel_manager_->get_vco2_offset());
    buf[5] = static_cast<uint16_t>(vco2_offset);

    // [6] VCO Mix (0-65535)
    buf[6] = panel_manager_->get_vco_mix();

    // [7] VCF Type (0=LPF, 1=HPF)
    buf[7] = 0; // TODO: get from panel_manager if VCF type selector added

    // [8] VCF Cutoff (0-65535 Hz)
    buf[8] = panel_manager_->get_vcf_cutoff();

    // [9] VCF Resonance (0-65535)
    buf[9] = panel_manager_->get_vcf_resonance();

    // [10-11] VCA Gains (0-65535 = 0.0-1.0 for each channel)
    buf[10] = 65535; // Left gain (full volume)
    buf[11] = 65535; // Right gain (full volume)
}
