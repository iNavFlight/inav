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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_system_preempt_check                    PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for preemption that could have occurred as a   */
/*    result scheduling activities occurring while the preempt disable    */
/*    flag was set.                                                       */
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
/*    _tx_thread_system_return              Return to the system          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Other ThreadX Components                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_system_preempt_check(VOID)
{

TX_INTERRUPT_SAVE_AREA

UINT            core_index;
UINT            restore_needed;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Set the restore needed flag.  */
    restore_needed =  TX_TRUE;

    /* Pickup the index.  */
    core_index =  TX_SMP_CORE_ID;

    /* Determine if the call is from initialization, an ISR or if the preempt disable flag is set.  */
    if (_tx_thread_system_state[core_index] == ((ULONG) 0))
    {

        /* Ensure the preempt disable flag is not set.  */
        if (_tx_thread_preempt_disable == ((UINT) 0))
        {

            /* Thread execution - now determine if preemption should take place.  */
            if (_tx_thread_current_ptr[core_index] != _tx_thread_execute_ptr[core_index])
            {

                /* Yes, thread preemption should take place.  */

#ifdef TX_ENABLE_STACK_CHECKING
            TX_THREAD   *thread_ptr;

                /* Pickup the next execute pointer.  */
                thread_ptr =  _tx_thread_execute_ptr[core_index];

                /* Determine if there is a thread pointer.  */
                if (thread_ptr != TX_NULL)
                {

                    /* Check this thread's stack.  */
                    TX_THREAD_STACK_CHECK(thread_ptr)
                }
#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                /* Determine if an idle system return is present.  */
                if (_tx_thread_execute_ptr[core_index] == TX_NULL)
                {

                    /* Yes, increment the return to idle return count.  */
                    _tx_thread_performance_idle_return_count++;
                }
                else
                {

                    /* No, there is another thread ready to run and will be scheduled upon return.  */
                    _tx_thread_performance_non_idle_return_count++;
                }
#endif

#ifndef TX_NOT_INTERRUPTABLE

                /* Increment the preempt disable flag in order to keep the protection.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE
#endif

                /* Return to the system so the higher priority thread can be scheduled.  */
                _tx_thread_system_return();

#ifdef TX_NOT_INTERRUPTABLE

                /* Restore interrupts.  */
                TX_RESTORE
#endif

                /* Clear the restore needed flag, since the interrupt poster/protection has been done.  */
                restore_needed =  TX_FALSE;
            }
        }
    }

    /* Determine if the protection still needs to be restored.  */
    if (restore_needed == TX_TRUE)
    {

        /* Restore interrupts.  */
        TX_RESTORE
    }
}

