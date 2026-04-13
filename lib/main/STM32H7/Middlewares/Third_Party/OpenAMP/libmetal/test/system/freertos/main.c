/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "metal-test.h"
#include "FreeRTOS.h"
#include "task.h"

#define MAIN_THREAD_STACK_SZ 512

static void run_tests(void *param)
{
	(void)param;

	(void)metal_tests_run(NULL);
	 vTaskDelete(NULL);
}

int main(void)
{
	BaseType_t stat;

	stat = xTaskCreate(run_tests, "run_tests", MAIN_THREAD_STACK_SZ,
			   NULL, 2, NULL);
	if (stat != pdPASS) {
		metal_log(METAL_LOG_ERROR, "failed to create run_tests thread\n");
	} else {
		/* Start running FreeRTOS tasks */
		vTaskStartScheduler();
	}

	/* Will not get here, unless a call is made to vTaskEndScheduler() */
	while (1) {
		__asm__("wfi\n\t");
	}

	/* suppress compilation warnings*/
	return 0;
}
