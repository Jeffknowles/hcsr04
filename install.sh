#/bin/sh

cd PixelBone
make
cd ..
cp PixelBone/ws281x.bin ./
make install
dtc -O dtb -o hcsr04-00A0.dtbo -b 0 -@ hcsr04.dts
