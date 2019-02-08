/**
 *	STM32F4 Discovery Skeleton Project
 *
 *	@ide		Keil uVision 5
 *	@conf		PLL parameters are set in "Options for Target" -> "C/C++" -> "Defines"
 *	@packs		STM32F4xx Keil packs version 2.2.0 or greater required
 *	@stdperiph	STM32F4xx Standard peripheral drivers version 1.5.0 or greater required
 */
/* Include core modules */
#include "stm32f4xx.h"
/* Include helper library */
#include "ess_helper.h"
#include "lab.h"

void execute_lab()
{
	//execute_lab1();
	//execute_lab2();
	execute_lab3();
}

int main(void) {
	/* Initialize system */
	SystemInit();
	/* Initialize peripherals on board */
	ess_helper_init();
	// Execute lab task
	execute_lab();
	// Main loop 
	/*while (1) {
		
	}*/
}
