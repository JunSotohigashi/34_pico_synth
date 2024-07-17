#include "spi_device.hpp"

SPIDevice::SPIDevice(spi_inst_t *spi, uint pin_cs)
    : spi(spi),
      pin_cs(pin_cs)
{
}