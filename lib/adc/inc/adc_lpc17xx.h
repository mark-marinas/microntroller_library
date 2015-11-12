/*
 * adc_lpc17xx.h
 *
 *  Created on: Nov 10, 2015
 *      Author: mmarinas
 */

#ifndef ADC_LPC17XX_H_
#define ADC_LPC17XX_H_

#include <stdint.h>

typedef enum {
	ADC_CHANNEL0,
	ADC_CHANNEL1,
	ADC_CHANNEL2,
	ADC_CHANNEL3,
	ADC_CHANNEL4,
	ADC_CHANNEL5,
	ADC_CHANNEL6,
	ADC_CHANNEL7,
} adc_channel_t;

typedef enum {
	MANUAL,
	BURST
} trigger_mode_t;

typedef struct {
	adc_channel_t channel;
	uint32_t rate;
	trigger_mode_t trigger_mode;
	void (*irqhandler)(void *);
	int done;
	uint16_t result;
} lpc17xx_adc_config_t;

typedef lpc17xx_adc_config_t adc_config_t;

void LPC17XX_ADC_IRQ_Handler_Default(void);

#endif /* ADC_LPC17XX_H_ */
