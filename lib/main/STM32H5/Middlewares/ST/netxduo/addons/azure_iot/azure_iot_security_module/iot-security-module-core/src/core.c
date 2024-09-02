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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/components_manager.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/serializer.h"
#include "asc_security_core/utils/ievent_loop.h"
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/utils/irand.h"
#include "asc_security_core/utils/notifier.h"
#include "asc_security_core/utils/os_utils.h"

#include "asc_security_core/core.h"
#include "asc_security_core/version.h"

#define CORE_OBJECT_POOL_COUNT 1
#define CORE_RESTART_SLEEP 1

typedef struct core {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(struct core);

    const char *security_module_id;
    uint32_t security_module_version;
    collector_collection_t *collector_collection_ptr;

    uint8_t *message_buffer;
    size_t message_buffer_size;
#ifdef ASC_DYNAMIC_MEMORY_ENABLED
    bool message_allocated;
#endif
    bool message_empty;

    serializer_t *serializer;
    unsigned long init_random_collect_offset;
    unsigned long nearest_collect_time;
    event_loop_timer_handler h_collect;
    event_loop_timer_handler h_start;
    notifier_t security_module_state_notifier;
} core_t;

OBJECT_POOL_DECLARATIONS(core_t)
OBJECT_POOL_DEFINITIONS(core_t, CORE_OBJECT_POOL_COUNT)

typedef struct {
    unsigned long minimum;
    unsigned long curr_time;
    bool is_in_start;
} calc_nearest_collect_t;

static asc_result_t _cm_init(component_id_t id);
static asc_result_t _cm_deinit(component_id_t id);
static asc_result_t _cm_start(component_id_t id);
static asc_result_t _cm_stop(component_id_t id);
static component_ops_t _ops = {
    .init = _cm_init,
    .deinit = _cm_deinit,
    .start = _cm_start,
    .stop = _cm_stop,
};

COMPONENTS_FACTORY_DEFINITION(CollectorsCore, &_ops)

static void _core_deinit(core_t *core_ptr, component_id_t id);
static asc_result_t _set_next_collect_timer(core_t *core_ptr, unsigned long curr_time, bool is_in_start);
static void _security_module_state_cb(notifier_t *notifier, int message_num, void *payload);

static void _init_random_collect_offset(core_t *core_ptr)
{
    unsigned long now = itime_time(NULL);
    if (now == ITIME_FAILED) {
        log_error("Error get current time");
        now = 0;
    }
#ifdef ASC_FIRST_FORCE_COLLECTION_INTERVAL
    #if ASC_FIRST_FORCE_COLLECTION_INTERVAL < 0
        core_ptr->init_random_collect_offset = now;
    #else
        core_ptr->init_random_collect_offset = now + ASC_FIRST_FORCE_COLLECTION_INTERVAL;
    #endif
#else
    core_ptr->init_random_collect_offset = now + (unsigned long)(irand_int() % (2 * ASC_FIRST_COLLECTION_INTERVAL) + ASC_FIRST_COLLECTION_INTERVAL);
#endif
}

static unsigned long _calculate_collector_time_offset(core_t *core_ptr, collector_t *collector_ptr, component_state_enum_t state)
{
    unsigned long offset, base_collecting_time = core_ptr->init_random_collect_offset;

    // TODO it breaks sequence of collections and cause to increasing of security messages - need to sync on init_random_collect_offset
    if (state == COMPONENT_RUNNING) {
        unsigned long now = itime_time(NULL);
        if (now == ITIME_FAILED) {
            log_error("Error get current time");
            now = 0;
        }
        base_collecting_time = now + ASC_FIRST_COLLECTION_INTERVAL;
    }
    if (collector_ptr->internal.interval > core_ptr->init_random_collect_offset) {
        // should never happens
        offset = base_collecting_time;
    } else {
        if (base_collecting_time >= collector_ptr->internal.interval) {
            offset = base_collecting_time - collector_ptr->internal.interval;
        } else {
            // should never happens
            offset = base_collecting_time;
        }
    }
    return offset;
}

