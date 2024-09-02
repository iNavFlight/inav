/*
 *  Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *  Copyright (c) 2023, STMicroelectronics. All rights reserved.
 *
 *  SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SECURITY_CNT_H__
#define __SECURITY_CNT_H__

/**
 * @file security_cnt.h
 *
 * @note The interface must be implemented in a fail-safe way that is
 *       resistant to asynchronous power failures or it can use hardware
 *       counters that have this capability, if supported by the platform.
 *       When a counter incrementation was interrupted it must be able to
 *       continue the incrementation process or recover the previous consistent
 *       status of the counters. If the counters have reached a stable status
 *       (every counter incrementation operation has finished), from that point
 *       their value cannot decrease due to any kind of power failure.
 *
 * @note A security counter might be implemented using non-volatile OTP memory
 *       (i.e. fuses) in which case it is the responsibility of the platform
 *       code to map each possible security counter values onto the fuse bits
 *       as the direct usage of counter values can be costly / impractical.
 */

#include <stdint.h>
#include "bootutil/fault_injection_hardening.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialises the security counters.
 *
 * @return                  FIH_SUCCESS on success
 */
fih_int boot_nv_security_counter_init(void);

/**
 * Reads the stored value of a given image's security counter.
 *
 * @param image_id          Index of the image (from 0).
 * @param security_cnt      Pointer to store the security counter value.
 *
 * @return                  FIH_SUCCESS on success
 */
fih_int boot_nv_security_counter_get(uint32_t image_id, fih_int *security_cnt);

/**
 * Updates the stored value of a given image's security counter with a new
 * security counter value if the new one is greater.
 *
 * @param image_id          Index of the image (from 0).
 * @param img_security_cnt  New security counter value. The new value must be
 *                          between 0 and UINT32_MAX and it must be greater than
 *                          or equal to the current security counter value.
 * @param updated           Pointer to cnt updated status flag (1: yes, 0: no)
 *
 * @return                  0 on success; nonzero on failure.
 */
int32_t boot_nv_security_counter_update(uint32_t image_id,
                                        uint32_t img_security_cnt,
                                        uint32_t *updated
);

#ifdef __cplusplus
}
#endif

#endif /* __SECURITY_CNT_H__ */
