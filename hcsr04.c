#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <prussdrv.h> 
#include <pruss_intc_mapping.h>
#include <time.h>
#include <math.h>
#include <stdlib.h> 
#include <tgmath.h> 
#include <inttypes.h>

const int pingLen = 20.; // ping length in microseconds
const int minIPI = 100; // minimum interping interval in miliseconds
const float sense_thresh_i = 300; // threshold where responses turn on
const int pingPin = 11; // trigger for sonar pulses
const int echoPin = 12; // return for sonar pulses
const int phonePin1 = 9; //
const int phonePin2 = 10;
const int dialPin = 5;  // analog pin for the dial
//const int modePins[2] = {3, 4}; // pins for the 3way mode switch
//const int buttonPin = 2;  // pin for the tigger button

// int ledPins[nch] = {5, 6, 7, 8, 13, 3, 2, 4,-1,-1}; // indicates the arduino pin for each light
const int sensory_factor = 10;
const bool printout = false;
const bool pong_only_in_range = true;
	

const float thresh = 10;
const float k = 1; // magnitude of the leak


// connection settings - declare connections between neurons
#define maxCon 5
#define nch 8 // number of neurons
// const int maxCon = 5;




uint64_t GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}


float doPing(unsigned int *pruData) {
	// Wait for the PRU interrupt
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

	return (float) pruData[0];
}


float dur2cm(float dur) {
	return (float) dur / 58.44;
}

// main function 
int main(void) {

	// initialize variables
	int ch;
	int i;
	int ii;
	int syn;

	uint64_t last_time = GetTimeStamp(NULL);
	float duration = 0; 
	float target_distance = 0;
	float sense_thresh = sense_thresh_i;
	bool button_pressed = false;
	int mode = 0;
	long last_ping = 0;
	long currentIPI = minIPI;
	float v[nch] =  {0,   0,   0,   0,   0,  0,   0,   0,   0,   0};
	long spike_len[nch] =     {1,  20,  35,  20,  10, 27,  31,  50,  70, 300};

	int connections[nch][maxCon] = {  // row i indicates (densly) the connections emmenating from the ith element
	  // -1 is a placeholder for no connection.  each row needs macCon entries
	  {1, 2, 3, -1, -1  },  // 0's outputs
	  {2, 3, -1, -1, -1 }, // 1's outputs
	  {3, 4, 5, -1, -1  }, // 2's outputs
	  {4, 5, 9, -1, -1  }, // 3's outputs
	  {4, 5, 6, 8,  -1  }, // 4's outputs
	  {3, 4, 6, 7, 8    }, // 5's outputs
	  {3, 5, 7, 8, 9    }, // 6's outputs
	  {8, 9, 4, -1, -1  }, // 7's outputs
	  {9, 6, 5, -1, -1  }, // 8's outputs
	  {6, 7, 8, -1, -1  }  // 9's outputs
	}; 





	/* Initialize the PRU */
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
	prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &pruDataMem);
    unsigned int pruData = (unsigned int *) pruDataMem;

	// printf("%d",sizeof(pruData));
	/* Execute code on PRU */
	printf(">> Executing HCSR-04 code\n");
	prussdrv_exec_program(0, "hcsr04.bin");


	/* Main Loop */
	i = 0;
	while (1) {
		i = i + 1;

		// measure distance
		duration = doPing(pruData);
		target_distance = dur2cm(duration);

		printf("%d: Distance = %.2f cm   ", i, target_distance);
		for(ii=0;ii<nch;++ii) {
 			 printf("%.2f ", v[ii]);
		 }
        printf("\n");
		

		float dt = (float)  GetTimeStamp(NULL) - last_time;
		last_time = GetTimeStamp(NULL);
		printf("%f  ", dt);
		  // set v[0] based on sonar
		 if (target_distance < sense_thresh & v[0] >= 0) {
		    v[0] = v[0] + (float) 1 * sense_thresh / target_distance;
		 }

		// loop thru neurons
		for (ch = 0; ch < nch; ch++) {
		if (v[ch] >= 0) { // if neuron is in integrate mode
		    v[ch] = v[ch]  - k * v[ch] * (float) dt; // decay v to 0
		    v[ch] = fmax(v[ch], 0);
		    // if the neuron crosses threshold, fire and increment outputs
		    if (v[ch] > thresh) {
		        // if (ledPins[ch] > 0) {
		        //   digitalWrite(ledPins[ch], HIGH);
		        // }
		        printf("%d spike", ch);
		        v[ch] = -1; // v<0 stores that the neuron is in firing state
		        for (syn = 0; syn < maxCon; syn++) { // loop thru synaptic outputs
		          	// if connection is real and postsyn element is not in firing, incriment its v
		          	if (connections[ch][syn] >= 0 & v[connections[ch][syn]] >= 0) {
		            	v[connections[ch][syn]] ++;
		            }
		        }
		      }
		    }
		    else { // otherwise if neuron is in spike mode
		      if (v[ch] < -1 * spike_len[ch]) { // if the time since spike onset is up, end spike
		        v[ch] = 0; // set voltage to 0
		        // if (ledPins[ch] > 0) {
		        //   digitalWrite(ledPins[ch], LOW);
		        // }
		      }
		      else {
		        v[ch] = v[ch] - (float) dt * 1000; // otherwise decrment v by dt to record time
		      }
		    }
		  }
		// sleep(0.01);


	}

	/* Disable PRU and close memory mapping*/
	prussdrv_pru_disable(0);
	prussdrv_exit();
	printf(">> PRU Disabled.\r\n");
	
	return (0);

}







