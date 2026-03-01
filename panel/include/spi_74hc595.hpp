#pragma once

#include "spi_device.hpp"
#include <cstdint>

class SPI74HC595 : public SPIDevice {
private:
    uint8_t value_8bit_;
    
public:
    SPI74HC595(spi_inst_t* spi, uint pin_cs) 
        : SPIDevice(spi, pin_cs), value_8bit_(0) {
        put_8bit(0);
    }
    
    // Set all 8 bits at once
    void put_8bit(uint8_t value) {
        value_8bit_ = value;
        write_bytes(&value_8bit_, 1);
    }
    
    // Set individual pin (0-7)
    void put(uint8_t pin, bool value) {
        if (value) {
            value_8bit_ |= (1 << pin);
        } else {
            value_8bit_ &= ~(1 << pin);
        }
        write_bytes(&value_8bit_, 1);
    }
    
    // Get current value
    uint8_t get_value() const {
        return value_8bit_;
    }
};
