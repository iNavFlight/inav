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
#include <stdbool.h>
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/utils/collection/stack.h"

collection_item_t *stack_pop(stack_collection_t *stack)
{
    if (!stack) {
        return NULL;
    }
    collection_item_t *current_head = stack->head;
    if (current_head == NULL) {
        return NULL;
    }
    collection_item_t *new_head = current_head->next;
    current_head->previous = current_head->next = NULL;
    if (new_head != NULL) {
        new_head->previous = NULL;
    }
    stack->head = new_head;
    return current_head;
}

void stack_push(stack_collection_t *stack, collection_item_t *item)
{
    if (!stack || !item) {
        return;
    }
    collection_item_t *current_head = stack->head;
    if (current_head == NULL) {
        stack->head = item;
        item->next = NULL;
        item->previous = NULL;
        stack->size++;
    } else {
        item->next = current_head;
        item->previous = NULL;
        current_head->previous = item;
        stack->head = item;
        stack->size++;
    }
}

collection_item_t * stack_peek(stack_collection_t *stack)
{
    if (!stack) {
        return NULL;
    }
    return stack->head;
}
