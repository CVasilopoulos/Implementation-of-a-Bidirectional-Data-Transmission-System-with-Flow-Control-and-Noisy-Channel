
#include "TransmitChannel.h"
#include <stdlib.h>
#include "string.h"
#include "math.h"
#include "xil_printf.h"
#include "CRC.h"
#include "UART.h"
#include "RingBuffer.h"
#include "Timer.h"

volatile unsigned char SeqNo = 0;
volatile unsigned char AckNo = 0;
volatile send_state_t tx_event = SEND_FRAME;
static unsigned char frame[FRAME_SZ];
static unsigned char DAC_samples[FRAME_SZ_UP];
static unsigned int idx = 0;

/*
 * Brief	:	Build the frame
 * Return	:	void
 * Parameter: 	ptr to data, ptr to frame, length of frame
 */
static void MakeFrame(unsigned char *pre_frame, unsigned char *frame, unsigned char len)
{
	unsigned char i;
	static unsigned short crc;
	frame[0] = 85;
	frame[1] = 85;
	frame[2] = 85;
	frame[3] = 85;
	frame[4] = 85;
	frame[5] = 85;
	frame[6] = 85;
	frame[7] = 86;

	if (len > 1)
	{
		frame[8] = (len & 0x3F); // dont count the control field
		for (i = 9; i < 29; ++i)
			frame[i] = pre_frame[i-9];
	}
	else
	{
		frame[8] = (0 & 0x3F);
		memset((frame+9), 25, 21);
	}
	frame[8] |= ((SeqNo << 6) & 0x40);
	frame[8] |= ((AckNo << 7) & 0x80);

	crc = Fast_Lookup_CRC16(frame+8, 21);

	frame[29] = ((unsigned char)( crc & 0xFFu )) ; // LSB
	frame[30] = ((unsigned char)((crc >> 8) & 0xFFu )); // MSB
}

volatile unsigned char main_flag = 1;
unsigned char Tx[500];
static unsigned char ResendFrame[FRAME_SZ_UP];
unsigned int tick1, tick2;

volatile unsigned char end_flag = 0;
static unsigned char first = 1;

/*
 * Brief	:	Stream frame to channel - event cases (stop n wait protocol)
 * Return	:	void
 * Parameter: 	void
 */
void stream_frame()
{
	if (idx == 500)
		end_flag = 1;

	switch (tx_event)
	{
		case SEND_FRAME:
			// make frame for constant length of 20
			MakeFrame(&Tx[idx], frame, 20);
			idx += 20;
			SymbolsToUpsampling(frame, coeffs, DAC_samples, FRAME_SZ_UP, FRAME_SZ);
			tx_event = WAIT_FOR_ACK;
			memcpy(ResendFrame, DAC_samples, FRAME_SZ_UP);
			// timer start to measure in the end the elapsed time
			if (first) {
				first = 0;
				XTmrCtr_SetOptions(&TimerCounterInst_2,TIMER_CNTR_2,XTC_DOWN_COUNT_OPTION);
				XTmrCtr_Reset(&TimerCounterInst_2, TIMER_CNTR_2);
				tick1 = XTmrCtr_GetValue(&TimerCounterInst_2,TIMER_CNTR_2);
				XTmrCtr_Start(&TimerCounterInst_2, TIMER_CNTR_2);
			}
			// timer start for timeout period - Wait ACK or retransmission in fault case
			XTmrCtr_Start(&TimerCounterInst_0, TIMER_CNTR_0);
			// send to UART_2
			XUartLite_Send(&UART_Inst_Ptr_2, DAC_samples, FRAME_SZ_UP);
			while (TotalSentCount_2 != FRAME_SZ_UP);
			SeqNo = (SeqNo + 1) & 1;
			break;

		case SEND_ACK:
			MakeFrame(NULL, frame, 1);
			SymbolsToUpsampling(frame, coeffs, DAC_samples, FRAME_SZ_UP, FRAME_SZ);
			tx_event = DO_NOTHING;
			XUartLite_Send(&UART_Inst_Ptr_2, DAC_samples, FRAME_SZ_UP);
			while (TotalSentCount_2 != FRAME_SZ_UP);
			break;

		case RESEND_FRAME:
			tx_event = WAIT_FOR_ACK;
			REG_WRITE(LED_ADDRESS,0x00);
			// timer start for timeout period - Wait ACK or retransmission in fault case
			XTmrCtr_Start(&TimerCounterInst_0, TIMER_CNTR_0);
			XUartLite_Send(&UART_Inst_Ptr_2, ResendFrame, FRAME_SZ_UP);
			while (TotalSentCount_2 != FRAME_SZ_UP);

			break;

		case DO_NOTHING:
			break;

		case WAIT_FOR_ACK:
			break;

		case RESEND_ACK:
			break;

		default:
			break;
	}
}

/*
 * Brief	:	Upsampling Symbols and apply SRRC filter
 * Return	:	void
 * Parameter: 	void
 */
void SymbolsToUpsampling(unsigned char *packet, const float *h_ptr,
		unsigned char *DAC_samples, const unsigned short DAC_size, const unsigned char nBytes)
{
	unsigned short tmp = 0, i;
	unsigned char m, j, tmp1, tmp2;
	unsigned char idx;

	signed char *up_packet = (signed char *)calloc((DAC_size+33), sizeof(signed char));

	if (up_packet != NULL)
	  memset(up_packet, 0, (DAC_size+33));
	else
	  xil_printf("mem leak \r\n");


	for (i = 0; i < nBytes; ++i)
	{
		tmp2 = packet[i];
		for (j = 0; j < 8; ++j)
		{
		   tmp1 = (!!((tmp2) << j & 0x80));
		   if (tmp1 == 1)
		   {
				up_packet[tmp] = 1;
				up_packet[tmp + 4] = -1;
		   }
		   else
		   {
				up_packet[tmp] = -1;
				up_packet[tmp + 4] = 1;
		   }
		   tmp += 8;
		}
	}

	// Run SRRC filter with 17 coefficients, cause of symmetry in FIR filter
  	float sum = 0;
	for (i = 0; i < DAC_size; ++i)
	{
		for (m = 0, idx = 32; m < 16; ++m, --idx)
		{
			sum += *(h_ptr + m) * ( *(up_packet + m + i) + *(up_packet + idx + i) );
		}
		sum += (*(h_ptr + idx) * *(up_packet + idx + i));

		sum = fmaxf(-1, fminf(1, sum));
		DAC_samples[i] = (unsigned char)(round((127.5 * sum) + 127.5));
		sum = 0;
	}

	free(up_packet);
}


