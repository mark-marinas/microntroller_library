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
#else

#endif

error_code_t	SPI_Config(void *config) {
	error_code_t error = NO_ERROR;
	spi_clkdiv_t spi_div;
	lpc17xx_spi_config_t *_config = config;

	if ( ( error = SPI_GetDividers(_config, &spi_div) ) != NO_ERROR ) {
		return error;
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
