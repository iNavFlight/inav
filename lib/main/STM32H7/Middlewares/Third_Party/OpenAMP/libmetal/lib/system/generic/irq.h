/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/irq.c
 * @brief	Generic libmetal irq definitions.
 */

#ifndef __METAL_IRQ__H__
#error "Include metal/irq.h instead of metal/generic/irq.h"
#endif

#ifndef __METAL_GENERIC_IRQ__H__
#define __METAL_GENERIC_IRQ__H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      default interrupt handler 
 * @param[in]  vector interrupt vector
 */
void metal_irq_isr(unsigned int vector);


#ifdef __cplusplus
}
#endif

#endif /* __METAL_GENERIC_IRQ__H__ */
