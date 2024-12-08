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
#ifdef TX_ENABLE_EVENT_TRACE
#include "tx_trace.h"
#endif
#include "tx_thread.h"
#include "tx_block_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_block_allocate                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a block from the specified memory block     */
/*    pool.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to pool control block     */
/*    block_ptr                         Pointer to place allocated block  */
/*                                        pointer                         */
/*    wait_option                       Suspension option                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_suspend         Suspend thread                    */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
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
UINT  _tx_block_allocate(TX_BLOCK_POOL *pool_ptr, VOID **block_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

UINT                        status;
TX_THREAD                   *thread_ptr;
UCHAR                       *work_ptr;
UCHAR                       *temp_ptr;
UCHAR                       **next_block_ptr;
UCHAR                       **return_ptr;
UINT                        suspended_count;
TX_THREAD                   *next_thread;
TX_THREAD                   *previous_thread;
#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY       *entry_ptr;
ULONG                       time_stamp =  ((ULONG) 0);
#endif
#ifdef TX_ENABLE_EVENT_LOGGING
UCHAR                       *log_entry_ptr;
ULONG                       upper_tbu;
ULONG                       lower_tbu;
#endif


    /* Disable interrupts to get a block from the pool.  */
    TX_DISABLE

#ifdef TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO

    /* Increment the total allocations counter.  */
    _tx_block_pool_performance_allocate_count++;

    /* Increment the number of allocations on this pool.  */
    pool_ptr -> tx_block_pool_performance_allocate_count++;
#endif

#ifdef TX_ENABLE_EVENT_TRACE

    /* If trace is enabled, save the current event pointer.  */
    entry_ptr =  _tx_trace_buffer_current_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BLOCK_ALLOCATE, pool_ptr, 0, wait_option, pool_ptr -> tx_block_pool_available, TX_TRACE_BLOCK_POOL_EVENTS)

    /* Save the time stamp for later comparison to verify that
       the event hasn't been overwritten by the time the allocate
       call succeeds.  */
    if (entry_ptr != TX_NULL)
    {

        time_stamp =  entry_ptr -> tx_trace_buffer_entry_time_stamp;
    }
#endif

#ifdef TX_ENABLE_EVENT_LOGGING
    log_entry_ptr =  *(UCHAR **) _tx_el_current_event;

    /* Log this kernel call.  */
    TX_EL_BLOCK_ALLOCATE_INSERT

    /* Store -1 in the third event slot.  */
    *((ULONG *) (log_entry_ptr + TX_EL_EVENT_INFO_3_OFFSET)) =  (ULONG) -1;

    /* Save the time stamp for later comparison to verify that
       the event hasn't been overwritten by the time the allocate
       call succeeds.  */
    lower_tbu =  *((ULONG *) (log_entry_ptr + TX_EL_EVENT_TIME_LOWER_OFFSET));
    upper_tbu =  *((ULONG *) (log_entry_ptr + TX_EL_EVENT_TIME_UPPER_OFFSET));
#endif

    /* Determine if there is an available block.  */
    if (pool_ptr -> tx_block_pool_available != ((UINT) 0))
    {

        /* Yes, a block is available.  Decrement the available count.  */
        pool_ptr -> tx_block_pool_available--;

        /* Pickup the current block pointer.  */
        work_ptr =  pool_ptr -> tx_block_pool_available_list;

        /* Return the first available block to the caller.  */
        temp_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)));
        return_ptr =  TX_INDIRECT_VOID_TO_UCHAR_POINTER_CONVERT(block_ptr);
        *return_ptr =  temp_ptr;

        /* Modify the available list to point at the next block in the pool. */
        next_block_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(work_ptr);
        pool_ptr -> tx_block_pool_available_list =  *next_block_ptr;

        /* Save the pool's address in the block for when it is released!  */
        temp_ptr =  TX_BLOCK_POOL_TO_UCHAR_POINTER_CONVERT(pool_ptr);
        *next_block_ptr =  temp_ptr;

#ifdef TX_ENABLE_EVENT_TRACE

        /* Check that the event time stamp is unchanged.  A different
           timestamp means that a later event wrote over the byte
           allocate event.  In that case, do nothing here.  */
        if (entry_ptr != TX_NULL)
        {

            /* Is the time stamp the same?  */
            if (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp)
            {

                /* Timestamp is the same, update the entry with the address.  */
#ifdef TX_MISRA_ENABLE
                entry_ptr -> tx_trace_buffer_entry_info_2 =  TX_POINTER_TO_ULONG_CONVERT(*block_ptr);
#else
                entry_ptr -> tx_trace_buffer_entry_information_field_2 =  TX_POINTER_TO_ULONG_CONVERT(*block_ptr);
#endif
            }
        }
#endif

#ifdef TX_ENABLE_EVENT_LOGGING
        /* Store the address of the allocated block.  */
        *((ULONG *) (log_entry_ptr + TX_EL_EVENT_INFO_3_OFFSET)) =  (ULONG) *block_ptr;
