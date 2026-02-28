# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-synth C CXX ASM)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

option(BUILD_BOOTSTRAP_PICO_SYNTH "Build bootstrap pico-synth target" ON)

if(BUILD_BOOTSTRAP_PICO_SYNTH)
    add_executable(pico-synth pico-synth.cpp)
    pico_set_program_name(pico-synth "pico-synth")
    pico_set_program_version(pico-synth "0.1")
    pico_generate_pio_header(pico-synth ${CMAKE_CURRENT_LIST_DIR}/blink.pio)
    pico_enable_stdio_uart(pico-synth 0)
    pico_enable_stdio_usb(pico-synth 1)
    target_link_libraries(pico-synth
        pico_stdlib
        hardware_spi
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks)
    target_include_directories(pico-synth PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})
    pico_add_extra_outputs(pico-synth)
endif()

set(OLD_DIR ${CMAKE_CURRENT_LIST_DIR}/old)
set(KEYBOARD_DIR ${OLD_DIR}/pico-synth-keyboard)
set(PANEL_DIR ${OLD_DIR}/pico-synth-panel)
set(SOUND_DIR ${OLD_DIR}/pico-synth-sound)

add_executable(pico-synth-keyboard
    ${KEYBOARD_DIR}/pico-synth-keyboard.c
    ${KEYBOARD_DIR}/usb_descriptors.c)
pico_set_program_name(pico-synth-keyboard "pico-synth-keyboard")
pico_set_program_version(pico-synth-keyboard "0.1")
pico_enable_stdio_uart(pico-synth-keyboard 0)
pico_enable_stdio_usb(pico-synth-keyboard 0)
target_link_libraries(pico-synth-keyboard
    pico_stdlib
    pico_multicore
    tinyusb_device
    tinyusb_board)
target_include_directories(pico-synth-keyboard PRIVATE
    ${KEYBOARD_DIR}
    ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(pico-synth-keyboard)

add_executable(pico-synth-panel
    ${PANEL_DIR}/pico-synth-panel.cpp
    ${PANEL_DIR}/spi_device.cpp
    ${PANEL_DIR}/spi_mcp3008.cpp
    ${PANEL_DIR}/spi_74hc595.cpp)
pico_set_program_name(pico-synth-panel "pico-synth-panel")
pico_set_program_version(pico-synth-panel "0.1")
pico_enable_stdio_uart(pico-synth-panel 0)
pico_enable_stdio_usb(pico-synth-panel 1)
target_link_libraries(pico-synth-panel
    pico_stdlib
    pico_multicore
    hardware_spi)
target_include_directories(pico-synth-panel PRIVATE
    ${PANEL_DIR}
    ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(pico-synth-panel)

foreach(unit RANGE 0 7)
    math(EXPR unit_1 "${unit} * 2 + 1")
    math(EXPR unit_2 "${unit} * 2 + 2")

    add_executable(pico-synth-sound${unit}
        ${SOUND_DIR}/pico-synth-sound.cpp
        ${SOUND_DIR}/voice.cpp
        ${SOUND_DIR}/oscillator.cpp
        ${SOUND_DIR}/envelope_generator.cpp
        ${SOUND_DIR}/filter.cpp)

    target_compile_definitions(pico-synth-sound${unit} PRIVATE
        UNIT_1=${unit_1}
        UNIT_2=${unit_2})

    pico_set_program_name(pico-synth-sound${unit} "pico-synth-sound${unit}")
    pico_set_program_version(pico-synth-sound${unit} "0.1")
    pico_enable_stdio_uart(pico-synth-sound${unit} 0)
    pico_enable_stdio_usb(pico-synth-sound${unit} 1)

    target_link_libraries(pico-synth-sound${unit}
        pico_stdlib
        pico_rand
        pico_multicore
        hardware_clocks
        hardware_divider
        hardware_interp
        hardware_pwm
        hardware_spi)

    target_include_directories(pico-synth-sound${unit} PRIVATE
        ${SOUND_DIR}
        ${CMAKE_CURRENT_LIST_DIR})

    pico_add_extra_outputs(pico-synth-sound${unit})
endforeach()

