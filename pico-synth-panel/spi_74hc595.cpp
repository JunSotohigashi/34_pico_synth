#include "spi_74hc595.hpp"

SPI74HC595::SPI74HC595(spi_inst_t *spi, uint pin_cs)
    : SPIDevice(spi, pin_cs)
{
}