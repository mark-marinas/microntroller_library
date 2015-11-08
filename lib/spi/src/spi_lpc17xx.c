/*
 * lpc17xx_spi.c
 *
 *  Created on: Nov 5, 2015
 *      Author: mark.marinas
 */

#include <stdint.h>
#include "peripheral_config.h"
#include "data_types.h"
#include "gpio_lpc17xx.h"
#include "clk_lpc17xx.h"
#include "spi_lpc17xx.h"
#include "stdperip.h"
#include "utils.h"

typedef struct {
	pin_func_t func;
	lpc17xx_gpio_config_t sclk_pin;
	lpc17xx_gpio_config_t ssel_pin;
	lpc17xx_gpio_config_t miso_pin;
	lpc17xx_gpio_config_t mosi_pin;
	int					  clk_lsb;
} spi_map_t;


static spi_map_t lpc17xx_spi_map[] = {
		{ ALT3_FUNC,
		 { PORT0, PIN15, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		 { PORT0, PIN16, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		 { PORT0, PIN17, INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		 { PORT0, PIN18, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		 16
		},
		{ ALT2_FUNC,
		  { PORT0, PIN15, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		  { PORT0, PIN16, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		  { PORT0, PIN17, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		  { PORT0, PIN18, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		 10
		},
		{ ALT2_FUNC,
		  { PORT0,  PIN7, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		  { PORT0,  PIN6, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		  { PORT0,  PIN8, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		  { PORT0,  PIN9, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		 20
		}
};


void *spi_configs[config_SPI0_EN + config_SSPI0_EN + config_SSPI1_EN];



static error_code_t SPI_WriteDataReg(lpc17xx_spi_config_t *config, uint16_t data);
static error_code_t SPI_Config_Pins(spi_port_t spi_port);
static void ClearIRQ(lpc17xx_spi_config_t *config);

#if (config_SPI_GetDivHook == 0)
	typedef struct {
		int clk_div;
		uint32_t spi_counter;
		uint32_t spi_divisor;
	} spi_clkdiv_t;

	int SPI_GetCounterVal(int min_counter, uint32_t systemfreq, uint32_t spi_freq, int div) {
		uint32_t spi_clock, spi_counter, _spi_freq;
		spi_clock = systemfreq/div;
		_spi_freq  = spi_freq ;
		spi_counter = spi_clock/_spi_freq;
		if (spi_counter >= 8 ) {
			if (spi_counter%2 != 0) {
				spi_counter++;
			}
			return spi_counter;
		}
		return 0;
	}

	error_code_t SPI_GetDividers(lpc17xx_spi_config_t *config, void *spi_div) {
		error_code_t error = NO_ERROR;
		uint32_t spi_counter;

		UpdateClockValues();
		system_clock_dividers_t clk_div;
		system_clock_dividers_t clk_div_final;
		int valid_clk_div_found = 0;
		for (clk_div = PCLKDIV_BY_4; ( clk_div <= PCLKDIV_BY_8 && valid_clk_div_found == 0); clk_div++) {
			switch (clk_div) {
				case PCLKDIV_BY_4:
					spi_counter = SPI_GetCounterVal(MIN_COUNTER, SystemFrequency, config->freq, 4);
					if (spi_counter != 0) {
						clk_div_final = clk_div;
						valid_clk_div_found = 1;
					}
					break;
				case PCLKDIV_BY_1:
					spi_counter = SPI_GetCounterVal(MIN_COUNTER, SystemFrequency, config->freq, 1);
					if (spi_counter != 0) {
						clk_div_final = clk_div;
						valid_clk_div_found = 1;
					}
					break;
				case PCLKDIV_BY_2:
					spi_counter = SPI_GetCounterVal(MIN_COUNTER, SystemFrequency, config->freq, 2);
					if (spi_counter != 0) {
						clk_div_final = clk_div;
						valid_clk_div_found = 1;
					}
					break;
				case PCLKDIV_BY_8:
					spi_counter = SPI_GetCounterVal(MIN_COUNTER, SystemFrequency, config->freq, 8);
					if (spi_counter != 0) {
						clk_div_final = clk_div;
						valid_clk_div_found = 1;
					}
					break;
			}
		}
		if (valid_clk_div_found == 0) {
			return SPI_INVALID_FREQUENCY;
		}

		((spi_clkdiv_t *)spi_div)->clk_div = clk_div_final;
		((spi_clkdiv_t *)spi_div)->spi_counter = spi_counter;
		return error;
	}

	/*
	freq = ( PCLK / (CPSDVSR * [SCR+1])

	scr is 8 bits, and so is CPSDVSR

	assume a PCLK divisor.
	assume a CPSDVSR (even number only) .
	calculate SCR, round up.

	freq * cpsdvsr * (scr + 1) = pclk
	scr = (pclk/(freq*cpsdvsr)) - 1
	scr = 25M/(8MHz*2) - 1
	*/
	int SSPI_GetCounterVal(uint32_t systemfreq, uint32_t spi_freq, int div, spi_clkdiv_t *clkdiv) {
		uint32_t sspi_clock = systemfreq/div;
		uint8_t cpsdvr;
		uint32_t scr;
		uint8_t div_found = 1;
		for (cpsdvr = 2; cpsdvr <=254; cpsdvr+=2) {
			scr = (sspi_clock/(spi_freq * cpsdvr) ) - 1;
			if ((scr + 1) < 0xFF ) {
				div_found = 1;
				break;
			}
		}
		if (div_found) {
			clkdiv->spi_counter = scr;
			clkdiv->spi_divisor = cpsdvr;
		} else {
			return 1;
		}
		return 0;
	}

	error_code_t SSPI_GetDividers(lpc17xx_spi_config_t *config, void *spi_div) {
		error_code_t error = NO_ERROR;

		spi_clkdiv_t *clkdiv = spi_div;

		UpdateClockValues();
		if ( (SSPI_GetCounterVal(SystemFrequency, config->freq, 1, clkdiv)) == 0 ) {
			clkdiv->clk_div = PCLKDIV_BY_1;
			return error;
		} else if ( (SSPI_GetCounterVal(SystemFrequency, config->freq, 2, clkdiv)) == 0 ) {
			clkdiv->clk_div = PCLKDIV_BY_2;
			return error;
		} else if ( (SSPI_GetCounterVal(SystemFrequency, config->freq, 4, clkdiv)) == 0 ) {
			clkdiv->clk_div = PCLKDIV_BY_4;
			return error;
		} else if ( (SSPI_GetCounterVal(SystemFrequency, config->freq, 8, clkdiv)) == 0 ) {
			clkdiv->clk_div = PCLKDIV_BY_8;
			return error;
		}


		return SPI_INVALID_FREQUENCY;
	}
#else

#endif


static error_code_t SPI_Config_Pins(spi_port_t spi_port) {
	//Dont use GPIO_Config, as it could have a post hook.
	error_code_t error = NO_ERROR;

	spi_map_t *spi_pin = &(lpc17xx_spi_map[spi_port]);
	
	if ( (error = GPIO_Set_Func( &(spi_pin->miso_pin), spi_pin->func ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Func( &(spi_pin->mosi_pin), spi_pin->func ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Func( &(spi_pin->sclk_pin), spi_pin->func ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Func( &(spi_pin->ssel_pin), PRIMARY_FUNC ) ) != NO_ERROR ) {
		return error;
	}

	if ( (error = GPIO_Set_Mode( &(spi_pin->miso_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Mode( &(spi_pin->mosi_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Mode( &(spi_pin->sclk_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Mode( &(spi_pin->ssel_pin) ) ) != NO_ERROR ) {
		return error;
	}

	if ( (error = GPIO_Set_OpenDrain_Mode( &(spi_pin->miso_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_OpenDrain_Mode( &(spi_pin->mosi_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_OpenDrain_Mode( &(spi_pin->sclk_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_OpenDrain_Mode( &(spi_pin->ssel_pin) ) ) != NO_ERROR ) {
		return error;
	}

	if ( (error = GPIO_Set_Direction( &(spi_pin->miso_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Direction( &(spi_pin->mosi_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Direction( &(spi_pin->sclk_pin) ) ) != NO_ERROR ) {
		return error;
	}
	if ( (error = GPIO_Set_Direction( &(spi_pin->ssel_pin) ) ) != NO_ERROR ) {
		return error;
	}
	
	return error;
}


static error_code_t SPI_WriteClockDiv( lpc17xx_spi_config_t *config, spi_clkdiv_t *spi_div, int lsb) {
	error_code_t error = NO_ERROR;
	switch (config->port) {
		case SPI0:
		case SSPI1:
			error = WriteReg ( &LPC_SC->PCLKSEL0, spi_div->clk_div, lsb,  lsb + 1 );
			break;
		case SSPI0:
			error = WriteReg ( &LPC_SC->PCLKSEL1, spi_div->clk_div, lsb,  lsb + 1 );
			break;
	}
	return error;
}

static error_code_t SPI_WriteControl(lpc17xx_spi_config_t *config) {
	error_code_t error = NO_ERROR;
	uint32_t control = 0;
	switch (config->port) {
		case SPI0:
			control = (config->bits != SPI_8_BITS) ? ( 1 <<2 ) : 0; //BitEn.
			control |= (config->clk_phase << 3) ; 					//Clock Phase
			control |= (config->clk_polarity << 4);					//Clock Polarity
			control |= (config->mode << 5);							//Mode, master or slave.
			control |= (config->lsbf << 6);							//lsbf
			control |= (1 << 7);									//SPIE
			control |= config->bits << 8;							//Bits
			LPC_SPI->SPCR = control;
			break;
		case SSPI0:
		case SSPI1:
			return FEATURE_NOT_SUPPORTED;
			break;
	}
	return error;
}

static error_code_t SPI_WriteCounters( lpc17xx_spi_config_t *config, spi_clkdiv_t *spi_div ) {
	error_code_t error = NO_ERROR;
	switch (config->port) {
		case SPI0:
			LPC_SPI->SPCCR = spi_div->spi_counter & 0xFF;
			break;
		case SSPI0:
			LPC_SSP0->CPSR = spi_div->spi_divisor;
			error = WriteReg ( &(LPC_SSP0->CR0), spi_div->spi_counter, 8, 15);
			break;
		case SSPI1:
			LPC_SSP1->CPSR = spi_div->spi_divisor;
			error = WriteReg ( &(LPC_SSP1->CR0), spi_div->spi_counter, 8, 15);
			break;
	}
	return error;
}

int spi_get_irq(lpc17xx_spi_config_t *config) {
	switch (config->port) {
		case SPI0:
			return LPC_SPI->SPSR;
			break;
		case SSPI0:
			break;
		case SSPI1:
			break;
	}
	return 0;
}

static void	spi_irqhandler_default (void *data) {
	lpc17xx_spi_config_t *config = data;

	if (config->port == SPI0) {
		spi_command_t *cmd= config->buffer;
		int irq = spi_get_irq(config);

		if (irq > (1 << SPI_IRQ_SPIF)) {
			cmd->status = SPI_READY_FAIL;
		} else {
			if (cmd->operation == SPI_WRITE) {
				LPC_SPI->SPDR;
				ClearIRQ(config);
				if (cmd->dataCounter < cmd->writeDataSize) {
					SPI_WriteDataReg(config, cmd->writeBuffer[cmd->dataCounter]);
					cmd->dataCounter++;
				} else {
					cmd->status = SPI_READY_PASS;
				}
			} else {
				if (cmd->dataCounter < cmd->readDataSize) {					
					cmd->readBuffer[cmd->dataCounter++] = LPC_SPI->SPDR;
					ClearIRQ(config);
					SPI_WriteDataReg(config, config->dummyData);
				} else {
					LPC_SPI->SPDR;
					ClearIRQ(config);
					cmd->status = SPI_READY_PASS;
				}
			}
		}
	} else {
		//TODO: SSPI0/SSPI1.
	}
}

static error_code_t SPI_EnableIRQ (lpc17xx_spi_config_t *config) {
	error_code_t error = NO_ERROR;
	switch (config->port) {
		case SPI0:
			break;
		case SSPI0:
		case SSPI1:
			return FEATURE_NOT_SUPPORTED;
			break;
	}
	if (config->irqhandler == 0) {
		config->irqhandler = spi_irqhandler_default;
	}
	NVIC_EnableIRQ(SPI_IRQn + config->port);
	return error;
}

static int IS_TXBuffer_Empty( int spi_port ) {
	uint32_t status;
	switch (spi_port){
		case SPI0:
			ReadReg((volatile uint32_t *) &(LPC_SPI->SPSR), &status, 7, 7 );
			break;
		case SSPI0:
			break;
		case SSPI1:
			break;
	}
	return status;
}

static void ClearIRQ(lpc17xx_spi_config_t *config) {
	switch (config->port) {
		case SPI0:
			LPC_SPI->SPINT = 1;
			break;
		case SSPI0:
		case SSPI1:
			break;
	}
}

static error_code_t SPI_ReadDataReg(lpc17xx_spi_config_t *config, uint16_t *data) {
	error_code_t error = NO_ERROR;
	switch (config->port) {
		case SPI0:
			*data = LPC_SPI->SPDR;
			break;
		case SSPI0:
		case SSPI1:
			error = FEATURE_NOT_SUPPORTED;
			break;
	}
	return error;
}

static error_code_t SPI_WriteDataReg(lpc17xx_spi_config_t *config, uint16_t data) {
	error_code_t error = NO_ERROR;
	switch (config->port) {
		case SPI0:
			LPC_SPI->SPDR = data;
			break;
		case SSPI0:
		case SSPI1:
			error = FEATURE_NOT_SUPPORTED;
			break;
	}
	return error;
}

static error_code_t SPI_SetSSEL(lpc17xx_spi_config_t *config, signal_level_t level) {
	lpc17xx_gpio_config_t *ss = &(lpc17xx_spi_map[config->port].ssel_pin);
	return (GPIO_SetLevel( ss, level));
}

static void SPI_ClrDRDeassertSSEL(lpc17xx_spi_config_t *config) {
	uint16_t dummy;
	ClearIRQ(config);
	SPI_ReadDataReg(config, &dummy);
	SPI_SetSSEL(config, HI);	
}

error_code_t	SPI_Config(void *config) {
	error_code_t error = NO_ERROR;
	spi_clkdiv_t spi_div;
	lpc17xx_spi_config_t *_config = config;
	spi_map_t *spi_map = (spi_map_t *) &(lpc17xx_spi_map[_config->port]);

	//Configure the PINS.
	error = SPI_Config_Pins(_config->port);

	//PCLK Dividers.
	if ( ( error = ((_config->port == SPI0) ? SPI_GetDividers(_config, &spi_div) : SSPI_GetDividers(_config, &spi_div))  ) != NO_ERROR ) {
		return error;
	}
	error  = SPI_WriteClockDiv(_config, &spi_div, spi_map->clk_lsb);
	//Control Registers.
	error |= SPI_WriteControl(_config);
	//Counters.
	error |= SPI_WriteCounters(_config, &spi_div);
	//Enable IRQ.
	error |= SPI_EnableIRQ(_config);
	spi_configs[_config->port] = _config;
	return error;
}

error_code_t	LPC17XX_SPI_Read_default(int spi_port, void *data) {
	error_code_t error = NO_ERROR;
	spi_command_t *cmd = data;
	cmd->dataCounter = 0;
	cmd->operation = SPI_READ;

	lpc17xx_spi_config_t *config = spi_configs[spi_port];
	config->buffer  = cmd;

	if (spi_port == SPI0) {
		cmd->status = SPI_BUSY;
		error  = SPI_SetSSEL(config, LO);
		
		if (cmd->readRegValid == 1) {
			error |= SPI_WriteDataReg(config, cmd->readReg);
		} else {
			error |= SPI_WriteDataReg(config, config->dummyData);
		}
	} else {
		error = FEATURE_NOT_SUPPORTED;
	}

	if (error != NO_ERROR) {
		SPI_ClrDRDeassertSSEL(config);
		return SPI_READ_ERROR;
	}

	while (cmd->status == SPI_BUSY);

	config->buffer = 0;

	if (cmd->status == SPI_READY_FAIL) {
		error = SPI_READ_ERROR;
	}
	SPI_ClrDRDeassertSSEL(config);
	return error;
}

error_code_t	LPC17XX_SPI_Write_default(int spi_port, void *data) {
	error_code_t error = NO_ERROR;
	spi_command_t *cmd = data;
	
	lpc17xx_spi_config_t *config = spi_configs[spi_port];
	cmd->dataCounter = 0;
	config->buffer  = cmd;

	int operation = cmd->operation;
	
	if (spi_port == SPI0) {
		cmd->dataCounter = 0;
		cmd->status = SPI_BUSY;
		error  = SPI_SetSSEL(config, LO);
		cmd->operation = SPI_WRITE;
		//now lets write the SPI Reg, if its valid.
		if (cmd->writeRegValid == 1) {
			error |= SPI_WriteDataReg(config, cmd->writeReg);
		} else {
			cmd->dataCounter++;
			error |= SPI_WriteDataReg(config, cmd->writeBuffer[0]);
		}
	} else {
		error = FEATURE_NOT_SUPPORTED;
	}
	
	if (error != NO_ERROR) {
		SPI_ClrDRDeassertSSEL(config);
		return (SPI_WRITE_ERROR);	
	}
	
	while (cmd->status == SPI_BUSY);
	config->buffer = 0;
	
	if (cmd->status == SPI_READY_FAIL) {
		SPI_ClrDRDeassertSSEL(config);
		return (SPI_WRITE_ERROR);
	}
	
	//Check if this is a back to back Write-Read.
	if ( (operation & SPI_READ) == SPI_READ ) {
			ClearIRQ(config);
			cmd->operation = SPI_READ;
			
			error = LPC17XX_SPI_Read_default(config->port, cmd);
	}
	
	SPI_ClrDRDeassertSSEL(config);
	return error;
}
