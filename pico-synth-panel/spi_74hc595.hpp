#ifndef SPI_74HC165_HPP
#define SPI_74HC165_HPP

#include "spi_device.hpp"

class SPI74HC595 : SPIDevice
{
public:
    SPI74HC595(spi_inst_t *spi, uint pin_cs);
    void put_8bit(uint8_t value);
    void put(uint8_t pin, bool value);

private:
    uint8_t value_8bit;
};

#endif