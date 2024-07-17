#include "spi_74hc165.hpp"

SPI74HC165::SPI74HC165(spi_inst_t *spi, uint pin_cs)
    : SPIDevice(spi, pin_cs)
{
}