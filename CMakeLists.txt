cmake_minimum_required(VERSION 3.3)
project(openchronos_ng_elf)

# --- start ---
INCLUDE(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME eCos)

# specify the cross compiler
CMAKE_FORCE_C_COMPILER(msp430-elf-gcc GNU)

# --- end ---

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(~/msp430-elf-gcc/include)

include_directories(.)
include_directories(drivers)
include_directories(modules)

set(SOURCE_FILES
    drivers/adc12.c
    drivers/adc12.h
    drivers/battery.c
    drivers/battery.h
    drivers/buzzer.c
    drivers/buzzer.h
    drivers/display.c
    drivers/display.h
    drivers/dsp.c
    drivers/dsp.h
    drivers/infomem.c
    drivers/infomem.h
    drivers/pmm.c
    drivers/pmm.h
    drivers/ports.c
    drivers/ports.h
    drivers/radio.c
    drivers/radio.h
    drivers/rf1a.c
    drivers/rf1a.h
    drivers/rtc_dst.c
    drivers/rtc_dst.h
    drivers/rtca.c
    drivers/rtca.h
    drivers/rtca_now.h
    drivers/temperature.c
    drivers/temperature.h
    drivers/timer.c
    drivers/timer.h
    drivers/utils.h
    drivers/vti_as.c
    drivers/vti_as.h
    drivers/vti_ps.c
    drivers/vti_ps.h
    modules/accelerometer.c
    modules/alarm.c
    modules/battery.c
    modules/clock.c
    modules/music.c
    modules/otp.c
    modules/reset.c
    modules/stopwatch.c
    modules/temperature.c
    modules/tide.c
    boot.c
    config.h
    menu.c
    menu.h
    messagebus.c
    messagebus.h
    modinit.c
    modinit.h
    openchronos.c
    openchronos.h)

add_executable(openchronos_ng_elf ${SOURCE_FILES})