#include <stdio.h>
#include <unistd.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>


#include <stdbool.h>



const int pingLen = 20.; // ping length in microseconds
const int minIPI = 100; // minimum interping interval in miliseconds
const float sense_thresh_i = 500; // threshold where responses turn on
const int pingPin = 11; // trigger for sonar pulses
const int echoPin = 12; // return for sonar pulses
const int phonePin1 = 9; //
const int phonePin2 = 10;
const int dialPin = 5;  // analog pin for the dial
//const int modePins[2] = {3, 4}; // pins for the 3way mode switch
//const int buttonPin = 2;  // pin for the tigger button
const int nch = 10; // number of neurons
const int ledPins[nch] = {5, 6, 7, 8, 13, 3, 2, 4}; // indicates the arduino pin for each light
const int sensory_factor = 10;
const bool printout = false;
const bool pong_only_in_range = true;




// connection settings - declare connections between neurons
const int maxCon = 5;
const int connections[nch][maxCon] = {  // row i indicates (densly) the connections emmenating from the ith element
  // -1 is a placeholder for no connection.  each row needs macCon entries
  {1, 2, 3, -1, -1  }, // 0's outputs
  {2, 3, -1, -1, -1  }, // 1's outputs
  {3, 4, 5, -1, -1  }, // 2's outputs
  {4, 5, 9, -1, -1  }, // 3's outputs
  {4, 5, 6, 8,  -1  }, // 4's outputs
  {3, 4, 6, 7, 8    }, // 5's outputs
  {3, 5, 7, 8, 9    }, // 6's outputs
  {8, 9, 4, -1, -1  }, // 7's outputs
  {9, 6, 5, -1, -1  }, // 8's outputs
  {6, 7, 8, -1, -1  }
}; // 9's outputs





// main function 
int main(void) {

	// initialize variables
	float duration = 0; 
	float target_distance = 0;
	float sense_thresh = sense_thresh_i;
	bool button_pressed = false;
	int mode = 0;
	long last_ping = 0;
	long currentIPI = minIPI;
		//// pin settings



	const int minIPI = 100; // minimum interping interval in miliseconds
	const float sense_thresh_i = 500; // threshold where responses turn on





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
	unsigned int *pruData = (unsigned int *) pruDataMem;

	/* Execute code on PRU */
	printf(">> Executing HCSR-04 code\n");
	prussdrv_exec_program(0, "hcsr04.bin");



	/* Main Loop */
	int i = 0;
	while (1) {
		i = i + 1;

		// measure distance
		duration = doPing();
		target_distance = (float) duration / 58.44;
		printf("%d: Distance = %.2f cm\n", i, target_distance);
		// sleep(0.01);


	}

	/* Disable PRU and close memory mapping*/
	prussdrv_pru_disable(0);
	prussdrv_exit();
	printf(">> PRU Disabled.\r\n");
	
	return (0);

}






float doPing(pruData) {
	// Wait for the PRU interrupt
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

	return (float) pruData[0];

}