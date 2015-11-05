/*
 * fifo.h
 *
 *  Created on: Oct 29, 2015
 *      Author: mark.marinas
 */

#ifndef FIFO_H_
#define FIFO_H_

#include "data_types.h"

struct fifo_t {
	int size;
	int head, tail;
	void *buffer;
	void (*fn_fifo_get)(struct fifo_t *fifo, void *data);
	void (*fn_fifo_put)(struct fifo_t *fifo, void *data);
} ;

typedef struct fifo_t fifo_t;

error_code_t FIFO_Put(fifo_t *fifo, void *data);
error_code_t FIFO_Get(fifo_t *fifo, void *data);
error_code_t FIFO_Init(fifo_t *fifo, int size, void *buffer, void (*fn_fifo_get)(fifo_t *fifo, void *data), void (*fn_fifo_put)(fifo_t *fifo, void *data)  );
error_code_t FIFO_Clr(fifo_t *fifo);





#endif /* FIFO_H_ */
