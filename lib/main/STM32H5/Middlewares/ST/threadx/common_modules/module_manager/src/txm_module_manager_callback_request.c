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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_callback_request                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a notification callback function request to     */
/*    the associated module.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_callback_queue             Module callback request queue     */
/*    callback_request                  Callback request                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_queue_send                     Send module callback request      */
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
VOID  _txm_module_manager_callback_request(TX_QUEUE *module_callback_queue, TXM_MODULE_CALLBACK_MESSAGE  *callback_message)
{

TX_INTERRUPT_SAVE_AREA

TXM_MODULE_CALLBACK_MESSAGE      *queued_message;
UINT                            enqueued;
UINT                            found;
UINT                            status;


    /* Lockout interrupts.  */
    TX_DISABLE

    /* Determine if the queue is valid.  */
    if ((module_callback_queue) && (module_callback_queue -> tx_queue_id == TX_QUEUE_ID))
    {

        /* Yes, the queue is valid.  */

        /* Pickup the current callback request in the queue.  */
        queued_message =  (TXM_MODULE_CALLBACK_MESSAGE *) module_callback_queue -> tx_queue_read;

        /* Pickup the number of items enqueued.  */
        enqueued =  module_callback_queue -> tx_queue_enqueued;

        /* Set the found flag to false.  */
        found =  TX_FALSE;

        /* Loop to look for duplicates in the queue.  */
        while (enqueued != 0)
        {

            /* Does this entry match the new callback message?  */
            if ((queued_message -> txm_module_callback_message_application_function == callback_message -> txm_module_callback_message_application_function) &&
                (queued_message -> txm_module_callback_message_param_1 == callback_message -> txm_module_callback_message_param_1) &&
                (queued_message -> txm_module_callback_message_param_2 == callback_message -> txm_module_callback_message_param_2) &&
                (queued_message -> txm_module_callback_message_param_3 == callback_message -> txm_module_callback_message_param_3) &&
                (queued_message -> txm_module_callback_message_param_4 == callback_message -> txm_module_callback_message_param_4) &&
                (queued_message -> txm_module_callback_message_param_5 == callback_message -> txm_module_callback_message_param_5) &&
                (queued_message -> txm_module_callback_message_param_6 == callback_message -> txm_module_callback_message_param_6) &&
                (queued_message -> txm_module_callback_message_param_7 == callback_message -> txm_module_callback_message_param_7) &&
                (queued_message -> txm_module_callback_message_param_8 == callback_message -> txm_module_callback_message_param_8) &&
                (queued_message -> txm_module_callback_message_reserved1 == callback_message -> txm_module_callback_message_reserved1) &&
                (queued_message -> txm_module_callback_message_reserved2 == callback_message -> txm_module_callback_message_reserved2))
            {

                /* Update the activation count in the queued request.  */
                queued_message -> txm_module_callback_message_activation_count++;

                /* Set the found flag to true.  */
                found =  TX_TRUE;

                /* Get out of the loop.  */
                break;
            }

            /* Decrease the number of messages to examine.  */
            enqueued--;

            /* Move the callback message to the next message.  */
            queued_message++;

            /* Check for wrap?  */
            if (((ULONG *) queued_message) >= module_callback_queue -> tx_queue_end)
            {

                /* Yes, set the queued message to the beginning of the queue.  */
                queued_message =  (TXM_MODULE_CALLBACK_MESSAGE *) module_callback_queue -> tx_queue_start;
            }
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Determine if we need to send the new callback request.  */
        if (found == TX_FALSE)
        {

            /* Yes, send the message.  */
            status =  _tx_queue_send(module_callback_queue, (VOID *) callback_message, TX_NO_WAIT);

            /* Determine if an error was detected.  */
            if (status != TX_SUCCESS)
            {

                /* Error, increment the error counter and return.  */
                _txm_module_manager_callback_error_count++;
            }
        }

        /* Increment the total number of callbacks.  */
        _txm_module_manager_callback_total_count++;
    }
    else
    {

        /* Module instance is not valid.  */

        /* Error, increment the error counter and return.  */
        _txm_module_manager_callback_error_count++;

        /* Restore interrupts.  */
        TX_RESTORE
    }
}

