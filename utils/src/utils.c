/*
 * utils.c
 *
 *  Created on: Oct 26, 2015
 *      Author: mmarinas
 */

#include "utils.h"
#include "peripheral_config.h"

error_code_t WriteReg(volatile uint32_t *reg, uint32_t val, uint32_t lsb, uint32_t msb) {
	uint32_t mask = 0;
	uint32_t i;

	if ( (lsb > msb) | (msb > 31) ) {
		return INVALID_BIT_POSITION;
	}
	for (i=lsb; i<=msb; i++) {
		mask |= (1 << i);
	}
	mask = ~mask;
	*reg = *reg & mask;

	val = val << lsb;
	*reg = *reg | val;

	return NO_ERROR;
}

error_code_t WriteReg8(volatile uint8_t *reg, uint8_t val, uint32_t lsb, uint32_t msb) {
	uint8_t mask = 0;
	uint32_t i;

	if ( (lsb > msb) | (msb > 7) ) {
		return INVALID_BIT_POSITION;
	}
	for (i=lsb; i<=msb; i++) {
		mask |= (1 << i);
	}
	mask = ~mask;
	*reg = *reg & mask;

	val = val << lsb;
	*reg = *reg | val;

	return NO_ERROR;
}

error_code_t ReadReg(volatile uint32_t *reg, uint32_t *val, uint32_t lsb, uint32_t msb) {
	volatile uint32_t regval = *reg;
	uint32_t mask = 0;
	uint32_t i = 0;

	if ( (lsb > msb) | (msb > 31) ) {
		return INVALID_BIT_POSITION;
	}

	for (i=lsb; i<=msb; i++) {
		mask |= (1 << i);
	}
	regval &= mask;
	regval = regval >> lsb;

	*val = regval;
	return NO_ERROR;
}

error_code_t ReadReg8(volatile uint8_t *reg, uint8_t *val, uint32_t lsb, uint32_t msb) {
	volatile uint8_t regval = *reg;
	uint8_t mask = 0;
	uint32_t i = 0;

	if ( (lsb > msb) | (msb > 7) ) {
		return INVALID_BIT_POSITION;
	}

	for (i=lsb; i<=msb; i++) {
		mask |= (1 << i);
	}
	regval &= mask;
	regval = regval >> lsb;

	*val = regval;
	return NO_ERROR;
}
