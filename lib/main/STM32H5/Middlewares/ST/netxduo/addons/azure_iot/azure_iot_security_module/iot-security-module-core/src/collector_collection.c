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

#include <stdlib.h>
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/object_pool.h"

#include "asc_security_core/collector_collection.h"

#define COLLECTOR_COLLECTION_OBJECT_POOL_COUNT 1

struct priority_collectors {
    collector_priority_t priority;
    collector_t *current_collector_ptr;

    linked_list_t collector_list;
};

struct collector_collection {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(struct collector_collection);

    priority_collectors_t collector_array[COLLECTOR_PRIORITY_COUNT];
};

OBJECT_POOL_DECLARATIONS(collector_collection_t)
OBJECT_POOL_DEFINITIONS(collector_collection_t, COLLECTOR_COLLECTION_OBJECT_POOL_COUNT)

static void _collector_collection_init_collector_lists(collector_collection_t *collector_collection_ptr);
static void _collector_collection_deinit_collector_lists(linked_list_t *collector_list_ptr);
static bool _collector_collection_type_match_function(void *item, void *match_context);


collector_collection_t *collector_collection_init(void)
{
    log_debug("Init collector collection");
    asc_result_t result = ASC_RESULT_OK;
    collector_collection_t *collector_collection_ptr = NULL;

    collector_collection_ptr = object_pool_get(collector_collection_t);
    if (collector_collection_ptr == NULL) {
        log_error("Failed to initialized collector collection");
        result = ASC_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    memset(collector_collection_ptr, 0, sizeof(collector_collection_t));

    _collector_collection_init_collector_lists(collector_collection_ptr);
    
cleanup:
    if (result != ASC_RESULT_OK) {
        log_error("Failed to initialize collector collection, result=[%d]", result);
        collector_collection_ptr = NULL;
    }

    return collector_collection_ptr;
}


void collector_collection_deinit(collector_collection_t *collector_collection_ptr)
{
    if (collector_collection_ptr == NULL) {
        return;
    }

    for (int priority=0; priority < COLLECTOR_PRIORITY_COUNT; priority++) {
        _collector_collection_deinit_collector_lists(&(collector_collection_ptr->collector_array[priority].collector_list));
        collector_collection_ptr->collector_array[priority].current_collector_ptr = NULL;
    }

    object_pool_free(collector_collection_t, collector_collection_ptr);
    collector_collection_ptr = NULL;
}


priority_collectors_t *collector_collection_get_head_priority(collector_collection_t *collector_collection_ptr)
{
    return &(collector_collection_ptr->collector_array[COLLECTOR_PRIORITY_HIGH]);
}


priority_collectors_t *collector_collection_get_next_priority(collector_collection_t *collector_collection_ptr, priority_collectors_t *priority_collectors_ptr)
{
    uint32_t current_priority = (uint32_t)priority_collectors_ptr->priority + 1;
    if (current_priority == COLLECTOR_PRIORITY_COUNT) {
        return NULL;
    }

    return &(collector_collection_ptr->collector_array[current_priority]);
}


priority_collectors_t *collector_collection_get_by_priority(collector_collection_t *collector_collection_ptr, collector_priority_t collector_priority)
{
    if (collector_priority >= COLLECTOR_PRIORITY_COUNT) {
        return NULL;
    }

    return &(collector_collection_ptr->collector_array[collector_priority]);
}


static bool _collector_collection_type_match_function(void *item, void *match_context)
{
    collector_t *collector_ptr = item;
    return collector_ptr == NULL ? false : collector_ptr->internal.type == *((collector_enum_t *)match_context);
}


collector_t *collector_collection_get_collector_by_priority(collector_collection_t *collector_collection_ptr, collector_enum_t type)
{
    collector_t *collector_ptr = NULL;
    priority_collectors_t *priority_collector_ptr = NULL;

    if (collector_collection_ptr == NULL) {
        goto cleanup;
    } 
    priority_collector_ptr = collector_collection_get_head_priority(collector_collection_ptr);

    while (priority_collector_ptr != NULL) {
        linked_list_t *collector_list = priority_collectors_get_list(priority_collector_ptr);

        collector_ptr = linked_list_find(collector_list, _collector_collection_type_match_function, &type);
        if (collector_ptr != NULL) {
            goto cleanup;
        }

        priority_collector_ptr = collector_collection_get_next_priority(collector_collection_ptr, priority_collector_ptr);
    }

cleanup:
    return collector_ptr;
}


void collector_collection_foreach(collector_collection_t *collector_collection_ptr, void(*action_function)(collector_t *collector, void *context), void *context)
{
    if (action_function == NULL) {
        return;
    }
    for (priority_collectors_t *prioritized_collectors = collector_collection_get_head_priority(collector_collection_ptr) ; prioritized_collectors != NULL; prioritized_collectors = collector_collection_get_next_priority(collector_collection_ptr, prioritized_collectors)) {
        linked_list_t *linked_list_handle = priority_collectors_get_list(prioritized_collectors);
        collector_t *curr = NULL;
        linked_list_foreach(linked_list_handle, curr) {
            action_function(curr, context);
        }
    }
}

collector_priority_t priority_collectors_get_priority(priority_collectors_t *priority_collectors_ptr)
{
    return priority_collectors_ptr->priority;
}


linked_list_t *priority_collectors_get_list(priority_collectors_t *priority_collectors_ptr)
{
    return &(priority_collectors_ptr->collector_list);
}


static void _collector_collection_init_collector_lists(collector_collection_t *collector_collection_ptr)
{
    for (unsigned int priority=0; priority < COLLECTOR_PRIORITY_COUNT; priority++) {
        linked_list_init(&(collector_collection_ptr->collector_array[priority].collector_list));
        collector_collection_ptr->collector_array[priority].current_collector_ptr = NULL;
        collector_collection_ptr->collector_array[priority].priority = (collector_priority_t)priority;
    }
}


collector_t *priority_collectors_get_current_collector(priority_collectors_t *priority_collectors_ptr)
{
    return priority_collectors_ptr->current_collector_ptr;
}


void priority_collectors_set_current_collector(priority_collectors_t *priority_collectors_ptr, collector_t *current_item)
{
    priority_collectors_ptr->current_collector_ptr = current_item;
}


collector_t *priority_collectors_get_next_cyclic_collector(priority_collectors_t *priority_collectors_ptr)
{
    collector_t *current_item = priority_collectors_ptr->current_collector_ptr;

    if (current_item == NULL) {
        return NULL;
    }

    if (current_item->next == NULL) {
        current_item = linked_list_get_first(&priority_collectors_ptr->collector_list);
    } else {
        current_item = current_item->next;
    }

    return current_item;
}


asc_result_t collector_collection_register(collector_collection_t *collector_collection_ptr, collector_t *collector_ptr, unsigned long collect_offset)
{
    asc_result_t result = ASC_RESULT_OK;
    collector_priority_t priority = collector_get_priority(collector_ptr);

    linked_list_t *current_collector_list_handle = &(collector_collection_ptr->collector_array[priority].collector_list);

    if (linked_list_add_last(current_collector_list_handle, collector_ptr) == NULL){
        log_error("Could not append collector type=[%d] to collector list", collector_ptr->internal.type);
        result = ASC_RESULT_EXCEPTION;
        goto cleanup;
    }
    collector_collection_ptr->collector_array[priority].current_collector_ptr = linked_list_get_first(current_collector_list_handle);
    collector_set_last_collected_timestamp(collector_ptr, collect_offset);
cleanup:

    return result;  
}


void collector_collection_unregister(collector_collection_t *collector_collection_ptr, collector_t *collector_ptr)
{
    collector_priority_t priority = collector_get_priority(collector_ptr);

    linked_list_t *current_collector_list_handle = &(collector_collection_ptr->collector_array[priority].collector_list);

    linked_list_remove(current_collector_list_handle, collector_ptr, NULL);
    collector_collection_ptr->collector_array[priority].current_collector_ptr = linked_list_get_first(current_collector_list_handle);
}


static void _collector_collection_deinit_collector_lists(linked_list_t *collector_list_ptr)
{
    collector_t *collector_ptr = linked_list_get_first(collector_list_ptr);
    while (collector_ptr != NULL) {
        linked_list_remove(collector_list_ptr, collector_ptr, NULL);
        collector_ptr = linked_list_get_first(collector_list_ptr);
    }
}