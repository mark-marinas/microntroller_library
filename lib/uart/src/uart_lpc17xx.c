/*
 * uart_lpc17xx.c
 *
 *  Created on: Oct 27, 2015
 *      Author: mark.marinas
 */


//TODO:
//Blocking should still have a timeout.
/*
 * There is a sample at UART_GetChar.
*/

#include "peripheral_config.h"
#include "data_types.h"
#include "utils.h"

#include "gpio_lpc17xx.h"
#include "uart_lpc17xx.h"
#include "clk_lpc17xx.h"

#if ( using_OS == freeRTOS)
	#include "stdirq.h"
	//	If using OS, create a thread that blocks until a data is received using
	//	xQueueReceive( uartX_notifier, &ulReceivedValue, portMAX_DELAY );
	#if (config_UART0_EN == 1)
		QueueHandle_t uart0_notifier =  NULL  ;
	#endif
	#if (config_UART1_EN == 1)
		QueueHandle_t uart1_notifier =  NULL  ;
	#endif
	#if (config_UART2_EN == 1)
		QueueHandle_t uart2_notifier =  NULL  ;
	#endif
	#if (config_UART3_EN == 1)
		QueueHandle_t uart3_notifier =  NULL  ;
	#endif
#elif (using_OS == no_os)
	//If no OS, just read the FIFO whenever data is needed.
#else
	#error "Unknown OS"
#endif


#define IIR_RLS			3
#define IIR_RDA			2
#define IIR_CTI			6
#define IIR_THRE		1
#define IIR_UNKNOWN		(IIR_RLS + IIR_RDA + IIR_CTI + IIR_THRE)

#if (config_UART0_EN == 1)
	#if (config_UART_Buffer_Size == 0)
		#error "Buffer Size Undefined"
	#else
		char tx0_buffer[config_UART_Buffer_Size];
		char rx0_buffer[config_UART_Buffer_Size];
		uart_buffer_ptr_t uart0_fifo;
	#endif
#endif

#if (config_UART1_EN == 1)
	#if (config_UART_Buffer_Size == 0)
		#error "Buffer Size Undefined"
	#else
		char tx1_buffer[config_UART_Buffer_Size];
		char rx1_buffer[config_UART_Buffer_Size];
		uart_buffer_ptr_t uart1_fifo;
	#endif
#endif

#if (config_UART2_EN == 1)
	#if (config_UART_Buffer_Size == 0)
		#error "Buffer Size Undefined"
	#else
		char tx2_buffer[config_UART_Buffer_Size];
		char rx2_buffer[config_UART_Buffer_Size];
		uart_buffer_ptr_t uart2_fifo;
	#endif
#endif

#if (config_UART3_EN == 1)
	#if (config_UART_Buffer_Size == 0)
		#error "Buffer Size Undefined"
	#else
		char tx3_buffer[config_UART_Buffer_Size];
		char rx3_buffer[config_UART_Buffer_Size];
		uart_buffer_ptr_t uart3_fifo;
	#endif
#endif



void *uart_configs[config_UART0_EN + config_UART1_EN + config_UART2_EN + config_UART3_EN] = { 0 } ;

#if (config_UART_PostHook == 1)
extern error_code_t UART_Config_PostHook(void *config);
#endif


typedef struct {
	pin_func_t func;
	lpc17xx_gpio_config_t tx_pin;
	lpc17xx_gpio_config_t rx_pin;
	int					  clk_lsb;
	int					  pconp_lsb;
} uart_map_t;


#if (config_UART_GetDivHook == 0)
const uart_fractional_divider_t uartLCR[] = {
	{ PCLKDIV_BY_4, B2400,   2, 2, 4, 15   },
	{ PCLKDIV_BY_4, B9600,   0, 92, 10, 13 },
	{ PCLKDIV_BY_4, B115200, 0, 10, 5, 14  },
	{-1, 0,      0, 0,  0, 0   },
} ;

