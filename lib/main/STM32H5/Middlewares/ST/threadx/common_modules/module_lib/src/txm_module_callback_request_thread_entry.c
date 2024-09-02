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
/**   Module                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef TXM_MODULE
#define TXM_MODULE
#endif

#ifndef TX_SOURCE_CODE
#define TX_SOURCE_CODE
#endif


/* Include necessary system files.  */

#include "txm_module.h"
#include "tx_queue.h"


/* Define the global module entry pointer from the start thread of the module.
   This structure contains the pointer to the request queue as well as the
   pointer to the callback response queue.  */

extern TXM_MODULE_THREAD_ENTRY_INFO    *_txm_module_entry_info;

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_callback_request_thread_entry           PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes all module callback requests, transferred   */
/*    by the resident code via the callback queue. When the callback is   */
/*    complete, the response is sent back to the resident code to         */
/*    acknowledge it.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                                Module thread ID                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_queue_receive                  Receive callback request          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Initial thread stack frame                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*  01-31-2022      Scott Larson            Modified comments and added   */
/*                                            CALL_NOT_USED option,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID _txm_module_callback_request_thread_entry(ULONG id)
{

TX_QUEUE                    *request_queue;
TXM_MODULE_CALLBACK_MESSAGE callback_message;
ULONG                       activation_count;
VOID                        (*timer_callback)(ULONG);
VOID                        (*events_set_notify)(TX_EVENT_FLAGS_GROUP *);
VOID                        (*semaphore_put_notify)(TX_SEMAPHORE *);
VOID                        (*queue_send_notify)(TX_QUEUE *);
VOID                        (*thread_entry_exit_notify)(TX_THREAD *, UINT);
UINT                        status;

    /* Disable warning of parameter not used. */
    TX_PARAMETER_NOT_USED(id);

    /* Pickup pointer to the request queue.  */
    request_queue =  _txm_module_entry_info -> txm_module_thread_entry_info_callback_request_queue;

    /* Loop to process callback messages from the module manager.  */
    while(1)
    {

        /* Wait for the callback request for the module.  */
        status =  _txe_queue_receive(request_queue, (VOID *) &callback_message, TX_WAIT_FOREVER);

        /* Check to see if a request was received.  */
        if (status != TX_SUCCESS)
        {

            /* This should not happen - get out of the loop.  */
            break;
        }

        /* Pickup the activation count in the message.  */
        activation_count =  callback_message.txm_module_callback_message_activation_count;

        /* Loop to call the callback function the correct number of times.  */
        while (activation_count)
        {

            /* Decrement the activation count.  */
            activation_count--;

            /* Now dispatch the callback function.  */
            switch (callback_message.txm_module_callback_message_type)
            {

            case TXM_TIMER_CALLBACK:

                /* Setup timer callback pointer.  */
                timer_callback =  (void (*)(ULONG)) callback_message.txm_module_callback_message_application_function;

                /* Call application's timer callback.  */
                (timer_callback)((ULONG) callback_message.txm_module_callback_message_param_1);

                break;

            case TXM_EVENTS_SET_CALLBACK:

                /* Setup events set callback pointer.  */
                events_set_notify =  (void (*)(TX_EVENT_FLAGS_GROUP *)) callback_message.txm_module_callback_message_application_function;

                /* Call events set notify callback.  */
                (events_set_notify)((TX_EVENT_FLAGS_GROUP *) callback_message.txm_module_callback_message_param_1);

                break;

            case TXM_QUEUE_SEND_CALLBACK:

                /* Setup queue send callback pointer.  */
                queue_send_notify =  (void (*)(TX_QUEUE *)) callback_message.txm_module_callback_message_application_function;

                /* Call queue send notify callback.  */
                (queue_send_notify)((TX_QUEUE *) callback_message.txm_module_callback_message_param_1);

                break;

            case TXM_SEMAPHORE_PUT_CALLBACK:

                /* Setup semaphore put callback pointer.  */
                semaphore_put_notify =  (void (*)(TX_SEMAPHORE *)) callback_message.txm_module_callback_message_application_function;

                /* Call semaphore put notify callback.  */
                (semaphore_put_notify)((TX_SEMAPHORE *) callback_message.txm_module_callback_message_param_1);

                break;

            case TXM_THREAD_ENTRY_EXIT_CALLBACK:

                /* Setup thread entry/exit callback pointer.  */
                thread_entry_exit_notify =  (void (*)(TX_THREAD *, UINT)) callback_message.txm_module_callback_message_application_function;

                /* Call thread entry/exit notify callback.  */
                (thread_entry_exit_notify)((TX_THREAD *) callback_message.txm_module_callback_message_param_1, (UINT) callback_message.txm_module_callback_message_param_2);

                break;

            default:

#ifdef TXM_MODULE_ENABLE_NETX

                /* Determine if there is a NetX callback.  */
                if ((callback_message.txm_module_callback_message_type >= TXM_NETX_CALLBACKS_START) && (callback_message.txm_module_callback_message_type < TXM_NETX_CALLBACKS_END))
                {

                    /* Call the NetX module callback function.  */
                    _txm_module_netx_callback_request(&callback_message);
                }
#endif

#ifdef TXM_MODULE_ENABLE_NETXDUO

                /* Determine if there is a NetX Duo callback.  */
                if ((callback_message.txm_module_callback_message_type >= TXM_NETXDUO_CALLBACKS_START) && (callback_message.txm_module_callback_message_type < TXM_NETXDUO_CALLBACKS_END))
                {

                    /* Call the NetX Duo module callback function.  */
                    _txm_module_netxduo_callback_request(&callback_message);
                }
#endif

#ifdef TXM_MODULE_ENABLE_FILEX

                /* Determine if there is a FileX callback.  */
                if ((callback_message.txm_module_callback_message_type >= TXM_FILEX_CALLBACKS_START) && (callback_message.txm_module_callback_message_type < TXM_FILEX_CALLBACKS_END))
                {

                    /* Call the FileX module callback function.  */
                    _txm_module_filex_callback_request(&callback_message);
                }
#endif

#ifdef TXM_MODULE_ENABLE_GUIX

                /* Determine if there is a GUIX callback.  */
                if ((callback_message.txm_module_callback_message_type >= TXM_GUIX_CALLBACKS_START) && (callback_message.txm_module_callback_message_type < TXM_GUIX_CALLBACKS_END))
                {

                    /* Call the GUIX module callback function.  */
                    _txm_module_guix_callback_request(&callback_message);
                }
#endif

#ifdef TXM_MODULE_ENABLE_USBX

                /* Determine if there is a USBX callback.  */
                if ((callback_message.txm_module_callback_message_type >= TXM_USBX_CALLBACKS_START) && (callback_message.txm_module_callback_message_type < TXM_USBX_CALLBACKS_END))
                {

                    /* Call the USBX callback function.  */
                    _txm_module_usbx_callback_request(&callback_message);
                }
#endif

                break;
            }
        }
    }
}
