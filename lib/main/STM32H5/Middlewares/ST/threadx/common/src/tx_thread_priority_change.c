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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_priority_change                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function changes the priority of the specified thread.  It     */
/*    also returns the old priority and handles preemption if the calling */
/*    thread is currently executing and the priority change results in a  */
/*    higher priority thread ready for execution.                         */
/*                                                                        */
/*    Note: the preemption threshold is automatically changed to the new  */
/*    priority.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to suspend  */
/*    new_priority                          New thread priority           */
/*    old_priority                          Old thread priority           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume          Resume thread                     */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*    _tx_thread_system_suspend         Suspend thread                    */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
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
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            change thread state from    */
/*                                            TX_SUSPENDED to             */
/*                                            TX_PRIORITY_CHANGE before   */
/*                                            calling                     */
/*                                            _tx_thread_system_suspend,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_priority_change(TX_THREAD *thread_ptr, UINT new_priority, UINT *old_priority)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *execute_ptr;
TX_THREAD       *next_execute_ptr;
UINT            original_priority;


    /* Lockout interrupts while the thread is being suspended.  */
    TX_DISABLE

    /* Save the previous priority.  */
    *old_priority =  thread_ptr -> tx_thread_user_priority;

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_PRIORITY_CHANGE, thread_ptr, new_priority, thread_ptr -> tx_thread_priority, thread_ptr -> tx_thread_state, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_PRIORITY_CHANGE_INSERT

    /* Determine if this thread is currently ready.  */
    if (thread_ptr -> tx_thread_state != TX_READY)
    {

        /* Setup the user priority and threshold in the thread's control
           block.  */
        thread_ptr -> tx_thread_user_priority =               new_priority;
        thread_ptr -> tx_thread_user_preempt_threshold =      new_priority;

        /* Determine if the actual thread priority should be setup, which is the
           case if the new priority is higher than the priority inheritance.  */
        if (new_priority < thread_ptr -> tx_thread_inherit_priority)
        {

            /* Change thread priority to the new user's priority.  */
            thread_ptr -> tx_thread_priority =           new_priority;
            thread_ptr -> tx_thread_preempt_threshold =  new_priority;
        }
        else
        {

            /* Change thread priority to the priority inheritance.  */
            thread_ptr -> tx_thread_priority =           thread_ptr -> tx_thread_inherit_priority;
            thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_inherit_priority;
        }

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Set the state to priority change.  */
        thread_ptr -> tx_thread_state =    TX_PRIORITY_CHANGE;

        /* Pickup the next thread to execute.  */
        execute_ptr =  _tx_thread_execute_ptr;

        /* Save the original priority.  */
        original_priority =  thread_ptr -> tx_thread_priority;

#ifdef TX_NOT_INTERRUPTABLE

        /* Increment the preempt disable flag.  */
        _tx_thread_preempt_disable++;

        /* Call actual non-interruptable thread suspension routine.  */
        _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));

        /* At this point, the preempt disable flag is still set, so we still have
           protection against all preemption.  */

        /* Setup the new priority for this thread.  */
        thread_ptr -> tx_thread_user_priority =           new_priority;
        thread_ptr -> tx_thread_user_preempt_threshold =  new_priority;

        /* Determine if the actual thread priority should be setup, which is the
           case if the new priority is higher than the priority inheritance.  */
        if (new_priority < thread_ptr -> tx_thread_inherit_priority)
        {

            /* Change thread priority to the new user's priority.  */
            thread_ptr -> tx_thread_priority =           new_priority;
            thread_ptr -> tx_thread_preempt_threshold =  new_priority;
        }
        else
        {

            /* Change thread priority to the priority inheritance.  */
            thread_ptr -> tx_thread_priority =           thread_ptr -> tx_thread_inherit_priority;
            thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_inherit_priority;
        }

        /* Resume the thread with the new priority.  */
        _tx_thread_system_ni_resume(thread_ptr);

#else

        /* Increment the preempt disable flag by 2 to prevent system suspend from
           returning to the system.  */
        _tx_thread_preempt_disable =  _tx_thread_preempt_disable + ((UINT) 3);

        /* Set the suspending flag. */
        thread_ptr -> tx_thread_suspending =  TX_TRUE;

        /* Setup the timeout period.  */
        thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);

        /* Restore interrupts.  */
        TX_RESTORE

        /* The thread is ready and must first be removed from the list.  Call the
           system suspend function to accomplish this.  */
        _tx_thread_system_suspend(thread_ptr);

        /* At this point, the preempt disable flag is still set, so we still have
           protection against all preemption.  */

        /* Setup the new priority for this thread.  */
        thread_ptr -> tx_thread_user_priority =           new_priority;
        thread_ptr -> tx_thread_user_preempt_threshold =  new_priority;

        /* Determine if the actual thread priority should be setup, which is the
           case if the new priority is higher than the priority inheritance.  */
        if (new_priority < thread_ptr -> tx_thread_inherit_priority)
        {

            /* Change thread priority to the new user's priority.  */
            thread_ptr -> tx_thread_priority =           new_priority;
            thread_ptr -> tx_thread_preempt_threshold =  new_priority;
        }
        else
        {

            /* Change thread priority to the priority inheritance.  */
            thread_ptr -> tx_thread_priority =           thread_ptr -> tx_thread_inherit_priority;
            thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_inherit_priority;
        }

        /* Resume the thread with the new priority.  */
        _tx_thread_system_resume(thread_ptr);

        /* Disable interrupts.  */
        TX_DISABLE
#endif

        /* Decrement the preempt disable flag.  */
        _tx_thread_preempt_disable--;

        /* Pickup the next thread to execute.  */
        next_execute_ptr =  _tx_thread_execute_ptr;

        /* Determine if this thread is not the next thread to execute.  */
        if (thread_ptr != next_execute_ptr)
        {

            /* Make sure the thread is still ready.  */
            if (thread_ptr -> tx_thread_state == TX_READY)
            {

                /* Now check and see if this thread has an equal or higher priority.  */
                if (thread_ptr -> tx_thread_priority <= next_execute_ptr -> tx_thread_priority)
                {

                    /* Now determine if this thread was the previously executing thread.  */
                    if (thread_ptr == execute_ptr)
                    {

                        /* Yes, this thread was previously executing before we temporarily suspended and resumed
                           it in order to change the priority. A lower or same priority thread cannot be the next thread
                           to execute in this case since this thread really didn't suspend.  Simply reset the execute
                           pointer to this thread.  */
                        _tx_thread_execute_ptr =  thread_ptr;

                        /* Determine if we moved to a lower priority. If so, move the thread to the front of its priority list.  */
                        if (original_priority < new_priority)
                        {

                            /* Ensure that this thread is placed at the front of the priority list.  */
                            _tx_thread_priority_list[thread_ptr -> tx_thread_priority] =  thread_ptr;
                        }
                    }
                }
            }
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();
    }

    /* Return success if we get here!  */
    return(TX_SUCCESS);
}

