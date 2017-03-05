#include <stdio.h>
#include <unistd.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>




const int minIPI = 100; // minimum interping interval in miliseconds
const float sense_thresh_i = 500; // threshold where responses turn on

//
float sense_thresh = sense_thresh_i;
boolean button_pressed = false;
int mode = 0;
float target_distance = sense_thresh;
long last_ping = 0;
long currentIPI = minIPI;

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
	float target_distance = 0;






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
		doPing()
		target_distance = (float) pruData[0] / 58.44;

		// Wait for the PRU interrupt
		// prussdrv_pru_wait_event (PRU_EVTOUT_0);
		// prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

		
		// Print the distance received from the sonar
		// At 20 degrees in dry air the speed of sound is 342.2 cm/sec
		// so it takes 29.12 us to make 1 cm, i.e. 58.44 us for a roundtrip of 1 cm
		printf("%3d: Distance = %.2f cm\n", i, target_distance);
		// sleep(0.01);
	}

	/* Disable PRU and close memory mapping*/
	prussdrv_pru_disable(0);
	prussdrv_exit();
	printf(">> PRU Disabled.\r\n");
	
	return (0);

}






int doPing(void) {
	// Wait for the PRU interrupt
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);


}