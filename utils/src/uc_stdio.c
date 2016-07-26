/*
 * uc_stdio.c
 *
 *  Created on: Nov 10, 2015
 *      Author: mmarinas
 */

#include "uc_stdio.h"
#include <stdarg.h>
#include <string.h>
#ifdef TEST_STDIO
	#include <stdio.h>
#else
	#include "stdperip.h"
#endif

#define FORMAT_INT	0
#define FORMAT_FLOAT	1	
#define FORMAT_CHAR	2
#define FORMAT_STRING	3
#define FORMAT_HEX	4


static int debug_port = 0;

void SetDebug_Port(int port) {
	debug_port = port;
}

static int get_format(char format)
{
	switch(format) {
		case 'd':
			return FORMAT_INT;
		case 'c':
			return FORMAT_CHAR;
		case 'f':
			return FORMAT_FLOAT;
		case 's':
			return FORMAT_STRING;
		case 'x':
			return FORMAT_HEX;
		default:
			return -1;
	}
}

static int print_char(char ch)
{
	#ifdef TEST_STDIO
		printf ("%c", ch);
	#else
		UART_PutChar(debug_port, ch);
	#endif

	return 0;
}

static int print_int(unsigned int num, int format)
{
	char intstr[10] = { 0 }; //the biggest int is 4294967296, which is 10 characters; 
	int len = 0;
	int div = (format == FORMAT_INT)?10:16;
	int _num = num;
	int idx;

	if (num == 0) {
		len++;
	}

	while(num) {
		int remainder = num%div;
		if (remainder < 10) {
			intstr[len++] = '0' + remainder;
		} else {
			intstr[len++] = '7' + remainder; //instead of saying A + (remainder%10). '7' is 'A' - 10
		}
		num = num/div;	
	}

	for (idx=len-1; idx >= 0; idx--) {
		print_char(intstr[idx]);
	}	
	return len;
}

static int print_float(double f)
{
        int printed_chars = 0;
        int whole = (int) f;
        printed_chars += print_int(whole, FORMAT_INT);
        printed_chars++;
        print_char('.');
        float fdecimals = (f - (float) whole)*1000;
        int decimals = (int) fdecimals;
        printed_chars += print_int(decimals, FORMAT_INT);

        return printed_chars;

}

int print_string (int str, int format)
{
	char *pstr = (unsigned char *)str;
	int printed_chars;
	while(*pstr) {
		print_char(*pstr);
		pstr++;
		printed_chars++;
	}
	return printed_chars;
}

static int print(int format, int str)
{
	switch (format) {
		case FORMAT_INT:
			return (print_int(str, FORMAT_INT));
		case FORMAT_HEX:
			return (print_int(str, FORMAT_HEX));
		case FORMAT_STRING:
			return (print_string(str, FORMAT_STRING));
		case FORMAT_CHAR:
			print_char(str);
			return 1;
		default:
			return -1;
	}
}

int uc_printf(const char *format, ...) 
{
	int idx;
	int print_format;
	va_list vl;
	int chars_printed = 0;
	int format_len = strlen(format);
	
	va_start(vl, format);

	for (idx=0; idx<format_len; idx++) {
		if (format[idx] == '%') {
			idx++;
			print_format = get_format(format[idx]); 
			if (print_format < 0) {
				return -1;
			}
			if (print_format != FORMAT_FLOAT) {
				chars_printed += print(print_format,va_arg(vl, int));
			} else {
				chars_printed += print_float(va_arg(vl, double) );
			}
		} else {
			print_char(format[idx]);
			chars_printed++;
		}
	}		
	va_end(vl);
	return chars_printed;
}
int uc_sprintf(char *str, const char *format, ...) 
{

}
int uc_snprintf(char *str, int size, const char *format, ...) 
{

}


#ifdef TEST_STDIO
int main()
{
	uc_printf("%f\n", 1.5);
	uc_printf("%d\n", 150);
	uc_printf("%x\n", 150);
	uc_printf("%c\n", 'A');
	uc_printf("%s\n", "This is a string");

	uc_printf ("My name is %s and I am %x years old. I stand %f tall\n", "mark", 36, 5.7);

	return 0;
}
#endif

