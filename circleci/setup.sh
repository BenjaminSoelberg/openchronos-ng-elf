rm -rf ~/cached

mkdir -p ~/cached
mkdir -p ~/temp
cd ~/temp
pwd
wget https://github.com/BenjaminSoelberg/msp430-elf/archive/sources.zip -O msp430-elf.zip
unzip msp430-elf.zip

rm -rf ~/cached
