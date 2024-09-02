
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
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/collection/list.h"
#include "asc_security_core/utils/itime.h"

#include "asc_security_core/utils/event_loop_be.h"

/** @brief An opaque structure representing a timer. */
typedef struct event_timer event_timer_t;

/** @brief Flag to know if we are in timer callback to avoid inserting to another timer callbacks that where created
*          in current loop.
*/
static bool _in_timer_cb;

struct event_timer {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(struct event_timer);
    unsigned long delay;
    unsigned long repeat;
    event_loop_timer_cb_t cb;
    event_timer_t **self;
    void *ctx;
    bool added_in_cb;
};

OBJECT_POOL_DECLARATIONS(event_timer_t)
OBJECT_POOL_DEFINITIONS(event_timer_t, OBJECT_POOL_BE_EVENT_LOOP_TIMERS_COUNT)


static linked_list_t _event_timer_linked_list;
static bool _event_loop_initialized;
static bool _event_loop_is_running;

static bool _run_until(int max_count);
static void _timer_del(event_timer_t *t);
// #define DEBUG_BE 1
#if (DEBUG_BE) && (ASC_LOG_LEVEL == LOG_LEVEL_DEBUG)
static void _timers_list_debug_print(void)
{
    event_timer_t *curr = NULL;

    log_error("Timers total number %zu", linked_list_get_size(&_event_timer_linked_list));
    linked_list_foreach(&_event_timer_linked_list, curr) {
        log_debug("timer=[%p] with delay=[%lu]", (void *)curr, curr->delay);
    }
}
#else
#define _timers_list_debug_print()
#endif

static bool _be_init(void)
{
    _event_loop_initialized = true;
    _event_loop_is_running = true;
    /* Default deinit function is not set, because in case of periodic timer we will want to remove timer without free. */
    linked_list_init(&_event_timer_linked_list);
    return true;
}

static bool _be_deinit(bool flush)
{
    if (!_event_loop_initialized) {
        return true;
    }
    bool flush_result = true;
    if (flush) {
        flush_result = _run_until(EVENT_LOOP_DEFAULT_MAX_COUNT_RUN_UNTIL);
    }
    event_timer_t *curr;
    while((curr = linked_list_get_first(&_event_timer_linked_list))) {
        _timer_del(curr);
    }
    _event_loop_initialized = false;
    if (!flush_result) {
        log_warn("There are active and referenced handles or requests on event loop");
    }
    return flush_result;
}

static void _timers_reset_added_in_run()
{
    event_timer_t *curr = NULL;
    linked_list_foreach(&_event_timer_linked_list, curr) {
        curr->added_in_cb = false;
    }
}

static void _timer_del(event_timer_t *t)
{
#ifdef DEBUG_BE
    log_debug("deleting timer=[%p]", (void *)t);
#endif
    if (t) {
        event_timer_t **self = t->self;
#ifdef DEBUG_BE
        log_debug("deleting timer=[%p] delay=[%lu] repeat=[%lu] ", (void *)t, t->delay, t->repeat);
#endif

        linked_list_remove(&_event_timer_linked_list, t, NULL);
        object_pool_free(event_timer_t, t);

        if (self) {
            *self = NULL;
        }
    }
    _timers_list_debug_print();
}

