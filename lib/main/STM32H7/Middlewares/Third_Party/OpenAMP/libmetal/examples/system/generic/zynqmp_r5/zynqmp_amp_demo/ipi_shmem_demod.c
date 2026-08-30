/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*****************************************************************************
 * ipi_shmem_demo.c - shared memory with IPI demo
 * This demo will:
 *
 *  1. Get the shared memory device I/O region.
 *  2. Get the IPI device I/O region.
 *  3. Register IPI interrupt handler.
 *  4. Wait for remote IPI notification to receive message.
 *  5. When message is received, check if it is shutdown message.
 *  6. If it is shutdown message, do cleanup, otherwise, echo it back to the
 *     shared buffer.
 *  7. Kick IPI to notify there is a message written to the shared memory
 *     if it echos back the message.
 *  8. Repeat 4.
 *  9. Clean up: disable IPI interrupt, deregister the IPI interrupt handler.
 *
 * Here is the Shared memory structure of this demo:
 * |0x0   - 0x03        | number of APU to RPU buffers available to RPU |
 * |0x04  - 0x07        | number of APU to RPU buffers consumed by RPU |
 * |0x08  - 0x1FFC      | address array for shared buffers from APU to RPU |
 * |0x2000 - 0x2003     | number of RPU to APU buffers available to APU |
 * |0x2004 - 0x2007     | number of RPU to APU buffers consumed by APU |
 * |0x2008 - 0x3FFC     | address array for shared buffers from RPU to APU |
 * |0x04000 - 0x103FFC  | APU to RPU buffers |
 * |0x104000 - 0x203FFC | RPU to APU buffers |
 */

#include <sys/types.h>
#include <metal/sys.h>
#include <metal/io.h>
#include <metal/alloc.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/errno.h>
#include "common.h"

#define BUF_SIZE_MAX 512
#define SHUTDOWN "shutdown"

/* Shared memory offsets */
#define SHM_DESC_OFFSET_RX 0x0
#define SHM_BUFF_OFFSET_RX 0x04000
#define SHM_DESC_OFFSET_TX 0x02000
#define SHM_BUFF_OFFSET_TX 0x104000

/* Shared memory descriptors offset */
#define SHM_DESC_AVAIL_OFFSET 0x00
#define SHM_DESC_USED_OFFSET  0x04
#define SHM_DESC_ADDR_ARRAY_OFFSET 0x08

#define PKGS_TOTAL 1024

#define BUF_SIZE_MAX 512
#define SHUTDOWN "shutdown"

struct msg_hdr_s {
	uint32_t index;
	uint32_t len;
};

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
 * @brief   ipi_shmem_echod() - shared memory IPI demo
 *          This task will:
 *          * Wait for IPI interrupt from the remote
 *          * Once it received the interrupt, copy the content from
 *            the ping buffer to the pong buffer.
 *          * Update the shared memory descriptor for the new available
 *            pong buffer.
 *          * Trigger IPI to notifty the remote.
 * @param[in] ipi_io - IPI metal i/o region
 * @param[in] shm_io - shared memory metal i/o region
 * @return - return 0 on success, otherwise return error number indicating
 *		 type of error.
 */
static int ipi_shmem_echod(struct metal_io_region *ipi_io,
		struct metal_io_region *shm_io)
{
	int ret = 0;
	uint32_t rx_count, rx_avail;
	unsigned long tx_avail_offset, rx_avail_offset;
	unsigned long rx_used_offset;
	unsigned long tx_addr_offset, rx_addr_offset;
	unsigned long tx_data_offset, rx_data_offset;
	void *lbuf = NULL;
	struct msg_hdr_s *msg_hdr;
	uint32_t ipi_mask = IPI_MASK;

	lbuf = metal_allocate_memory(BUF_SIZE_MAX);
	if (!lbuf) {
		LPERROR("Failed to allocate local buffer for msg.\n");
		ret = -ENOMEM;
		goto out;
	}
	/* Clear shared memory */
	metal_io_block_set(shm_io, 0, 0, metal_io_region_size(shm_io));

