#!/bin/bash

#Setup script to download and build the TI open source MSP430 elf GCC compiler
set CACHE=$HOME/cached
set GCC_HOME=$CACHE/msp430-elf-gcc

mkdir -p ~/cached
mkdir -p ~/temp
cd ~/temp

if [ ! -d $GCC_HOME/include ]; then
  echo Downloading msp430 includes...
  wget https://github.com/BenjaminSoelberg/msp430-elf/archive/gcc_rh.zip -O msp430-elf-gcc_rh.zip
  unzip msp430-elf-gcc_rh.zip
  cp -r msp430-elf-gcc_rh/* $GCC_HOME
fi

if [! -d $GCC_HOME ]; then
  echo Downloading msp430 elf gcc compiler source...

  wget https://github.com/BenjaminSoelberg/msp430-elf/archive/sources.zip -O msp430-elf-sources.zip
  unzip msp430-elf-sources.zip

  echo Configuring msp-430-elf-gcc...
  cd msp430-elf-sources
  mkdir -p msp430-gcc-obj
  cd  msp430-gcc-obj
  ../tools/configure --prefix=$HOME/cached/msp430-elf-gcc --target=msp430-elf --enable-languages=c,c++ --disable-itcl --disable-tk --disable-tcl --disable-libgui --disable-gdbtk

  echo Compiling msp-430-elf-gcc...
  make

  echo Installing msp-430-elf-gcc...
  make install
fi
