/*
 * lpc17xx_spi.h
 *
 *  Created on: Nov 5, 2015
 *      Author: mark.marinas
 */

#ifndef LPC17XX_SPI_H_
#define LPC17XX_SPI_H_

#include <stdint.h>

#define MIN_COUNTER	8

typedef enum {
	SPI0,
	SSPI0,
	SSPI1
} spi_port_t;

typedef enum {
	SPI_CLK_RISING,
	SPI_CLK_FALLING
} spi_cpol_t;


typedef enum {
	SPI_PHASE_INPHASE,
	SPI_PHASE_OUTOFPHASE
} spi_cpha_t;

typedef enum {
	MSB_FIRST,
	LSB_FIRST
} spi_lsbf_t;

typedef enum {
	SPI_SLAVE,
	SPI_MASTER
} spi_mode_t;

typedef enum {
	SPI_16_BITS,
	SPI_8_BITS = 8,
	SPI_9_BITS,
	SPI_10_BITS,
	SPI_11_BITS,
	SPI_12_BITS,
	SPI_13_BITS,
	SPI_14_BITS,
	SPI_15_BITS,
} spi_bitlen_t;

typedef struct {
	spi_port_t		port;
	spi_cpol_t 		clk_polarity;
	spi_cpha_t 		clk_phase;
	active_signal_level_t slave_sel_polarity;
	spi_lsbf_t 		lsbf;
	spi_mode_t		mode;
	spi_bitlen_t	bits;
	uint32_t		freq;
	void			(*irqhandler)(void *);
} lpc17xx_spi_config_t;

typedef lpc17xx_spi_config_t spi_config_t;

#endif /* LPC17XX_SPI_H_ */
