# openchronos-ng-elf project
# Copyright (C) 2017 Alexandre Pretyman

cmake_minimum_required(VERSION 3.2)

project(openchronos-ng-elf C)

set(openchronos_binary_filename "openchronos.elf")

set(source_files
    config.h
    messagebus.c
    openchronos.c
    boot.c
    modinit.c
    menu.c

    drivers/battery.c
    drivers/vti_as.c
    drivers/infomem.c
    drivers/buzzer.c
    drivers/display.c
    drivers/temperature.c
    drivers/rtca.c
    drivers/rtc_dst.c
    drivers/ports.c
    drivers/dsp.c
    drivers/radio.c
    drivers/vti_ps.c
    drivers/adc12.c
    drivers/timer.c
    drivers/pmm.c
    drivers/rf1a.c

    modules/battery.c
    modules/alarm.c
    modules/music.c
    modules/reset.c
    modules/tide.c
    modules/temperature.c
    modules/clock.c
    modules/otp.c
    modules/stopwatch.c
    modules/accelerometer.c
)
add_executable(${openchronos_binary_filename} ${source_files})
target_include_directories(${openchronos_binary_filename} PRIVATE .)


# Only needed for wireless update?
#set(openchronos_hex_filename "openchronos.txt")
#find_package(PythonInterp)
#add_custom_command(
#    TARGET
#      ${openchronos_binary_filename}
#    POST_BUILD
#    COMMAND
#      ${PYTHON_EXECUTABLE}
#      ${CMAKE_CURRENT_LIST_DIR}/tools/memory.py
#      -i ${openchronos_binary_filename}
#      -o ${openchronos_hex_filename}
#    BYPRODUCTS
#      ${openchronos_hex_filename}
#)

