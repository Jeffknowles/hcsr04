#/bin/sh

cd PixelBone
make
cd ..
cp PixelBone/ws281x.bin ./
make