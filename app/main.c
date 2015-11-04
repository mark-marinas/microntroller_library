/*
    FreeRTOS V8.2.2 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/******************************************************************************
 * This project provides two demo applications.  A simple blinky style project,
 * and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting (defined in this file) is used to
 * select between the two.  The simply blinky demo is implemented and described
 * in main_blinky.c.  The more comprehensive test and demo application is
 * implemented and described in main_full.c.
 *
 * This file implements the code that is not demo specific, including the
 * hardware setup and FreeRTOS hook functions.  It also contains a dummy
 * interrupt service routine called Dummy_IRQHandler() that is provided as an
 * example of how to use interrupt safe FreeRTOS API functions (those that end
 * in "FromISR").
 *
 *****************************************************************************/

//Unedited version is from FreeRTOS\Demo\CORTEX_M0_STM32F0518_IAR

/* Standard includes. */
#include "string.h"
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
/* Demo application include. */
#include "ParTest.h"
#include "LPC17xx.h"

#include "stdperip.h"
#include "gpio_lpc17xx.h"
#include "stdirq.h"

/* Set mainCREATE_SIMPLE_BLINKY_DEMO_ONLY to one to run the simple blinky demo,
or 0 to run the more comprehensive test and demo application. */
#define mainCREATE_SIMPLE_BLINKY_DEMO_ONLY	1


/*-----------------------------------------------------------*/

/*
 * Perform any application specific hardware configuration.  The clocks,
 * memory, etc. are configured before main() is called.
 */
static void prvSetupHardware( void );

/* main_blinky() is used when mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 1.
main_full() is used when mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 0. */
extern void main_blinky( void );
extern void main_full( void );

/*-----------------------------------------------------------*/

#include "stdperip.h"
#include "uart_lpc17xx.h"
#include "i2c_lpc17xx.h"



int main( void )
{

	uart_config_t uart0;
	gpio_config_t key1;
	i2c_config_t i2c0;
	i2c_command_t command;

	error_code_t error;
	uart0.baudrate = B2400;
	uart0.block_type = NON_BLOCKING;
	uart0.buffer = 0;
	uart0.irqhandler = 0;
	uart0.parity = UART_PARITY_NONE;
	uart0.uart_port = COM0;
	uart0.wordlen = UART_WORDLEN8;
	uart0.stopbit = STOP_BIT_1_BIT;


	key1.Direction = INPUT;
	key1.Initial_Value = LO;
	key1.Interrupt_Type = INTERRUPT_ENABLED_BOTH;
	key1.Pin = PIN11;
	key1.Port= PORT2;
	key1.Pin_Mode = PULLUP_PULLDOWN_DISABLED;
	key1.Pin_Mode_OD = PIN_MODE_OPEN_DRAIN_NORMAL;
	key1.Pin_Typedef = 0;
	key1.irqhandler = 0;

	i2c0.i2c_port = I2C0;
	i2c0.i2c_mode = MASTER;
	i2c0.datarate = STANDARD;
	i2c0.buffer = 0;
	i2c0.irqhandler = 0;

	if ( (error = GPIO_Config(&key1)) != NO_ERROR ) {
		while (1);
	}

	if ( (error = UART_Config(&uart0)) != NO_ERROR) {
		while (1);
	}

	if ( (error = I2C_Config(&i2c0)) != NO_ERROR ) {
		while (1);
	}
	UART_PutChars(COM0, "InitDone\n\r",10);

#if (1)
	int size = 18;
	char fw_version[18] = "PowerAvrVersion5.5";
	char fw_version_read[18] = { 0 };
	command.address = 0x50;
	command.data = fw_version;
	command.operation = WRITE;
	command.reg = 0x00;
	command.size = 1; //11;

	int i, j;
	for (j=0; j<1; j++) {
		for (i=0; i<size;i++) {
			//UART_PutChars(COM0,"Writing", 9);
			UART_PutChars(COM0,".", 1);
			command.reg = 0x00 + i;
			command.size = 1;
			command.data = &(fw_version[i]);
			error = I2C_Write(I2C0, &command);
			if (error != NO_ERROR) {
				break;
			}
		}
		if (error != NO_ERROR) {
			while (1);
		}
	}
	UART_PutChars(COM0, "WritDone\n\r",10);


	command.operation = READ;
	command.data = fw_version_read;
	command.size = 1 ;//11;
	for (j=0; j<1; j++) {
		for (i=0; i<size;i++) {
				fw_version_read[i] = 'X';
		}
		for (i=0; i<size;i++) {
			//UART_PutChars(COM0,"Reading\n\r", 9);
			command.reg = 0x00 + i;
			command.size = 1;
			command.data = &(fw_version_read[i]);
			error = I2C_Read(I2C0, &command);
			if (error != NO_ERROR) {
				break;
			}
		}
		if (error != NO_ERROR) {
			while (1);
		}
	}

	UART_PutChars(COM0, "ReadDone\n\r",10);
	UART_PutChars(COM0, &(fw_version_read[0]), size/*11*/);
	UART_PutChars(COM0, (char*) "\n\r", 2);
#endif

	pin_interrupt_type_t key1_status;
	char rising_str[] = "Rising  Edge\n\r"; //14
	char falling_str[] ="Falling Edge\n\r";
	while (1) {
		error = GPIO_GetIRQ(&key1, &key1_status);
		if (error != NO_ERROR) {
			while (1);
		}
		if (key1_status == INTERRUPT_ENABLED_FALLING) {
			UART_PutChars(COM0, falling_str, 14);
			error = GPIO_ClrIRQ(&key1);
			if (error != NO_ERROR) {
				while (1);
			}
		} else if (key1_status == INTERRUPT_ENABLED_RISING) {
			UART_PutChars(COM0, rising_str, 14);
			error = GPIO_ClrIRQ(&key1);
			if (error != NO_ERROR) {
				while (1);
			}
		}
	}

	//UART TEST
	char rx_data;
	char tx_data[] = "This is x nnnnn\n\r";
	while (1) {
		//if ( UART_GetChar(COM0, &rx_data) != FIFO_EMPTY ) {
			tx_data[8] = 'a';
			//UART_PutChar(COM0, rx_data);
			UART_PutChars(COM0, tx_data, 17 );
			int i;
			tx_data[8] = 'b';
			for (i=0; i<17; i++) {
				UART_PutChar(COM0, tx_data[i]);
			}
		//}
	}


	/* Prepare the hardware to run this demo. */
	prvSetupHardware();
	//vConfigureTimerForRunTimeStats();
	/* The mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting is described at the top
	of this file. */
	#if mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 1
	{
		main_blinky();
	}
	#else
	{
		main_full();
	}
	#endif

	return 0;
}

