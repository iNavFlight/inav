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
/**   Mutex                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_mutex.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_mutex_prioritize                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function places the highest priority suspended thread at the   */
/*    front of the suspension list.  All other threads remain in the same */
/*    FIFO suspension order.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mutex_ptr                         Pointer to mutex control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_preempt_check   Check for preemption              */
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
UINT  _tx_mutex_prioritize(TX_MUTEX *mutex_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
TX_THREAD       *priority_thread_ptr;
TX_THREAD       *head_ptr;
UINT            suspended_count;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
UINT            list_changed;
#ifdef TX_MISRA_ENABLE
UINT            status;
#endif


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_PRIORITIZE, mutex_ptr, mutex_ptr -> tx_mutex_suspended_count, TX_POINTER_TO_ULONG_CONVERT(&suspended_count), 0, TX_TRACE_MUTEX_EVENTS)

    /* Log this kernel call.  */
    TX_EL_MUTEX_PRIORITIZE_INSERT

    /* Pickup the suspended count.  */
    suspended_count =  mutex_ptr -> tx_mutex_suspended_count;

    /* Determine if there are fewer than 2 suspended threads.  */
    if (suspended_count < ((UINT) 2))
    {

        /* Restore interrupts.  */
        TX_RESTORE
    }

    /* Determine if there how many threads are suspended on this mutex.  */
    else if (suspended_count == ((UINT) 2))
    {

        /* Pickup the head pointer and the next pointer.  */
        head_ptr =  mutex_ptr -> tx_mutex_suspension_list;
        next_thread =  head_ptr -> tx_thread_suspended_next;

        /* Determine if the next suspended thread has a higher priority.  */
        if ((next_thread -> tx_thread_priority) < (head_ptr -> tx_thread_priority))
        {

            /* Yes, move the list head to the next thread.  */
            mutex_ptr -> tx_mutex_suspension_list =  next_thread;
        }

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Remember the suspension count and head pointer.  */
        head_ptr =   mutex_ptr -> tx_mutex_suspension_list;

        /* Default the highest priority thread to the thread at the front of the list.  */
        priority_thread_ptr =  head_ptr;

        /* Setup search pointer.  */
        thread_ptr =  priority_thread_ptr -> tx_thread_suspended_next;

        /* Disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Set the list changed flag to false.  */
        list_changed =  TX_FALSE;

        /* Search through the list to find the highest priority thread.  */
        do
        {

            /* Is the current thread higher priority?  */
            if (thread_ptr -> tx_thread_priority < priority_thread_ptr -> tx_thread_priority)
            {

                /* Yes, remember that this thread is the highest priority.  */
                priority_thread_ptr =  thread_ptr;
            }

            /* Restore interrupts temporarily.  */
            TX_RESTORE

            /* Disable interrupts again.  */
            TX_DISABLE

            /* Determine if any changes to the list have occurred while
               interrupts were enabled.  */

            /* Is the list head the same?  */
            if (head_ptr != mutex_ptr -> tx_mutex_suspension_list)
            {

                /* The list head has changed, set the list changed flag.  */
                list_changed =  TX_TRUE;
            }
            else
            {

                /* Is the suspended count the same?  */
                if (suspended_count != mutex_ptr -> tx_mutex_suspended_count)
                {

                    /* The list head has changed, set the list changed flag.  */
                    list_changed =  TX_TRUE;
                }
            }

            /* Determine if the list has changed.  */
            if (list_changed == TX_FALSE)
            {

                /* Move the thread pointer to the next thread.  */
                thread_ptr =  thread_ptr -> tx_thread_suspended_next;
            }
            else
            {

                /* Remember the suspension count and head pointer.  */
                head_ptr =   mutex_ptr -> tx_mutex_suspension_list;
                suspended_count =  mutex_ptr -> tx_mutex_suspended_count;

                /* Default the highest priority thread to the thread at the front of the list.  */
                priority_thread_ptr =  head_ptr;

                /* Setup search pointer.  */
                thread_ptr =  priority_thread_ptr -> tx_thread_suspended_next;

                /* Reset the list changed flag.  */
                list_changed =  TX_FALSE;
            }

        } while (thread_ptr != head_ptr);

        /* Release preemption.  */
        _tx_thread_preempt_disable--;

        /* Now determine if the highest priority thread is at the front
           of the list.  */
        if (priority_thread_ptr != head_ptr)
        {

            /* No, we need to move the highest priority suspended thread to the
               front of the list.  */

            /* First, remove the highest priority thread by updating the
               adjacent suspended threads.  */
            next_thread =                                  priority_thread_ptr -> tx_thread_suspended_next;
            previous_thread =                              priority_thread_ptr -> tx_thread_suspended_previous;
            next_thread -> tx_thread_suspended_previous =  previous_thread;
            previous_thread -> tx_thread_suspended_next =  next_thread;

            /* Now, link the highest priority thread at the front of the list.  */
            previous_thread =                                      head_ptr -> tx_thread_suspended_previous;
            priority_thread_ptr -> tx_thread_suspended_next =      head_ptr;
            priority_thread_ptr -> tx_thread_suspended_previous =  previous_thread;
            previous_thread -> tx_thread_suspended_next =          priority_thread_ptr;
            head_ptr -> tx_thread_suspended_previous =             priority_thread_ptr;

            /* Move the list head pointer to the highest priority suspended thread.  */
            mutex_ptr -> tx_mutex_suspension_list =  priority_thread_ptr;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();
    }

#ifdef TX_MISRA_ENABLE

    /* Initialize status to success.  */
    status =  TX_SUCCESS;

    /* Define extended processing option.  */
    status =  TX_MUTEX_PRIORITIZE_MISRA_EXTENSION(status);

    /* Return completion status.  */
    return(status);
#else

    /* Return successful completion.  */
    return(TX_SUCCESS);
#endif
}

