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

#ifndef OBJECT_POOL_DEF_H
#define OBJECT_POOL_DEF_H

#include <stddef.h>

#include <asc_config.h>

#include "asc_security_core/utils/collection/stack.h"

typedef struct {
    bool initialized;
    stack_collection_t stack;
    size_t item_size;
    size_t size;
    size_t current_size;
    size_t failures;
} object_pool_t;

#endif /* OBJECT_POOL_DEF_H */