/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	metal-test.h
 * @brief	Top level include internal to libmetal tests.
 */

#ifndef __METAL_TEST__H__
#define __METAL_TEST__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <metal/sys.h>
#include <metal/list.h>

typedef int (*test_func_t)(void);
typedef void *(*metal_thread_t)(void *);

struct metal_test_case {
	struct metal_list node;
	const char *name;
	test_func_t test;
};


extern void metal_add_test_case(struct metal_test_case *test_case);


#define METAL_ADD_TEST(func)						\
__attribute__ ((constructor)) static void metal_test_##func() {		\
	static struct metal_test_case metal_test_##func = {		\
		.name	= #func,					\
		.test	= func,						\
	};								\
	metal_add_test_case(&metal_test_##func);			\
}

/**
 * @brief        run all tests cases
 *
 * @param[in]    metal init parameters
 *
 * @return       non-zero on error
 */
int metal_tests_run(struct metal_init_params *params);


/**
 * @brief        run child threads and wait until all they finish.
 *               if tids1 is zero, that is not required to store
 *               threads ids for the caller, it will not return
 *               until all the threads finishes.
 * @param[in]    threads how many threads to run
 * @param[in]    child routine which the threads will run
 * @param[in]    arg argument passed to the child threads
 *
 * @return       zero on no errors, non-zero on errors
 */
extern int metal_run(int threads, metal_thread_t child, void *arg);

/**
 * @brief        run child threads and return without waiting
 *               for all the threads to finish.
 * @param[in]    threads  how many threads to run
 * @param[in]    child routine which the threads will run
 * @param[in]    arg argument passed to the child threads
 * @param[in]    tids pointers to store the threads ids.
 *                    the caller is required to make sure the tids is
 *                    big enough to store all the child threads ids.
 * @param[out]   threads_out number of threads created
 *
 * @return       zero on no errors, non-zero on errors
 */
extern int metal_run_noblock(int threads, metal_thread_t child,
		    void *arg, void *tids, int *threads_out);

/**
 * @brief        do not return until all the specified child threads
 *               finish.
 * @param[in]    threads how many threads to wait
 * @param[in]    tids pointers to the threads ids.
 *
*/
extern void metal_finish_threads(int threads, void *tids);

#ifdef __cplusplus
}
#endif

#endif /* __METAL_TEST__H__ */
