/*
 * UART.h
 *
 *  Created on: 10 Эях 2015
 *      Author: stelkork
 */

#ifndef UART_H_
#define UART_H_

#include "xuartlite.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "Interrupts.h"
#include "TransmitChannel.h"

#define PARSE_GET_LEN(byte) \
	( 0x3Fu & byte )

#define PARSE_GET_SEQ_NO(byte) \
	( (byte >> 6) & 0x1u )

#define PARSE_GET_ACK_NO(byte) \
	( (byte >> 7) & 0x1u )

/* Define UARTs' IDs (from xparameters.h) */
#define UART_1_DEV_ID	  	XPAR_UARTLITE_1_DEVICE_ID
#define UART_1_INT_IRQ_ID	XPAR_INTC_0_UARTLITE_1_VEC_ID
#define UART_2_DEV_ID	  	XPAR_UARTLITE_2_DEVICE_ID
#define UART_2_INT_IRQ_ID	XPAR_INTC_0_UARTLITE_2_VEC_ID


/************************** Address Definitions *****************************/
/* BASE ADDRESSES */
#define UART1_BASE_ADDR						XPAR_UARTLITE_1_BASEADDR
#define UART2_BASE_ADDR						XPAR_UARTLITE_2_BASEADDR

#define UART1_RTS_ADDR						XPAR_GPIO_1_BASEADDR
#define UART1_CTS_ADDR						(XPAR_GPIO_1_BASEADDR + 0x8)

#define RTS_ON		0x00
#define RTS_OFF 	0x01
#define CTS_ON		0x00

#define BUF_SIZE	1

#define	DAC_CO_AIN 0.007843137254
#define DAC_CO_BIN -1.0

#define REG_WRITE(Addr, Value) \
		(*(volatile unsigned int *)((Addr)) = (((unsigned int)(Value))))

#define REG_READ(Addr) \
		(*(volatile unsigned int *)(Addr))


int UART_Init(XUartLite *UART_Inst_Ptr_1, u16 Uart_1_Dev_ID,  XUartLite *UART_Inst_Ptr_2, u16 Uart_2_Dev_ID);
void ParseRecvFrame(unsigned char rx_byte);


/************************** Device Pointer Declarations *****************************/
XUartLite UART_Inst_Ptr_1;            /* The instance of the UartLite Device */
XUartLite UART_Inst_Ptr_2;            /* The instance of the UartLite Device */


extern const float coeffs[17];
extern volatile unsigned char enter_once;
extern volatile unsigned char parse_frame;
extern volatile unsigned char CanSend;
extern volatile unsigned short TotalSentCount_2;
extern unsigned char Rx[500];

#endif /* UART_H_ */
