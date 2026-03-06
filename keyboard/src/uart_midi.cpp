/**
 * @file uart_midi.cpp
 * @brief UART MIDI transmitter implementation
 */

#include "uart_midi.hpp"
#include "hardware/gpio.h"

namespace keyboard {

UartMidi::UartMidi(uart_inst_t* uart, uint tx_pin, uint rx_pin, uint32_t baud_rate)
    : uart_(uart)
{
    uart_init(uart_, baud_rate);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
}

void UartMidi::send(const MidiMessage& msg) {
    uint8_t len = format_midi_hex(msg, tx_buffer_);
    const uint8_t* data = reinterpret_cast<const uint8_t*>(tx_buffer_);
    
    // Non-blocking: write as much as possible without stalling
    for (uint8_t i = 0; i < len; ++i) {
        if (uart_is_writable(uart_)) {
            uart_putc_raw(uart_, data[i]);
        }
        // If FIFO full, silently skip (low-level retry in OS)
    }
}

bool UartMidi::is_writable() const {
    return uart_is_writable(uart_);
}

} // namespace keyboard
