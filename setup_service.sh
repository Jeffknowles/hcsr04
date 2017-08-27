

cp start_script.sh /usr/bin/sln_ss.sh
chmod u+x /usr/bin/sln_ss.sh


rm /opt/sln -r
mkdir /opt/sln
cp ./hcsr04.bin /opt/sln
cp ws281x.bin /opt/sln
cp lednetwork /opt/sln

# cp -r ./ /opt/sln
chmod u+x /opt/sln -R
chmod +x /opt/sln/lednetwork
chmod +x /opt/sln/install-driver.sh
cp sln.service /lib/systemd/sln.service
ln /lib/systemd/sln.service /etc/systemd/system/sln.service



echo "cape_enable=bone_capemgr.enable_partno=BBBIO-JKPWM" >> /boot/uboot/uEnv.txt
echo "cape_enable=bone_capemgr.enable_partno=BB-ADC" >> /boot/uboot/uEnv.txt
echo "cape_enable=bone_capemgr.enable_partno=hcsr04" >> /boot/uboot/uEnv.txt


# echo "CAPE=BBIO-JKPWM:00A0" >> /


cp ./dbto_blank /etc/initramfs-tools/hooks/dtbo
chmod +x /etc/initramfs-tools/hooks/dtbo
/opt/scripts/tools/developers/update_initrd.sh

systemctl daemon-reload
systemctl start sln.service
systemctl enable sln.service
systemctl status sln.service