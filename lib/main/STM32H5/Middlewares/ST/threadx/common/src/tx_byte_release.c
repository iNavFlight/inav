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
/**   Byte Memory                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_byte_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_byte_release                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns previously allocated memory to its            */
/*    associated memory byte pool.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    memory_ptr                        Pointer to allocated memory       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    [TX_PTR_ERROR | TX_SUCCESS]       Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_preempt_check   Check for preemption              */
/*    _tx_thread_system_resume          Resume thread service             */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*    _tx_byte_pool_search              Search the byte pool for memory   */
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
UINT  _tx_byte_release(VOID *memory_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT                status;
TX_BYTE_POOL        *pool_ptr;
TX_THREAD           *thread_ptr;
UCHAR               *work_ptr;
UCHAR               *temp_ptr;
UCHAR               *next_block_ptr;
TX_THREAD           *susp_thread_ptr;
UINT                suspended_count;
TX_THREAD           *next_thread;
TX_THREAD           *previous_thread;
ULONG               memory_size;
ALIGN_TYPE          *free_ptr;
TX_BYTE_POOL        **byte_pool_ptr;
UCHAR               **block_link_ptr;
UCHAR               **suspend_info_ptr;


    /* Default to successful status.  */
    status =  TX_SUCCESS;

    /* Set the pool pointer to NULL.  */
    pool_ptr =  TX_NULL;

    /* Lockout interrupts.  */
    TX_DISABLE

    /* Determine if the memory pointer is valid.  */
    work_ptr =  TX_VOID_TO_UCHAR_POINTER_CONVERT(memory_ptr);
    if (work_ptr != TX_NULL)
    {

        /* Back off the memory pointer to pickup its header.  */
        work_ptr =  TX_UCHAR_POINTER_SUB(work_ptr, ((sizeof(UCHAR *)) + (sizeof(ALIGN_TYPE))));

        /* There is a pointer, pickup the pool pointer address.  */
        temp_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)));
        free_ptr =  TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(temp_ptr);
        if ((*free_ptr) != TX_BYTE_BLOCK_FREE)
        {

            /* Pickup the pool pointer.  */
            temp_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)));
            byte_pool_ptr =  TX_UCHAR_TO_INDIRECT_BYTE_POOL_POINTER(temp_ptr);
            pool_ptr =  *byte_pool_ptr;

            /* See if we have a valid pool pointer.  */
            if (pool_ptr == TX_NULL)
            {

                /* Return pointer error.  */
                status =  TX_PTR_ERROR;
            }
            else
            {

                /* See if we have a valid pool.  */
                if (pool_ptr -> tx_byte_pool_id != TX_BYTE_POOL_ID)
                {

                    /* Return pointer error.  */
                    status =  TX_PTR_ERROR;

                    /* Reset the pool pointer is NULL.  */
                    pool_ptr =  TX_NULL;
                }
            }
        }
        else
        {

            /* Return pointer error.  */
            status =  TX_PTR_ERROR;
        }
    }
    else
    {

        /* Return pointer error.  */
        status =  TX_PTR_ERROR;
    }

    /* Determine if the pointer is valid.  */
    if (pool_ptr == TX_NULL)
    {

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* At this point, we know that the pointer is valid.  */

        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(thread_ptr)

        /* Indicate that this thread is the current owner.  */
        pool_ptr -> tx_byte_pool_owner =  thread_ptr;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

        /* Increment the total release counter.  */
        _tx_byte_pool_performance_release_count++;

        /* Increment the number of releases on this pool.  */
        pool_ptr -> tx_byte_pool_performance_release_count++;
#endif

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_BYTE_RELEASE, pool_ptr, TX_POINTER_TO_ULONG_CONVERT(memory_ptr), pool_ptr -> tx_byte_pool_suspended_count, pool_ptr -> tx_byte_pool_available, TX_TRACE_BYTE_POOL_EVENTS)

        /* Log this kernel call.  */
        TX_EL_BYTE_RELEASE_INSERT

        /* Release the memory.  */
        temp_ptr =   TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)));
        free_ptr =   TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(temp_ptr);
        *free_ptr =  TX_BYTE_BLOCK_FREE;

        /* Update the number of available bytes in the pool.  */
        block_link_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(work_ptr);
        next_block_ptr =  *block_link_ptr;
        pool_ptr -> tx_byte_pool_available =
            pool_ptr -> tx_byte_pool_available + TX_UCHAR_POINTER_DIF(next_block_ptr, work_ptr);

        /* Determine if the free block is prior to current search pointer.  */
        if (work_ptr < (pool_ptr -> tx_byte_pool_search))
        {

            /* Yes, update the search pointer to the released block.  */
            pool_ptr -> tx_byte_pool_search =  work_ptr;
        }

        /* Determine if there are threads suspended on this byte pool.  */
        if (pool_ptr -> tx_byte_pool_suspended_count != TX_NO_SUSPENSIONS)
        {

            /* Now examine the suspension list to find threads waiting for
               memory.  Maybe it is now available!  */
            while (pool_ptr -> tx_byte_pool_suspended_count != TX_NO_SUSPENSIONS)
            {

                /* Pickup the first suspended thread pointer.  */
                susp_thread_ptr =  pool_ptr -> tx_byte_pool_suspension_list;

                /* Pickup the size of the memory the thread is requesting.  */
                memory_size =  susp_thread_ptr -> tx_thread_suspend_info;

                /* Restore interrupts.  */
                TX_RESTORE

                /* See if the request can be satisfied.  */
                work_ptr =  _tx_byte_pool_search(pool_ptr, memory_size);

                /* Optional processing extension.  */
                TX_BYTE_RELEASE_EXTENSION

                /* Disable interrupts.  */
                TX_DISABLE

                /* Indicate that this thread is the current owner.  */
                pool_ptr -> tx_byte_pool_owner =  thread_ptr;

                /* If there is not enough memory, break this loop!  */
                if (work_ptr == TX_NULL)
                {

                  /* Break out of the loop.  */
                    break;
                }

                /* Check to make sure the thread is still suspended.  */
                if (susp_thread_ptr ==  pool_ptr -> tx_byte_pool_suspension_list)
                {

                    /* Also, makes sure the memory size is the same.  */
                    if (susp_thread_ptr -> tx_thread_suspend_info == memory_size)
                    {

                        /* Remove the suspended thread from the list.  */

                        /* Decrement the number of threads suspended.  */
                        pool_ptr -> tx_byte_pool_suspended_count--;

                        /* Pickup the suspended count.  */
                        suspended_count =  pool_ptr -> tx_byte_pool_suspended_count;

                        /* See if this is the only suspended thread on the list.  */
                        if (suspended_count == TX_NO_SUSPENSIONS)
                        {

                            /* Yes, the only suspended thread.  */

                            /* Update the head pointer.  */
                            pool_ptr -> tx_byte_pool_suspension_list =  TX_NULL;
                        }
                        else
                        {

                            /* At least one more thread is on the same expiration list.  */

                            /* Update the list head pointer.  */
                            next_thread =                                susp_thread_ptr -> tx_thread_suspended_next;
                            pool_ptr -> tx_byte_pool_suspension_list =   next_thread;

                            /* Update the links of the adjacent threads.  */
                            previous_thread =                              susp_thread_ptr -> tx_thread_suspended_previous;
                            next_thread -> tx_thread_suspended_previous =  previous_thread;
                            previous_thread -> tx_thread_suspended_next =  next_thread;
                        }

                        /* Prepare for resumption of the thread.  */

                        /* Clear cleanup routine to avoid timeout.  */
                        susp_thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

                        /* Return this block pointer to the suspended thread waiting for
                           a block.  */
                        suspend_info_ptr =   TX_VOID_TO_INDIRECT_UCHAR_POINTER_CONVERT(susp_thread_ptr -> tx_thread_additional_suspend_info);
                        *suspend_info_ptr =  work_ptr;

                        /* Clear the memory pointer to indicate that it was given to the suspended thread.  */
                        work_ptr =  TX_NULL;

                        /* Put return status into the thread control block.  */
                        susp_thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;

#ifdef TX_NOT_INTERRUPTABLE

                        /* Resume the thread!  */
                        _tx_thread_system_ni_resume(susp_thread_ptr);

                        /* Restore interrupts.  */
                        TX_RESTORE
#else
                        /* Temporarily disable preemption.  */
                        _tx_thread_preempt_disable++;

                        /* Restore interrupts.  */
                        TX_RESTORE

                        /* Resume thread.  */
                        _tx_thread_system_resume(susp_thread_ptr);
#endif

                        /* Lockout interrupts.  */
                        TX_DISABLE
                    }
                }

                /* Determine if the memory was given to the suspended thread.  */
                if (work_ptr != TX_NULL)
                {

                    /* No, it wasn't given to the suspended thread.  */

                    /* Put the memory back on the available list since this thread is no longer
                       suspended.  */
                    work_ptr =  TX_UCHAR_POINTER_SUB(work_ptr, (((sizeof(UCHAR *)) + (sizeof(ALIGN_TYPE)))));
                    temp_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)));
                    free_ptr =  TX_UCHAR_TO_ALIGN_TYPE_POINTER_CONVERT(temp_ptr);
                    *free_ptr =  TX_BYTE_BLOCK_FREE;

                    /* Update the number of available bytes in the pool.  */
                    block_link_ptr =  TX_UCHAR_TO_INDIRECT_UCHAR_POINTER_CONVERT(work_ptr);
                    next_block_ptr =  *block_link_ptr;
                    pool_ptr -> tx_byte_pool_available =
                        pool_ptr -> tx_byte_pool_available + TX_UCHAR_POINTER_DIF(next_block_ptr, work_ptr);

                    /* Determine if the current pointer is before the search pointer.  */
                    if (work_ptr < (pool_ptr -> tx_byte_pool_search))
                    {

                        /* Yes, update the search pointer.  */
                        pool_ptr -> tx_byte_pool_search =  work_ptr;
                    }
                }
            }

            /* Restore interrupts.  */
            TX_RESTORE

            /* Check for preemption.  */
            _tx_thread_system_preempt_check();
        }
        else
        {

            /* No, threads suspended, restore interrupts.  */
            TX_RESTORE
        }
    }

    /* Return completion status.  */
    return(status);
}

