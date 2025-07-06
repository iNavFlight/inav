/******************************************************************************
 *
 * Copyright (C) 2010 - 2017 Xilinx, Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 ******************************************************************************/

#include <xparameters.h>
#include <xil_cache.h>
#include <xil_exception.h>
#include <xstatus.h>
#include <xscugic.h>
#include <xreg_cortexr5.h>

#include <metal/io.h>
#include <metal/device.h>
#include <metal/sys.h>
#include <metal/irq.h>

#include "platform_config.h"
#include "common.h"

#ifdef STDOUT_IS_16550
 #include <xuartns550_l.h>

 #define UART_BAUD 9600
#endif

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID

#define IPI_IRQ_VECT_ID         65

#define SHM_BASE_ADDR   0x3ED80000
#define TTC0_BASE_ADDR  0xFF110000
#define IPI_BASE_ADDR   0xFF310000

/* Default generic I/O region page shift */
/* Each I/O region can contain multiple pages.
 * In FreeRTOS system, the memory mapping is flat, there is no
 * virtual memory.
 * We can assume there is only one page in the FreeRTOS system.
 */
#define DEFAULT_PAGE_SHIFT (-1UL)
#define DEFAULT_PAGE_MASK  (-1UL)

extern XScuGic xInterruptController;

const metal_phys_addr_t metal_phys[] = {
	IPI_BASE_ADDR, /**< base IPI address */
	SHM_BASE_ADDR, /**< shared memory base address */
	TTC0_BASE_ADDR, /**< base TTC0 address */
};

/* Define metal devices table for IPI, shared memory and TTC devices.
 * Linux system uses device tree to describe devices. Unlike Linux,
 * there is no standard device abstraction for FreeRTOS system, we
 * uses libmetal devices structure to describe the devices we used in
 * the example.
 * The IPI, shared memory and TTC devices are memory mapped
 * devices. For this type of devices, it is required to provide
 * accessible memory mapped regions, and interrupt information.
 * In FreeRTOS system, the memory mapping is flat. As you can see
 * in the table before, we set the virtual address "virt" the same
 * as the physical address.
 */
static struct metal_device metal_dev_table[] = {
	{
		/* IPI device */
		.name = IPI_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = {
			{
				.virt = (void *)IPI_BASE_ADDR,
				.physmap = &metal_phys[0],
				.size = 0x1000,
				.page_shift = DEFAULT_PAGE_SHIFT,
				.page_mask = DEFAULT_PAGE_MASK,
				.mem_flags = DEVICE_NONSHARED | PRIV_RW_USER_RW,
				.ops = {NULL},
			}
		},
		.node = {NULL},
		.irq_num = 1,
		.irq_info = (void *)IPI_IRQ_VECT_ID,
	},
	{
		/* Shared memory management device */
		.name = SHM_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = {
			{
				.virt = (void *)SHM_BASE_ADDR,
				.physmap = &metal_phys[1],
				.size = 0x1000000,
				.page_shift = DEFAULT_PAGE_SHIFT,
				.page_mask = DEFAULT_PAGE_MASK,
				.mem_flags = NORM_SHARED_NCACHE |
						PRIV_RW_USER_RW,
				.ops = {NULL},
			}
		},
		.node = {NULL},
		.irq_num = 0,
		.irq_info = NULL,
	},
	{
		/* ttc0 */
		.name = TTC_DEV_NAME,
		.bus = NULL,
		.num_regions = 1,
		.regions = {
			{
				.virt = (void *)TTC0_BASE_ADDR ,
				.physmap = &metal_phys[2],
				.size = 0x1000,
				.page_shift = DEFAULT_PAGE_SHIFT,
				.page_mask = DEFAULT_PAGE_MASK,
				.mem_flags = DEVICE_NONSHARED | PRIV_RW_USER_RW,
				.ops = {NULL},
			}
		},
		.node = {NULL},
		.irq_num = 0,
		.irq_info = NULL,
	},
};

/**
 * Extern global variables
 */
struct metal_device *ipi_dev = NULL;
struct metal_device *shm_dev = NULL;
struct metal_device *ttc_dev = NULL;

/**
 * @brief enable_caches() - Enable caches
 */
void enable_caches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheEnable();
#endif
#endif
}

/**
 * @brief disable_caches() - Disable caches
 */
void disable_caches()
{
	Xil_DCacheDisable();
	Xil_ICacheDisable();
}

