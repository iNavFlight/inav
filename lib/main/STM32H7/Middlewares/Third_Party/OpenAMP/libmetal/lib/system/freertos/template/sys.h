/*
 * Copyright (c) 2018, Linaro Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	freertos/template/sys.h
 * @brief	freertos template system primitives for libmetal.
 */

#ifndef __METAL_FREERTOS_SYS__H__
#error "Include metal/sys.h instead of metal/freertos/@PROJECT_MACHINE@/sys.h"
#endif

#ifndef __METAL_FREERTOS_TEMPLATE_SYS__H__
#define __METAL_FREERTOS_TEMPLATE_SYS__H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef METAL_INTERNAL

void sys_irq_enable(unsigned int vector);

void sys_irq_disable(unsigned int vector);

#endif /* METAL_INTERNAL */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_FREERTOS_SYS__H__ */
