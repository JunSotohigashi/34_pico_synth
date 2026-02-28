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
    uart_write_blocking(uart_, reinterpret_cast<const uint8_t*>(tx_buffer_), len);
}

bool UartMidi::is_writable() const {
    return uart_is_writable(uart_);
}

} // namespace keyboard
