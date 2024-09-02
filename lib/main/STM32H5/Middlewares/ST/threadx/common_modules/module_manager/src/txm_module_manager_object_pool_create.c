/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX Component                                                     */
/**                                                                       */
/**   Module Manager                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE

#include "tx_api.h"
#include "txm_module.h"
#include "tx_byte_pool.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    txm_module_manager_object_pool_create               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an object pool for the module manager,        */
/*    which is used by modules to allocate system resources outside       */
/*    the memory area of the module. This is especially useful in         */
/*    memory protection.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_memory                     Object memory address             */
/*    object_memory_size                Size in bytes of memory area      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_byte_pool_create              Create module memory byte pool    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_object_pool_create(VOID *object_memory, ULONG object_memory_size)
{

    /* Create a byte pool for allocating RAM areas for modules.  */
    _tx_byte_pool_create(&_txm_module_manager_object_pool, "Module Manager Object Pool", object_memory, object_memory_size);

    /* Indicate the module manager object pool has been created.  */
    _txm_module_manager_object_pool_created =  TX_TRUE;

    /* Return success.  */
    return(TX_SUCCESS);
}

