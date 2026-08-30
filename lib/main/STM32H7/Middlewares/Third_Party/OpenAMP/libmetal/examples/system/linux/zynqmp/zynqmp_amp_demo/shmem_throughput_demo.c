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
 *  6. Upload throughput measurement:
 *     Start TTC APU counter, write data to shared memory and kick IPI to
 *     notify remote. It will iterate for 1000 times, stop TTC APU counter.
 *     Wait for RPU IPI kick to know RPU has finished receiving packages
 *     and RPU TX counter is ready to read. Read the APU TX and RPU RX
 *     counter values and save them. Repeat for different package sizes.
 *     After this measurement, kick IPI to notify the remote, the
 *     measurement has finished.
 *  7. Download throughput measurement:
 *     Start TTC APU counter, wait for IPI kick, check if data is available,
 *     if yes, read as much data as possible from shared memory. It will
 *     iterates untill 1000 packages have been received, stop TTC APU counter.
 *     Wait for RPU IPI kick so that APU can get the TTC RPU TX counter
 *     value. Kick IPI to notify the remote it has read the TTCi counter.
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
#include <error.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/alloc.h>
#include <metal/irq.h>
#include "common.h"

#define TTC_CNT_APU_TO_RPU 2 /* APU to RPU TTC counter ID */
#define TTC_CNT_RPU_TO_APU 3 /* RPU to APU TTC counter ID */

#define TTC_CLK_FREQ_HZ	100000000
#define NS_PER_SEC 1000000000

/* Shared memory offsets */
#define SHM_DESC_OFFSET_TX 0x0
#define SHM_BUFF_OFFSET_TX 0x400000
#define SHM_DESC_OFFSET_RX 0x200000
#define SHM_BUFF_OFFSET_RX 0x800000

/* Shared memory descriptors offset */
#define SHM_DESC_AVAIL_OFFSET 0x00
#define SHM_DESC_ADDR_ARRAY_OFFSET 0x04

#define ITERATIONS 1000

#define BUF_SIZE_MAX 4096
#define PKG_SIZE_MAX 1024
#define PKG_SIZE_MIN 16
#define TOTAL_DATA_SIZE (1024 * 4096)

#define MB (1024 * 1024) /* Mega Bytes */

struct channel_s {
	struct metal_device *ipi_dev; /* IPI metal device */
	struct metal_io_region *ipi_io; /* IPI metal i/o region */
	struct metal_device *shm_dev; /* Shared memory metal device */
	struct metal_io_region *shm_io; /* Shared memory metal i/o region */
	struct metal_device *ttc_dev; /* TTC metal device */
	struct metal_io_region *ttc_io; /* TTC metal i/o region */
	uint32_t ipi_mask; /* RPU IPI mask */
	atomic_int remote_nkicked; /* 0 - kicked from remote */
};

/**
 * @brief read_timer() - return TTC counter value
 *
 * @param[in] ttc_io - TTC timer i/o region
 * @param[in] cnt_id - counter ID
 */
static inline uint32_t read_timer(struct metal_io_region *ttc_io,
				unsigned long cnt_id)
{
	unsigned long offset = XTTCPS_CNT_VAL_OFFSET +
				XTTCPS_CNT_OFFSET(cnt_id);

	return metal_io_read32(ttc_io, offset);
}

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
 * @brief measure_shmem_throughput() - Show throughput of using shared memory.
 *        - Upload throughput measurement:
 *          Start TTC APU counter, write data to shared memory and kick IPI to
 *          notify remote. It will iterate for 1000 times, stop TTC APU
 *          counter. Wait for RPU IPI kick to know RPU has finished receiving
 *          packages and RPU TX counter is ready to read. Read the APU TX and
 *          RPU RX counter values and save them. Repeat for different package
 *          sizes. After this measurement, kick IPI to notify the remote, the
 *          measurement has finished.
 *        - Download throughput measurement:
 *          Start TTC APU counter, wait for IPI kick, check if data is
 *          available, if yes, read as much data as possible from shared
 *          memory. It will iterates untill 1000 packages have been received,
 *          stop TTC APU counter. Wait for RPU IPI kick so that APU can get
 *          the TTC RPU TX counter value. Kick IPI to notify the remote it
 *          has read the TTCi counter. Repeat for different package size.
 *
 * @param[in] ch - channel information, which contains the IPI i/o region,
 *                 shared memory i/o region and the ttc timer i/o region.
 * @return - 0 on success, error code if failure.
 */
