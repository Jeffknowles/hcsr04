#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <stdlib.h> 
#include <tgmath.h> 
#include <inttypes.h>
#include <sys/time.h>  

#include <prussdrv.h> 
#include <pruss_intc_mapping.h>

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
#define prunum 0



double doPing(unsigned int *pruData) {
	// Wait for the PRU interrupt
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

	return (double) pruData[0];
}

float random_float(const float min, const float max)
{
    if (max == min) return min;
    else if (min < max) return (max - min) * ((float)rand() / RAND_MAX) + min;

    // return 0 if min > max
    return 0;
}


double dur2cm(double dur) {
	return (double) dur / 58.44;
}

// main function 
int main(void) {

	// initialize variables
	int ch;
	int i;
	int ii;
	int iii;
	int syn;

	// setup timers
	struct timeval new_time, last_time;
    double dt;
    double time_since_last_ping = 0;
    gettimeofday(&last_time, NULL);
    gettimeofday(&new_time, NULL);

    // initialize sonar parameters
	double duration = 0; 
	double target_distance = 0;
	double sense_thresh = sense_thresh_i;
	double currentIPI = minIPI;

	// initialize neurons 
	double v[nch] =  {0};
	double spike_len[nch] =   {10};
	int connections[nch][maxCon];
	float weights[nch][maxCon];
	// generate connections among neurons
	for ( ii=0; ii<(nch-10); ii++){
		connections[ii][0] = ii + 1; 
		weights[ii][0] = 5;
		connections[ii][1] = ii + 2; 
		weights[ii][1] = 5;

		for (iii=2; iii<maxCon; iii++){
			connections[ii][iii]=rand() % nch;
			weights[ii][iii] = random_float((float) -10, (float) 10);
		}
	}



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
	/* Execute code on PRU */
	printf(">> Executing HCSR-04 code\n");
	prussdrv_exec_program(prunum, "hcsr04.bin");



	/* Main Loop */
	i = 0;
	while (1) {
		i = i + 1;

		gettimeofday(&new_time, NULL);
		dt = 0;//(double) (new_time.tv_sec - last_time.tv_sec);      // sec 
        dt += (double) (new_time.tv_usec - last_time.tv_usec)/1000000;   // us to s
		last_time = new_time;
		dt = fmax(dt,0);


		// measure distance
		if (time_since_last_ping > currentIPI) {
				duration = doPing(pruData);
				target_distance = dur2cm(duration);
			    // target_distance = 90; 
				time_since_last_ping = 0; 
		}
		else {
			time_since_last_ping = time_since_last_ping + dt; 
		}

		printf("%05.5f  %05.5f  \n", dt, time_since_last_ping);
		printf("%d: Distance = %04.1f cm   ", i, target_distance);
		// for (ii=0;ii<20;++ii) {
 	// 		 printf("% 04.1f ", v[ii]);
		//  }
        printf("\n");
		
		  // set v[0] based on sonar
		 if (target_distance < sense_thresh & v[0] >= 0) {
		    v[0] = v[0] + (double) 0.1 * sense_thresh / target_distance;
		 }

		// loop thru neurons
		for (ch = 0; ch < nch; ch++) {
			if (v[ch] >= 0) { // if neuron is in integrate mode
		    	v[ch] = v[ch]  - k * v[ch] * (double) dt; // decay v to 0
		    	v[ch] = fmax(v[ch], 0);
		    // if the neuron crosses threshold, fire and increment outputs
		    if (v[ch] > thresh) {
		        // if (ledPins[ch] > 0) {
		        //   digitalWrite(ledPins[ch], HIGH);
		        // }
		        printf("ch%d spike \n", ch);
		        v[ch] = -1; // v<0 stores that the neuron is in firing state
		        for (syn = 0; syn < maxCon; syn++) { // loop thru synaptic outputs
		          	// if connection is real and postsyn element is not in firing, incriment its v
		          	if (connections[ch][syn] >= 0 & v[connections[ch][syn]] >= 0) {
		            	v[connections[ch][syn]] += weights[ch][syn];
		            	v[connections[ch][syn]] = fmax(v[connections[ch][syn]],0);
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
		        v[ch] = v[ch] - dt*1000; // otherwise decrment v by dt to record time
		      }
		    }
		  }
		// sleep(0.01);


	}

	/* Disable PRU and close memory mapping*/
	prussdrv_pru_disable(prunum);
	prussdrv_exit();
	printf(">> PRU Disabled.\r\n");
	
	return (0);

}







