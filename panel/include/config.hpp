#pragma once

#include <cstdint>

// ============================================================================
// Hardware Configuration (Existing Panel Hardware)
// ============================================================================

// UART Configuration (Keyboard MIDI)
constexpr uint8_t PIN_UART_TX = 0;
constexpr uint8_t PIN_UART_RX = 1;
constexpr uint32_t UART_BAUDRATE = 115200;

// SPI Panel (spi0) - ADC + LED
constexpr uint8_t PIN_SCK_PANEL = 18;
constexpr uint8_t PIN_MOSI_PANEL = 19;
constexpr uint8_t PIN_MISO_PANEL = 16;
constexpr uint8_t PIN_CS_ADC1 = 7;
constexpr uint8_t PIN_CS_ADC2 = 8;
constexpr uint8_t PIN_CS_SR_UNIT_LOW = 2;
constexpr uint8_t PIN_CS_SR_LFO_HIGH_VCF = 3;
constexpr uint8_t PIN_CS_SR_LFO_LOW = 4;
constexpr uint8_t PIN_CS_SR_UNIT_HIGH = 5;
constexpr uint8_t PIN_CS_SR_VCO = 6;
constexpr uint32_t SPI_BAUDRATE_PANEL = 1000 * 1000;  // 1 MHz

// SPI Sound (spi1) - Sound Module Communication
constexpr uint8_t PIN_SCK_SOUND = 14;
constexpr uint8_t PIN_MOSI_SOUND = 15;
constexpr uint8_t PIN_MISO_SOUND = 12;
constexpr uint8_t PIN_CS_SOUND = 13;
constexpr uint32_t SPI_BAUDRATE_SOUND = 1000 * 1000;  // 1 MHz

// Switches (Active Low, Pull-up)
constexpr uint8_t PIN_SW_VCO1_R = 29;
constexpr uint8_t PIN_SW_VCO1_L = 28;
constexpr uint8_t PIN_SW_VCO2_R = 27;
constexpr uint8_t PIN_SW_VCO2_L = 26;
constexpr uint8_t PIN_SW_VCF_R = 25;
constexpr uint8_t PIN_SW_VCF_L = 24;
constexpr uint8_t PIN_SW_LFO_WAVE_R = 23;
constexpr uint8_t PIN_SW_LFO_WAVE_L = 22;
constexpr uint8_t PIN_SW_LFO_TARGET_R = 21;
constexpr uint8_t PIN_SW_LFO_TARGET_L = 20;

// Voice/Unit Configuration
constexpr uint8_t N_UNIT = 16;  // 8 Pico × 2 channels

// Stream Configuration
constexpr uint8_t STREAM_LENGTH = 34;  // Words (16-bit each)

// MIDI Constants
constexpr uint8_t MIDI_NOTE_ON = 0x90;
constexpr uint8_t MIDI_NOTE_OFF = 0x80;
constexpr uint8_t MIDI_CC = 0xB0;
constexpr uint8_t MIDI_SUSTAIN_CC = 0x40;

