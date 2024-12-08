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
/**   Queue                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_queue.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_queue_cleanup                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes queue timeout and thread terminate          */
/*    actions that require the queue data structures to be cleaned        */
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
VOID  _tx_queue_cleanup(TX_THREAD  *thread_ptr, ULONG suspension_sequence)
{

#ifndef TX_NOT_INTERRUPTABLE
TX_INTERRUPT_SAVE_AREA
#endif

TX_QUEUE            *queue_ptr;
UINT                suspended_count;
TX_THREAD           *next_thread;
TX_THREAD           *previous_thread;


#ifndef TX_NOT_INTERRUPTABLE

    /* Disable interrupts to remove the suspended thread from the queue.  */
    TX_DISABLE

    /* Determine if the cleanup is still required.  */
    if (thread_ptr -> tx_thread_suspend_cleanup == &(_tx_queue_cleanup))
    {

        /* Check for valid suspension sequence.  */
        if (suspension_sequence == thread_ptr -> tx_thread_suspension_sequence)
        {

            /* Setup pointer to queue control block.  */
            queue_ptr =  TX_VOID_TO_QUEUE_POINTER_CONVERT(thread_ptr -> tx_thread_suspend_control_block);

            /* Check for NULL queue pointer.  */
            if (queue_ptr != TX_NULL)
            {

                /* Is the queue ID valid?  */
                if (queue_ptr -> tx_queue_id == TX_QUEUE_ID)
                {

                    /* Determine if there are any thread suspensions.  */
                    if (queue_ptr -> tx_queue_suspended_count != TX_NO_SUSPENSIONS)
                    {
#else

                        /* Setup pointer to queue control block.  */
                        queue_ptr =  TX_VOID_TO_QUEUE_POINTER_CONVERT(thread_ptr -> tx_thread_suspend_control_block);
#endif

                        /* Yes, we still have thread suspension!  */

                        /* Clear the suspension cleanup flag.  */
                        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

                        /* Decrement the suspended count.  */
                        queue_ptr -> tx_queue_suspended_count--;

                        /* Pickup the suspended count.  */
                        suspended_count =  queue_ptr -> tx_queue_suspended_count;

                        /* Remove the suspended thread from the list.  */

                        /* See if this is the only suspended thread on the list.  */
                        if (suspended_count == TX_NO_SUSPENSIONS)
                        {

                            /* Yes, the only suspended thread.  */

                            /* Update the head pointer.  */
                            queue_ptr -> tx_queue_suspension_list =  TX_NULL;
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
                            if (queue_ptr -> tx_queue_suspension_list == thread_ptr)
                            {

                                /* Update the list head pointer.  */
                                queue_ptr -> tx_queue_suspension_list =         next_thread;
                            }
                        }

                        /* Now we need to determine if this cleanup is from a terminate, timeout,
                           or from a wait abort.  */
                        if (thread_ptr -> tx_thread_state == TX_QUEUE_SUSP)
                        {

                            /* Timeout condition and the thread still suspended on the queue.
                               Setup return error status and resume the thread.  */

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

                            /* Increment the total timeouts counter.  */
                            _tx_queue_performance_timeout_count++;

                            /* Increment the number of timeouts on this queue.  */
                            queue_ptr -> tx_queue_performance_timeout_count++;
#endif

                            /* Setup return status.  */
                            if (queue_ptr -> tx_queue_enqueued != TX_NO_MESSAGES)
                            {

                                /* Queue full timeout!  */
                                thread_ptr -> tx_thread_suspend_status =  TX_QUEUE_FULL;
                            }
                            else
                            {

                                /* Queue empty timeout!  */
                                thread_ptr -> tx_thread_suspend_status =  TX_QUEUE_EMPTY;
                            }

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

