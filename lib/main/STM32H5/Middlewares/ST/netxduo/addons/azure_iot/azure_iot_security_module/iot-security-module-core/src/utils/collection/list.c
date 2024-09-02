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

#include <asc_config.h>

#include "asc_security_core/utils/collection/list.h"

void linked_list_init(linked_list_t *list)
{
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->initialized = true;
}

void *linked_list_get_first(linked_list_t *list)
{
    if (list == NULL) {
        return NULL;
    }
    return list->head;
}

void *linked_list_find(linked_list_t *list, linked_list_condition_cb condition_function, void *condition_input)
{
    if (list == NULL || (condition_function) == NULL) {
        return NULL;
    }
    collection_item_t *curr = NULL;
    linked_list_foreach(list, curr) {
        if (condition_function(curr, condition_input) == true) {
            break;
        }
    }
    return curr;
}

void *linked_list_add_or_update(linked_list_t *list, linked_list_condition_cb condition_function,
    linked_list_update_cb update_func, void *data)
{
    if (list == NULL || (condition_function) == NULL || update_func == NULL) {
        return NULL;
    }
    collection_item_t *curr = NULL;
    linked_list_foreach(list, curr) {
        if (condition_function(curr, data) == true) {
            update_func(curr, data);
            return curr;
        }
    }
    return linked_list_add_last(list, data);
}

void *linked_list_add_last(linked_list_t *list, void *data)
{
    collection_item_t *item = data;
    if (list == NULL || item == NULL) {
        return NULL;
    }
    item->next = NULL;
    if (list->head == NULL) {
        item->previous = NULL;
        list->head = item;
        list->tail = item;
    } else {
        item->previous = list->tail;
        list->tail->next = item;
        list->tail = item;
    }

    list->size += 1;
    return data;
}

void *linked_list_add_first(linked_list_t *list, void *data)
{
    collection_item_t *item = data;
    if (list == NULL || item == NULL) {
        return NULL;
    }
    item->previous = NULL;
    item->next = NULL;
    if (list->head == NULL) {
        list->head = item;
        list->tail = item;
    } else {
        list->head->previous = item;
        item->next = list->head;
        list->head = item;
    }
    list->size += 1;
    return item;
}

void *linked_list_insert_before(linked_list_t *list, void *before, void *data)
{
    collection_item_t *item = data;
    if (list == NULL || item == NULL) {
        return NULL;
    }
    if (before == NULL) {
        return linked_list_add_last(list, item);
    }
    collection_item_t *before_item = linked_list_find(list, linked_list_condition_default, before);
    if (before_item == NULL) {
        return linked_list_add_last(list, item);
    }
    collection_item_t *previous_item = before_item->previous;
    if (previous_item == NULL) {
        return linked_list_add_first(list, item);
    }
    item->next = before_item;
    item->previous = previous_item;
    previous_item->next = item;
    before_item->previous = item;
    list->size += 1;
    return item;
}

void *linked_list_remove(linked_list_t *list, void *data, linked_list_free_cb free_function)
{
    collection_item_t *item = data;
    if (list == NULL || item == NULL) {
        return NULL;
    }
    if (linked_list_find(list, linked_list_condition_default, item) == NULL) {
        return NULL;
    }
    collection_item_t *previous_item = item->previous;
    collection_item_t *next_item = item->next;
    if (next_item != NULL) {
        next_item->previous = previous_item;
    }
    if (previous_item == NULL) {
        list->head = next_item;
    } else {
        previous_item->next = next_item;
    }
    if (item == list->tail) {
        list->tail = previous_item;
    }
    list->size -= 1;
    if (free_function != NULL) {
        free_function(item);
        return NULL;
    }
    return item;
}

void linked_list_flush(linked_list_t *list, linked_list_free_cb free_function)
{
    if (list == NULL) {
        return;
    }
    while (list->head != NULL) {
        linked_list_remove(list, list->head, free_function);
    }
    linked_list_init(list);
}

size_t linked_list_get_size(linked_list_t *list)
{
    if (list == NULL) {
        return 0;
    }
    return list->size;
}

bool linked_list_condition_default(void *item, void *condition_input)
{
    return (item==condition_input);
}
