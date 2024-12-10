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
#include "tx_thread.h"
#include "txm_module.h"


/* Define the user's fault notification callback function pointer.  This is
   setup via the txm_module_manager_memory_fault_notify API.  */

VOID    (*_txm_module_manager_fault_notify)(TX_THREAD *, TXM_MODULE_INSTANCE *);


/* Define a macro that can be used to allocate global variables useful to
   store information about the last fault. This macro is defined in
   txm_module_port.h and is usually populated in the assembly language
   fault handling prior to the code calling _txm_module_manager_memory_fault_handler.  */

TXM_MODULE_MANAGER_FAULT_INFO


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_memory_fault_handler            Cortex-M7       */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles a fault associated with a memory protected    */
/*    module.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_terminate              Terminate thread                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Fault handler                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-15-2021      Scott Larson            Initial Version 6.1.9         */
/*                                                                        */
/**************************************************************************/
VOID  _txm_module_manager_memory_fault_handler(VOID)
{

TXM_MODULE_INSTANCE     *module_instance_ptr;
TX_THREAD               *thread_ptr;

    /* Pickup the current thread.  */
    thread_ptr =  _tx_thread_current_ptr;

    /* Initialize the module instance pointer to NULL.  */
    module_instance_ptr =  TX_NULL;
    
    /* Is there a thread?  */
    if (thread_ptr)
    {
        /* Pickup the module instance.  */
        module_instance_ptr =  thread_ptr -> tx_thread_module_instance_ptr;

        /* Terminate the current thread.  */
        _tx_thread_terminate(_tx_thread_current_ptr);
    }
    
    /* Determine if there is a user memory fault notification callback.  */
    if (_txm_module_manager_fault_notify)
    {
        /* Yes, call the user's notification memory fault callback.  */
        (_txm_module_manager_fault_notify)(thread_ptr, module_instance_ptr);
    }
}