static core_t *core_init(component_id_t id)
{
    asc_result_t result = ASC_RESULT_OK;
    core_t *core_ptr = NULL;

    core_ptr = object_pool_get(core_t);
    if (core_ptr == NULL) {
        log_error("Failed to init core");
        result = ASC_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }
    memset(core_ptr, 0, sizeof(core_t));
    core_ptr->nearest_collect_time = CORE_NEAREST_TIMER_UNSET_VAL;

    core_ptr->security_module_id = os_utils_get_security_module_id();
    if (core_ptr->security_module_id == NULL) {
        log_error("Failed to retrieve security module id");
        result = ASC_RESULT_EXCEPTION;
        goto cleanup;
    }

    core_ptr->security_module_version = SECURITY_MODULE_VERSION;

    core_ptr->collector_collection_ptr = collector_collection_init();
    if (core_ptr->collector_collection_ptr == NULL) {
        log_error("Failed to init core collectors");
        result = ASC_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    core_ptr->serializer = serializer_init();
    if (core_ptr->serializer == NULL) {
        log_error("Failed to init serializer");
        result = ASC_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    result = serializer_message_begin(core_ptr->serializer, core_ptr->security_module_id, core_ptr->security_module_version);

#ifdef ASC_DYNAMIC_MEMORY_ENABLED
    core_ptr->message_allocated = false;
#endif
    core_ptr->message_empty = true;
    _init_random_collect_offset(core_ptr);

    core_ptr->security_module_state_notifier.notify = _security_module_state_cb;
    result = notifier_subscribe(NOTIFY_TOPIC_SECURITY_MODULE_STATE, &core_ptr->security_module_state_notifier);

cleanup:
    if (result != ASC_RESULT_OK) {
        log_error("Failed to init client core_t");
        _core_deinit(core_ptr, id);
        core_ptr = NULL;
    }

    return core_ptr;
}

static void _core_deinit(core_t *core_ptr, component_id_t id)
{
    if (core_ptr != NULL) {
        notifier_unsubscribe(NOTIFY_TOPIC_SECURITY_MODULE_STATE, &core_ptr->security_module_state_notifier);
        if (core_ptr->collector_collection_ptr != NULL) {
            collector_collection_deinit(core_ptr->collector_collection_ptr);
        }

#ifdef ASC_DYNAMIC_MEMORY_ENABLED
        if (core_ptr->message_allocated) {
            free(core_ptr->message_buffer);
            core_ptr->message_buffer = NULL;
        }
#endif
        serializer_deinit(core_ptr->serializer);

        core_ptr->security_module_id = NULL;
        core_ptr->collector_collection_ptr = NULL;

        object_pool_free(core_t, core_ptr);
        core_ptr = NULL;
    }
    components_manager_set_self_ctx(NULL);
}

static void _start_stop_all_collectors(collector_t *collector_ptr, void *context)
{
    bool do_start = *(bool*)context;

    component_id_t collector_id = components_manager_get_id(collector_ptr->internal.type);
    component_id_t core_id = components_manager_get_self_id();

    if (do_start) {
        components_manager_start(collector_id, core_id);
    } else {
        components_manager_stop(collector_id, core_id, true, false);
    }
}

#ifdef ASC_COMPONENT_CORE_SUPPORTS_RESTART
static void _reset_last_collected_timestamp(collector_t *collector_ptr, void *context)
{
    core_t *core_ptr = context;

    unsigned long offset = _calculate_collector_time_offset(core_ptr, collector_ptr, COMPONENT_STOPED);

    collector_set_last_collected_timestamp(collector_ptr, offset);
}

static void cb_do_start(event_loop_timer_handler h, void *ctx)
{
    core_t *core_ptr = ctx;
    ievent_loop_t *el = ievent_loop_get_instance();

    el->timer_delete(h);

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return;
    }
    bool do_start = true;
    component_id_t core_id = components_manager_get_self_id();
    _init_random_collect_offset(core_ptr);
    collector_collection_foreach(core_ptr->collector_collection_ptr, _reset_last_collected_timestamp, core_ptr);
    collector_collection_foreach(core_ptr->collector_collection_ptr, _start_stop_all_collectors, &do_start);
    components_manager_start(core_id, core_id);
}
#endif

static void _start_stop_collection(bool do_start, bool restart)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return;
    }
    component_id_t core_id = components_manager_get_self_id();

#ifdef ASC_COMPONENT_CORE_SUPPORTS_RESTART
    if (restart) {
        bool restart_do_start = false;
        collector_collection_foreach(core_ptr->collector_collection_ptr, _start_stop_all_collectors, &restart_do_start);
        components_manager_stop(core_id, core_id, false, false);
        core_message_deinit();
        ievent_loop_t *el = ievent_loop_get_instance();
        el->timer_delete(core_ptr->h_start);
        core_ptr->h_start = el->timer_create(cb_do_start, core_ptr, CORE_RESTART_SLEEP, 0, &core_ptr->h_start);
    } else
