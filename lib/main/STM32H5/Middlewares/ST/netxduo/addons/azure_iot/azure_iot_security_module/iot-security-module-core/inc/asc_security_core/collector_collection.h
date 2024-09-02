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

#ifndef IOTSECURITY_COLLECTOR_COLLECTION_H
#define IOTSECURITY_COLLECTOR_COLLECTION_H

#include <stdint.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"
#include "asc_security_core/collector.h"
#include "asc_security_core/utils/collection/list.h"

typedef struct priority_collectors priority_collectors_t;
typedef struct collector_collection collector_collection_t;


/**
 * @brief Initialize collector collection
 *
 * @return A @c collector_collection_t* representing the newly created collector collection.
 */
collector_collection_t *collector_collection_init(void);


/**
 * @brief Deinitialize collector collection
 *
 * @param collector_collection_ptr    representing the collector collection.
 */
void collector_collection_deinit(collector_collection_t *collector_collection_ptr);

/**
 * @brief Register collector to collector_collection
 *
 * @param collector_collection_ptr  the collector colleciton
 * @param collector_ptr             a @c collector_t* collector handle
 * @param random_collect_offset     a random start collection offset
 * 
 * @return  ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t collector_collection_register(collector_collection_t *collector_collection_ptr, collector_t *collector_ptr, unsigned long collect_offset);

/**
 * @brief Unregister collector from collector_collection
 *
 * @param collector_collection_ptr  the collector colleciton
 * @param collector_ptr             a @c collector_t* collector handle
 */
void collector_collection_unregister(collector_collection_t *collector_collection_ptr, collector_t *collector_ptr);

/**
 * @brief Return CollectorCollection specific priority collection
 *
 * @param collector_collection_ptr   the collector colleciton
 * @param collector_priority            the priority to retreive
 *
 * @return A @c priority_collectors_t * which stands for the specific priority collectors
 */
priority_collectors_t *collector_collection_get_by_priority(collector_collection_t *collector_collection_ptr, collector_priority_t collector_priority);


/**
 * @brief Return CollectorCollection head priority collection
 *
 * @param collector_collection_ptr   the collector colleciton
 * @param collector_collection          the current priority collection
 *
 * @return A @c priority_collectors_t * which stands for the first and highest priority collectors
 */
priority_collectors_t *collector_collection_get_head_priority(collector_collection_t *collector_collection_ptr);


/**
 * @brief Return next priority
 *
 * @param current_priority  out param
 *
 * @return A @c priority_collectors_t * which stands for the next priority collectors
 */
priority_collectors_t *collector_collection_get_next_priority(collector_collection_t *collector_collection_ptr, priority_collectors_t *priority_collectors_ptr);


/**
 * @brief Return a collector handle with the same name
 *
 * @param type  The type of the collecotr
 *
 * @return A @c collector_t* collector handle with the same type
 */
collector_t *collector_collection_get_collector_by_priority(collector_collection_t *collector_collection_ptr, collector_enum_t type);


/**
 * @brief traverse through the collection and calls action_function for each collector
 *
 * @param action_function   a predicate to call to for each collector
 * @param context           caller context
 */
void collector_collection_foreach(collector_collection_t *collector_collection_ptr, void(*action_function)(collector_t *collector, void *context), void *context);

/**
 * @brief returns the list of collectors of the priority collection
 *
 * @param priority_collectors_ptr    the priority collection
 *
 * @return A @c linked_list_t which stands for the list of collectors of the priority collection
 */
linked_list_t *priority_collectors_get_list(priority_collectors_t *priority_collectors_ptr);


/**
 * @brief returns the priority of the priority collection
 *
 * @param priority_collectors_ptr  the priority collection
 *
 * @return A @c COLLECTOR_PRIORITY which stands for the priority of the priority collection
 */
collector_priority_t priority_collectors_get_priority(priority_collectors_t *priority_collectors_ptr);


/**
 * @brief returns the priority of the current collector collection
 *
 * @param priority_collectors_ptr    the priority collection
 *
 * @return A @c collector_t* which stands for the current collector of the priority collection
 */
collector_t *priority_collectors_get_current_collector(priority_collectors_t *priority_collectors_ptr);


/**
 * @brief sets the current collector iterator
 *
 * @param priority_collectors_ptr    the priority collection
 * @param collector_ptr              the collector to be replaced as current
 */
void priority_collectors_set_current_collector(priority_collectors_t *priority_collectors_ptr, collector_t *collector_ptr);


/**
 * @brief searches for the next non-empty collector
 *
 * @param priority_collectors_ptr    the priority collection
 *
 * This function iterates through the priority collection and goes to the begining in case it reached the end
 *
 * @return A @c collector_t* indicating the position of the next cyclic collector.
 */
collector_t *priority_collectors_get_next_cyclic_collector(priority_collectors_t *priority_collectors_ptr);

#endif /* IOTSECURITY_COLLECTOR_COLLECTION_H */