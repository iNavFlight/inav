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
/** NetX Component                                                        */
/**                                                                       */
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_tcp.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_thread_resume                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resumes a thread suspended on a TCP service within    */
/*    NetX.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to resume   */
/*    status                                Return status                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume              Resume suspended thread       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_client_socket_unbind          Client socket unbind          */
/*                                            processing                  */
/*    _nx_tcp_socket_connection_reset       Received reset processing     */
/*    _nx_tcp_socket_state_closing          Socket state closing          */
/*                                            processing                  */
/*    _nx_tcp_socket_state_data_check       Socket data check processing  */
/*    _nx_tcp_socket_state_fin_wait2        Socket FIN wait-2 processing  */
/*    _nx_tcp_socket_state_fin_wait1        Socket FIN wait-1 processing  */
/*    _nx_tcp_socket_state_last_ack         Socket last ack processing    */
/*    _nx_tcp_socket_state_syn_received     Socket SYN received           */
/*                                            processing                  */
/*    _nx_tcp_socket_state_syn_sent         Socket SYN sent processing    */
/*    _nx_tcp_socket_state_transmit_check   Socket transmit check         */
/*                                            processing                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tcp_socket_thread_resume(TX_THREAD **suspension_list_head, UINT status)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD *thread_ptr;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup the thread pointer.  */
    thread_ptr =  *suspension_list_head;

    /* Determine if there still is a thread suspended.  */
    if (thread_ptr)
    {

        /* Determine if there are anymore threads on the suspension list.  */
        if (thread_ptr == thread_ptr -> tx_thread_suspended_next)
        {

            /* Only this thread is on the suspension list.  Simply set the
               list head to NULL to reflect an empty suspension list.  */
            *suspension_list_head =  TX_NULL;
        }
        else
        {

            /* More than one thread is on the suspension list, we need to
               adjust the link pointers and move the next entry to the
               front of the list.  */
            *suspension_list_head =  thread_ptr -> tx_thread_suspended_next;

            /* Update the links of the adjacent threads.  */
            (thread_ptr -> tx_thread_suspended_next) -> tx_thread_suspended_previous =
                thread_ptr -> tx_thread_suspended_previous;
            (thread_ptr -> tx_thread_suspended_previous) -> tx_thread_suspended_next =
                thread_ptr -> tx_thread_suspended_next;
        }

        /* Prepare for resumption of the thread.  */

        /* Clear cleanup routine to avoid timeout.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Put return status into the thread control block.  */
        thread_ptr -> tx_thread_suspend_status =  status;

        /* Resume thread.  */
        _tx_thread_system_resume(thread_ptr);
    }
    else
    {

        /* Nothing was suspended.  Simply restore interrupts.  */
        TX_RESTORE
    }
}

