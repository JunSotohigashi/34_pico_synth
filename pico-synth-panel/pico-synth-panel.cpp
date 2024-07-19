#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "spi_mcp3008.hpp"
#include "spi_74hc595.hpp"

// SPI Defines
#define SPI_PORT spi0
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS_ADC1 7
#define PIN_CS_ADC2 8
#define PIN_CS_SR1 2
#define PIN_CS_SR2 3
#define PIN_CS_SR3 4
#define PIN_CS_SR4 5
#define PIN_CS_SR5 6

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    SPIMCP3008 adc1(SPI_PORT, PIN_CS_ADC1);
    SPIMCP3008 adc2(SPI_PORT, PIN_CS_ADC2);
    SPI74HC595 led1(SPI_PORT, PIN_CS_SR1);
    SPI74HC595 led2(SPI_PORT, PIN_CS_SR2);
    SPI74HC595 led3(SPI_PORT, PIN_CS_SR3);
    SPI74HC595 led4(SPI_PORT, PIN_CS_SR4);
    SPI74HC595 led5(SPI_PORT, PIN_CS_SR5);
    
    led1.put_8bit(0b10101010);
    
    while (true)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            printf(">CHa%u: %4u\n", i, adc1.read(i));
        }
        for (uint8_t i = 0; i < 8; i++)
        {
            printf(">CHb%u: %4u\n", i, adc2.read(i));
        }
        // sleep_ms(10);
    }
}
