/*
 * RingBuffer.h
 *
 *  Created on: Nov 2, 2016
 *      Author: TasosPet
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#define RING_BUF_SIZE 		2048
#define RING_BUF_SIZE_MASK (RING_BUF_SIZE-1)
#define HWM_BUF_SIZE 		153
#define LWM_BUF_SIZE 		51

typedef unsigned short rb_size_t; // caution this holds indices e.t.c (max limit 65535)

typedef struct
{
	unsigned char buf[RING_BUF_SIZE];
	volatile rb_size_t wr_idx;
	volatile rb_size_t rd_idx;
} ring_buffer_t;


rb_size_t RingBufferNumItems(ring_buffer_t *rb);

rb_size_t RingBufferFreeItems(ring_buffer_t *rb);

unsigned char ring_buffer_hwm_reached(ring_buffer_t *rb);

unsigned char ring_buffer_lwm_reached(ring_buffer_t *rb);

void RingBufferInit(ring_buffer_t *rb);

unsigned char RingBufferWrite(ring_buffer_t *rb, const unsigned char data);

unsigned char RingBufferRead(ring_buffer_t *rb,  unsigned char *data);

unsigned char RingBufferWriteArray(ring_buffer_t *rb, const unsigned char *data, rb_size_t len);

rb_size_t RingBufferReadArray(ring_buffer_t *rb, unsigned char *data, rb_size_t len);

extern ring_buffer_t rb_tx;
extern ring_buffer_t rb_rx;
extern ring_buffer_t ParseRecvBuffer;

#endif /* RINGBUFFER_H_ */