static int measure_shmem_throughput(struct channel_s* ch)
{
	void *lbuf = NULL;
	int ret = 0;
	size_t s, i;
	uint32_t rx_count, rx_avail, tx_count, iterations;
	unsigned long tx_avail_offset, rx_avail_offset;
	unsigned long tx_addr_offset, rx_addr_offset;
	unsigned long tx_data_offset, rx_data_offset;
	uint32_t buf_phy_addr_32;
	uint32_t *apu_tx_count = NULL;
	uint32_t *apu_rx_count = NULL;
	uint32_t *rpu_tx_count = NULL;
	uint32_t *rpu_rx_count = NULL;

	/* allocate memory for receiving data */
	lbuf = metal_allocate_memory(BUF_SIZE_MAX);
	if (!lbuf) {
		LPERROR("Failed to allocate memory.\r\n");
		return -ENOMEM;
	}
	memset(lbuf, 0xA, BUF_SIZE_MAX);

	/* allocate memory for saving counter values */
	for (s = PKG_SIZE_MIN, i = 0; s <= PKG_SIZE_MAX; s <<=1, i++);
	apu_tx_count = metal_allocate_memory(i * sizeof(uint32_t));
	apu_rx_count = metal_allocate_memory(i * sizeof(uint32_t));
	rpu_tx_count = metal_allocate_memory(i * sizeof(uint32_t));
	rpu_rx_count = metal_allocate_memory(i * sizeof(uint32_t));
	if (!apu_tx_count || !apu_rx_count || !rpu_tx_count || !rpu_rx_count) {
		LPERROR("Failed to allocate memory.\r\n");
		ret = -ENOMEM;
		goto out;
	}

	/* Clear shared memory */
	metal_io_block_set(ch->shm_io, 0, 0, metal_io_region_size(ch->shm_io));

	LPRINTF("Starting shared mem throughput demo\n");

	/* for each data size, measure send throughput */
	for (s = PKG_SIZE_MIN, i = 0; s <= PKG_SIZE_MAX; s <<= 1, i++) {
		tx_count = 0;
		iterations = TOTAL_DATA_SIZE / s;
		/* Set tx buffer address offset */
		tx_avail_offset = SHM_DESC_OFFSET_TX + SHM_DESC_AVAIL_OFFSET;
		tx_addr_offset = SHM_DESC_OFFSET_TX +
				SHM_DESC_ADDR_ARRAY_OFFSET;
		tx_data_offset = SHM_DESC_OFFSET_TX + SHM_BUFF_OFFSET_TX;
		/* Reset APU TTC counter */
		reset_timer(ch->ttc_io, TTC_CNT_APU_TO_RPU);
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
			metal_io_write32(ch->shm_io, tx_avail_offset,
					tx_count);
			/* Kick IPI to notify RPU data is ready in
			 * the shared memory */
			metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET,
					ch->ipi_mask);
		}
		/* Stop RPU TTC counter */
		stop_timer(ch->ttc_io, TTC_CNT_APU_TO_RPU);
		/* Wait for RPU to signal RPU RX TTC counter is ready to
		 * read */
		wait_for_notified(&ch->remote_nkicked);
		/* Read TTC counter values */
		apu_tx_count[i] = read_timer(ch->ttc_io, TTC_CNT_APU_TO_RPU);
		rpu_rx_count[i] = read_timer(ch->ttc_io, TTC_CNT_RPU_TO_APU);
	}

	/* Kick IPI to notify RPU that APU has read the RPU RX TTC counter
	 * value */
	metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);

	/* for each data size, meaasure block read throughput */
	for (s = PKG_SIZE_MIN, i = 0; s <= PKG_SIZE_MAX; s <<= 1, i++) {
		rx_count = 0;
		iterations = TOTAL_DATA_SIZE / s;
		/* Set rx buffer address offset */
		rx_avail_offset = SHM_DESC_OFFSET_RX + SHM_DESC_AVAIL_OFFSET;
		rx_addr_offset = SHM_DESC_OFFSET_RX +
				SHM_DESC_ADDR_ARRAY_OFFSET;
		rx_data_offset = SHM_DESC_OFFSET_RX + SHM_BUFF_OFFSET_RX;

		wait_for_notified(&ch->remote_nkicked);
		/* Data has arrived, seasure start. Reset RPU TTC counter */
		reset_timer(ch->ttc_io, TTC_CNT_APU_TO_RPU);
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
		stop_timer(ch->ttc_io, TTC_CNT_APU_TO_RPU);
		/* Clear remote kicked flag -- 0 is kicked */
		atomic_init(&ch->remote_nkicked, 1);
		/* Kick IPI to notify remote it is ready to read data */
		metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
		/* Wait for RPU to signal RPU TX TTC counter is ready to
		 * read */
		wait_for_notified(&ch->remote_nkicked);
		/* Read TTC counter values */
		apu_rx_count[i] = read_timer(ch->ttc_io, TTC_CNT_APU_TO_RPU);
		rpu_tx_count[i] = read_timer(ch->ttc_io, TTC_CNT_RPU_TO_APU);
		/* Kick IPI to notify RPU APU has read the RPU TX TTC counter
		 * value */
		metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
	}

	/* Print the measurement result */
	for (s = PKG_SIZE_MIN, i = 0; s <= PKG_SIZE_MAX; s <<= 1, i++) {
		LPRINTF("Shared memory throughput of pkg size %lu : \n", s);
		LPRINTF("    APU send: %x, %lu MB/s\n", apu_tx_count[i],
			s * iterations * TTC_CLK_FREQ_HZ / apu_tx_count[i] / MB);
		LPRINTF("    APU receive: %x, %lu MB/s\n", apu_rx_count[i],
			s * iterations * TTC_CLK_FREQ_HZ / apu_rx_count[i] / MB);
		LPRINTF("    RPU send: %x, %lu MB/s\n", rpu_tx_count[i],
			s * iterations * TTC_CLK_FREQ_HZ / rpu_tx_count[i] / MB);
		LPRINTF("    RPU receive: %x, %lu MB/s\n", rpu_rx_count[i],
			s * iterations * TTC_CLK_FREQ_HZ / rpu_rx_count[i] / MB);
	}

	LPRINTF("Finished shared memory throughput\n");

