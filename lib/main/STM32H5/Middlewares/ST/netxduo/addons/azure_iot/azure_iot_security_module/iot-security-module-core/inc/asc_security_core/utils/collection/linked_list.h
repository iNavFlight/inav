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

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <asc_config.h>

#include "asc_security_core/utils/collection/collection.h"

#define LINKED_LIST_DECLARATIONS(type) \
typedef void (*linked_list_##type##_deinit_function)(type *item);\
typedef void (*linked_list_##type##_action)(type *item, void *context);\
typedef struct {\
    type *head;\
    type *tail;\
    uint32_t size;\
    linked_list_##type##_deinit_function deinit;\
    bool initialized; \
} linked_list_##type;\
typedef linked_list_##type *linked_list_##type##_handle;\
typedef struct {\
    type *current;\
    type *next;\
} linked_list_iterator_##type;\
typedef linked_list_iterator_##type *linked_list_iterator_##type##_handle;\
void linked_list_##type##_init(linked_list_##type##_handle linked_list, linked_list_##type##_deinit_function deinit_function);\
void linked_list_##type##_deinit(linked_list_##type##_handle linked_list);\
type *linked_list_##type##_get_first(linked_list_##type##_handle linked_list);\
type *linked_list_##type##_get_last(linked_list_##type##_handle linked_list);\
uint32_t linked_list_##type##_get_size(linked_list_##type##_handle linked_list);\
type *linked_list_##type##_add_first(linked_list_##type##_handle linked_list, type *item);\
type *linked_list_##type##_pop_first(linked_list_##type##_handle linked_list);\
type *linked_list_##type##_add_last(linked_list_##type##_handle linked_list, type *item);\
type *linked_list_##type##_pop_last(linked_list_##type##_handle linked_list);\
void linked_list_##type##_remove(linked_list_##type##_handle linked_list, type *item);\
type *linked_list_##type##_insert(linked_list_##type##_handle linked_list, type *before, type *item);\
void linked_list_##type##_concat(linked_list_##type##_handle first_linked_list, linked_list_##type##_handle second_linked_list);\
void linked_list_##type##_foreach(linked_list_##type##_handle linked_list, linked_list_##type##_action action_function, void *context);\
typedef bool (*linked_list_##type##_condition)(type *item, void *condition_input);\
type *linked_list_##type##_find(linked_list_##type##_handle linked_list, linked_list_##type##_condition condition_function, void *condition_input);\
void linked_list_iterator_##type##_init(linked_list_iterator_##type##_handle linked_list_iterator, linked_list_##type##_handle linked_list);\
type *linked_list_iterator_##type##_next(linked_list_iterator_##type##_handle linked_list_iterator);\
type *linked_list_iterator_##type##_current(linked_list_iterator_##type##_handle linked_list_iterator);\

#define LINKED_LIST_DEFINITIONS(type) \
static bool linked_list_##type##_condition_default(type *a, void *b) \
{\
    return (a==(type*)b); \
}\
void linked_list_##type##_init(linked_list_##type##_handle linked_list, linked_list_##type##_deinit_function deinit_function) \
{\
    if ((linked_list) == NULL) {\
        return;\
    }\
    (linked_list)->head = NULL;\
    (linked_list)->tail = NULL;\
    (linked_list)->size = 0;\
    (linked_list)->deinit = deinit_function;\
    (linked_list)->initialized = true;\
}\
void linked_list_##type##_deinit(linked_list_##type##_handle linked_list) \
{\
    if ((linked_list) == NULL) {\
        return;\
    }\
\
    while ((linked_list)->head != NULL) {\
        linked_list_##type##_remove((linked_list), (linked_list)->head);\
    }\
    (linked_list)->initialized = false;\
}\
type *linked_list_##type##_get_first(linked_list_##type##_handle linked_list) \
{\
    return (linked_list)->head;\
}\
type *linked_list_##type##_get_last(linked_list_##type##_handle linked_list) \
{\
    return (linked_list)->tail;\
}\
uint32_t linked_list_##type##_get_size(linked_list_##type##_handle linked_list) \
{\
    return (linked_list)->size;\
}\
type *linked_list_##type##_add_first(linked_list_##type##_handle linked_list, type *item) \
{\
    if ((linked_list) == NULL || (item) == NULL) {\
        return NULL;\
    }\
\
    item->previous = NULL;\
    item->next = NULL;\
\
    if ((linked_list)->head == NULL) {\
        (linked_list)->head = item;\
        (linked_list)->tail = item;\
    } else { \
        (linked_list)->head->previous = item;\
        item->next = (linked_list)->head;\
        (linked_list)->head = item;\
    }\
\
    (linked_list)->size += 1;\
    return item;\
}\
type *linked_list_##type##_pop_first(linked_list_##type##_handle linked_list) \
{\
    type *item;\
    if ((linked_list) == NULL) {\
        return NULL;\
    }\
    if ((linked_list)->head == NULL) {\
        return NULL;\
    }\
\
    type *head = (linked_list)->head;\
    type *next_item = head->next;\
\
    if (next_item == NULL) {\
        (linked_list)->tail = NULL;\
    } else {\
        next_item->previous = NULL;\
    }\
\
    item = head;\
\
    (linked_list)->head = next_item;\
    (linked_list)->size -= 1;\
\
    return item;\
}\
type *linked_list_##type##_add_last(linked_list_##type##_handle linked_list, type *item) \
{\
    if ((linked_list) == NULL || (item) == NULL) {\
        return NULL;\
    }\
\
    (item)->next = NULL;\
    if ((linked_list)->head == NULL) {\
        (item)->previous = NULL;\
        (linked_list)->head = item;\
        (linked_list)->tail = item;\
    } else {\
        (item)->previous = (linked_list)->tail;\
        (linked_list)->tail->next = item;\
        (linked_list)->tail = item;\
    }\
\
    (linked_list)->size += 1;\
    return item;\
}\
type *linked_list_##type##_pop_last(linked_list_##type##_handle linked_list) \
{\
    if ((linked_list) == NULL) {\
        return NULL;\
    }\
    if ((linked_list)->tail == NULL) {\
        return NULL;\
    }\
    type *tail = (linked_list)->tail;\
    type *previous_item = tail->previous;\
\
    if (previous_item == NULL) {\
        (linked_list)->head = NULL;\
    } else {\
        previous_item->next = NULL;\
    }\
\
    (linked_list)->tail = previous_item;\
    (linked_list)->size -= 1;\
    return tail;\
}\
void linked_list_##type##_remove(linked_list_##type##_handle linked_list, type *item) \
{\
    if ((linked_list) == NULL || (item) == NULL) {\
        return;\
    }\
    if (linked_list_##type##_find(linked_list, linked_list_##type##_condition_default, item) == NULL) {\
        return;\
    }\
    type *previous_item = (item)->previous;\
    type *next_item = (item)->next;\
\
    if (next_item != NULL) {\
        next_item->previous = previous_item;\
    }\
\
    if (previous_item == NULL) {\
        (linked_list)->head = next_item;\
    } else {\
        previous_item->next = next_item;\
    }\
\
    if ((item) == (linked_list)->tail) {\
        (linked_list)->tail = previous_item;\
    }\
    if ((linked_list)->deinit != NULL) {\
        (linked_list)->deinit(item);\
    }\
\
    (linked_list)->size -= 1;\
    return;\
}\
type *linked_list_##type##_insert(linked_list_##type##_handle linked_list, type *before, type *item) \
{\
    if ((linked_list) == NULL || (item) == NULL) {\
        return NULL;\
    }\
    if ((before) == NULL) {\
        return linked_list_##type##_add_last(linked_list, item); \
    }\
    type *before_item = linked_list_##type##_find(linked_list, linked_list_##type##_condition_default, before);\
    if ((before_item) == NULL) {\
        return NULL; \
    }\
    type *previous_item = (before_item)->previous;\
    if (previous_item == NULL) {\
        return linked_list_##type##_add_first(linked_list, item); \
    } \
    item->next = before_item; \
    item->previous = previous_item; \
    previous_item->next = item; \
    before_item->previous = item; \
    (linked_list)->size += 1;\
    return item;\
}\
void linked_list_##type##_concat(linked_list_##type##_handle first_linked_list, linked_list_##type##_handle second_linked_list) \
{\
    if ((first_linked_list) == NULL) {\
        return;\
    }\
    if ((second_linked_list) == NULL) {\
        return;\
    }\
    type *second_head = (second_linked_list)->head;\
    if ((second_head) == NULL) {\
        return;\
    }\
\
    if ((first_linked_list)->head == NULL) {\
        (second_head)->previous = NULL;\
        (first_linked_list)->head = second_head;\
        (first_linked_list)->tail = second_head;\
    } else {\
        (second_head)->previous = (first_linked_list)->tail;\
        if ((first_linked_list)->tail != NULL) {\
            (first_linked_list)->tail->next = second_head;\
            (first_linked_list)->tail = second_head;\
        }\
    }\
\
    (first_linked_list)->size += (second_linked_list)->size;\
    return;\
}\
void linked_list_##type##_foreach(linked_list_##type##_handle linked_list, linked_list_##type##_action action_function, void *context) \
{\
    if ((linked_list) == NULL || (action_function) == NULL) {\
        return;\
    }\
    type *item = (linked_list)->head;\
\
    while (item != NULL) {\
        linked_list_##type##_action action = action_function;\
        action(item, context);\
        item = item->next;\
    }\
\
    return;\
}\
type *linked_list_##type##_find(linked_list_##type##_handle linked_list, linked_list_##type##_condition condition_function, void *condition_input) \
{\
    if ((linked_list) == NULL || (condition_function) == NULL) {\
        return NULL;\
    }\
    type *item = (linked_list)->head;\
\
    while (item != NULL) {\
        linked_list_##type##_condition condition = condition_function;\
        if (condition(item, condition_input) == true) {\
            break;\
        }\
\
        item = item->next;\
    }\
\
    return item;\
}\
void linked_list_iterator_##type##_init(linked_list_iterator_##type##_handle linked_list_iterator, linked_list_##type##_handle linked_list) \
{\
    if ((linked_list) == NULL) {\
        return;\
    }\
    (linked_list_iterator)->current = (linked_list)->head;\
    return;\
}\
type *linked_list_iterator_##type##_next(linked_list_iterator_##type##_handle linked_list_iterator) \
{\
    type *current = (linked_list_iterator)->current;\
    if ((linked_list_iterator)->current != NULL) {\
        (linked_list_iterator)->current = (linked_list_iterator)->current->next;\
    }\
    return current;\
}\
type *linked_list_iterator_##type##_current(linked_list_iterator_##type##_handle linked_list_iterator) \
{\
    return (linked_list_iterator)->current;\
}\

#endif /* LINKED_LIST */