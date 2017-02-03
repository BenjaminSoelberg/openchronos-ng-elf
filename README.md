INTRODUCTION
============

Origin: [https://github.com/BenjaminSoelberg/openchronos-ng-elf](https://github.com/BenjaminSoelberg/openchronos-ng-elf)

Latest build status on Circle CI : [![Circle CI](https://circleci.com/gh/BenjaminSoelberg/openchronos-ng-elf.svg?style=svg)](https://circleci.com/gh/BenjaminSoelberg/openchronos-ng-elf)

Releases and snapshot, including firmware can be found here : [https://github.com/BenjaminSoelberg/openchronos-ng-elf/releases](https://github.com/BenjaminSoelberg/openchronos-ng-elf/releases)

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


Linux/Fedora 64 bit installation preparation
---------------------------------------------
If you are running on a 64 bit Fedora system you need to install some packages first:

```sudo dnf install ncurses-libs.i686 zlib.i686 libstdc++.i686 python-urwid```


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
or add it to your .bashrc.

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

Deselect anything saying *EXPERIMENTAL* as they are not fully functioning.

Build with:<br>
```make clean && make```

The newly build firmware is in the binary file *openchronos.elf* and intel format in *openchronos.txt*

Boot Menu
------------------------------------
In openchronos-ng, the watch no longer boots directly into the clock firmware.

To enter the BOOT menu you can either:
* Use the Reset menu
* Reinsert the battery

If the display shows BOOT you have successfully reset the watch and are now in the boot menu.
Press the backlight button to enter the wireless flash updater (RFBSL). Any other button will run the watch firmware.

Flashing the firmware using mspdebug
---------------------------------------
1) Connect the internal watch module to the USB FET module

2) Program it using mspdebug<br>
```make usb-install```

3) Disconnect the watch module and the watch should display BOOT<br>
Press any button except the backlight and you should be up and running the new firmware

*Please note that this method is slow but very useful if flashing over wireless fails.*

Flashing the firmware using wireless (RFBSL)
------------------------------------
1) Connect the USB CC11x1 module

2) Reset the watch using the menu or reinserting the battery

3) Program it using ChronosTool.py (Note that sudo might be required. Also repeat this step if it fails)<br>
```sudo make install```

4) Press Enter

5) Enter RFBSL by pressing the backlight button on the watch

6) Press any button except the backlight and you should be up and running the new firmware.

Please note:<br>
* RFBSL seems to fail a lot on some MacBooks (mine is a MacBook Pro Retina 2012)
* RFBSL seems to fail a lot for some if the watch battery is below 93%

Useful links
-------------

If you need to reinstall the original firmware you can download a flash tool here (using windows XP):

[http://www.bm-innovations.com/index.php/ez430-chronos](http://www.bm-innovations.com/index.php/ez430-chronos)

If you are on windows 7/8/10 and wish to update the watch via bm-innovations Flash tool then download the FET drivers 
(MSP430Drivers-1_00_00_01-windows-installer.exe confirmed working on Win10) from:

[http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430_FET_Drivers/latest/index_FDS.html](http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430_FET_Drivers/latest/index_FDS.html)

Data sheet for the CC430F6137 used in the watch:

[http://www.ti.com/product/CC430F6137/technicaldocuments](http://www.ti.com/product/CC430F6137/technicaldocuments)

eZ430 Chronos wiki:

[http://processors.wiki.ti.com/index.php/EZ430-Chronos](http://processors.wiki.ti.com/index.php/EZ430-Chronos)

Join us at Slack Team Channel:

[https://openchronos.slack.com](https://openchronos.slack.com)