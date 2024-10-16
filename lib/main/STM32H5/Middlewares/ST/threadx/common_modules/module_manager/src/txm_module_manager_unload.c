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
#include "tx_byte_pool.h"
#include "tx_initialize.h"
#include "tx_mutex.h"
#include "tx_thread.h"
#include "txm_module.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_unload                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function unloads a previously loaded module.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Module instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_byte_release                      Release data area             */
/*    _tx_mutex_get                         Get protection mutex          */
/*    _tx_mutex_put                         Release protection mutex      */
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
UINT  _txm_module_manager_unload(TXM_MODULE_INSTANCE *module_instance)
{

TX_INTERRUPT_SAVE_AREA

TXM_MODULE_INSTANCE *next_module, *previous_module;
CHAR                *memory_ptr;


    /* Check for interrupt call.  */
    if (TX_THREAD_GET_SYSTEM_STATE() != 0)
    {

        /* Now, make sure the call is from an interrupt and not initialization.  */
        if (TX_THREAD_GET_SYSTEM_STATE() < TX_INITIALIZE_IN_PROGRESS)
        {

            /* Invalid caller of this function, return appropriate error code.  */
            return(TX_CALLER_ERROR);
        }
    }

    /* Determine if the module manager has not been initialized yet.  */
    if (_txm_module_manager_ready != TX_TRUE)
    {

        /* Module manager has not been initialized.  */
        return(TX_NOT_AVAILABLE);
    }

    /* Determine if the module is valid.  */
    if (module_instance == TX_NULL)
    {

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Get module manager protection mutex.  */
    _tx_mutex_get(&_txm_module_manager_mutex, TX_WAIT_FOREVER);

    /* Determine if the module is already valid.  */
    if (module_instance -> txm_module_instance_id != TXM_MODULE_ID)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Determine if the module instance is in the  state.  */
    if ((module_instance -> txm_module_instance_state != TXM_MODULE_LOADED) && (module_instance -> txm_module_instance_state != TXM_MODULE_STOPPED))
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if the module is not ready.  */
        return(TX_NOT_DONE);
    }

    /* Pickup the module data memory allocation address.  */
    memory_ptr =  module_instance -> txm_module_instance_data_allocation_ptr;

    /* Release the module's data memory.  */
    _tx_byte_release(memory_ptr);

    /* Determine if there was memory allocated for the code.  */
    if (module_instance -> txm_module_instance_code_allocation_ptr)
    {

        /* Yes, release the module's code memory.  */
        memory_ptr =  module_instance -> txm_module_instance_code_allocation_ptr;

        /* Release the module's data memory.  */
        _tx_byte_release(memory_ptr);
    }

    /* Temporarily disable interrupts.  */
    TX_DISABLE

    /* Clear some of the module information.  */
    module_instance -> txm_module_instance_id =     0;
    module_instance -> txm_module_instance_state =  TXM_MODULE_UNLOADED;

    /* Call port-specific unload function.  */
    TXM_MODULE_MANAGER_MODULE_UNLOAD(module_instance);

    /* Remove the module from the linked list of loaded modules.  */

    /* See if the module is the only one on the list.  */
    if ((--_txm_module_manger_loaded_count) == 0)
    {

        /* Only created module, just set the created list to NULL.  */
        _txm_module_manager_loaded_list_ptr =  TX_NULL;
    }
    else
    {

        /* Otherwise, not the only created module, link-up the neighbors.  */
        next_module =                                module_instance -> txm_module_instance_loaded_next;
        previous_module =                            module_instance -> txm_module_instance_loaded_previous;
        next_module -> txm_module_instance_loaded_previous =  previous_module;
        previous_module -> txm_module_instance_loaded_next =  next_module;

        /* See if we have to update the created list head pointer.  */
        if (_txm_module_manager_loaded_list_ptr == module_instance)
        {

            /* Yes, move the head pointer to the next link. */
            _txm_module_manager_loaded_list_ptr =  next_module;
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release the protection mutex.  */
    _tx_mutex_put(&_txm_module_manager_mutex);

    /* Return success.  */
    return(TX_SUCCESS);
}
