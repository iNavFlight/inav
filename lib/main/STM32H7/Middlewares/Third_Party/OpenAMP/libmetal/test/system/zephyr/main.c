/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "metal-test-internal.h"
#include <kernel.h>
#include <metal/log.h>
#include <misc/printk.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define BLK_SIZE_MIN 128
#define BLK_SIZE_MAX 1024
#define BLK_NUM_MIN 64
#define BLK_NUM_MAX 16
#define BLK_ALIGN BLK_SIZE_MIN

K_MEM_POOL_DEFINE(kmpool, BLK_SIZE_MIN, BLK_SIZE_MAX, BLK_NUM_MAX, BLK_ALIGN);
struct k_mem_block block[BLK_NUM_MIN];

extern int init_system(void);
extern void metal_generic_default_poll(void);

extern void metal_test_add_alloc();
extern void metal_test_add_atomic();
extern void metal_test_add_irq();
extern void metal_test_add_mutex();

extern void *metal_zephyr_allocate_memory(unsigned int size)
{
	int i;
	struct k_mem_block *blk;

	for (i = 0; i < sizeof(block)/sizeof(block[0]); i++) {
		blk = &block[i];
		if (!blk->data) {
			if (k_mem_pool_alloc(&kmpool, blk,size, K_NO_WAIT))
				printk("Failed to alloc 0x%x memory.\n", size);
			return blk->data;
		}
	}

	printk("Failed to alloc 0x%x memory, no free blocks.\n", size);
	return NULL;
}

extern void metal_zephyr_free_memory(void *ptr)
{
	int i;
	struct k_mem_block *blk;

	for (i = 0; i < sizeof(block)/sizeof(block[0]); i++) {
		blk = &block[i];
		if (blk->data == ptr) {
			k_mem_pool_free(blk);
			blk->data = NULL;
			return;
		}
	}
	printk("Failed to free %p, no block matches.\n", ptr);
}

void metal_test_add_functions()
{
	metal_test_add_alloc();
	metal_test_add_atomic();
	metal_test_add_irq();
	metal_test_add_mutex();
}

int main(void)
{
	metal_test_add_functions();
	(void)metal_tests_run(NULL);

	while (1)
               metal_generic_default_poll();

	/* will not return, but quiet the compiler */
	return 0;
}
