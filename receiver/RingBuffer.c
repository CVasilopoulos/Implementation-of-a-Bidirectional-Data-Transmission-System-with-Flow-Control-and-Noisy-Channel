/*
 * RingBuffer.c
 *
 *  Created on: Nov 2, 2016
 *      Author: TasosPet
 */

#include <string.h>
#include "RingBuffer.h"
#include "xil_printf.h"
#include "TransmitChannel.h"

ring_buffer_t rb_tx;
ring_buffer_t rb_rx;
ring_buffer_t ParseRecvBuffer;

/*
 * Brief	:	Check if the ring buffer is full
 * Return	:	true/false
 * Parameter: 	ring buffer pointer
 */
static
unsigned char RingBufferIsFull(ring_buffer_t *rb)
{
	return ( ( rb->wr_idx - rb->rd_idx ) & RING_BUF_SIZE_MASK ) == RING_BUF_SIZE_MASK;
}

/*
 * Brief	:	Check if the ring buffer is empty
 * Return	:	true/false
 * Parameter: 	ring buffer pointer
 */
static
unsigned char RingBufferIsEmpty(ring_buffer_t *rb)
{
	return ( rb->wr_idx == rb->rd_idx );
}

/*
 * Brief	:	number of items that are existed in the ring buffer
 * Return	:	true/false
 * Parameter: 	ring buffer pointer
 */
rb_size_t RingBufferNumItems(ring_buffer_t *rb)
{
	return ( (rb->wr_idx - rb->rd_idx) & RING_BUF_SIZE_MASK );
}

/*
 * Brief	:	number of items that can be written in the ring buffer
 * Return	:	true/false
 * Parameter: 	ring buffer pointer
 */
rb_size_t RingBufferFreeItems(ring_buffer_t *rb)
{ // panta deixnei RING_BUF_SIZE - 1 max size
	return ( RING_BUF_SIZE_MASK - RingBufferNumItems(rb) );
}

/*
 * Brief	:	Check if the ring buffer has reached high level
 * Return	:	true/false
 * Parameter: 	ring buffer pointer
 */
unsigned char ring_buffer_hwm_reached(ring_buffer_t *rb)
{
	return ( (rb->wr_idx - rb->rd_idx) & RING_BUF_SIZE_MASK ) >= HWM_BUF_SIZE;
}

/*
 * Brief	:	Check if the ring buffer has reached low level
 * Return	:	true/false
 * Parameter: 	ring buffer pointer
 */
unsigned char ring_buffer_lwm_reached(ring_buffer_t *rb)
{
	return ( (rb->wr_idx - rb->rd_idx) & RING_BUF_SIZE_MASK ) <= LWM_BUF_SIZE;
}

/*
 * Brief	:	Ring buffer initialization
 * Return	:	none
 * Parameter: 	ring buffer pointer
 */
void RingBufferInit(ring_buffer_t *rb)
{
	rb->rd_idx = 0;
	rb->wr_idx = 0;
}

/*
 * Brief	:	Push data to ring buffer
 * Return	:	true/false
 * Parameter: 	ring buffer pointer, data to be written
 */
unsigned char RingBufferWrite(ring_buffer_t *rb, const unsigned char data)
{
	if ( RingBufferIsFull(rb) )
	{
		return 0;
	}

	rb->buf[rb->wr_idx] = data;
	++rb->wr_idx;
	rb->wr_idx &= RING_BUF_SIZE_MASK;

	return 1;
}

/*
 * Brief	:	Pull data from ring buffer
 * Return	:	true/false
 * Parameter: 	ring buffer pointer, data read pointer
 */
unsigned char RingBufferRead(ring_buffer_t *rb, unsigned char *data)
{
	if (RingBufferIsEmpty(rb))
	{
		return 0;
	}

	*data = rb->buf[rb->rd_idx];
	++rb->rd_idx;
	rb->rd_idx &= RING_BUF_SIZE_MASK;

	return 1;
}

unsigned char RingBufferWriteArray(ring_buffer_t *rb, const unsigned char *data, rb_size_t len)
{
	rb_size_t nItemsFree;
	nItemsFree = RingBufferFreeItems(rb);

	if ( nItemsFree > len )
	{
		memcpy(&( rb->buf[rb->wr_idx] ), data, len);
		rb->wr_idx += len;
		rb->wr_idx &= RING_BUF_SIZE_MASK;
		return 1;
	}
	else
		return 0;
}

/*
 * Brief	:	Pull array of data from ring buffer
 * Return	:	true/false
 * Parameter: 	ring buffer pointer, data read pointer, length of data
 */
rb_size_t RingBufferReadArray(ring_buffer_t *rb, unsigned char *data, rb_size_t len)
{
	static rb_size_t nItems;
	nItems = RingBufferNumItems(rb); // items that are existed in the rb and can read

	if (nItems > len) // len usually here is the MAX_PAYLOAD_SIZE
		nItems = len;
	else if (nItems == 0)
		return 0;

	memcpy(data, &( rb->buf[rb->rd_idx] ), nItems);
	rb->rd_idx += nItems;
	rb->rd_idx &= RING_BUF_SIZE_MASK;

	return nItems; // returns the len of data read
}

