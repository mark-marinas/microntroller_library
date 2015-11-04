/*
 * uart_lpc17xx.h
 *
 *  Created on: Oct 27, 2015
 *      Author: mark.marinas
 */
#include "data_types.h"
#include "fifo.h"

#ifndef UART_LPC17XX_H_
#define UART_LPC17XX_H_

#define UART_COUNT	4

typedef enum {
	COM0,
	COM1,
	COM2,
	COM3
} uart_port_t;

typedef enum {
	B2400,
	B9600,
	B115200,
	BAUTO
} baudrate_t;

typedef enum {
	UART_WORDLEN5,
	UART_WORDLEN6,
	UART_WORDLEN7,
	UART_WORDLEN8,
} uart_wordlen_t;

typedef enum {
	STOP_BIT_1_BIT,
	STOP_BIT_2_BITS,
} uart_stop_bit_t;

typedef enum {
	UART_PARITY_ODD,
	UART_PARITY_EVEN,
	UART_PARITY_FORCED_1,
	UART_PARITY_FORCED_0,
	UART_PARITY_NONE
} uart_parity_t;

typedef enum {
	UART_BREAK_CONTROL_DISABLE,
	UART_BREAK_CONTROL_ENABLE
} uart_break_control_t;

typedef struct {
	int 		clkdiv;
	baudrate_t	baud;
	int 		udlm, udll;
	int 		divAddVal, 	MulVal;
} uart_fractional_divider_t;


typedef struct {
	fifo_t uart_tx_fifo;
	fifo_t uart_rx_fifo;
} uart_buffer_ptr_t;

typedef struct {
	uart_port_t		uart_port;
	baudrate_t 		baudrate;
	uart_wordlen_t  wordlen;
	uart_stop_bit_t stopbit;
	uart_parity_t	parity;
	blocking_type_t block_type;
	uart_buffer_ptr_t *buffer;
	void			(*irqhandler)(void *);
} lpc17xx_uart_config_t;

typedef lpc17xx_uart_config_t uart_config_t;



#endif /* UART_LPC17XX_H_ */
