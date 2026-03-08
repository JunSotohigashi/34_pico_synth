#include "lfo.hpp"
#include <cmath>

// Update frequency: 20ms cycle = 50Hz
static constexpr float DT = 0.02f;
static constexpr float TWO_PI = 6.28318530718f;

LFO::LFO() : wavetype_(LFOWaveType::SIN), target_(LFOTarget::VCO1_PITCH),
             freq_(1.0f), depth_(0.0f), time_(0.0f) {}

void LFO::update()
{
    // Advance phase [0, 1) at rate freq_ Hz
    time_ += freq_ * DT;
    
    // Wrap to [0, 1)
    if (time_ >= 1.0f)
    {
        time_ -= static_cast<int>(time_);
    }
}

float LFO::get_value(LFOTarget target) const
{
    // Only generate output if this LFO is targeting the requested target
    if (target_ != target)
    {
        return 0.0f;
    }
    
    // Generate waveform in range [-1, +1]
    float raw_value = 0.0f;
    
    switch (wavetype_)
    {
    case LFOWaveType::SAW_UP:
        raw_value = 2.0f * time_ - 1.0f;
        break;
        
    case LFOWaveType::SAW_DOWN:
        raw_value = 1.0f - 2.0f * time_;
        break;
        
    case LFOWaveType::SIN:
        raw_value = std::sinf(TWO_PI * time_);
        break;
        
    case LFOWaveType::TRI:
        // Triangle: rises 0->0.5, falls 0.5->1.0
        if (time_ < 0.5f)
        {
            raw_value = 4.0f * time_ - 1.0f;
        }
        else
        {
            raw_value = 3.0f - 4.0f * time_;
        }
        break;
        
    case LFOWaveType::SQUARE:
        raw_value = (time_ < 0.5f) ? 1.0f : -1.0f;
        break;
        
    case LFOWaveType::RANDOM:
        // Simple LCG-based random: updates only at zero-crossing
        // For now, return fixed pattern (proper S&H needs state)
        raw_value = (time_ < 0.5f) ? 0.7f : -0.5f;
        break;
    }
    
    // Apply depth scaling
    return raw_value * depth_;
}
