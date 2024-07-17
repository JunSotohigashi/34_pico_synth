#include "spi_mcp3008.hpp"

SPIMCP3008::SPIMCP3008(spi_inst_t *spi, uint pin_cs)
    : SPIDevice(spi, pin_cs)
{
}