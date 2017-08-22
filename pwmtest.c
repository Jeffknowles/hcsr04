#include <stdio.h>
#include <stdlib.h>
#include "BBBIOlib/BBBio_lib/BBBiolib.h"
/* -------------------------------------------------------------- */
int main(void)
{
	/* NOTICE :
	 *	please load BBBIO-EHRPWM overlay first
	 *	help : https://github.com/VegetableAvenger/BBBIOlib/tree/master/overlay
	 **/
	iolib_init();

	const float PWM_HZ = 100.0f ;	/* 100 Hz */
	const float duty_A = 20.0f ; 	/* 20% Duty cycle for PWM 0_A output */
	const float duty_B = 50.0f ;	/* 50% Duty cycle for PWM 0_B output*/

	printf("PWM Demo setting ....\n");
	BBBIO_PWMSS_Setting(BBBIO_PWMSS0, PWM_HZ ,duty_A , duty_B);

	printf("PWM %d enable for 10s ....\n", BBBIO_PWMSS0);
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS0);
	sleep(10);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
	printf("close\n");


	iolib_init();
	BBBIO_sys_Enable_GPIO(BBBIO_GPIO2);

	BBBIO_GPIO_set_dir(BBBIO_GPIO2 ,
			   BBBIO_GPIO_PIN_10 |BBBIO_GPIO_PIN_11 | BBBIO_GPIO_PIN_12 |BBBIO_GPIO_PIN_13 ,	// Input
			   BBBIO_GPIO_PIN_6 | BBBIO_GPIO_PIN_7 | BBBIO_GPIO_PIN_8 | BBBIO_GPIO_PIN_9);		// Output

	int count =0;
	int DIPvalue=0 ;		// finger switch value
	int LEDvalue =0;
	while(count < 100)
	{
	    // Read DIP value
	    LEDvalue =0;
	    DIPvalue = BBBIO_GPIO_get(BBBIO_GPIO2 ,BBBIO_GPIO_PIN_10 |BBBIO_GPIO_PIN_11 | BBBIO_GPIO_PIN_12 |BBBIO_GPIO_PIN_13 );

	    // check value , and seting which LED must be glittered .
	    if(DIPvalue & BBBIO_GPIO_PIN_11)
		LEDvalue |=BBBIO_GPIO_PIN_7;

	    if(DIPvalue & BBBIO_GPIO_PIN_10)
		LEDvalue |=BBBIO_GPIO_PIN_6;

	    if(DIPvalue & BBBIO_GPIO_PIN_13)
		LEDvalue |=BBBIO_GPIO_PIN_9;

	    if(DIPvalue & BBBIO_GPIO_PIN_12)
		LEDvalue |=BBBIO_GPIO_PIN_8;

	    // glitter LED
	    BBBIO_GPIO_high(BBBIO_GPIO2 , LEDvalue);
	    iolib_delay_ms(100);

	    // close all
            BBBIO_GPIO_low(BBBIO_GPIO2 , BBBIO_GPIO_PIN_6 | BBBIO_GPIO_PIN_7 | BBBIO_GPIO_PIN_8 | BBBIO_GPIO_PIN_9);
	    iolib_delay_ms(100);
	    count ++;
	}
	iolib_free();
	return 0;
}




