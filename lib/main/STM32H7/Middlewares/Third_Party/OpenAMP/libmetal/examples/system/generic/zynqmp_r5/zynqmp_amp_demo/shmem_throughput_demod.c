/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*****************************************************************************
 * shmem_throughput_demo_task.c
 * This is the remote side of the shared memory throughput demo.
 * This demo does the following steps:
 *
 *  1. Get the shared memory device libmetal I/O region.
 *  1. Get the TTC timer device libemtal I/O region.
 *  2. Get IPI device libmetal I/O region and the IPI interrupt vector.
 *  3. Register IPI interrupt handler.
 *  6. Download throughput measurement:
 *     Start TTC RPU counter, wait for IPI kick, check if data is available,
 *     if yes, read as much data as possible from shared memory. It will
 *     iterates untill 1000 packages have been received, stop TTC RPU counter
 *     and kick IPI to notify the remote. Repeat for different package size.
 *  7. Upload throughput measurement:
 *     Start TTC RPU counter, write data to shared memory and kick IPI to
 *     notify remote. It will iterate for 1000 times, stop TTC RPU counter.
 *     wait for APU IPI kick to know APU has finished receiving packages.
 *     Kick IPI to notify it TTC RPU conter value is ready to read.
 *     Repeat for different package size.
 *  8. Cleanup resource:
 *     disable IPI interrupt and deregister the IPI interrupt handler.
 *
 * Here is the Shared memory structure of this demo:
 * |0x0   - 0x03         | number of APU to RPU buffers available to RPU |
 * |0x04  - 0x1FFFFF     | address array for shared buffers from APU to RPU |
 * |0x200000 - 0x200004  | number of RPU to APU buffers available to APU |
 * |0x200004 - 0x3FFFFF  | address array for shared buffers from RPU to APU |
 * |0x400000 - 0x7FFFFF  | APU to RPU buffers |
 * |0x800000 - 0xAFFFFF  | RPU to APU buffers |
 */

#include <unistd.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/alloc.h>
#include "common.h"

#define TTC_CNT_APU_TO_RPU 2 /* APU to RPU TTC counter ID */
#define TTC_CNT_RPU_TO_APU 3 /* RPU to APU TTC counter ID */

/* Shared memory offsets */
#define SHM_DESC_OFFSET_RX 0x0
#define SHM_BUFF_OFFSET_RX 0x400000
#define SHM_DESC_OFFSET_TX 0x200000
#define SHM_BUFF_OFFSET_TX 0x800000

/* Shared memory descriptors offset */
#define SHM_DESC_AVAIL_OFFSET 0x00
#define SHM_DESC_ADDR_ARRAY_OFFSET 0x04

#define BUF_SIZE_MAX 4096
#define PKG_SIZE_MAX 1024
#define PKG_SIZE_MIN 16
#define TOTAL_DATA_SIZE (1024 * 4096)

struct channel_s {
	struct metal_io_region *ipi_io; /* IPI metal i/o region */
	struct metal_io_region *shm_io; /* Shared memory metal i/o region */
	struct metal_io_region *ttc_io; /* TTC metal i/o region */
	uint32_t ipi_mask; /* RPU IPI mask */
	atomic_int remote_nkicked; /* 0 - kicked from remote */
};

/**
 * @brief reset_timer() - function to reset TTC counter
 *        Set the RST bit in the Count Control Reg.
 *
 * @param[in] ttc_io - TTC timer i/o region
 * @param[in] cnt_id - counter id
 */
static inline void reset_timer(struct metal_io_region *ttc_io,
			unsigned long cnt_id)
{
	uint32_t val;
	unsigned long offset = XTTCPS_CNT_CNTRL_OFFSET +
				XTTCPS_CNT_OFFSET(cnt_id);

	val = XTTCPS_CNT_CNTRL_RST_MASK;
	metal_io_write32(ttc_io, offset, val);
}

/**
 * @brief stop_timer() - function to stop TTC counter
 *        Set the disable bit in the Count Control Reg.
 *
 * @param[in] ttc_io - TTC timer i/o region
 * @param[in] cnt_id - counter id
 */
static inline void stop_timer(struct metal_io_region *ttc_io,
			unsigned long cnt_id)
{
	uint32_t val;
	unsigned long offset = XTTCPS_CNT_CNTRL_OFFSET +
				XTTCPS_CNT_OFFSET(cnt_id);

	val = XTTCPS_CNT_CNTRL_DIS_MASK;
	metal_io_write32(ttc_io, offset, val);
}

