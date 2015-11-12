/*
 * stdirq.h
 *
 *  Created on: Oct 29, 2015
 *      Author: mmarinas
 */

#ifndef STDIRQ_H_
#define STDIRQ_H_

#include "peripheral_config.h"

#if ( using_OS == freeRTOS)
	//	If using OS, create a thread that blocks until a data is received using
	//	xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );
	#include "FreeRTOS.h"
	#include "queue.h"

	#if (config_UART0_EN == 1)
		extern QueueHandle_t uart0_notifier;
	#endif
	#if (config_UART1_EN == 1)
		extern QueueHandle_t uart1_notifier;
	#endif
	#if (config_UART2_EN == 1)
		extern QueueHandle_t uart2_notifier;
	#endif
	#if (config_UART3_EN == 1)
		extern QueueHandle_t uart3_notifier;
	#endif

	//	xQueueReceive( gpio_notifier[idx], &ulReceivedValue, portMAX_DELAY );
	#if (config_GPIO_Interrupt_Count > 0)
		extern QueueHandle_t gpio_notifier[];
	#endif
#elif (using_OS == no_os)

#else
	#error "Unknown OS"
#endif

error_code_t	GPIO_GetIRQ(void *config, void *status);
error_code_t	GPIO_ClrIRQ(void *config);


#endif /* STDIRQ_H_ */