#endif
    {
        collector_collection_foreach(core_ptr->collector_collection_ptr, _start_stop_all_collectors, &do_start);

        if (do_start) {
            components_manager_start(core_id, core_id);
        } else {
            components_manager_stop(core_id, core_id, false, false);
        }
    }
}

static void _security_module_state_cb(notifier_t *notifier, int message_num, void *payload)
{
    switch (message_num)
    {
    case NOTIFY_SECURITY_MODULE_CONNECTED:
        _start_stop_collection(true, false);
        log_info("Security Module inserted to 'Connected' state");
        break;
#ifdef ASC_COMPONENT_CORE_SUPPORTS_RESTART
    case NOTIFY_SECURITY_MODULE_RESTART:
        _start_stop_collection(true, true);
        log_info("Security Module inserted to 'Connected' state with restart");
        break;
#endif
    case NOTIFY_SECURITY_MODULE_PENDING:
        _start_stop_collection(true, false);
        log_info("Security Module inserted to 'Pending' state");
        break;
    case NOTIFY_SECURITY_MODULE_SUSPENDED:
        _start_stop_collection(false, false);
        log_info("Security Module inserted to 'Suspended' state");
        break;
    default:
        log_error("Unsupported Security Module state");
        break;
    }
}

static asc_result_t core_collect(core_t *core_ptr, unsigned long now)
{
    asc_result_t result = ASC_RESULT_OK;
    bool at_least_one_success = false;
    bool time_passed = false;

    if (now == ITIME_FAILED) {
        log_error("Error get current time");
        return ASC_RESULT_IMPOSSIBLE;
    }

    core_message_deinit();

    for (priority_collectors_t *prioritized_collectors = collector_collection_get_head_priority(core_ptr->collector_collection_ptr);
            prioritized_collectors != NULL;
            prioritized_collectors = collector_collection_get_next_priority(core_ptr->collector_collection_ptr, prioritized_collectors)
        ) {
        linked_list_t *collector_list = priority_collectors_get_list(prioritized_collectors);
        collector_t *current_collector = NULL;
        linked_list_foreach(collector_list, current_collector)
        {
            component_info_t *info = components_manager_get_info(components_manager_get_id(current_collector->internal.type));
            if (!info || info->state != COMPONENT_RUNNING) {
                continue;
            }

            unsigned long last_collected = collector_get_last_collected_timestamp(current_collector);
            unsigned long interval = current_collector->internal.interval;
            unsigned long delta;
            if (now > last_collected) {
                delta = now - last_collected;
            } else {
                log_error("Current time is less than last collected interval");
                delta = interval;
            }

            if (delta >= interval) {
                time_passed = true;
                result = collector_serialize_events(current_collector, core_ptr->serializer);
                // overwrite last collected time with common start value to store sequence of collections
                collector_set_last_collected_timestamp(current_collector, now);
                if (result == ASC_RESULT_EMPTY) {
                    log_debug("empty, collector type=[%d]", current_collector->internal.type);
                    continue;
                } else if (result != ASC_RESULT_OK) {
                    log_error("failed to collect, collector type=[%d]", current_collector->internal.type);
                    goto error;
                }
                at_least_one_success = true;
                core_ptr->message_empty = false;
            }
        }
    }

    return (!time_passed || at_least_one_success) ? ASC_RESULT_OK : result;

error:
    // In case of serializer failure, it is unsafe to keep building the message
    core_message_deinit();

    return ASC_RESULT_EXCEPTION;
}

static void _collector_collection_calc_nearest_collect(collector_t *collector_ptr, void *context)
{
    calc_nearest_collect_t *ctx = (calc_nearest_collect_t *)context;
    component_info_t *info = components_manager_get_info(components_manager_get_id(collector_ptr->internal.type));

    /* Exclude failed collectors from nearest collect time calculation */
    if (!info) {
        return;
    }
    if (!ctx->is_in_start) {
        if (info->state != COMPONENT_RUNNING) {
            return;
        }
    } else { /* If we are in core start function take in account also subscribed collectors */
        if (info->state != COMPONENT_RUNNING && info->state != COMPONENT_SUBSCRIBED) {
            return;
        }
    }
    
    unsigned long curr_time = ctx->curr_time;
    unsigned long last_collected = collector_get_last_collected_timestamp(collector_ptr);
    unsigned long interval = collector_ptr->internal.interval;
    unsigned long next_time;

    if (last_collected > curr_time) {
        next_time = last_collected - curr_time + interval;
    } else {
        unsigned long delta = curr_time - last_collected;
        if (interval <= delta) {
            next_time = 0;
        } else {
            next_time = interval - delta;
        }
    }

    if (next_time < ctx->minimum) {
        ctx->minimum = next_time;
    }
}

