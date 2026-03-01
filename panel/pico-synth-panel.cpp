#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/sync.h>
#include <pico/util/queue.h>
#include <hardware/uart.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <cstdio>
#include <cstring>

#include "config.hpp"
#include "spi_mcp3008.hpp"
#include "spi_74hc595.hpp"
#include "selector.hpp"

// ============================================================================
// Global Variables (Shared between cores)
// ============================================================================

semaphore_t g_sem;
uint16_t g_unit_state = 0;           // 16-bit bitmap for Unit 0-15
uint8_t g_unit_note[N_UNIT] = {0};   // Note number for each unit
uint8_t g_unit_velocity[N_UNIT] = {0};  // Velocity for each unit

enum class KeyState : uint8_t {
    OFF = 0,
    PUSH = 1,
    RELEASE = 2
};

// ============================================================================
// Core 1: UART MIDI Reception Thread
// ============================================================================

void core1_uart_midi() {
    char buf[256] = {0};
    char buf_msg[12] = {0};
    uint8_t buf_index = 0;
    
    uint8_t unit_next = 0;
    queue_t unit_busy;
    queue_init(&unit_busy, sizeof(uint8_t), N_UNIT);
    
    KeyState note_state[128] = {KeyState::OFF};
    bool sustain = false;
    
    while (true) {
        if (uart_is_readable(uart0)) {
            buf[buf_index] = uart_getc(uart0);
            
            // Message end detection (newline)
            if (buf[buf_index] == '\n') {
                // Extract last 9 characters: "90 3c 7f\n"
                for (uint16_t i = 0; i <= 8; i++) {
                    buf_msg[i] = buf[(buf_index + i - 8) & 0xFF];
                }
                buf_msg[10] = '\0';
                
                // Parse ASCII hex to binary
                uint8_t msg[3];
                sscanf(buf_msg, "%hhx %hhx %hhx", &msg[0], &msg[1], &msg[2]);
                
                sem_acquire_blocking(&g_sem);
                
                // Note Off (0x90 with velocity 0)
                if (msg[0] == 0x90 && msg[2] == 0) {
                    if (sustain) {
                        note_state[msg[1]] = KeyState::RELEASE;
                    } else {
                        note_state[msg[1]] = KeyState::OFF;
                        // Find and release unit
                        uint8_t i_max = queue_get_level(&unit_busy);
                        for (uint8_t i = 0; i < i_max; i++) {
                            uint8_t unit;
                            queue_remove_blocking(&unit_busy, &unit);
                            if ((g_unit_state & (1 << unit)) && (g_unit_note[unit] == msg[1])) {
                                g_unit_state &= ~(1 << unit);
                                g_unit_note[unit] = 0;
                                g_unit_velocity[unit] = 0;
                            } else {
                                queue_add_blocking(&unit_busy, &unit);
                            }
                        }
                    }
                }
                // Note On
                else if (msg[0] == 0x90 && msg[2] != 0) {
                    note_state[msg[1]] = KeyState::PUSH;
                    bool found_free_unit = false;
                    
                    // Find free unit (round-robin)
                    for (uint8_t i = 0; i < N_UNIT; i++) {
                        uint8_t unit = (i + unit_next) % N_UNIT;
                        if (!(g_unit_state & (1 << unit))) {
                            g_unit_state |= (1 << unit);
                            g_unit_note[unit] = msg[1];
                            g_unit_velocity[unit] = msg[2];
                            unit_next = (unit + 1) % N_UNIT;
                            queue_add_blocking(&unit_busy, &unit);
                            found_free_unit = true;
                            break;
                        }
                    }
                    
                    // Voice stealing if no free unit
                    if (!found_free_unit) {
                        uint8_t unit;
                        queue_remove_blocking(&unit_busy, &unit);
                        g_unit_note[unit] = msg[1];
                        g_unit_velocity[unit] = msg[2];
                        unit_next = (unit + 1) % N_UNIT;
                        queue_add_blocking(&unit_busy, &unit);
                    }
                }
                // Sustain Pedal (CC 64)
                else if (msg[0] == 0xB0 && msg[1] == 0x40) {
                    sustain = (bool)msg[2];
                    
                    // Release all RELEASE notes when sustain OFF
                    if (!sustain) {
                        for (uint8_t note = 0; note < 128; note++) {
                            if (note_state[note] == KeyState::RELEASE) {
                                note_state[note] = KeyState::OFF;
                                uint8_t u_max = queue_get_level(&unit_busy);
                                for (uint8_t u = 0; u < u_max; u++) {
                                    uint8_t unit;
                                    queue_remove_blocking(&unit_busy, &unit);
                                    if ((g_unit_state & (1 << unit)) && (g_unit_note[unit] == note)) {
                                        g_unit_state &= ~(1 << unit);
                                        g_unit_note[unit] = 0;
                                        g_unit_velocity[unit] = 0;
                                    } else {
                                        queue_add_blocking(&unit_busy, &unit);
                                    }
                                }
                            }
                        }
                    }
                }
                
                sem_release(&g_sem);
            }
            buf_index++;
        }
    }
}

