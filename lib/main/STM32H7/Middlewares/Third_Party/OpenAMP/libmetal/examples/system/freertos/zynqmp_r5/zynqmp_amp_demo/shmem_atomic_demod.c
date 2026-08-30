/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 /*****************************************************************************
 * atomic_shmem_demod.c - Shared memory atomic operation demo
 * This task will:
 *  1. Get the shared memory device I/O region.
 *  2. Get the IPI device I/O region.
 *  3. Register IPI interrupt handler.
 *  4. Wait for the APU to kick IPI to start the demo
 *  5. Once notification is received, start atomic add by
 *     1 for 5000 times over the shared memory
 *  6. Trigger IPI to notify the remote it has finished calculation.
 *  7. Clean up: Disable IPI interrupt, deregister the IPI interrupt handler.
 */
#include <metal/shmem.h>
#include <metal/atomic.h>
#include <metal/device.h>
#include <metal/io.h>
#include <sys/time.h>
#include <stdio.h>
#include "common.h"
#include "sys_init.h"

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
 * @brief   atomic_add_shmemd() - Shared memory atomic operation demo
 *          This task will:
 *          * Wait for the remote to write to shared memory.
 *          * Once it receives the notification via polling, start atomic add by
 *            1 for 5000 times to first 32 bits of memory in the shared memory
 *            which is pointed to by shm_io.
 *          * Write to shared mem to notify the remote once it finishes
 *            calculation.
 *
 * @param[in] ipi_io - IPI metal i/o region
 * @param[in] shm_io - shared memory metal i/o region
 * @return - If setup failed, return the corresponding error number. Otherwise
 *          return 0 on success.
 */
int atomic_add_shmemd(struct metal_io_region *ipi_io,
		struct metal_io_region *shm_io)
{
	atomic_int *shm_int;
	uint32_t ipi_mask = IPI_MASK;
	int i;

	LPRINTF("Starting atomic add on shared memory demo.\n");
	shm_int = (atomic_int *)metal_io_virt(shm_io,
					ATOMIC_INT_OFFSET);

	/* Wait for notification from the remote to start the demo */
	wait_for_notified(&remote_nkicked);

	/* Do atomic add over the shared memory */
	for (i = 0; i < ITERATIONS; i++)
		atomic_fetch_add(shm_int, 1);

	/* Write to IPI trigger register to notify the remote it has finished
	 * the atomic operation. */
	metal_io_write32(ipi_io, IPI_TRIG_OFFSET, ipi_mask);

	LPRINTF("Shared memory with atomics test finished\n");
	return 0;
}

int atomic_shmem_demod()
{
	struct metal_io_region *ipi_io = NULL, *shm_io = NULL;
	int ipi_irq;
	int ret = 0;

	print_demo("atomic operation over shared memory");

	/* Get shared memory device IO region */
	if (!shm_dev) {
		ret = -ENODEV;
		goto out;
	}
	shm_io = metal_device_io_region(shm_dev, 0);
	if (!shm_io) {
		LPERROR("Failed to map io region for %s.\n", shm_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get IPI device IO region */
	if (!ipi_dev) {
		ret = -ENODEV;
		goto out;
	}
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
	ret = atomic_add_shmemd(ipi_io, shm_io);

	/* disable IPI interrupt */
	metal_io_write32(ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* unregister IPI irq handler */
	metal_irq_unregister(ipi_irq, 0, ipi_dev, ipi_io);

out:
	return ret;

}

