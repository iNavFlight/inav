/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <unistd.h>

#include <metal/sys.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/atomic.h>
#include <metal/cpu.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "sys_init.h"

#define IPI_TRIG_OFFSET 0x0
#define IPI_OBS_OFFSET  0x4
#define IPI_ISR_OFFSET  0x10
#define IPI_IMR_OFFSET  0x14
#define IPI_IER_OFFSET  0x18
#define IPI_IDR_OFFSET  0x1C

#define IPI_MASK        0x1000000

#define IPI_DEV_NAME    "ff310000.ipi"
#define SHM0_DESC_DEV_NAME    "3ed00000.shm_desc"
#define SHM1_DESC_DEV_NAME    "3ed10000.shm_desc"
#define SHM_DEV_NAME    "3ed20000.shm"
#define BUS_NAME        "platform"
#define D0_SHM_OFFSET   0x00000
#define D1_SHM_OFFSET   0x20000

#define BUF_SIZE_MAX 512
#define SHUTDOWN "shutdown"

#define LPRINTF(format, ...) \
	printf("SERVER> " format, ##__VA_ARGS__)

struct shm_mg_s {
	uint32_t avails;
	uint32_t used;
};

typedef uint64_t shm_addr_t;

struct msg_hdr_s {
	uint32_t index;
	int32_t len;
};

struct channel_s {
	struct metal_device *ipi_dev;
	struct metal_io_region *ipi_io;
	unsigned int ipi_mask;
	struct metal_device *shm0_desc_dev;
	struct metal_io_region *shm0_desc_io;
	struct metal_device *shm1_desc_dev;
	struct metal_io_region *shm1_desc_io;
	struct metal_device *shm_dev;
	struct metal_io_region *shm_io;
	atomic_int notified;
	unsigned long d0_start_offset;
	unsigned long d1_start_offset;
};

static struct channel_s ch0;

extern int system_init();
extern int run_comm_task(void *task, void *arg);
extern void wait_for_interrupt(void);

static int ipi_irq_isr (int vect_id, void *priv)
{
	(void)vect_id;
	struct channel_s *ch = (struct channel_s *)priv;
	uint64_t val = 1;

	if (!ch)
		return METAL_IRQ_NOT_HANDLED;
	val = metal_io_read32(ch->ipi_io, IPI_ISR_OFFSET);
	if (val & ch->ipi_mask) {
		metal_io_write32(ch->ipi_io, IPI_ISR_OFFSET, ch->ipi_mask);
		atomic_flag_clear(&ch->notified);
		return METAL_IRQ_HANDLED;
	}
	return METAL_IRQ_NOT_HANDLED;
}

static int ipi_task_shm_atomicd(void *arg)
{
	struct channel_s *ch = (struct channel_s *)arg;
	atomic_int *shm_int;
	unsigned int flags;
	int i;

	shm_int = (atomic_int *)metal_io_virt(ch->shm0_desc_io, 0);

	LPRINTF("Wait for atomic test to start.\n");
	while (1) {
		do {
			flags = metal_irq_save_disable();
			if (!atomic_flag_test_and_set(&ch->notified)) {
				metal_irq_restore_enable(flags);
				break;
			}
			wait_for_interrupt();
			metal_irq_restore_enable(flags);
		} while(1);
		for (i = 0; i < 1000; i++)
			atomic_fetch_add(shm_int, 1);
			//*((unsigned int volatile *)shm_int) += 1;
		/* memory barrier */
		atomic_thread_fence(memory_order_acq_rel);

		/* Send the message */
		LPRINTF("SENDING message...\n");
		metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
		break;
	}

	return 0;
}

static int ipi_task_echod(void *arg)
{
	struct channel_s *ch = (struct channel_s *)arg;
	struct shm_mg_s *shm0_mg, *shm1_mg;
	shm_addr_t *shm0_addr_array, *shm1_addr_array;
	struct msg_hdr_s *msg_hdr;
	unsigned int flags;
	void *d0, *d1, *lbuf;
	metal_phys_addr_t d0_pa;
	int len;

	shm0_mg = (struct shm_mg_s *)metal_io_virt(ch->shm0_desc_io, 0);
	shm1_mg = (struct shm_mg_s *)metal_io_virt(ch->shm1_desc_io, 0);
	shm0_addr_array = (void *)shm0_mg + sizeof(struct shm_mg_s);
	shm1_addr_array = (void *)shm1_mg + sizeof(struct shm_mg_s);
	d1 = metal_io_virt(ch->shm_io, ch->d1_start_offset);
	lbuf = malloc(BUF_SIZE_MAX);
	if (!lbuf) {
		LPRINTF("ERROR: Failed to allocate local buffer for msg.\n");
		return -1;
	}

	LPRINTF("Wait for echo test to start.\n");
	while (1) {
		do {
			flags = metal_irq_save_disable();
			if (!atomic_flag_test_and_set(&ch->notified)) {
				metal_irq_restore_enable(flags);
				break;
			}
			wait_for_interrupt();
			metal_irq_restore_enable(flags);
		} while(1);
		atomic_thread_fence(memory_order_acq_rel);
		while(shm0_mg->used != shm0_mg->avails) {
			d0_pa = (metal_phys_addr_t)shm0_addr_array[shm0_mg->used];
			d0 = metal_io_phys_to_virt(ch->shm_io, d0_pa);
			if (!d0) {
				LPRINTF("ERROR: failed to get rx addr:0x%lx.\n",
					d0_pa);
				goto out;
			}
			/* Copy msg header from shared buf to local mem */
			len = metal_io_block_read(ch->shm_io,
				metal_io_virt_to_offset(ch->shm_io, d0),
				lbuf, sizeof(struct msg_hdr_s));
			if (len < (int)sizeof(struct msg_hdr_s)) {
				LPRINTF("ERROR: failed to get msg header.\n");
				goto out;
			}
			msg_hdr = lbuf;
			if (msg_hdr->len < 0) {
				LPRINTF("ERROR: wrong msg length: %d.\n",
					(int)msg_hdr->len);
				goto out;
			} else {
				/* copy msg data from shared buf to local mem */
				d0 += sizeof(struct msg_hdr_s);
				len = metal_io_block_read(ch->shm_io,
					metal_io_virt_to_offset(ch->shm_io, d0),
					lbuf + sizeof(struct msg_hdr_s),
					msg_hdr->len);
#if DEBUG
				LPRINTF("received: %d, %d\n",
					(int)msg_hdr->index, (int)msg_hdr->len);
#endif
				/* Check if the it is the shutdown message */
				if (!strncmp((lbuf + sizeof(struct msg_hdr_s)),
					 SHUTDOWN, sizeof(SHUTDOWN))) {
					LPRINTF("Received shutdown message\n");
					goto out;
				}
			}
			/* Copy the message back to the other end */
			metal_io_block_write(ch->shm_io,
				metal_io_virt_to_offset(ch->shm_io, d1),
				lbuf,
				sizeof(struct msg_hdr_s) + msg_hdr->len);

			/* Update the d1 address */
			shm1_addr_array[shm1_mg->avails] =
					(uint64_t)metal_io_virt_to_phys(
						ch->shm_io, d1);
			d1 += (sizeof(struct msg_hdr_s) + msg_hdr->len);
			shm0_mg->used++;
			shm1_mg->avails++;
			/* memory barrier */
			atomic_thread_fence(memory_order_acq_rel);

			/* Send the message */
			metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET,
					ch->ipi_mask);
		}
	}

out:
	free(lbuf);
	return 0;
}

