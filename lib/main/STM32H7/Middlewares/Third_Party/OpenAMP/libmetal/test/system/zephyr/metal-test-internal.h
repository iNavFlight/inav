/*
 * Copyright (c) 2017, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	system/zephyr/metal-test-internal.h
 * @brief	Zephyr include internal to libmetal tests.
 */

#ifndef __METAL_TEST_ZEPHYR_INTERNAL__H__
#define __METAL_TEST_ZEPHYR_INTERNAL__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "metal-test.h"

#undef METAL_ADD_TEST
#define METAL_ADD_TEST(func)						\
void metal_test_add_##func() {		\
	static struct metal_test_case metal_test_##func = {		\
		.name	= #func,					\
		.test	= func,						\
	};								\
	metal_add_test_case(&metal_test_##func);			\
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_TEST_ZEPHYR_INTERNAL__H__ */
