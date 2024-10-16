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

#ifndef _EVENT_BE_H_
#define _EVENT_BE_H_
#include <asc_config.h>

#include "asc_security_core/components_factory_enum.h"

#include "asc_security_core/utils/ievent_loop.h"

#ifndef ASC_EXTRA_BE_TIMERS_OBJECT_POOL_ENTRIES
#ifndef ASC_BE_TIMERS_OBJECT_POOL_ENTRIES
#define OBJECT_POOL_BE_EVENT_LOOP_TIMERS_COUNT (COMPONENTS_COUNT + 1)
#else
#define OBJECT_POOL_BE_EVENT_LOOP_TIMERS_COUNT (ASC_BE_TIMERS_OBJECT_POOL_ENTRIES)
#endif
#else
#ifndef ASC_BE_TIMERS_OBJECT_POOL_ENTRIES
#define OBJECT_POOL_BE_EVENT_LOOP_TIMERS_COUNT ((COMPONENTS_COUNT + 1) + ASC_EXTRA_BE_TIMERS_OBJECT_POOL_ENTRIES)
#else
#define OBJECT_POOL_BE_EVENT_LOOP_TIMERS_COUNT ((ASC_BE_TIMERS_OBJECT_POOL_ENTRIES) + ASC_EXTRA_BE_TIMERS_OBJECT_POOL_ENTRIES)
#endif
#endif


/**
 * @brief   Attach specific best effort event loop implementation.
 *
 * @return  @c ievent_loop_t structure represents event loop based on base effort
 */
ievent_loop_t *event_loop_be_instance_attach(void);

#endif //_EVENT_BE_H_