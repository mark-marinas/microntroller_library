/*
 * fifo.c
 *
 *  Created on: Oct 29, 2015
 *      Author: mark.marinas
 */

#include "fifo.h"

static error_code_t FIFO_ISFull(fifo_t *fifo);
static error_code_t FIFO_ISEmpty(fifo_t *fifo);
static int FIFO_GetNext(fifo_t *fifo, int prev);
static void fn_get_default(fifo_t *fifo, void *data);
static void fn_put_default(fifo_t *fifo, void *data);

static void fn_put_default(fifo_t *fifo, void *data) {
	char *_data = (char *)data;
	char *_buffer;

	_buffer = (char *) (fifo->buffer);
	_buffer = &(_buffer[fifo->tail]);

	*_buffer = *(_data);
}

static void fn_get_default(fifo_t *fifo, void *data) {
	char *_data = data;
	char *_buffer = (char *) (fifo->buffer);
	*_data  = _buffer[fifo->head] ;
}

static int FIFO_GetNext(fifo_t *fifo, int prev) {
	int next;
	if ( (next = prev + 1 ) >= fifo->size) {
		next = 0;
	}

	return next;
}

error_code_t FIFO_Put(fifo_t *fifo, void *data) {
	if ( FIFO_ISFull(fifo) == FIFO_FULL) {
		return FIFO_FULL;
	}
	fifo->fn_fifo_put(fifo, data);
	fifo->tail = FIFO_GetNext(fifo, fifo->tail);
	return NO_ERROR;
}

error_code_t FIFO_Get(fifo_t *fifo, void *data) {
	if (FIFO_ISEmpty(fifo) == FIFO_EMPTY) {
		return FIFO_EMPTY;
	}
	fifo->fn_fifo_get(fifo, data);
	fifo->head = FIFO_GetNext(fifo, fifo->head);
	return NO_ERROR;
}

error_code_t FIFO_Init(fifo_t *fifo, int size, void *buffer, void (*fn_fifo_get)(fifo_t *fifo, void *data), void (*fn_fifo_put)(fifo_t *fifo, void *data)  ) {
	if (size == 0 || buffer == 0) {
		return FIFO_INIT_ERROR;
	}

	if (fn_fifo_put == 0 && fn_fifo_get == 0) {
		fifo->fn_fifo_put = fn_put_default ;
		fifo->fn_fifo_get = fn_get_default ;
	} else {
		fifo->fn_fifo_put = fn_fifo_put ;
		fifo->fn_fifo_get = fn_fifo_get ;
	}

	fifo->head = fifo->tail = 0;
	fifo->size = size;
	fifo->buffer = buffer;

	return NO_ERROR;
}

static error_code_t FIFO_ISFull(fifo_t *fifo) {
	int next = FIFO_GetNext(fifo, fifo->tail);
	if (fifo->head == next) {
		return FIFO_FULL;
	}
	return NO_ERROR;
}

static error_code_t FIFO_ISEmpty(fifo_t *fifo) {
	if (fifo->head == fifo->tail) {
		return FIFO_EMPTY;
	}
	return NO_ERROR;
}

error_code_t FIFO_Clr(fifo_t *fifo) {
	fifo->head = fifo->tail = 0;
	return NO_ERROR;
}
