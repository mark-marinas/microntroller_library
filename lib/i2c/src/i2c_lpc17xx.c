/*
 * i2c_lpc17xx.c
 *
 *  Created on: Nov 2, 2015
 *      Author: mark.marinas
 */

#include "gpio_lpc17xx.h"
#include "peripheral_config.h"
#include "utils.h"
#include "clk_lpc17xx.h"
#include "i2c_lpc17xx.h"

typedef struct {
	pin_func_t func;
	lpc17xx_gpio_config_t sda;
	lpc17xx_gpio_config_t scl;
	int clk_lsb;
} i2c_map_t;

static const i2c_map_t lpc17xx_i2c_map[] = {
		{ ALT1_FUNC,
		  { PORT0, PIN27, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED,  PIN_MODE_OPEN_DRAIN, 0, 0, 0 },
		  { PORT0, PIN28, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED,  PIN_MODE_OPEN_DRAIN, 0, 0, 0 },
		  14
		},
		{ ALT3_FUNC,
		  { PORT0, PIN19, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED,  PIN_MODE_OPEN_DRAIN, 0, 0, 0 },
		  { PORT0, PIN20, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED,  PIN_MODE_OPEN_DRAIN, 0, 0, 0 },
		  6
		},
		{ ALT2_FUNC,
		  { PORT0, PIN10, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED,  PIN_MODE_OPEN_DRAIN, 0, 0, 0 },
		  { PORT0, PIN11, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED,  PIN_MODE_OPEN_DRAIN, 0, 0, 0 },
		  20
		},
};

void *i2c_configs[config_I2C0_EN + config_I2C1_EN + config_I2C2_EN] ;

#if (config_I2C_GetDivHook == 0)
i2c_clkdiv_t i2c_clk_divs[] = {
		{ PCLKDIV_BY_2, 250, 250 },
		{ PCLKDIV_BY_1, 125, 125 },
		{ PCLKDIV_BY_2,   2,   2 }
};

i2c_clkdiv_t *I2C_GetDividers(i2c_datarate_t datarate) {
	return ( &(i2c_clk_divs[datarate]));
}
#else
	extern i2c_clkdiv_t *I2C_GetDividers(i2c_datarate_t datarate);
#endif

#if (config_I2C_PostHook == 1)
extern error_code_t I2C_Config_PostHook(void *config);
#endif


/* Function Prototypes */

static error_code_t I2C_Config_Pins(i2c_port_t i2c_port);
static error_code_t I2C_Config_ClkDivs(lpc17xx_i2c_config_t *config, i2c_clkdiv_t *div);
static error_code_t i2c_data_write(lpc17xx_i2c_config_t *config, unsigned char data);
static error_code_t i2c_data_read(lpc17xx_i2c_config_t *config);
static error_code_t i2c_address(lpc17xx_i2c_config_t *config, i2c_operation_t operation);
static error_code_t i2c_reg(lpc17xx_i2c_config_t *config);
static error_code_t i2c_stop(lpc17xx_i2c_config_t *config);
static error_code_t i2c_set_start(lpc17xx_i2c_config_t *config);
static error_code_t i2c_clear_start(lpc17xx_i2c_config_t *config);
static error_code_t i2c_set_ack(lpc17xx_i2c_config_t *config);
static error_code_t i2c_clear_ack(lpc17xx_i2c_config_t *config);
static error_code_t i2c_clear_interrupt(lpc17xx_i2c_config_t *config);
static void decode_i2cstat(lpc17xx_i2c_config_t *config, int status);
static void i2c_irqhandler_default(void *data);
static error_code_t Enable_I2C(lpc17xx_i2c_config_t *config);
static error_code_t IS_Enabled_I2C(lpc17xx_i2c_config_t *config);
static error_code_t Enable_I2C_IRQ(lpc17xx_i2c_config_t *config);
static error_code_t I2C_execute_command(i2c_port_t i2c_port, void *data, i2c_operation_t operation);
static error_code_t I2C_GetStat(lpc17xx_i2c_config_t *_config, uint32_t *status);


