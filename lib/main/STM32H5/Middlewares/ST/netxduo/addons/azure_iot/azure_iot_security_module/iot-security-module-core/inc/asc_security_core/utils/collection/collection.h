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

#ifndef COLLECTION_H
#define COLLECTION_H
#include <asc_config.h>

/* Pay your attention, that this implementation is not thread safe. */
/* This macro must be first in object */
#define COLLECTION_INTERFACE(type)\
type *previous;\
type *next

typedef struct {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(void);
} collection_item_t;

#endif /* COLLECTION_H */