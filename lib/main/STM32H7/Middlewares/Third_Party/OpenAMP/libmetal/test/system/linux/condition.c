/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pthread.h>

#include "metal-test.h"
#include <metal/log.h>
#include <metal/sys.h>
#include <metal/mutex.h>
#include <metal/condition.h>

#define COUNTER_MAX 10

#define THREADS 10

METAL_MUTEX_DEFINE(lock);
static struct metal_condition nempty_condv = METAL_CONDITION_INIT;
static struct metal_condition nfull_condv = METAL_CONDITION_INIT;
static unsigned int counter;

static void *consumer_thread(void *arg)
{
	(void)arg;
	metal_mutex_acquire(&lock);
	while (!counter)
		metal_condition_wait(&nempty_condv, &lock);
	counter--;
	metal_condition_signal(&nfull_condv);
	metal_mutex_release(&lock);

	return NULL;
}

static void *producer_thread(void *arg)
{
	(void)arg;
	metal_mutex_acquire(&lock);
	while (counter == COUNTER_MAX)
		metal_condition_wait(&nfull_condv, &lock);
	counter++;
	metal_condition_signal(&nempty_condv);
	metal_mutex_release(&lock);

	return NULL;
}
static int condition(void)
{
	int ret;
	int ts_created;
	pthread_t tids[THREADS];

	/** TC1 consumer threads go first */
	/** create 10 consumer threads first */
	ret = metal_run_noblock(THREADS, consumer_thread, NULL, tids,
				&ts_created);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create consumer thread: %d.\n",
			  ret);
		goto out;
	}

	/** create 10 producer threads next */
	ret = metal_run(THREADS, producer_thread, NULL);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create producer thread: %d.\n",
			  ret);
		goto out;
	}

	/** wait for consumer threads to finish */
	metal_finish_threads(THREADS, (void *)tids);

	/** TC2 producer threads go first */
	/** create 10 producer threads first */
	ret = metal_run_noblock(THREADS, producer_thread, NULL, tids,
				&ts_created);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create consumer thread: %d.\n",
			  ret);
		goto out;
	}

	/** create 10 consumer threads next */
	ret = metal_run(THREADS, consumer_thread, NULL);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create producer thread: %d.\n",
			  ret);
		goto out;
	}

out:
	/** wait for producer threads to finish */
	metal_finish_threads(THREADS, (void *)tids);
	return ret;
}
METAL_ADD_TEST(condition);
