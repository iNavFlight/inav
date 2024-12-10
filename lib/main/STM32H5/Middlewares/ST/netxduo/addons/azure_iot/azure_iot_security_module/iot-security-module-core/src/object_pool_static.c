
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
#include <stdbool.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/object_pool_static.h"

static bool _object_pool_init(object_pool_t *pool, uintptr_t offset1, uintptr_t offset2, uintptr_t objs)
{
    if (!pool || pool->size == 0) {
        log_fatal("Wrong definition pool=[%p] size=[%zu]", (void *)pool, pool ? pool->size : 0);
        return false;
    }
    if (!pool->initialized) {
        if (offset1 != offset2) {
            log_fatal("Wrong offset of pool typedef, COLLECTION_INTERFACE(...) is not on top.");
            return false;
        }
        for (size_t i=0; i<pool->size; i++) {
            uintptr_t obj = objs + (i * pool->item_size);
            stack_push(&pool->stack, (collection_item_t *)obj);
        }
        pool->initialized = true;
    }
    return true;
}

void *__object_pool_get(object_pool_t *pool, uintptr_t offset1, uintptr_t offset2, uintptr_t objs)
{
    if (!_object_pool_init(pool, offset1, offset2, objs)) {
        return NULL;
    }
    if (pool->current_size >= pool->size) {
        if (++pool->failures % pool->size == 0) {
            log_debug("Pool exceeded objects [%zu/%zu] failures=[%zu]", pool->current_size, pool->size, pool->failures);
        }
        return NULL;
    }
    pool->current_size++;
    return (void *)stack_pop(&pool->stack);
}

void __object_pool_free(object_pool_t *pool, void *obj)
{
    if (!pool || !obj || !pool->initialized) {
        return;
    }
    if (pool->current_size == 0) {
        log_fatal("Invalid memory free");
    } else {
        pool->current_size--;
        stack_push(&pool->stack, (collection_item_t *)obj);
    }
}

size_t __object_pool_get_available_size(object_pool_t *pool)
{
    return (pool->size - pool->current_size);
}