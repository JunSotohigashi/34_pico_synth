#include "spi_mcp3008.hpp"

SPIMCP3008::SPIMCP3008(spi_inst_t *spi, uint pin_cs)
    : SPIDevice(spi, pin_cs)
{
}

uint16_t SPIMCP3008::read(uint8_t channel)
{
    // read on single mode
    uint8_t tx[3] = {0x01, (uint8_t)((channel | 0b1000) << 4), 0x00};
    uint8_t rx[3];
    write_read_bytes(tx, rx, 3);
    uint16_t result = (rx[1] << 8 | rx[2]) & 0b1111111111;
    return result;
}