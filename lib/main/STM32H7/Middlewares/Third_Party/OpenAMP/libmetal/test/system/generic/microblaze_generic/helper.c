/*
 * Copyright (c) 2017 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"

/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	init_platform();
	return 0;
}
