/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "metal-test.h"

int main(void)
{
	int status;

	status = metal_tests_run(NULL);

	return status;
}
