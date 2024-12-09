/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Arm Limited
 * Copyright (c) 2023 STMicroelectronics
 */

#include "bootutil/fault_injection_hardening.h"

#ifdef FIH_ENABLE_DELAY

#include "low_level_rng.h"
#include "boot_hal_flowcontrol.h"

/* Mbedtls implementation of the delay RNG (based on a DRGB) is replaced by
 * another RNG implementation (based on NDRBG) that is backed by a pure entropy
 * source (physical process that provides full entropy outputs).
 * This is not provided as a header API and a C file implementation due to
 * issues with inlining.
 */

#define RNG_NUMBER 10
unsigned char seed_buf[RNG_NUMBER] = {0};
size_t index_seed_buf = 0;

int fih_delay_init(void)
{
  size_t len = 0U;

  /* generate several random */
  RNG_GetBytes((unsigned char *)seed_buf, sizeof(seed_buf),(size_t *)&len);
  return (1);
}

unsigned char fih_delay_random_uchar(void)
{
  unsigned char delay;

  delay = seed_buf[index_seed_buf];
  index_seed_buf++;

  if ( RNG_NUMBER == index_seed_buf )
  {
    fih_delay_init();
    index_seed_buf = 0;
  }
  return delay;
}

#endif /* FIH_ENABLE_DELAY */
