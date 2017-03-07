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
#include <sys/time.h>  

const double minIPI = 0.1; // minimum interping interval in miliseconds
const double sense_thresh_i = 100; // threshold where responses turn on
// const int pingPin = 11; // trigger for sonar pulses
// const int echoPin = 12; // return for sonar pulses
// const int phonePin1 = 9; //
// const int phonePin2 = 10;
// const int dialPin = 5;  // analog pin for the dial
//const int modePins[2] = {3, 4}; // pins for the 3way mode switch
//const int buttonPin = 2;  // pin for the tigger button

// int ledPins[nch] = {5, 6, 7, 8, 13, 3, 2, 4,-1,-1}; // indicates the arduino pin for each light
const int sensory_factor = 10;
const bool printout = false;
const bool pong_only_in_range = true;
	

const double thresh = 10;
const double k = 1; // magnitude of the leak


// connection settings - declare connections between neurons
#define maxCon 10
#define nch 1000 // number of neurons
// const int maxCon = 5;




// uint64_t GetTimeStamp() {
//     struct timeval tv;
//     gettimeofday(&tv,NULL);
//     return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
// }


double doPing(unsigned int *pruData) {
	// Wait for the PRU interrupt
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

	return (double) pruData[0];
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
	int syn;


	struct timeval new_time, last_time;
    double dt;
    double time_since_last_ping = 0;
    gettimeofday(&last_time, NULL);
    gettimeofday(&new_time, NULL);
    // gettimeofdat($last_ping, NULL);
	// uint64_t last_time = GetTimeStamp(NULL);
	double duration = 0; 
	double target_distance = 0;
	double sense_thresh = sense_thresh_i;
	double currentIPI = minIPI;
	double v[nch] =  {0,   0,   0,   0,   0,  0,   0,   0,   0,   0};
	double spike_len[nch] =     {10,  20,  35,  20,  10, 27,  31,  50,  70, 300};

	int connections[nch][maxCon];
	// = {  // row i indicates (densly) the connections emmenating from the ith element
	//   // -1 is a placeholder for no connection.  each row needs macCon entries
	//   {1, 2, 3, -1, -1  },  // 0's outputs
	//   {2, 3, -1, -1, -1 }, // 1's outputs
	//   {3, 4, 5, -1, -1  }, // 2's outputs
	//   {4, 5, 9, -1, -1  }, // 3's outputs
	//   {4, 5, 6, 8,  -1  }, // 4's outputs
	//   {3, 4, 6, 7, 8    }, // 5's outputs
	//   {3, 5, 7, 8, 9    }, // 6's outputs
	//   {8, 9, 4, -1, -1  }, // 7's outputs
	//   {9, 6, 5, -1, -1  }, // 8's outputs
	//   {6, 7, 8, -1, -1  }  // 9's outputs
	// }; 


	for ( ii=0; ii<(nch-10); ii++){
		connections[ii][0] = ii + 1; 
		connections[ii][1] = ii + 2;
		connections[ii][2] = ii + 3;
		connections[ii][3] = ii + 4;
		connections[ii][4] = ii + 5;
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

	// printf("%d",sizeof(pruData));
	/* Execute code on PRU */
	printf(">> Executing HCSR-04 code\n");
	prussdrv_exec_program(0, "hcsr04.bin");


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
				time_since_last_ping = 0; 
		}
		else {
			time_since_last_ping = time_since_last_ping + dt; 
		}

		printf("%.5f  %0.5f  ", dt, time_since_last_ping);
		printf("%d: Distance = %.2f cm   ", i, target_distance);
		for (ii=0;ii<20;++ii) {
 			 printf("%02f ", v[ii]);
		 }
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
		            	v[connections[ch][syn]] += 1;
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
	prussdrv_pru_disable(0);
	prussdrv_exit();
	printf(">> PRU Disabled.\r\n");
	
	return (0);

}







