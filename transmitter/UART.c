
#include "string.h"
#include "UART.h"
#include "Interrupts.h"
#include "RingBuffer.h"
#include "Timer.h"
#include "CRC.h"
#include "TransmitChannel.h"
#include "math.h"
#include "xil_printf.h"
#include "xuartlite_l.h"

volatile unsigned char enter_once = 0;
volatile unsigned char parse_frame = 0;
volatile unsigned char CanSend = 1;

// coefficients for SRRC filter
// use of 17 coefficients, cause of FIR filter symmetry
const float coeffs[17] = {0.001312776772708,
								0.008559203621943,
								0.006150923988750,
								-0.006135952548125,
								-0.016355772279267,
								-0.009494078779339,
								0.016459431708466,
								0.041988360136140,
								0.036703163551880,
								-0.014183217311842,
								-0.086852432606889,
								-0.121209984634712,
								-0.054419440606070,
								0.132929271640255,
								0.390537211695895,
								0.615020626771225,
								0.704021649600782};

static signed char EndPreSympat[28] = {1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1, 1, -1};
static float PowerTaps[64];
static float RRC_Taps[33];
static float Midobit[16];
static signed char PreSymbols[28];
static unsigned char bitPacket[8];
static unsigned char RxBuffer[CNTRL_FIELD_SZ+PAYLOAD_SZ+CRC_SZ];
volatile unsigned short TotalSentCount_2;
unsigned char Rx[500];
static unsigned int recv_idx = 0;
static double clkPeriod = (double)(1.0/(double)XPAR_TIMER_0_CLOCK_FREQ_HZ);

/*
 * Brief	:	Convert to symbol
 * Return	:	void
 * Parameter: 	void
 */
static unsigned char ToSymbol(const unsigned char *c)
{
    return ( ( c[0] & 1 ) ? 0x80 : 0x00 ) |
           ( ( c[1] & 1 ) ? 0x40 : 0x00 ) |
           ( ( c[2] & 1 ) ? 0x20 : 0x00 ) |
           ( ( c[3] & 1 ) ? 0x10 : 0x00 ) |
           ( ( c[4] & 1 ) ? 0x08 : 0x00 ) |
           ( ( c[5] & 1 ) ? 0x04 : 0x00 ) |
           ( ( c[6] & 1 ) ? 0x02 : 0x00 ) |
           ( ( c[7] & 1 ) ? 0x01 : 0x00 );
}

static void SendHandler_UART_1(void *CallBackRef, unsigned int EventData)
{
}

static void RecvHandler_UART_1(void *CallBackRef, unsigned int EventData)
{
	unsigned char rx_char;

	if ( ring_buffer_hwm_reached(&rb_tx) && !enter_once )
	{
		*(volatile unsigned int *)(UART1_RTS_ADDR) = RTS_OFF;
		enter_once = 1;
		*(volatile unsigned int *)(LED_ADDRESS) = 0xF;
		// start the timer -> checking periodically for RTS = ON
		XTmrCtr_Start(&TimerCounterInst_1, TIMER_CNTR_1);
	}
	XUartLite_Recv(&UART_Inst_Ptr_1, &rx_char, BUF_SIZE);
	RingBufferWrite(&rb_tx, rx_char);
}

/*
 * Brief	:	UART 2 Send intr handler
 * Return	:	void
 * Parameter: 	void
 */
static void SendHandler_UART_2(void *CallBackRef, unsigned int EventData)
{
	TotalSentCount_2 = EventData;
}

/*
 * Brief	:	UART2 recv intr handler
 * Return	:	void
 * Parameter: 	void
 */
static void RecvHandler_UART_2(void *CallBackRef, unsigned int EventData)
{
	unsigned char rx_char;
	static int nByte = -1;
	nByte += XUartLite_Recv(&UART_Inst_Ptr_2, &rx_char, BUF_SIZE);
	RingBufferWrite(&ParseRecvBuffer, rx_char);
}

