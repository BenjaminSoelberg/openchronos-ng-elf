cmake_minimum_required(VERSION 3.3)

set(MSP430_TI $ENV{MSP430_TI})
project(openchronos_ng_elf)

# --- start ---
include(CMakeForceCompiler)

# this one is important
set(CMAKE_SYSTEM_NAME generic)

# specify the cross compiler
CMAKE_FORCE_C_COMPILER(msp430-elf-gcc GNU)

# where is the target environment
set(CMAKE_FIND_ROOT_PATH ${MSP430_TI})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# --- end ---

set(CMAKE_C_FLAGS "-mmcu=cc430f6137 -ffunction-sections -fdata-sections -Wall")

include_directories(${MSP430_TI}/include)

include_directories(.)
include_directories(drivers)
include_directories(modules)

add_executable(openchronos_ng_elf openchronos.c)