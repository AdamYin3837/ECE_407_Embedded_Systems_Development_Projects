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
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
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

# Pull in Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(Project1 C CXX ASM)

pico_sdk_init()

# Add executable. 
# Includes Project1.c, ws2812.pio, AND encoder.pio
add_executable(Project1 
    Project1.c 
    encoder.c
    rgb_led.c
    ws2812_led.c
    game.c
    meter.c
    wheel.c
    ws2812.pio
)

pico_set_program_name(Project1 "Project1")
pico_set_program_version(Project1 "0.1")

pico_enable_stdio_uart(Project1 1)
pico_enable_stdio_usb(Project1 0)

# Add standard libraries
target_link_libraries(Project1
        pico_stdlib
        hardware_pwm
        hardware_pio
        hardware_gpio
        hardware_irq
        hardware_sync
        hardware_clocks
)

# Generate headers for BOTH PIO files
pico_generate_pio_header(Project1 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_include_directories(Project1 PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(Project1)
