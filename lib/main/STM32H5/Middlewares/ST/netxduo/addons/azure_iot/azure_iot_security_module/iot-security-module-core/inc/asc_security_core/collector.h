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

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <assert.h>
#include <stdbool.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"
#include "asc_security_core/component_info.h"

#include "asc_security_core/object_pool.h"
#include "asc_security_core/serializer.h"

#define COLLECTOR_OBJECT_POOL_COUNT COLLECTORS_COUNT

typedef struct collector_internal_t collector_internal_t;

/**
 * @brief Initialize the collector internal
 *
 * @param collector_internal_ptr   A handle to the collector internal to initialize.
 *
 * @return ASC_RESULT_OK on success
 */
typedef asc_result_t (*collector_init_function_t)(collector_internal_t *collector_internal_ptr);

/**
 * @brief Serialize events from the collector
 *
 * @param collector_internal_ptr    A handle to the collector internal.
 * @param serializer                The serializer the collector should use.
 *
 * @return  ASC_RESULT_OK on success
 *          ASC_RESULT_EMPTY when there are no events to serialize. In that case, serializer remains unchanged.
 *          ASC_RESULT_EXCEPTION otherwise
 */
typedef asc_result_t (*collector_serialize_function_t)(collector_internal_t *collector_internal_ptr, serializer_t *serializer);

typedef enum {
    COLLECTOR_PRIORITY_HIGH = 0,
    COLLECTOR_PRIORITY_MEDIUM = 1,
    COLLECTOR_PRIORITY_LOW = 2,
    COLLECTOR_PRIORITY_COUNT = 3
} collector_priority_t;

struct collector_internal_t {
    collector_enum_t type;
    collector_priority_t priority;
    unsigned long interval;

    collector_serialize_function_t collect_function;
    void *state;
};

typedef enum {
    COLLECTOR_STATUS_OK,
    COLLECTOR_STATUS_EXCEPTION
} collector_status_t;

/**
 * @struct collector_t
 * @brief collector struct
 *        base class for all collectors
 *
 */
typedef struct collector_t {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(struct collector_t);

    /*@{*/
    bool disabled; /**< Indicates if the collector is disabled. */
    collector_status_t status;
    unsigned int failure_count;
    /*@}*/

    /**
    * @name Timestamps
    */
    /*@{*/
    unsigned long last_collected_timestamp;
    unsigned long last_sent_timestamp;
    /*@}*/

    collector_internal_t internal;
} collector_t;

OBJECT_POOL_DECLARATIONS(collector_t)

/**
 * @brief Default allocate and initialize collector
 *
 * @param id                    The component ID provided by component manager on init function
 * @param type                  The collector type
 * @param priority              The collector priority
 * @param event_queue_max_size  The event queue max capacity
 * @param collect_function      The collect function of the collector
 * @param interval              The interval of sending security messages
 * @param state                 The initial state value
 *
 * @return ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t collector_default_create(component_id_t id, collector_enum_t type, collector_priority_t priority, collector_serialize_function_t collect_function, unsigned long interval, void *state);

/**
 * @brief Collector priority getter
 *
 * @param collector_ptr  collector ptr
 *
 * @return Collector priority
 */
collector_priority_t collector_get_priority(collector_t *collector_ptr);

/**
 * @brief Collector last collected timestamp getter
 *
 * @param collector_ptr  collector ptr
 *
 * @return Collector last collected timestamp
 */
unsigned long collector_get_last_collected_timestamp(collector_t *collector_ptr);

/**
 * @brief Collector last collected timestamp setter
 *
 * @param collector_ptr             collector_t*
 * @param last_collected_timestamp  the timestamp
 *
 * @return ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t collector_set_last_collected_timestamp(collector_t *collector_ptr, unsigned long last_collected_timestamp);

/**
 * @brief Serialize the events in the collector
 *
 * @param collector_ptr     The collector handle
 * @param serializer        The serializer to use
 *
 * @return  ASC_RESULT_OK on success
 *          ASC_RESULT_EMPTY when there are no events. In that case, serializer will be unchanged.
 *          ASC_RESULT_EXCEPTION otherwise
 */
asc_result_t collector_serialize_events(collector_t *collector, serializer_t *serializer);

/**
 * @brief Default collector subscribe to core function
 *
 * @param id     The component ID provided by component manager
 *
 * @return  ASC_RESULT_OK on success, ASC_RESULT_MEMORY_EXCEPTION otherwise
 */
asc_result_t collector_default_subscribe(component_id_t id);

/**
 * @brief Default collector unsubscribe from core function
 *
 * @param id     The component ID provided by component manager
 *
 * @return  ASC_RESULT_OK on success, ASC_RESULT_MEMORY_EXCEPTION otherwise
 */
asc_result_t collector_default_unsubscribe(component_id_t id);

/**
 * @brief Default collector deinit function
 *
 * @param id     The component ID provided by component manager
 *
 * @return  ASC_RESULT_OK on success, ASC_RESULT_MEMORY_EXCEPTION otherwise
 */
asc_result_t collector_default_deinit(component_id_t id);

/**
 * @brief Default collector start function
 *
 * @param id     The component ID provided by component manager
 *
 * @return  ASC_RESULT_OK
 */
asc_result_t collector_default_start(component_id_t id);

/**
 * @brief Default collector stop function
 *
 * @param id     The component ID provided by component manager
 *
 * @return  ASC_RESULT_OK
 */
asc_result_t collector_default_stop(component_id_t id);

#define COLLECTOR_OPS_DEFINITIONS(__ops, __init, __deinit, __subscribe, __unsubscribe, __start, __stop) \
static component_ops_t _ops##__ops = { \
    .init = __init, \
    .deinit = __deinit, \
    .subscribe = __subscribe, \
    .unsubscribe = __unsubscribe, \
    .start = __start, \
    .stop = __stop, \
}
#endif /* COLLECTOR_H */
