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

extern sem_t _tx_linux_isr_semaphore;
UINT _tx_linux_timer_waiting = 0;
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_context_restore                        SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function restores the interrupt context if it is processing a  */
/*    nested interrupt.  If not, it returns to the interrupt thread if no */
/*    preemption is necessary.  Otherwise, if preemption is necessary or  */
/*    if no thread was running, the function returns to the scheduler.    */
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
/*    sem_trywait                                                         */
/*    tx_linux_sem_post                                                   */
/*    tx_linux_sem_wait                                                   */
/*    _tx_linux_thread_resume                                             */
/*    _tx_linux_mutex_release_all                                         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ISRs                                  Interrupt Service Routines    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_thread_context_restore(VOID)
{

TX_THREAD   *current_thread;

    /* The critical section is already in force at this point.  */

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("CONTEXT_RESTORE", __FILE__, __LINE__);

    /* For Linux, ISRs are always mapped to core 0.  */

    /* Decrement the nested interrupt count.  */
    _tx_thread_system_state[0]--;

    /* Pickup current thread.  */
    current_thread =  _tx_thread_current_ptr[0];

    /* Determine if this is the first nested interrupt and if a ThreadX
       application thread was running at the time.  */
    if ((!_tx_thread_system_state[0]) && (current_thread))
    {

        /* Yes, this is the first and last interrupt processed.  */

        /* Check to see if preemption is required.  */
        if ((_tx_thread_preempt_disable == 0) && (current_thread != _tx_thread_execute_ptr[0]))
        {

            /* Preempt the running application thread.  We don't need to suspend the
               application thread since that is done in the context save processing.  */

            /* Indicate that this thread was suspended asynchronously.  */
            current_thread -> tx_thread_linux_suspension_type =  1;

            /* Save the remaining time-slice and disable it.  */
            if (_tx_timer_time_slice[0])
            {

                current_thread -> tx_thread_time_slice =  _tx_timer_time_slice[0];
                _tx_timer_time_slice[0] =  0;
            }

            /* Clear the current thread pointer.  */
            _tx_thread_current_ptr[0] =  TX_NULL;

            /* Clear this mapping entry.  */
            _tx_linux_virtual_cores[0].tx_thread_smp_core_mapping_thread =          TX_NULL;
            _tx_linux_virtual_cores[0].tx_thread_smp_core_mapping_linux_thread_id = 0;

            /* Indicate that this thread is now ready for scheduling again by another core.  */
            current_thread -> tx_thread_smp_core_control =  1;

            /* Make sure semaphore is 0. */
            while(!sem_trywait(&_tx_linux_scheduler_semaphore));

            /* Indicate it is in timer ISR. */
            _tx_linux_timer_waiting = 1;

            /* Wakeup the system thread by setting the system semaphore.  */
            tx_linux_sem_post(&_tx_linux_scheduler_semaphore);

            if(_tx_thread_execute_ptr[0])
            {
                if(_tx_thread_execute_ptr[0] -> tx_thread_linux_suspension_type == 2)
                {

                    /* Unlock linux mutex. */
                    _tx_linux_mutex_release_all(&_tx_linux_mutex);

                    /* Wait until TX_THREAD start running. */
                    tx_linux_sem_wait(&_tx_linux_isr_semaphore);

                    _tx_linux_mutex_obtain(&_tx_linux_mutex);

                    /* Make sure semaphore is 0. */
                    while(!sem_trywait(&_tx_linux_isr_semaphore));
                }
            }

            /* Indicate it is not in timer ISR. */
            _tx_linux_timer_waiting = 0;
        }
        else
        {

            /* Since preemption is not required, resume the interrupted thread.  */
            _tx_linux_thread_resume(current_thread -> tx_thread_linux_thread_id);
        }
    }

    /* Unlock linux mutex. */
    _tx_thread_smp_unprotect(TX_INT_ENABLE);
}

