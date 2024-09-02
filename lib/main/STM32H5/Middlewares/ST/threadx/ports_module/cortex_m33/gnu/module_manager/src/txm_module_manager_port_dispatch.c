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
#include "txm_module_manager_util.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_port_dispatch                  Cortex-M33       */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function dispatches the module's kernel request based upon the */
/*    ID and parameters specified in the request.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Module pointer                    */
/*    kernel_request                    Module's kernel request           */
/*    param_0                           First parameter                   */
/*    param_1                           Second parameter                  */
/*    param_2                           Third parameter                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Port-specific ThreadX API Calls                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _txm_module_manager_kernel_dispatch                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  03-02-2021      Scott Larson            Initial Version 6.1.5         */
/*                                                                        */
/**************************************************************************/
ALIGN_TYPE _txm_module_manager_port_dispatch(TXM_MODULE_INSTANCE *module_instance, ULONG kernel_request, ALIGN_TYPE param_0, ALIGN_TYPE param_1, ALIGN_TYPE param_2)
{

ALIGN_TYPE return_value = TX_NOT_AVAILABLE;

    switch(kernel_request)
    {
        case TXM_THREAD_SECURE_STACK_ALLOCATE_CALL:
        {
            if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
            {
                if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
                    return(TXM_MODULE_INVALID_MEMORY);
            }

            return_value = (ALIGN_TYPE) _txe_thread_secure_stack_allocate(
                (TX_THREAD *) param_0,
                (ULONG) param_1
            );
            break;
        }
        
        case TXM_THREAD_SECURE_STACK_FREE_CALL:
        {
            if (module_instance -> txm_module_instance_property_flags & TXM_MODULE_MEMORY_PROTECTION)
            {
                if (!TXM_MODULE_MANAGER_PARAM_CHECK_OBJECT_FOR_USE(module_instance, param_0, sizeof(TX_THREAD)))
                    return(TXM_MODULE_INVALID_MEMORY);
            }

            return_value = (ALIGN_TYPE) _txe_thread_secure_stack_free(
                (TX_THREAD *) param_0
            );
            break;
        }
        
        default:
        {
            /* Unhandled kernel request, return an error!  */
            break;
        }
    }
    
    return(return_value);
}
