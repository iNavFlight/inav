/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*****************************************************************************
 * ipi_latency_demod.c
 * This is the remote side of the IPI latency measurement demo.
 * This demo does the follwing steps:
 *
 *  1. Open the shared memory device.
 *  1. Open the TTC timer device.
 *  2. Open the IPI device.
 *  3. Register IPI interrupt handler.
 *  6. When it receives IPI interrupt, the IPI interrupt handler to stop
 *     the RPU to APU TTC counter.
 *  7. Check the shared memory to see if demo is on. If the demo is on,
 *     reest the RPU to APU TTC counter and kick IPI to notify the remote.
 *  8. If the shared memory indicates the demo is off, cleanup resource:
 *     disable IPI interrupt and deregister the IPI interrupt handler.
 */

#include <unistd.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include "common.h"

#define TTC_CNT_APU_TO_RPU 2 /* APU to RPU TTC counter ID */
#define TTC_CNT_RPU_TO_APU 3 /* RPU to APU TTC counter ID */

#define TTC_CLK_FREQ_HZ	100000000

/* Shared memory offset */
#define SHM_DEMO_CNTRL_OFFSET    0x0

#define DEMO_STATUS_IDLE         0x0
#define DEMO_STATUS_START        0x1 /* Status value to indicate demo start */

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
			/* stop RPU -> APU timer */
			stop_timer(ch->ttc_io, TTC_CNT_APU_TO_RPU);
			metal_io_write32(ch->ipi_io, IPI_ISR_OFFSET,
					ch->ipi_mask);
			atomic_flag_clear(&ch->remote_nkicked);
			return METAL_IRQ_HANDLED;
		}
	}
	return METAL_IRQ_NOT_HANDLED;
}


/**
 * @brief measure_ipi_latencyd() - measure IPI latency with libmetal
 *        Loop until APU tells RPU to stop via shared memory.
 *        In loop, wait for interrupt (interrupt handler stops APU to
 *        RPU TTC counter). Then reset count on RPU to APU TTC counter
 *        and kick IPI to notify APU.
 *
 * @param[in] ch - channel information
 * @return - 0 on success, error code if failure.
 */
static int measure_ipi_latencyd(struct channel_s *ch)
{
	LPRINTF("Starting IPI latency demo\r\n");
	while(1) {
		wait_for_notified(&ch->remote_nkicked);
		if (metal_io_read32(ch->shm_io, SHM_DEMO_CNTRL_OFFSET) ==
			DEMO_STATUS_START) {
			/* Reset RPU to APU TTC counter */
			reset_timer(ch->ttc_io, TTC_CNT_RPU_TO_APU);
			/* Kick IPI to notify the remote */
			metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET,
					ch->ipi_mask);
		} else {
			break;
		}
	}
	return 0;
}

int ipi_latency_demod()
{
	struct channel_s ch;
	int ipi_irq;
	int ret = 0;

	print_demo("IPI latency");
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
	ret = measure_ipi_latencyd(&ch);

	/* disable IPI interrupt */
	metal_io_write32(ch.ipi_io, IPI_IDR_OFFSET, IPI_MASK);
	/* unregister IPI irq handler */
	metal_irq_unregister(ipi_irq, 0, ipi_dev, &ch);

out:
	return ret;

}

