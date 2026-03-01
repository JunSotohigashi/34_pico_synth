#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/sync.h>
#include <hardware/uart.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <cstdio>
#include <cstring>
#include <climits>

#include "config.hpp"
#include "spi_mcp3008.hpp"
#include "spi_74hc595.hpp"
#include "selector.hpp"

// ============================================================================
// Global Variables (Shared between cores)
// ============================================================================

semaphore_t g_sem;
uint16_t g_unit_state = 0;              // 16-bit bitmap for Unit 0-15
uint8_t g_unit_note[N_UNIT] = {0};      // Note number for each unit
uint8_t g_unit_velocity[N_UNIT] = {0};  // Velocity for each unit
uint16_t g_unit_sustain = 0;            // Units in sustain (RELEASE state)

// Bidirectional mapping
uint8_t g_note_to_unit[128];            // note → unit (255 = unassigned)
uint32_t g_unit_history[N_UNIT] = {0};  // Ring buffer for voice stealing (timestamp)
uint8_t g_last_unit_used = 255;         // Last unit used for note on (to avoid reusing)

enum class KeyState : uint8_t {
    OFF = 0,
    PUSH = 1,
    RELEASE = 2
};

// ============================================================================
// Helper Functions for Voice Allocation
// ============================================================================

/// Find the oldest unit (minimum history counter) for voice stealing
uint8_t find_oldest_unit() {
    uint32_t min_history = UINT32_MAX;
    uint8_t oldest_unit = 0;
    for (uint8_t u = 0; u < N_UNIT; u++) {
        if (g_unit_history[u] < min_history) {
            min_history = g_unit_history[u];
            oldest_unit = u;
        }
    }
    return oldest_unit;
}

/// Find a free unit (not active) using Round-Robin allocation
uint8_t find_free_unit(bool avoid_last) {
    uint8_t start_unit = (g_last_unit_used == 255) ? 0 : (g_last_unit_used + 1) % N_UNIT;
    
    // Round-Robin search starting from next unit
    for (uint8_t i = 0; i < N_UNIT; i++) {
        uint8_t u = (start_unit + i) % N_UNIT;
        if (!(g_unit_state & (1 << u))) {
            return u;
        }
    }
    
    return 255;  // No free unit
}

/// Assign note to unit with velocity
void assign_unit(uint8_t unit, uint8_t note, uint8_t velocity, uint32_t& history_counter) {
    g_unit_state |= (1 << unit);
    g_unit_note[unit] = note;
    g_unit_velocity[unit] = velocity;
    g_note_to_unit[note] = unit;
    g_unit_history[unit] = history_counter++;
    g_last_unit_used = unit;
}

/// Release a unit (clear note assignment)
void release_unit(uint8_t unit) {
    uint8_t note = g_unit_note[unit];
    g_unit_state &= ~(1 << unit);
    g_unit_note[unit] = 0;
    g_unit_velocity[unit] = 0;
    g_unit_sustain &= ~(1 << unit);
    g_note_to_unit[note] = 255;
}

/// Handle Note On message
void handle_note_on(uint8_t note, uint8_t velocity, uint32_t& history_counter, bool sustain, KeyState note_state[]) {
    uint8_t unit = g_note_to_unit[note];
    
    // Case 1: Note already assigned - retrigger by OFF→ON transition
    if (unit != 255 && (g_unit_state & (1 << unit))) {
        // Transition 0→1 will be detected by Sound's interrupt handler
        g_unit_velocity[unit] = velocity;
        g_unit_sustain &= ~(1 << unit);
        g_unit_history[unit] = history_counter++;
        note_state[note] = KeyState::PUSH;
        return;
    }
    
    // Case 2: Need to assign unit
    unit = find_free_unit(false);  // Always use Round-Robin
    
    // Voice stealing if no free unit
    if (unit == 255) {
        unit = find_oldest_unit();
        uint8_t old_note = g_unit_note[unit];
        
        // Turn OFF unit to signal note end
        g_unit_state &= ~(1 << unit);
        g_unit_note[unit] = 0;
        g_unit_velocity[unit] = 0;
        
        // Mark old note as stolen
        g_note_to_unit[old_note] = 255;
        note_state[old_note] = KeyState::OFF;
    }
    
    // Assign unit to new note
    assign_unit(unit, note, velocity, history_counter);
    g_unit_sustain &= ~(1 << unit);
}

