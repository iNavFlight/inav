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
/**   Packet Pool Management (Packet)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_packet_pool_delete                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a previously created packet pool.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                              Packet pool control block     */
/*                                            pointer                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Return status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume              Resume threads suspended      */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nx_packet_pool_delete(NX_PACKET_POOL *pool_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD *thread_ptr;      /* Working thread pointer  */


    /* Disable interrupts to remove the packet pool from the created list.  */
    TX_DISABLE

    /* Decrement the number of packet pools created.  */
    _nx_packet_pool_created_count--;

    /* Clear the packet pool ID to make it invalid.  */
    pool_ptr -> nx_packet_pool_id =  0;

    /* See if the packet pool only one on the list.  */
    if (pool_ptr == pool_ptr -> nx_packet_pool_created_next)
    {

        /* Only created packet pool, just set the created list to NULL.  */
        _nx_packet_pool_created_ptr =  NX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        (pool_ptr -> nx_packet_pool_created_next) -> nx_packet_pool_created_previous =
            pool_ptr -> nx_packet_pool_created_previous;
        (pool_ptr -> nx_packet_pool_created_previous) -> nx_packet_pool_created_next =
            pool_ptr -> nx_packet_pool_created_next;

        /* See if we have to update the created list head pointer.  */
        if (_nx_packet_pool_created_ptr == pool_ptr)
        {

            /* Yes, move the head pointer to the next link. */
            _nx_packet_pool_created_ptr =  pool_ptr -> nx_packet_pool_created_next;
        }
    }

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the packet pool suspension list to resume any and all
       threads suspended on this packet pool.  */
    thread_ptr =  pool_ptr -> nx_packet_pool_suspension_list;
    while (pool_ptr -> nx_packet_pool_suspended_count)
    {
        /* Lockout interrupts.  */
        TX_DISABLE

        /* Clear the cleanup pointer, this prevents the timeout from doing
           anything.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Temporarily disable preemption again.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Set the return status in the thread to NX_POOL_DELETED.  */
        thread_ptr -> tx_thread_suspend_status =  NX_POOL_DELETED;

        /* Move the thread pointer ahead.  */
        thread_ptr =  thread_ptr -> tx_thread_suspended_next;

        /* Resume the thread.  */
        _tx_thread_system_resume(thread_ptr -> tx_thread_suspended_previous);

        /* Decrease the suspended count.  */
        pool_ptr -> nx_packet_pool_suspended_count--;
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Release previous preempt disable.  */
    _tx_thread_preempt_disable--;

    /* Restore interrupts.  */
    TX_RESTORE

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_PACKET_POOL_DELETE, pool_ptr, 0, 0, 0, NX_TRACE_PACKET_EVENTS, 0, 0);

    /* If trace is enabled, unregister this object.  */
    NX_TRACE_OBJECT_UNREGISTER(pool_ptr);

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return NX_SUCCESS.  */
    return(NX_SUCCESS);
}

