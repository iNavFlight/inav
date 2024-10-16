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
/*    _tx_thread_info_get                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves information from the specified thread.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                        Pointer to thread control block   */
/*    name                              Destination for the thread name   */
/*    state                             Destination for thread state      */
/*    run_count                         Destination for thread run count  */
/*    priority                          Destination for thread priority   */
/*    preemption_threshold              Destination for thread preemption-*/
/*                                        threshold                       */
/*    time_slice                        Destination for thread time-slice */
/*    next_thread                       Destination for next created      */
/*                                        thread                          */
/*    next_suspended_thread             Destination for next suspended    */
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
UINT  _tx_thread_info_get(TX_THREAD *thread_ptr, CHAR **name, UINT *state, ULONG *run_count,
                UINT *priority, UINT *preemption_threshold, ULONG *time_slice,
                TX_THREAD **next_thread, TX_THREAD **next_suspended_thread)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_INFO_GET, thread_ptr, thread_ptr -> tx_thread_state, 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the name of the thread.  */
    if (name != TX_NULL)
    {

        *name =  thread_ptr -> tx_thread_name;
    }

    /* Pickup the thread's current state.  */
    if (state != TX_NULL)
    {

        *state =  thread_ptr -> tx_thread_state;
    }

    /* Pickup the number of times the thread has been scheduled.  */
    if (run_count != TX_NULL)
    {

        *run_count =  thread_ptr -> tx_thread_run_count;
    }

    /* Pickup the thread's priority.  */
    if (priority != TX_NULL)
    {

        *priority =  thread_ptr -> tx_thread_user_priority;
    }

    /* Pickup the thread's preemption-threshold.  */
    if (preemption_threshold != TX_NULL)
    {

        *preemption_threshold =  thread_ptr -> tx_thread_user_preempt_threshold;
    }

    /* Pickup the thread's current time-slice.  */
    if (time_slice != TX_NULL)
    {

        *time_slice =  thread_ptr -> tx_thread_time_slice;
    }

    /* Pickup the next created thread.  */
    if (next_thread != TX_NULL)
    {

        *next_thread =  thread_ptr -> tx_thread_created_next;
    }

    /* Pickup the next thread suspended.  */
    if (next_suspended_thread != TX_NULL)
    {

        *next_suspended_thread =  thread_ptr -> tx_thread_suspended_next;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

