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
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_time_slice_change                       PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes thread time slice change requests.  The     */
/*    previous time slice is returned to the caller.  If the new request  */
/*    is made for an executing thread, it is also placed in the actual    */
/*    time-slice countdown variable.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread             */
/*    new_time_slice                        New time slice                */
/*    old_time_slice                        Old time slice                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Service return status         */
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
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_time_slice_change(TX_THREAD *thread_ptr, ULONG new_time_slice, ULONG *old_time_slice)
{

TX_INTERRUPT_SAVE_AREA

ULONG    core_index;


    /* Lockout interrupts while the thread is being resumed.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_TIME_SLICE_CHANGE, thread_ptr, new_time_slice, thread_ptr -> tx_thread_new_time_slice, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_TIME_SLICE_CHANGE_INSERT

    /* Return the old time slice.  */
    *old_time_slice =  thread_ptr -> tx_thread_new_time_slice;

    /* Setup the new time-slice.  */
    thread_ptr -> tx_thread_time_slice =      new_time_slice;
    thread_ptr -> tx_thread_new_time_slice =  new_time_slice;

    /* Pickup index.  */
    core_index =  thread_ptr -> tx_thread_smp_core_mapped;

    /* Determine if this thread is the currently executing thread.  */
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    if ((core_index < ((ULONG) TX_THREAD_SMP_MAX_CORES)) && (thread_ptr == _tx_thread_current_ptr[core_index]))
#else

    if ((core_index < _tx_thread_smp_max_cores) && (thread_ptr == _tx_thread_current_ptr[core_index]))
#endif
    {

        /* Yes, update the time-slice countdown variable.  */
        _tx_timer_time_slice[core_index] =  new_time_slice;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

