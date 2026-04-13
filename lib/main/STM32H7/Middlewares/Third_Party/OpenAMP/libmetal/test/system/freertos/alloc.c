/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/errno.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "metal-test.h"
#include <metal/alloc.h>
#include <metal/log.h>
#include <metal/sys.h>

static const int test_count = 10;

static void *alloc_thread(void *arg)
{
	int i;
	void *ptr;
	void *rv = 0;

	(void)arg;

	for (i = 0; i < test_count; i++) {
		/* expecting the implementation to be thread safe */
		ptr = metal_allocate_memory(256 /*10*i*/);
		if (!ptr) {
			metal_log(METAL_LOG_DEBUG, "failed to allocate memmory\n");
		        rv = (void *)-ENOMEM;
			break;
		}

		metal_free_memory(ptr);
	}

	return rv;
}

static int alloc(void)
{
	const int threads = 10;
	int rc;

	rc =  metal_run(threads, alloc_thread, NULL);

	return rc;
}
METAL_ADD_TEST(alloc);