out:
	if (lbuf)
		metal_free_memory(lbuf);
	if (apu_tx_count)
		metal_free_memory(apu_tx_count);
	if (apu_rx_count)
		metal_free_memory(apu_rx_count);
	if (rpu_tx_count)
		metal_free_memory(rpu_tx_count);
	if (rpu_rx_count)
		metal_free_memory(rpu_rx_count);
	return ret;
}

int shmem_throughput_demo()
{
	struct metal_device *dev;
	struct metal_io_region *io;
	struct channel_s ch;
	int ipi_irq;
	int ret = 0;

	print_demo("shared memory throughput");
	memset(&ch, 0, sizeof(ch));

	/* Open shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", SHM_DEV_NAME);
		goto out;
	}

	/* Get shared memory device IO region */
	io = metal_device_io_region(dev, 0);
	if (!io) {
		LPERROR("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.shm_dev = dev;
	ch.shm_io = io;

	/* Open TTC device */
	ret = metal_device_open(BUS_NAME, TTC_DEV_NAME, &dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", TTC_DEV_NAME);
		goto out;
	}

	/* Get TTC IO region */
	io = metal_device_io_region(dev, 0);
	if (!io) {
		LPERROR("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.ttc_dev = dev;
	ch.ttc_io = io;

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &dev);
	if (ret) {
		LPERROR("Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Get IPI device IO region */
	io = metal_device_io_region(dev, 0);
	if (!io) {
		LPERROR("Failed to map io region for %s.\n", dev->name);
		ret = -ENODEV;
		goto out;
	}
	ch.ipi_dev = dev;
	ch.ipi_io = io;

	/* Get the IPI IRQ from the opened IPI device */
	ipi_irq = (intptr_t)ch.ipi_dev->irq_info;

	/* disable IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* clear old IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_ISR_OFFSET, IPI_MASK);
	/* initialize remote_nkicked */
	atomic_init(&ch.remote_nkicked, 1);
	ch.ipi_mask = IPI_MASK;
	/* Register IPI irq handler */
	metal_irq_register(ipi_irq, ipi_irq_handler, ch.ipi_dev, &ch);
	/* Enable IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_IER_OFFSET, IPI_MASK);

	/* Run atomic operation demo */
	ret = measure_shmem_throughput(&ch);

	/* disable IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* unregister IPI irq handler by setting the handler to 0 */
	metal_irq_unregister(ipi_irq, 0, ch.ipi_dev, &ch);

out:
	if (ch.ttc_dev)
		metal_device_close(ch.ttc_dev);
	if (ch.shm_dev)
		metal_device_close(ch.shm_dev);
	if (ch.ipi_dev)
		metal_device_close(ch.ipi_dev);
	return ret;

}

