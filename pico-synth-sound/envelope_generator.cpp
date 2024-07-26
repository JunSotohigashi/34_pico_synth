#include "envelope_generator.hpp"
#include "fixed_point.hpp"

#define CYCLE_DIV 100

EG::EG()
    : value(Fixed_16_16::zero),
      state(EGState::Ready),
      state_old(EGState::Ready),
      attack(Fixed_16_16::from_float(0.008)),
      decay(Fixed_16_16::from_float(0.001)),
      sustain(Fixed_16_16::from_float(0.5)),
      release(Fixed_16_16::from_float(0.001)),
      tau((Fixed_16_16::from_float(1.0f / (3.0f * 400.0f)))), // 5.0s for slope, 400.0Hz cycle
      cycle(0)
{
}

Fixed_16_16 EG::get_value()
{
    if (state == EGState::Ready)
    {
        value = Fixed_16_16::zero;
    }

    if (state == EGState::Attack)
    {
        if (attack == Fixed_16_16::zero)
        {
            value = Fixed_16_16::one;
            state = EGState::Decay;
        }
        else
        {
            Fixed_16_16 delta = tau / attack;
            if (Fixed_16_16::one - value < delta)
            {
                value = Fixed_16_16::one;
                state = EGState::Decay;
            }
            else if (delta == Fixed_16_16::zero)
                value += Fixed_16_16::epsilon;
            else
                value += delta;
        }
    }

    if (state == EGState::Decay)
    {
        if (decay == Fixed_16_16::zero)
        {
            value = sustain;
            state = EGState::Sustain;
        }
        else
        {
            Fixed_16_16 delta = tau * (Fixed_16_16::one - sustain) / decay;
            if (value - sustain < delta)
            {
                value = sustain;
                state = EGState::Sustain;
            }
            else if (delta == Fixed_16_16::zero)
                value -= Fixed_16_16::epsilon;
            else
                value -= delta;
        }
    }

    if (state == EGState::Sustain)
    {
        value = sustain;
    }

    if (state == EGState::Release)
    {
        if (state_old != EGState::Release && value > sustain)
            value = sustain;

        if (release == Fixed_16_16::zero)
        {
            value = Fixed_16_16::zero;
            state = EGState::Ready;
        }
        else
        {
            Fixed_16_16 delta = tau * sustain / release;
            if (value < delta)
            {
                value = Fixed_16_16::zero;
                state = EGState::Ready;
            }
            else if (delta == Fixed_16_16::zero)
                value -= Fixed_16_16::epsilon;
            else
                value -= delta;
        }
    }

    if (value > Fixed_16_16::one)
        value = Fixed_16_16::one;
    if (value < Fixed_16_16::zero)
        value = Fixed_16_16::zero;

    state_old = state;
    return value;
}

void EG::gate_on()
{
    state = EGState::Attack;
}

void EG::gate_off()
{
    state = EGState::Release;
}

void EG::set_adsr(Fixed_16_16 attack, Fixed_16_16 decay, Fixed_16_16 sustain, Fixed_16_16 release)
{
    this->attack = attack;
    this->decay = decay;
    this->sustain = sustain;
    this->release = release;
}