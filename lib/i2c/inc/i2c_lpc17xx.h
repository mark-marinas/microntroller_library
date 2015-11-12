/*
 * i2c_lpc17xx.h
 *
 *  Created on: Nov 2, 2015
 *      Author: mmarinas
 */

#ifndef I2C_LPC17XX_H_
#define I2C_LPC17XX_H_

#include "data_types.h"

typedef struct {
	int clkdiv;
	int clkH, clkL;
} i2c_clkdiv_t;

typedef enum {
	STANDARD,
	FAST,
	FAST_PLUS
} i2c_datarate_t;

typedef enum {
	I2C0,
	I2C1,
	I2C2
} i2c_port_t;

typedef enum {
	MASTER,
	SLAVE
} i2c_mode_t;

typedef enum {
	WRITE,
	READ
} i2c_operation_t;

/* TODO:
 * Support for SLAVE MODE.
 */
typedef enum {
	IDLE  = 0xF8,
	ERROR = 0x00,
	START = 0x08,
	RESTART = 0x10,
	MASTER_SLAVEWr_ACK = 0x18,
	MASTER_SLAVEWr_NAK = 0x20,
	MASTER_DATAWr_ACK  = 0x28,
	MASTER_DATAWr_NAK  = 0x30,
	MASTER_ARB_LOST  = 0x38,
	MASTER_SLARd_ACK  = 0x40,
	MASTER_SLARd_NAK  = 0x48,
	MASTER_DATARd_ACK = 0x50,
	MASTER_DATARd_NAK = 0x58
} i2c_status_t;


typedef struct {
	i2c_operation_t operation;
	char	address;
	char	reg;
	char    *data;
	i2c_status_t  expected_status;
	int 			size;
	int			data_written;
	int			wait;
} i2c_command_t;

typedef struct {
	i2c_port_t i2c_port;
	i2c_mode_t i2c_mode;
	i2c_datarate_t datarate;
	void 		   *buffer;
	void (*irqhandler)(void *);
} lpc17xx_i2c_config_t;

typedef lpc17xx_i2c_config_t i2c_config_t;

#endif /* I2C_LPC17XX_H_ */
