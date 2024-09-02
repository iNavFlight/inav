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
#include "tx_initialize.h"
#include "tx_timer.h"
#include "tx_thread.h"


/* Determine if debugging is enabled.  */

#ifdef TX_THREAD_SMP_DEBUG_ENABLE


/* Define the maximum number of debug entries.   */

#ifndef TX_THREAD_SMP_MAX_DEBUG_ENTRIES
#define TX_THREAD_SMP_MAX_DEBUG_ENTRIES 100
#endif


/* Define the debug information structures.   */

typedef struct TX_THREAD_SMP_DEBUG_ENTRY_STRUCT
{

    ULONG                       tx_thread_smp_debug_entry_id;
    ULONG                       tx_thread_smp_debug_entry_suspend;
    ULONG                       tx_thread_smp_debug_entry_core_index;
    ULONG                       tx_thread_smp_debug_entry_time;
    ULONG                       tx_thread_smp_debug_entry_timer_clock;
    TX_THREAD                   *tx_thread_smp_debug_entry_thread;
    UINT                        tx_thread_smp_debug_entry_thread_priority;
    UINT                        tx_thread_smp_debug_entry_thread_threshold;
    ULONG                       tx_thread_smp_debug_entry_thread_core_control;
    TX_THREAD                   *tx_thread_smp_debug_entry_current_thread;
    TX_THREAD_SMP_PROTECT       tx_thread_smp_debug_protection;
    ULONG                       tx_thread_smp_debug_entry_preempt_disable;
    ULONG                       tx_thread_smp_debug_entry_system_state[TX_THREAD_SMP_MAX_CORES];
#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
    ULONG                       tx_thread_smp_debug_entry_preempt_map;
#endif
    TX_THREAD                   *tx_thread_smp_debug_entry_preempt_thread;
    ULONG                       tx_thread_smp_debug_entry_priority_map;
    ULONG                       tx_thread_smp_debug_entry_reschedule_pending;
    TX_THREAD                   *tx_thread_smp_debug_entry_current_threads[TX_THREAD_SMP_MAX_CORES];
    TX_THREAD                   *tx_thread_smp_debug_entry_execute_threads[TX_THREAD_SMP_MAX_CORES];

} TX_THREAD_SMP_DEBUG_ENTRY_INFO;


/* Define the circular array of debug entries.  */

TX_THREAD_SMP_DEBUG_ENTRY_INFO  _tx_thread_smp_debug_info_array[TX_THREAD_SMP_MAX_DEBUG_ENTRIES];


/* Define the starting index.  */

ULONG                           _tx_thread_smp_debug_info_current_index;


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_smp_debug_entry_insert                  PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is responsible for making an entry in the circular    */
/*    debug log.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                                    ID of event                   */
/*    suspend                               Flag set to true for suspend  */
/*                                            events                      */
/*    thread_ptr                            Specified thread              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_smp_time_get               Get global time stamp         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Internal routines                                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
void  _tx_thread_smp_debug_entry_insert(ULONG id, ULONG suspend, VOID *thread_void_ptr)
{

ULONG                       i;
ULONG                       core_index;
TX_THREAD_SMP_DEBUG_ENTRY_INFO     *entry_ptr;
TX_THREAD                   *thread_ptr;


    /* Spin, if an error is detected. No sense in populating the debug after the error occurs.  */
    while (_tx_thread_smp_system_error)
    {

        /* Spin here!  */
    }

    /* Check for a bad current index.  */
    while (_tx_thread_smp_debug_info_current_index >= TX_THREAD_SMP_MAX_DEBUG_ENTRIES)
    {

        /* Spin here!  */
    }

    thread_ptr =  (TX_THREAD *) thread_void_ptr;

    /* It is assumed that interrupts are locked out at this point.  */

    /* Setup pointer to debug entry.  */
    entry_ptr =  &_tx_thread_smp_debug_info_array[_tx_thread_smp_debug_info_current_index++];

    /* Check for wrap on the index.  */
    if (_tx_thread_smp_debug_info_current_index >= TX_THREAD_SMP_MAX_DEBUG_ENTRIES)
    {

        /* Wrap back to 0. */
        _tx_thread_smp_debug_info_current_index =  0;
    }

    /* Get the index.  */
    core_index =  TX_SMP_CORE_ID;

    /* We know at this point that multithreading and interrupts are disabled...  so start populating the array.  */
    entry_ptr -> tx_thread_smp_debug_entry_id =                         id;
    entry_ptr -> tx_thread_smp_debug_entry_suspend =                    suspend;
    entry_ptr -> tx_thread_smp_debug_entry_thread =                     thread_ptr;
    entry_ptr -> tx_thread_smp_debug_entry_time =                       _tx_thread_smp_time_get();
    entry_ptr -> tx_thread_smp_debug_entry_timer_clock =                _tx_timer_system_clock;
    entry_ptr -> tx_thread_smp_debug_entry_core_index =                  core_index;
    entry_ptr -> tx_thread_smp_debug_entry_current_thread =             _tx_thread_current_ptr[core_index];
    if (entry_ptr -> tx_thread_smp_debug_entry_current_thread)
    {

        entry_ptr -> tx_thread_smp_debug_entry_thread_priority =     (entry_ptr -> tx_thread_smp_debug_entry_current_thread) -> tx_thread_priority;
        entry_ptr -> tx_thread_smp_debug_entry_thread_threshold =    (entry_ptr -> tx_thread_smp_debug_entry_current_thread) -> tx_thread_preempt_threshold;
        entry_ptr -> tx_thread_smp_debug_entry_thread_core_control =  (entry_ptr -> tx_thread_smp_debug_entry_current_thread) -> tx_thread_smp_core_control;
    }
    else
    {

        entry_ptr -> tx_thread_smp_debug_entry_thread_priority =     0;
        entry_ptr -> tx_thread_smp_debug_entry_thread_threshold =    0;
        entry_ptr -> tx_thread_smp_debug_entry_thread_core_control =  0;
    }

    entry_ptr -> tx_thread_smp_debug_protection =                       _tx_thread_smp_protection;
    entry_ptr -> tx_thread_smp_debug_entry_preempt_disable  =           _tx_thread_preempt_disable;
#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
    entry_ptr -> tx_thread_smp_debug_entry_preempt_map =                _tx_thread_preempted_maps[0];
#endif
    entry_ptr -> tx_thread_smp_debug_entry_preempt_thread =             _tx_thread_preemption__threshold_scheduled;
    entry_ptr -> tx_thread_smp_debug_entry_priority_map =               _tx_thread_priority_maps[0];

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    /* Loop to save the current and execute lists.  */
    for (i = 0; i < TX_THREAD_SMP_MAX_CORES; i++)
#else

    /* Loop to save the current and execute lists.  */
    for (i = 0; i < _tx_thread_smp_max_cores; i++)
#endif
    {

        /* Save the pointers.  */
        entry_ptr -> tx_thread_smp_debug_entry_current_threads[i] =     _tx_thread_current_ptr[i];
        entry_ptr -> tx_thread_smp_debug_entry_execute_threads[i] =     _tx_thread_execute_ptr[i];

        /* Save the system state.  */
        entry_ptr -> tx_thread_smp_debug_entry_system_state[i] =        _tx_thread_system_state[i];
    }
}


#endif

