/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "metal-test.h"

extern int init_system(void);
extern void metal_generic_default_poll(void);

int main(void)
{
	(void)init_system();
	(void)metal_tests_run(NULL);

	while (1)
               metal_generic_default_poll();

	/* will not return, but quiet the compiler */
	return 0;
}
