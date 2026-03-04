#pragma once

#include <cmath>
#include "vco.hpp"
#include "vcf.hpp"
#include "vca.hpp"
#include "eg.hpp"
#include "lfo.hpp"

class SoundUnit
{
private:
    VCO *vco_;
    VCF *vcf_;
    EG *vcf_eg_;
    VCA *vca_;
    EG *vca_eg_;
    LFO *lfo_;
    float portamento_tau_;
    float target_freq_;
    float current_freq_;
    uint8_t velocity_;

public:
    SoundUnit();
    ~SoundUnit();

    void gate_on(uint8_t note, uint8_t velocity);
    void gate_off();
    void set_note(uint8_t note);
    void set_velocity(uint8_t v);
    void update();
    void serialize(uint16_t *buf) const;

    VCO *get_vco() { return vco_; }
    VCF *get_vcf() { return vcf_; }
    VCA *get_vca() { return vca_; }
    EG *get_vcf_eg() { return vcf_eg_; }
    EG *get_vca_eg() { return vca_eg_; }
    LFO *get_lfo() { return lfo_; }
};
