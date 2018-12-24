#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "xil_printf.h"
#include "Interrupts.h"
#include "UART.h"
#include "Timer.h"
#include "RingBuffer.h"
#include "CRC.h"
#include "TransmitChannel.h"
#include "xparameters.h"
#include "xtmrctr.h"

int main()
{
	int Status;
    init_platform();

    *(volatile unsigned int *)(UART1_RTS_ADDR) = RTS_OFF;

    // Calculate table of CRC for fast computing later
    CalculateTable_CRC16();

//	RingBufferInit(&rb_tx);
//	RingBufferInit(&rb_rx);
	RingBufferInit(&ParseRecvBuffer);
	Timers_Init();
    UART_Init(&UART_Inst_Ptr_1, XPAR_UARTLITE_1_DEVICE_ID,  &UART_Inst_Ptr_2, XPAR_UARTLITE_2_DEVICE_ID);

    Status = SetupInterruptSystem(&UART_Inst_Ptr_1, UART_1_INT_IRQ_ID, &UART_Inst_Ptr_2, UART_2_INT_IRQ_ID,
    		&TimerCounterInst_0, TMRCTR_INTERRUPT_ID_0, &TimerCounterInst_1, TMRCTR_INTERRUPT_ID_1);

	if (Status != XST_SUCCESS)
		return XST_FAILURE;

    xil_printf("Started \n\r");
	*(volatile unsigned int *)(UART1_RTS_ADDR) = RTS_ON;

	unsigned char rx_ch;

	// initialize the Tx characters to be sent to framer and then to channel
	unsigned int i;
	volatile unsigned char flag = 1;
	while (flag)
    {
		while( RingBufferRead(&ParseRecvBuffer, &rx_ch) )
			ParseRecvFrame(rx_ch);

		stream_frame();
    }

	for( i = 0; i < 500; ++i) {
			xil_printf("idx at i: %d, %d \r\n",i,Rx[i]);
	}

	*(volatile unsigned int *)(UART1_RTS_ADDR) = RTS_OFF;
	cleanup_platform();
    return 0;
}
