/*
 * utils.h
 *
 *  Created on: Oct 26, 2015
 *      Author: mark.marinas
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include "data_types.h"

error_code_t WriteReg(volatile uint32_t *reg, uint32_t val, uint32_t lsb, uint32_t msb);
error_code_t WriteReg8(volatile uint8_t *reg, uint8_t val, uint32_t lsb, uint32_t msb);
error_code_t ReadReg(volatile uint32_t *reg, uint32_t *val, uint32_t lsb, uint32_t msb);
error_code_t ReadReg8(volatile uint8_t *reg, uint8_t *val, uint32_t lsb, uint32_t msb);

#define exec(command, timeout)  time_t start_time, end_time ; \
                                GetTime(&start_time); end_time = start_time; \
                                while ( difftime(end_time, start_time) < timeout ) { \
                                        if ( ( command ) == NO_ERROR ) { \
                                                error = NO_ERROR; \
                                                break; \
                                        } \
                                        GetTime(&end_time); \
                                }


#endif /* UTILS_H_ */