/* Code */

static error_code_t I2C_Config_Pins(i2c_port_t i2c_port) {
	//Dont use GPIO_Config, as it could have a post hook.
	error_code_t error = NO_ERROR;

	i2c_map_t i2c_pin = lpc17xx_i2c_map[i2c_port];
	if ( (error = GPIO_Set_Func( &(i2c_pin.scl), i2c_pin.func ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Func( &(i2c_pin.sda), i2c_pin.func ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Mode( &(i2c_pin.scl) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Mode( &(i2c_pin.sda) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_OpenDrain_Mode( &(i2c_pin.scl) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_OpenDrain_Mode( &(i2c_pin.sda) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Direction( &(i2c_pin.scl) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Direction( &(i2c_pin.sda) ) ) != NO_ERROR ) {
		return error;
	}
	return error;
}

static error_code_t I2C_Config_ClkDivs(lpc17xx_i2c_config_t *config, i2c_clkdiv_t *div) {
	error_code_t error = NO_ERROR;
	switch (config->i2c_port) {
		case I2C0:
			error  = WriteReg( &(LPC_I2C0->I2SCLH), div->clkH, 0, 15) ;
			error |= WriteReg( &(LPC_I2C0->I2SCLL), div->clkL, 0, 15) ;
			error |= WriteReg( &(LPC_SC->PCLKSEL0), div->clkdiv, lpc17xx_i2c_map[config->i2c_port].clk_lsb, lpc17xx_i2c_map[config->i2c_port].clk_lsb + 1) ;
			break;
		case I2C1:
			error  = WriteReg( &(LPC_I2C1->I2SCLH), div->clkH, 0, 15) ;
			error |= WriteReg( &(LPC_I2C1->I2SCLL), div->clkL, 0, 15) ;
			error |= WriteReg( &(LPC_SC->PCLKSEL1), div->clkdiv, lpc17xx_i2c_map[config->i2c_port].clk_lsb, lpc17xx_i2c_map[config->i2c_port].clk_lsb + 1) ;
			break;
		case I2C2:
			error  = WriteReg( &(LPC_I2C2->I2SCLH), div->clkH, 0, 15) ;
			error |= WriteReg( &(LPC_I2C2->I2SCLL), div->clkL, 0, 15) ;
			error |= WriteReg( &(LPC_SC->PCLKSEL1), div->clkdiv, lpc17xx_i2c_map[config->i2c_port].clk_lsb, lpc17xx_i2c_map[config->i2c_port].clk_lsb + 1) ;
			break;
	}
	return error;
}

static error_code_t i2c_data_write(lpc17xx_i2c_config_t *config, unsigned char data) {
	error_code_t error = NO_ERROR;
	switch (config->i2c_port) {
		case I2C0:
			error = WriteReg (&(LPC_I2C0->I2DAT), data, 0, 7);
			break;
		case I2C1:
			error = WriteReg (&(LPC_I2C1->I2DAT), data, 0, 7);
			break;
		case I2C2:
			error = WriteReg (&(LPC_I2C2->I2DAT), data, 0, 7);
			break;
	}
	return error;
}

static error_code_t i2c_data_read(lpc17xx_i2c_config_t *config) {
	error_code_t error = NO_ERROR;
	i2c_command_t *cmd = config->buffer;
	uint32_t data_read;

	switch (config->i2c_port) {
		case I2C0:
			error = ReadReg ( &(LPC_I2C0->I2DAT), &data_read,  0, 7 );
			cmd->data[cmd->data_written] = data_read;
			break;
		case I2C1:
			error = ReadReg ( &(LPC_I2C1->I2DAT), &data_read,  0, 7 );
			cmd->data[cmd->data_written] = data_read;
			break;
		case I2C2:
			error = ReadReg ( &(LPC_I2C2->I2DAT), &data_read,  0, 7 );
			cmd->data[cmd->data_written] = data_read;
			break;
	}
	return error;
}

static error_code_t i2c_address(lpc17xx_i2c_config_t *config, i2c_operation_t operation) {
	error_code_t error = NO_ERROR;
	i2c_command_t *cmd = config->buffer;
	unsigned char address = ( cmd->address << 1 ) | operation ;
	error = i2c_data_write(config, address);
	return error;
}

static error_code_t i2c_reg(lpc17xx_i2c_config_t *config) {
	error_code_t error = NO_ERROR;
	i2c_command_t *cmd = config->buffer;
	error = i2c_data_write(config, cmd->reg);
	return error;
}

static error_code_t i2c_stop(lpc17xx_i2c_config_t *config) {
	error_code_t error;
	switch (config->i2c_port) {
		case I2C0:
			error = WriteReg ( &(LPC_I2C0->I2CONSET), 1, 4, 4 );
			break;
		case I2C1:
			error = WriteReg ( &(LPC_I2C1->I2CONSET), 1, 4, 4 );
			break;
		case I2C2:
			error = WriteReg ( &(LPC_I2C2->I2CONSET), 1, 4, 4 );
			break;
	}
	return error;
}

static error_code_t i2c_set_start(lpc17xx_i2c_config_t *config) {
	error_code_t error;
	switch (config->i2c_port) {
		case I2C0:
			error = WriteReg (&(LPC_I2C0->I2CONSET), 1, 5, 5);
			break;
		case I2C1:
			error = WriteReg (&(LPC_I2C1->I2CONSET), 1, 5, 5);
			break;
		case I2C2:
			error = WriteReg (&(LPC_I2C2->I2CONSET), 1, 5, 5);
			break;
	}
	return error;
}

static error_code_t i2c_clear_start(lpc17xx_i2c_config_t *config) {
	error_code_t error = NO_ERROR;
	switch (config->i2c_port) {
		case I2C0:
			LPC_I2C0->I2CONCLR = 1 << 5;
			break;
		case I2C1:
			LPC_I2C1->I2CONCLR = 1 << 5;
			break;
		case I2C2:
			LPC_I2C2->I2CONCLR = 1 << 5;
			break;
	}
	return error;
}

static error_code_t i2c_set_ack(lpc17xx_i2c_config_t *config) {
	error_code_t error = NO_ERROR;
	switch (config->i2c_port) {
		case I2C0:
			error = WriteReg (&(LPC_I2C0->I2CONSET), 1, 2, 2);
			break;
		case I2C1:
			error = WriteReg (&(LPC_I2C1->I2CONSET), 1, 2, 2);
			break;
		case I2C2:
			error = WriteReg (&(LPC_I2C2->I2CONSET), 1, 2, 2);
			break;
	}
	return error;
}

static error_code_t i2c_clear_ack(lpc17xx_i2c_config_t *config) {
	error_code_t error = NO_ERROR;
	switch (config->i2c_port) {
		case I2C0:
			LPC_I2C0->I2CONCLR = 1 << 2;
			break;
		case I2C1:
			LPC_I2C1->I2CONCLR = 1 << 2;
			break;
		case I2C2:
			LPC_I2C2->I2CONCLR = 1 << 2;
			break;
	}
	return error;
}

static error_code_t i2c_clear_interrupt(lpc17xx_i2c_config_t *config) {
	error_code_t error = NO_ERROR;
	switch (config->i2c_port) {
		case I2C0:
			LPC_I2C0->I2CONCLR = 1 << 3;
			break;
		case I2C1:
			LPC_I2C1->I2CONCLR = 1 << 3;
			break;
		case I2C2:
			LPC_I2C2->I2CONCLR = 1 << 3;
			break;
	}
	return error;
}

static void decode_i2cstat(lpc17xx_i2c_config_t *config, int status) {
	error_code_t error = NO_ERROR;
	i2c_command_t *cmd = config->buffer;
	if (cmd->expected_status != status || status == ERROR) {
		cmd->expected_status = ERROR;
		//TODO: Should I issue a STOP command?
	} else {
		switch (status) {
			case START:
				//Send Address byte.
				//After Sending the address, next status should be SLA+W ACK.
				error = i2c_clear_start(config);
				cmd->expected_status = MASTER_SLAVEWr_ACK ;
				error |= i2c_address(config, WRITE);
				break;
			case MASTER_SLAVEWr_ACK:
				//Send Register to Write.
				error = i2c_reg(config);
				cmd->expected_status = MASTER_DATAWr_ACK;
				break;
			case MASTER_DATAWr_ACK:
				//If operation is read, RESTART.
				if (cmd->operation == READ) {
					cmd->expected_status = RESTART;
					error = i2c_set_start(config);
				//Else,just send the data.
				} else if (cmd->data_written < cmd->size ) {
					error = i2c_data_write(config, cmd->data[cmd->data_written]);
					cmd->data_written++;
				//If previous send was the last byte, then next should be STOP.
				} else {
					cmd->expected_status = IDLE;
					error = i2c_stop(config);
				}
				break;
			case RESTART:
				//Send address byte, with direction bit set to READ. I2C will ACK the address.
				error = i2c_clear_start(config);
				cmd->expected_status = MASTER_SLARd_ACK ;
				error |= i2c_address(config, READ);
				break;
			case MASTER_SLARd_ACK:
				//I2C will receive the first byte, set the ACK bit.
				if (cmd->data_written == ( cmd->size - 1) ) {
					error  |= i2c_clear_ack(config);
					cmd->expected_status = MASTER_DATARd_NAK;
				} else {
					error = i2c_set_ack(config);
					cmd->expected_status = MASTER_DATARd_ACK;
				}
				break;
			case MASTER_DATARd_ACK:
				error = i2c_data_read(config);
				//A byte of data was received.
				cmd->data_written++;
				if (cmd->data_written == ( cmd->size - 1) ) {
					//The next byte will be the last byte, send a NAK.
					error  |= i2c_clear_ack(config);
					cmd->expected_status = MASTER_DATARd_NAK;
				} else {
					error |= i2c_set_ack(config);
					cmd->expected_status = MASTER_DATARd_ACK;
				}
				break;
			case MASTER_DATARd_NAK:
				error = i2c_data_read(config);
				cmd->expected_status = IDLE;
				i2c_stop(config);
				break;
			default:
				break;
		}

	}
	if (error != NO_ERROR) {
		cmd->expected_status = ERROR;
	}
	i2c_clear_interrupt(config);
}

static error_code_t I2C_GetStat(lpc17xx_i2c_config_t *_config, uint32_t *status) {
	error_code_t error = NO_ERROR;
	switch (_config->i2c_port) {
		case I2C0:
			error = ReadReg ((volatile uint32_t *) &LPC_I2C0->I2STAT, status, 0, 7);
			break;
		case I2C1:
			error = ReadReg ((volatile uint32_t *) &LPC_I2C1->I2STAT, status, 0, 7);
			break;
		case I2C2:
			error = ReadReg ((volatile uint32_t *) &LPC_I2C2->I2STAT, status, 0, 7);
			break;
	}
	return error;
}

static void i2c_irqhandler_default(void *data) {
	lpc17xx_i2c_config_t *_config = data;
	uint32_t status;
	error_code_t error;

	error = I2C_GetStat(_config, &status);

	if (error != NO_ERROR) {
		((i2c_command_t *)(_config->buffer))->expected_status = ERROR;
	} else {
		decode_i2cstat(_config, status);
	}
}

static error_code_t Enable_I2C(lpc17xx_i2c_config_t *config) {
	error_code_t error = NO_ERROR;
	if (config->i2c_mode == SLAVE) {
		return FEATURE_NOT_SUPPORTED;
	}
	switch (config->i2c_port) {
		case I2C0:
			error = WriteReg ( &(LPC_I2C0->I2CONSET), 1, 6, 6) ;
			break;
		case I2C1:
			error = WriteReg ( &(LPC_I2C1->I2CONSET), 1, 6, 6) ;
			break;
		case I2C2:
			error = WriteReg ( &(LPC_I2C2->I2CONSET), 1, 6, 6) ;
			break;
	}
	return error;
}

static error_code_t IS_Enabled_I2C(lpc17xx_i2c_config_t *config) {
	error_code_t error = NO_ERROR;
	uint32_t status;


	switch (config->i2c_port) {
		case I2C0:
			error = ReadReg ( &(LPC_I2C0->I2CONSET), &status, 6, 6) ;
			break;
		case I2C1:
			error = ReadReg ( &(LPC_I2C1->I2CONSET),&status,  6, 6) ;
			break;
		case I2C2:
			error = ReadReg ( &(LPC_I2C2->I2CONSET),&status,  6, 6) ;
			break;
	}

	if ( status == 0) {
		return UNITIALIZED_I2C;
	}
	return error;
}

static error_code_t Enable_I2C_IRQ(lpc17xx_i2c_config_t *config) {
	if (config->i2c_mode == SLAVE) {
		return FEATURE_NOT_SUPPORTED;
	}
	NVIC_EnableIRQ(I2C0_IRQn + config->i2c_port);
	return NO_ERROR;
}

error_code_t	I2C_Config(void *config) {
	error_code_t error = NO_ERROR;
	lpc17xx_i2c_config_t *_config = config;

	//Configure the PINS.
	error = I2C_Config_Pins(_config->i2c_port);

	//Config Clks.
	error|= I2C_Config_ClkDivs(_config, I2C_GetDividers(_config->datarate));

	_config->buffer = 0;
	if (_config->irqhandler == 0) {
		_config->irqhandler = i2c_irqhandler_default;
	}
	_config->buffer = 0;
	i2c_configs[_config->i2c_port] = _config;

	error |= Enable_I2C(_config);
	error |= Enable_I2C_IRQ(_config);

	#if (config_I2C_PostHook == 1)
		return (error | I2C_Config_PostHook(void *config));
	#endif

	return error;
}


static error_code_t I2C_execute_command(i2c_port_t i2c_port, void *data, i2c_operation_t operation) {
	lpc17xx_i2c_config_t *_config = i2c_configs[i2c_port] ; // config;
	i2c_command_t *cmd = data;
	uint32_t status;

	if (IS_Enabled_I2C(_config) != NO_ERROR) {
		return UNITIALIZED_I2C;
	}

	cmd->data_written = 0;
	cmd->expected_status = START;
	cmd->operation = operation;
	_config->buffer = cmd;

	i2c_set_start(_config);
	//TODO: This should timeout.
	while ( 1 ) {
		I2C_GetStat(_config, &status);
		if ( (cmd->expected_status == IDLE || cmd->expected_status == ERROR) && (status == IDLE) ) {
			break;
		}
	}
	_config->buffer = 0;
	if (cmd->expected_status == ERROR) {
		return I2C_ERROR;
	}

	if (cmd->wait > 0) {
		//TODO: Wait for a while before exiting. Looks like this is necessary, for example for successive writes to EEPROM.
	}
	return NO_ERROR;
}

//Sample call.
// 	unsigned char buffer[8]
//	i2c_command_t command;

//	command.address = 0xA0;
//	command.data = buffer;
//	command.reg = 0xF0;
//	command.size = sizeof(buffer); //All other members of struct can be left blank.
// error = I2C_Read(&config, &command);

error_code_t	LPC17XX_I2C_Read_default(int i2c_port, void *data) {
	if ( I2C_execute_command(i2c_port, data, READ) != NO_ERROR) {
		return I2C_READ_ERROR;
	}
	return NO_ERROR;
}

error_code_t	LPC17XX_I2C_Write_default(int i2c_port, void *data) {
	if ( I2C_execute_command(i2c_port, data, WRITE) != NO_ERROR) {
		return I2C_WRITE_ERROR;
	}
	return NO_ERROR;
}
