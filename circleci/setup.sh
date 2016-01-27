
if [ ! -d ~/cache/msp430-elf-gcc ]; then exit; fi
#!/bin/bash
mkdir -p ~/cached

if [ ! -d ~/cached/msp430-elf-gcc ]; then exit 0; fi

echo Downloading msp-430-elf-gcc...
mkdir -p ~/temp
cd ~/temp
wget https://github.com/BenjaminSoelberg/msp430-elf/archive/sources.zip -O msp430-elf-sources.zip
unzip msp430-elf-sources.zip

echo Configuring msp-430-elf-gcc...
cd ~/temp/msp430-elf-sources
mkdir -p msp430-gcc-obj
cd  msp430-gcc-obj
../tools/configure --prefix=/home/ubuntu/cached/msp430-elf-gcc --target=msp430-elf --enable-languages=c,c++ --disable-itcl --disable-tk --disable-tcl --disable-libgui --disable-gdbtk

echo Compiling msp-430-elf-gcc...
make

echo Installing msp-430-elf-gcc...
make install

#rm -rf ~/cached
