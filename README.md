INTRODUCTION
============

Origin: [https://github.com/BenjaminSoelberg/openchronos-ng-elf](https://github.com/BenjaminSoelberg/openchronos-ng-elf)

Latest build status on Circle CI : [![Circle CI](https://circleci.com/gh/BenjaminSoelberg/openchronos-ng-elf.svg?style=svg)](https://circleci.com/gh/BenjaminSoelberg/openchronos-ng-elf)

Modular opensource firmware for the TI eZ430 Chronos.

openchronos-ng is a major rework of openchronos. Compared to openchronos it has the following features:

* system message bus for system<->module communication.
* hardware RTC timekeeping (no more clock inaccuracy).
* rework of timer and ports drivers.
* implementation of a module system (drop in applications).
* rework of the display routines.
* rework of the menu system.

The firmware code is also conceptually simpler and smaller which leaves room for more modules (applications).

This repository is a fork of (seems unmaintained) :
[http://sourceforge.net/projects/openchronos-ng/](http://sourceforge.net/projects/openchronos-ng)

INSTALLATION
============

Linux/Ubuntu 64 bit installation preparation
---------------------------------------------
If you are running on a 64 bit Ubuntu system you need to install some packages first:

```sudo apt-get install lib32z1 lib32ncurses5 lib32stdc++6 python-urwid```

Compiler and debugger installation
----------------------------------

Download the open source MSP430 GCC compiler from TI here:

[http://www.ti.com/tool/msp430-gcc-opensource](http://www.ti.com/tool/msp430-gcc-opensource)

**Please note that the installer is a 32 bit binary and will exit with no error if above dependencies isn't available on your system.**

Execute :

```
chmod +x msp430-gcc-full-linux-installer-3.5.0.0.run
./msp430-gcc-full-linux-installer-3.5.0.0.run
sudo apt-get install mspdebug
```

Add this to your .profile
```
export MSP430_TI=~/ti/gcc
export PATH=$PATH:$MSP430_TI/bin
```

Run the following command to add exports to your current shell (including the dot):
```
. ~/.profile
```

Support package
----------------

Install the following support packages from TI in order to be able to download new firmware to the watch using the wireless radio:

Linux: [http://www.ti.com/lit/zip/slac388](http://www.ti.com/lit/zip/slac388)

```
unzip slac388c.zip
chmod +x Chronos-Setup
./Chronos-Setup
```

Windows:[http://www.ti.com/lit/zip/slac341](http://www.ti.com/lit/zip/slac341)


Compiling the firmware
----------------------
Clone it from GitHub:
```
git clone https://github.com/BenjaminSoelberg/openchronos-ng-elf
```

Setup which modules to compile with:
```
cd openchronos-ng-elf
make config
```

Deselect anything saying Experimental, **very important as they don't compile yet**

Build with:
```make clean && make```

The newly build firmware is in the binary file *openchronos.elf*

Flashing the firmware using mspdebug
---------------------------------------
Connect the watch module to the USB FET.

Program it using mspdebug:
```mspdebug rf2500 "prog openchronos.elf"```

Disconnect the watch module and the watch should display BOOT.
Press any button except the backlight and you should be up and running the new firmware.


Boot Menu & RFBSL (flashing over RF)
------------------------------------

In openchronos-ng, the watch no longer boots directly into the
main menu anymore. When the watch is reset (or you put the battery
for the first time), you have the choice to either enter flash mode
(press # button) or continue into main menu (press any other button).

Use the TI software to flash using RF (use *openchronos.txt*).

Usefull links
-------------

If you need to reinstall the original firmware you can download a flash tool here (using windows XP):

[http://www.bm-innovations.com/index.php/ez430-chronos](http://www.bm-innovations.com/index.php/ez430-chronos)

Data sheet for the CC430F6137 used in the watch:

[http://www.ti.com/product/CC430F6137/technicaldocuments](http://www.ti.com/product/CC430F6137/technicaldocuments)

eZ430 Chronos wiki:

[http://processors.wiki.ti.com/index.php/EZ430-Chronos](http://processors.wiki.ti.com/index.php/EZ430-Chronos)
