/*******************************************************************************/
/*                                                                             */
/* Copyright (c) Microsoft Corporation. All rights reserved.                   */
/*                                                                             */
/* This software is licensed under the Microsoft Software License              */
/* Terms for Microsoft Azure Defender for IoT. Full text of the license can be */
/* found in the LICENSE file at https://aka.ms/AzureDefenderForIoT_EULA        */
/* and in the root directory of this software.                                 */
/*                                                                             */
/*******************************************************************************/

#ifndef CORE_H
#define CORE_H

#include <stdbool.h>
#include <limits.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"
#include "asc_security_core/collector_collection.h"
#include "asc_security_core/model/security_message.h"

#define CORE_NEAREST_TIMER_UNSET_VAL LONG_MAX

/**
 * @brief Get a security message from the core.
 *
 * @param core_ptr              The core ptr
 * @param security_message_ptr  The message buffer to write into
 *
 * @return  ASC_RESULT_OK on success,
 *          ASC_RESULT_EMPTY when there are no events to send, in that case message_ptr remains unchanged,
 *          ASC_RESULT_EXCEPTION otherwise.
 */
asc_result_t core_message_get(security_message_t* security_message_ptr);

/**
 * @brief Deinit the last security message.
 *
 * @return  ASC_RESULT_OK on success,
 *          ASC_RESULT_EXCEPTION otherwise.
 */
asc_result_t core_message_deinit(void);

/**
 * @brief Register collector to core
 *
 * @param collector_ptr             A @c collector_t* collector handle
 * 
 * @return  ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t core_collector_register(collector_t *collector_ptr);

/**
 * @brief Unregister collector from collector_collection
 *
 * @param collector_ptr             A @c collector_t* collector handle
 * 
 * @return  ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t core_collector_unregister(collector_t *collector_ptr);

/**
 * @brief Get random generated collection start time, what is generating on startup of system to avoid sending message
 *          from many devices simultaneously.
 *
 * @return  init_random_collect_offset value.
 */
unsigned long core_get_init_random_collect_offset(void);

/**
 * @brief Get current (last set) collect time.
 *
 * @return  nearest_collect_time value. In case of unset timer the CORE_NEAREST_TIMER_UNSET_VAL=LONG_MAX will be returned.
 */
unsigned long core_get_nearest_collect_time(void);

/**
 * @brief Get collectors collections that registered to core
 *
 * @return Pointer to collector collection @c collector_collection_t .
 */
collector_collection_t *core_get_collector_collection(void);

#endif /* CORE_H */