#### Spikeing Led Network

Jeff Knowles
jeff.knowles@gmail.com	


This code runs a spiking neural network in C that interpretes input from enviromental sensors to drive visual and audio outputs. Runs on a Beaglebone Black, and executes PRU code to interact with sensors and drive LED light displays. Originally based on basic code for the HCS04 as well as a branch from PixelBone as it split off from LEDScape, making it easy to use one PRU for sensing and one to drive the lights.    



===



Hardware configuration:

	* TRIGGER		P8_12 gpio1[12] GPIO44	out	pulldown		Mode: 7 
	* ECHO			P8_11 gpio1[13] GPIO45	in	pulldown		Mode: 7 *** with R 1KOhm
	* GND			P9_1 or P9_2	GND
	* VCC			P9_5 or P9_6	VDD_5V
	
	Make sure there is a 1KOhm resistor between ECHO pin and your beaglebone
	as input pins are 3.3V while sensor emits 5V

### To build:
make
### To install driver:

	make install_spikinglednetwork.sh
	It will copy device tree driver to /lib/firmware and add it to the cape manager. Currently works for Pixelbone and 	HCSR04 communication. 
	
### To run:
Under construction: 

	

### Reference material:

- [HC-SR04 manual and datasheet](http://www.cytron.com.my/viewProduct.php?pcode=SN-HC-SR04&name=Ultrasonic%20Ranging%20Module)
- [TI PRU docs](http://processors.wiki.ti.com/index.php/Programmable_Realtime_Unit_Software_Development)
- [Introduction to the BeagleBone Black Device Tree](https://learn.adafruit.com/introduction-to-the-beaglebone-black-device-tree/overview)
- [Using a Beaglebone with an HC-SR04 sonar](http://teknoman117.wordpress.com/2013/04/30/using-a-beaglebone-with-an-hc-sr04-sonar/)
