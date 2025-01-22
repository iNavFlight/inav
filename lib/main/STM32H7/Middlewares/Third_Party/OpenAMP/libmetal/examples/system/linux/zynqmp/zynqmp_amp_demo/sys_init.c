
/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <unistd.h>
#include <metal/sys.h>
#include "common.h"

int sys_init()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	int ret;

	ret = metal_init(&init_param);
	if (ret)
		LPERROR("Failed to initialize libmetal\n");
	return ret;
}

void sys_cleanup()
{
	metal_finish();
}

void wait_for_interrupt(void) {
	return;
}
