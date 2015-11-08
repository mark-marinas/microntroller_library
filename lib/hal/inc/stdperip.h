/*
 * stdperip.h
 *
 *  Created on: Oct 26, 2015
 *      Author: mark.marinas
 */

#ifndef STDPERIP_H_
#define STDPERIP_H_

#include "data_types.h"
#include "peripheral_config.h"

//GPIO
error_code_t 	GPIO_Config(void *config);
error_code_t	GPIO_GetLevel(void *config, signal_level_t *val);
error_code_t	GPIO_SetLevel(void *config, signal_level_t  val);

//UART
error_code_t	UART_Config(void *config);
error_code_t	UART_GetChar(int uart_port, char *data);
error_code_t	UART_PutChar(int uart_port, char  data);
error_code_t	UART_GetChars(int uart_port, char *data, int size);
error_code_t	UART_PutChars(int uart_port, char *data, int size);

//SPI
error_code_t	SPI_Config(void *config);
error_code_t	SPI_Read (int spi_port, void *data);
error_code_t	SPI_Write(int spi_port, void *data);

//I2C
error_code_t	I2C_Config(void *config);
error_code_t	I2C_Read(int i2c_port, void *data);
error_code_t	I2C_Write(int i2c_port, void *data);


#endif /* STDPERIP_H_ */
