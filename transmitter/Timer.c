
#include "Timer.h"
#include "Interrupts.h"
#include "RingBuffer.h"
#include "UART.h"


static void Timer0CounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	tx_event = RESEND_FRAME;
	REG_WRITE(LED_ADDRESS,0xF0);
	XTmrCtr_Stop((XTmrCtr *)CallBackRef, TmrCtrNumber);
}

static void Timer1CounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	if ( ring_buffer_lwm_reached(&rb_tx) )
	{
		REG_WRITE(UART1_RTS_ADDR,RTS_ON);
		enter_once = 0;
		REG_WRITE(LED_ADDRESS,0x0);
		XTmrCtr_Stop((XTmrCtr *)CallBackRef, TmrCtrNumber);
	}
}

static void Timer2CounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
}

static void Timer0_Init(XTmrCtr *TmrCtrInstancePtr, u8 TmrCtrNumber, u16 DeviceId)
{
	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires, specify a pointer to the
	 * timer counter driver instance as the callback reference so the handler
	 * is able to access the instance data
	 */
	XTmrCtr_SetHandler(TmrCtrInstancePtr, Timer0CounterHandler, TmrCtrInstancePtr);

	/*
	 * Enable the interrupt of the timer counter so interrupts will occur
	 * and use auto reload mode such that the timer counter will reload
	 * itself automatically and continue repeatedly, without this option
	 * it would expire once only
	 */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TmrCtrNumber,	XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

	/*
	 * Set a reset value for the timer counter such that it will expire
	 * earlier than letting it roll over from 0, the reset value is loaded
	 * into the timer counter when it is started
	 */
	XTmrCtr_SetResetValue(TmrCtrInstancePtr, TmrCtrNumber, RESET_0_VALUE);

	/*
	 * Start the timer counter such that it's incrementing by default,
	 * then wait for it to timeout a number of times
	 */
//	XTmrCtr_Start(TmrCtrInstancePtr_0, TmrCtrNumber_0);

}

static void Timer1_Init(XTmrCtr *TmrCtrInstancePtr, u8 TmrCtrNumber, u16 DeviceId)
{
	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires, specify a pointer to the
	 * timer counter driver instance as the callback reference so the handler
	 * is able to access the instance data
	 */
	XTmrCtr_SetHandler(TmrCtrInstancePtr, Timer1CounterHandler, TmrCtrInstancePtr);

	/*
	 * Enable the interrupt of the timer counter so interrupts will occur
	 * and use auto reload mode such that the timer counter will reload
	 * itself automatically and continue repeatedly, without this option
	 * it would expire once only
	 */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TmrCtrNumber,	XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

	/*
	 * Set a reset value for the timer counter such that it will expire
	 * earlier than letting it roll over from 0, the reset value is loaded
	 * into the timer counter when it is started
	 */
	XTmrCtr_SetResetValue(TmrCtrInstancePtr, TmrCtrNumber, RESET_1_VALUE);

	/*
	 * Start the timer counter such that it's incrementing by default,
	 * then wait for it to timeout a number of times
	 */
	//XTmrCtr_Start(TmrCtrInstancePtr_1, TmrCtrNumber_1);

}

static void Timer2_Init(XTmrCtr *TmrCtrInstancePtr, u8 TmrCtrNumber, u16 DeviceId)
{
	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires, specify a pointer to the
	 * timer counter driver instance as the callback reference so the handler
	 * is able to access the instance data
	 */
	XTmrCtr_SetHandler(TmrCtrInstancePtr, Timer2CounterHandler, TmrCtrInstancePtr);

	/*
	 * Enable the interrupt of the timer counter so interrupts will occur
	 * and use auto reload mode such that the timer counter will reload
	 * itself automatically and continue repeatedly, without this option
	 * it would expire once only
	 */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TmrCtrNumber,	XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

	/*
	 * Set a reset value for the timer counter such that it will expire
	 * earlier than letting it roll over from 0, the reset value is loaded
	 * into the timer counter when it is started
	 */
	XTmrCtr_SetResetValue(TmrCtrInstancePtr, TmrCtrNumber, RESET_2_VALUE);

	/*
	 * Start the timer counter such that it's incrementing by default,
	 * then wait for it to timeout a number of times
	 */
	//XTmrCtr_Start(TmrCtrInstancePtr_1, TmrCtrNumber_1);

}

void Timers_Init()
{
	Timer0_Init(&TimerCounterInst_0, TIMER_CNTR_0, XPAR_TIMER_0_DEVICE_ID);
    Timer1_Init(&TimerCounterInst_1, TIMER_CNTR_1, XPAR_TIMER_1_DEVICE_ID);
//	Timer2_Init(&TimerCounterInst_2, TIMER_CNTR_2, XPAR_TIMER_2_DEVICE_ID);
}
