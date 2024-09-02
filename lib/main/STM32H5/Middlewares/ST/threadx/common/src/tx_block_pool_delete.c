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
/**   Block Pool                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_block_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_block_pool_delete                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes the specified block pool.  All threads        */
/*    suspended on the block pool are resumed with the TX_DELETED status  */
/*    code.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to pool control block     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_preempt_check   Check for preemption              */
/*    _tx_thread_system_resume          Resume thread service             */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_block_pool_delete(TX_BLOCK_POOL *pool_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
TX_THREAD       *next_thread;
UINT            suspended_count;
TX_BLOCK_POOL   *next_pool;
TX_BLOCK_POOL   *previous_pool;


    /* Disable interrupts to remove the block pool from the created list.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BLOCK_POOL_DELETE, pool_ptr, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), 0, 0, TX_TRACE_BLOCK_POOL_EVENTS)

    /* Log this kernel call.  */
    TX_EL_BLOCK_POOL_DELETE_INSERT

    /* Optional block pool delete extended processing.  */
    TX_BLOCK_POOL_DELETE_EXTENSION(pool_ptr)

    /* If trace is enabled, unregister this object.  */
    TX_TRACE_OBJECT_UNREGISTER(pool_ptr)

    /* Clear the block pool ID to make it invalid.  */
    pool_ptr -> tx_block_pool_id =  TX_CLEAR_ID;

    /* Decrement the number of block pools.  */
    _tx_block_pool_created_count--;

    /* See if the block pool is the only one on the list.  */
    if (_tx_block_pool_created_count == TX_EMPTY)
    {

        /* Only created block pool, just set the created list to NULL.  */
        _tx_block_pool_created_ptr =  TX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        next_pool =                                    pool_ptr -> tx_block_pool_created_next;
        previous_pool =                                pool_ptr -> tx_block_pool_created_previous;
        next_pool -> tx_block_pool_created_previous =  previous_pool;
        previous_pool -> tx_block_pool_created_next =  next_pool;

        /* See if we have to update the created list head pointer.  */
        if (_tx_block_pool_created_ptr == pool_ptr)
        {

            /* Yes, move the head pointer to the next link. */
            _tx_block_pool_created_ptr =  next_pool;
        }
    }

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Pickup the suspension information.  */
    thread_ptr =                                 pool_ptr -> tx_block_pool_suspension_list;
    pool_ptr -> tx_block_pool_suspension_list =  TX_NULL;
    suspended_count =                            pool_ptr -> tx_block_pool_suspended_count;
    pool_ptr -> tx_block_pool_suspended_count =  TX_NO_SUSPENSIONS;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the block pool suspension list to resume any and all threads suspended
       on this block pool.  */
    while (suspended_count != TX_NO_SUSPENSIONS)
    {

        /* Decrement the suspension count.  */
        suspended_count--;

        /* Lockout interrupts.  */
        TX_DISABLE

        /* Clear the cleanup pointer, this prevents the timeout from doing
           anything.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Set the return status in the thread to TX_DELETED.  */
        thread_ptr -> tx_thread_suspend_status =  TX_DELETED;

        /* Move the thread pointer ahead.  */
        next_thread =  thread_ptr -> tx_thread_suspended_next;

#ifdef TX_NOT_INTERRUPTABLE

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Temporarily disable preemption again.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Resume the thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif

        /* Move to next thread.  */
        thread_ptr =  next_thread;
    }

    /* Execute Port-Specific completion processing. If needed, it is typically defined in tx_port.h.  */
    TX_BLOCK_POOL_DELETE_PORT_COMPLETION(pool_ptr)

    /* Disable interrupts.  */
    TX_DISABLE

    /* Release previous preempt disable.  */
    _tx_thread_preempt_disable--;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

