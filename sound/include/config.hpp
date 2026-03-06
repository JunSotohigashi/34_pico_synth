#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"

constexpr uint8_t PIN_OUT_L = 14;
constexpr uint8_t PIN_OUT_R = 15;

inline spi_inst_t *const SPI_PORT_SOUND = spi0;
constexpr uint8_t PIN_SCK_SOUND = 18;
constexpr uint8_t PIN_RX_SOUND = 16;
constexpr uint8_t PIN_TX_SOUND = 19;
constexpr uint8_t PIN_CS_SOUND = 17;

constexpr uint32_t SPI_BAUDRATE_SOUND = 1000 * 1000;
constexpr uint32_t AUDIO_SAMPLE_RATE = 40000;
constexpr int32_t AUDIO_TIMER_PERIOD_US = -25;  // 40kHz (Fixed_16_16)

constexpr uint8_t STREAM_LENGTH = 20;

constexpr uint8_t IDX_UNIT_ID = 0;
constexpr uint8_t IDX_VCO1_PHASE_DELTA = 1;
constexpr uint8_t IDX_VCO2_PHASE_DELTA = 2;
constexpr uint8_t IDX_VCO1_WAVE = 3;
constexpr uint8_t IDX_VCO2_WAVE = 4;
constexpr uint8_t IDX_VCO1_DUTY = 5;
constexpr uint8_t IDX_VCO_MIX = 6;
constexpr uint8_t IDX_VCF_TYPE = 7;
constexpr uint8_t IDX_VCF_B0_HI = 8;
constexpr uint8_t IDX_VCF_B0_LO = 9;
constexpr uint8_t IDX_VCF_B1_HI = 10;
constexpr uint8_t IDX_VCF_B1_LO = 11;
constexpr uint8_t IDX_VCF_B2_HI = 12;
constexpr uint8_t IDX_VCF_B2_LO = 13;
constexpr uint8_t IDX_VCF_A1_HI = 14;
constexpr uint8_t IDX_VCF_A1_LO = 15;
constexpr uint8_t IDX_VCF_A2_HI = 16;
constexpr uint8_t IDX_VCF_A2_LO = 17;
constexpr uint8_t IDX_VCA_GAIN_L = 18;
constexpr uint8_t IDX_VCA_GAIN_R = 19;

#ifndef UNIT_1
#define UNIT_1 0
#endif
#ifndef UNIT_2
#define UNIT_2 1
#endif
