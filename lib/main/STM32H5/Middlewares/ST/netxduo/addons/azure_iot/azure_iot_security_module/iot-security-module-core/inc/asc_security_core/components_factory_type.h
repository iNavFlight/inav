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

#ifndef __COMPONENTS_FACTORY__TYPE_H__
#define __COMPONENTS_FACTORY__TYPE_H__

#include <stdbool.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"
#include "asc_security_core/component_info.h"

/**
 * @brief Factory load the component function prototype.
 */
typedef asc_result_t (*component_load_function_t)(void);

/**
 * @brief Struct of factory entry.
 */
typedef struct {
    component_t component;
} components_factory_t;

/**
 * @brief Factory global array.
 */
extern components_factory_t g_component_factory[];

/**
 * @brief Get array of component load functions.
 *
 * @return An @c component_load_function_t array of component load functions.
 */
component_load_function_t **components_factory_get_load_array(void);

/**
 * @brief Factory unload - clean built-in components data from @c g_component_factory .
 * 
 * @return none.
 */
void components_factory_unload(void);

/**
 * @brief Factory unload - clean built-in component with specified index from @c g_component_factory .
 * 
 * @param index        A component's index.
 * 
 * @return none.
 */
void component_factory_unload(int index);

/**
 * @brief Generate ID for component with specified index.
 * 
 * @param index        A component's index.
 * 
 * @return A component's ID.
 */
component_id_t components_factory_create_id(int index);

/**
 * @brief Insert component with specified index to @c g_component_factory .
 * 
 * @param name          A component's name.
 * @param index         A component's index.
 * @param ops           A component's LCM ops.
 * @param auto_disable  Is componet auto disable to run.
 * 
 * @return ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t components_factory_set(const char *name, int index, component_ops_t *ops, bool auto_disable);

/**
 * @brief Dynamic loading API .
 */
#ifdef ASC_DYNAMIC_FACTORY_ENABLED
/**
 * @brief Insert dynamic loaded collector with specified index to @c g_component_factory .
 * 
 * @param name          A collector's name.
 * @param index         A collector's index.
 * @param ops           A collector's LCM ops.
 * @param auto_disable  Is collector auto disable to run.
 * 
 * @return ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t components_factory_set_dynamic_collector(const char *name, int index, component_ops_t *ops, bool auto_disable);

/**
 * @brief Insert dynamic loaded component with specified index to @c g_component_factory .
 * 
 * @param name          A component's name.
 * @param index         A component's index.
 * @param ops           A component's LCM ops.
 * @param auto_disable  Is componet auto disable to run.
 * 
 * @return ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t components_factory_set_dynamic_component(const char *name, int index, component_ops_t *ops, bool auto_disable);

/**
 * @brief Dynamic loading of all existing collectors and componets to @c g_component_factory .
 * 
 * @param __path        A directory of dynamic stuff - if NULL taken from components_factory_dynamic_path_set() and ASC_DYNAMIC_FACTORY_PATH.
 *
 * @return ASC_RESULT_OK on success, corresponding error code otherwise.
 */
asc_result_t components_factory_load_dynamic(const char *__path);

/**
 * @brief Unload all dynamic loaded entries from @c g_component_factory .
 * 
 * @return none.
 */
void components_factory_unload_dynamic(void);


#ifdef ASC_DYNAMIC_FACTORY_PATH_RUNTIME_SET
/**
 * @brief Overwrite default path to dynamic stuff.
 * 
 * @param __path    A directory of dynamic stuff.
 *
 * @return none.
 */
void components_factory_dynamic_path_set(const char *__path);
#endif
#endif // ASC_DYNAMIC_FACTORY_ENABLED

#endif // __COMPONENTS_FACTORY__TYPE_H__
