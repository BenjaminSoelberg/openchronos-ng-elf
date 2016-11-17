cmake_minimum_required(VERSION 3.2)

project(openchronos_ng_elf)

set(SUPPORTED_DEVICES cc430f6137)
set(EXECUTABLE_NAME openchronos)

include(toolchains/Platform/msp430-gcc.cmake)
include(toolchains/toolchain-msp430-gcc-ti.cmake)

# where is the target environment
set(CMAKE_FIND_ROOT_PATH $ENV{MSP430_TI})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

include_directories(. drivers modules)

set(SOURCE_FILES
        openchronos.c
        config.h
        boot.c
        menu.c
        messagebus.c
        modinit.c
        drivers
        drivers/adc12.c
        drivers/battery.c
        drivers/buzzer.c
        drivers/display.c
        drivers/dsp.c
        drivers/infomem.c
        drivers/pmm.c
        drivers/ports.c
        drivers/radio.c
        drivers/rf1a.c
        drivers/rtc_dst.c
        drivers/rtca.c
        drivers/temperature.c
        drivers/timer.c
        drivers/vti_as.c
        modules
        modules/alarm.c
        modules/temperature.c
        modules/clock.c
        modules/stopwatch.c)

add_msp430_executable(openchronos "" ${SOURCE_FILES})