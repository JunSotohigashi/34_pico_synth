#pragma once

#include <cstdint>

// ============================================================================
// Enum Type Definitions
// ============================================================================

enum class VCOWaveType : uint8_t
{
    SAW = 0,
    SIN = 1,
    TRI = 2,
    SQUARE = 3
};

enum class VCFFilterType : uint8_t
{
    LPF = 0,
    HPF = 1
};

enum class LFOWaveType : uint8_t
{
    SAW_UP = 0,
    SAW_DOWN = 1,
    SIN = 2,
    TRI = 3,
    SQUARE = 4,
    RANDOM = 5
};

enum class LFOTarget : uint8_t
{
    VCO1_PITCH = 0,
    VCO2_DUTY = 1,
    VCO_MIX = 2,
    VCF_CUTOFF = 3,
    VCA_GAIN = 4,
    VCA_PAN = 5
};

enum class EGState : uint8_t
{
    IDLE = 0,
    ATTACK = 1,
    DECAY = 2,
    SUSTAIN = 3,
    RELEASE = 4
};

enum class SoundUnitState : uint8_t
{
    IDLE = 0,
    ACTIVE = 1,
    SUSTAIN = 2
};
