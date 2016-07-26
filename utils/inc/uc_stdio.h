/*
 * my_stdio.h
 *
 *  Created on: Nov 10, 2015
 *      Author: mmarinas
 */

#ifndef MY_STDIO_H_
#define MY_STDIO_H_

#include "data_types.h"

void	SetDebug_Port(int port);
int uc_printf(const char *format, ...);
int uc_sprintf(char *str, const char *format, ...);
int uc_snprintf(char *str, int size, const char *format, ...);


#endif /* MY_STDIO_H_ */
