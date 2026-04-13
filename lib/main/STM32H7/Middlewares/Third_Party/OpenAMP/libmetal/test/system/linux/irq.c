/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <metal/errno.h>
#include <sys/eventfd.h>

/* We need to find the internal MAX_IRQS limit */
/* Could be retrieved from platform specific files in the future */
#define METAL_INTERNAL

#include "metal-test.h"
#include <metal/irq.h>
#include <metal/log.h>
#include <metal/sys.h>
#include <metal/list.h>
#include <metal/utilities.h>


int irq_handler(int irq, void *priv)
{
	(void)irq;
	(void)priv;

	return 0;
}


static int irq(void)
{
	int rc = 0;
	char *err_msg="";
	enum metal_log_level mll= metal_get_log_level();
	int i, tst_irq[6];

	/* Do not show LOG_ERROR or LOG_DEBUG for expected fail case */
	metal_set_log_level(METAL_LOG_CRITICAL);

	for (i=1; i < 6; i++) {
		/* we only want to test the lib API, so create 'virtual' IRQs */
		tst_irq[i] = eventfd(0,0);
		metal_log(METAL_LOG_DEBUG, "%s: irq %d associated with fd %d\n", __func__, i, tst_irq[i]);
	}

	rc = metal_irq_register(tst_irq[1], irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 1 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[2], irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[2], irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[3], irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 3 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[4], irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 4 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[4], irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 4 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[1], irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 1 fail drv_id 2\n";
		goto out;
	}

	rc = metal_irq_unregister(tst_irq[1], 0, 0, (void *)0);
	if (rc) {
		err_msg = "unregister irq 1 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[1], 0, 0, (void *)0);
	if (!rc) {
		err_msg = "unregister irq 1 success but fail expected\n";
		goto out;
	}

	rc = metal_irq_unregister(tst_irq[2], 0, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 2 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[2], 0, 0, (void *)2);
	if (!rc) {
		err_msg = "unregister irq 2 drv_id 2 success but fail expected\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[2], irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[2], 0, 0, (void *)1);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 1 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[2], 0, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 2 failed \n";
		goto out;
	}

	rc = metal_irq_register(tst_irq[3], irq_handler, 0, (void *)1);
	if (!rc) {
		err_msg = "register irq 3 drv_id 1 overwrite fail expected\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[3], irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 3 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[3], irq_handler+1, 0, (void *)0);
	if (!rc) {
		err_msg = "unregister irq 3 match handler success but fail expected\n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[3], irq_handler, 0, (void *)0);
	if (rc) {
		err_msg = "unregister irq 3 match handler failed \n";
		goto out;
	}

	rc = metal_irq_unregister(tst_irq[4], irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 4 match handler and drv_id 2 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[4], irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "unregister irq 4 match handler and drv_id 1 failed \n";
		goto out;
	}

	rc = metal_irq_register(tst_irq[5], irq_handler, (void *)10, (void *)1);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[5], irq_handler, (void *)20, (void *)2);
	if (rc) {
		err_msg = "register irq 5 fail dev 20 drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[5], irq_handler, (void *)10, (void *)3);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 3\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[5], irq_handler, 0, (void *)4);
	if (rc) {
		err_msg = "register irq 5 fail drv_id 4\n";
		goto out;
	}
	rc = metal_irq_register(tst_irq[5], irq_handler, (void *)10, (void *)5);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 5\n";
		goto out;
	}

	rc = metal_irq_unregister(tst_irq[5], irq_handler, (void *)10, (void *)3);
	if (rc) {
		err_msg = "unregister irq 5 match handle, dev 10 and drv_id 3 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[5], 0, 0, (void *)4);
	if (rc) {
		err_msg = "unregister irq 5 drv_id 4 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[5], 0, (void *)10, 0);
	if (rc) {
		err_msg = "unregister irq 5 dev 10 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(tst_irq[5], 0, (void *)20, (void *)2);
	if (rc) {
		err_msg = "unregister irq 5 match dev 20 and drv_id 2 failed \n";
		goto out;
	}

	rc = 0;

out:
	for (i=1; i < 6; i++) {
		close(tst_irq[i]);
	}
	metal_set_log_level(mll);
	if ((err_msg[0] != '\0') && (!rc))
		rc = -EINVAL;
	if (rc) metal_log(METAL_LOG_ERROR, "%s", err_msg);
	return rc;
}

METAL_ADD_TEST(irq);
