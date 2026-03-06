#include "sound_unit.hpp"

SoundUnit::SoundUnit() : portamento_tau_(0.0f), target_freq_(440.0f), current_freq_(440.0f), velocity_(0)
{
    vco_ = new VCO();
    vcf_ = new VCF();
    vcf_eg_ = new EG();
    vca_ = new VCA();
    vca_eg_ = new EG();
    lfo_ = new LFO();
}

SoundUnit::~SoundUnit()
{
    delete vco_;
    delete vcf_;
    delete vcf_eg_;
    delete vca_;
    delete vca_eg_;
    delete lfo_;
}

void SoundUnit::gate_on(uint8_t note, uint8_t velocity)
{
    velocity_ = velocity;
    target_freq_ = 440.0f * powf(2.0f, (note - 69) / 12.0f);
    vcf_eg_->gate_on();
    vca_eg_->gate_on();
}

void SoundUnit::gate_off()
{
    vcf_eg_->gate_off();
    vca_eg_->gate_off();
}

void SoundUnit::set_note(uint8_t note)
{
    target_freq_ = 440.0f * powf(2.0f, (note - 69) / 12.0f);
}

void SoundUnit::set_velocity(uint8_t v)
{
    velocity_ = v;
}

void SoundUnit::update()
{
    current_freq_ += (target_freq_ - current_freq_) * portamento_tau_;
    vco_->set_freq(current_freq_);
    vcf_eg_->update();
    vca_eg_->update();
    lfo_->update();
    
    // Apply VCA EG to gain
    vca_->set_gain(vca_eg_->get_value());
}

void SoundUnit::serialize(uint16_t *buf) const
{
    vco_->serialize(buf);
    vcf_->serialize(buf);
    vca_->serialize(buf);
}
