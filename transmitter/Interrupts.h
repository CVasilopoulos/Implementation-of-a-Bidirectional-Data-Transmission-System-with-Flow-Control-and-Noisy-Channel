/*
 * Interrupts.h
 *
 *  Created on: 10 Эях 2015
 *      Author: stelkork
 */

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include "xparameters.h"
#include "xintc.h"
#include "xuartlite.h"
#include "xtmrctr.h"

int SetupInterruptSystem(XUartLite *UartLitePtr_1, u8 UartLite_1_Id, XUartLite *UartLitePtr_2, u8 UartLite_2_Id,
		XTmrCtr* TmrCtrInstancePtr_0, u8 Timer_0_Id, XTmrCtr* TmrCtrInstancePtr_1, u8 Timer_1_Id);

#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID

XIntc InterruptController;

#endif /* INTERRUPTS_H_ */
