#/bin/sh

# disable hdmi
sudo cp /boot/uEnv.txt /boot/uEnv.txt.backup
sudo sed -i 's/#cape_disable=capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN$/cape_disable=capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN/g' /boot/uEnv.txt



cd PixelBone
make
cd ..
cp PixelBone/ws281x.bin ./
make install
dtc -O dtb -o hcsr04-00A0.dtbo -b 0 -@ hcsr04.dts
cd BBBIOlib
make
cd overlay
make install
cd ../../
