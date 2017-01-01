# openchronos-ng-elf project
#
# Copyright (C) 2016 Benjamin Sølberg <benjamin.soelberg@gmail.com>
# Copyright (C) 2017 Alexandre Pretyman
#
# http://github.com/BenjaminSoelberg/openchronos-ng-elf
#
# This file is part of openchronos-ng.
#
# openchronos-ng is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# openchronos-ng is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
    modules/hashutils.c
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

