/*
 * retarget.c
 *
 *  Created on: Oct 26, 2015
 *      Author: mark.marinas
 */

#include <stdio.h>

/*
int fputc(int ch, FILE *f) {
    return 0;
}

int fgetc(FILE *f) {
	return 0;
}
*/

int _write(int ch, FILE *f) {
	return 0;
}

int _close_r(FILE *f) {
	return 0;
}

int _sbrk(FILE *f) {
	return 0;
}

int _fstat_r(FILE *f) {
	return 0;
}

int _isatty_r(FILE *F) {
	return 0;
}

int _lseek_r(FILE *f) {
	return 0;
}

int _read(FILE *f) {
	return 0;
}

FILE __stdin;
FILE __stdout;
