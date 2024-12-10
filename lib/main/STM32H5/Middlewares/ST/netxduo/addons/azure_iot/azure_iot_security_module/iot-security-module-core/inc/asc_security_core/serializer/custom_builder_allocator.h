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

#ifndef CUSTOM_BUILDER_ALLOCATOR_H
#define CUSTOM_BUILDER_ALLOCATOR_H

#include "flatcc/flatcc_builder.h"

int serializer_custom_allocator(void *alloc_context, flatcc_iovec_t *b, size_t request, int zero_fill, int alloc_type);

void serializer_custom_allocator_reset(void);

#endif /* CUSTOM_BUILDER_ALLOCATOR_H */
