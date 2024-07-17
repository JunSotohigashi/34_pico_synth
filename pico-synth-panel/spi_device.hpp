#ifndef SPI_DEVICE_HPP
#define SPI_DEVICE_HPP

#include "pico/stdlib.h"
#include "hardware/spi.h"

class SPIDevice
{
public:
    SPIDevice(spi_inst_t *spi, uint pin_cs);

private:
    spi_inst_t *spi;
    uint pin_cs;
};

#endif