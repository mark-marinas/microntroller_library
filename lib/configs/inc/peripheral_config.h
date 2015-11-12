/*
 * peripheral_config.h
 *
 *  Created on: Oct 27, 2015
 *      Author: mmarinas
 */

#ifndef PERIPHERAL_CONFIG_H_
#define PERIPHERAL_CONFIG_H_

#include "data_types.h"

// List all supported mcus here. *
#define LPC1768							1

// List all supported OSes here. **
#define no_os							0
#define freeRTOS						1

// mcu_name - name of the mcu, pick from the list of supported mcus *.
// using_OS - os to use, pick from the list of supported OSes **
// 			  if running on OS, modify the makefile to include the .c/.h for the OS.
#define mcu_name					LPC1768
#define using_OS					no_os

//GPIO Configs.
// config_GPIO_PostHook 		- if 1, will call UART_Config_PostHook (should be implemented by the user) after configuring the GPIO Pin.
// config_GPIO_Interrupt_Count 	- number of Pins in the project to be used in interrupt mode. Needed to statically determine the size of interrupt handlers.
// GPIO_Generic_IRQ_Handler		- for LPC1768 (not sure for other mcus), all GPIO interrupts are routed to EINT3. This is the Generic handler for the EINT3.
//								  It only determines wihch pin interrupted, and calls the corresponding pin's interrupt handler. Replace this to customize.
// GPIO_GetIRQ					- Determine if there is pending interrupt for a pin. Replace this to customize.
// GPIO_ClrIRQ					- Clears the Pending interrupt for a GPIO. This is not related to the actual HARDWARE interrupt bit.
//								  The HARDWARE interrupt bit should be automatically cleared by the interrupt handler.
//							      Use this only in cases where the pending interrupt is being monitored.
#define	config_GPIO_PostHook			0
#define config_GPIO_Interrupt_Count		1
#define GPIO_Generic_IRQ_Handler		LPC17XX_GPIO_IRQ_Handler_Default
#define GPIO_GetIRQ						LPC17XX_GPIO_GetIRQ_Default
#define GPIO_ClrIRQ						LPC17XX_GPIO_GPIO_ClrIRQ_Default

//UART Configs
// config_UART_PostHook 			- same use as config_GPIO_PostHook, but only this time its for UART.
// config_UART_GetDivHook 			- pre-calculated values for DLL/DLM/udivMul/udivAdd, based on the assumption that system clock is running at 100MHz. Set this to 1 if you wish to use a different value.
// config_UART_Buffer_Size			- uart uses fifo for tx and rx, and on interrupt mode. This value is the size of the fifo.
// config_UART[X]_EN				- 1 if uart[x] is enabled. Needed to determine the size of the interrupt callback functions.
// UART_GetChar(s)/UART_PutChar(s) 	- default GetChar/PutChar. Change if you wish to implement your own.
#define config_UART_PostHook			0
#define config_UART_GetDivHook			0
#define	config_UART_Buffer_Size			128

#define config_UART0_EN					1
#define config_UART1_EN					0
#define config_UART2_EN					0
#define config_UART3_EN					0
#define UART_GetChar					LPC17XX_UART_GetChar_Default
#define UART_PutChar					LPC17XX_UART_PutChar_Default
#define UART_GetChars					LPC17XX_UART_GetChars_Default
#define UART_PutChars					LPC17XX_UART_PutChars_Default

//I2C Configs.
// Refer to UART. Same explanation.
#define config_I2C_PostHook				0
#define config_I2C_GetDivHook			0
#define config_I2C0_EN					1
#define config_I2C1_EN					0
#define config_I2C2_EN					0
#define I2C_Read						LPC17XX_I2C_Read_default
#define I2C_Write						LPC17XX_I2C_Write_default

//SPI Configs.
// Refer to UART. Same explanation.
#define	config_SPI_PostHook				0
#define config_SPI_GetDivHook			0

#define	config_SPI0_EN					1
#define config_SSPI0_EN					0
#define config_SSPI1_EN					0
#define SPI_Read						LPC17XX_SPI_Read_default
#define SPI_Write						LPC17XX_SPI_Write_default

//ADC
#define config_ADC_PostHook				0
#define	config_ADC0_En					0
#define	config_ADC1_En					0
#define	config_ADC2_En					0
#define	config_ADC3_En					0
#define	config_ADC4_En					0
#define	config_ADC5_En					1
#define	config_ADC6_En					0
#define	config_ADC7_En					0

#define ADC_Generic_IRQ_Handler			LPC17XX_ADC_IRQ_Handler_Default

#endif /* PERIPHERAL_CONFIG_H_ */
