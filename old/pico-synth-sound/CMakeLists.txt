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

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()
        
project(pico-synth-sound C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

foreach(unit RANGE 0 7)
        math(EXPR unit_1 "${unit} * 2 + 1")
        math(EXPR unit_2 "${unit} * 2 + 2")
        message("UNIT${unit}: ${unit_1}, ${unit_2}")
        
        # Add executable. Default name is the project name, version 0.1
        
        add_executable(pico-synth-sound${unit}
                pico-synth-sound.cpp
                voice.cpp
                oscillator.cpp
                envelope_generator.cpp
                filter.cpp)

        target_compile_definitions(pico-synth-sound${unit} PRIVATE
                UNIT_1=${unit_1}
                UNIT_2=${unit_2})
        
        pico_set_program_name(pico-synth-sound${unit} "pico-synth-sound${unit}")
        pico_set_program_version(pico-synth-sound${unit} "0.1")
        
        # Modify the below lines to enable/disable output over UART/USB
        pico_enable_stdio_uart(pico-synth-sound${unit} 0)
        pico_enable_stdio_usb(pico-synth-sound${unit} 1)
        
        # Add the standard library to the build
        target_link_libraries(pico-synth-sound${unit}
                pico_stdlib
                pico_rand
                pico_multicore)
        
        # Add the standard include files to the build
        target_include_directories(pico-synth-sound${unit} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}
          ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
        )
        
        # Add any user requested libraries
        target_link_libraries(pico-synth-sound${unit}
                hardware_clocks
                hardware_divider
                hardware_interp
                hardware_pwm
                hardware_spi
                )
        
        pico_add_extra_outputs(pico-synth-sound${unit})
    
endforeach()