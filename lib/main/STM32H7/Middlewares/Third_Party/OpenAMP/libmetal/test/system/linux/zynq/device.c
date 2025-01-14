/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <unistd.h>

#include "metal-test.h"
#include <metal/device.h>

static int device(void)
{
	struct metal_device *device;
	struct metal_io_region *io;
	uint32_t idcode;
	int error;

	error = metal_device_open("platform", "f8000000.slcr", &device);
	if (error)
		return error;

	io = metal_device_io_region(device, 0);
	if (!io) {
		metal_device_close(device);
		return -ENODEV;
	}

	idcode = metal_io_read32(io, 0x530);
	metal_log(METAL_LOG_DEBUG, "Read id code %x\n", idcode);

	metal_device_close(device);

	return 0;
}
METAL_ADD_TEST(device);