// ============================================================================
// Main (Core 0): Main Loop
// ============================================================================

int main() {
    stdio_init_all();
    sem_init(&g_sem, 1, 1);
    
    // UART initialization
    uart_init(uart0, UART_BAUDRATE);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);
    
    // SPI Panel initialization (spi0)
    spi_init(spi0, SPI_BAUDRATE_PANEL);
    gpio_set_function(PIN_MISO_PANEL, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_PANEL, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_PANEL, GPIO_FUNC_SPI);
    
    // SPI Sound initialization (spi1)
    spi_init(spi1, SPI_BAUDRATE_SOUND);
    spi_set_format(spi1, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_SOUND, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS_SOUND, GPIO_OUT);
    gpio_put(PIN_CS_SOUND, true);
    
    // ADC (MCP3008 × 2)
    SPIMCP3008 adc1(spi0, PIN_CS_ADC1);
    SPIMCP3008 adc2(spi0, PIN_CS_ADC2);
    
    // LED Shift Registers (74HC595 × 5)
    SPI74HC595 led_unit_low(spi0, PIN_CS_SR_UNIT_LOW);
    SPI74HC595 led_unit_high(spi0, PIN_CS_SR_UNIT_HIGH);
    SPI74HC595 led_lfo_high_vcf(spi0, PIN_CS_SR_LFO_HIGH_VCF);
    SPI74HC595 led_lfo_low(spi0, PIN_CS_SR_LFO_LOW);
    SPI74HC595 led_vco(spi0, PIN_CS_SR_VCO);
    
    // Selectors (Switches + LED)
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
    
    printf("=== Panel Module Started ===\n");
    printf("Core 0: Main loop\n");
    printf("Core 1: UART MIDI reception\n");
    
    // Launch Core 1
    multicore_launch_core1(core1_uart_midi);
    
    // Main loop
    while (true) {
        // 1. Update switches
        sel_vco1.update();
        sel_vco2.update();
        sel_vcf.update();
        sel_lfo_wave.update();
        sel_lfo_target.update();
        
        // 2. Update unit LED
        led_unit_high.put_8bit((g_unit_state & 0xFF) << 1 | (g_unit_state & 0xFF) >> 7);
        led_unit_low.put_8bit((g_unit_state >> 8 & 0xFF) << 1 | (g_unit_state >> 8 & 0xFF) >> 7);
        
        // 3. Build stream (34 words × 16 bit)
        uint16_t stream[STREAM_LENGTH];
        stream[0] = 0xFFFF;  // Header
        
        // Unit state (Word 1-16)
        sem_acquire_blocking(&g_sem);
        for (uint8_t ch = 0; ch < N_UNIT; ch++) {
            stream[ch + 1] = ((g_unit_state >> ch) & 1) << 15 
                           | g_unit_note[ch] << 8 
                           | g_unit_velocity[ch];
        }
        sem_release(&g_sem);
        
        // Mode settings (Word 17)
        stream[17] = sel_vco1.get_state() << 10 
                   | sel_vco2.get_state() << 8 
                   | sel_vcf.get_state() << 7 
                   | sel_lfo_wave.get_state() << 3 
                   | sel_lfo_target.get_state();
        
        // Parameters (Word 18-33)
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
        
        // 4. Send stream via SPI Sound
        for (uint8_t i = 0; i < STREAM_LENGTH; i++) {
            gpio_put(PIN_CS_SOUND, false);
            sleep_us(10);
            spi_write16_blocking(spi1, &stream[i], 1);
            sleep_us(10);
            gpio_put(PIN_CS_SOUND, true);
        }
        
        // Debug output (optional)
        // for (uint8_t i = 0; i < STREAM_LENGTH; i++) {
        //     printf("%04x", stream[i]);
        // }
        // printf("\n");
    }
    
    return 0;
}
