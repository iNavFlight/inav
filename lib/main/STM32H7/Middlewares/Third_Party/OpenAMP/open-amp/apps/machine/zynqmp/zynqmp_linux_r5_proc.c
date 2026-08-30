/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017 Xilinx, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**************************************************************************
 * FILE NAME
 *
 *       platform_info.c
 *
 * DESCRIPTION
 *
 *       This file define Xilinx ZynqMP R5 to A53 platform specific 
 *       remoteproc implementation.
 *
 **************************************************************************/
#include <metal/alloc.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/irq.h>
#include <metal/device.h>
#include <metal/utilities.h>
#include <openamp/remoteproc.h>
#include <openamp/rpmsg_virtio.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include "platform_info.h"

/* IPI REGs OFFSET */
#define IPI_TRIG_OFFSET          0x00000000    /* IPI trigger register offset */
#define IPI_OBS_OFFSET           0x00000004    /* IPI observation register offset */
#define IPI_ISR_OFFSET           0x00000010    /* IPI interrupt status register offset */
#define IPI_IMR_OFFSET           0x00000014    /* IPI interrupt mask register offset */
#define IPI_IER_OFFSET           0x00000018    /* IPI interrupt enable register offset */
#define IPI_IDR_OFFSET           0x0000001C    /* IPI interrupt disable register offset */

static int zynqmp_linux_r5_proc_irq_handler(int vect_id, void *data)
{
    struct remoteproc *rproc = data;
    struct remoteproc_priv *prproc;
    unsigned int ipi_intr_status;

    (void)vect_id;
    if (!rproc)
        return METAL_IRQ_NOT_HANDLED;
    prproc = rproc->priv;
    ipi_intr_status = (unsigned int)metal_io_read32(prproc->ipi_io,
                            IPI_ISR_OFFSET);
    if (ipi_intr_status & prproc->ipi_chn_mask) {
        atomic_flag_clear(&prproc->ipi_nokick);
        metal_io_write32(prproc->ipi_io, IPI_ISR_OFFSET,
                 prproc->ipi_chn_mask);
        return METAL_IRQ_HANDLED;
    }   
    return METAL_IRQ_NOT_HANDLED;
}

static struct remoteproc *
zynqmp_linux_r5_proc_init(struct remoteproc *rproc,
              struct remoteproc_ops *ops, void *arg)
{
    struct remoteproc_priv *prproc = arg;
    struct metal_device *dev;
    unsigned int irq_vect;
    metal_phys_addr_t mem_pa;
    int ret;

    if (!rproc || !prproc || !ops)
        return NULL;
    rproc->priv = prproc;
    rproc->ops = ops;
    prproc->ipi_dev = NULL;
    prproc->shm_dev = NULL;
    /* Get shared memory device */
    ret = metal_device_open(prproc->shm_bus_name, prproc->shm_name,
                &dev);
    if (ret) {
        fprintf(stderr, "ERROR: failed to open shm device: %d.\n", ret);
        goto err1;
    }
    printf("Successfully open shm device.\n");
    prproc->shm_dev = dev;
    prproc->shm_io = metal_device_io_region(dev, 0);
    if (!prproc->shm_io)
        goto err2;
    mem_pa = metal_io_phys(prproc->shm_io, 0);
    remoteproc_init_mem(&prproc->shm_mem, "shm", mem_pa, mem_pa,
                metal_io_region_size(prproc->shm_io),
                prproc->shm_io);
    remoteproc_add_mem(rproc, &prproc->shm_mem);
    printf("Successfully added shared memory\n");
    /* Get IPI device */
    ret = metal_device_open(prproc->ipi_bus_name, prproc->ipi_name,
                &dev);
    if (ret) {
        printf("failed to open ipi device: %d.\n", ret);
        goto err2;
    }
    prproc->ipi_dev = dev;
    prproc->ipi_io = metal_device_io_region(dev, 0);
    if (!prproc->ipi_io)
        goto err3;
    printf("Successfully probed IPI device\n");
    atomic_store(&prproc->ipi_nokick, 1);

    /* Register interrupt handler and enable interrupt */
    irq_vect = (uintptr_t)dev->irq_info;
    metal_irq_register(irq_vect, zynqmp_linux_r5_proc_irq_handler,
               dev, rproc);
    metal_irq_enable(irq_vect);
    metal_io_write32(prproc->ipi_io, IPI_IER_OFFSET,
             prproc->ipi_chn_mask);
    printf("Successfully initialized Linux r5 remoteproc.\n");
    return rproc;
err3:
    metal_device_close(prproc->ipi_dev);
err2:
    metal_device_close(prproc->shm_dev);
err1:
    return NULL;
}

static void zynqmp_linux_r5_proc_remove(struct remoteproc *rproc)
{
    struct remoteproc_priv *prproc;
    struct metal_device *dev;

    if (!rproc)
        return;
    prproc = rproc->priv;
    metal_io_write32(prproc->ipi_io, IPI_IDR_OFFSET, prproc->ipi_chn_mask);
    dev = prproc->ipi_dev;
    if (dev) {
        metal_irq_disable((uintptr_t)dev->irq_info);
        metal_irq_unregister((uintptr_t)dev->irq_info, NULL, NULL,
                     NULL);
        metal_device_close(dev);
    }
    if (prproc->shm_dev)
        metal_device_close(prproc->shm_dev);
}

static void *
zynqmp_linux_r5_proc_mmap(struct remoteproc *rproc, metal_phys_addr_t *pa,
              metal_phys_addr_t *da, size_t size,
              unsigned int attribute, struct metal_io_region **io)
{
    struct remoteproc_priv *prproc;
    metal_phys_addr_t lpa, lda;
    struct metal_io_region *tmpio;

    (void)attribute;
    (void)size;
    if (!rproc)
        return NULL;
    prproc = rproc->priv;
    lpa = *pa;
    lda = *da;

    if (lpa == METAL_BAD_PHYS && lda == METAL_BAD_PHYS)
        return NULL;
    if (lpa == METAL_BAD_PHYS)
        lpa = lda;
    if (lda == METAL_BAD_PHYS)
        lda = lpa;
    tmpio = prproc->shm_io;
    if (!tmpio)
        return NULL;

    *pa = lpa;
    *da = lda;
    if (io)
        *io = tmpio;
    return metal_io_phys_to_virt(tmpio, lpa);
}

static int zynqmp_linux_r5_proc_notify(struct remoteproc *rproc, uint32_t id)
{
    struct remoteproc_priv *prproc;

    (void)id;
    if (!rproc)
        return -1;
    prproc = rproc->priv;

    /* TODO: use IPI driver instead and pass ID */
    metal_io_write32(prproc->ipi_io, IPI_TRIG_OFFSET,
              prproc->ipi_chn_mask);
    return 0;
}

/* processor operations from r5 to a53. It defines
 * notification operation and remote processor managementi operations. */
struct remoteproc_ops zynqmp_linux_r5_proc_ops = {
    .init = zynqmp_linux_r5_proc_init,
    .remove = zynqmp_linux_r5_proc_remove,
    .mmap = zynqmp_linux_r5_proc_mmap,
    .notify = zynqmp_linux_r5_proc_notify,
    .start = NULL,
    .stop = NULL,
    .shutdown = NULL,
};
