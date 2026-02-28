/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "metal-test.h"
#include <metal/compiler.h>
#include <metal/sys.h>
#include <metal/utilities.h>

static METAL_DECLARE_LIST(test_cases);

/*
 * Not every enviornment has strerror() implemented.
 */
#ifdef NOT_HAVE_STRERROR
char metal_weak *strerror(int errnum)
{
	static char errstr[33];
	int i, j;

	if (errnum < 0)
		return NULL;

	i = 0;
	while (errnum) {
		int digit = errnum % 10;

		errstr[i++] = '0' + (char)digit;
		errnum /= 10;
	}
	errstr[i] = '\0';

	j = i - 1;
	for (i = 0; i < j; i++, j--) {
		char *tmp = &errstr[i];

		errstr[i] = errstr[j];
		errstr[j] = *tmp;
	}
	return errstr;
}
#endif

void metal_add_test_case(struct metal_test_case *test_case)
{
	metal_list_add_tail(&test_cases, &test_case->node);
}

int metal_tests_run(struct metal_init_params *params)
{
	struct metal_init_params dparams = METAL_INIT_DEFAULTS;
	struct metal_test_case *test_case;
	struct metal_list *node;
	int error, errors = 0;
	const char *dots = "..................................";
	const char *pad;

	if (!params)
		params = &dparams;

	params->log_level = METAL_LOG_DEBUG;
	error = metal_init(params);
	if (error)
		return error;

	metal_list_for_each(&test_cases, node) {
		test_case = metal_container_of(node, struct metal_test_case,
					       node);
		pad = dots + strlen(test_case->name);
		metal_log(METAL_LOG_INFO,"running [%s]\n", test_case->name);
		error = test_case->test();
		metal_log(METAL_LOG_INFO,"result [%s]%s %s%s%s\n",
		       test_case->name, pad,
		       error ? "fail" : "pass",
		       error ? " - error: " : "",
		       error ? strerror(-error) : "");
		if (error)
			errors++;
	}

	metal_finish();

	return errors;
}
