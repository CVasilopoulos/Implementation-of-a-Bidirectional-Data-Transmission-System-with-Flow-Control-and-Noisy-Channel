
#include "Interrupts.h"

/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the UartLite device. This function is application specific since the
* actual system may or may not have an interrupt controller. The UartLite
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param    UartLitePtr contains a pointer to the instance of the UartLite
*           component which is going to be connected to the interrupt
*           controller.
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note     None.
*
****************************************************************************/
int SetupInterruptSystem(XUartLite *UartLitePtr_1, u8 UartLite_1_Id, XUartLite *UartLitePtr_2, u8 UartLite_2_Id,
		XTmrCtr* TmrCtrInstancePtr_0, u8 Timer_0_Id, XTmrCtr* TmrCtrInstancePtr_1, u8 Timer_1_Id)
{
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController, UartLite_1_Id, (XInterruptHandler)XUartLite_InterruptHandler, (void *)UartLitePtr_1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController, UartLite_2_Id, (XInterruptHandler)XUartLite_InterruptHandler, (void *)UartLitePtr_2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device
	 */

	Status = XIntc_Connect(&InterruptController, Timer_1_Id, (XInterruptHandler)XTmrCtr_InterruptHandler, (void *)TmrCtrInstancePtr_1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = XIntc_Connect(&InterruptController, Timer_0_Id, (XInterruptHandler)XTmrCtr_InterruptHandler, (void *)TmrCtrInstancePtr_0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

//	Status = XIntc_Connect(&InterruptController, Timer_2_Id, (XInterruptHandler)XTmrCtr_InterruptHandler, (void *)TmrCtrInstancePtr_2);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}



	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the UartLite can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the Timer device.
	 */
	XIntc_Enable(&InterruptController, Timer_1_Id);
	XIntc_Enable(&InterruptController, Timer_0_Id);
//	XIntc_Enable(&InterruptController, Timer_2_Id);

	/*
	 * Enable the interrupt for the UartLite devices.
	 */
	XIntc_Enable(&InterruptController, UartLite_1_Id);
	XIntc_Enable(&InterruptController, UartLite_2_Id);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XIntc_InterruptHandler, &InterruptController);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
