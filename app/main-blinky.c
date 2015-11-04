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
 * NOTE 1:  This project provides two demo applications.  A simple blinky style
 * project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the simply blinky style version.
 *
 * NOTE 2:  This file only contains the source code that is specific to the
 * basic demo.  Generic functions, such FreeRTOS hook functions, and functions
 * required to configure the hardware, are defined in main.c.
 ******************************************************************************
 *
 * main_blinky() creates one queue, and two tasks.  It then starts the
 * scheduler.
 *
 * The Queue Send Task:
 * The queue send task is implemented by the prvQueueSendTask() function in
 * this file.  prvQueueSendTask() sits in a loop that causes it to repeatedly
 * block for 200 milliseconds, before sending the value 100 to the queue that
 * was created within main_blinky().  Once the value is sent, the task loops
 * back around to block for another 200 milliseconds.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the prvQueueReceiveTask() function
 * in this file.  prvQueueReceiveTask() sits in a loop where it repeatedly
 * blocks on attempts to read data from the queue that was created within
 * main_blinky().  When data is received, the task checks the value of the
 * data, and if the value equals the expected 100, toggles LED 1.  The 'block
 * time' parameter passed to the queue receive function specifies that the
 * task should be held in the Blocked state indefinitely to wait for data to
 * be available on the queue.  The queue receive task will only leave the
 * Blocked state when the queue send task writes to the queue.  As the queue
 * send task writes to the queue every 200 milliseconds, the queue receive
 * task leaves the Blocked state every 200 milliseconds, and therefore toggles
 * the LED every 200 milliseconds.
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
/* Demo includes. */
#include "ParTest.h"

/* Hardware includes. */

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the portTICK_PERIOD_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS			( 200 / portTICK_PERIOD_MS )
#define mainQUEUE_SEND_FREQUENCY2_MS			( 1000 / portTICK_PERIOD_MS )
#define mainQUEUE_SEND_FREQUENCY3_MS			( 1 / portTICK_PERIOD_MS )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define mainQUEUE_LENGTH					( 1 )

/* Values passed to the two tasks just to check the task parameter
functionality. */
#define mainQUEUE_SEND_PARAMETER			( 0x1111UL )
#define mainQUEUE_RECEIVE_PARAMETER			( 0x22UL )
/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );
static void prvReadGPIO(void *pvParameters ) ;

/*
 * Called by main() to create the simply blinky style application if
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 1.
 */
void main_blinky( void );

/*
 * The hardware only has a single LED.  Simply toggle it.
 */
extern void vMainToggleLED( void );

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;
static QueueHandle_t xQueue2 = NULL;
static xSemaphoreHandle xSemaphoreMain = NULL;


/*-----------------------------------------------------------*/

#define buttonPriority  (tskIDLE_PRIORITY + 2)
#define delayms(ms)			( ms / portTICK_PERIOD_MS )


static xSemaphoreHandle xSemaphoreBlue = NULL;
static xSemaphoreHandle xSemaphoreGreen = NULL;
static SemaphoreHandle_t xSemaphoreTasks ;
extern xSemaphoreHandle xSemaphoreSleep;


//Wait for xSemaphoreBlue, then Turn on blue LED for 30ms, then turn off.
void blueLEDTask( void *pvParameters) {
  TickType_t xNextWakeTime;
  xNextWakeTime = xTaskGetTickCount();
  
  for (;;) {
    if ( xSemaphoreTake(xSemaphoreBlue, portMAX_DELAY) == pdTRUE ) {
      xSemaphoreTake(xSemaphoreTasks, portMAX_DELAY );
      xNextWakeTime = xTaskGetTickCount();
      vTaskDelayUntil( &xNextWakeTime, delayms(3000) );
      xSemaphoreGive(xSemaphoreTasks);
    } 
  }
}

#define TASK_COUNT      ( 2 )
#define BLINK_DURATION  ( 5000 )

#include "data_types.h"
#include "gpio_lpc17xx.h"
#include "stdperip.h"

extern lpc17xx_gpio_config_t led2;
void greenLEDTask( void *pvParameters) {
  TimeOut_t xTimeOut;
  TickType_t xTicksToWait = delayms(BLINK_DURATION) ;
  TickType_t xNextWakeTime = xTaskGetTickCount();
  
  for (;;) {
	  vTaskDelayUntil( &xNextWakeTime, delayms(100) );
	  GPIO_SetLevel(&led2, LO);
	  vTaskDelayUntil( &xNextWakeTime, delayms(1000) );
	  GPIO_SetLevel(&led2, HI);
	  continue;
          if ( xSemaphoreTake(xSemaphoreGreen, portMAX_DELAY) == pdTRUE ) {
            //EXTI->IMR = 0;
            xTicksToWait = delayms(BLINK_DURATION) ;
            vTaskSetTimeOutState( &xTimeOut );
            xSemaphoreTake(xSemaphoreTasks, portMAX_DELAY );
            //Start Blinking.
            while (1) {
              xNextWakeTime = xTaskGetTickCount();
              vTaskDelayUntil( &xNextWakeTime, delayms(50) );
              xNextWakeTime = xTaskGetTickCount();
              vTaskDelayUntil( &xNextWakeTime, delayms(100) );
              if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdTRUE ) {
                break;
              }
            }
            xSemaphoreGive(xSemaphoreTasks);
          }
  }
}



