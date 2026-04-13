/*
 * Copyright (c) 2018 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**************************************************************************
 * FILE NAME
 *
 *       zynq_a9_rproc.c
 *
 * DESCRIPTION
 *
 *       This file define Xilinx Zynq A9 platform specific remoteproc
 *       implementation.
 *
 **************************************************************************/

#include <openamp/remoteproc.h>
#include <metal/atomic.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <platform_info.h>
#include <xil_printf.h>

/* SCUGIC macros */
#define GIC_DIST_SOFTINT                   0xF00
#define GIC_SFI_TRIG_CPU_MASK              0x00FF0000
#define GIC_SFI_TRIG_SATT_MASK             0x00008000
#define GIC_SFI_TRIG_INTID_MASK            0x0000000F
#define GIC_CPU_ID_BASE                    (1 << 4)

static int zynq_a9_proc_irq_handler(int vect_id, void *data)
{
	struct remoteproc *rproc = data;
	struct remoteproc_priv *prproc;

	(void)vect_id;
	if (!rproc)
		return METAL_IRQ_NOT_HANDLED;
	prproc = rproc->priv;
	atomic_flag_clear(&prproc->nokick);
	return METAL_IRQ_HANDLED;
}

static struct remoteproc *
zynq_a9_proc_init(struct remoteproc *rproc,
			struct remoteproc_ops *ops, void *arg)
{
	struct remoteproc_priv *prproc = arg;
	struct metal_device *dev;
	unsigned int irq_vect;
	int ret;

	if (!rproc || !prproc || !ops)
		return NULL;
	ret = metal_device_open(prproc->gic_bus_name, prproc->gic_name,
				&dev);
	if (ret) {
		xil_printf("failed to open GIC device: %d.\r\n", ret);
		return NULL;
	}
	rproc->priv = prproc;
	prproc->gic_dev = dev;
	prproc->gic_io = metal_device_io_region(dev, 0);
	if (!prproc->gic_io)
		goto err1;
	rproc->ops = ops;
	atomic_flag_test_and_set(&prproc->nokick);

	/* Register interrupt handler and enable interrupt */
	irq_vect = prproc->irq_notification;
	metal_irq_register(irq_vect, zynq_a9_proc_irq_handler, NULL, rproc);
	metal_irq_enable(irq_vect);
	xil_printf("Successfully intialize remoteproc.\r\n");
	return rproc;
err1:
	metal_device_close(dev);
	return NULL;
}

static void zynq_a9_proc_remove(struct remoteproc *rproc)
{
	struct remoteproc_priv *prproc;
	struct metal_device *dev;

	if (!rproc)
		return;
	prproc = rproc->priv;
	metal_irq_disable(prproc->irq_to_notify);
	metal_irq_unregister(prproc->irq_to_notify, NULL, NULL, NULL);
	dev = prproc->gic_dev;
	if (dev)
		metal_device_close(dev);
}

static void *
zynq_a9_proc_mmap(struct remoteproc *rproc, metal_phys_addr_t *pa,
			metal_phys_addr_t *da, size_t size,
			unsigned int attribute, struct metal_io_region **io)
{
	struct remoteproc_mem *mem;
	metal_phys_addr_t lpa, lda;
	struct metal_io_region *tmpio;

	lpa = *pa;
	lda = *da;

	if (lpa == METAL_BAD_PHYS && lda == METAL_BAD_PHYS)
		return NULL;
	if (lpa == METAL_BAD_PHYS)
		lpa = lda;
	if (lda == METAL_BAD_PHYS)
		lda = lpa;

	if (!attribute)
		attribute = NORM_NONCACHE | STRONG_ORDERED;
	mem = metal_allocate_memory(sizeof(*mem));
	if (!mem)
		return NULL;
	tmpio = metal_allocate_memory(sizeof(*tmpio));
	if (!tmpio) {
		metal_free_memory(mem);
		return NULL;
	}
	remoteproc_init_mem(mem, NULL, lpa, lda, size, tmpio);
	/* va is the same as pa in this platform */
	metal_io_init(tmpio, (void *)lpa, &mem->pa, size,
		      sizeof(metal_phys_addr_t)<<3, attribute, NULL);
	remoteproc_add_mem(rproc, mem);
	*pa = lpa;
	*da = lda;
	if (io)
		*io = tmpio;
	return metal_io_phys_to_virt(tmpio, mem->pa);
}

static int zynq_a9_proc_notify(struct remoteproc *rproc, uint32_t id)
{
	struct remoteproc_priv *prproc;
	unsigned long mask = 0;

	(void)id;
	if (!rproc)
		return -1;
	prproc = rproc->priv;
	if (!prproc->gic_io)
		return -1;

	mask = ((1 << (GIC_CPU_ID_BASE + prproc->cpu_id)) |
		(prproc->irq_to_notify))
	    & (GIC_SFI_TRIG_CPU_MASK | GIC_SFI_TRIG_INTID_MASK);
	/* Trigger IPI */
	metal_io_write32(prproc->gic_io, GIC_DIST_SOFTINT, mask);
	return 0;
}

/* processor operations from r5 to a53. It defines
 * notification operation and remote processor managementi operations. */
struct remoteproc_ops zynq_a9_proc_ops = {
	.init = zynq_a9_proc_init,
	.remove = zynq_a9_proc_remove,
	.mmap = zynq_a9_proc_mmap,
	.notify = zynq_a9_proc_notify,
	.start = NULL,
	.stop = NULL,
	.shutdown = NULL,
};