/**
 * @brief init_uart() - Initialize UARTs
 */
void init_uart()
{
#ifdef STDOUT_IS_16550
	XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ,
			   UART_BAUD);
	XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif
	/* Bootrom/BSP configures PS7/PSU UART to 115200 bps */
}

/**
 * @brief init_irq() - Initialize GIC and connect IPI interrupt
 *        This function will initialize the GIC and connect the IPI
 *        interrupt.
 *
 * @return 0 - succeeded, non-0 for failures
 */
int init_irq()
{
	int ret = 0;
	XScuGic_Config *IntcConfig;	/* The configuration parameters of
					 * the interrupt controller */

	Xil_ExceptionDisable();
	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return (int)XST_FAILURE;
	}

	ret = XScuGic_CfgInitialize(&xInterruptController, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (ret != XST_SUCCESS) {
		return (int)XST_FAILURE;
	}

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			&xInterruptController);

	Xil_ExceptionEnable();
	/* Connect IPI Interrupt ID with libmetal ISR */
	XScuGic_Connect(&xInterruptController, IPI_IRQ_VECT_ID,
			   (Xil_ExceptionHandler)metal_irq_isr,
			   (void *)IPI_IRQ_VECT_ID);

	XScuGic_Enable(&xInterruptController, IPI_IRQ_VECT_ID);

	return 0;
}

/**
 * @brief platform_register_metal_device() - Statically Register libmetal
 *        devices.
 *        This function registers the IPI, shared memory and
 *        TTC devices to the libmetal generic bus.
 *        Libmetal uses bus structure to group the devices. Before you can
 *        access the device with libmetal device operation, you will need to
 *        register the device to a libmetal supported bus.
 *        For non-Linux system, libmetal only supports "generic" bus, which is
 *        used to manage the memory mapped devices.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
int platform_register_metal_device(void)
{
	unsigned int i;
	int ret;
	struct metal_device *dev;

	for (i = 0; i < sizeof(metal_dev_table)/sizeof(struct metal_device);
	     i++) {
		dev = &metal_dev_table[i];
		xil_printf("registering: %d, name=%s\n", i, dev->name);
		ret = metal_register_generic_device(dev);
		if (ret)
			return ret;
	}
	return 0;
}

/**
 * @brief open_metal_devices() - Open registered libmetal devices.
 *        This function opens all the registered libmetal devices.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
int open_metal_devices(void)
{
	int ret;

	/* Open shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &shm_dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", SHM_DEV_NAME);
		goto out;
	}

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &ttc_dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", TTC_DEV_NAME);
		goto out;
	}

out:
	return ret;
}

/**
 * @brief close_metal_devices() - close libmetal devices
 *        This function closes all the libmetal devices which have
 *        been opened.
 *
 */
void close_metal_devices(void)
{
	/* Close shared memory device */
	if (shm_dev)
		metal_device_close(shm_dev);

	/* Close IPI device */
	if (ipi_dev)
		metal_device_close(ipi_dev);

	/* Close TTC device */
	if (ttc_dev)
		metal_device_close(ttc_dev);
}

/**
 * @brief sys_init() - Register libmetal devices.
 *        This function register the libmetal generic bus, and then
 *        register the IPI, shared memory descriptor and shared memory
 *        devices to the libmetal generic bus.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
int sys_init()
{
	struct metal_init_params metal_param = METAL_INIT_DEFAULTS;
	int ret;

	enable_caches();
	init_uart();
	if (init_irq()) {
		LPERROR("Failed to initialize interrupt\n");
	}

	/* Initialize libmetal environment */
	metal_init(&metal_param);
	/* Register libmetal devices */
	ret = platform_register_metal_device();
	if (ret) {
		LPERROR("%s: failed to register devices: %d\n", __func__, ret);
		return ret;
	}

	/* Open libmetal devices which have been registered */
	ret = open_metal_devices();
	if (ret) {
		LPERROR("%s: failed to open devices: %d\n", __func__, ret);
		return ret;
	}
	return 0;
}

/**
 * @brief sys_cleanup() - system cleanup
 *        This function finish the libmetal environment
 *        and disable caches.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
void sys_cleanup()
{
	/* Close libmetal devices which have been opened */
	close_metal_devices();
	/* Finish libmetal environment */
	metal_finish();
	disable_caches();
}

