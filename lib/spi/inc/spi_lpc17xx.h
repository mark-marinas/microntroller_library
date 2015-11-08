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
	SSPI_4_BITS = 3,
	SSPI_5_BITS,
	SSPI_6_BITS,
	SSPI_7_BITS,
	SSPI_8_BITS,
	SSPI_9_BITS,
	SSPI_10_BITS,
	SSPI_11_BITS,
	SSPI_12_BITS,
	SSPI_13_BITS,
	SSPI_14_BITS,
	SSPI_15_BITS,
	SSPI_16_BITS,
} spi_bitlen_t;

typedef struct {
	spi_port_t		port;
	spi_cpol_t 		clk_polarity;
	spi_cpha_t 		clk_phase;
	spi_lsbf_t 		lsbf;
	spi_mode_t		mode;
	spi_bitlen_t	bits;
	uint32_t		freq;
	void 			*buffer;
	void			(*irqhandler)(void *);
	uint16_t	dummyData;
} lpc17xx_spi_config_t;

typedef lpc17xx_spi_config_t spi_config_t;

typedef enum {
	SPI_READ = 1,
	SPI_WRITE
} spi_operation_t;

typedef enum {
	SPI_BUSY,
	SPI_READY_PASS,
	SPI_READY_FAIL
} spi_status_t;

typedef struct {
	uint16_t writeReg;
	uint16_t *writeBuffer;
	int			  writeDataSize;
	int				writeRegValid;
	
	uint16_t readReg;
	uint16_t *readBuffer;
	int				readDataSize;
	int				readRegValid;
	
	int 			operation;
	int 			dataCounter;
	spi_status_t status;
} spi_command_t;

typedef enum {
	SPI_IRQ_ABORT=3,
	SPI_IRQ_MODF,
	SPI_IRQ_ROVR,
	SPI_IRQ_WCOL,
	SPI_IRQ_SPIF
} spi_irq_type;

#endif /* LPC17XX_SPI_H_ */
