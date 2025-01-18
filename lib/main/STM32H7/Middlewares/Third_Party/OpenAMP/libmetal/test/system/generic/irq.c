/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <metal/errno.h>

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
	int rc = 0, flags_1, flags_2;
	char *err_msg="";
	enum metal_log_level mll= metal_get_log_level();

	/* Do not show LOG_ERROR or LOG_DEBUG for expected fail case */
	metal_set_log_level(METAL_LOG_CRITICAL);

	rc = metal_irq_register(1, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 1 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(2, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(2, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(3, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 3 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(4, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 4 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(4, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 4 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(1, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 1 fail drv_id 2\n";
		goto out;
	}

	rc = metal_irq_unregister(1, 0, 0, (void *)0);
	if (rc) {
		err_msg = "unregister irq 1 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(1, 0, 0, (void *)0);
	if (!rc) {
		err_msg = "unregister irq 1 fail expected\n";
		goto out;
	}

	rc = metal_irq_unregister(2, 0, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 2 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(2, 0, 0, (void *)2);
	if (!rc) {
		err_msg = "unregister irq 2 drv_id 2 fail expected\n";
		goto out;
	}
	rc = metal_irq_register(2, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_unregister(2, 0, 0, (void *)1);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 1 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(2, 0, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 2 failed \n";
		goto out;
	}

	rc = metal_irq_register(3, irq_handler, 0, (void *)1);
	if (!rc) {
		err_msg = "register irq 3 drv_id 1 overwrite fail expected\n";
		goto out;
	}
	rc = metal_irq_register(3, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 3 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_unregister(3, irq_handler+1, 0, (void *)0);
	if (!rc) {
		err_msg = "unregister irq 3 match handler fail expected\n";
		goto out;
	}
	rc = metal_irq_unregister(3, irq_handler, 0, (void *)0);
	if (rc) {
		err_msg = "unregister irq 3 match handler failed \n";
		goto out;
	}

	rc = metal_irq_unregister(4, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 4 match handler and drv_id 2 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(4, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "unregister irq 4 match handler and drv_id 1 failed \n";
		goto out;
	}

	rc = metal_irq_register(5, irq_handler, (void *)10, (void *)1);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(5, irq_handler, (void *)20, (void *)2);
	if (rc) {
		err_msg = "register irq 5 fail dev 20 drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(5, irq_handler, (void *)10, (void *)3);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 3\n";
		goto out;
	}
	rc = metal_irq_register(5, irq_handler, 0, (void *)4);
	if (rc) {
		err_msg = "register irq 5 fail drv_id 4\n";
		goto out;
	}
	rc = metal_irq_register(5, irq_handler, (void *)10, (void *)5);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 5\n";
		goto out;
	}

	rc = metal_irq_unregister(5, irq_handler, (void *)10, (void *)3);
	if (rc) {
		err_msg = "unregister irq 5 match handle, dev 10 and drv_id 3 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(5, 0, 0, (void *)4);
	if (rc) {
		err_msg = "unregister irq 5 drv_id 4 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(5, 0, (void *)10, 0);
	if (rc) {
		err_msg = "unregister irq 5 dev 10 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(5, 0, (void *)20, (void *)2);
	if (rc) {
		err_msg = "unregister irq 5 match dev 20 and drv_id 2 failed \n";
		goto out;
	}

	rc = metal_irq_register(-1, irq_handler, 0, (void *)1);
	if (!rc) {
		err_msg = "register irq -1 should have failed\n";
		goto out;
	}

	/* global interrupt disable/enable normal behavior */
	flags_1=metal_irq_save_disable();
	metal_irq_restore_enable(flags_1);

	/* global interrupt less common */
	flags_1=metal_irq_save_disable();
	flags_2=metal_irq_save_disable();
	metal_irq_restore_enable(flags_2);
	metal_irq_restore_enable(flags_1);

	rc = 0;

out:
	metal_set_log_level(mll);
	if ((err_msg[0] != '\0') && (!rc))
		rc = -EINVAL;
	if (rc) metal_log(METAL_LOG_ERROR, "%s", err_msg);
	return rc;
}

METAL_ADD_TEST(irq);
