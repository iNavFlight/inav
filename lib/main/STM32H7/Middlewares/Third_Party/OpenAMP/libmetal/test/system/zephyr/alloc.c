/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include <metal/alloc.h>
#include <metal/log.h>
#include <metal/sys.h>
#include <misc/printk.h>
#include "metal-test-internal.h"

static int alloc(void)
{
	void *ptr;

	ptr = metal_allocate_memory(1000);
	if (!ptr) {
		metal_log(METAL_LOG_DEBUG, "failed to allocate memmory\n");
		return errno;
	}

	metal_free_memory(ptr);

	return 0;
}
METAL_ADD_TEST(alloc);
