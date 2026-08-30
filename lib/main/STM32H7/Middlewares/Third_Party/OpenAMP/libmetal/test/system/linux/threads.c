/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/sys.h>
#include <metal/utilities.h>
#include "metal-test.h"

int metal_run(int threads, metal_thread_t child, void *arg)
{
	pthread_t tids[threads];
	int error, ts_created;

	error = metal_run_noblock(threads, child, arg, tids, &ts_created);

	metal_finish_threads(ts_created, (void *)tids);

	return error;
}

int metal_run_noblock(int threads, metal_thread_t child,
		     void *arg, void *tids, int *threads_out)
{
	int error, i;
	pthread_t *tid_p = (pthread_t *)tids;

	if (!tids) {
		metal_log(METAL_LOG_ERROR, "invalid arguement, tids is NULL.\n");
		return -EINVAL;
	}

	error = 0;
	for (i = 0; i < threads; i++) {
		error = -pthread_create(&tid_p[i], NULL, child, arg);
		if (error) {
			metal_log(METAL_LOG_ERROR, "failed to create thread - %s\n",
				  strerror(error));
			break;
		}
	}

	*threads_out = i;
	return error;
}

void metal_finish_threads(int threads, void *tids)
{
	int i;
	pthread_t *tid_p = (pthread_t *)tids;

	if (!tids) {
		metal_log(METAL_LOG_ERROR, "invalid argument, tids is NULL.\n");
		return;
	}

	for (i = 0; i < threads; i++)
		(void)pthread_join(tid_p[i], NULL);
}