static void cb_collect(event_loop_timer_handler h, void *ctx)
{
    core_t *core_ptr = ctx;
    ievent_loop_t *el = ievent_loop_get_instance();

    el->timer_delete(h);

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return;
    }
    unsigned long curr_time = itime_time(NULL);

    if (core_collect(core_ptr, curr_time) == ASC_RESULT_OK) {
        notifier_notify(NOTIFY_TOPIC_COLLECT, NOTIFY_MESSAGE_READY, NULL);
    }
    _set_next_collect_timer(core_ptr, curr_time, false);
}

static asc_result_t _set_next_collect_timer(core_t *core_ptr, unsigned long now, bool is_in_start)
{
    ievent_loop_t *el = ievent_loop_get_instance();
    calc_nearest_collect_t ctx;

    // stop existing timer
    el->timer_delete(core_ptr->h_collect);

    if (now == ITIME_FAILED) {
        log_error("Error get current time");
        core_ptr->h_collect = el->timer_create(cb_collect, core_ptr, ASC_HIGH_PRIORITY_INTERVAL, 0, &core_ptr->h_collect);
        core_ptr->nearest_collect_time = ASC_HIGH_PRIORITY_INTERVAL;
        return ASC_RESULT_OK;
    }

    ctx.minimum = CORE_NEAREST_TIMER_UNSET_VAL;
    ctx.curr_time = now;
    ctx.is_in_start = is_in_start;

    collector_collection_foreach(core_ptr->collector_collection_ptr, _collector_collection_calc_nearest_collect, &ctx);

    if (ctx.minimum == CORE_NEAREST_TIMER_UNSET_VAL) {
        log_info("the list of collectors is empty");
        ctx.minimum = ASC_HIGH_PRIORITY_INTERVAL;
        core_ptr->h_collect = el->timer_create(cb_collect, core_ptr, ctx.minimum, 0, &core_ptr->h_collect);
    } else {
        log_debug("the nearest collection interval=[%lu]", ctx.minimum);
        core_ptr->h_collect = el->timer_create(cb_collect, core_ptr, ctx.minimum, 0, &core_ptr->h_collect);
    }

    core_ptr->nearest_collect_time = ctx.minimum;

    return ASC_RESULT_OK;
}

/* API Functions */
asc_result_t core_message_get(security_message_t* security_message_ptr)
{
    asc_result_t result = ASC_RESULT_OK;
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL || security_message_ptr == NULL) {
        result = ASC_RESULT_MEMORY_EXCEPTION;
        log_error("core uninitialized");
        goto cleanup;
    }

    if (core_ptr->message_empty) {
        result = ASC_RESULT_EMPTY;
        log_debug("message empty");
        goto cleanup;
    }

    if (serializer_get_state(core_ptr->serializer) == SERIALIZER_STATE_MESSAGE_PROCESSING &&
        serializer_message_end(core_ptr->serializer) != ASC_RESULT_OK) {
        result = ASC_RESULT_EXCEPTION;
        log_error("failed to end message");
        goto cleanup;
    }

    if (core_ptr->message_buffer == NULL) {
        result = serializer_buffer_get(core_ptr->serializer, &core_ptr->message_buffer, &core_ptr->message_buffer_size);
        if (result != ASC_RESULT_OK && result != ASC_RESULT_IMPOSSIBLE) {
            log_error("failed in serializer_buffer_get");
            goto cleanup;
        }

        if (result == ASC_RESULT_IMPOSSIBLE) {
#ifndef ASC_DYNAMIC_MEMORY_ENABLED
            log_error("failed in serializer_buffer_get, message too big");
            result = ASC_RESULT_EXCEPTION;
            goto cleanup;
#else /* ASC_DYNAMIC_MEMORY_ENABLED */
            result = ASC_RESULT_OK;
            log_debug("failed in serializer_buffer_get on first attempt, re-allocating buffer...");
            if (serializer_buffer_get_size(core_ptr->serializer, &core_ptr->message_buffer_size) != ASC_RESULT_OK) {
                result = ASC_RESULT_EXCEPTION;
                log_error("failed in serializer_buffer_get_size");
                goto cleanup;
            }

            core_ptr->message_buffer = (uint8_t*)malloc(core_ptr->message_buffer_size);
            if (core_ptr->message_buffer == NULL) {
                result = ASC_RESULT_MEMORY_EXCEPTION;
                log_error("failed to allocate message buffer");
                goto cleanup;
            }

            core_ptr->message_allocated = true;

            if (serializer_buffer_get_copy(core_ptr->serializer, core_ptr->message_buffer, core_ptr->message_buffer_size) != ASC_RESULT_OK) {
                result = ASC_RESULT_EXCEPTION;
                log_error("failed in serializer_buffer_get_copy");
                goto cleanup;
            }
            log_debug("re-allocating buffer done successfully");
#endif /* ASC_DYNAMIC_MEMORY_ENABLED */
        }
    }

    // set security message properties
    security_message_ptr->data = core_ptr->message_buffer;
    security_message_ptr->size = core_ptr->message_buffer_size;

