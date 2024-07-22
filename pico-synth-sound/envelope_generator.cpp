#include "envelope_generator.hpp"
#include "fixed_point.hpp"

#define CYCLE_DIV 100

EG::EG()
    : value(Fixed_16_16::zero),
      state(EGState::Ready),
      attack(Fixed_16_16::from_float(0.008)),
      decay(Fixed_16_16::from_float(0.001)),
      sustain(Fixed_16_16::from_float(0.5)),
      release(Fixed_16_16::from_float(0.001)),
      cycle(0)
{
}

Fixed_16_16 EG::get_value()
{
    if (cycle == 0)
    {
        Fixed_16_16 threshold = Fixed_16_16::from_float(0.01);

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
                if (Fixed_16_16::one - value < attack)
                {
                    value = Fixed_16_16::one;
                    state = EGState::Decay;
                }
                else
                    value += attack;
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
                if (value - sustain < decay)
                {
                    value = sustain;
                    state = EGState::Sustain;
                }
                else
                    value -= decay;
            }
        }

        if (state == EGState::Sustain)
        {
            value = sustain;
        }

        if (state == EGState::Release)
        {
            if (value < release)
            {
                value = Fixed_16_16::zero;
                state = EGState::Ready;
            }
            else
                value -= release;
        }
    }

    cycle = (cycle + 1) % CYCLE_DIV;

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