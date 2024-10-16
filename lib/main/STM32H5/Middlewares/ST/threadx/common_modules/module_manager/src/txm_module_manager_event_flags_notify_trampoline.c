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
#include "tx_event_flags.h"
#include "tx_thread.h"
#include "txm_module.h"


#ifndef TX_DISABLE_NOTIFY_CALLBACKS
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_event_flags_notify_trampoline   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the event flags set notification call from  */
/*    ThreadX.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Event flags group pointer         */
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
VOID  _txm_module_manager_event_flags_notify_trampoline(TX_EVENT_FLAGS_GROUP *group_ptr)
{

TX_INTERRUPT_SAVE_AREA

TXM_MODULE_INSTANCE         *module_instance;
TXM_MODULE_CALLBACK_MESSAGE  callback_message;
TX_QUEUE                    *module_callback_queue;


    /* We now know the callback is for a module.  */

    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup the module instance pointer.  */
    module_instance =  (TXM_MODULE_INSTANCE *) group_ptr -> tx_event_flags_group_module_instance;

    /* Determine if this module is still valid.  */
    if ((module_instance) && (module_instance -> txm_module_instance_id == TXM_MODULE_ID) &&
        (module_instance -> txm_module_instance_state == TXM_MODULE_STARTED))
    {

        /* Yes, the module is still valid.  */

        /* Pickup the module's callback message queue.  */
        module_callback_queue =  &(module_instance -> txm_module_instance_callback_request_queue);

        /* Build the queue notification message.  */
        callback_message.txm_module_callback_message_type =                  TXM_EVENTS_SET_CALLBACK;
        callback_message.txm_module_callback_message_activation_count =      1;
        callback_message.txm_module_callback_message_application_function =  (VOID (*)(VOID)) group_ptr -> tx_event_flags_group_set_module_notify;
        callback_message.txm_module_callback_message_param_1 =               (ALIGN_TYPE) group_ptr;
        callback_message.txm_module_callback_message_param_2 =               0;
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
#endif
