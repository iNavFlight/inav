/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <metal/errno.h>
#include "FreeRTOS.h"
#include "task.h"
#include <metal/sys.h>
#include <metal/utilities.h>
#include "metal-test.h"

#define TEST_THREAD_STACK_SIZE 128

typedef struct {
		metal_thread_t thread_func;
		void *arg;
	} thread_wrap_arg_t;

static void thread_wrapper(void *arg)
{
	thread_wrap_arg_t *wrap_p = (thread_wrap_arg_t *)arg;
	(void)wrap_p->thread_func(wrap_p->arg);
	vPortFree(wrap_p);
	vTaskDelete(NULL);
}       

int metal_run(int threads, metal_thread_t child, void *arg)
{
	TaskHandle_t tids[threads];
	int error, ts_created;

	error = metal_run_noblock(threads, child, arg, (void *)tids, &ts_created);

	metal_finish_threads(ts_created, (void *)tids);

	return error;
}


int metal_run_noblock(int threads, metal_thread_t child,
		     void *arg, void *tids, int *threads_out)
{
	int i;
	TaskHandle_t *tid_p = (TaskHandle_t *)tids;
	BaseType_t stat = pdPASS;
	char tn[15];
	thread_wrap_arg_t *wrap_p;

	if (!tids) {
		metal_log(METAL_LOG_ERROR, "invalid argument, tids is NULL.\n");
		return -EINVAL;
	}

	for (i = 0; i < threads; i++) {
		snprintf(tn, metal_dim(tn), "%d", i);
		wrap_p = pvPortMalloc(sizeof(thread_wrap_arg_t));
		if (!wrap_p) {
			metal_log(METAL_LOG_ERROR, "failed to allocate wrapper %d\n", i);
			break;
		}
			
		wrap_p->thread_func = child;
		wrap_p->arg = arg;
		stat = xTaskCreate(thread_wrapper, tn, TEST_THREAD_STACK_SIZE,
				   wrap_p, 2, &tid_p[i]);
		if (stat != pdPASS) {
			metal_log(METAL_LOG_ERROR, "failed to create thread %d\n", i);
			vPortFree(wrap_p);
			break;
		}
	}

	*threads_out = i;
	return pdPASS == stat ? 0 : -ENOMEM;
}


void metal_finish_threads(int threads, void *tids)
{
	int i;
	TaskHandle_t *tid_p = (TaskHandle_t *)tids;

	if (!tids) {
		metal_log(METAL_LOG_ERROR, "invalid argument, tids is NULL.\n");
		return;
	}

	for (i = 0; i < threads; i++) {
		eTaskState ts;
		do {
			taskYIELD();
			ts=eTaskGetState(tid_p[i]);
		} while (ts != eDeleted);
	}
}
