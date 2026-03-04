#include "eg.hpp"

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
}

void EG::gate_off()
{
    state_ = EGState::RELEASE;
}

void EG::update()
{
    // EG envelope update (placeholder)
}
