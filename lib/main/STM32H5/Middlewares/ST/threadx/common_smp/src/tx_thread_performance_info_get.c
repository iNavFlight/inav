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
#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_performance_info_get                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves performance information from the specified  */
/*    thread.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Pointer to thread control block   */
/*    resumptions                       Destination for number of times   */
/*                                        thread was resumed              */
/*    suspensions                       Destination for number of times   */
/*                                        thread was suspended            */
/*    solicited_preemptions             Destination for number of times   */
/*                                        thread called another service   */
/*                                        that resulted in preemption     */
/*    interrupt_preemptions             Destination for number of times   */
/*                                        thread was preempted by another */
/*                                        thread made ready in Interrupt  */
/*                                        Service Routine (ISR)           */
/*    priority_inversions               Destination for number of times   */
/*                                        a priority inversion was        */
/*                                        detected for this thread        */
/*    time_slices                       Destination for number of times   */
/*                                        thread was time-sliced          */
/*    relinquishes                      Destination for number of thread  */
/*                                        relinquishes                    */
/*    timeouts                          Destination for number of timeouts*/
/*                                        for thread                      */
/*    wait_aborts                       Destination for number of wait    */
/*                                        aborts for thread               */
/*    last_preempted_by                 Destination for pointer of the    */
/*                                        thread that last preempted this */
/*                                        thread                          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _tx_thread_performance_info_get(TX_THREAD *thread_ptr, ULONG *resumptions, ULONG *suspensions,
                ULONG *solicited_preemptions, ULONG *interrupt_preemptions, ULONG *priority_inversions,
                ULONG *time_slices, ULONG *relinquishes, ULONG *timeouts, ULONG *wait_aborts, TX_THREAD **last_preempted_by)
{

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA
UINT                    status;


    /* Determine if this is a legal request.  */
    if (thread_ptr == TX_NULL)
    {

        /* Thread pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }

    /* Determine if the thread ID is invalid.  */
    else if (thread_ptr -> tx_thread_id != TX_THREAD_ID)
    {

        /* Thread pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_PERFORMANCE_INFO_GET, thread_ptr, thread_ptr -> tx_thread_state, 0, 0, TX_TRACE_THREAD_EVENTS)

        /* Log this kernel call.  */
        TX_EL_THREAD_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */

        /* Retrieve number of resumptions for this thread.  */
        if (resumptions != TX_NULL)
        {

            *resumptions =  thread_ptr -> tx_thread_performance_resume_count;
        }

        /* Retrieve number of suspensions for this thread.  */
        if (suspensions != TX_NULL)
        {

            *suspensions =  thread_ptr -> tx_thread_performance_suspend_count;
        }

        /* Retrieve number of solicited preemptions for this thread.  */
        if (solicited_preemptions != TX_NULL)
        {

            *solicited_preemptions =  thread_ptr -> tx_thread_performance_solicited_preemption_count;
        }

        /* Retrieve number of interrupt preemptions for this thread.  */
        if (interrupt_preemptions != TX_NULL)
        {

            *interrupt_preemptions =  thread_ptr -> tx_thread_performance_interrupt_preemption_count;
        }

        /* Retrieve number of priority inversions for this thread.  */
        if (priority_inversions != TX_NULL)
        {

            *priority_inversions =  thread_ptr -> tx_thread_performance_priority_inversion_count;
        }

        /* Retrieve number of time-slices for this thread.  */
        if (time_slices != TX_NULL)
        {

            *time_slices =  thread_ptr -> tx_thread_performance_time_slice_count;
        }

        /* Retrieve number of relinquishes for this thread.  */
        if (relinquishes != TX_NULL)
        {

            *relinquishes =  thread_ptr -> tx_thread_performance_relinquish_count;
        }

        /* Retrieve number of timeouts for this thread.  */
        if (timeouts != TX_NULL)
        {

            *timeouts =  thread_ptr -> tx_thread_performance_timeout_count;
        }

        /* Retrieve number of wait aborts for this thread.  */
        if (wait_aborts != TX_NULL)
        {

            *wait_aborts =  thread_ptr -> tx_thread_performance_wait_abort_count;
        }

        /* Retrieve the pointer of the last thread that preempted this thread.  */
        if (last_preempted_by != TX_NULL)
        {

            *last_preempted_by =  thread_ptr -> tx_thread_performance_last_preempting_thread;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return completion status.  */
        status =  TX_SUCCESS;
    }
#else
UINT                    status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (thread_ptr != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (resumptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (solicited_preemptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (interrupt_preemptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (priority_inversions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (time_slices != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (relinquishes != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (timeouts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (wait_aborts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (last_preempted_by != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
#endif

    /* Return completion status.  */
    return(status);
}

