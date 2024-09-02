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

#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>
#include <stddef.h>

#include <asc_config.h>

#include "asc_security_core/utils/collection/collection.h"

/* Pay your attention, that this implementation is not thread safe. */

/**
 * @brief   Typedef of free function
 *
 * @param   item       The item to free
 *
 * @return  none
 */
typedef void (*linked_list_free_cb)(void *item);

/**
 * @brief   Typedef of condition function
 *
 * @param   item       The item to perform the condition
 * @param   condition_input The condition input
 *
 * @return  true on confition is true, otherwise false
 */
typedef bool (*linked_list_condition_cb)(void *item, void *condition_input);

/**
 * @brief   Typedef of updating function
 *
 * @param   a         The old item
 * @param   b         The new item
 *
 * @return  none
 */
typedef void (*linked_list_update_cb)(void *a, void *b);

typedef struct linked_list_t {
    collection_item_t *head;
    collection_item_t *tail;
    size_t size;
    bool initialized;
} linked_list_t;

/* APIs */
/* The void *data parameter for this implementation of linked list MUST be from following type:
typedef struct NAME_OF_DATA_FOR_LINKED_LIST_t {
    // This macro must be first in object 
    COLLECTION_INTERFACE(struct NAME_OF_DATA_FOR_LINKED_LIST_t);
    < Any data >
} NAME_OF_DATA_FOR_LINKED_LIST_t;
*/

/** @brief Add item to tail of linked list **/
void *linked_list_add_last(linked_list_t *list, void *data);

/** @brief Add item to head of linked list **/
void *linked_list_add_first(linked_list_t *list, void *data);

/** @brief Insert item before specified item of linked list. If 'before' item does not exist or NULL - add to tail **/
void *linked_list_insert_before(linked_list_t *list, void *before, void *data);

/** @brief Remove item from linked list. Returns the pointer on removed item on success and if the free_function == NULL, otherwise NULL **/
void *linked_list_remove(linked_list_t *list, void *data, linked_list_free_cb free_function);

/** @brief Add item to tail if not exists. If equal item was found by @c linked_list_condition_cb the @c linked_list_update_cb will performed **/
void *linked_list_add_or_update(linked_list_t *list, linked_list_condition_cb condition_function,
    linked_list_update_cb update_func, void *data);

/** @brief Init linked list object **/
void linked_list_init(linked_list_t *list);

/** @brief Find linked list item that meets the conditions of @c linked_list_condition_cb and condition_input. Returns NULL on fail. **/
void *linked_list_find(linked_list_t *list, linked_list_condition_cb condition_function, void *condition_input);

/** @brief Get head item of linked list **/
void *linked_list_get_first(linked_list_t *list);

/** @brief Flush the linked list **/
void linked_list_flush(linked_list_t *list, linked_list_free_cb free_function);

/** @brief Get current size of linked list **/
size_t linked_list_get_size(linked_list_t *list);

/** @brief The default @c linked_list_condition_cb condition function returns true is if condition_input equals to item object pointer **/
bool linked_list_condition_default(void *item, void *condition_input);

/** @brief Run over linked list object **/
#define linked_list_foreach(list, iter) for (iter = (void *)((list)->head); (iter); iter = (iter)->next)

#endif /* __LIST_H__ */