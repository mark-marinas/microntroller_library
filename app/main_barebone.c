/*
 * main_barebone.c
 *
 *  Created on: Nov 4, 2015
 *      Author: mark.marinas
 */



#include "LPC17xx.h"
#include "stdperip.h"
#include "gpio_lpc17xx.h"
#include "stdirq.h"
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
	command.size = 1 ;
	for (j=0; j<1; j++) {
		for (i=0; i<size;i++) {
				fw_version_read[i] = 'X';
		}
		for (i=0; i<size;i++) {
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
	UART_PutChars(COM0, &(fw_version_read[0]), size);
	UART_PutChars(COM0, (char*) "\n\r", 2);

	pin_interrupt_type_t key1_status;
	char rising_str[] = "Rising  Edge\n\r";
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

	#if (UART_TEST == 1)
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
	#endif



	return 0;
}




