/**
 * @file main.cpp
 * @brief Keyboard module main entry point
 */

#include "config.hpp"
#include "uart_midi.hpp"
#include "usb_midi.hpp"
#include "keyboard_scanner.hpp"
#include "midi_protocol.hpp"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"

using namespace keyboard;

// Global instances
static UartMidi* g_uart_midi = nullptr;
static UsbMidi* g_usb_midi = nullptr;
static KeyboardScanner* g_scanner = nullptr;

// Inter-core communication
static queue_t g_midi_queue;

// Pedal state
static bool g_pedal_default = false;
static bool g_pedal_previous = false;

/**
 * @brief Core 1: Keyboard scanning
 */
void core1_main() {
    uint16_t scan_results[N_PIN_HIGH];
    
    while (true) {
        // Scan keyboard matrix
        g_scanner->scan(scan_results);
        
        // Process scan results
        uint8_t key_index = 0;
        uint8_t velocity = 0;
        KeyEvent event = g_scanner->process_scan(scan_results, key_index, velocity);
        
        if (event == KeyEvent::Press) {
            // Match note numbering with legacy keyboard implementation.
            uint8_t note = MIDI_BASE_NOTE + key_index;
            MidiMessage msg = MidiMessage::note_on(note, velocity);
            
            // Send to queue for Core 0 to transmit
            queue_try_add(&g_midi_queue, &msg);
        } else if (event == KeyEvent::Release) {
            uint8_t note = MIDI_BASE_NOTE + key_index;
            MidiMessage msg = MidiMessage::note_off(note);
            
            queue_try_add(&g_midi_queue, &msg);
        }
        
        // Check pedal state
        bool pedal_current = gpio_get(PIN_PEDAL);
        if (pedal_current != g_pedal_previous) {
            MidiMessage msg = (pedal_current != g_pedal_default)
                ? MidiMessage::sustain_on()
                : MidiMessage::sustain_off();
            
            queue_try_add(&g_midi_queue, &msg);
            g_pedal_previous = pedal_current;
        }
        
        sleep_ms(SCAN_INTERVAL_MS);
    }
}

/**
 * @brief Core 0: Main loop (USB MIDI and UART transmission)
 */
int main() {
    // Initialize USB MIDI (includes board_init and tusb_init)
    static UsbMidi usb_midi;
    usb_midi.init();
    g_usb_midi = &usb_midi;
    
    // Initialize pedal pin
    gpio_init(PIN_PEDAL);
    gpio_set_dir(PIN_PEDAL, GPIO_IN);
    gpio_pull_up(PIN_PEDAL);
    g_pedal_default = gpio_get(PIN_PEDAL);
    g_pedal_previous = g_pedal_default;
    
    // Initialize UART MIDI
    static UartMidi uart_midi(uart0, PIN_UART_TX, PIN_UART_RX, UART_BAUD_RATE);
    g_uart_midi = &uart_midi;
    
    // Initialize keyboard scanner
    static KeyboardScanner scanner;
    scanner.init();
    g_scanner = &scanner;
    
    // Initialize inter-core queue
    queue_init(&g_midi_queue, sizeof(MidiMessage), 32);
    
    // Launch Core 1 for keyboard scanning
    multicore_launch_core1(core1_main);
    
    // Core 0 main loop: Handle USB tasks and message transmission
    while (true) {
        // Process USB tasks
        g_usb_midi->task();
        
        // Send messages from queue
        while (!queue_is_empty(&g_midi_queue)) {
            MidiMessage msg;
            if (queue_try_remove(&g_midi_queue, &msg)) {
                // Send via both USB and UART
                g_usb_midi->send(msg);
                g_uart_midi->send(msg);
            }
        }
        
        // Receive USB MIDI from host and forward to UART
        if (g_usb_midi->available()) {
            MidiMessage msg;
            if (g_usb_midi->read(msg)) {
                g_uart_midi->send(msg);
            }
        }
        
        sleep_ms(1);
    }
    
    return 0;
}
