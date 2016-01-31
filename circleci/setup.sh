#!/bin/bash

#Setup script to download and build the TI open source MSP430 elf GCC compiler
CACHE=$HOME/cached
GCC_HOME=$CACHE/msp430-elf-gcc

mkdir -p $CACHE

mkdir -p ~/temp
cd ~/temp

if [ ! -d $GCC_HOME ]; then
  echo Downloading msp430 elf gcc compiler source...
  wget https://github.com/BenjaminSoelberg/msp430-elf/archive/sources.zip -O msp430-elf-sources.zip
  unzip msp430-elf-sources.zip

  echo Configuring msp-430-elf-gcc...
  cd msp430-elf-sources
  mkdir -p msp430-gcc-obj
  cd  msp430-gcc-obj
  ../tools/configure --prefix=$GCC_HOME --target=msp430-elf --enable-languages=c,c++ --disable-itcl --disable-tk --disable-tcl --disable-libgui --disable-gdbtk

  echo Compiling msp-430-elf-gcc...
  make

  echo Installing msp-430-elf-gcc...
  make install
else
  echo MSP430 elf gcc compiler already exists, no need to install it.
fi

if [ ! -f $GCC_HOME/include/msp430.h ]; then
  echo Downloading msp430 includes...
  wget https://github.com/BenjaminSoelberg/msp430-elf/archive/gcc_rh.zip -O msp430-elf-gcc_rh.zip
  unzip msp430-elf-gcc_rh.zip
  cp -rf msp430-elf-gcc_rh/include/* $GCC_HOME/include/
else
  echo MSP430 includes already exists, no need to install it.
fi
