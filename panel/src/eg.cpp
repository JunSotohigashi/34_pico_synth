#include "eg.hpp"
#include <cmath>

// Update frequency: 20ms cycle = 50Hz
static constexpr float DT = 0.02f;  // Delta time in seconds (20ms per cycle)
static constexpr float THRESHOLD = 0.001f;  // State transition threshold

EG::EG() : value_(0.0f), state_(EGState::IDLE), attack_(0.0f), decay_(0.0f),
           sustain_(1.0f), release_(0.0f), eg_int_(1.0f) {}

void EG::set_adsr(float a, float d, float s, float r, float eg_int)
{
    attack_ = a;
    decay_ = d;
    sustain_ = s;
    release_ = r;
    eg_int_ = eg_int;
}

void EG::gate_on()
{
    state_ = EGState::ATTACK;
    value_ = 0.0f;  // Start attack phase from 0
}

void EG::gate_off()
{
    state_ = EGState::RELEASE;
    // Keep current value and decay from there.
}

void EG::update()
{
    switch (state_)
    {
    case EGState::IDLE:
        value_ = 0.0f;
        break;

    case EGState::ATTACK:
        if (attack_ < THRESHOLD)
        {
            // Instant attack
            value_ = 1.0f;
            state_ = EGState::DECAY;
        }
        else
        {
            // Exponential rise to 1.0
            float coeff = 1.0f - expf(-DT / attack_);
            value_ = value_ + (1.0f - value_) * coeff;
            
            if (value_ > (1.0f - THRESHOLD))
            {
                value_ = 1.0f;
                state_ = EGState::DECAY;
            }
        }
        break;

    case EGState::DECAY:
        if (decay_ < THRESHOLD)
        {
            // Instant decay
            value_ = sustain_;
            state_ = EGState::SUSTAIN;
        }
        else
        {
            // Exponential decay to sustain level
            float coeff = 1.0f - expf(-DT / decay_);
            value_ = value_ + (sustain_ - value_) * coeff;
            
            if (fabsf(value_ - sustain_) < THRESHOLD)
            {
                value_ = sustain_;
                state_ = EGState::SUSTAIN;
            }
        }
        break;

    case EGState::SUSTAIN:
        value_ = sustain_;
        break;

    case EGState::RELEASE:
        if (release_ < THRESHOLD)
        {
            // Instant release
            value_ = 0.0f;
            state_ = EGState::IDLE;
        }
        else
        {
            // Exponential decay to 0.0
            float coeff = 1.0f - expf(-DT / release_);
            value_ = value_ * (1.0f - coeff);
            
            if (value_ < THRESHOLD)
            {
                value_ = 0.0f;
                state_ = EGState::IDLE;
            }
        }
        break;
    }

    // Clamp value to [0.0, 1.0]
    if (value_ < 0.0f) value_ = 0.0f;
    if (value_ > 1.0f) value_ = 1.0f;
}
