mkdir -p ~/cached

mkdir -p ~/temp
cd ~/temp
wget https://github.com/BenjaminSoelberg/msp430-elf/archive/sources.zip -O msp430-elf-sources.zip
unzip msp430-elf-sources.zip > unzip.out
tail unzip.out

cd ~/temp/msp430-elf-sources
mkdir -p msp430-gcc-obj
cd  msp430-gcc-obj
../tools/configure --prefix=/home/ubuntu/cached/msp430-elf-gcc --target=msp430-elf --enable-languages=c,c++ --disable-itcl --disable-tk --disable-tcl --disable-libgui --disable-gdbtk

make >make.out
tail make.out

make install>install.out
tail install.txt

#rm -rf ~/cached
