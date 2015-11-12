/*
 * uc_stdio.c
 *
 *  Created on: Nov 10, 2015
 *      Author: mmarinas
 */

#include <stdarg.h>
#include "stdperip.h"
#include <stdint.h>

static int debug_port = 0;

void SetDebug_Port(int port) {
	debug_port = port;
}

static char dec2char(int dec) {
	if (dec < 10) {
		return ('0' + dec);
	}

	return ('A' + dec-10);
}

int print_int(uint32_t dec) {
	int count = 0;
	int _dec = dec;
	int digit;
	int first = 1;

	if (dec == 0xbf) {
		digit  = 1;
	}

	uint32_t biggest = 1000000000; //biggest for unsigned 32 bit.
	while (_dec > 0) {
		digit = 1;
		if (_dec >= biggest) {
			digit = _dec/biggest;
			UART_PutChar(debug_port, dec2char(digit));
			_dec  -= biggest*digit;
			count++;
			first = 0;
		} else if (first == 0) {
			UART_PutChar(debug_port, '0');
		}
		biggest /= 10;
	}
	return count;
}

static int print_string(uint32_t str) {
	int count = 0;
	char *str_pointer = (char *) str;
	while (*str_pointer) {
		UART_PutChar(debug_port, *str_pointer);
		str_pointer++;
		count++;
	}
	return count;
}

int	uc_printf(char *format, ...) {
	int count = 0;

	va_list vl;
	va_start(vl, format);

	int index = 0;
	char ch;
	while ( (ch = format[index]) != 0) {
		if (ch == '%') {
			if ( format[index+1] == 'd') {
				count += print_int(va_arg(vl, uint32_t));
				index++;
			} else if ( format[index+1] == 's') {
				count += print_string(va_arg(vl, uint32_t));
				index++;
			} else if ( format[index+1] == 'c') {
				char _ch = va_arg(vl, uint32_t) ;
				UART_PutChar (debug_port, _ch);
				index++;
				count++;
			} else {
				UART_PutChar(debug_port, ch);
				count++;
			}
		} else {
			UART_PutChar(debug_port, ch);
			count++;
		}
		index++;
	}
	va_end(vl);
	return count;
}
