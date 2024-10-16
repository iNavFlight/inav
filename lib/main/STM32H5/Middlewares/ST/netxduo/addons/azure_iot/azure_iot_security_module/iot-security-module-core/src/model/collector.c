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

#include "asc_security_core/utils/itime.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/components_manager.h"
#include "asc_security_core/core.h"
#include "asc_security_core/utils/notifier.h"

#include "asc_security_core/collector.h"

OBJECT_POOL_DEFINITIONS(collector_t, COLLECTOR_OBJECT_POOL_COUNT)

static collector_t *collector_alloc(component_id_t id)
{
    collector_t *collector_ptr = object_pool_get(collector_t);

    __components_manager_set_ctx(id, collector_ptr);
    if (collector_ptr == NULL) {
        log_error("Failed to allocate collector=[%s]", components_manager_get_name(id));
        goto cleanup;
    }
    memset(collector_ptr, 0, sizeof(collector_t));

cleanup:
    return collector_ptr;
}

static asc_result_t collector_init_with_params(collector_internal_t *collector_internal_ptr, collector_enum_t type, collector_priority_t priority, collector_serialize_function_t collect_function, unsigned long interval, void *state)
{
    if (collector_internal_ptr == NULL) {
        return ASC_RESULT_BAD_ARGUMENT;
    }

    memset(collector_internal_ptr, 0, sizeof(*collector_internal_ptr));

    collector_internal_ptr->type = type;
    collector_internal_ptr->priority = priority;
    collector_internal_ptr->interval = interval;
    collector_internal_ptr->collect_function = collect_function;
    collector_internal_ptr->state = state;

    return ASC_RESULT_OK;
}

asc_result_t collector_default_create(component_id_t id, collector_enum_t type, collector_priority_t priority, collector_serialize_function_t collect_function, unsigned long interval, void *state)
{
    asc_result_t result = ASC_RESULT_OK;
    collector_t *collector_ptr = collector_alloc(id);

    if (collector_ptr == NULL) {
        log_error("Failed to init collector type=[%d] name=[%s]", type, components_manager_get_name(id));
        result = ASC_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }
    result = collector_init_with_params(&collector_ptr->internal, type, priority, collect_function, interval, state);
    if (result != ASC_RESULT_OK) {
        log_error("Failed to init internal collector parameters type=[%d] name=[%s]", type, components_manager_get_name(id));
        goto cleanup;
    }
#ifdef ASC_COLLECTORS_INFO_SUPPORT
    notifier_notify(NOTIFY_TOPIC_SYSTEM, NOTIFY_SYSTEM_CONFIGURATION, &collector_ptr->internal);
#endif

cleanup:
    return result;
}

asc_result_t collector_default_deinit(component_id_t id)
{
    collector_t *collector_ptr = __components_manager_get_ctx(id);

    if (collector_ptr == NULL) {
        log_error("collector uninitialized name=[%s]", components_manager_get_name(id));
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    memset(collector_ptr, 0, sizeof(collector_t));
    object_pool_free(collector_t, collector_ptr);
    __components_manager_set_ctx(id, NULL);
    return ASC_RESULT_OK;
}

asc_result_t collector_default_subscribe(component_id_t id)
{
    collector_t *collector_ptr = __components_manager_get_ctx(id);

    if (collector_ptr == NULL) {
        log_error("collector uninitialized name=[%s]", components_manager_get_name(id));
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    return core_collector_register(collector_ptr);
}

asc_result_t collector_default_unsubscribe(component_id_t id)
{
    collector_t *collector_ptr = __components_manager_get_ctx(id);

    if (collector_ptr == NULL) {
        log_error("collector uninitialized name=[%s]", components_manager_get_name(id));
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    return core_collector_unregister(collector_ptr);
}

asc_result_t collector_default_start(component_id_t id)
{
    return ASC_RESULT_OK;
}

asc_result_t collector_default_stop(component_id_t id)
{
    return ASC_RESULT_OK;
}

collector_priority_t collector_get_priority(collector_t *collector_ptr)
{
    collector_priority_t priority = COLLECTOR_PRIORITY_COUNT;

    if (collector_ptr == NULL) {
        log_error("Failed to retrieve collector priority, bad argument");
    } else {
        priority = collector_ptr->internal.priority;
    }

    return priority;
}

unsigned long collector_get_last_collected_timestamp(collector_t *collector_ptr)
{
    unsigned long last_collected_timestamp = 0;

    if (collector_ptr == NULL) {
        log_error("Failed to retrieve collector last collected timestamp, bad argument");
    } else {
        last_collected_timestamp = collector_ptr->last_collected_timestamp;
    }

    return last_collected_timestamp;
}

asc_result_t collector_set_last_collected_timestamp(collector_t *collector_ptr, unsigned long last_collected_timestamp)
{
    if (collector_ptr == NULL) {
        log_error("Failed to set collector last collected timestamp, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }

    collector_ptr->last_collected_timestamp = last_collected_timestamp;

    return ASC_RESULT_OK;
}

asc_result_t collector_serialize_events(collector_t *collector_ptr, serializer_t *serializer)
{
    if (collector_ptr == NULL || serializer == NULL) {
        log_error("Collector failed to serialize events, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }
    unsigned long now = itime_time(NULL);
    if (now == ITIME_FAILED) {
        log_error("Error get current time");
        now = 0;
    }

    collector_ptr->last_collected_timestamp = now;

    if (collector_ptr->internal.collect_function == NULL) {
        log_error("Collector failed to serialize events, internal collect function is NULL");
        return ASC_RESULT_EXCEPTION;
    }

    return collector_ptr->internal.collect_function(&collector_ptr->internal, serializer);
}
