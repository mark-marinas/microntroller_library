/*
 * adc_lpc17xx.c
 *
 *  Created on: Nov 10, 2015
 *      Author: mmarinas
 */


#include "peripheral_config.h"
#include "data_types.h"
#include "adc_lpc17xx.h"
#include "gpio_lpc17xx.h"
#include "clk_lpc17xx.h"
#include "utils.h"
#include "LPC17xx.h"
#include "uc_stdio.h"


#if ( using_OS == freeRTOS)
	#include "stdirq.h"
	//	If using OS, create a thread that blocks until a data is received using
	//	xQueueReceive( uartX_notifier, &ulReceivedValue, portMAX_DELAY );
	QueueHandle_t adc_notifier =  NULL  ;
#endif

typedef struct {
	pin_func_t func;
	lpc17xx_gpio_config_t pin_config;
} adc_map_t;

static adc_map_t lpc17xx_adc_map[] = {
		{ ALT1_FUNC, { PORT0, PIN23, INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 } },
		{ ALT1_FUNC, { PORT0, PIN24, INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 } },
		{ ALT1_FUNC, { PORT0, PIN25, INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 } },
		{ ALT1_FUNC, { PORT0, PIN26, INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 } },
		{ ALT3_FUNC, { PORT1, PIN30, INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 } },
		{ ALT3_FUNC, { PORT1, PIN31, INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 } },
		{ ALT2_FUNC, { PORT0, PIN3 , INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 } },
		{ ALT2_FUNC, { PORT0, PIN2 , INPUT, INTERRUPT_DISABLED, PULLUP_PULLDOWN_DISABLED, PIN_MODE_OPEN_DRAIN_NORMAL, 0, 0, 0 } },
};

//void *adc_configs[config_ADC0_En + config_ADC1_En + config_ADC2_En + config_ADC3_En + config_ADC4_En + config_ADC5_En + config_ADC6_En + config_ADC7_En] = { 0 } ;
void *adc_configs[8];
static void LPC17XX_ADC_IRQ_Channel_Handler_Default(void *config) {
	//Does nothing if not running on OS.
	#if ( using_OS == freeRTOS )
		xQueueSend( adc_notifier, config, 0U );
	#endif
}

static void Enable_ADC_IRQ(void) {
	NVIC_EnableIRQ(ADC_IRQn);
}

void LPC17XX_ADC_IRQ_Handler_Default(void) {
	int adcr = LPC_ADC->ADGDR;
	int channel = (adcr >> 24) & 0x07;
	int result  = (adcr >> 4) & 0xFFF;
	int overrun = (adcr >> 30) & 0x01; //TODO: Is there a use for this?
	int done = (adcr >> 31) & 0x01;

	lpc17xx_adc_config_t *config = adc_configs[channel];
	if (config->trigger_mode == MANUAL) {
		//Stop conversion
		WriteReg (&(LPC_ADC->ADCR), 0, 24, 26);
	}
	config->done = done;
	config->result = result;
	config->irqhandler(config);
}

error_code_t 	ADC_Config(void *config) {
	error_code_t error;
	lpc17xx_adc_config_t *_config = config;
	adc_map_t adc_map =  lpc17xx_adc_map[_config->channel];

	//Configure the Pins.
	error = GPIO_Set_Func(&(adc_map.pin_config), adc_map.func);
	error|= GPIO_Set_Mode(&(adc_map.pin_config));
	error|= GPIO_Set_OpenDrain_Mode(&(adc_map.pin_config));
	error|= GPIO_Set_Direction(&(adc_map.pin_config));

	//Enable PCONP.
	LPC_SC->PCONP |= (1 << 12);
	//Enable PDN
	LPC_ADC->ADCR |= (1 << 21);

	//Set the clock.
	//Max of 200KHz sampling rate.
	if (_config->rate > 200e3) {
		_config->rate = 200e3;
	}
	UpdateClockValues();
	//Sampling rate = (SystemFrequency/Clock_Divider)/( ADC_PreScaler + 1).
	//ADC_PreScaler = ( (SystemFrequency/ClockDiver) / Sampling Rate) - 1;
	int div;
	float f_adc_prescaler;
	int   i_adc_prescaler;
	int   div_found = 0;
	for (div=1; div<=8; div=div*2) {
		f_adc_prescaler = ( (SystemFrequency/div) / _config->rate ) - 1;
		i_adc_prescaler = f_adc_prescaler;
		if ( (f_adc_prescaler - i_adc_prescaler ) != 0 ) {
			i_adc_prescaler += 1;
		}
		if (i_adc_prescaler <= 0xFF) {
			div_found = 1;
			break;
		}
	}
	if (div_found == 0) {
		return INVALID_ADC_SAMPLING_RATE;
	}
	error |= WriteReg( &(LPC_ADC->ADCR), i_adc_prescaler, 8, 15);
	switch (div) {
		case 1:
			error |= WriteReg(&(LPC_SC->PCLKSEL0), PCLKDIV_BY_1, 24, 25);
			break;
		case 2:
			error |= WriteReg(&(LPC_SC->PCLKSEL0), PCLKDIV_BY_2, 24, 25);
			break;
		case 4:
			error |= WriteReg(&(LPC_SC->PCLKSEL0), PCLKDIV_BY_4, 24, 25);
			break;
		case 8:
			error |= WriteReg(&(LPC_SC->PCLKSEL0), PCLKDIV_BY_8, 24, 25);
			break;
	}

	//Set BURST Mode.
	if (_config->trigger_mode == BURST) {
		LPC_ADC->ADCR |= (1 << 16);
		LPC_ADC->ADINTEN &= ~(1 << 8);
	}

	if (_config->irqhandler == 0) {
		_config->irqhandler = LPC17XX_ADC_IRQ_Channel_Handler_Default;
	}
	//Clear the done flag.
	_config->done = 0;
	adc_configs[_config->channel] = _config;
	#if ( using_OS == freeRTOS)
		if (adc_notifier == NULL) {
			adc_notifier = xQueueCreate( 1, sizeof( lpc17xx_adc_config_t ) );
		}
	#endif

	//Enable ADC Interrupt.
	Enable_ADC_IRQ();

	return error;
}

error_code_t	ADC_Read(void *config) {
	error_code_t error = NO_ERROR;
	lpc17xx_adc_config_t *_config = config;

	_config->done = 0;
	error  = WriteReg (&(LPC_ADC->ADCR), 1, _config->channel, _config->channel);
	//Start conversion
	error |= WriteReg (&(LPC_ADC->ADCR), 1, 24, 26);
	//Wait until conversion is done. TODO: This should Time out.
	while (_config->done == 0);
	return error;
}