	/* Set tx/rx buffer address offset */
	tx_avail_offset = SHM_DESC_OFFSET_TX + SHM_DESC_AVAIL_OFFSET;
	rx_avail_offset = SHM_DESC_OFFSET_RX + SHM_DESC_AVAIL_OFFSET;
	rx_used_offset = SHM_DESC_OFFSET_RX + SHM_DESC_USED_OFFSET;
	tx_addr_offset = SHM_DESC_OFFSET_TX + SHM_DESC_ADDR_ARRAY_OFFSET;
	rx_addr_offset = SHM_DESC_OFFSET_RX + SHM_DESC_ADDR_ARRAY_OFFSET;
	tx_data_offset = SHM_DESC_OFFSET_TX + SHM_BUFF_OFFSET_TX;
	rx_data_offset = SHM_DESC_OFFSET_RX + SHM_BUFF_OFFSET_RX;

	LPRINTF("Wait for echo test to start.\n");
	rx_count = 0;
	while (1) {
		wait_for_notified(&remote_nkicked);
		rx_avail = metal_io_read32(shm_io, rx_avail_offset);
		while(rx_count != rx_avail) {
			uint32_t buf_phy_addr_32;

			/* Received ping from the other side */

			/* Get the buffer location from the shared memory
			 * rx address array.
			 */
			buf_phy_addr_32 = metal_io_read32(shm_io,
					rx_addr_offset);
			rx_data_offset = metal_io_phys_to_offset(shm_io,
					(metal_phys_addr_t)buf_phy_addr_32);
			if (rx_data_offset == METAL_BAD_OFFSET) {
				LPERROR("[%u]failed to get rx offset: 0x%x, 0x%lx.\n",
					rx_count, buf_phy_addr_32,
					metal_io_phys(shm_io, rx_addr_offset));
				ret = -EINVAL;
				goto out;
			}
			rx_addr_offset += sizeof(buf_phy_addr_32);

			/* Read message header from shared memory */
			metal_io_block_read(shm_io, rx_data_offset, lbuf,
				sizeof(struct msg_hdr_s));
			msg_hdr = (struct msg_hdr_s *)lbuf;

			/* Check if the message header is valid */
			if (msg_hdr->len > (BUF_SIZE_MAX - sizeof(*msg_hdr))) {
				LPERROR("wrong msg: length invalid: %u, %u.\n",
					BUF_SIZE_MAX - sizeof(*msg_hdr),
					msg_hdr->len);
				ret = -EINVAL;
				goto out;
			}
			rx_data_offset += sizeof(*msg_hdr);
			/* Read message */
			metal_io_block_read(shm_io,
					rx_data_offset,
					lbuf + sizeof(*msg_hdr), msg_hdr->len);
			rx_data_offset += msg_hdr->len;
			rx_count++;
			/* increase rx used count to indicate it has consumed
			 * the received data */
			metal_io_write32(shm_io, rx_used_offset, rx_count);

			/* Check if the it is the shutdown message */
			if (msg_hdr->len == strlen(SHUTDOWN) &&
				!strncmp(SHUTDOWN,
					(lbuf + sizeof(struct msg_hdr_s)),
					strlen(SHUTDOWN))) {
					LPRINTF("Received shutdown message\n");
					goto out;
			}
			/* Copy the message back to the other end */
			metal_io_block_write(shm_io, tx_data_offset, msg_hdr,
				sizeof(struct msg_hdr_s) + msg_hdr->len);

			/* Write to the address array to tell the other end
			 * the buffer address.
			 */
			buf_phy_addr_32 = (uint32_t)metal_io_phys(shm_io,
						tx_data_offset);
			metal_io_write32(shm_io, tx_addr_offset,
					buf_phy_addr_32);
			tx_data_offset += sizeof(struct msg_hdr_s) +
				msg_hdr->len;
			tx_addr_offset += sizeof(uint32_t);

			/* Increase number of available buffers */
			metal_io_write32(shm_io, tx_avail_offset, rx_count);
			/* Kick IPI to notify data is in shared buffer */
			metal_io_write32(ipi_io, IPI_TRIG_OFFSET,
					ipi_mask);
		}

	}

out:
	LPRINTF("IPI with shared memory demo finished with exit code: %i.\n",
		ret);

	if (lbuf)
		metal_free_memory(lbuf);
	return ret;
}

int ipi_shmem_demod()
{
	struct metal_io_region *ipi_io = NULL, *shm_io = NULL;
	int ipi_irq;
	int ret = 0;

	print_demo("IPI and shared memory");

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
	ret = ipi_shmem_echod(ipi_io, shm_io);

	/* disable IPI interrupt */
	metal_io_write32(ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* unregister IPI irq handler */
	metal_irq_unregister(ipi_irq, 0, ipi_dev, ipi_io);

out:
	return ret;

}

