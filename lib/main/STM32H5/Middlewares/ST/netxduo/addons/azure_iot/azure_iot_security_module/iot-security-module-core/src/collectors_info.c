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
#include <stdio.h>
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/collectors_info.h"
#include "asc_security_core/collector.h"
#include "asc_security_core/components_factory.h"
#include "asc_security_core/utils/containerof.h"
#include "asc_security_core/utils/notifier.h"

typedef struct notifier_container_t {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(struct notifier_container_t);

    notifier_t notifier;
    collector_info_t info[COLLECTORS_INFO_SIZE];
} notifier_container_t;

OBJECT_POOL_DECLARATIONS(notifier_container_t)
OBJECT_POOL_DEFINITIONS(notifier_container_t, 1)

static void _collector_info_cb(notifier_t *notifier, int message_num, void *payload)
{
    notifier_container_t *container = containerof(notifier, notifier_container_t, notifier);
    collector_info_t *info = container->info;
    collector_internal_t *collector_internal_ptr = payload;

    if (collector_internal_ptr == NULL) {
        log_error("Wrong (NULL) data was recieved");
        return;
    }

    if (collector_internal_ptr->type >= COLLECTORS_INFO_SIZE) {
        log_error("Wrong collector type=[%d]", collector_internal_ptr->type);
    } else {
        info[collector_internal_ptr->type].interval = collector_internal_ptr->interval;
        log_debug("Updated configuration for collector=[%d] with interval=[%lu]\n",
            collector_internal_ptr->type,
            info[collector_internal_ptr->type].interval);
    }
}

collectors_info_t *collectors_info_init(void)
{
    notifier_container_t *container = object_pool_get(notifier_container_t);

    if (container == NULL) {
        log_error("Failed to allocate notifier container object");
        return 0;
    }
    memset(container, 0, sizeof(notifier_container_t));
    container->notifier.notify = _collector_info_cb;
    if (notifier_subscribe(NOTIFY_TOPIC_SYSTEM, &container->notifier) != ASC_RESULT_OK) {
        collectors_info_deinit((collectors_info_t *)container);
        return NULL;
    }

    return (collectors_info_t *)container;
}

void collectors_info_deinit(collectors_info_t *collectors_info)
{
    notifier_container_t *container = (notifier_container_t *)collectors_info;

    if (container == NULL) {
        log_error("collectors_info_t *is NULL");
        return;
    }
    notifier_unsubscribe(NOTIFY_TOPIC_SYSTEM, &container->notifier);
    object_pool_free(notifier_container_t, container);
}

collector_info_t *collectors_info_get(collectors_info_t *collectors_info, uint32_t *size)
{
    notifier_container_t *container = (notifier_container_t *)collectors_info;

    if (container == NULL) {
        log_error("collectors_info_t *is NULL");
        return NULL;
    }
    if (size) {
        *size = COLLECTORS_INFO_SIZE;
    }
    return container->info;
}