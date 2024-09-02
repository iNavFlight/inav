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
#include "tx_thread.h"
#include "tx_timer.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_timeout                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles thread timeout processing.  Timeouts occur in */
/*    two flavors, namely the thread sleep timeout and all other service  */
/*    call timeouts.  Thread sleep timeouts are processed locally, while  */
/*    the others are processed by the appropriate suspension clean-up     */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timeout_input                         Contains the thread pointer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Suspension Cleanup Functions                                        */
/*    _tx_thread_system_resume          Resume thread                     */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_timer_expiration_process          Timer expiration function     */
/*    _tx_timer_thread_entry                Timer thread function         */
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
VOID  _tx_thread_timeout(ULONG timeout_input)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
VOID            (*suspend_cleanup)(struct TX_THREAD_STRUCT *suspend_thread_ptr, ULONG suspension_sequence);
ULONG           suspension_sequence;


    /* Pickup the thread pointer.  */
    TX_THREAD_TIMEOUT_POINTER_SETUP(thread_ptr)

    /* Disable interrupts.  */
    TX_DISABLE

    /* Determine how the thread is currently suspended.  */
    if (thread_ptr -> tx_thread_state == TX_SLEEP)
    {

#ifdef TX_NOT_INTERRUPTABLE

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Increment the disable preemption flag.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Lift the suspension on the sleeping thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif
    }
    else
    {

        /* Process all other suspension timeouts.  */

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

        /* Increment the total number of thread timeouts.  */
        _tx_thread_performance_timeout_count++;

        /* Increment the number of timeouts for this thread.  */
        thread_ptr -> tx_thread_performance_timeout_count++;
#endif

        /* Pickup the cleanup routine address.  */
        suspend_cleanup =  thread_ptr -> tx_thread_suspend_cleanup;

#ifndef TX_NOT_INTERRUPTABLE

        /* Pickup the suspension sequence number that is used later to verify that the
           cleanup is still necessary.  */
        suspension_sequence =  thread_ptr -> tx_thread_suspension_sequence;
#else

        /* When not interruptable is selected, the suspension sequence is not used - just set to 0.  */
        suspension_sequence =  ((ULONG) 0);
#endif

#ifndef TX_NOT_INTERRUPTABLE

        /* Restore interrupts.  */
        TX_RESTORE
#endif

        /* Call any cleanup routines.  */
        if (suspend_cleanup != TX_NULL)
        {

            /* Yes, there is a function to call.  */
            (suspend_cleanup)(thread_ptr, suspension_sequence);
        }

#ifdef TX_NOT_INTERRUPTABLE

        /* Restore interrupts.  */
        TX_RESTORE
#endif
    }
}

