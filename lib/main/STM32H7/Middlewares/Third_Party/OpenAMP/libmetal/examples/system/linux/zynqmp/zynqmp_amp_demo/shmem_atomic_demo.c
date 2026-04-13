/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 /*****************************************************************************
 * atomic_shmem_demod.c - Shared memory atomic operation demo
 * This demo will:
 *
 *  1. Open the shared memory device.
 *  2. Open the IPI device.
 *  3. Register IPI interrupt handler.
 *  4. Kick IPI to notify the other end to start the demo
 *  5. Start atomic add by 1 for 5000 times over the shared memory
 *  6. Wait for remote IPI kick to know when the remote has finished the demo.
 *  7. Verify the result. As two sides both have done 5000 times of adding 1,
 *     check if the end result is 5000*2.
 *  8. Clean up: deregister the IPI interrupt handler, close the IPI device
 *     , close the shared memory device.
 */

#include <unistd.h>
#include <stdio.h>
#include <metal/atomic.h>
#include <metal/cpu.h>
#include <metal/io.h>
#include <sys/types.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/errno.h>
#include <time.h>
#include "common.h"

#define ATOMIC_INT_OFFSET 0x0 /* shared memory offset for atomic operation */
#define ITERATIONS 5000

static atomic_int remote_nkicked; /* is remote kicked, 0 - kicked,
				       1 - not-kicked */

static int ipi_irq_handler (int vect_id, void *priv)
{
	(void)vect_id;
	struct metal_io_region *ipi_io = (struct metal_io_region *)priv;
	uint32_t ipi_mask = IPI_MASK;
	uint64_t val = 1;

	if (!ipi_io)
		return METAL_IRQ_NOT_HANDLED;
	val = metal_io_read32(ipi_io, IPI_ISR_OFFSET);
	if (val & ipi_mask) {
		metal_io_write32(ipi_io, IPI_ISR_OFFSET, ipi_mask);
		atomic_flag_clear(&remote_nkicked);
		return METAL_IRQ_HANDLED;
	}
	return METAL_IRQ_NOT_HANDLED;
}

/**
 * @brief   atomic_add_shmem() - Shared memory atomic operation demo
 *          This task will:
 *          * Write to shared memory to notify the remote to start atomic add on
 *            the shared memory for 1000 times.
 *          * Start atomic add by 1 for 5000 times to first 32 bits of memory in
 *            the shared memory which is pointed to by shm_io.
 *          * Wait for the remote to write to shared memory
 *          * Once it received the polling kick from the remote, it will check
 *            if the value stored in the shared memory is the same as the
 *            expected.
 *          * It will print if the atomic add test has passed or not.
 * @param[in] ipi_io - IPI metal i/o region
 * @param[in] shm_io - shared memory metal i/o region
 * @return - If setup failed, return the corresponding error number. Otherwise
 *          return 0 on success.
 */
static int atomic_add_shmem(struct metal_io_region *ipi_io,
		struct metal_io_region *shm_io)
{
	int i, ret;
	atomic_int *shm_int;
	uint32_t ipi_mask = IPI_MASK;

	LPRINTF("Starting atomic shared memory task.\n");

	/* Initialize the shared memory on which we run the atomic add */
	shm_int = (atomic_int *)metal_io_virt(shm_io,
					ATOMIC_INT_OFFSET);
	atomic_store(shm_int, 0);

	/* Kick the remote to notify demo starts. */
	metal_io_write32(ipi_io, IPI_TRIG_OFFSET, ipi_mask);

	/* Do atomic add over the shared memory */
	for (i = 0; i < ITERATIONS; i++) {
		atomic_fetch_add(shm_int, 1);
	}

	/* Wait for kick from RPU to know when RPU finishes the demo */
	wait_for_notified(&remote_nkicked);

	if (atomic_load(shm_int) == (ITERATIONS << 1 )) {
		LPRINTF("shm atomic demo PASSED!\n");
		ret = 0;
	} else {
		LPRINTF("shm atomic demo FAILED. expected: %u, actual: %u\n",
			(unsigned int)(ITERATIONS << 1), atomic_load(shm_int));
		ret = -1;
	}

	return ret;
}

int atomic_shmem_demo()
{
	struct metal_device *ipi_dev = NULL, *shm_dev = NULL;
	struct metal_io_region *ipi_io = NULL, *shm_io = NULL;
	int ipi_irq;
	int ret = 0;

	print_demo("atomic operation over shared memory");

	/* Open shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &shm_dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", SHM_DEV_NAME);
		goto out;
	}

	/* Get shared memory device IO region */
	shm_io = metal_device_io_region(shm_dev, 0);
	if (!shm_io) {
		LPERROR("Failed to map io region for %s.\n", shm_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &ipi_dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Get IPI device IO region */
	ipi_io = metal_device_io_region(ipi_dev, 0);
	if (!ipi_io) {
		LPERROR("Failed to map io region for %s.\n", ipi_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get the IPI IRQ from the opened IPI device */
	ipi_irq = (intptr_t)ipi_dev->irq_info;

	/* disable IPI interrupt */
	metal_io_write32(ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* clear old IPI interrupt */
	metal_io_write32(ipi_io, IPI_ISR_OFFSET, IPI_MASK);
	/* Register IPI irq handler */
	metal_irq_register(ipi_irq, ipi_irq_handler, ipi_dev, ipi_io);
	/* initialize remote_nkicked */
	atomic_init(&remote_nkicked, 1);
	/* Enable IPI interrupt */
	metal_io_write32(ipi_io, IPI_IER_OFFSET, IPI_MASK);

	/* Run atomic operation demo */
	ret = atomic_add_shmem(ipi_io, shm_io);

	/* disable IPI interrupt */
	metal_io_write32(ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* unregister IPI irq handler by setting the handler to 0 */
	metal_irq_unregister(ipi_irq, 0, ipi_dev, ipi_io);

out:
	if (shm_dev)
		metal_device_close(shm_dev);
	if (ipi_dev)
		metal_device_close(ipi_dev);
	return ret;

}
