/*
 * Memory based image store Operation
 *
 * Copyright(c) 2018 Xilinx Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/io.h>
#include <metal/sys.h>
#include <openamp/remoteproc_loader.h>
#include <stdarg.h>
#include <stdio.h>
/* Xilinx headers */
#include <pm_api_sys.h>
#include <pm_defs.h>
#include <xil_printf.h>

#define LPRINTF(format, ...) xil_printf(format, ##__VA_ARGS__)
//#define LPRINTF(format, ...)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

struct mem_file {
	const void *base;
};

int mem_image_open(void *store, const char *path, const void **image_data)
{
	struct mem_file *image = store;
	const void *fw_base = image->base;

	(void)(path);
	if (image_data == NULL) {
		LPERROR("%s: input image_data is NULL\r\n", __func__);
		return -EINVAL;
	}
	*image_data = fw_base;
	/* return an abitrary length, as the whole firmware is in memory */
	return 0x100;
}

void mem_image_close(void *store)
{
	/* The image is in memory, does nothing */
	(void)store;
}

int mem_image_load(void *store, size_t offset, size_t size,
		   const void **data, metal_phys_addr_t pa,
		   struct metal_io_region *io,
		   char is_blocking)
{
	struct mem_file *image = store;
	const void *fw_base = image->base;

	(void)is_blocking;

	LPRINTF("%s: offset=0x%x, size=0x%x\n\r",
		__func__, offset, size);
	if (pa == METAL_BAD_PHYS) {
		if (data == NULL) {
			LPERROR("%s: data is NULL while pa is ANY\r\n",
				__func__);
			return -EINVAL;
		}
		*data = (const void *)((const char *)fw_base + offset);
	} else {
		void *va;

		if (io == NULL) {
			LPERROR("%s, io is NULL while pa is not ANY\r\n",
				__func__);
			return -EINVAL;
		}
		va = metal_io_phys_to_virt(io, pa);
		if (va == NULL) {
			LPERROR("%s: no va is found\r\n", __func__);
			return -EINVAL;
		}
		memcpy(va, (const void *)((const char *)fw_base + offset), size);
	}

	return (int)size;
}

struct image_store_ops mem_image_store_ops = {
	.open = mem_image_open,
	.close = mem_image_close,
	.load = mem_image_load,
	.features = SUPPORT_SEEK,
};