cleanup:
    if (result == ASC_RESULT_EXCEPTION || result == ASC_RESULT_MEMORY_EXCEPTION) {
        core_message_deinit();
    }

    return result;
}

asc_result_t core_collector_register(collector_t *collector_ptr)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    component_state_enum_t state = components_manager_get_info(components_manager_get_self_id())->state;
    unsigned long offset = _calculate_collector_time_offset(core_ptr, collector_ptr, state);

    asc_result_t result = collector_collection_register(core_ptr->collector_collection_ptr, collector_ptr, offset);

    if (state == COMPONENT_RUNNING) {
        _set_next_collect_timer(core_ptr, itime_time(NULL), false);
    }
    return result;
}

asc_result_t core_collector_unregister(collector_t *collector_ptr)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return ASC_RESULT_MEMORY_EXCEPTION;
    }

    collector_collection_unregister(core_ptr->collector_collection_ptr, collector_ptr);

    component_state_enum_t state = components_manager_get_info(components_manager_get_self_id())->state;

    if (state == COMPONENT_RUNNING) {
        _set_next_collect_timer(core_ptr, itime_time(NULL), false);
    }
    return ASC_RESULT_OK;
}

asc_result_t core_message_deinit(void)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return ASC_RESULT_MEMORY_EXCEPTION;
    }

#ifdef ASC_DYNAMIC_MEMORY_ENABLED
    if (core_ptr->message_allocated) {
        free(core_ptr->message_buffer);
        core_ptr->message_allocated = false;
    }
#endif

    if (serializer_reset(core_ptr->serializer) != ASC_RESULT_OK) {
        log_error("failed in serializer_reset");
        return ASC_RESULT_EXCEPTION;
    }

    if (serializer_message_begin(core_ptr->serializer, core_ptr->security_module_id, core_ptr->security_module_version) != ASC_RESULT_OK) {
        log_error("failed in serializer_message_begin");
        return ASC_RESULT_EXCEPTION;
    }

    core_ptr->message_buffer = NULL;
    core_ptr->message_buffer_size = 0;
    core_ptr->message_empty = true;

    return ASC_RESULT_OK;
}

unsigned long core_get_init_random_collect_offset(void)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return 0;
    }
    return core_ptr->init_random_collect_offset;
}

unsigned long core_get_nearest_collect_time(void)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return 0;
    }
    return core_ptr->nearest_collect_time;
}

collector_collection_t *core_get_collector_collection(void)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return NULL;
    }
    return core_ptr->collector_collection_ptr;
}

/* OPS Functions */
static asc_result_t _cm_init(component_id_t id)
{
    core_t *ptr = core_init(id);

    if (ptr == NULL) {
        components_manager_set_self_ctx(NULL);
        return ASC_RESULT_EXCEPTION;
    }
    components_manager_set_self_ctx(ptr);

    return ASC_RESULT_OK;
}

static asc_result_t _cm_deinit(component_id_t id)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return ASC_RESULT_MEMORY_EXCEPTION;
    }

    _core_deinit(core_ptr, id);
    return ASC_RESULT_OK;
}

asc_result_t _cm_stop(component_id_t id)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    ievent_loop_t *el = ievent_loop_get_instance();
    el->timer_delete(core_ptr->h_start);
    el->timer_delete(core_ptr->h_collect);
    core_ptr->nearest_collect_time = CORE_NEAREST_TIMER_UNSET_VAL;

    return ASC_RESULT_OK;
}

static asc_result_t _cm_start(component_id_t id)
{
    core_t *core_ptr = components_manager_get_self_ctx();

    if (core_ptr == NULL) {
        log_error("core uninitialized");
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    ievent_loop_t *el = ievent_loop_get_instance();
    el->timer_delete(core_ptr->h_start);
    /* Last parameter is 'true' because we do not want to base on sequence so we taking in account all registered collectors here */
    return _set_next_collect_timer(core_ptr, itime_time(NULL), true);
}
