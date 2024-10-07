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
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_initialize.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_resume                                  PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes application resume thread services.  Actual */
/*    thread resumption is performed in the core service.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to resume   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Service return status         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_smp_rebalance_execute_list Rebalance the execution list  */
/*    _tx_thread_system_resume              Resume thread                 */
/*    _tx_thread_system_ni_resume           Non-interruptable resume      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_resume(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT        status;
UINT        core_index;


    /* Lockout interrupts while the thread is being resumed.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RESUME_API, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&status), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_RESUME_INSERT

    /* Determine if the thread is suspended or in the process of suspending.
       If so, call the thread resume processing.  */
    if (thread_ptr -> tx_thread_state == TX_SUSPENDED)
    {

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

        /* Call the actual resume service to resume the thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif

        /* Disable interrupts.  */
        TX_DISABLE

        /* Determine if the thread's preemption-threshold needs to be restored.  */
        if (_tx_thread_smp_current_state_get() >= TX_INITIALIZE_IN_PROGRESS)
        {

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

            /* Clear the preemption bit maps, since nothing has yet run during initialization.  */
            TX_MEMSET(_tx_thread_preempted_maps, 0, sizeof(_tx_thread_preempted_maps));
#if TX_MAX_PRIORITIES > 32
            _tx_thread_preempted_map_active =  ((ULONG) 0);
#endif
#endif
            _tx_thread_preemption__threshold_scheduled =  TX_NULL;

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(14, 0, thread_ptr);
#endif

            /* Get the core index.  */
            core_index =  TX_SMP_CORE_ID;

            /* Call the rebalance routine. This routine maps cores and ready threads.  */
            _tx_thread_smp_rebalance_execute_list(core_index);

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(15, 0, thread_ptr);
#endif
        }

        /* Setup successful return status.  */
        status =  TX_SUCCESS;
    }
    else if (thread_ptr -> tx_thread_delayed_suspend != TX_FALSE)
    {

        /* Clear the delayed suspension.  */
        thread_ptr -> tx_thread_delayed_suspend =  TX_FALSE;

        /* Setup delayed suspend lifted return status.  */
        status =  TX_SUSPEND_LIFTED;
    }
    else
    {

        /* Setup invalid resume return status.  */
        status =  TX_RESUME_ERROR;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status. */
    return(status);
}

