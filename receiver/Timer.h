/*
 * Timer.h
 *
 *  Created on: 10 Эях 2015
 *      Author: stelkork
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "xparameters.h"
#include "xtmrctr.h"
#include "xil_exception.h"
#include "xil_printf.h"

#define TIMER_CNTR_0		 		XPAR_TIMER_0_DEVICE_ID
#define TIMER_CNTR_1	 			XPAR_TIMER_1_DEVICE_ID
#define TIMER_CNTR_2	 			XPAR_TIMER_2_DEVICE_ID
#define TMRCTR_INTERRUPT_ID_0		XPAR_INTC_0_TMRCTR_0_VEC_ID
#define TMRCTR_INTERRUPT_ID_1		XPAR_INTC_0_TMRCTR_1_VEC_ID
#define TMRCTR_INTERRUPT_ID_2		XPAR_INTC_0_TMRCTR_2_VEC_ID

/*
 * The following constant is used to set the reset value of the timer counter,
 * making this number larger reduces the amount of time this example consumes
 * because it is the value the timer counter is loaded with when it is started
 */
#define RESET_0_VALUE	 		0xF1194D81 // Resend
#define RESET_1_VALUE	 		0xFFF85EE1 // RTS checking
#define RESET_2_VALUE	 		0xFFF85EE1 // Scheduler

#define LED_ADDRESS				XPAR_LEDS_BASEADDR

// 0xFE8287C1 -> 0.25 sec
// 0xFF676981 -> 0.1 sec
// 0xFFF85EE1 -> 5 ms
// 0xFFB3B4C1 -> 50 ms
// 0xFD050F81 -> 500 ms // PERIPOU 43 PAYLOAD_SIZE
// 0xFBD3E281 -> 700 ms


void Timers_Init();

XTmrCtr TimerCounterInst_0;
XTmrCtr TimerCounterInst_1;
XTmrCtr TimerCounterInst_2;


#endif /* TIMER_H_ */
