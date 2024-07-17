#ifndef SPI_74HC595_HPP
#define SPI_74HC595_HPP

#include "spi_device.hpp"

class SPI74HC165 : SPIDevice
{
public:
    SPI74HC165(spi_inst_t *spi, uint pin_cs);

private:
};

#endif