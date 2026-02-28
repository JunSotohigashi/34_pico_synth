# Keyboard module CMakeLists.txt

add_executable(pico-synth-keyboard
    src/main.cpp
    src/uart_midi.cpp
    src/usb_midi.cpp
    src/keyboard_scanner.cpp
    src/usb_descriptors.c
)

target_include_directories(pico-synth-keyboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pico-synth-keyboard
    pico_stdlib
    pico_multicore
    hardware_gpio
    hardware_uart
    tinyusb_device
    tinyusb_board
)

pico_set_program_name(pico-synth-keyboard "pico-synth-keyboard")
pico_set_program_version(pico-synth-keyboard "0.2")

# Disable stdio (conflicts with USB MIDI)
pico_enable_stdio_uart(pico-synth-keyboard 0)
pico_enable_stdio_usb(pico-synth-keyboard 0)

pico_add_extra_outputs(pico-synth-keyboard)
