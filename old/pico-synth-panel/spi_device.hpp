#ifndef SPI_DEVICE_HPP
#define SPI_DEVICE_HPP

#include "pico/stdlib.h"
#include "hardware/spi.h"

class SPIDevice
{
public:
    SPIDevice(spi_inst_t *spi, uint pin_cs);

protected:
    int write_bytes(uint8_t *src, size_t len);
    int write_read_bytes(uint8_t *src, uint8_t *dst, size_t len);
    int read_bytes(uint8_t *dst, size_t len);

private:
    spi_inst_t *spi;
    uint pin_cs;
};

#endif