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
/*    _tx_thread_performance_system_info_get              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves thread system performance information.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    resumptions                       Destination for total number of   */
/*                                        thread resumptions              */
/*    suspensions                       Destination for total number of   */
/*                                        thread suspensions              */
/*    solicited_preemptions             Destination for total number of   */
/*                                        thread preemption from thread   */
/*                                        API calls                       */
/*    interrupt_preemptions             Destination for total number of   */
/*                                        thread preemptions as a result  */
/*                                        of threads made ready inside of */
/*                                        Interrupt Service Routines      */
/*    priority_inversions               Destination for total number of   */
/*                                        priority inversions             */
/*    time_slices                       Destination for total number of   */
/*                                        time-slices                     */
/*    relinquishes                      Destination for total number of   */
/*                                        relinquishes                    */
/*    timeouts                          Destination for total number of   */
/*                                        timeouts                        */
/*    wait_aborts                       Destination for total number of   */
/*                                        wait aborts                     */
/*    non_idle_returns                  Destination for total number of   */
/*                                        times threads return when       */
/*                                        another thread is ready         */
/*    idle_returns                      Destination for total number of   */
/*                                        times threads return when no    */
/*                                        other thread is ready           */
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
UINT  _tx_thread_performance_system_info_get(ULONG *resumptions, ULONG *suspensions,
                ULONG *solicited_preemptions, ULONG *interrupt_preemptions, ULONG *priority_inversions,
                ULONG *time_slices, ULONG *relinquishes, ULONG *timeouts, ULONG *wait_aborts,
                ULONG *non_idle_returns, ULONG *idle_returns)
{

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_PERFORMANCE_SYSTEM_INFO_GET, 0, 0, 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_PERFORMANCE_SYSTEM_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve total number of thread resumptions.  */
    if (resumptions != TX_NULL)
    {

        *resumptions =  _tx_thread_performance_resume_count;
    }

    /* Retrieve total number of thread suspensions.  */
    if (suspensions != TX_NULL)
    {

        *suspensions =  _tx_thread_performance_suspend_count;
    }

    /* Retrieve total number of solicited thread preemptions.  */
    if (solicited_preemptions != TX_NULL)
    {

        *solicited_preemptions =  _tx_thread_performance_solicited_preemption_count;
    }

    /* Retrieve total number of interrupt thread preemptions.  */
    if (interrupt_preemptions != TX_NULL)
    {

        *interrupt_preemptions =  _tx_thread_performance_interrupt_preemption_count;
    }

    /* Retrieve total number of thread priority inversions.  */
    if (priority_inversions != TX_NULL)
    {

        *priority_inversions =  _tx_thread_performance_priority_inversion_count;
    }

    /* Retrieve total number of thread time-slices.  */
    if (time_slices != TX_NULL)
    {

        *time_slices =  _tx_thread_performance_time_slice_count;
    }

    /* Retrieve total number of thread relinquishes.  */
    if (relinquishes != TX_NULL)
    {

        *relinquishes =  _tx_thread_performance_relinquish_count;
    }

    /* Retrieve total number of thread timeouts.  */
    if (timeouts != TX_NULL)
    {

        *timeouts =  _tx_thread_performance_timeout_count;
    }

    /* Retrieve total number of thread wait aborts.  */
    if (wait_aborts != TX_NULL)
    {

        *wait_aborts =  _tx_thread_performance_wait_abort_count;
    }

    /* Retrieve total number of thread non-idle system returns.  */
    if (non_idle_returns != TX_NULL)
    {

        *non_idle_returns =  _tx_thread_performance_non_idle_return_count;
    }

    /* Retrieve total number of thread idle system returns.  */
    if (idle_returns != TX_NULL)
    {

        *idle_returns =  _tx_thread_performance_idle_return_count;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);

#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (resumptions != TX_NULL)
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
    else if (non_idle_returns != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (idle_returns != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }

    /* Return completion status.  */
    return(status);
#endif
}

