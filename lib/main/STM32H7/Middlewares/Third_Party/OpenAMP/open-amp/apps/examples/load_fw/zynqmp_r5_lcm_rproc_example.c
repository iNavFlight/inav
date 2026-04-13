/*
 * ZynqMP RPU life cycle management remoteproc example implementation
 *
 * Copyright(c) 2018 Xilinx Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/alloc.h>
#include <metal/sys.h>
#include <metal/utilities.h>
#include <openamp/remoteproc.h>
#include <stdarg.h>
#include <stdio.h>
/* Xilinx headers */
#include <pm_api_sys.h>
#include <pm_defs.h>
#include <xil_mpu.h>
#include <xil_printf.h>
#include <xreg_cortexr5.h>

#define LPRINTF(format, ...) xil_printf(format, ##__VA_ARGS__)
//#define LPRINTF(format, ...)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

#define RPU_BASE_ADDR 0xFF9A0000
#define RPU_GLBL_CNTL_OFFSET 0
#define RPU0_CFG_BASE_OFFSET 0x100
#define RPU1_CFG_BASE_OFFSET 0x200
#define RPU_CFG_BASE_OFFSET(n) (RPU0_CFG_BASE_OFFSET << (n))
/* Boot memory bit. high for OCM, low for TCM */
#define VINITHI_BIT		metal_bit(2)
/* CPU halt bit, high: processor is running. low: processor is halt */
#define nCPUHALT_BIT		metal_bit(0)
/* RPU mode, high: split mode. low: lock step mode */
#define SLSPLIT_BIT		metal_bit(3)
/* Clamp mode. high: split mode. low: lock step mode */
#define SLCLAMP_BIT		metal_bit(4)
/* TCM mode. high: combine RPU TCMs. low: split TCM for RPU1 and RPU0 */
#define TCM_COMB_BIT		metal_bit(6)

struct r5_rproc_priv {
	struct remoteproc *rproc;
	int cpu_id;
	metal_phys_addr_t rpu_base;
	struct metal_io_region rpu_io;
};

/**
 * r5_rproc_boot_addr_config - configure the boot address of R5
 * @pdata: platform data
 *
 * This function will set the boot address based on if the
 * boot memory in the ELF file is TCM or OCM
 */
static uint32_t r5_rproc_boot_addr_config(struct r5_rproc_priv *priv,
				      metal_phys_addr_t bootaddr)
{
	uint32_t rpu_resetaddr;

	LPRINTF("%s: R5 ID: %d, boot_addr 0x%x\n",
		 __func__, priv->cpu_id, bootaddr);

	if (bootaddr < 0x40000)
		rpu_resetaddr = 0;
	else
		rpu_resetaddr = 1;
	return rpu_resetaddr;
}

/**
 * r5_rproc_mode_config - configure R5 operation mode
 * @pdata: platform data
 *
 * configure R5 to split mode or lockstep mode
 * based on the platform data.
 */
static void r5_rproc_mode_config(struct r5_rproc_priv *priv)
{
	uint32_t tmp;

	LPRINTF("%s: mode: %d\n", __func__, priv->cpu_id);
	tmp = metal_io_read32(&priv->rpu_io, RPU_GLBL_CNTL_OFFSET);
	if (priv->cpu_id == NODE_RPU) {
		/* RPU lock step mode */
		tmp &= ~SLSPLIT_BIT;
		tmp |= TCM_COMB_BIT;
		tmp |= SLCLAMP_BIT;
	} else {
		/* RPU split mode */
		tmp |= SLSPLIT_BIT;
		tmp &= ~TCM_COMB_BIT;
		tmp &= ~SLCLAMP_BIT;
	}
	metal_io_write32(&priv->rpu_io, RPU_GLBL_CNTL_OFFSET, tmp);
}

struct remoteproc *r5_rproc_init(struct remoteproc *rproc,
				 struct remoteproc_ops *ops, void *arg)
{
	struct r5_rproc_priv *priv;
	unsigned int cpu_id = *((unsigned int *)arg);

	if (cpu_id < NODE_RPU_0 || cpu_id > NODE_RPU_1) {
		xil_printf("rproc init: invalide node id: %d\n\r", cpu_id);
		return NULL;
	}

	xil_printf("rproc init: node id: %d\n\r", cpu_id);
	priv = metal_allocate_memory(sizeof(*priv));
	if (!priv)
		return NULL;
	memset(priv, 0, sizeof(*priv));
	priv->rproc = rproc;
	priv->cpu_id = cpu_id;
	priv->rproc->ops = ops;
	metal_list_init(&priv->rproc->mems);
	priv->rproc->priv = priv;
	priv->rpu_base = RPU_BASE_ADDR;
	metal_io_init(&priv->rpu_io, (void *)RPU_BASE_ADDR, &priv->rpu_base,
		      0x1000, (metal_phys_addr_t)(-1),
		      DEVICE_NONSHARED | PRIV_RW_USER_RW, NULL);

	r5_rproc_mode_config(priv);
	rproc->state = RPROC_READY;
	return priv->rproc;
}

void r5_rproc_remove(struct remoteproc *rproc)
{
	if (rproc) {
		struct r5_rproc_priv *priv;

		priv = (struct r5_rproc_priv *)rproc->priv;
		metal_free_memory(priv);
	}
}

void *r5_rproc_mmap(struct remoteproc *rproc,
		    metal_phys_addr_t *pa, metal_phys_addr_t *da,
		    size_t size, unsigned int attribute,
		    struct metal_io_region **io)
{
	struct remoteproc_mem *mem;
	struct r5_rproc_priv *priv;
	metal_phys_addr_t lpa, lda;