int main(void)
{
	struct metal_device *device;
	struct metal_io_region *io;
	int irq;
	uint32_t val;
	int ret = 0;

	ret = sys_init();
	if (ret) {
		LPRINTF("ERROR: Failed to initialize system\n");
		return -1;
	}
	memset(&ch0, 0, sizeof(ch0));

	atomic_store(&ch0.notified, 1);

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Map IPI device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		LPRINTF("ERROR: Failed to map io regio for %s.\n",
			  device->name);
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}

	/* Store the IPI device and I/O region */
	ch0.ipi_dev = device;
	ch0.ipi_io = io;

	/* Open shared memory0 descriptor device */
	ret = metal_device_open(BUS_NAME, SHM0_DESC_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n",
			SHM0_DESC_DEV_NAME);
		goto out;
	}

	/* Map shared memory0 descriptor device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		LPRINTF("ERROR: Failed to map io regio for %s.\n",
			  device->name);
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}
	/* Store the shared memory0 descriptor device and I/O region */
	ch0.shm0_desc_dev = device;
	ch0.shm0_desc_io = io;

	/* Open shared memory1 descriptor device */
	ret = metal_device_open(BUS_NAME, SHM1_DESC_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n",
			SHM1_DESC_DEV_NAME);
		goto out;
	}

	/* Map shared memory1 descriptor device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		LPRINTF("ERROR: Failed to map io regio for %s.\n",
			  device->name);
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}
	/* Store the shared memory0 descriptor device and I/O region */
	ch0.shm1_desc_dev = device;
	ch0.shm1_desc_io = io;

	/* Open shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n", SHM_DEV_NAME);
		goto out;
	}

	/* Map shared memory device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		LPRINTF("ERROR: Failed to map io regio for %s.\n",
			  device->name);
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}

	/* Store the shared memory device and I/O region */
	ch0.shm_dev = device;
	ch0.shm_io = io;
	ch0.d1_start_offset = D1_SHM_OFFSET;

	/* Get interrupt ID from IPI metal device */
	irq = (intptr_t)ch0.ipi_dev->irq_info;
	if (irq < 0) {
		LPRINTF("ERROR: Failed to request interrupt for %s.\n",
			  device->name);
		ret = -EINVAL;
		goto out;
	}

	ch0.ipi_mask = IPI_MASK;

	LPRINTF("Try to register IPI interrupt.\n");
	ret =  metal_irq_register(irq, ipi_irq_isr, ch0.ipi_dev, &ch0);
	LPRINTF("registered IPI interrupt.\n");
	if (ret)
		goto out;

	/* Enable interrupt */
	metal_io_write32(ch0.ipi_io, IPI_IER_OFFSET, ch0.ipi_mask);
	val = metal_io_read32(ch0.ipi_io, IPI_IMR_OFFSET);
	if (val & ch0.ipi_mask) {
		LPRINTF("ERROR: Failed to enable IPI interrupt.\n");
		ret = -1;
		goto out;
	}
	LPRINTF("enabled IPI interrupt.\n");
	ret = ipi_task_shm_atomicd((void *)&ch0);
	if (ret) {
		LPRINTF("ERROR: Failed to run shared memory atomic task.\n");
		goto out;
	}
	ret = ipi_task_echod((void*)&ch0);
	if (ret)
		LPRINTF("ERROR: Failed to run IPI communication task.\n");

out:
	if (ch0.ipi_dev)
		metal_device_close(ch0.ipi_dev);
	if (ch0.shm0_desc_dev)
		metal_device_close(ch0.shm0_desc_dev);
	if (ch0.shm1_desc_dev)
		metal_device_close(ch0.shm1_desc_dev);
	if (ch0.shm_dev)
		metal_device_close(ch0.shm_dev);
	sys_cleanup();

	return ret;
}