/**
 * @brief ipi_irq_handler() - IPI interrupt handler
 *        It will clear the notified flag to mark it's got an IPI interrupt.
 *        It will stop the RPU->APU timer and will clear the notified
 *        flag to mark it's got an IPI interrupt
 *
 * @param[in] vect_id - IPI interrupt vector ID
 * @param[in/out] priv - communication channel data for this application.
 *
 * @return - If the IPI interrupt is triggered by its remote, it returns
 *           METAL_IRQ_HANDLED. It returns METAL_IRQ_NOT_HANDLED, if it is
 *           not the interrupt it expected.
 *
 */
static int ipi_irq_handler (int vect_id, void *priv)
{
	struct channel_s *ch = (struct channel_s *)priv;
	uint32_t val;

	(void)vect_id;

	if (ch) {
		val = metal_io_read32(ch->ipi_io, IPI_ISR_OFFSET);
		if (val & ch->ipi_mask) {
			metal_io_write32(ch->ipi_io, IPI_ISR_OFFSET,
					ch->ipi_mask);
			atomic_flag_clear(&ch->remote_nkicked);
			return METAL_IRQ_HANDLED;
		}
	}
	return METAL_IRQ_NOT_HANDLED;
}

/**
 * @brief measure_shmem_throughputd() - measure shmem throughpput with libmetal
 *        - Download throughput measurement:
 *          Start TTC RPU counter, wait for IPI kick, check if data is
 *          available, if yes, read as much data as possible from shared
 *          memory. It will iterates untill 1000 packages have been received,
 *          stop TTC RPU counter and kick IPI to notify the remote. Repeat
 *          for different package size.
 *        - Upload throughput measurement:
 *          Start TTC RPU counter, write data to shared memory and kick IPI
 *          to notify remote. It will iterate for 1000 times, stop TTC RPU
 *          counter.Wait for APU IPI kick to know APU has received all the
 *          packages. Kick IPI to notify it TTC RPU conter value is ready to
 *          read. Repeat for different package size.
 *
 * @param[in] ch - channel information
 * @return - 0 on success, error code if failure.
 */