void ParseRecvFrame(unsigned char rx_byte)
{
	static unsigned char EoF_detected = 0, vio_flag = 0;
	static unsigned short i;
	static unsigned char j;
	static unsigned char frame_status = 0;
	static unsigned short ind_start, mob_cnt, ind_mob, ind_symb, ind_lapre, bit_ind = 0; /// ind_end
	static unsigned int cnt = 0;
	static unsigned char sy2bi, frame_byte = 0;
	static unsigned char RRC_Taps_idx = 0;
	static unsigned char bit;
	static signed char b1s, b2s;
	static float vin;
	static float acc = 0;
	static float RRC_Output;
	static float powVal, previousPowVal = 0;
	static unsigned short crc;
	static unsigned char ackNo, seqNo, frameLen, buf_temp;
	static unsigned int led_val = 0x1;

	++cnt; // kathe fora poy erxetai to neo stoixeio

	if (vio_flag && cnt < 1984)
		return;
	else if (vio_flag && cnt == 1984)
		goto WRONG_FRAME_END;

	if ( !EoF_detected )
	{
		vin = (float)(DAC_CO_AIN * rx_byte + DAC_CO_BIN);

		powVal = previousPowVal + (float)( ( (vin * vin) - (PowerTaps[63] * PowerTaps[63]) ) / 64.0 );
		previousPowVal = powVal;

		for (i = 63; i > 0; --i)
			*(PowerTaps+i) = *(PowerTaps+i-1);

		PowerTaps[0] = vin;

		// SRRC filter - management as a cyclic buffer
		for (i = 0, j = 32; i < 16; ++i, --j)
			acc += coeffs[i] * ( RRC_Taps[(RRC_Taps_idx + i) % 33] + RRC_Taps[(RRC_Taps_idx + j) % 33] );

		acc += coeffs[j] * RRC_Taps[(RRC_Taps_idx + j) % 33];

		RRC_Output = acc;
		acc = 0;

		RRC_Taps[RRC_Taps_idx++] = vin;
		RRC_Taps_idx %= 33;

		/// cases
		if ( frame_status == 0 && powVal > 0.3 )
		{
			frame_status = 1;
			ind_start = cnt;
			mob_cnt = 0;
//			xil_printf("SoF detected at: %d \r\n", ind_start); fflush(stdout);
		}

		if (frame_status == 1)
		{
			Midobit[( (cnt - 2) & 15 )] += vin;
			Midobit[( (cnt - 1) & 15 )] += vin;
			Midobit[( (cnt) & 15)] += vin;
			Midobit[( (cnt + 1) & 15)] += vin;
			Midobit[( (cnt + 2) & 15)] += vin;
			++mob_cnt;

			if (mob_cnt == 48)
			{
				float min_midobit = (Midobit[0] > 0.0) ? Midobit[0] : (- Midobit[0]);
				min_midobit = roundf(min_midobit *100)/100;
				for (i = 0; i < 16; ++i)
				{
					float tmp_min = (Midobit[i] > 0.0) ? Midobit[i] : (- Midobit[i]);
					tmp_min = roundf(tmp_min*100)/100 ;
					if ( min_midobit > tmp_min )
					{
						min_midobit = tmp_min;
						ind_mob = i;
					}
				}
				frame_status = 2;
				unsigned char temp = (cnt & 15);
				ind_mob = cnt - temp + (ind_mob );
				ind_mob = (ind_mob > cnt) ? (ind_mob - 16) : ind_mob ;
//				xil_printf("Midobit at sample: %d \r\n", ind_mob);
				ind_symb = ind_mob - 2;
				memset(PreSymbols, 0, 28);
			}
		}

		if (frame_status == 2)
		{
			if ( !((cnt - ind_symb) & 3) )
			{
				for (i = 27; i > 0; --i)
					*(PreSymbols + i) = *(PreSymbols + i -1);

				PreSymbols[0] = (RRC_Output >= 0) ? 1 : -1;
			}
			static unsigned char end_pre = 0;
			end_pre = 0;
			for (i = 0; i < 28; ++i)
			{
				if (PreSymbols[i] == EndPreSympat[i] )
					++end_pre;
			}
			if (end_pre == 28)
			{
				frame_status = 3;
				ind_lapre = cnt;
				sy2bi = 0;
//				xil_printf("EoP detected at: %d \r\n", ind_lapre);
			}
		}

		if (frame_status == 3)
		{
			if ( cnt > ind_lapre && (!((cnt - ind_lapre) & 3)) )
			{
				if (sy2bi == 0)
				{
					b1s = (RRC_Output >= 0) ? 1 : -1;
					sy2bi = 1;
				}
				else
				{
					b2s = (RRC_Output >= 0) ? 1 : -1;
					sy2bi = 0;
					if ( b1s == -1 && b2s == 1 )
						bit = 0;
					else if ( b1s == 1 && b2s == -1)
						bit = 1;
					else
					{
						bit = 1;
						vio_flag = 1;
						return;
					}
					bitPacket[bit_ind++] = bit; /// isos exception handling sto bit level TODO!!!!!!
					if (bit_ind == 8)
					{
						RxBuffer[frame_byte++] = ToSymbol(bitPacket);
						bit_ind = 0;
					}
					if (frame_byte == 23) /// exei erthei olo to length tou frame payload
					{
						EoF_detected = 1;
						crc = (((unsigned short)RxBuffer[PAYLOAD_SZ+CRC_SZ])<<8) | RxBuffer[PAYLOAD_SZ+CRC_SZ-1];
						buf_temp = RxBuffer[0];
						seqNo = PARSE_GET_SEQ_NO(buf_temp);
						ackNo = PARSE_GET_ACK_NO(buf_temp);
						frameLen = PARSE_GET_LEN(buf_temp);
						led_val ^= 0x1;
						REG_WRITE(LED_ADDRESS,led_val);

						if ( ( crc == Fast_Lookup_CRC16(RxBuffer,21)) )
						{
							if (frameLen == 20)
							{
								if (seqNo == AckNo)
								{
									for (i = 0; i < 20; ++i)
										Rx[recv_idx+i] = RxBuffer[i+1];
									recv_idx += 20;
									AckNo = (AckNo + 1) & 1;
									tx_event = SEND_ACK;
//									xil_printf("send ack \r\n");
								}
							}
							else {
								if (ackNo == SeqNo) {
									XTmrCtr_Stop(&TimerCounterInst_0, TIMER_CNTR_0);
									tx_event = SEND_FRAME;
//									xil_printf("send frame \r\n");

									// check the elapsed time
									if (end_flag)
									{
										XTmrCtr_Stop(&TimerCounterInst_2, TIMER_CNTR_2);
										tick2 = XTmrCtr_GetValue(&TimerCounterInst_2, TIMER_CNTR_2);
										double elapsedTime = (double)(tick2-tick1)*clkPeriod;
//										xil_printf("elapsedTime: %f \r\n",elapsedTime);
										main_flag = 0;
									}
								}
							}
						}
						frame_status = 0;
						memset(PowerTaps, 0, 64*sizeof(float));
						memset(RRC_Taps, 0, 33*sizeof(float));
						memset(Midobit, 0, 16*sizeof(float));
						previousPowVal = 0;
						RRC_Taps_idx = 0;
						frame_byte = 0;
						bit_ind = 0;
					}
				}
			}
		}
	}
	if (cnt == FRAME_SZ_UP)
	{
WRONG_FRAME_END:
		EoF_detected = 0;
		cnt = 0;
		vio_flag = 0;
	}
}

