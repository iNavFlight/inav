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
/*    _tx_thread_context_save                           SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function saves the context of an executing thread in the       */
/*    beginning of interrupt processing.  The function also ensures that  */
/*    the system stack is used upon return to the calling ISR.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_linux_debug_entry_insert                                        */
/*    _tx_linux_mutex_obtain                                              */
/*    _tx_linux_thread_suspend                                            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ISRs                                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_thread_context_save(VOID)
{

TX_THREAD   *thread_ptr;
UINT        interrupt_posture;

    /* Loop to perform retries on thread preemption.  */
    while(1)
    {

        /* Lock mutex to ensure other threads are not playing with
           the core ThreadX data structures.  */
        interrupt_posture =  _tx_thread_smp_protect();

        /* Check for a system error condition.  */
        if (interrupt_posture != TX_FALSE)
        {

            /* This should not happen... increment the system error counter.  */
            _tx_linux_system_error++;
        }

        /* Debug entry.  */
        _tx_linux_debug_entry_insert("CONTEXT_SAVE", __FILE__, __LINE__);

        /* All ISRs are assumed to be serviced on core 0.  */

        /* Pickup the current thread pointer.  */
        thread_ptr =  _tx_thread_current_ptr[0];

        /* If an application thread is running, suspend it to simulate preemption. */
        if ((thread_ptr) && (_tx_thread_system_state[0] == 0))
        {

            /* Yes, this is the first interrupt and an application thread is running...
               suspend it!  */
            _tx_linux_thread_suspend(thread_ptr -> tx_thread_linux_thread_id);

            /* Debug entry.  */
            _tx_linux_debug_entry_insert("CONTEXT_SAVE-suspend_thread", __FILE__, __LINE__);
        }

        /* Increment the nested interrupt condition.  */
        _tx_thread_system_state[0]++;

        /* Do not release the protection for ISRs, since in SMP mode other threads might be scheduled on
           interrupted virtual core 0, which will confuse ThreadX.  */

        /* Get out of the loop.  */
        break;
    }
}

