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
/*    _nx_tcp_socket_thread_suspend                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function suspends a thread on a TCP service within             */
/*    NetX.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    suspension_list_head                  Pointer to the suspension list*/
/*    mutex_ptr                             Pointer to mutex to release   */
/*    suspend_cleanup                       Suspension cleanup routine    */
/*    wait_option                           Optional timeout value        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_put                          Release protection            */
/*    _tx_thread_system_suspend             Suspend thread                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_client_socket_bind            Client socket bind processing */
/*    _nx_tcp_client_socket_connect         Client socket connect         */
/*                                            processing                  */
/*    _nx_tcp_socket_disconnect             Socket disconnect processing  */
/*    _nx_tcp_socket_receive                Socket receive processing     */
/*    _nx_tcp_socket_send                   Socket send processing        */
/*    _nx_tcp_server_socket_accept          Server socket accept          */
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
VOID  _nx_tcp_socket_thread_suspend(TX_THREAD **suspension_list_head, VOID (*suspend_cleanup)(TX_THREAD * NX_CLEANUP_PARAMETER), NX_TCP_SOCKET *socket_ptr, TX_MUTEX *mutex_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD *thread_ptr;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    thread_ptr =  _tx_thread_current_ptr;

    /* Setup suspension list.  */
    if (*suspension_list_head)
    {

        /* This list is not NULL, add current thread to the end. */
        thread_ptr -> tx_thread_suspended_next =      *suspension_list_head;
        thread_ptr -> tx_thread_suspended_previous =  (*suspension_list_head) -> tx_thread_suspended_previous;
        ((*suspension_list_head) -> tx_thread_suspended_previous) -> tx_thread_suspended_next =  thread_ptr;
        (*suspension_list_head) -> tx_thread_suspended_previous =   thread_ptr;
    }
    else
    {

        /* No other threads are suspended.  Setup the head pointer and
           just setup this threads pointers to itself.  */
        *suspension_list_head =  thread_ptr;
        thread_ptr -> tx_thread_suspended_next =        thread_ptr;
        thread_ptr -> tx_thread_suspended_previous =    thread_ptr;
    }

    /* Setup cleanup routine pointer.  */
    thread_ptr -> tx_thread_suspend_cleanup =  suspend_cleanup;

    /* Setup cleanup information, i.e. this pool control
       block.  */
    thread_ptr -> tx_thread_suspend_control_block =  (void *)socket_ptr;

    /* Set the state to suspended.  */
    thread_ptr -> tx_thread_state =  TX_TCP_IP;

    /* Set the suspending flag.  */
    thread_ptr -> tx_thread_suspending =  TX_TRUE;

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Save the timeout value.  */
    thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release protection.  */
    tx_mutex_put(mutex_ptr);

    /* Call actual thread suspension routine.  */
    _tx_thread_system_suspend(thread_ptr);
}

