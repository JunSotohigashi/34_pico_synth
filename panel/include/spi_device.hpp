#pragma once

#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <cstdint>

class SPIDevice {
protected:
    spi_inst_t* spi_;
    uint pin_cs_;
    
public:
    SPIDevice(spi_inst_t* spi, uint pin_cs) 
        : spi_(spi), pin_cs_(pin_cs) {
        gpio_set_function(pin_cs_, GPIO_FUNC_SIO);
        gpio_set_dir(pin_cs_, GPIO_OUT);
        gpio_put(pin_cs_, true);
    }
    
    int write_bytes(const uint8_t* src, size_t len) {
        gpio_put(pin_cs_, false);
        sleep_us(50);
        int n = spi_write_blocking(spi_, src, len);
        sleep_us(50);
        gpio_put(pin_cs_, true);
        return n;
    }
    
    int write_read_bytes(const uint8_t* src, uint8_t* dst, size_t len) {
        gpio_put(pin_cs_, false);
        sleep_us(50);
        int n = spi_write_read_blocking(spi_, src, dst, len);
        sleep_us(50);
        gpio_put(pin_cs_, true);
        return n;
    }
    
    int read_bytes(uint8_t* dst, size_t len) {
        gpio_put(pin_cs_, false);
        sleep_us(50);
        int n = spi_read_blocking(spi_, 0, dst, len);
        sleep_us(50);
        gpio_put(pin_cs_, true);
        return n;
    }
};
