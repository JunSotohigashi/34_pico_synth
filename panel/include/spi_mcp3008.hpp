#pragma once

#include "spi_device.hpp"
#include <cstdint>

class SPIMCP3008 : private SPIDevice {
public:
    SPIMCP3008(spi_inst_t* spi, uint pin_cs) 
        : SPIDevice(spi, pin_cs) {}
    
    // Read single-ended channel (0-7)
    uint16_t read(uint8_t channel) {
        // MCP3008 single mode: start bit + single/diff + D2 D1 D0
        uint8_t tx[3] = {
            0x01, 
            static_cast<uint8_t>((channel | 0b1000) << 4), 
            0x00
        };
        uint8_t rx[3];
        write_read_bytes(tx, rx, 3);
        
        // Result is 10 bits: [rx[1] low 2 bits][rx[2] all 8 bits]
        uint16_t result = ((rx[1] & 0x03) << 8) | rx[2];
        return result;
    }
};
