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
        modules/alarm.c
        modules)

add_msp430_executable(openchronos "" ${SOURCE_FILES})