uart_fractional_divider_t *UART_GetDividers(baudrate_t baud) {
	uart_fractional_divider_t *div = (uart_fractional_divider_t *) uartLCR ;
	if (SystemFrequency != 100e6) {
		//return 0;
	}
	while (div->clkdiv != -1) {
			if (div->baud == baud) {
					return div;
			}
			div++;
	}
	return 0;
}
#else
extern const uart_fractional_divider_t uartLCR[];
extern uart_fractional_divider_t *UART_GetDividers(baudrate_t baud);
#endif

static const uart_map_t lpc17xx_uart_map[UART_COUNT] = {
		  { ALT1_FUNC,
			{ PORT0, PIN2, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
			{ PORT0, PIN3,  INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
			6,
			3
		  }
		 ,{	ALT1_FUNC,
			{ PORT0, PIN15, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		    { PORT0, PIN16,  INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		    8,
		    4
		  }
		 ,{	ALT1_FUNC,
			{ PORT0, PIN10, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
			{ PORT0, PIN11,  INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
			16,
			24
		  }
		 ,{	ALT2_FUNC,
			{ PORT0,  PIN0, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
			{ PORT0,  PIN1,  INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
			18,
			25
		  }
};


/* Function Prototype */
static int UART_GetIRQ_Type (uart_port_t uart_port);
static char uart_rx(uart_port_t uart_port);
static void uart_tx(uart_port_t uart_port, char ch);
static void disable_thre(uart_port_t uart_port);
static void enable_thre(uart_port_t uart_port);
static void	uart_irqhandler_default (void *data);
static error_code_t Uart_Config_Pins(uart_port_t uart_port);
static void wait_thre_disabled(uart_port_t uart_port);


/* Codes */
static int UART_GetIRQ_Type (uart_port_t uart_port) {
	error_code_t error = NO_ERROR;
	uint32_t iir_type;
	switch (uart_port) {
		case COM0:
			error = ReadReg( (volatile uint32_t *) &(LPC_UART0->IIR), &iir_type, 1, 3 );
			break;
		case COM1:
			error = ReadReg( (volatile uint32_t *) &(LPC_UART1->IIR), &iir_type, 1, 3 );
			break;
		case COM2:
			error = ReadReg( (volatile uint32_t *)&(LPC_UART2->IIR), &iir_type, 1, 3 );
			break;
		case COM3:
			error = ReadReg( (volatile uint32_t *)&(LPC_UART3->IIR), &iir_type, 1, 3 );
			break;
		default:
			error = NO_ERROR + 1;
			break;
	}
	if (error != NO_ERROR) {
		return IIR_UNKNOWN;
	}
	return iir_type;
}

static char uart_rx(uart_port_t uart_port) {
	switch (uart_port) {
		case COM0:
			return ( LPC_UART0->RBR );
			break;
		case COM1:
			return ( LPC_UART1->RBR );
			break;
		case COM2:
			return ( LPC_UART2->RBR );
			break;
		case COM3:
			return ( LPC_UART3->RBR );
			break;
	}
	return 0;
}

static void uart_tx(uart_port_t uart_port, char ch) {
	switch (uart_port) {
		case COM0:
			LPC_UART0->THR = ch;
			break;
		case COM1:
			LPC_UART1->THR = ch;
			break;
		case COM2:
			LPC_UART2->THR = ch;
			break;
		case COM3:
			LPC_UART3->THR = ch;
			break;
	}
}

static void disable_thre(uart_port_t uart_port) {
	switch (uart_port) {
		case COM0:
			WriteReg( &(LPC_UART0->IER), 0, 1, 1 );
			break;
		case COM1:
			WriteReg( &(LPC_UART1->IER), 0, 1, 1 );
			break;
		case COM2:
			WriteReg( &(LPC_UART2->IER), 0, 1, 1 );
			break;
		case COM3:
			WriteReg( &(LPC_UART3->IER), 0, 1, 1 );
			break;
	}
}

static void enable_thre(uart_port_t uart_port) {
	switch (uart_port) {
		case COM0:
			WriteReg( &(LPC_UART0->IER), 1, 1, 1 );
			break;
		case COM1:
			WriteReg( &(LPC_UART1->IER), 1, 1, 1 );
			break;
		case COM2:
			WriteReg( &(LPC_UART2->IER), 1, 1, 1 );
			break;
		case COM3:
			WriteReg( &(LPC_UART3->IER), 1, 1, 1 );
			break;
	}
}

static void	uart_irqhandler_default (void *data) {
	lpc17xx_uart_config_t *_config = data;
	int interrupt_type = UART_GetIRQ_Type(_config->uart_port) ;
	error_code_t error;
	char ch;

	switch (interrupt_type) {
		case IIR_RLS:
			break;
		case IIR_RDA:
			break;
		case IIR_CTI:
			ch = uart_rx(_config->uart_port);
			if ((error = FIFO_Put(&(_config->buffer->uart_rx_fifo), &ch)) != NO_ERROR ) { //RX FIFO is full.

			} else {
				error = FIFO_NOT_EMPTY;
			}
			#if (using_OS == freeRTOS)
				switch (_config->uart_port) {
					#if	(config_UART0_EN == 1)
					case COM0:
						xQueueSend( uart0_notifier, &error, 0U );
						break;
					#endif
					#if	(config_UART1_EN == 1)
					case COM1:
						xQueueSend( uart1_notifier, &error, 0U );
						break;
					#endif
					#if	(config_UART2_EN == 1)
					case COM2:
						xQueueSend( uart2_notifier, &error, 0U );
						break;
					#endif
					#if	(config_UART3_EN == 1)
					case COM3:
						xQueueSend( uart3_notifier, &error, 0U );
						break;
					#endif
				}
			#elif (using_os == no_os)

			#else
				#error "Unknow OS"
			#endif
			break;
		case IIR_THRE:
			if ((error = FIFO_Get(&(_config->buffer->uart_tx_fifo), &ch)) != NO_ERROR ) { //TX FIFO is empty.
				disable_thre(_config->uart_port);
			} else {
				uart_tx(_config->uart_port, ch);
			}
			break;
	}
}

static error_code_t Uart_Config_Pins(uart_port_t uart_port) {
	//Dont use GPIO_Config, as it could have a post hook.
	error_code_t error = NO_ERROR;

	uart_map_t uart_pin = lpc17xx_uart_map[uart_port];
	if ( (error = GPIO_Set_Func( &(uart_pin.tx_pin), uart_pin.func ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Func( &(uart_pin.rx_pin), uart_pin.func ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Mode( &(uart_pin.tx_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Mode( &(uart_pin.rx_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_OpenDrain_Mode( &(uart_pin.tx_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_OpenDrain_Mode( &(uart_pin.rx_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Direction( &(uart_pin.tx_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Direction( &(uart_pin.rx_pin) ) ) != NO_ERROR ) {
		return error;
	}
	return error;
}


static void wait_thre_disabled(uart_port_t uart_port) {
	uint32_t thre_status;

	switch (uart_port) {
		case COM0:
			ReadReg( (volatile uint32_t *) &(LPC_UART0)->IER, &thre_status, 1, 1) ;
			while (thre_status) {
				ReadReg( (volatile uint32_t *) &(LPC_UART0->IER), &thre_status, 1, 1) ;
			}
			break;
		case COM1:
			ReadReg( (volatile uint32_t *) &(LPC_UART1->IER), &thre_status, 1, 1) ;
			while (thre_status) {
				ReadReg( (volatile uint32_t *) &(LPC_UART1->IER), &thre_status, 1, 1) ;
			}
			break;
		case COM2:
			ReadReg( (volatile uint32_t *) &(LPC_UART2->IER), &thre_status, 1, 1) ;
			while (thre_status) {
				ReadReg( (volatile uint32_t *) &(LPC_UART2->IER), &thre_status, 1, 1) ;
			}
			break;
		case COM3:
			ReadReg( (volatile uint32_t *) &(LPC_UART3->IER), &thre_status, 1, 1) ;
			while (thre_status) {
				ReadReg( (volatile uint32_t *) &(LPC_UART3->IER), &thre_status, 1, 1) ;
			}
			break;
	}
}


error_code_t	UART_Config(void *config) {
	error_code_t	error = NO_ERROR;
	lpc17xx_uart_config_t *_config = config;
	uart_fractional_divider_t *div;

	//Get the Fractional Dividers
	if ( (div = UART_GetDividers(_config->baudrate) ) == 0 ) {
		return INVALID_BAUD_RATE ;
	}

	//Configure UART Pins
	if ( (error = Uart_Config_Pins(_config->uart_port)) != NO_ERROR ) {
		return error;
	}

	//PCONP
	if ( ( error = WriteReg ( &(LPC_SC->PCONP), 1, lpc17xx_uart_map[_config->uart_port].pconp_lsb, lpc17xx_uart_map[_config->uart_port].pconp_lsb) ) != NO_ERROR ) {
		return error;
	}

	//PCLK
	switch (_config->uart_port) {
		case COM0:
		case COM1:
			error = WriteReg (&(LPC_SC->PCLKSEL0), div->clkdiv, lpc17xx_uart_map[_config->uart_port].clk_lsb, lpc17xx_uart_map[_config->uart_port].clk_lsb + 1 );
			break;
		case COM2:
		case COM3:
			error = WriteReg (&(LPC_SC->PCLKSEL1), div->clkdiv, lpc17xx_uart_map[_config->uart_port].clk_lsb, lpc17xx_uart_map[_config->uart_port].clk_lsb + 1 );
			break;
	}
	if (error != NO_ERROR) {
		return error;
	}

	volatile LPC_UART_TypeDef      *uart_typedef;
	volatile LPC_UART1_TypeDef    *uart1_typedef;
	switch (_config->uart_port) {
		case COM0:
			uart_typedef = (LPC_UART_TypeDef *) LPC_UART0;
			break;
		case COM1:
			uart1_typedef = (LPC_UART1_TypeDef *) LPC_UART1;
			break;
		case COM2:
			uart_typedef = (LPC_UART_TypeDef *) LPC_UART2;
			break;
		case COM3:
			uart_typedef = (LPC_UART_TypeDef *) LPC_UART3;
			break;
	}

	if ( _config->uart_port != COM1) {
		error  = 	WriteReg8( &uart_typedef->LCR, 1, 7, 7 ); 			//DLAB.
		error |= 	WriteReg8( &uart_typedef->DLL,div->udll, 0, 7 ); 	//DLL
		error |=	WriteReg8( &uart_typedef->DLM,div->udlm, 0, 7 );	//DLM
		error |=	WriteReg8( &uart_typedef->FDR,div->divAddVal, 0, 3 );//DIVADD
		error |= 	WriteReg8( &uart_typedef->FDR,div->MulVal, 4, 7 );	 //DIVMULVAL
		error |=	WriteReg8( &uart_typedef->LCR,_config->wordlen, 0, 1);//WORDLEN
		error |=    WriteReg8( &uart_typedef->LCR,_config->stopbit, 2, 2);//STOPBITS
		if (_config->parity == UART_PARITY_NONE) {
			error |= WriteReg8( &uart_typedef->LCR, 0, 3, 3 );			  //PARITYENABLE
		} else {
			error |= WriteReg8( &uart_typedef->LCR, 1, 3, 3 );			  //PARITYENABLE
			error |= WriteReg8( &uart_typedef->LCR,_config->parity, 4, 5);//PARITY
		}
		error |= WriteReg8 ( &uart_typedef->FCR, 0x00, 6, 7) ;			  //RX-TRIGGER LEVEL, always use 1 bit. If more is needed, use POST-HOOK.
		error |= WriteReg8 ( &uart_typedef->FCR, 0x3, 1, 2);			  //RESET RX/TX FIFO
		error |= WriteReg8 ( &uart_typedef->FCR, 1, 0, 0);				  //ENABLE FIFO.
		error |= WriteReg8 ( &uart_typedef->FCR, 0, 3, 3);			      //DMA-MODE, NONE.
		error |= WriteReg8 ( &uart_typedef->LCR, 0, 7, 7 ); 			  //DLAB.
		error |= WriteReg  ( &uart_typedef->IER, 1, 0, 0); 			  //RBR IRQ
	} else {
		error  = 	WriteReg8( &uart1_typedef->LCR, 1, 7, 7 ); 			//DLAB.
		error |= 	WriteReg8( &uart1_typedef->DLL,div->udll, 0, 7 ); 	//DLL
		error |=	WriteReg8( &uart1_typedef->DLM,div->udlm, 0, 7 );	//DLM
		error |=	WriteReg( &uart1_typedef->FDR,div->divAddVal, 0, 3 );//DIVADD
		error |= 	WriteReg( &uart1_typedef->FDR,div->MulVal, 4, 7 );	 //DIVMULVAL
		error |=	WriteReg8( &uart1_typedef->LCR,_config->wordlen, 0, 1);//WORDLEN
		error |=    WriteReg8( &uart1_typedef->LCR,_config->stopbit, 2, 2);//STOPBITS
		if (_config->parity == UART_PARITY_NONE) {
			error |= WriteReg8( &uart1_typedef->LCR, 0, 3, 3 );			  //PARITYENABLE
		} else {
			error |= WriteReg8( &uart1_typedef->LCR, 1, 3, 3 );			  //PARITYENABLE
			error |= WriteReg8( &uart1_typedef->LCR,_config->parity, 4, 5);//PARITY
		}
		error |= WriteReg8 ( &uart1_typedef->FCR, 0x00, 6, 7) ;			  //RX-TRIGGER LEVEL, always use 1 bit. If more is needed, use POST-HOOK.
		error |= WriteReg8 ( &uart1_typedef->FCR, 0x3, 1, 2);			  //RESET RX/TX FIFO
		error |= WriteReg8 ( &uart1_typedef->FCR, 0, 3, 3);			      //DMA-MODE, NONE.
		error |= WriteReg8 ( &uart1_typedef->FCR, 1, 0, 0);				  //ENABLE FIFO.
		error |= WriteReg8 ( &uart1_typedef->LCR, 0, 7, 7 ); 			  //DLAB.
		error |= WriteReg  ( &uart1_typedef->IER, 1, 0, 0); 			  //RBR IRQ
	}
	if (error != NO_ERROR) {
		return error;
	}


	//Assign the transmit and receive buffers.
	switch (_config->uart_port) {
		case COM0:
			#if (config_UART0_EN == 1)
				error  = FIFO_Init( &(uart0_fifo.uart_tx_fifo), config_UART_Buffer_Size, tx0_buffer, 0, 0);
				error |= FIFO_Init( &(uart0_fifo.uart_rx_fifo), config_UART_Buffer_Size, rx0_buffer, 0, 0);
				_config->buffer = &uart0_fifo;
			#else
				return UNDEFINED_UART_PORT;
			#endif
			break;
		case COM1:
			#if (config_UART1_EN == 1)
				error  = FIFO_Init( &(uart1_fifo.uart_tx_fifo), config_UART_Buffer_Size, tx1_buffer, 0, 0);
				error |= FIFO_Init( &(uart1_fifo.uart_rx_fifo), config_UART_Buffer_Size, rx1_buffer, 0, 0);
				_config->buffer = &uart1_fifo;
			#else
				return UNDEFINED_UART_PORT;
			#endif
			break;
		case COM2:
			#if (config_UART2_EN == 1)
				error  = FIFO_Init( &(uart2_fifo.uart_tx_fifo), config_UART_Buffer_Size, tx2_buffer, 0, 0);
				error |= FIFO_Init( &(uart2_fifo.uart_rx_fifo), config_UART_Buffer_Size, rx2_buffer, 0, 0);
				_config->buffer = &uart2_fifo;
			#else
				return UNDEFINED_UART_PORT;
			#endif
			break;
		case COM3:
			#if (config_UART3_EN == 1)
				error  = FIFO_Init( &(uart3_fifo.uart_tx_fifo), config_UART_Buffer_Size, tx3_buffer, 0, 0);
				error |= FIFO_Init( &(uart3_fifo.uart_rx_fifo), config_UART_Buffer_Size, rx3_buffer, 0, 0);
				_config->buffer = &uart3_fifo;
			#else
				return UNDEFINED_UART_PORT;
			#endif
			break;
		default:
			return UNDEFINED_UART_PORT;
	}

	if (error != NO_ERROR) {
		return error;
	}

	NVIC_EnableIRQ(UART0_IRQn + _config->uart_port);
	#if ( using_OS == freeRTOS)
		switch (_config->uart_port) {
			#if (config_UART0_EN == 1)
			case COM0:
					uart0_notifier = xQueueCreate( 1, sizeof( uint32_t ) );
					break;
			#endif
			#if (config_UART1_EN == 1)
			case COM1:
					uart1_notifier = xQueueCreate( 1, sizeof( uint32_t ) );
					break;
			#endif
			#if (config_UART2_EN == 1)
			case COM2:
					uart2_notifier = xQueueCreate( 1, sizeof( uint32_t ) );
					break;
			#endif
			#if (config_UART3_EN == 1)
			case COM3:
					uart3_notifier = xQueueCreate( 1, sizeof( uint32_t ) );
					break;
			#endif
		}
	#elif (using_OS == no_os)

	#else
		#error "Unknown OS"
	#endif

	#if (config_UART_PostHook == 1)
		if ( (error = UART_Config_PostHook(config)) != NO_ERROR ) {
			return error;
		}
	#endif

	if (_config->irqhandler == 0) {
		_config->irqhandler = uart_irqhandler_default ;
	}

	uart_configs[_config->uart_port] = config;

	return error;
}



//TODO: This shouldn't be implemented here. Put it in the timer section.
typedef struct {
	int t1, t2;
} time_t;

void GetTime(time_t *t) {

}

float difftime(time_t t1, time_t t2) {
	return 0.0 ;
}

error_code_t	LPC17XX_UART_GetChar_Default(int uart_port, char *data) {
	lpc17xx_uart_config_t *_config = uart_configs[uart_port] ;

	if (_config == 0) {
		return UNITIALIZED_UART;
	}


	if (_config->block_type == BLOCKING ) {

			while ( FIFO_Get( &(_config->buffer->uart_rx_fifo), data) == FIFO_EMPTY ) ;
			return NO_ERROR;
			//TODO: Even blocking should timeout.
			/*
			error_code_t error = TIMEOUT;
			exec(FIFO_Get( &(_config->buffer->uart_rx_fifo), data), 10);
			return error;
			*/
	} else {
		return ( FIFO_Get( &(_config->buffer->uart_rx_fifo), data) );
	}
}



error_code_t	LPC17XX_UART_PutChar_Default(int uart_port, char  data) {
	lpc17xx_uart_config_t *_config = uart_configs[uart_port];

	if (_config == 0) {
		return UNITIALIZED_UART;
	}

	char first_data;
	wait_thre_disabled(uart_port);
	while ( FIFO_Put( &(_config->buffer->uart_tx_fifo), &data) == FIFO_FULL) ;
	enable_thre(uart_port);
	FIFO_Get(&(_config->buffer->uart_tx_fifo), &first_data);
	uart_tx(_config->uart_port, first_data);

	//If Blocking, wait till everything is sent.
	if (_config->block_type == BLOCKING) {
		wait_thre_disabled(uart_port);
	}

	return NO_ERROR;
}

error_code_t	LPC17XX_UART_GetChars_Default(int uart_port, char *data, int size) {
	int _size = size;
	int index = 0;
	error_code_t error;
	while ( _size > 0) {
		if ( ( error = UART_GetChar(uart_port, &(data[index]))) != NO_ERROR ) {
			return error;
		} else {
			index ++;
			size --;
		}
	}
	return NO_ERROR;
}

error_code_t	LPC17XX_UART_PutChars_Default(int uart_port, char *data, int size) {
	lpc17xx_uart_config_t *_config = uart_configs[uart_port];
	int index = 0;
	int _size = size;
	char first_data;

	if (_config == 0) {
		return UNITIALIZED_UART;
	}

	while (_size > 0) {
		wait_thre_disabled(uart_port);
		while ( FIFO_Put( &(_config->buffer->uart_tx_fifo), &data[index]) != FIFO_FULL) {
			_size--;
			index++;
			if (_size == 0) {
				break;
			}
		}
		enable_thre(uart_port);
		FIFO_Get(&(_config->buffer->uart_tx_fifo), &first_data);
		uart_tx(_config->uart_port, first_data);
	}
	wait_thre_disabled(uart_port);
	return NO_ERROR;
}
