/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 /***************************************************************************
  * libmetal_amp_demo.c
  *
  * This application shows how to use IPI to trigger interrupt and how to
  * setup shared memory with libmetal API for communication between processors.
  *
  * This app does the following:
  * 1.  Initialize the platform hardware such as UART, GIC.
  * 2.  Connect the IPI interrupt.
  * 3.  Register IPI device, shared memory descriptor device and shared memory
  *     device with libmetal in the intialization.
  * 4.  In the main application it does the following,
  *     * open the registered libmetal devices: IPI device, shared memory
  *       descriptor device and shared memory device.
  *     * Map the shared memory descriptor as non-cached memory.
  *     * Map the shared memory as non-cached memory. If you do not map the
  *       shared memory as non-cached memory, make sure you flush the cache,
  *       before you notify the remote.
  * 7.  Register the IPI interrupt handler with libmetal.
  * 8.  Run the atomic demo task ipi_task_shm_atomicd():
  *     * Wait for the IPI interrupt from the remote.
  *     * Once it receives the interrupt, it does atomic add by 1 to the
  *       first 32bit of the shared memory descriptor location by 1000 times.
  *     * It will then notify the remote after the calucation.
  *     * As the remote side also does 1000 times add after it has notified
  *       this end. The remote side will check if the result is 2000, if not,
  *       it will error.
  * 9.  Run the shared memory echo demo task ipi_task_echod()
  *     * Wait for the IPI interrupt from the other end.
  *     * If an IPI interrupt is received, copy the message to the current
  *       available RPU to APU buffer, increase the available buffer indicator,
  *       and trigger IPI to notify the remote.
  *     * If "shutdown" message is received, cleanup the libmetal source.
  */

#include <unistd.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include "common.h"

/**
 * @brief    main function of the demo application.
 *           Here are the steps for the main function:
 *           * Setup libmetal resources
 *           * Run the IPI with shared memory demo.
 *           * Run the shared memory demo.
 *           * Run the atomic across shared memory demo.
 *           * Run the ipi latency demo.
 *           * Run the shared memory latency demo.
 *           * Run the shared memory throughput demo.
 *           * Cleanup libmetal resources
 *           Report if any of the above demos failed.
 * @return   0 - succeeded, non-zero for failures.
 */
int main(void)
{
	int ret;

	ret = sys_init();

	if (ret) {
		LPERROR("Failed to initialize system.\n");
		return ret;
	}

	ret = shmem_demod();
	if (ret){
		LPERROR("shared memory demo failed.\n");
		return ret;
	}

	ret = atomic_shmem_demod();
	if (ret){
		LPERROR("shared memory atomic demo failed.\n");
		return ret;
	}

	ret = ipi_shmem_demod();
	if (ret){
		LPERROR("shared memory atomic demo failed.\n");
		return ret;
	}

	ret = ipi_latency_demod();
	if (ret){
		LPERROR("IPI latency demo failed.\n");
		return ret;
	}

	ret = shmem_latency_demod();
	if (ret){
		LPERROR("shared memory latency demo failed.\n");
		return ret;
	}

	ret = shmem_throughput_demod();
	if (ret){
		LPERROR("shared memory thoughput demo failed.\n");
		return ret;
	}

	sys_cleanup();
	return ret;
}