/// Handle Note Off message
void handle_note_off(uint8_t note, bool sustain) {
    uint8_t unit = g_note_to_unit[note];
    
    if (unit == 255) {
        return;  // Note not assigned
    }
    
    if (sustain) {
        // Mark unit as sustained (keep sound playing)
        if (g_unit_state & (1 << unit)) {
            g_unit_sustain |= (1 << unit);
        }
    } else {
        // Immediate release
        if (g_unit_state & (1 << unit)) {
            g_unit_sustain &= ~(1 << unit);
            release_unit(unit);
        }
    }
}

/// Handle Sustain Pedal CC (0x40)
void handle_sustain(uint8_t cc_value) {
    bool sustain = (bool)cc_value;
    
    // Release all RELEASE notes when sustain OFF
    if (!sustain) {
        for (uint8_t note = 0; note < 128; note++) {
            // (note_state tracking is local to core1_uart_midi)
            // So we only release notes that are not PUSH
            uint8_t unit = g_note_to_unit[note];
            if (unit != 255 && (g_unit_state & (1 << unit))) {
                // Check if key is released (by trying all released notes)
                // For now, we track this in the calling context
            }
        }
    }
}

/// ============================================================================
// Core 1: UART MIDI Reception Thread
// ============================================================================

void core1_uart_midi() {
    char buf[256] = {0};
    char buf_msg[12] = {0};
    uint8_t buf_index = 0;
    
    // Initialize bidirectional mappings
    for (uint8_t i = 0; i < 128; i++) {
        g_note_to_unit[i] = 255;  // 255 = unassigned
    }
    
    KeyState note_state[128] = {KeyState::OFF};
    bool sustain = false;
    uint32_t history_counter = 1;  // Global timestamp for voice stealing priority
    
    while (true) {
        if (!uart_is_readable(uart0)) {
            continue;
        }
        
        buf[buf_index] = uart_getc(uart0);
        
        // Wait for message end (newline)
        if (buf[buf_index] != '\n') {
            buf_index++;
            continue;
        }
        
        // Extract last 9 characters: "90 3c 7f\n"
        for (uint16_t i = 0; i <= 8; i++) {
            buf_msg[i] = buf[(buf_index + i - 8) & 0xFF];
        }
        buf_msg[10] = '\0';
        
        // Parse ASCII hex to binary: "90 3c 7f" → [0x90, 0x3c, 0x7f]
        uint8_t msg[3];
        sscanf(buf_msg, "%hhx %hhx %hhx", &msg[0], &msg[1], &msg[2]);
        
        sem_acquire_blocking(&g_sem);
        {
            uint8_t status = msg[0];
            uint8_t data1 = msg[1];
            uint8_t data2 = msg[2];
            
            // Note Off (0x80 or 0x90 with velocity=0)
            if ((status == 0x80) || (status == 0x90 && data2 == 0)) {
                uint8_t note = data1;
                note_state[note] = sustain ? KeyState::RELEASE : KeyState::OFF;
                handle_note_off(note, sustain);
            }
            // Note On (0x90 with velocity > 0)
            else if (status == 0x90 && data2 != 0) {
                uint8_t note = data1;
                uint8_t velocity = data2;
                note_state[note] = KeyState::PUSH;
                handle_note_on(note, velocity, history_counter, sustain, note_state);
            }
            // Control Change (0xB0)
            else if (status == 0xB0) {
                uint8_t cc_num = data1;
                uint8_t cc_val = data2;
                
                // Sustain Pedal (CC 64)
                if (cc_num == 0x40) {
                    sustain = (cc_val != 0);
                    
                    // Release all sustained notes when sustain OFF
                    if (!sustain) {
                        for (uint8_t note = 0; note < 128; note++) {
                            if (note_state[note] == KeyState::RELEASE) {
                                uint8_t unit = g_note_to_unit[note];
                                if (unit != 255 && (g_unit_state & (1 << unit))) {
                                    g_unit_sustain &= ~(1 << unit);
                                    release_unit(unit);
                                }
                                note_state[note] = KeyState::OFF;
                            }
                        }
                    }
                }
            }
        }
        sem_release(&g_sem);
        
        buf_index++;
    }
}

// ============================================================================
// Helper Functions for Main Loop
// ============================================================================

