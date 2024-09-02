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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_smp_core_preempt                       SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function preempts the specified core in situations where the   */
/*    thread corresponding to this core is no longer ready or when the    */
/*    core must be used for a higher-priority thread. If the specified is */
/*    the current core, this processing is skipped since the will give up */
/*    control subsequently on its own.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    core                                  The core to preempt           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    ReleaseSemaphore                      Let scheduler run to preempt  */
/*                                            thread on core              */
/*    _tx_win32_debug_entry_insert          Make debug log entry          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX Source                                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
void  _tx_thread_smp_core_preempt(UINT core)
{

TX_THREAD   *preempt_thread;


    /* Protection is in force at this point.  */

    /* Pickup the thread pointer on the selected core.  */
    preempt_thread =  _tx_thread_current_ptr[core];

    /* Determine if there is a thread to preempt.  */
    if (preempt_thread)
    {

        /* Yes, set the deferred preemption flag for this thread. This preemption will be
           completed in the scheduler.   */
        preempt_thread -> tx_thread_linux_deferred_preempt =  TX_TRUE;

        /* Debug entry.  */
        _tx_linux_debug_entry_insert("CORE_PREEMPT_deferred", __FILE__, __LINE__);

        /* Release the semaphore that the main scheduling thread is waiting
           on.  Note that the main scheduling algorithm will take care of
           preempting the thread on this core.  */
        tx_linux_sem_post(&_tx_linux_scheduler_semaphore);
    }
}


