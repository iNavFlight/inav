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

#ifndef __CONTAINER_OF_H__
#define __CONTAINER_OF_H__
#include <asc_config.h>

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

static inline char *_containerof(char *ptr, size_t offset) {
    return ptr ? ptr - offset : NULL;
}

#define containerof(ptr, containing_type, field) \
    ((containing_type *)(_containerof((char *)(ptr), \
        offsetof(containing_type, field))))

#endif