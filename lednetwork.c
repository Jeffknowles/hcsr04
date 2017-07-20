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


const double minIPI = 0.04; // minimum interping interval in miliseconds
const double maxIPI = 2;
const double sense_thresh_i = 400; // threshold where responses turn on
// const int pingPin = 11; // trigger for sonar pulses
// const int echoPin = 12; // return for sonar pulses
// const int phonePin1 = 9; //
// const int phonePin2 = 10;
// const int dialPin = 5;  // analog pin for the dial
//const int modePins[2] = {3, 4}; // pins for the 3way mode switch
//const int buttonPin = 2;  // pin for the tigger button
const bool printout = false;
const bool pong_only_in_range = true;
	

const double thresh = 20;
const double k = 10; // magnitude of the leak
const double sensory_factor = 0.1;
const double ao_max = 4096;
// connection settings - declare connections between neurons
// connection settings - declare connections between neurons
#define maxCon 40
#define nch 400 // number of neurons
#define num_pixels 250
#define num_sonar_inputs 2 
#define num_sound_inputs 2
#define num_touch_inputs 2

int readao( FILE* f0 ) {
    char value_str[7];
    long int value_int = 0;

    // FILE* f0 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");
            fread(&value_str, 6, 6, f0);
            value_int = strtol(value_str,NULL,0);
            fflush(stdout);
            rewind(f0);
            return value_int;
    }

double doPing(unsigned int *pruData) {
	// Wait for the PRU interrupt
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU1_ARM_INTERRUPT);

	return (double) pruData[0];
}


void doStartupLightDisplay(ledscape_t *leds, ledscape_frame_t *frame,  unsigned *frame_num, uint8_t *rgb_off, uint8_t *rgb_spike[num_pixels][3])
{

	uint32_t i;
	uint32_t ii;
	uint32_t iii;
    struct timespec tim, tim2, tim3;
    tim.tv_sec = 0;
    tim.tv_nsec = 50000000L;
    tim3.tv_sec = 0;
    tim3.tv_nsec = 50000000L;
    uint32_t np = (uint32_t) num_pixels-50;



	// flash lights
	for (i=0; i<1;i++){
		for (ii=0; ii<np; ii++){
	  		// ledscape_set_color(frame, 0, ii, rgb_spike[ii][0], rgb_spike[ii][1], rgb_spike[ii][2]);
	  		ledscape_set_color(frame, 0, ii,( uint8_t) 100, (uint8_t) 0, (uint8_t) 0);
	  			
	  	}
	  	ledscape_draw(leds, frame_num);
	  	nanosleep(&tim , &tim2);
	  	for (ii=0; ii<np; ii++){
	  		// ledscape_set_color(frame, 0, ii, rgb_spike[ii][0], rgb_spike[ii][1], rgb_spike[ii][2]);
	  		ledscape_set_color(frame, 0, ii,( uint8_t) 0, (uint8_t) 100, (uint8_t) 0);
	  			
	  	}
	  	ledscape_draw(leds, frame_num);
	  	nanosleep(&tim , &tim2);
	  	for (ii=0; ii<np; ii++){
	  		// ledscape_set_color(frame, 0, ii, rgb_spike[ii][0], rgb_spike[ii][1], rgb_spike[ii][2]);
	  		ledscape_set_color(frame, 0, ii,( uint8_t) 0, (uint8_t) 0, (uint8_t) 100);
	  			
	  	}
	  	ledscape_draw(leds, frame_num);
	  	nanosleep(&tim , &tim2);
	  	for (ii=0; ii<np; ii++){
	  		ledscape_set_color(frame, 0, ii, rgb_off[0], rgb_off[1], rgb_off[2]);
	  	}
	  	ledscape_draw(leds, frame_num);
	  	nanosleep(&tim , &tim2);
  	}

  	for (i=0; i<50; i++)
  	{
	  	for (ii=0; ii<np; ii++){
		  		// ledscape_set_color(frame, 0, ii, rgb_spike[ii][0], rgb_spike[ii][1], rgb_spike[ii][2]);
		  		ledscape_set_color(frame, 0, ii, (uint8_t) i, (uint8_t) i, (uint8_t) i);
		  	}
		  	ledscape_draw(leds, frame_num);
	  }
	for (iii=0; iii<5; iii++){
	  	for (i=0; i<50; i++)
	  	{
		  	for (ii=0; ii<np; ii++){
			  		// ledscape_set_color(frame, 0, ii, rgb_spike[ii][0], rgb_spike[ii][1], rgb_spike[ii][2]);
			  		ledscape_set_color(frame, 0, ii, (uint8_t) 55-i, (uint8_t) 55-i, (uint8_t) 55-i);
			  	}
			  	ledscape_draw(leds, frame_num);
		  }
	}
  	for (ii=0; ii<np; ii++){
	  		// ledscape_set_color(frame, 0, ii, rgb_spike[ii][0], rgb_spike[ii][1], rgb_spike[ii][2]);
	  		ledscape_set_color(frame, 0, ii, (uint8_t) 50, (uint8_t) 50, (uint8_t) 50);
	  	}
	ledscape_draw(leds, frame_num);
	nanosleep(&tim , &tim2);
	for (ii=0; ii<np; ii++){
	  		// ledscape_set_co lor(frame, 0, ii, rgb_spike[ii][0], rgb_spike[ii][1], rgb_spike[ii][2]);
	  	    ledscape_draw(leds, frame_num);
	  		// // nanosleep(&tim , &tim2);
	  		// ledscape_set_color(frame, 0, ii, rgb_off[0], rgb_off[1], rgb_off[2]);
	  		// ledscape_draw(leds, frame_num);
	  	}
}



