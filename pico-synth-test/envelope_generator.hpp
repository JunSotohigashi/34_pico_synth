#ifndef ENVELOPE_GENERATOR_HPP
#define ENVELOPE_GENERATOR_HPP

#include "pico/stdlib.h"

enum class EGState
{
    Ready,
    Attack,
    Decay,
    Sustain,
    Release
};

class EG
{
public:
    EG();
    // ゲインをQ12で出力する
    uint16_t get_value();
    void gate_on();
    void gate_off();

private:
    uint32_t value28;
    EGState state;
    uint16_t attack;
    uint16_t decay;
    uint16_t sustain;
    uint16_t release;
    uint16_t tau;
};

#endif