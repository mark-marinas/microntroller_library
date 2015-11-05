/*
 * stdirq.c
 *
 *  Created on: Oct 27, 2015
 *      Author: mark.marinas
 */

#include "peripheral_config.h"
#include "stdirq.h"

#if (mcu_name == LPC1768)
	#include "uart_lpc17xx.h"
	#include "i2c_lpc17xx.h"
	#include "gpio_lpc17xx.h"
	#include "spi_lpc17xx.h"
#endif


/*
 * **************************************************
 * 	GPIO INTERRUPT HANDLER
 * 	**************************************************
 */

//All GPIO Interrupts are mapped to EINT3_IRQHandler
//THis calls the Generic IRQ Handler, which in turn will call the specific IRQ Handler.
void EINT3_IRQHandler (void) {
	GPIO_Generic_IRQ_Handler();
}

/*
 * **************************************************
 * 	UART INTERRUPT HANDLERS
 * 	**************************************************
 */
extern void *uart_configs[] ;
#if (config_UART0_EN == 1)
void UART0_IRQHandler (void) {
	uart_config_t * _u = uart_configs[0];
	_u->irqhandler(_u);
}
#endif

#if (config_UART1_EN == 1)
void UART1_IRQHandler (void) {
	uart_config_t * _u = uart_configs[1];
	_u->irqhandler(_u);
}
#endif

#if (config_UART2_EN == 1)
void UART2_IRQHandler (void) {
	uart_config_t * _u = uart_configs[2];
	_u->irqhandler(_u);
}
#endif

#if (config_UART3_EN == 1)
void UART3_IRQHandler (void) {
	uart_config_t * _u = uart_configs[3];
	_u->irqhandler(_u);
}
#endif


/*
 * **************************************************
 * 	I2C INTERRUPT HANDLERS
 * 	**************************************************
 */
extern void *i2c_configs[] ;
#if config_I2C0_EN == 1
void I2C0_IRQHandler(void) {
	i2c_config_t * _i = i2c_configs[0];
	_i->irqhandler(_i);
}
#endif

#if config_I2C1_EN == 1
void I2C1_IRQHandler(void) {
	i2c_config_t * _i = i2c_configs[1];
	_i->irqhandler(_i);
}
#endif

#if config_I2C2_EN == 1
void I2C0_IRQHandler(void) {
	i2c_config_t * _i = i2c_configs[2];
	_i->irqhandler(_i);
}
#endif

/*
 * **************************************************
 * 	SPI INTERRUPT HANDLERS
 * 	**************************************************
 */
extern void *spi_configs[] ;
#if (config_SPI0_EN == 1)
void SPI_IRQHandler (void) {
	lpc17xx_spi_config_t *s = spi_configs[0];
	s->irqhandler(s);
}
#endif

#if (config_SSPI0_EN == 1)
void SSP0_IRQHandler (void) {
	lpc17xx_spi_config_t *s = spi_configs[1];
	s->irqhandler(s);
}
#endif

#if (config_SSPI1_EN == 1)
void SSP1_IRQHandler (void) {
	lpc17xx_spi_config_t *s = spi_configs[2];
	s->irqhandler(s);
}
#endif
