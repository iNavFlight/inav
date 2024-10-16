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

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stddef.h>

#include <asc_config.h>

#include "asc_security_core/utils/collection/collection.h"

/* Pay your attention, that this implementation is not thread safe. */

typedef struct {
    collection_item_t *head;
    size_t size;
} stack_collection_t;

void stack_push(stack_collection_t *stack, collection_item_t *item);
collection_item_t * stack_pop(stack_collection_t *stack);
collection_item_t * stack_peek(stack_collection_t *stack);

#endif /* STACK_H */