/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Arm Limited
 */

#ifndef __FAULT_INJECTION_HARDENING_DELAY_RNG_H__
#define __FAULT_INJECTION_HARDENING_DELAY_RNG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief Set up the RNG for use with random delays. Called once at startup.
 */
int fih_delay_init(void);

/**
 * \brief Get a random unsigned char from an RNG seeded with an entropy source.
 *
 * \return A random value that fits inside an unsigned char.
 */
unsigned char fih_delay_random_uchar(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FAULT_INJECTION_HARDENING_DELAY_RNG_H__ */