lpc17xx_gpio_config_t button = { 1, 25, INPUT, INTERRUPT_DISABLED,PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, LO, 0, 0} ;
lpc17xx_gpio_config_t led 	 = { PORT2, PIN6,OUTPUT, INTERRUPT_DISABLED,PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, LO, 0, 0} ;
lpc17xx_gpio_config_t led2 	 = { PORT2, PIN7,OUTPUT, INTERRUPT_DISABLED,PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, LO, 0, 0} ;
/*-----------------------------------------------------------*/
static void prvSetupHardware( void )
{
	if ( GPIO_Config(&button) != NO_ERROR ) {
		while (1);
	}
	if ( GPIO_Config(&led) != NO_ERROR ) {
		while (1);
	}
	if ( GPIO_Config(&led2) != NO_ERROR ) {
		while (1);
	}

	/*
	signal_level_t	button_val;
	while (1) {
		if ( GPIO_GetLevel(&button, &button_val) != NO_ERROR ) {
			while (1);
		}
		if (button_val == HI) {
			if ( GPIO_SetLevel(&led, HI) != NO_ERROR ) {
				while (1);
			}
		} else if ( button_val == LO) {
			if ( GPIO_SetLevel(&led, LO) != NO_ERROR ) {
				while (1);
			}
		}
	}

	vParTestInitialise();
	*/
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

                                                           
xSemaphoreHandle xSemaphoreSleep = NULL;
void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
}
/*-----------------------------------------------------------*/

#ifdef JUST_AN_EXAMPLE_ISR

void Dummy_IRQHandler(void)
{
long lHigherPriorityTaskWoken = pdFALSE;

	/* Clear the interrupt if necessary. */
	Dummy_ClearITPendingBit();

	/* This interrupt does nothing more than demonstrate how to synchronise a
	task with an interrupt.  A semaphore is used for this purpose.  Note
	lHigherPriorityTaskWoken is initialised to zero. Only FreeRTOS API functions
	that end in "FromISR" can be called from an ISR. */
	xSemaphoreGiveFromISR( xTestSemaphore, &lHigherPriorityTaskWoken );

	/* If there was a task that was blocked on the semaphore, and giving the
	semaphore caused the task to unblock, and the unblocked task has a priority
	higher than the current Running state task (the task that this interrupt
	interrupted), then lHigherPriorityTaskWoken will have been set to pdTRUE
	internally within xSemaphoreGiveFromISR().  Passing pdTRUE into the
	portEND_SWITCHING_ISR() macro will result in a context switch being pended to
	ensure this interrupt returns directly to the unblocked, higher priority,
	task.  Passing pdFALSE into portEND_SWITCHING_ISR() has no effect. */
	portEND_SWITCHING_ISR( lHigherPriorityTaskWoken );
}

#endif /* JUST_AN_EXAMPLE_ISR */

void vConfigureTimerForRunTimeStats( void )
{
const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	collecting run time statistical information - basically the percentage
	of CPU time that each task is utilising.  It is called automatically when
	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	to 1). */

	/* Power up and feed the timer. */
	LPC_SC->PCONP |= 0x02UL;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<2))) | (0x01 << 2);

	/* Reset Timer 0 */
	LPC_TIM0->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	LPC_TIM0->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	but not too fast so as to overflow all the time. */
	LPC_TIM0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

	/* Start the counter. */
	LPC_TIM0->TCR = TCR_COUNT_ENABLE;
}


