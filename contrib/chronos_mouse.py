#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Control mouse in X11 using Chronos watch.
# Requires xdotool for clicking.
#   * - left button click
#   # - middle button click
#   up key - right mouse click
#
# Original script (getting acceleration data) information below.
# 
# Mouse control added by Igor Brkic <igor@hyperglitch.com>
# 2010/02/06
#


#Get acceleration data from Chronos watch.
#Taken from info posted at: http://e2e.ti.com/support/microcontrollers/msp43016-bit_ultra-low_power_mcus/f/166/t/32714.aspx
#x, y, and z values may not be "in order". The first three bytes from the packet
#of data sent from the watch seemed to be different than what was listed
#on the page, though the datatype byte was in the correct place. So YMMV.
#
#Written by Sean Brewer (seabre)
#seabre986@gmail.com
#


import serial
import array
import os
import time

from Xlib import X, display


def startAccessPoint():
    return array.array('B', [0xFF, 0x07, 0x03]).tostring()

def accDataRequest():
    return array.array('B', [0xFF, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00]).tostring()

#Open COM port 6 (check your system info to see which port
#yours is actually on.)
#argments are 5 (COM6), 115200 (bit rate), and timeout is set so
#the serial read function won't loop forever.
ser = serial.Serial("/dev/ttyACM0",115200,timeout=1)

#Start access point
ser.write(startAccessPoint())

#setup X11
d = display.Display()
s = d.screen()
root = s.root

x=d.screen().width_in_pixels/2
y=d.screen().height_in_pixels/2
root.warp_pointer(x, y)
d.sync()

speed=3

last_button=1

while True:
    #Send request for acceleration data
    ser.write(accDataRequest())
    accel = ser.read(7)


    if last_button<0: 
        if ord(accel[6])==17:
            last_button=100
            button=1
        elif ord(accel[6])==33:
            last_button=100
            button=2
        elif ord(accel[6])==49:
            last_button=100
            button=3

        if last_button==100:
            os.system("xdotool click "+str(button))

    last_button-=1


    if accel[0]!=0:
        if ord(accel[1])>100:
            addx=-(255-ord(accel[1]))**1.3/speed
        else:
            addx=ord(accel[1])**1.3/speed

        if ord(accel[0])>100:
          addy=-(255-ord(accel[0]))**1.3/speed
        else:
          addy=ord(accel[0])**1.3/speed

        x+=addx
        y+=addy

        root.warp_pointer(x, y)
        d.sync()

ser.close()