/// Update all selector switches and LEDs
void update_selectors(Selector& sel_vco1, Selector& sel_vco2, Selector& sel_vcf,
                      Selector& sel_lfo_wave, Selector& sel_lfo_target) {
    sel_vco1.update();
    sel_vco2.update();
    sel_vcf.update();
    sel_lfo_wave.update();
    sel_lfo_target.update();
}

/// Update unit status LEDs based on g_unit_state
void update_unit_leds(SPI74HC595& led_unit_high, SPI74HC595& led_unit_low) {
    // Rotate 16-bit unit state into shift register LED pattern
    led_unit_high.put_8bit((g_unit_state & 0xFF) << 1 | (g_unit_state & 0xFF) >> 7);
    led_unit_low.put_8bit((g_unit_state >> 8 & 0xFF) << 1 | (g_unit_state >> 8 & 0xFF) >> 7);
}

/// Build parameter stream (34 words × 16-bit) from ADCs and selector state
void build_stream(uint16_t stream[STREAM_LENGTH],
                  SPIMCP3008& adc1, SPIMCP3008& adc2,
                  const Selector& sel_vco1, const Selector& sel_vco2,
                  const Selector& sel_vcf, const Selector& sel_lfo_wave,
                  const Selector& sel_lfo_target) {
    // Word 0: Header
    stream[0] = 0xFFFF;
    
    // Words 1-16: Unit state (note, velocity, active flag)
    sem_acquire_blocking(&g_sem);
    for (uint8_t ch = 0; ch < N_UNIT; ch++) {
        bool is_active = (g_unit_state >> ch) & 1;
        bool is_sustained = (g_unit_sustain >> ch) & 1;
        uint8_t velocity = g_unit_velocity[ch];
        
        // Force gate OFF if velocity is 0 (unless sustained)
        if (velocity == 0 && !is_sustained) {
            is_active = false;
        }
        
        stream[ch + 1] = (is_active ? 1 : 0) << 15 
                       | g_unit_note[ch] << 8 
                       | velocity;
    }
    sem_release(&g_sem);
    
    // Word 17: Mode settings (selector states)
    stream[17] = sel_vco1.get_state() << 10 
               | sel_vco2.get_state() << 8 
               | sel_vcf.get_state() << 7 
               | sel_lfo_wave.get_state() << 3 
               | sel_lfo_target.get_state();
    
    // Words 18-33: Parameter values (ADC inputs)
    stream[18] = adc1.read(2);  // VCO Duty
    stream[19] = adc1.read(3);  // VCO2 Tune
    stream[20] = adc1.read(1);  // VCO Mix
    stream[21] = adc1.read(0);  // VCF CutOff
    stream[22] = adc1.read(7);  // VCF Resonance
    stream[23] = adc1.read(6);  // VCF Attack
    stream[24] = adc1.read(5);  // VCF Decay
    stream[25] = adc2.read(7);  // VCF Sustain
    stream[26] = adc1.read(4);  // VCF Release
    stream[27] = adc2.read(6);  // VCA Attack
    stream[28] = adc2.read(5);  // VCA Decay
    stream[29] = adc2.read(4);  // VCA Sustain
    stream[30] = adc2.read(3);  // VCA Release
    stream[31] = adc2.read(2);  // VCA Gain
    stream[32] = adc2.read(1);  // LFO Speed
    stream[33] = adc2.read(0);  // LFO Depth
}

/// Transmit stream to sound units via SPI
void transmit_stream(uint16_t stream[STREAM_LENGTH]) {
    for (uint8_t i = 0; i < STREAM_LENGTH; i++) {
        gpio_put(PIN_CS_SOUND, false);
        sleep_us(10);
        spi_write16_blocking(spi1, &stream[i], 1);
        sleep_us(10);
        gpio_put(PIN_CS_SOUND, true);
    }
}

/// ============================================================================
// Main (Core 0): Main Loop with Selector, LED, ADC, and SPI Updates
// ============================================================================

