/*
 * Copyright (c) 2018-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_NV_COUNTERS_H
#define PLAT_NV_COUNTERS_H

/**
 * \file plat_nv_counters.h
 *
 * \note The interfaces defined in this file must be implemented for each
 *       SoC.
 * \note The interface must be implemented in a fail-safe way that is
 *       resistant to asynchronous power failures or it can use hardware
 *       counters that have this capability, if supported by the platform.
 *       When a counter incrementation was interrupted it must be able to
 *       continue the incrementation process or recover the previous consistent
 *       status of the counters. If the counters have reached a stable status
 *       (every counter incrementation operation has finished), from that point
 *       their value cannot decrease due to any kind of power failure.
 */

#include <stdint.h>

enum nv_counter_t {
    PLAT_NV_COUNTER_3,      /* Used by bootloader */
    PLAT_NV_COUNTER_4,      /* Used by bootloader */
    PLAT_NV_COUNTER_5,      /* Used by bootloader */
    PLAT_NV_COUNTER_6,      /* Used by bootloader */
    PLAT_NV_COUNTER_MAX
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialises all non-volatile (NV) counters.
 *
 * \return  HAL_OK if the initialization succeeds, otherwise
 *          HAL_ERROR
 */
HAL_StatusTypeDef plat_init_nv_counter(void);

/**
 * \brief Reads the given non-volatile (NV) counter.
 *
 * \param[in]  counter_id  NV counter ID.
 * \param[in]  size        Size of the buffer to store NV counter value
 *                         in bytes.
 * \param[out] val         Pointer to store the current NV counter value.
 *
 * \return  HAL_OK if the value is read correctly. Otherwise,
 *          it returns HAL_ERROR.
 */
HAL_StatusTypeDef plat_read_nv_counter(enum nv_counter_t counter_id,
                                       uint32_t size, uint8_t *val);

/**
 * \brief Sets the given non-volatile (NV) counter to the specified value.
 *
 * \param[in] counter_id  NV counter ID.
 * \param[in] value       New value of the NV counter. The maximum value that
 *                        can be set depends on the constraints of the
 *                        underlying implementation, but it always must be
 *                        greater than or equal to the current NV counter value.
 * \param[out] updated    Pointer to cnt updated status flag (1: yes, 0: no)
 *
 * \retval HAL_OK                       The NV counter is set successfully
 * \retval HAL_ERROR                    An error occurred
 */
HAL_StatusTypeDef plat_set_nv_counter(enum nv_counter_t counter_id,
                                      uint32_t value, uint32_t *updated);

#ifdef __cplusplus
}
#endif

#endif /* PLAT_NV_COUNTERS_H */