float random_float(const float min, const float max)
{
    if (max == min) return min;
    else if (min < max) return (max - min) * ((float)rand() / RAND_MAX) + min;

    // return 0 if min > max
    return 0;
}

static uint16_t highest_bit(uint64_t v) {
    uint16_t out = 0;
    while (v > 0) {
        v >>= 1;
        ++out;
    }
    return out;
}

uint32_t myrandint( uint32_t LIMIT ) {
    static bool init = 0;
    static uint16_t n;
    static uint16_t shift;
    if (!init) {
        uint16_t randbits = highest_bit(RAND_MAX + (uint64_t)1L);
        uint16_t outbits = highest_bit( (uint64_t) LIMIT);
        n = (outbits + randbits - 1)/randbits;
        shift = randbits;
        init = 1;
    }
    uint32_t out = 0;
    for (uint16_t i=0; i<n; ++i) {
        out |= rand() << (i*shift);
    }
    return out % LIMIT;
}

double dur2cm(double dur) {
	return (double) dur / 58.44;
}

// main function 
int main(void) {

	// initialize variables
	uint32_t ch;
	uint32_t i;
	uint32_t ii;
	uint32_t iii;
	uint32_t syn;
	uint32_t loop_spikes; 
	uint32_t rep_spikes;
	uint32_t ao_values[2]; // for ai reads lets go back and check whether its possible or faster to do it all in one

	// open analog channel files
	FILE* a0 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");
	FILE* a1 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", "r");
	FILE* a2 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage2_raw", "r");
	FILE* a3 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage3_raw", "r");
	FILE* a4 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage4_raw", "r");


	// setup timers
	struct timeval new_time, last_time;
    double dt;
    double max_dt;
    double time_since_last_ping = 0;
    gettimeofday(&last_time, NULL);
    gettimeofday(&new_time, NULL);

    // initialize sonar parameters
	double duration = 0; 
	double target_distance = 0;
	double sense_thresh = sense_thresh_i;
	double currentIPI = minIPI;

	// setup input parameters
	uint32_t sonar_inputs[num_sonar_inputs];
	sonar_inputs[0] = (uint32_t) 0;
	sonar_inputs[1] = (uint32_t) 50;
	uint32_t sound_inputs[num_sound_inputs];
	sound_inputs[0] = (uint32_t) 25;
	sound_inputs[1] = (uint32_t) 75;
	uint32_t touch_inputs[num_sound_inputs];
	touch_inputs[0] = (uint32_t) 100;
	touch_inputs[1] = (uint32_t) 101;

	printf("%f", thresh);
	// initialize neurons 
	double v[nch] =  {0};
	double dv[nch] = {0};
	double spike_len[nch] =   {20};
	for (ii=1; ii<nch; ii++){
		spike_len[ii] = (double) random_float((float) 40, (float) 60);
	}



	// generate connections among neurons
	uint32_t connections[nch][maxCon];
	float weights[nch][maxCon];
	// generate linear layer
	uint32_t linear_layer_length = 150;
	for ( ii=0; ii<(linear_layer_length); ii++){

		connections[ii][0] = ii + 1; 
		weights[ii][0] = 18;
		connections[ii][1] = ii + 2; 
		weights[ii][1] = 10;
		connections[ii][2] = ii + 3;
		weights[ii][3] = 8;

	}
	for (ii=0; ii<(linear_layer_length); ii++){ // random synapses onto second layer
		for (iii=3; iii<maxCon; iii++){
			connections[ii][iii]=linear_layer_length + (uint32_t) myrandint( (uint32_t) (nch-linear_layer_length));
			// printf("%d \n", connections[ii][iii]);
			weights[ii][iii] = random_float((float) 0, (float) 14);
		}
	}
	for (ii=linear_layer_length; ii<(nch); ii++){ // random synapses onto second layer
		for (iii=3; iii<maxCon; iii++){
			connections[ii][iii]=linear_layer_length + (uint32_t) myrandint( (uint32_t) (nch-linear_layer_length));
			if (random_float((float) 0., (float) 1.) < 0.5) {
				weights[ii][iii] = random_float((float) -5, (float) 0.);
			}
			else {
				weights[ii][iii] = random_float((float) 0, (float) 5);
		}
		}
	}





 	/* Initialize the PRU for LEDS */
  	ledscape_t *const leds = ledscape_init(num_pixels);
  	uint8_t rgb_spike[num_pixels][3];
  	for (ii=0; ii<num_pixels; ii++){
  		rgb_spike[ii][0] = (uint8_t) myrandint( (uint32_t) 50);
  		rgb_spike[ii][1] = (uint8_t) myrandint( (uint32_t) 50);
  		rgb_spike[ii][2] = (uint8_t) myrandint( (uint32_t) 50);
  		printf("%d %d %d\n",rgb_spike[ii][0],rgb_spike[ii][1],rgb_spike[ii][2]);
  	}

  	uint8_t rgb_off[3] = {0,0,0};
  	const unsigned frame_num = i++ % 2;
  	ledscape_frame_t *const frame = ledscape_frame(leds, frame_num);
  	for (ii=0; ii<num_pixels; ii++){
  		ledscape_set_color(frame, 0, ii, rgb_off[0], rgb_off[1], rgb_off[2]);
  	}
  	ledscape_draw(leds, frame_num);



	/* Initialize the PRU for sonar*/
	printf(">> Initializing PRU\n");
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	prussdrv_init();
	/* Open PRU Interrupt */
	if (prussdrv_open (PRU_EVTOUT_0)) {
		// Handle failure
		fprintf(stderr, ">> PRU open failed\n");
		return 1;
	}
	/* Get the interrupt initialized */
	prussdrv_pruintc_init(&pruss_intc_initdata);
	/* Get pointers to PRU local memory */
	void *pruDataMem;
	prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, &pruDataMem);
    unsigned int pruData = (unsigned int *) pruDataMem;
	/* Execute code on PRU */
	printf(">> Executing HCSR-04 code\n");
	prussdrv_exec_program(prunum, "hcsr04.bin");



	// doStartupLightDisplay(leds, frame, frame_num, rgb_off, rgb_spike);
	/* Main Loop */
	i = 0;
	loop_spikes = 0;
	rep_spikes;
	while (1) {
		

		// stupid dt calc and loop observations 
		i = i + 1;
		gettimeofday(&new_time, NULL);
		dt = 0;//(double) (new_time.tv_sec - last_time.tv_sec);      // sec 
        dt += (double) (new_time.tv_usec - last_time.tv_usec)/1000000;   // us to s
		last_time = new_time;
		dt = fmax(dt,0);
		max_dt = fmax(dt,max_dt);
		rep_spikes = rep_spikes+loop_spikes; 

		// read and interperate input 
		ao_values[0] = readao(a0);
		sense_thresh = (((double) ao_values[0]) / ao_max)*sense_thresh_i;
		currentIPI = (double)(01 * sense_thresh * 2 * 29)/1000000; //set ipi based on a0

		// measure distance
		if (time_since_last_ping > currentIPI) {
			duration = doPing(pruData);
			target_distance = dur2cm(duration);
			if ( printout ) {
				printf("%d: Distance = %05.1f cm    loop_spikes = %03d   spike rate = %06.1f Hz   dt= %08f  max_dt=%08f ipi=%08f , ao=%d \n", i, target_distance,loop_spikes, (double) rep_spikes / (double) time_since_last_ping, dt, max_dt, time_since_last_ping, ao_values[0]);
			 }  
		    // target_distance = 90; 
		    time_since_last_ping = 0; 
			rep_spikes = 0; 
			max_dt = 0; 
		}
		else {
			time_since_last_ping = time_since_last_ping + dt; 
		}


		// printf("%05.5f  %05.5f  \n", dt, time_since_last_ping);
		// // printf("%d: Distance = %04.1f cm   ", i, target_distance);
		// // for (ii=0;ii<20;++ii) {
 	// // 		 printf("% 04.1f ", v[ii]);
		// //  }
  //       printf("\n");
		
		// set sonar input nodes based on sonar
		for (ch = 0; ch < num_sonar_inputs; ch++){
			  // set v[0] based on sonar
			 if (target_distance < sense_thresh & v[sonar_inputs[ch]] >= 0) {
			    v[sonar_inputs[ch]] = v[sonar_inputs[ch]] + (double) sensory_factor * sense_thresh / target_distance;
			 }
		}
		// set audio input nodes based on a1 (sensitivity) a2 (sound envelope; see spec) 
		ao_values[0] = readao(a1);
		ao_values[1] = readao(a2);
		for (ch = 0; ch < num_sound_inputs; ch++){
			  // set v[0] based on sonar
			 if ((double) ao_values[1] / ao_max >= 0.1 & v[sound_inputs[ch]] >= 0) {
			    v[sound_inputs[ch]] = v[sound_inputs[ch]] + ((double) 2*ao_values[0] / ao_max)*(log((double) ao_values[1] / ao_max));
			 }
		}
		// set touch input nodes based on a3 (sensitivity) a4 (touch resistance analog circuit; see spec) 
		ao_values[0] = readao(a3);
		ao_values[1] = readao(a4);
		for (ch = 0; ch < num_touch_inputs; ch++){
			  // set v[0] based on sonar
			 if ((double) ao_values[1] / ao_max >= 0.1 & v[touch_inputs[ch]] >= 0) {
			    v[touch_inputs[ch]] = v[touch_inputs[ch]] + ((double) 2*ao_values[0] / ao_max)*((double) ao_values[1] / ao_max);
			 }
		}

		// loop thru neurons
		 loop_spikes = 0; 
		for (ch = 0; ch < nch; ch++) {
			if (v[ch] >= 0) { // if neuron is in integrate mode
		    	v[ch] = v[ch]  + dv[ch] - k * v[ch] * (double) dt; // decay v to 0
		    	dv[ch] = 0; 
		    	v[ch] = fmax(v[ch], 0);
		    	v[ch] = fmin(v[ch], thresh+1);
		    	if (ch < num_pixels){
		    		ledscape_set_color(frame, 0, ch, (uint8_t) 0, (uint8_t) 0,(uint8_t) round(50* v[ch] / (thresh*1.5))); 
			    }
		        // if the neuron crosses threshold, fire and increment outputs
			    if (v[ch] > thresh) {
			        loop_spikes = loop_spikes+1;
			        if (loop_spikes < 1000){
			        	if (ch < num_pixels){
			        		ledscape_set_color(frame, 0, ch, rgb_spike[ch][0], rgb_spike[ch][1], rgb_spike[ch][2]);
			        	}
			        }
			        // printf("ch%d spike ", ch);
			        v[ch] = -1; // v<0 stores that the neuron is in firing state
			      }
			    }
		    else { // otherwise if neuron is in spike mode
		      if (v[ch] < -1 * spike_len[ch]) { // if the time since spike onset is up, end spike
		        v[ch] = 0; // set voltage to 0
		        ledscape_set_color(frame, 0, ch, rgb_off[0], rgb_off[1], rgb_off[2]);
		        for (syn = 0; syn < maxCon; syn++) { // loop thru synaptic outputs
			          	// if connection is real and postsyn element is not in firing, incriment its v
			          	if (connections[ch][syn] >= 0 & v[connections[ch][syn]] >= 0) {
			            	dv[connections[ch][syn]] += weights[ch][syn];

			            }
			        }
		      }
		      else {
		        v[ch] = v[ch] - dt*1000; // otherwise decrment v by dt to record time
		      }
		    }
		  }
		  ledscape_draw(leds, frame_num);


	}

	/* Disable PRU and close memory mapping*/
	prussdrv_pru_disable(prunum);
	prussdrv_exit();
	printf(">> Sonar PRU Disabled.\r\n");
	ledscape_close(leds);
	printf(">> LED PRU Disabled.\r\n");

	// close files for AI
	fclose(a0);
	return (0);

}






