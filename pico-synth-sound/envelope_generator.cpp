#include "envelope_generator.hpp"
#include "fixed_point.hpp"

EG::EG()
    : state(EGState::Ready)
{
}

fpm::fixed_16_16 EG::get_value()
{
    fpm::fixed_16_16 zero{0};
    fpm::fixed_16_16 one{1};
    fpm::fixed_16_16 threshold{0.05};

    if (state == EGState::Ready)
    {
        value = zero;
    }

    if (state == EGState::Attack)
    {
        if (attack == zero)
        {
            value = one;
            state = EGState::Decay;
        }
        else
        {
            value += (one - value) * attack * tau;
            if (value + threshold > one)
            {
                value = one;
                state = EGState::Decay;
            }
        }
    }

    if (state == EGState::Decay)
    {
        if (decay == zero)
        {
            value = sustain;
            state = EGState::Sustain;
        }
        else
        {
            value -= (value - sustain) * decay * tau;
            if (value + threshold < sustain)
            {
                value = sustain;
                state = EGState::Sustain;
            }
        }
    }

    if (state == EGState::Sustain)
    {
        value = sustain;
    }

    if (state == EGState::Release)
    {
        if (value < threshold)
        {
            value = zero;
            state = EGState::Ready;
        }
        else
        {
            value -= value * release * tau;
        }
    }

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