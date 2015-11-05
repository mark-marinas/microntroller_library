/*
 * clk_lpc17xx.h
 *
 *  Created on: Oct 27, 2015
 *      Author: mmarinas
 */

#ifndef CLK_LPC17XX_H_
#define CLK_LPC17XX_H_


#include <stdint.h>

extern uint32_t SystemFrequency;


typedef enum {
	PCLKDIV_BY_4,
	PCLKDIV_BY_1,
	PCLKDIV_BY_2,
	PCLKDIV_BY_8
} system_clock_dividers_t;

void UpdateClockValues(void);

#endif /* CLK_LPC17XX_H_ */

