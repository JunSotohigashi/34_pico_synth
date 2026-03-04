#include "vco.hpp"

VCO::VCO() : freq_(440.0f), wavetype1_(VCOWaveType::SAW), wavetype2_(VCOWaveType::SAW),
             vco1_duty_(0.5f), vco2_offset_(0.0f), mix_(0.5f) {}

void VCO::serialize(uint16_t *buf) const
{
    // VCO serialization (placeholder)
}