static int measure_shmem_throughputd(struct channel_s *ch)
{
	void *lbuf = NULL;
	int ret = 0;
	size_t s;
	uint32_t rx_count, rx_avail, tx_count, iterations;
	unsigned long tx_avail_offset, rx_avail_offset;
	unsigned long tx_addr_offset, rx_addr_offset;
	unsigned long tx_data_offset, rx_data_offset;
	uint32_t buf_phy_addr_32;

	/* allocate memory for receiving data */
	lbuf = metal_allocate_memory(BUF_SIZE_MAX);
	if (!lbuf) {
		LPERROR("Failed to allocate memory.\r\n");
		return -1;
	}
	memset(lbuf, 0xA, BUF_SIZE_MAX);

	/* Clear shared memory */
	metal_io_block_set(ch->shm_io, 0, 0, metal_io_region_size(ch->shm_io));

	LPRINTF("Starting shared mem throughput demo\n");

	/* for each data size, measure block receive throughput */
	for (s = PKG_SIZE_MIN; s <= PKG_SIZE_MAX; s <<= 1) {
		rx_count = 0;
		iterations = TOTAL_DATA_SIZE / s;
		/* Set rx buffer address offset */
		rx_avail_offset = SHM_DESC_OFFSET_RX + SHM_DESC_AVAIL_OFFSET;
		rx_addr_offset = SHM_DESC_OFFSET_RX +
				SHM_DESC_ADDR_ARRAY_OFFSET;
		rx_data_offset = SHM_DESC_OFFSET_RX + SHM_BUFF_OFFSET_RX;
		wait_for_notified(&ch->remote_nkicked);
		/* Data has arrived, seasure start. Reset RPU TTC counter */
		reset_timer(ch->ttc_io, TTC_CNT_RPU_TO_APU);
		while (1) {
			rx_avail = metal_io_read32(ch->shm_io, rx_avail_offset);

			while(rx_count != rx_avail) {
				/* Get the buffer location from the shared
				 * memory rx address array.
			         */
				buf_phy_addr_32 = metal_io_read32(ch->shm_io,
							rx_addr_offset);
				rx_data_offset = metal_io_phys_to_offset(
					ch->shm_io,
					(metal_phys_addr_t)buf_phy_addr_32);
				if (rx_data_offset == METAL_BAD_OFFSET) {
					LPERROR(
					"[%u]failed to get rx offset: 0x%x, 0x%lx.\n",
					rx_count, buf_phy_addr_32,
					metal_io_phys(ch->shm_io,
						rx_addr_offset));
					ret = -EINVAL;
					goto out;
				}
				rx_addr_offset += sizeof(buf_phy_addr_32);
				/* Read data from shared memory */
				metal_io_block_read(ch->shm_io, rx_data_offset,
						lbuf, s);
				rx_count++;
			}
			if (rx_count < iterations)
				/* Need to wait for more data */
				wait_for_notified(&ch->remote_nkicked);
			else
				break;
		}
		/* Stop RPU TTC counter */
		stop_timer(ch->ttc_io, TTC_CNT_RPU_TO_APU);
		/* Clear remote kicked flag -- 0 is kicked */
		atomic_init(&ch->remote_nkicked, 1);
		/* Kick IPI to notify RPU TTC counter value is ready */
		metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
	}

	/* for each data size, measure send throughput */
	for (s = PKG_SIZE_MIN; s <= PKG_SIZE_MAX; s <<= 1) {
		tx_count = 0;
		iterations = TOTAL_DATA_SIZE / s;

		/* Set tx buffer address offset */
		tx_avail_offset = SHM_DESC_OFFSET_TX + SHM_DESC_AVAIL_OFFSET;
		tx_addr_offset = SHM_DESC_OFFSET_TX +
				SHM_DESC_ADDR_ARRAY_OFFSET;
		tx_data_offset = SHM_DESC_OFFSET_TX + SHM_BUFF_OFFSET_TX;
		/* Wait for APU to signal it is ready for the measurement */
		wait_for_notified(&ch->remote_nkicked);
		/* Data has arrived, seasure start. Reset RPU TTC counter */
		reset_timer(ch->ttc_io, TTC_CNT_RPU_TO_APU);
		while (tx_count < iterations) {
			/* Write data to the shared memory*/
			metal_io_block_write(ch->shm_io, tx_data_offset,
					lbuf, s);

			/* Write to the address array to tell the other end
			 * the buffer address.
			 */
			buf_phy_addr_32 = (uint32_t)metal_io_phys(ch->shm_io,
						tx_data_offset);
			metal_io_write32(ch->shm_io, tx_addr_offset,
					buf_phy_addr_32);
			tx_data_offset += s;
			tx_addr_offset += sizeof(buf_phy_addr_32);

			/* Increase number of available buffers */
			tx_count++;
			metal_io_write32(ch->shm_io, tx_avail_offset, tx_count);
			/* Kick IPI to notify remote data is ready in the
			 * shared memory */
			metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET,
					ch->ipi_mask);
		}
		/* Stop RPU TTC counter */
		stop_timer(ch->ttc_io, TTC_CNT_RPU_TO_APU);
		/* Wait for IPI kick to know when the remote is ready
		 * to read the TTC counter value */
		wait_for_notified(&ch->remote_nkicked);
		/* Kick IPI to notify RPU TTC counter value is ready */
		metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
	}

out:
	if (lbuf)
		metal_free_memory(lbuf);
	return ret;
}

int shmem_throughput_demod()
{
	struct channel_s ch;
	int ipi_irq;
	int ret = 0;

	print_demo("shared memory throughput");
	memset(&ch, 0, sizeof(ch));

	/* Get shared memory device IO region */
	if (!shm_dev) {
		ret = -ENODEV;
		goto out;
	}
	ch.shm_io = metal_device_io_region(shm_dev, 0);
	if (!ch.shm_io) {
		LPERROR("Failed to map io region for %s.\n", shm_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get TTC IO region */
	ch.ttc_io = metal_device_io_region(ttc_dev, 0);
	if (!ch.ttc_io) {
		LPERROR("Failed to map io region for %s.\n", ttc_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* Get IPI device IO region */
	ch.ipi_io = metal_device_io_region(ipi_dev, 0);
	if (!ch.ipi_io) {
		LPERROR("Failed to map io region for %s.\n", ipi_dev->name);
		ret = -ENODEV;
		goto out;
	}

	/* disable IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* clear old IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_ISR_OFFSET, IPI_MASK);

	ch.ipi_mask = IPI_MASK;

	/* Get the IPI IRQ from the opened IPI device */
	ipi_irq = (intptr_t)ipi_dev->irq_info;

	/* Register IPI irq handler */
	metal_irq_register(ipi_irq, ipi_irq_handler, ipi_dev, &ch);
	/* initialize remote_nkicked */
	atomic_init(&ch.remote_nkicked, 1);
	/* Enable IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_IER_OFFSET, IPI_MASK);

	/* Run atomic operation demo */
	ret = measure_shmem_throughputd(&ch);

	/* disable IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* unregister IPI irq handler */
	metal_irq_unregister(ipi_irq, 0, ipi_dev, &ch);

out:
	return ret;

}

