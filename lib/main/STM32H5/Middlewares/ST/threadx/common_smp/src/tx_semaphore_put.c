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
/**   Semaphore                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_semaphore.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_semaphore_put                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function puts an instance into the specified counting          */
/*    semaphore.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    semaphore_ptr                     Pointer to semaphore control block*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Success completion status         */
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
UINT  _tx_semaphore_put(TX_SEMAPHORE *semaphore_ptr)
{

TX_INTERRUPT_SAVE_AREA

#ifndef TX_DISABLE_NOTIFY_CALLBACKS
VOID            (*semaphore_put_notify)(struct TX_SEMAPHORE_STRUCT *notify_semaphore_ptr);
#endif

TX_THREAD       *thread_ptr;
UINT            suspended_count;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;


    /* Disable interrupts to put an instance back to the semaphore.  */
    TX_DISABLE

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

    /* Increment the total semaphore put counter.  */
    _tx_semaphore_performance_put_count++;

    /* Increment the number of puts on this semaphore.  */
    semaphore_ptr -> tx_semaphore_performance_put_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_SEMAPHORE_PUT, semaphore_ptr, semaphore_ptr -> tx_semaphore_count, semaphore_ptr -> tx_semaphore_suspended_count, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), TX_TRACE_SEMAPHORE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_SEMAPHORE_PUT_INSERT

    /* Pickup the number of suspended threads.  */
    suspended_count =  semaphore_ptr -> tx_semaphore_suspended_count;

    /* Determine if there are any threads suspended on the semaphore.  */
    if (suspended_count == TX_NO_SUSPENSIONS)
    {

        /* Increment the semaphore count.  */
        semaphore_ptr -> tx_semaphore_count++;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Pickup the application notify function.  */
        semaphore_put_notify =  semaphore_ptr -> tx_semaphore_put_notify;
#endif

        /* Restore interrupts.  */
        TX_RESTORE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Determine if notification is required.  */
        if (semaphore_put_notify != TX_NULL)
        {

            /* Yes, call the appropriate notify callback function.  */
            (semaphore_put_notify)(semaphore_ptr);
        }
#endif
    }
    else
    {

        /* A thread is suspended on this semaphore.  */

        /* Pickup the pointer to the first suspended thread.  */
        thread_ptr =  semaphore_ptr -> tx_semaphore_suspension_list;

        /* Remove the suspended thread from the list.  */

        /* See if this is the only suspended thread on the list.  */
        suspended_count--;
        if (suspended_count == TX_NO_SUSPENSIONS)
        {

            /* Yes, the only suspended thread.  */

            /* Update the head pointer.  */
            semaphore_ptr -> tx_semaphore_suspension_list =  TX_NULL;
        }
        else
        {

            /* At least one more thread is on the same expiration list.  */

            /* Update the list head pointer.  */
            next_thread =                                     thread_ptr -> tx_thread_suspended_next;
            semaphore_ptr -> tx_semaphore_suspension_list =   next_thread;

            /* Update the links of the adjacent threads.  */
            previous_thread =                               thread_ptr -> tx_thread_suspended_previous;
            next_thread -> tx_thread_suspended_previous =   previous_thread;
            previous_thread -> tx_thread_suspended_next =   next_thread;
        }

        /* Decrement the suspension count.  */
        semaphore_ptr -> tx_semaphore_suspended_count =  suspended_count;

        /* Prepare for resumption of the first thread.  */

        /* Clear cleanup routine to avoid timeout.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Pickup the application notify function.  */
        semaphore_put_notify =  semaphore_ptr -> tx_semaphore_put_notify;
#endif

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

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Determine if notification is required.  */
        if (semaphore_put_notify != TX_NULL)
        {

            /* Yes, call the appropriate notify callback function.  */
            (semaphore_put_notify)(semaphore_ptr);
        }
#endif
    }

    /* Return successful completion.  */
    return(TX_SUCCESS);
}

