#pragma once

#include <cstdint>
#include "types.hpp"
#include "selector.hpp"
#include "spi_mcp3008.hpp"
#include "spi_74hc595.hpp"

class PanelManager
{
private:
    Selector *sel_vco1_;
    Selector *sel_vco2_;
    Selector *sel_vcf_;
    Selector *sel_lfo_wavetype_;
    Selector *sel_lfo_target_;
    SPIMCP3008 *adc1_;
    SPIMCP3008 *adc2_;
    SPI74HC595 *led_unit_low_;
    SPI74HC595 *led_unit_high_;
    SPI74HC595 *led_lfo_high_vcf_;
    SPI74HC595 *led_lfo_low_;
    SPI74HC595 *led_vco_;

    uint16_t sound_unit_status_;

    VCOWaveType vco1_wavetype_;
    VCOWaveType vco2_wavetype_;
    uint16_t vco1_duty_;
    uint16_t vco2_offset_;
    uint16_t vco_mix_;
    uint16_t vcf_cutoff_;
    uint16_t vcf_resonance_;
    uint16_t vcf_eg_int_;
    uint16_t vcf_attack_;
    uint16_t vcf_decay_;
    uint16_t vcf_sustain_;
    uint16_t vca_attack_;
    uint16_t vca_decay_;
    uint16_t vca_sustain_;
    uint16_t vca_release_;
    uint16_t portamento_;
    LFOWaveType lfo_wavetype_;
    LFOTarget lfo_target_;
    uint16_t lfo_freq_;
    uint16_t lfo_depth_;

public:
    PanelManager(Selector *vco1, Selector *vco2, Selector *vcf, Selector *lfo_wave, Selector *lfo_target,
                 SPIMCP3008 *adc1, SPIMCP3008 *adc2,
                 SPI74HC595 *led_unit_low, SPI74HC595 *led_unit_high,
                 SPI74HC595 *led_lfo_high_vcf, SPI74HC595 *led_lfo_low, SPI74HC595 *led_vco);

    void set_sound_unit_status(uint16_t status);
    void update();

    VCOWaveType get_vco1_wavetype() const { return vco1_wavetype_; }
    VCOWaveType get_vco2_wavetype() const { return vco2_wavetype_; }
    uint16_t get_vco1_duty() const { return vco1_duty_; }
    uint16_t get_vco2_offset() const { return vco2_offset_; }
    uint16_t get_vco_mix() const { return vco_mix_; }
    VCFFilterType get_vcf_type() const { return static_cast<VCFFilterType>(sel_vcf_->get_state()); }
    uint16_t get_vcf_cutoff() const { return vcf_cutoff_; }
    uint16_t get_vcf_resonance() const { return vcf_resonance_; }
    uint16_t get_vcf_eg_int() const { return vcf_eg_int_; }
    uint16_t get_vcf_attack() const { return vcf_attack_; }
    uint16_t get_vcf_decay() const { return vcf_decay_; }
    uint16_t get_vcf_sustain() const { return vcf_sustain_; }
    uint16_t get_vca_attack() const { return vca_attack_; }
    uint16_t get_vca_decay() const { return vca_decay_; }
    uint16_t get_vca_sustain() const { return vca_sustain_; }
    uint16_t get_vca_release() const { return vca_release_; }
    uint16_t get_portamento() const { return portamento_; }
    LFOWaveType get_lfo_wavetype() const { return lfo_wavetype_; }
    LFOTarget get_lfo_target() const { return lfo_target_; }
    uint16_t get_lfo_freq() const { return lfo_freq_; }
    uint16_t get_lfo_depth() const { return lfo_depth_; }
};
