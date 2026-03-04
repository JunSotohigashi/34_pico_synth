#include "panel_manager.hpp"

PanelManager::PanelManager(Selector *vco1, Selector *vco2, Selector *vcf, Selector *lfo_wave, Selector *lfo_target,
                           SPIMCP3008 *adc1, SPIMCP3008 *adc2,
                           SPI74HC595 *led_unit_low, SPI74HC595 *led_unit_high,
                           SPI74HC595 *led_lfo_high_vcf, SPI74HC595 *led_lfo_low, SPI74HC595 *led_vco)
    : sel_vco1_(vco1), sel_vco2_(vco2), sel_vcf_(vcf),
      sel_lfo_wavetype_(lfo_wave), sel_lfo_target_(lfo_target),
      adc1_(adc1), adc2_(adc2),
      led_unit_low_(led_unit_low), led_unit_high_(led_unit_high),
      led_lfo_high_vcf_(led_lfo_high_vcf), led_lfo_low_(led_lfo_low), led_vco_(led_vco),
      sound_unit_status_(0), vco1_wavetype_(VCOWaveType::SAW), vco2_wavetype_(VCOWaveType::SAW),
      vco1_duty_(0), vco2_offset_(0), vco_mix_(0), vcf_cutoff_(0), vcf_resonance_(0), vcf_eg_int_(0),
      vcf_attack_(0), vcf_decay_(0), vcf_sustain_(0), vca_attack_(0), vca_decay_(0),
      vca_sustain_(0), vca_release_(0), portamento_(0),
      lfo_wavetype_(LFOWaveType::SIN), lfo_target_(LFOTarget::VCO1_PITCH),
      lfo_freq_(0), lfo_depth_(0) {}

void PanelManager::set_sound_unit_status(uint16_t status)
{
    sound_unit_status_ = status;
}

void PanelManager::update()
{
    sel_vco1_->update();
    sel_vco2_->update();
    sel_vcf_->update();
    sel_lfo_wavetype_->update();
    sel_lfo_target_->update();

    vco1_wavetype_ = static_cast<VCOWaveType>(sel_vco1_->get_state());
    vco2_wavetype_ = static_cast<VCOWaveType>(sel_vco2_->get_state());
    vco1_duty_ = adc1_->read(2);
    vco2_offset_ = adc1_->read(3);
    vco_mix_ = adc1_->read(1);
    vcf_cutoff_ = adc1_->read(0);
    vcf_resonance_ = adc1_->read(7);
    vcf_eg_int_ = adc1_->read(6);
    vcf_attack_ = adc1_->read(5);
    vcf_decay_ = adc1_->read(4);
    vcf_sustain_ = adc2_->read(7);
    vca_attack_ = adc2_->read(6);
    vca_decay_ = adc2_->read(5);
    vca_sustain_ = adc2_->read(4);
    vca_release_ = adc2_->read(3);
    portamento_ = adc2_->read(2);
    lfo_wavetype_ = static_cast<LFOWaveType>(sel_lfo_wavetype_->get_state());
    lfo_target_ = static_cast<LFOTarget>(sel_lfo_target_->get_state());
    lfo_freq_ = adc2_->read(1);
    lfo_depth_ = adc2_->read(0);

    led_unit_high_->put_8bit((sound_unit_status_ & 0xFF) << 1 | (sound_unit_status_ & 0xFF) >> 7);
    led_unit_low_->put_8bit((sound_unit_status_ >> 8 & 0xFF) << 1 | (sound_unit_status_ >> 8 & 0xFF) >> 7);
}
