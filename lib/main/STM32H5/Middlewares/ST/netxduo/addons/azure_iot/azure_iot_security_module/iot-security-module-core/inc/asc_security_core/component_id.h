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

#ifndef __COMPONENT_ID_H__
#define __COMPONENT_ID_H__
#include <asc_config.h>

#include "asc_security_core/components_factory_type.h"

static component_id_t __auto_generated_self_id = 0;

/**
 * @brief Get self ID of specific component.
 * Can be called after COMPONENTS_FACTORY_DEFINITION() only.
 *
 * @return A @c component_id_t component ID.
 */
static inline component_id_t components_manager_get_self_id()
{
    return __auto_generated_self_id;
}

/**
 * @brief Set self ID of specific component.
 * Can be called instead COMPONENTS_FACTORY_DEFINITION().
 * 
 * @param component Component enumerator
 *
 */
static inline void components_manager_set_self_id(int component)
{
    __auto_generated_self_id = g_component_factory[component].component.info.id;
}

/**
 * @brief Reset static self ID in specific file.
 */
static inline void components_manager_reset_self_id()
{
    __auto_generated_self_id = 0;
}
#endif