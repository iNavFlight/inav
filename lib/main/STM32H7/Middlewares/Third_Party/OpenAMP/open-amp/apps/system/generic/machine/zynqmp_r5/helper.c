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
	uint32_t status;
	XScuGic_Config *int_ctrl_config; /* interrupt controller configuration params */
	uint32_t int_id;
	uint32_t mask_cpu_id = ((u32)0x1 << XPAR_CPU_ID);
	uint32_t target_cpu;

	mask_cpu_id |= mask_cpu_id << 8U;
	mask_cpu_id |= mask_cpu_id << 16U;

	Xil_ExceptionDisable();

	/*
	 * Initialize the interrupt controller driver
	 */
	int_ctrl_config = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == int_ctrl_config) {
		return XST_FAILURE;
	}

	status = XScuGic_CfgInitialize(&xInterruptController, int_ctrl_config,
				       int_ctrl_config->CpuBaseAddress);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Only associate interrupt needed to this CPU */
	for (int_id = 32U; int_id<XSCUGIC_MAX_NUM_INTR_INPUTS;int_id=int_id+4U) {
		target_cpu = XScuGic_DistReadReg(&xInterruptController,
						XSCUGIC_SPI_TARGET_OFFSET_CALC(int_id));
		/* Remove current CPU from interrupt target register */
		target_cpu &= ~mask_cpu_id;
		XScuGic_DistWriteReg(&xInterruptController,
					XSCUGIC_SPI_TARGET_OFFSET_CALC(int_id), target_cpu);
	}
	XScuGic_InterruptMaptoCpu(&xInterruptController, XPAR_CPU_ID, IPI_IRQ_VECT_ID);

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			&xInterruptController);

	Xil_ExceptionEnable();

	/* Connect Interrupt ID with ISR */
	XScuGic_Connect(&xInterruptController, IPI_IRQ_VECT_ID,
			(Xil_ExceptionHandler)metal_irq_isr,
			(void *)IPI_IRQ_VECT_ID);

	return 0;
}

static void system_metal_logger(enum metal_log_level level,
			   const char *format, ...)
{
	(void)level;
	(void)format;
}


/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	struct metal_init_params metal_param = {
		.log_handler = system_metal_logger,
		.log_level = METAL_LOG_INFO,
	};

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
