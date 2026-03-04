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

void SoundModuleManager::serialize(uint16_t *buf)
{
    buf[0] = 0xFFFF;

    for (uint8_t i = 0; i < 16; i++)
    {
        uint8_t note = voice_allocator_->get_unit_note(i);
        uint8_t velocity = voice_allocator_->get_unit_velocity(i);
        bool is_active = (voice_allocator_->get_unit_state(i) != SoundUnitState::IDLE);
        buf[i + 1] = (is_active ? 1 : 0) << 15 | (note << 8) | velocity;
    }

    uint8_t vco1_mode = static_cast<uint8_t>(panel_manager_->get_vco1_wavetype());
    uint8_t vco2_mode = static_cast<uint8_t>(panel_manager_->get_vco2_wavetype());
    uint8_t vcf_mode = 0;
    uint8_t lfo_wave = static_cast<uint8_t>(panel_manager_->get_lfo_wavetype());
    uint8_t lfo_target = static_cast<uint8_t>(panel_manager_->get_lfo_target());
    buf[17] = (vco1_mode << 10) | (vco2_mode << 8) | (vcf_mode << 7) | (lfo_wave << 3) | lfo_target;

    buf[18] = panel_manager_->get_vco1_duty();
    buf[19] = panel_manager_->get_vco2_offset();
    buf[20] = panel_manager_->get_vco_mix();
    buf[21] = panel_manager_->get_vcf_cutoff();
    buf[22] = panel_manager_->get_vcf_resonance();
    buf[23] = panel_manager_->get_vcf_attack();
    buf[24] = panel_manager_->get_vcf_decay();
    buf[25] = panel_manager_->get_vcf_sustain();
    buf[26] = 0;
    buf[27] = panel_manager_->get_vca_attack();
    buf[28] = panel_manager_->get_vca_decay();
    buf[29] = panel_manager_->get_vca_sustain();
    buf[30] = panel_manager_->get_vca_release();
    // buf[31] = panel_manager_->get_vca_gain();
    buf[32] = panel_manager_->get_lfo_freq();
    buf[33] = panel_manager_->get_lfo_depth();
}
