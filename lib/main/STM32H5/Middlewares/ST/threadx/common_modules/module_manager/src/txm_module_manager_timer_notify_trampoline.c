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
#include "tx_timer.h"
#include "txm_module.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_timer_notify_trampoline         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the timer expirations from ThreadX.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                                Timer ID                          */
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
VOID  _txm_module_manager_timer_notify_trampoline(ULONG id)
{

TX_INTERRUPT_SAVE_AREA

TXM_MODULE_INSTANCE         *module_instance;
TXM_MODULE_CALLBACK_MESSAGE  callback_message;
TX_QUEUE                    *module_callback_queue;
TX_TIMER                    *timer_ptr;
CHAR                        *internal_ptr;


    /* We now know the callback is for a module.  */

    /* Disable interrupts.  */
    TX_DISABLE

    /* Our expired timer pointer points to the internal timer,
     * we need to get to the full timer pointer.  */
    /* Pickup the current internal timer pointer.  */
    internal_ptr =  (CHAR *) _tx_timer_expired_timer_ptr;

    /* Get the timer pointer from the internal pointer.  */
    TX_USER_TIMER_POINTER_GET((TX_TIMER_INTERNAL *) internal_ptr, timer_ptr);

    /* Pickup the module instance pointer.  */
    module_instance =  (TXM_MODULE_INSTANCE *) timer_ptr -> tx_timer_module_instance;

    /* Determine if this module is still valid.  */
    if ((module_instance) && (module_instance -> txm_module_instance_id == TXM_MODULE_ID) &&
        (module_instance -> txm_module_instance_state == TXM_MODULE_STARTED))
    {

        /* Yes, the module is still valid.  */

        /* Pickup the module's callback message queue.  */
        module_callback_queue =  &(module_instance -> txm_module_instance_callback_request_queue);

        /* Build the queue notification message.  */
        callback_message.txm_module_callback_message_type =                  TXM_TIMER_CALLBACK;
        callback_message.txm_module_callback_message_activation_count =      1;
        callback_message.txm_module_callback_message_application_function =  (VOID (*)(VOID)) timer_ptr -> tx_timer_module_expiration_function;
        callback_message.txm_module_callback_message_param_1 =               (ULONG) id;
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

