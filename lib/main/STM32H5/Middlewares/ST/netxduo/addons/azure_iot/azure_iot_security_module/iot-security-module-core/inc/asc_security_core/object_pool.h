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

#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H
#include <asc_config.h>

#include "asc_security_core/utils/collection/collection.h"
#include "asc_security_core/utils/collection/stack.h"

#ifdef ASC_DYNAMIC_MEMORY_ENABLED
#include "asc_security_core/object_pool_dynamic.h"
#else
#include "asc_security_core/object_pool_static.h"
#endif

#endif /* OBJECT_POOL_H */