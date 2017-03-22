#/bin/sh

export SLOTS=/sys/devices/bone_capemgr.9/slots
export PINS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pins
export PINMUX=/sys/kernel/debug/pinctrl/44e10800.pinmux/pinmux-pins
export PINGROUPS=/sys/kernel/debug/pinctrl/44e10800.pinmux/pingroups
 echo BB-ADC > /sys/devices/bone_capemgr.*/slots  

echo hcsr04 > $SLOTS

cat $SLOTS

