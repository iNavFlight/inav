/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_cache.h"
#include <metal/sys.h>
#include <metal/irq.h>
#include "platform_info.h"


#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID

static XScuGic xInterruptController;

/* Interrupt Controller setup */
static int app_gic_initialize(void)
{
	u32 Status;
	XScuGic_Config *IntcConfig;	/* The configuration parameters of the interrupt controller */

	Xil_ExceptionDisable();

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&xInterruptController, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			&xInterruptController);

	Xil_ExceptionEnable();

	/* Connect notificaiton interrupt ID with ISR */
	XScuGic_Connect(&xInterruptController, SGI_NOTIFICATION,
			(Xil_ExceptionHandler)metal_irq_isr,
			(void *)SGI_NOTIFICATION);

	return 0;
}

/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	struct metal_init_params metal_param = METAL_INIT_DEFAULTS;

	/* Low level abstraction layer for openamp initialization */
	metal_init(&metal_param);

	/* configure the global interrupt controller */
	app_gic_initialize();

	return 0;
}

void cleanup_system()
{
	metal_finish();

	Xil_DCacheDisable();
	Xil_ICacheDisable();
	Xil_DCacheInvalidate();
	Xil_ICacheInvalidate();
}
