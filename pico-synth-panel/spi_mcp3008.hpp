#ifndef SPI_MCP3008_HPP
#define SPI_MCP3008_HPP

#include "spi_device.hpp"

class SPIMCP3008 : SPIDevice
{
public:
    SPIMCP3008(spi_inst_t *spi, uint pin_cs);

private:
};

#endif