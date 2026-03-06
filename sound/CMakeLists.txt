cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

set(PICO_BOARD pico CACHE STRING "Board type")
include(../pico_sdk_import.cmake)

project(pico-synth-sound C CXX ASM)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

foreach(unit RANGE 0 7)
    math(EXPR unit_1 "${unit} * 2")
    math(EXPR unit_2 "${unit} * 2 + 1")

    add_executable(pico-synth-sound${unit}
        src/main.cpp
        src/voice.cpp
        src/oscillator.cpp
        src/filter.cpp
    )

    target_compile_definitions(pico-synth-sound${unit} PRIVATE
        UNIT_1=${unit_1}
        UNIT_2=${unit_2}
    )

    target_include_directories(pico-synth-sound${unit} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(pico-synth-sound${unit}
        pico_stdlib
        pico_multicore
        pico_rand
        hardware_clocks
        hardware_interp
        hardware_pwm
        hardware_spi
        hardware_irq
        hardware_timer
    )

    pico_set_program_name(pico-synth-sound${unit} "pico-synth-sound${unit}")
    pico_set_program_version(pico-synth-sound${unit} "0.2")
    pico_enable_stdio_uart(pico-synth-sound${unit} 0)
    pico_enable_stdio_usb(pico-synth-sound${unit} 1)
    pico_add_extra_outputs(pico-synth-sound${unit})
endforeach()
