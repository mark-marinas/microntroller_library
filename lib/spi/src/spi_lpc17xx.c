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
#include "utils.h"

typedef struct {
	pin_func_t func;
	lpc17xx_gpio_config_t sclk_pin;
	lpc17xx_gpio_config_t ssel_pin;
	lpc17xx_gpio_config_t miso_pin;
	lpc17xx_gpio_config_t mosi_pin;
	int					  clk_lsb;
} spi_map_t;


static const spi_map_t lpc17xx_i2c_map[] = {
		{ ALT3_FUNC,
		  { PORT0, PIN15, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		  { PORT0, PIN16, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
		  { PORT0, PIN17, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 },
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

error_code_t	SPI_Config(void *config) {
	error_code_t error = NO_ERROR;
	spi_clkdiv_t spi_div;
	lpc17xx_spi_config_t *_config = config;
	spi_map_t *spi_map = (spi_map_t *) &(lpc17xx_i2c_map[_config->port]);

	if ( ( error = ((_config->port == SPI0) ? SPI_GetDividers(_config, &spi_div) : SSPI_GetDividers(_config, &spi_div))  ) != NO_ERROR ) {
		return error;
	} else {
		//Write the Clock divider
		if (_config->port == SPI0 || _config->port == SSPI1) {
			error = WriteReg ( &LPC_SC->PCLKSEL0, spi_div.clk_div, spi_map->clk_lsb,  spi_map->clk_lsb + 1 );
		} else {
			error = WriteReg ( &LPC_SC->PCLKSEL1, spi_div.clk_div, spi_map->clk_lsb,  spi_map->clk_lsb + 1 );
		}

		uint32_t control = 0;
		switch (_config->port) {
			case SPI0:
				//And the counters.
				LPC_SPI->SPCCR = spi_div.spi_counter & 0xF ;
				//And the control registers. Interrupt is hardcoded.
				control = (_config->clk_phase << 3 ) | (_config->clk_polarity << 4) | (_config->mode << 5) | (_config->lsbf << 6) |
						  (1 << 7) | (_config->bits << 8);
				WriteReg ( &(LPC_SPI->SPCR), control, 0, 31);
				break;
			case SSPI0:
				//And the counters.
				LPC_SSP0->CPSR = spi_div.spi_divisor;
				WriteReg ( &(LPC_SSP0->CR0), spi_div.spi_counter, 8, 15);
				//and control registers.
				control = LPC_SSP0->CR0 | (_config->bits) | (0 << 4) | (_config->clk_polarity << 6) | (_config->clk_phase << 7);
				WriteReg ( &(LPC_SSP0->CR0), control, 0, 31 );
				control = (1 << 1) | (_config->mode << 2) ;
				WriteReg ( &(LPC_SSP0->CR1), control, 0, 31);
				break;
			case SSPI1:
				//And the counters.
				//and control registers.
				LPC_SSP1->CPSR = spi_div.spi_divisor;
				WriteReg (&(LPC_SSP1->CR0), spi_div.spi_counter, 8, 15);
				break;
		}

	}


	return error;
}

error_code_t	LPC17XX_SPI_Read_default(int i2c_port, void *data) {
	error_code_t error = NO_ERROR;

	return error;
}
error_code_t	LPC17XX_SPI_Write_default(int i2c_port, void *data) {
	error_code_t error = NO_ERROR;

	return error;
}
