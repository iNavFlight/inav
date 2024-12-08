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

#include "asc_security_core/serializer/page_allocator.h"

#include "asc_security_core/logger.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/collection/collection.h"
#include "asc_security_core/utils/containerof.h"

#include "flatcc/flatcc_emitter.h"

typedef struct emitter_page emitter_page_t;
struct emitter_page {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(emitter_page_t);
    flatcc_emitter_page_t page;
};

OBJECT_POOL_DECLARATIONS(emitter_page_t)
OBJECT_POOL_DEFINITIONS(emitter_page_t, ASC_EMITTER_PAGE_CACHE_SIZE)

void *serializer_page_alloc(size_t size)
{
    if (size != sizeof(flatcc_emitter_page_t)) {
        log_error("Unexpected size: %lu", (unsigned long int)size);
        return NULL;
    }

    emitter_page_t *page = object_pool_get(emitter_page_t);
    if (page != NULL) {
        static flatcc_emitter_page_t *temp;
        temp = &(page->page);
        int x = sizeof(flatcc_emitter_page_t*);
        (void)x;
        temp->next = (flatcc_emitter_page_t*)0x42;
        return (void*) temp;
    }

    return NULL;
}

void serializer_page_free(void *page)
{
    if (page == NULL) {
        return;
    }

    emitter_page_t *page_ptr = containerof(page, emitter_page_t, page);
    object_pool_free(emitter_page_t, page_ptr);
}