#endif

        /* Set status to success.  */
        status =  TX_SUCCESS;

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Default the return pointer to NULL.  */
        return_ptr =   TX_INDIRECT_VOID_TO_UCHAR_POINTER_CONVERT(block_ptr);
        *return_ptr =  TX_NULL;

        /* Determine if the request specifies suspension.  */
        if (wait_option != TX_NO_WAIT)
        {

            /* Determine if the preempt disable flag is non-zero.  */
            if (_tx_thread_preempt_disable != ((UINT) 0))
            {

                /* Suspension is not allowed if the preempt disable flag is non-zero at this point, return error completion.  */
                status =  TX_NO_MEMORY;

                /* Restore interrupts.  */
                TX_RESTORE
            }
            else
            {

                /* Prepare for suspension of this thread.  */

#ifdef TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO

                /* Increment the total suspensions counter.  */
                _tx_block_pool_performance_suspension_count++;

                /* Increment the number of suspensions on this pool.  */
                pool_ptr -> tx_block_pool_performance_suspension_count++;
#endif

                /* Pickup thread pointer.  */
                TX_THREAD_GET_CURRENT(thread_ptr)

                /* Setup cleanup routine pointer.  */
                thread_ptr -> tx_thread_suspend_cleanup =  &(_tx_block_pool_cleanup);

                /* Setup cleanup information, i.e. this pool control
                   block.  */
                thread_ptr -> tx_thread_suspend_control_block =  (VOID *) pool_ptr;

                /* Save the return block pointer address as well.  */
                thread_ptr -> tx_thread_additional_suspend_info =  (VOID *) block_ptr;

#ifndef TX_NOT_INTERRUPTABLE

                /* Increment the suspension sequence number, which is used to identify
                   this suspension event.  */
                thread_ptr -> tx_thread_suspension_sequence++;
#endif

                /* Pickup the number of suspended threads.  */
                suspended_count =  (pool_ptr -> tx_block_pool_suspended_count);

                /* Increment the number of suspended threads.  */
                (pool_ptr -> tx_block_pool_suspended_count)++;

                /* Setup suspension list.  */
                if (suspended_count == TX_NO_SUSPENSIONS)
                {

                    /* No other threads are suspended.  Setup the head pointer and
                       just setup this threads pointers to itself.  */
                    pool_ptr -> tx_block_pool_suspension_list =     thread_ptr;
                    thread_ptr -> tx_thread_suspended_next =        thread_ptr;
                    thread_ptr -> tx_thread_suspended_previous =    thread_ptr;
                }
                else
                {

                    /* This list is not NULL, add current thread to the end. */
                    next_thread =                                   pool_ptr -> tx_block_pool_suspension_list;
                    thread_ptr -> tx_thread_suspended_next =        next_thread;
                    previous_thread =                               next_thread -> tx_thread_suspended_previous;
                    thread_ptr -> tx_thread_suspended_previous =    previous_thread;
                    previous_thread -> tx_thread_suspended_next =   thread_ptr;
                    next_thread -> tx_thread_suspended_previous =   thread_ptr;
                }

                /* Set the state to suspended.  */
                thread_ptr -> tx_thread_state =       TX_BLOCK_MEMORY;

#ifdef TX_NOT_INTERRUPTABLE

                /* Call actual non-interruptable thread suspension routine.  */
                _tx_thread_system_ni_suspend(thread_ptr, wait_option);

                /* Restore interrupts.  */
                TX_RESTORE
#else

                /* Set the suspending flag.  */
                thread_ptr -> tx_thread_suspending =  TX_TRUE;

                /* Setup the timeout period.  */
                thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

                /* Temporarily disable preemption.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Call actual thread suspension routine.  */
                _tx_thread_system_suspend(thread_ptr);
#endif

#ifdef TX_ENABLE_EVENT_TRACE

                /* Check that the event time stamp is unchanged.  A different
                   timestamp means that a later event wrote over the byte
                   allocate event.  In that case, do nothing here.  */
                if (entry_ptr != TX_NULL)
                {

                    /* Is the time-stamp the same?  */
                    if (time_stamp == entry_ptr -> tx_trace_buffer_entry_time_stamp)
                    {

                        /* Timestamp is the same, update the entry with the address.  */
#ifdef TX_MISRA_ENABLE
                        entry_ptr -> tx_trace_buffer_entry_info_2 =  TX_POINTER_TO_ULONG_CONVERT(*block_ptr);
#else
                        entry_ptr -> tx_trace_buffer_entry_information_field_2 =  TX_POINTER_TO_ULONG_CONVERT(*block_ptr);
#endif
                    }
                }
#endif

#ifdef TX_ENABLE_EVENT_LOGGING
                /* Check that the event time stamp is unchanged and the call is about
                   to return success.  A different timestamp means that a later event
                   wrote over the block allocate event.  A return value other than
                   TX_SUCCESS indicates that no block was available. In those cases,
                   do nothing here.  */
                if (lower_tbu ==  *((ULONG *) (log_entry_ptr + TX_EL_EVENT_TIME_LOWER_OFFSET)) &&
                    upper_tbu ==  *((ULONG *) (log_entry_ptr + TX_EL_EVENT_TIME_UPPER_OFFSET)) &&
                    ((thread_ptr -> tx_thread_suspend_status) == TX_SUCCESS))
                {

                    /* Store the address of the allocated block.  */
                    *((ULONG *) (log_entry_ptr + TX_EL_EVENT_INFO_3_OFFSET)) =  (ULONG) *block_ptr;
                }
#endif

                /* Return the completion status.  */
                status =  thread_ptr -> tx_thread_suspend_status;
            }
        }
        else
        {

            /* Immediate return, return error completion.  */
            status =  TX_NO_MEMORY;

            /* Restore interrupts.  */
            TX_RESTORE
        }
    }

    /* Return completion status.  */
    return(status);
}

