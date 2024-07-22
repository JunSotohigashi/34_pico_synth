#include "envelope_generator.hpp"
#include "fixed_point.hpp"

#define CYCLE_DIV 10

EG::EG()
    : value(Fixed_16_16::zero),
      state(EGState::Ready),
      attack(Fixed_16_16::from_float(0.5)),
      decay(Fixed_16_16::from_float(0.5)),
      sustain(Fixed_16_16::from_float(0.5)),
      release(Fixed_16_16::from_float(0.5)),
      tau(Fixed_16_16::from_float(4000.0)),
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
                Fixed_16_16 delta = (Fixed_16_16::one - value) / (attack * tau);
                value += delta == Fixed_16_16::zero ? Fixed_16_16::epsilon : delta;
                if (value + threshold > Fixed_16_16::one)
                {
                    value = Fixed_16_16::one;
                    state = EGState::Decay;
                }
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
                Fixed_16_16 delta = (value - sustain) / (decay * tau);
                value -= delta == Fixed_16_16::zero ? Fixed_16_16::epsilon : delta;
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
                value = Fixed_16_16::zero;
                state = EGState::Ready;
            }
            else
            {
                Fixed_16_16 delta = value / (release * tau);
                value -= delta == Fixed_16_16::zero ? Fixed_16_16::epsilon : delta;
            }
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