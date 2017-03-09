
#ifndef _hcsr04_h_
#define _hcsr04_h_

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stdlib.h> 
#include <tgmath.h> 
#include <inttypes.h>
#include <sys/time.h>  
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include "PixelBone/ledscape.h"
#include <prussdrv.h> 
#include <pruss_intc_mapping.h>
#define prunum 1



const double minIPI = 0.1; // minimum interping interval in miliseconds
const double sense_thresh_i = 100; // threshold where responses turn on
// const int pingPin = 11; // trigger for sonar pulses
// const int echoPin = 12; // return for sonar pulses
// const int phonePin1 = 9; //
// const int phonePin2 = 10;
// const int dialPin = 5;  // analog pin for the dial
//const int modePins[2] = {3, 4}; // pins for the 3way mode switch
//const int buttonPin = 2;  // pin for the tigger button
const int sensory_factor = 10;
const bool printout = false;
const bool pong_only_in_range = true;
	

const double thresh = 30;
const double k = 1; // magnitude of the leak


// connection settings - declare connections between neurons
#define maxCon 40
#define nch 1000 // number of neurons
#define num_pixels 128



double doPing(unsigned int *pruData);


#endif