int main() {
    stdio_init_all();
    sem_init(&g_sem, 1, 1);
    
    // ========================================================================
    // Hardware Initialization
    // ========================================================================
    
    // UART initialization (MIDI input)
    uart_init(uart0, UART_BAUDRATE);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);
    
    // SPI Panel initialization (spi0 for ADC and LED shift registers)
    spi_init(spi0, SPI_BAUDRATE_PANEL);
    gpio_set_function(PIN_MISO_PANEL, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_PANEL, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_PANEL, GPIO_FUNC_SPI);
    
    // SPI Sound initialization (spi1 for streaming to sound units)
    spi_init(spi1, SPI_BAUDRATE_SOUND);
    spi_set_format(spi1, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_SOUND, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS_SOUND, GPIO_OUT);
    gpio_put(PIN_CS_SOUND, true);
    
    // ADC (MCP3008 × 2) for parameter knobs
    SPIMCP3008 adc1(spi0, PIN_CS_ADC1);
    SPIMCP3008 adc2(spi0, PIN_CS_ADC2);
    
    // LED Shift Registers (SPI74HC595 × 5)
    SPI74HC595 led_unit_low(spi0, PIN_CS_SR_UNIT_LOW);
    SPI74HC595 led_unit_high(spi0, PIN_CS_SR_UNIT_HIGH);
    SPI74HC595 led_lfo_high_vcf(spi0, PIN_CS_SR_LFO_HIGH_VCF);
    SPI74HC595 led_lfo_low(spi0, PIN_CS_SR_LFO_LOW);
    SPI74HC595 led_vco(spi0, PIN_CS_SR_VCO);
    
    // Selector switches with LED feedback
    SPI74HC595* vco1_sr[4] = {&led_vco, &led_vco, &led_vco, &led_vco};
    uint8_t vco1_pin[4] = {1, 2, 3, 4};
    Selector sel_vco1(PIN_SW_VCO1_R, PIN_SW_VCO1_L, 4, vco1_sr, vco1_pin);
    
    SPI74HC595* vco2_sr[4] = {&led_vco, &led_vco, &led_vco, &led_vco};
    uint8_t vco2_pin[4] = {5, 6, 7, 0};
    Selector sel_vco2(PIN_SW_VCO2_R, PIN_SW_VCO2_L, 4, vco2_sr, vco2_pin);
    
    SPI74HC595* vcf_sr[2] = {&led_lfo_high_vcf, &led_lfo_high_vcf};
    uint8_t vcf_pin[2] = {7, 0};
    Selector sel_vcf(PIN_SW_VCF_R, PIN_SW_VCF_L, 2, vcf_sr, vcf_pin);
    
    SPI74HC595* lfo_wave_sr[6] = {&led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf, 
                                    &led_lfo_high_vcf, &led_lfo_high_vcf, &led_lfo_high_vcf};
    uint8_t lfo_wave_pin[6] = {1, 2, 3, 4, 5, 6};
    Selector sel_lfo_wave(PIN_SW_LFO_WAVE_R, PIN_SW_LFO_WAVE_L, 6, lfo_wave_sr, lfo_wave_pin);
    
    SPI74HC595* lfo_target_sr[6] = {&led_lfo_low, &led_lfo_low, &led_lfo_low, 
                                      &led_lfo_low, &led_lfo_low, &led_lfo_low};
    uint8_t lfo_target_pin[6] = {1, 2, 3, 4, 5, 6};
    Selector sel_lfo_target(PIN_SW_LFO_TARGET_R, PIN_SW_LFO_TARGET_L, 6, lfo_target_sr, lfo_target_pin);
    
    // printf("=== Panel Module Started ===\n");
    // printf("Core 0: Main loop (Selector, LED, ADC, SPI)\n");
    // printf("Core 1: UART MIDI reception + Voice Allocation\n");
    
    // Launch Core 1 for MIDI processing
    multicore_launch_core1(core1_uart_midi);
    
    // ========================================================================
    // Main Loop: Update Selectors → Update LEDs → Read ADCs → Build Stream → Send
    // ========================================================================
    uint16_t stream[STREAM_LENGTH];
    
    while (true) {
        // 1. Update selector switches and their LED feedback
        update_selectors(sel_vco1, sel_vco2, sel_vcf, sel_lfo_wave, sel_lfo_target);
        
        // 2. Update unit status LEDs based on active voices
        update_unit_leds(led_unit_high, led_unit_low);
        
        // 3. Build 34-word parameter stream
        build_stream(stream, adc1, adc2, sel_vco1, sel_vco2, sel_vcf, sel_lfo_wave, sel_lfo_target);
        
        // 4. Transmit stream to all sound units via SPI
        transmit_stream(stream);
    }
    
    return 0;
}
