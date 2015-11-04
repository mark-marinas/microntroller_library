/*
 * peripheral_config.h
 *
 *  Created on: Oct 27, 2015
 *      Author: mark.marinas
 */

#ifndef PERIPHERAL_CONFIG_H_
#define PERIPHERAL_CONFIG_H_

#include "data_types.h"

#define LPC1768							1


#define mcu_name					LPC1768
#define using_OS						0

//GPIO Configs.
#define	config_GPIO_PostHook			0
#define config_GPIO_Interrupt_Count		1
#define GPIO_Generic_IRQ_Handler		LPC17XX_GPIO_IRQ_Handler_Default
#define GPIO_GetIRQ						LPC17XX_GPIO_GetIRQ_Default
#define GPIO_ClrIRQ						LPC17XX_GPIO_GPIO_ClrIRQ_Default

//UART Configs
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

//I2C
#define config_I2C_PostHook				0
#define config_I2C_GetDivHook			0
#define config_I2C0_EN					1
#define config_I2C1_EN					0
#define config_I2C2_EN					0
#define I2C_Read						LPC17XX_I2C_Read_default
#define I2C_Write						LPC17XX_I2C_Write_default

#endif /* PERIPHERAL_CONFIG_H_ */
