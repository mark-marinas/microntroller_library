/*
 * dac_lpc17xx.c
 *
 *  Created on: Nov 21, 2015
 *      Author: mmarinas
 */

#include <stdint.h>
#include "data_types.h"
#include "dac_lpc17xx.h"
#include "gpio_lpc17xx.h"
#include "clk_lpc17xx.h"
#include "utils.h"
#include "fifo.h"
#include "LPC17xx.h"


#define AOUT_PORT	PORT0
#define AOUT_PIN	PIN26

#if (config_DAC_PostHook == 1)
	extern error_code_t DAC_Config_PostHook(void *config);
#endif

#if (config_DAC_Buffer_Size > 0)
	uint16_t dac_buffer[config_DAC_Buffer_Size];
	fifo_t dac_fifo;
#else
	#error "DAC Buffer Size not defined"
#endif



static void dac_fifo_get(fifo_t *fifo, void *data) {
	uint16_t *_data = data;

	*_data = ((uint16_t *)fifo->buffer)[fifo->head];
}

static void dac_fifo_put(fifo_t *fifo, void *data) {
	uint16_t *_data = data;
	((uint16_t *)fifo->buffer)[fifo->tail] = *_data;
}

static void DAC_Enable(void) {
	LPC_DAC->DACCTRL |= (1 << 2);
}

static int ISDACReady(void) {
	return ((LPC_DAC->DACCTRL & 0x01) == 1);
}


error_code_t DAC_SetSamplingRate(void *sampling_rate) {
	error_code_t error = NO_ERROR;

	UpdateClockValues();
	//Calculate the DACCNTVAL.
	int _sampling_rate = *((int *)sampling_rate);
	float sampling_interval = 1.00/_sampling_rate;
	int daccntval = SystemFrequency*sampling_interval;
	if (daccntval > 0xFFFF) {
		return INVALIID_DAC_SAMPLING_RATE;
	}
	LPC_DAC->DACCNTVAL = daccntval;
	return error;
}

error_code_t LPC17XX_DAC_Write_Fifo_Default(void *data, int size) {
	uint16_t *_data = data;
	error_code_t error = NO_ERROR;
	int i;
	for (i=0; i<size; i++) {
		if ( (error = FIFO_Put(&dac_fifo, &_data[i])) != NO_ERROR) {
			return error;
		}
	}
	return error;
}

error_code_t LPC17XX_DAC_Write_Value_Default(void *data) {
	error_code_t error;
	uint16_t value;
	if ( !(ISDACReady()) ) {
		return DAC_BUSY;
	}
	if ( (error = FIFO_Get(&dac_fifo, &value)) != NO_ERROR ) {
		return error;
	} else {
		LPC_DAC->DACR = (value & 0x3FF) << 6;
	}
	return NO_ERROR;
}

error_code_t	DAC_Config(void *config) {
	error_code_t error = NO_ERROR;
	lpc17xx_dac_config_t *_config = config;
	gpio_config_t dac_pin = { AOUT_PORT, AOUT_PIN, OUTPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 };

	//Set GPIO.
	GPIO_Set_Func(&dac_pin, ALT2_FUNC);
	GPIO_Set_Mode(&dac_pin);
	GPIO_Set_OpenDrain_Mode(&dac_pin);
	GPIO_Set_Direction(&dac_pin);
	//Set DAC PCLK and CNTVAL
	error = WriteReg( &(LPC_SC->PCLKSEL0), PCLKDIV_BY_1, 22, 23 );
	error |= DAC_SetSamplingRate(&(_config->sampling_rate));

	//Setup the fifo.
	error |= FIFO_Init(&dac_fifo, sizeof(dac_buffer)/sizeof(uint16_t), dac_buffer, dac_fifo_get, dac_fifo_put);

	//Then finally, Enable the DAC.
	DAC_Enable();

	#if (config_DAC_PostHook == 1)
		error |= DAC_Config_PostHook(config);
	#endif

	return error;
}
