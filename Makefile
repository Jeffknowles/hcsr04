CC = gcc
CFLAGS += \
	-std=c99 \
	-w \
	-Wall \
	-D_BSD_SOURCE \
	-Wp,-MMD,$(dir $@).$(notdir $@).d \
	-Wp,-MT,$@ \
	-I. \
	-O2 \
	-mtune=cortex-a8 \
	-march=armv7-a \
	-lm \

LDLIBS += \
	-lpthread \

LEDSCAPE_OBJS = ledscape.o util.o

PRU_ASM = pasm
DTC = dtc

all: hcsr04-00A0.dtbo hcsr04.bin hcsr04

hcsr04-00A0.dtbo: hcsr04.dts
	@echo "\n>> Compiling Driver"
	$(DTC) -O dtb -o hcsr04-00A0.dtbo -b 0 -@ hcsr04.dts

hcsr04.bin: hcsr04.p
	@echo "\n>> Generating PRU binary"
	$(PRU_ASM) -b hcsr04.p


hcsr04: lednetwork.c
	@echo "\n>> spiking lednetwork example"
	$(CC) $(CFLAGS) -c -o lednetwork.o lednetwork.c
	$(CC) -lpthread -lprussdrv -o lednetwork lednetwork.o PixelBone/ledscape.o PixelBone/pru.o -lm

pwm_test: pwmtest.c
	@echo "\n>> Compiling pwm example"
	$(CC) $(CFLAGS) -c -o pwmtest.o pwmtest.c BBBIOlib/BBBio_lib/libBBBio.a
	$(CC) -pwmtest pwmtest.o  -L BBBIOlib/BBBio_lib/ -lBBBio


rgb-test: rgb-test.c
	@echo "\n>> Compiling RGB example"
	$(CC) $(CFLAGS) -c -o rgb-test.o ledscape.o rgb-test.c
	$(CC) -lpthread -lprussdrv -o rgb-test rgb-test.o ledscape.o -lm 
# hcsr04dev: hcsr04.c
# 	@echo "\n>> Compiling HC-SR04 example dev"
# 	$(CC) $(CFLAGS) -Irdev/ -c -o  hcsr04.o hcsr04.c
# 	$(CC) -lpthread -o hcsr04dev hcsr04.o  -lm

clean:
	rm -rf hcsr04 hcsr04.o hcsr04.bin hcsr04-00A0.dtbo 

install: hcsr04-00A0.dtbo hcsr04.bin hcsr04
	cp hcsr04-00A0.dtbo /lib/firmware
	echo hcsr04 > /sys/devices/bone_capemgr.9/slots
	cat /sys/devices/bone_capemgr.9/slots