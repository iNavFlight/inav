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

#ifndef ASC_RESULT_H
#define ASC_RESULT_H
#include <asc_config.h>

typedef enum {
    ASC_RESULT_OK                       = 0,
    ASC_RESULT_EXCEPTION                = 1,
    ASC_RESULT_MEMORY_EXCEPTION         = 2,
    ASC_RESULT_NOT_SUPPORTED_EXCEPTION  = 3,
    ASC_RESULT_RESOURCE_DESTROYED       = 4,
    ASC_RESULT_TIMEOUT                  = 5,
    ASC_RESULT_PENDING                  = 6,
    ASC_RESULT_PARSE_EXCEPTION          = 7,
    ASC_RESULT_OFF                      = 8,
    ASC_RESULT_BAD_ARGUMENT             = 9,
    ASC_RESULT_EMPTY                    = 10,
    ASC_RESULT_FULL                     = 11,
    ASC_RESULT_UNINITIALIZED            = 12,
    ASC_RESULT_INITIALIZED              = 13,
    ASC_RESULT_IMPOSSIBLE               = 14,
} asc_result_t;


#endif /* ASC_RESULT_H */
