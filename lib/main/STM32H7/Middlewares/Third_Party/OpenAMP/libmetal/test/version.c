/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "metal-test.h"
#include <metal/config.h>
#include <metal/version.h>

static int version(void)
{
	char ver_def[16], ver_dyn[16];

	snprintf(ver_def, sizeof(ver_def), "%d.%d.%d",
		 METAL_VER_MAJOR,
		 METAL_VER_MINOR,
		 METAL_VER_PATCH);

	snprintf(ver_dyn, sizeof(ver_dyn), "%d.%d.%d",
		 metal_ver_major(), metal_ver_minor(), metal_ver_patch());

	return (strcmp(ver_def, METAL_VER) +
		strcmp(ver_dyn, metal_ver()));
}
METAL_ADD_TEST(version);
