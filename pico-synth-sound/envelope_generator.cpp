#include "envelope_generator.hpp"
#include "fixed_point.hpp"

EG::EG()
    : value28(0),
      state(EGState::Ready),
      attack(0xFFFF),
      decay(0xFFFF),
      sustain(0x5FFF),
      release(0xF),
      tau(0x1000)
{
}

uint16_t EG::get_value()
{
    if (state == EGState::Ready)
    {
        value28 = 0;
    }

    if (state == EGState::Attack)
    {
        if (attack == 0)
        {
            value28 = 0x10000000;
            state = EGState::Decay;
        }
        else
        {
            value28 += mul_q28_q28((0x10000000 - value28), 0x8000);
            if (value28 + 0x100000 > 0x10000000)
            {
                value28 = 0x10000000;
                state = EGState::Decay;
            }
        }
    }

    uint32_t sustain_value28 = ((uint32_t)sustain + 1) << 12;
    if (state == EGState::Decay)
    {
        if (decay == 0)
        {
            value28 = sustain_value28;
            state = EGState::Sustain;
        }
        else
        {
            value28 -= mul_q28_q28((value28 - sustain_value28), 0x8000);
            if (value28 + 0x100000 < sustain_value28)
            {
                value28 = sustain_value28;
                state = EGState::Sustain;
            }
        }
    }

    if (state == EGState::Sustain)
    {
        value28 = sustain_value28;
    }

    if (state == EGState::Release)
    {
        if (value28 < 0x100000)
        {
            value28 = 0;
            state = EGState::Ready;
        }
        else
        {
            value28 -= mul_q28_q28(value28, 0x8000);
        }
    }

    return value28 >> 16;
}

void EG::gate_on()
{
    state = EGState::Attack;
}

void EG::gate_off()
{
    state = EGState::Release;
}