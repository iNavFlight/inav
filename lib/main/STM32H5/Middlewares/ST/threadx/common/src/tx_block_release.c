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
/*    _tx_block_release                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns a previously allocated block to its           */
/*    associated memory block pool.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    block_ptr                         Pointer to memory block           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
UINT  _tx_block_release(VOID *block_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_BLOCK_POOL       *pool_ptr;
TX_THREAD           *thread_ptr;
UCHAR               *work_ptr;
UCHAR               **return_block_ptr;
UCHAR               **next_block_ptr;
UINT                suspended_count;
TX_THREAD           *next_thread;
TX_THREAD           *previous_thread;


    /* Disable interrupts to put this block back in the pool.  */
    TX_DISABLE

    /* Pickup the pool pointer which is just previous to the starting
       address of the block that the caller sees.  */
    work_ptr =        TX_VOID_TO_UCHAR_POINTER_CONVERT(block_ptr);
    work_ptr =        TX_UCHAR_POINTER_SUB(work_ptr, (sizeof(UCHAR *)));
    next_block_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(work_ptr);
    pool_ptr =        TX_UCHAR_TO_BLOCK_POOL_POINTER_CONVERT((*next_block_ptr));

#ifdef TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO

    /* Increment the total releases counter.  */
    _tx_block_pool_performance_release_count++;

    /* Increment the number of releases on this pool.  */
    pool_ptr -> tx_block_pool_performance_release_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BLOCK_RELEASE, pool_ptr, TX_POINTER_TO_ULONG_CONVERT(block_ptr), pool_ptr -> tx_block_pool_suspended_count, TX_POINTER_TO_ULONG_CONVERT(&work_ptr), TX_TRACE_BLOCK_POOL_EVENTS)

    /* Log this kernel call.  */
    TX_EL_BLOCK_RELEASE_INSERT

    /* Determine if there are any threads suspended on the block pool.  */
    thread_ptr =  pool_ptr -> tx_block_pool_suspension_list;
    if (thread_ptr != TX_NULL)
    {

        /* Remove the suspended thread from the list.  */

        /* Decrement the number of threads suspended.  */
        (pool_ptr -> tx_block_pool_suspended_count)--;

        /* Pickup the suspended count.  */
        suspended_count =  (pool_ptr -> tx_block_pool_suspended_count);

        /* See if this is the only suspended thread on the list.  */
        if (suspended_count == TX_NO_SUSPENSIONS)
        {

            /* Yes, the only suspended thread.  */

            /* Update the head pointer.  */
            pool_ptr -> tx_block_pool_suspension_list =  TX_NULL;
        }
        else
        {

            /* At least one more thread is on the same expiration list.  */

            /* Update the list head pointer.  */
            next_thread =                                thread_ptr -> tx_thread_suspended_next;
            pool_ptr -> tx_block_pool_suspension_list =  next_thread;

            /* Update the links of the adjacent threads.  */
            previous_thread =                              thread_ptr -> tx_thread_suspended_previous;
            next_thread -> tx_thread_suspended_previous =  previous_thread;
            previous_thread -> tx_thread_suspended_next =  next_thread;
        }

        /* Prepare for resumption of the first thread.  */

        /* Clear cleanup routine to avoid timeout.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Return this block pointer to the suspended thread waiting for
           a block.  */
        return_block_ptr =  TX_VOID_TO_INDIRECT_UCHAR_POINTER_CONVERT(thread_ptr -> tx_thread_additional_suspend_info);
        work_ptr =          TX_VOID_TO_UCHAR_POINTER_CONVERT(block_ptr);
        *return_block_ptr =  work_ptr;

        /* Put return status into the thread control block.  */
        thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;

#ifdef TX_NOT_INTERRUPTABLE

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Resume thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif
    }
    else
    {

        /* No thread is suspended for a memory block.  */

        /* Put the block back in the available list.  */
        *next_block_ptr =  pool_ptr -> tx_block_pool_available_list;

        /* Adjust the head pointer.  */
        pool_ptr -> tx_block_pool_available_list =  work_ptr;

        /* Increment the count of available blocks.  */
        pool_ptr -> tx_block_pool_available++;

        /* Restore interrupts.  */
        TX_RESTORE
    }

    /* Return successful completion status.  */
    return(TX_SUCCESS);
}

