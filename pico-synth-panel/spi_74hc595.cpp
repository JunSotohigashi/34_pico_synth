#include "spi_74hc595.hpp"

SPI74HC595::SPI74HC595(spi_inst_t *spi, uint pin_cs)
    : SPIDevice(spi, pin_cs),
      value_8bit(0)
{
    put_8bit(0);
}

void SPI74HC595::put_8bit(uint8_t value)
{
    // set 8bit output
    value_8bit = value;
    write_bytes(&value_8bit, 1);
}

void SPI74HC595::put(uint8_t pin, bool value)
{
    if (value)
    {
        value_8bit |= (1 << pin);
    }
    else
    {
        value_8bit &= ~(1 << pin);
    }
}