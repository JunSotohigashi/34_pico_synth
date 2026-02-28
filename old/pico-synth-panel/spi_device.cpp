#include "spi_device.hpp"

SPIDevice::SPIDevice(spi_inst_t *spi, uint pin_cs)
    : spi(spi),
      pin_cs(pin_cs)
{

    gpio_set_function(pin_cs, GPIO_FUNC_SIO);
    gpio_set_dir(pin_cs, GPIO_OUT);
    gpio_put(pin_cs, true);
}

int SPIDevice::write_bytes(uint8_t *src, size_t len)
{
    gpio_put(pin_cs, false);
    sleep_us(50);
    int n = spi_write_blocking(spi, src, len);
    sleep_us(50);
    gpio_put(pin_cs, true);
    return n;
}

int SPIDevice::write_read_bytes(uint8_t *src, uint8_t *dst, size_t len)
{
    gpio_put(pin_cs, false);
    sleep_us(50);
    int n = spi_write_read_blocking(spi, src, dst, len);
    sleep_us(50);
    gpio_put(pin_cs, true);
    return n;
}

int SPIDevice::read_bytes(uint8_t *dst, size_t len)
{
    gpio_put(pin_cs, false);
    sleep_us(50);
    int n = spi_read_blocking(spi, 0, dst, len);
    sleep_us(50);
    gpio_put(pin_cs, true);
    return n;
}