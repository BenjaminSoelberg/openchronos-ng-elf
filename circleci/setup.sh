rm -rf ~/cached

mkdir -p ~/cached
mkdir -p ~/temp/msp430-gcc-source/msp430-gcc-obj
cd ~/temp/msp430-gcc-source
wget https://github.com/BenjaminSoelberg/msp430-elf/archive/sources.zip -O msp430-elf.zip
unzip msp430-elf.zip >/dev/null
cd  msp430-gcc-obj
../tools/configure --prefix=~/msp430-elf-gcc --target=msp430-elf --enable-languages=c,c++ --disable-itcl --disable-tk --disable-tcl --disable-libgui --disable-gdbtk
make
#make install
rm -rf ~/cached
