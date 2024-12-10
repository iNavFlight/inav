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
#include "tx_queue.h"
#include "tx_thread.h"
#include "txm_module.h"

#ifndef TX_DISABLE_NOTIFY_CALLBACKS
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_thread_notify_trampoline        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the thread entry/exit notification call     */
/*    from ThreadX.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Thread pointer                    */
/*    type                              Entry or exit type                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _txm_module_manager_callback_request  Send module callback request  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX                                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _txm_module_manager_thread_notify_trampoline(TX_THREAD *thread_ptr, UINT type)
{

TX_INTERRUPT_SAVE_AREA

TXM_MODULE_INSTANCE             *module_instance;
TXM_MODULE_CALLBACK_MESSAGE     callback_message;
TX_QUEUE                        *module_callback_queue;
TXM_MODULE_THREAD_ENTRY_INFO    *thread_info;


    /* We now know the callback is for a module.  */

    /* Disable interrupts.  */
    TX_DISABLE

    /* Determine if the thread is valid.  */
    if ((thread_ptr) && (thread_ptr -> tx_thread_id == TX_THREAD_ID))
    {

        /* Pickup the module instance pointer.  */
        module_instance =  (TXM_MODULE_INSTANCE *) thread_ptr -> tx_thread_module_instance_ptr;

        /* Pickup the module's thread pointer.  */
        thread_info =  (TXM_MODULE_THREAD_ENTRY_INFO *) thread_ptr -> tx_thread_module_entry_info_ptr;

        /* Determine if this module is still valid.  */
        if ((module_instance) && (module_instance -> txm_module_instance_id == TXM_MODULE_ID) &&
            (module_instance -> txm_module_instance_state == TXM_MODULE_STARTED))
        {

            /* Yes, the module is still valid.  */

            /* Pickup the module's callback message queue.  */
            module_callback_queue =  &(module_instance -> txm_module_instance_callback_request_queue);

            /* Build the queue notification message.  */
            callback_message.txm_module_callback_message_type =                  TXM_THREAD_ENTRY_EXIT_CALLBACK;
            callback_message.txm_module_callback_message_activation_count =      1;
            callback_message.txm_module_callback_message_application_function =  (VOID (*)(VOID)) thread_info -> txm_module_thread_entry_info_exit_notify;
            callback_message.txm_module_callback_message_param_1 =               (ALIGN_TYPE) thread_ptr;
            callback_message.txm_module_callback_message_param_2 =               (ALIGN_TYPE) type;
            callback_message.txm_module_callback_message_param_3 =               0;
            callback_message.txm_module_callback_message_param_4 =               0;
            callback_message.txm_module_callback_message_param_5 =               0;
            callback_message.txm_module_callback_message_param_6 =               0;
            callback_message.txm_module_callback_message_param_7 =               0;
            callback_message.txm_module_callback_message_param_8 =               0;
            callback_message.txm_module_callback_message_reserved1 =             0;
            callback_message.txm_module_callback_message_reserved2 =             0;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Call the general processing that will place the callback on the
               module's callback request queue.  */
            _txm_module_manager_callback_request(module_callback_queue, &callback_message);
        }
        else
        {

            /* Module no longer valid.  */

            /* Error, increment the error counter and return.  */
            _txm_module_manager_callback_error_count++;

            /* Restore interrupts.  */
            TX_RESTORE
        }
    }
    else
    {

        /* Thread pointer is not valid.  */

        /* Error, increment the error counter and return.  */
        _txm_module_manager_callback_error_count++;

        /* Restore interrupts.  */
        TX_RESTORE
    }
}
#endif