int UART_Init(XUartLite *UART_Inst_Ptr_1, u16 Uart_1_Dev_ID,  XUartLite *UART_Inst_Ptr_2, u16 Uart_2_Dev_ID)
{
	int Status;

	Status = XUartLite_Initialize(UART_Inst_Ptr_1, Uart_1_Dev_ID);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	Status = XUartLite_Initialize(UART_Inst_Ptr_2, Uart_2_Dev_ID);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	XUartLite_ResetFifos(UART_Inst_Ptr_1);
	XUartLite_ResetFifos(UART_Inst_Ptr_2);

	XUartLite_SetSendHandler(UART_Inst_Ptr_1, SendHandler_UART_1, UART_Inst_Ptr_1);
	XUartLite_SetRecvHandler(UART_Inst_Ptr_1, RecvHandler_UART_1, UART_Inst_Ptr_1);

	XUartLite_SetSendHandler(UART_Inst_Ptr_2, SendHandler_UART_2, UART_Inst_Ptr_2);
	XUartLite_SetRecvHandler(UART_Inst_Ptr_2, RecvHandler_UART_2, UART_Inst_Ptr_2);

	XUartLite_EnableInterrupt(UART_Inst_Ptr_1);
	XUartLite_EnableInterrupt(UART_Inst_Ptr_2);

	return XST_SUCCESS;
}

