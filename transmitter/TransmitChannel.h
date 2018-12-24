
#ifndef TRANSMITCHANNEL_H_
#define TRANSMITCHANNEL_H_

#include "stdio.h"

typedef enum
{
	SEND_FRAME,
	RESEND_FRAME,
	WAIT_FOR_ACK,
	SEND_ACK, // obsolette just for testing
	RESEND_ACK,
	DO_NOTHING,
} send_state_t;

#define HEADER_SZ	 		8
#define HEADER_SZ_UP 		512 // (8*64) is constant
#define CNTRL_FIELD_SZ		1
#define CNTRL_FIELD_SZ_UP	64
#define PAYLOAD_SZ			20
#define PAYLOAD_SZ_UP		(20*64)
#define CRC_SZ 				2
#define CRC_SZ_UP			(2*64)
#define FRAME_SZ 			(HEADER_SZ+CNTRL_FIELD_SZ+PAYLOAD_SZ+CRC_SZ)
#define FRAME_SZ_UP			(HEADER_SZ_UP+CNTRL_FIELD_SZ_UP+PAYLOAD_SZ_UP+CRC_SZ_UP)
#define FRAME_SZ_UP_NO_CRC	(FRAME_SZ_UP-CRC_SZ_UP)
#define FRAME_SZ_NO_CRC		(FRAME_SZ-CRC_SZ)

void stream_frame();
void SymbolsToUpsampling(unsigned char *packet, const float *h_ptr,
		unsigned char *DAC_samples, const unsigned short DAC_size, const unsigned char nBytes);

extern volatile unsigned char SeqNo;
extern volatile unsigned char AckNo;
extern volatile send_state_t tx_event;
extern unsigned char Tx[500];
extern volatile unsigned char main_flag;
extern volatile unsigned char end_flag;
extern unsigned int tick1, tick2;

#endif /* TRANSMITCHANNEL_H_ */
