#include "lfo.hpp"

LFO::LFO() : wavetype_(LFOWaveType::SIN), target_(LFOTarget::VCO1_PITCH),
             freq_(1.0f), depth_(0.0f), time_(0.0f) {}

void LFO::update()
{
    // LFO update (placeholder)
}

float LFO::get_value(LFOTarget target) const
{
    // LFO value calculation (placeholder)
    return 0.0f;
}