	priv = rproc->priv;

	if (!da || !pa)
		return NULL;
	LPRINTF("%s: pa=0x%x, da=0x%x, size=0x%x, atrribute=0x%x\n\r",
		__func__, *pa, *da, size, attribute);
	lda = *da;
	lpa = *pa;
	if (!attribute)
		attribute = NORM_SHARED_NCACHE | PRIV_RW_USER_RW;
	if (lda <= 0x40000) {
		metal_phys_addr_t lda_end;

		lda_end = lda + size;
		if (priv->cpu_id == NODE_RPU_0 || priv->cpu_id == NODE_RPU) {
			lpa = 0xFFE00000 + lda;
			if (lda < 0x10000)
				XPm_RequestNode(NODE_TCM_0_A,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
			if (lda <= 0x20000 && lda_end >= 0x10000)
				XPm_RequestNode(NODE_TCM_1_A,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
			if (lda <= 0x30000 && lda_end >= 0x20000)
				XPm_RequestNode(NODE_TCM_0_B,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
			if (lda <= 0x40000 && lda_end >= 0x30000)
				XPm_RequestNode(NODE_TCM_1_B,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
		} else if (priv->cpu_id == NODE_RPU_1) {
			lpa = 0xFFE90000 + lda;
			if (lda < 0x10000)
				XPm_RequestNode(NODE_TCM_1_A,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
			if (lda <= 0x30000 && lda_end >= 0x20000)
				XPm_RequestNode(NODE_TCM_1_B,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
		} else {
			LPERROR("mmap failed: invalid cpu node: %d\n",
				priv->cpu_id);
			return NULL;
		}
	}
	if (lpa == METAL_BAD_PHYS)
		lpa = lda;
	if (lpa == METAL_BAD_PHYS)
		return NULL;
	mem = metal_allocate_memory(sizeof(*mem));
	if (!mem)
		return NULL;
	mem->pa = lpa;
	mem->da = lda;

	*io = metal_allocate_memory(sizeof(struct metal_io_region));
	if (!*io) {
		metal_free_memory(mem);
		return NULL;
	}
	metal_io_init(*io, (void *)mem->pa, &mem->pa, size,
		      sizeof(metal_phys_addr_t)<<3, attribute, NULL);
	mem->io = *io;
	metal_list_add_tail(&rproc->mems, &mem->node);
	*pa = lpa;
	*da = lda;
	mem->size = size;
	return metal_io_phys_to_virt(*io, mem->pa);
}

int r5_rproc_start(struct remoteproc *rproc)
{
	struct r5_rproc_priv *priv;
	int ret;
	uint32_t resetaddr;

	priv = rproc->priv;
	resetaddr = r5_rproc_boot_addr_config(priv, rproc->bootaddr);
	ret = XPm_RequestWakeUp(priv->cpu_id, true, resetaddr,
			      REQUEST_ACK_BLOCKING);
	if (ret != XST_SUCCESS) {
		LPRINTF("%s: Failed to start RPU 0x%x, ret=0x%x\n\r",
			__func__, priv->cpu_id, ret);
		return -1;
	} else {
		return 0;
	}
}

int r5_rproc_stop(struct remoteproc *rproc)
{
	/* It is lacking a stop operation in the libPM */
	(void)rproc;
	return 0;
}

int r5_rproc_shutdown(struct remoteproc *rproc)
{
	struct r5_rproc_priv *priv;
	int ret;
	struct remoteproc_mem *mem;
	struct metal_list *node;

	priv = rproc->priv;
	/* Delete all the registered remoteproc memories */
	metal_list_for_each(&rproc->mems, node) {
		struct metal_list *tmpnode;
		metal_phys_addr_t pa, pa_end;

		mem = metal_container_of(node, struct remoteproc_mem, node);
		tmpnode = node;
		/* Release TCM resource */
		pa = mem->pa;
		pa_end = metal_io_phys(mem->io, metal_io_region_size(mem->io));
		if (priv->cpu_id == NODE_RPU_0 || priv->cpu_id == NODE_RPU) {
			if (pa < 0xFFE10000)
				XPm_RequestNode(NODE_TCM_0_A,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
			if (pa <= 0xFFE20000 && pa_end >= 0xFFE10000)
				XPm_RequestNode(NODE_TCM_1_A,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
			if (pa <= 0xFFE30000 && pa_end >= 0xFFE20000)
				XPm_RequestNode(NODE_TCM_0_B,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
			if (pa <= 0xFFE40000 && pa_end >= 0xFFE30000)
				XPm_RequestNode(NODE_TCM_1_B,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
		} else if (priv->cpu_id == NODE_RPU_1) {
			if (pa < 0xFFEA0000)
				XPm_RequestNode(NODE_TCM_1_A,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
			if (pa <= 0xFFC0000 && pa_end >= 0xFFEB0000)
				XPm_RequestNode(NODE_TCM_1_B,
						PM_CAP_ACCESS, 0,
						REQUEST_ACK_BLOCKING);
		}
		node = tmpnode->prev;
		metal_list_del(tmpnode);
		metal_free_memory(mem->io);
		metal_free_memory(mem);
	}

	ret = XPm_ForcePowerDown(priv->cpu_id, REQUEST_ACK_BLOCKING);
	if (ret != XST_SUCCESS)
		return -1;
	else
		return 0;
}

struct remoteproc_ops r5_rproc_ops = {
	.init = r5_rproc_init,
	.remove = r5_rproc_remove,
	.start = r5_rproc_start,
	.stop = r5_rproc_stop,
	.shutdown = r5_rproc_shutdown,
	.mmap = r5_rproc_mmap,
};