extern lpc17xx_gpio_config_t led;
void usrButtonTask( void *pvParameters) {
  TickType_t xNextWakeTime = xTaskGetTickCount();
  for (;;) {
	  vTaskDelayUntil( &xNextWakeTime, delayms(100) );
	  GPIO_SetLevel(&led, HI);
	  vTaskDelayUntil( &xNextWakeTime, delayms(1000) );
	  GPIO_SetLevel(&led, LO);
#if (0)
   if (xSemaphoreTake(xSemaphoreMain, portMAX_DELAY)){
           //Give Semaphores for the tasks.
	   	   int i;
           for (i=0; i<TASK_COUNT; i++) {
                xSemaphoreGive(xSemaphoreTasks);
           }
     
            xSemaphoreGive(xSemaphoreBlue);
            xSemaphoreGive(xSemaphoreGreen);
            
            //Now take all Semaphores, block until everything has been given back.
            for (i=0; i<TASK_COUNT; i++) {
                  xSemaphoreTake(xSemaphoreTasks, portMAX_DELAY );
            }
            xSemaphoreGive(xSemaphoreSleep);
    } 
#endif
  }  
}


void main_blinky( void )
{
        static unsigned char ucParameterToPass;
        vSemaphoreCreateBinary(xSemaphoreMain);
        vSemaphoreCreateBinary(xSemaphoreBlue);
        vSemaphoreCreateBinary(xSemaphoreGreen);
        vSemaphoreCreateBinary(xSemaphoreSleep);
        xSemaphoreTasks = xSemaphoreCreateCounting( TASK_COUNT, 0 );
        
        /*
        if (xSemaphoreMain == NULL) { 
          for (;;);
        }
        if (xSemaphoreBlue == NULL) { 
          for (;;);
        }
        if (xSemaphoreGreen == NULL) { 
          for (;;) ;
        }
        if (xSemaphoreSleep == NULL) {
            for (;;);
        }
        if (xSemaphoreTasks == NULL) {
            for (;;);
        }
        
        xSemaphoreTake(xSemaphoreMain, 0);
        xSemaphoreTake(xSemaphoreBlue, 0);
        xSemaphoreTake(xSemaphoreGreen, 0);
        */
        if ( xTaskCreate(  usrButtonTask,					                                /* The function that implements the task. */
                      "BUT", 									/* The text name assigned to the task - for debug only as it is not used by the kernel. */
                      configMINIMAL_STACK_SIZE, 				                        /* The size of the stack to allocate to the task. */
                      ( void * ) &ucParameterToPass,                                           /* The parameter passed to the task - just to check the functionality. */
                      1, 		                                                                /* The priority assigned to the task. */
                      NULL ) != pdPASS ) {
                          for (;;);
        }
       
		#if (1)
        if ( xTaskCreate(  greenLEDTask,					                                /* The function that implements the task. */
                      "GRN", 									/* The text name assigned to the task - for debug only as it is not used by the kernel. */
                      configMINIMAL_STACK_SIZE, 				                        /* The size of the stack to allocate to the task. */
                      ( void * ) &ucParameterToPass,                                          /* The parameter passed to the task - just to check the functionality. */
                      2, 		                                                                /* The priority assigned to the task. */
                      NULL ) != pdPASS ) {
                          for (;;);
       }
	   #endif
#if (0)
        if ( xTaskCreate(  blueLEDTask,					                                /* The function that implements the task. */
                      "BLU", 									/* The text name assigned to the task - for debug only as it is not used by the kernel. */
                      configMINIMAL_STACK_SIZE, 				                        /* The size of the stack to allocate to the task. */
                      ( void * ) mainQUEUE_RECEIVE_PARAMETER,                                           /* The parameter passed to the task - just to check the functionality. */
                      2, 		                                                                /* The priority assigned to the task. */
                      NULL ) != pdPASS) {
                          for (;;);
        }
#endif    
         vTaskStartScheduler();
	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details. */
	for( ;; ) ;
}
/*-----------------------------------------------------------*/

