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

#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__
#include <asc_config.h>

#include "asc_security_core/asc_result.h"
#include "asc_security_core/utils/notifier_topics.h"
#include "asc_security_core/components_factory_enum.h"

#ifndef ASC_DYNAMIC_MEMORY_ENABLED
#ifndef ASC_NOTIFIERS_OBJECT_POOL_ENTRIES /* if ASC_NOTIFIERS_OBJECT_POOL_ENTRIES not defined set default */
#define ASC_NOTIFIERS_OBJECT_POOL_ENTRIES (NOTIFY_TOPICS_NUMBER * (COMPONENTS_COUNT+1))
#endif
#ifndef ASC_EXTRA_NOTIFIERS_OBJECT_POOL_ENTRIES  /* Add ASC_EXTRA_NOTIFIERS_OBJECT_POOL_ENTRIES */
#define NOTIFIERS_POOL_ENTRIES ASC_NOTIFIERS_OBJECT_POOL_ENTRIES
#else
#define NOTIFIERS_POOL_ENTRIES (ASC_NOTIFIERS_OBJECT_POOL_ENTRIES + ASC_EXTRA_NOTIFIERS_OBJECT_POOL_ENTRIES)
#endif
#else /* Set unlimited in case of ASC_DYNAMIC_MEMORY_ENABLED */
#define NOTIFIERS_POOL_ENTRIES 0
#endif

typedef struct notifier_t notifier_t;

struct notifier_t {
    void (*notify)(notifier_t *notifier, int msg_num, void *payload);
};

/** @brief Notify subscribers of a specific topic that an event has occurred.
 *         msg_num, payload and payload_len are topic specific.
 *
 *  @param topic Topic enumerator
 *  @param msg_num Message number that notifier callback will receive
 *  @param payload Pointer to data that notifier callback will receive
 *  @return Number of active subscribers
 */
int32_t notifier_notify(notify_topic_t topic, int msg_num, void *payload);

/** @brief Subscribe a notifier for a topic.
 *         Topics should be defined in notifier.h
 *  @param topic topic Topic enumerator
 *  @param notifier Topic notifier with callback filled in
 *  @return ASC_RESULT_OK on success, ASC_RESULT_EXCEPTION otherwise.
 */
asc_result_t notifier_subscribe(notify_topic_t topic, notifier_t *notifier);

/** @brief Unsubscribe a notifier for a topic
 *
 *  @param topic Topic enumerator
 *  @param notifier Topic notifier with callback filled in
 *  @return ASC_RESULT_OK on success, ASC_RESULT_EXCEPTION otherwise.
 */
asc_result_t notifier_unsubscribe(notify_topic_t topic, notifier_t *notifier);

/** @brief Unsubscribe all notifiers for a topic
 *
 *  @param topic Topic enumerator
 *  @return ASC_RESULT_OK on success, ASC_RESULT_EXCEPTION otherwise.
 */
asc_result_t notifier_deinit(notify_topic_t topic);
#endif