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
#include "tx_thread.h"
#include "tx_block_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_block_pool_cleanup                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes block allocate timeout and thread terminate */
/*    actions that require the block pool data structures to be cleaned   */
/*    up.                                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Pointer to suspended thread's     */
/*                                        control block                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume          Resume thread service             */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_timeout                Thread timeout processing         */
/*    _tx_thread_terminate              Thread terminate processing       */
/*    _tx_thread_wait_abort             Thread wait abort processing      */
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
VOID  _tx_block_pool_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence)
{

#ifndef TX_NOT_INTERRUPTABLE
TX_INTERRUPT_SAVE_AREA
#endif

TX_BLOCK_POOL       *pool_ptr;
UINT                suspended_count;
TX_THREAD           *next_thread;
TX_THREAD           *previous_thread;


#ifndef TX_NOT_INTERRUPTABLE

    /* Disable interrupts to remove the suspended thread from the block pool.  */
    TX_DISABLE

    /* Determine if the cleanup is still required.  */
    if (thread_ptr -> tx_thread_suspend_cleanup == &(_tx_block_pool_cleanup))
    {

        /* Check for valid suspension sequence.  */
        if (suspension_sequence == thread_ptr -> tx_thread_suspension_sequence)
        {

            /* Setup pointer to block pool control block.  */
            pool_ptr =  TX_VOID_TO_BLOCK_POOL_POINTER_CONVERT(thread_ptr -> tx_thread_suspend_control_block);

            /* Check for a NULL byte pool pointer.  */
            if (pool_ptr != TX_NULL)
            {

                /* Check for valid pool ID.  */
                if (pool_ptr -> tx_block_pool_id == TX_BLOCK_POOL_ID)
                {

                    /* Determine if there are any thread suspensions.  */
                    if (pool_ptr -> tx_block_pool_suspended_count != TX_NO_SUSPENSIONS)
                    {
#else

                        /* Setup pointer to block pool control block.  */
                        pool_ptr =  TX_VOID_TO_BLOCK_POOL_POINTER_CONVERT(thread_ptr -> tx_thread_suspend_control_block);
#endif

                        /* Yes, we still have thread suspension!  */

                        /* Clear the suspension cleanup flag.  */
                        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

                        /* Decrement the suspended count.  */
                        pool_ptr -> tx_block_pool_suspended_count--;

                        /* Pickup the suspended count.  */
                        suspended_count =  pool_ptr -> tx_block_pool_suspended_count;

                        /* Remove the suspended thread from the list.  */

                        /* See if this is the only suspended thread on the list.  */
                        if (suspended_count == TX_NO_SUSPENSIONS)
                        {

                            /* Yes, the only suspended thread.  */

                            /* Update the head pointer.  */
                            pool_ptr -> tx_block_pool_suspension_list =  TX_NULL;
                        }
                        else
                        {

                            /* At least one more thread is on the same suspension list.  */

                            /* Update the links of the adjacent threads.  */
                            next_thread =                                   thread_ptr -> tx_thread_suspended_next;
                            previous_thread =                               thread_ptr -> tx_thread_suspended_previous;
                            next_thread -> tx_thread_suspended_previous =   previous_thread;
                            previous_thread -> tx_thread_suspended_next =   next_thread;

                            /* Determine if we need to update the head pointer.  */
                            if (pool_ptr -> tx_block_pool_suspension_list == thread_ptr)
                            {

                                /* Update the list head pointer.  */
                                pool_ptr -> tx_block_pool_suspension_list =     next_thread;
                            }
                        }

                        /* Now we need to determine if this cleanup is from a terminate, timeout,
                           or from a wait abort.  */
                        if (thread_ptr -> tx_thread_state == TX_BLOCK_MEMORY)
                        {

                            /* Timeout condition and the thread still suspended on the block pool.
                               Setup return error status and resume the thread.  */

#ifdef TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO

                            /* Increment the total timeouts counter.  */
                            _tx_block_pool_performance_timeout_count++;

                            /* Increment the number of timeouts on this block pool.  */
                            pool_ptr -> tx_block_pool_performance_timeout_count++;
#endif

                            /* Setup return status.  */
                            thread_ptr -> tx_thread_suspend_status =  TX_NO_MEMORY;

#ifdef TX_NOT_INTERRUPTABLE

                            /* Resume the thread!  */
                            _tx_thread_system_ni_resume(thread_ptr);
#else
                            /* Temporarily disable preemption.  */
                            _tx_thread_preempt_disable++;

                            /* Restore interrupts.  */
                            TX_RESTORE

                            /* Resume the thread!  */
                            _tx_thread_system_resume(thread_ptr);

                            /* Disable interrupts.  */
                            TX_DISABLE
#endif
                        }
#ifndef TX_NOT_INTERRUPTABLE
                    }
                }
            }
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE
#endif
}

