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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "tx_thread.h"
#include "nx_ip.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_raw_packet_cleanup                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes raw packet receive timeout and thread       */
/*    terminate actions that require the IP data structures to be         */
/*    cleaned up to remove the suspended thread.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to suspended thread's */
/*                                            control block               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume              Resume thread service         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_delete                         Delete IP instance            */
/*    _nx_ip_raw_packet_disable             Disable raw packet processing */
/*    _tx_thread_timeout                    Thread timeout processing     */
/*    _tx_thread_terminate                  Thread terminate processing   */
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
VOID  _nx_ip_raw_packet_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER)
{

TX_INTERRUPT_SAVE_AREA

NX_IP *ip_ptr;    /* Working IP pointer  */

    NX_CLEANUP_EXTENSION

    /* Setup pointer to the IP control block.  */
    ip_ptr =  (NX_IP *)thread_ptr -> tx_thread_suspend_control_block;

    /* Disable interrupts to remove the suspended thread from the block pool.  */
    TX_DISABLE

    /* Determine if the cleanup is still required.  */
    if ((thread_ptr -> tx_thread_suspend_cleanup) && (ip_ptr) &&
        (ip_ptr -> nx_ip_id == NX_IP_ID))
    {

        /* Yes, we still have thread suspension!  */

        /* Clear the suspension cleanup flag.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Remove the suspended thread from the list.  */

        /* See if this is the only suspended thread on the list.  */
        if (thread_ptr == thread_ptr -> tx_thread_suspended_next)
        {

            /* Yes, the only suspended thread.  */

            /* Update the head pointer.  */
            ip_ptr -> nx_ip_raw_packet_suspension_list =  TX_NULL;
        }
        else
        {

            /* At least one more thread is on the same suspension list.  */

            /* Update the list head pointer.  */
            ip_ptr -> nx_ip_raw_packet_suspension_list =  thread_ptr -> tx_thread_suspended_next;

            /* Update the links of the adjacent threads.  */
            (thread_ptr -> tx_thread_suspended_next) -> tx_thread_suspended_previous =
                thread_ptr -> tx_thread_suspended_previous;
            (thread_ptr -> tx_thread_suspended_previous) -> tx_thread_suspended_next =
                thread_ptr -> tx_thread_suspended_next;
        }

        /* Decrement the suspension count.  */
        ip_ptr -> nx_ip_raw_packet_suspended_count--;

        /* Now we need to determine if this cleanup is from a terminate, timeout,
           or from a wait abort.  */
        if (thread_ptr -> tx_thread_state == TX_TCP_IP)
        {

            /* Thread still suspended on the packet pool.  Setup return error status and
               resume the thread.  */

            /* Setup return status.  */
            thread_ptr -> tx_thread_suspend_status =  NX_NO_PACKET;

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Resume the thread!  Check for preemption even though we are executing
               from the system timer thread right now which normally executes at the
               highest priority.  */
            _tx_thread_system_resume(thread_ptr);

            /* Finished, just return.  */
            return;
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE
}

