cmake_minimum_required(VERSION 3.13)

project(pico-synth-panel C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.0.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Panel executable
add_executable(pico-synth-panel
    pico-synth-panel.cpp
)

# Include directories
target_include_directories(pico-synth-panel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(pico-synth-panel
    pico_stdlib
    pico_multicore
    hardware_uart
    hardware_adc
    hardware_spi
    hardware_gpio
)

# Enable USB output for debug
pico_enable_stdio_usb(pico-synth-panel 1)
pico_enable_stdio_uart(pico-synth-panel 0)

# Generate UF2 firmware
pico_add_uf2_output(pico-synth-panel)

message(STATUS "Panel Module: pico-synth-panel target added")