static event_timer_t *_timer_add(event_timer_t *t, event_loop_timer_cb_t cb, void *ctx, unsigned long delay, unsigned long repeat, event_timer_t **self)
{
    event_timer_t *curr = NULL, *new;
    bool is_periodic = !!t;
    unsigned long now = itime_time(NULL);
    if (now == ITIME_FAILED) {
        log_error("Error get current time");
        now = 0;
    }

#ifdef DEBUG_BE
    log_debug("is_periodic=[%d]", is_periodic);
#endif
    
    if (is_periodic) {
        new = t;
    } else {
        if (!(new = object_pool_get(event_timer_t))) {
            log_error("Failed to allocate timer struct.");
            goto cleanup;
        }
    }
    new->delay = delay + now;
    new->repeat = repeat;
    new->ctx = ctx;
    new->self = self;
    new->cb = cb;
    new->added_in_cb = _in_timer_cb;

    linked_list_foreach(&_event_timer_linked_list, curr) {
        if (new->delay < curr->delay) {
            break;
        }
    }

    if (linked_list_insert_before(&_event_timer_linked_list, curr, new) != new) {
        log_error("Failed to insert timer to queue.");
        _timer_del(new);
        new = NULL;
        goto cleanup;
    }
#ifdef DEBUG_BE
    log_debug("added timer=[%p] delay=[%lu] offset=[%lu] repeat=[%lu] from preiodic=[%d]", (void *)new, delay, new->delay, new->repeat, is_periodic);
#endif
    _timers_list_debug_print();

cleanup:
    return new;
}

static void _be_timer_wrapper(event_timer_t *t)
{
    unsigned long repeat = t->repeat;

    linked_list_remove(&_event_timer_linked_list, t, NULL);

    // Add timer to next period so in parameter 'delay' we will set 'repeat' value
    if (repeat && _event_loop_is_running) {
        _timer_add(t, t->cb, t->ctx, t->repeat, t->repeat, t->self);
    }
#ifdef DEBUG_BE
    log_debug("calling timer=[%p] delay=[%lu] repeat=[%lu] ", (void *)t, t->delay, t->repeat);
#endif

    t->cb((event_loop_timer_handler)t, t->ctx);
}

static bool _run_once(void)
{
    event_timer_t *t;
    unsigned long now = itime_time(NULL);
    if (now == ITIME_FAILED) {
        log_error("Error get current time");
        now = 0;
    }

#ifdef DEBUG_BE
    log_debug("calling timers total count=[%zu]", linked_list_get_size(&_event_timer_linked_list));
#endif
    _timers_list_debug_print();

    /* stop() might be called in one of callbacks, so need to check it on each iteration.
     * _event_timer_linked_list might change on each call, so always get first.
     */
    _in_timer_cb = true;

    while(_event_loop_initialized && (t = linked_list_get_first(&_event_timer_linked_list))) {
        // we are adding timer to the end of same sorted sequence so if we met t->added_in_cb = true we can break
        if (!t->added_in_cb && t->delay <= now) {
            _be_timer_wrapper(t);
        } else {
            break;
        }
    }
    _in_timer_cb = false;
    _timers_reset_added_in_run();
    _timers_list_debug_print();

    return _event_loop_is_running;
}

static bool _run_until(int max_count)
{
    int cnt = 0;
    while (cnt++ < max_count) {
        if (!linked_list_get_size(&_event_timer_linked_list)) {
            return true;
        }
        _run_once();
    }
    return false;
}

static void _stop(void)
{
    _timers_list_debug_print();
    _event_loop_is_running = false;
}

static event_loop_timer_handler _timer_create(event_loop_timer_cb_t cb, void *ctx, unsigned long delay, unsigned long repeat, event_loop_timer_handler *self)
{
    if (!_event_loop_initialized) {
        return (event_loop_timer_handler)NULL;
    }
    return (event_loop_timer_handler)_timer_add(NULL, cb, ctx, delay, repeat, (event_timer_t **)self);
}

static void _timer_delete(event_loop_timer_handler handler)
{
    if (handler) {
        _timer_del((event_timer_t *)handler);
    }
}

ievent_loop_t *event_loop_be_instance_attach(void)
{
    static ievent_loop_t event_loop = {
        .init = _be_init,
        .deinit = _be_deinit,
        .run = NULL,
        .run_once = _run_once,
        .run_until = _run_until,
        .stop = _stop,
        .signal_create = NULL,
        .signal_delete = NULL,
        .timer_create = _timer_create,
        .timer_delete = _timer_delete,
    };

    return &event_loop;
}
