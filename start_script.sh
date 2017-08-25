#/bin/sh

sh /opt/sln/install-driver.sh



export SLOTS=/sys/devices/bone_capemgr.9/slots
# export PINS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pins
# export PINMUX=/sys/kernel/debug/pinctrl/44e10800.pinmux/pinmux-pins
# export PINGROUPS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pingroups

echo BBBIO-JKPWM > $SLOTS
echo BB-ADC > $SLOTS
# echo BBIO
echo hcsr04 > $SLOTS


sleep 20


exec /opt/sln/letnetwork > /dev/null

