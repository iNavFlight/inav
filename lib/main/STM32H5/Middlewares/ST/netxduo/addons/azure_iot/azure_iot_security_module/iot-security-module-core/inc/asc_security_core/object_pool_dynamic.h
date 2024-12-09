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

#ifndef OBJECT_POOL_DYNAMIC_H
#define OBJECT_POOL_DYNAMIC_H

#include <stdint.h>

#include <asc_config.h>

#include "asc_security_core/object_pool_def.h"

/* Pay your attention, that this implementation is not thread safe. */

/* The "type" in OBJECT_POOL_DECLARATIONS(type) OBJECT_POOL_DEFINITIONS(type) macros MUST be from following type:
typedef struct NAME_OF_DATA_FOR_LINKED_LIST_t {
    // This macro must be first in object 
    COLLECTION_INTERFACE(struct NAME_OF_DATA_FOR_LINKED_LIST_t);
    < Any data >
} NAME_OF_DATA_FOR_LINKED_LIST_t;
*/
#define OBJECT_POOL_DECLARATIONS(type)\
extern type _##type##_pool_test;\
extern object_pool_t _##type##_pool_obj;

#define OBJECT_POOL_DEFINITIONS(type, pool_size)\
type _##type##_pool_test;\
object_pool_t _##type##_pool_obj = {.item_size = sizeof(type), .size = pool_size, .initialized = false, .stack = {0}, .current_size = 0, .failures = 0};

void *__object_pool_get(object_pool_t *pool, uintptr_t offset1, uintptr_t offset2);
void __object_pool_free(object_pool_t *pool, void *obj);
size_t __object_pool_get_available_size(object_pool_t *pool);

#define object_pool_init(type) _##type##_pool_obj.initialized = true

#define object_pool_get(type)\
(type *)__object_pool_get((object_pool_t *)&_##type##_pool_obj, (uintptr_t)(&_##type##_pool_test), (uintptr_t)(&(_##type##_pool_test.previous)))

#define object_pool_free(type, obj)\
__object_pool_free((object_pool_t *)&_##type##_pool_obj, (void *)obj)

/* Returns number of available objects of specific type.
 * If the definition of pool (OBJECT_POOL_DEFINITIONS(type_t, 0)) was done with zero size - always returns SIZE_MAX.
 */
#define object_pool_get_available_size(type) \
__object_pool_get_available_size((object_pool_t *)&_##type##_pool_obj)

#endif /* OBJECT_POOL_DYNAMIC_H */