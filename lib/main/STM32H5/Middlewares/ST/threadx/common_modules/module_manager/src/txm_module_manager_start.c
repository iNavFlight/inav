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
#include "tx_mutex.h"
#include "tx_queue.h"
#include "tx_thread.h"
#include "txm_module.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_start                           PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function starts execution of the specified module.             */
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
/*    _txm_module_manager_thread_create     Module thread create          */
/*    _tx_mutex_get                         Get protection mutex          */
/*    _tx_mutex_put                         Release protection mutex      */
/*    _tx_queue_create                      Create module callback queue  */
/*    _tx_queue_delete                      Delete module callback queue  */
/*    _tx_thread_resume                     Resume start thread           */
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
/*  12-31-2020      Scott Larson            Modified comment(s),          */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_start(TXM_MODULE_INSTANCE *module_instance)
{

UINT    status;


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

    /* Determine if the module instance is valid.  */
    if (module_instance -> txm_module_instance_id != TXM_MODULE_ID)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Determine if the module instance is in the loaded state.  */
    if ((module_instance -> txm_module_instance_state != TXM_MODULE_LOADED) && (module_instance -> txm_module_instance_state != TXM_MODULE_STOPPED))
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if the module is not ready.  */
        return(TX_START_ERROR);
    }

    /* Check the priorities of the start/stop and callback request threads. */
    if (module_instance -> txm_module_instance_start_stop_priority < module_instance -> txm_module_instance_maximum_priority ||
        module_instance -> txm_module_instance_callback_priority < module_instance -> txm_module_instance_maximum_priority)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* At least one thread has an invalid priority.  */
        return(TX_PRIORITY_ERROR);
    }

    /* Create the module's callback request queue.  */
    status = _tx_queue_create(&(module_instance -> txm_module_instance_callback_request_queue), "Module Callback Request Queue", (sizeof(TXM_MODULE_CALLBACK_MESSAGE)/sizeof(ULONG)),
                              module_instance -> txm_module_instance_callback_request_queue_area, sizeof(module_instance -> txm_module_instance_callback_request_queue_area));

    /* Determine if there was an error.  */
    if (status)
    {

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if the module is not ready.  */
        return(TX_START_ERROR);
    }

    /* Create the module start thread.  */
    status =  _txm_module_manager_thread_create(&(module_instance -> txm_module_instance_start_stop_thread),
                                                "Module Start Thread",
                                                module_instance -> txm_module_instance_shell_entry_function,
                                                module_instance -> txm_module_instance_start_thread_entry,
                                                module_instance -> txm_module_instance_application_module_id,
                                                module_instance -> txm_module_instance_start_stop_stack_start_address,
                                                module_instance -> txm_module_instance_start_stop_stack_size,
                                                (UINT) module_instance -> txm_module_instance_start_stop_priority,
                                                (UINT) module_instance -> txm_module_instance_start_stop_priority,
                                                TXM_MODULE_TIME_SLICE,
                                                TX_DONT_START,
                                                sizeof(TX_THREAD),
                                                module_instance);

    /* Determine if the thread create was successful.  */
    if (status != TX_SUCCESS)
    {

        /* Delete the callback notification queue.  */
        _tx_queue_delete(&(module_instance -> txm_module_instance_callback_request_queue));

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return the error status.  */
        return(status);
    }

    /* Create the module callback thread.  */
    status =  _txm_module_manager_thread_create(&(module_instance -> txm_module_instance_callback_request_thread),
                                                "Module Callback Request Thread",
                                                module_instance -> txm_module_instance_shell_entry_function,
                                                module_instance -> txm_module_instance_callback_request_thread_entry,
                                                module_instance -> txm_module_instance_application_module_id,
                                                module_instance -> txm_module_instance_callback_stack_start_address,
                                                module_instance -> txm_module_instance_callback_stack_size,
                                                (UINT) module_instance -> txm_module_instance_callback_priority,
                                                (UINT) module_instance -> txm_module_instance_callback_priority,
                                                TX_NO_TIME_SLICE,
                                                TX_DONT_START,
                                                sizeof(TX_THREAD),
                                                module_instance);

    /* Determine if the thread create was successful.  */
    if (status != TX_SUCCESS)
    {

        /* Terminate the start thread.  */
        _tx_thread_terminate(&(module_instance -> txm_module_instance_start_stop_thread));

        /* Delete the start thread.  */
        _tx_thread_delete(&(module_instance -> txm_module_instance_start_stop_thread));

        /* Delete the callback notification queue.  */
        _tx_queue_delete(&(module_instance -> txm_module_instance_callback_request_queue));

        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return the error status.  */
        return(status);
    }


    /* Set the module state to started.  */
    module_instance -> txm_module_instance_state =  TXM_MODULE_STARTED;

    /* Release the protection mutex.  */
    _tx_mutex_put(&_txm_module_manager_mutex);

    /* Resume the module's start thread.  */
    _tx_thread_resume(&(module_instance -> txm_module_instance_start_stop_thread));

    /* Return success.  */
    return(TX_SUCCESS);
}

