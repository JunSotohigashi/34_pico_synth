#include "envelope_generator.hpp"
#include "fixed_point.hpp"

EG::EG()
    : value(Fixed_16_16::from_int32(0)),
      state(EGState::Ready),
      attack(Fixed_16_16::from_float(0.5)),
      decay(Fixed_16_16::from_float(0.5)),
      sustain(Fixed_16_16::from_float(0.5)),
      release(Fixed_16_16::from_float(0.5)),
      tau(Fixed_16_16::from_float(40000.0))
{
}

Fixed_16_16 EG::get_value()
{
    Fixed_16_16 zero = Fixed_16_16::from_int32(0);
    Fixed_16_16 one = Fixed_16_16::from_int32(1);
    Fixed_16_16 epsilon = Fixed_16_16::from_raw_value(1);
    Fixed_16_16 threshold = Fixed_16_16::from_float(0.01);

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
            Fixed_16_16 delta = (one - value) / (attack * tau);
            value += delta == zero ? epsilon : delta;
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
            Fixed_16_16 delta = (value - sustain) / (decay * tau);
            value -= delta == zero ? epsilon : delta;
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
            Fixed_16_16 delta = value / (release * tau);
            value -= delta == zero ? epsilon : delta;
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