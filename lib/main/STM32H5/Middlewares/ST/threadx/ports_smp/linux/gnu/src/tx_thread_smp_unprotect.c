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
/*    _tx_thread_smp_unprotect                          SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function releases previously obtained protection. The supplied */
/*    previous interrupt posture is restored.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Previous interrupt posture                                          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    pthread_self                                                        */
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
void  _tx_thread_smp_unprotect(UINT new_interrupt_posture)
{

UINT        core;
pthread_t   current_thread_id;

    /* Lock Linux mutex.  */
    _tx_linux_mutex_obtain(&_tx_linux_mutex);

    /* Pickup the current thread ID.  */
    current_thread_id = pthread_self();

    /* Pickup the current core.   */
    core =  _tx_thread_smp_core_get();

    /* Determine if this core owns the protection.  */
    if (_tx_thread_smp_protection.tx_thread_smp_protect_core == core)
    {

        /* Yes, this core owns the protection.  */

        /* Decrement the protection count.  */
        _tx_thread_smp_protection.tx_thread_smp_protect_count--;

        /* Is the protection still in force?  */
        if (_tx_thread_smp_protection.tx_thread_smp_protect_count == 0)
        {

            /* Restore the global interrupt disable value.  */
            _tx_linux_global_int_disabled_flag =  new_interrupt_posture;

            /* Determine if the preemption disable flag is set.  */
            if (_tx_thread_preempt_disable == 0)
            {

                /* Release the protection.  */

                /* Indicate the protection is no longer in force.  */
                _tx_thread_smp_protection.tx_thread_smp_protect_in_force =         TX_FALSE;
                _tx_thread_smp_protection.tx_thread_smp_protect_thread =           TX_NULL;
                _tx_thread_smp_protection.tx_thread_smp_protect_core =             0xFFFFFFFF;
                _tx_thread_smp_protection.tx_thread_smp_protect_linux_thread_id =  0;

                /* Debug entry.  */
                _tx_linux_debug_entry_insert("UNPROTECT-keep", __FILE__, __LINE__);
            }
            else
            {

                /* Debug entry.  */
                _tx_linux_debug_entry_insert("UNPROTECT-released", __FILE__, __LINE__);
            }
        }
        else
        {

            /* Debug entry.  */
            _tx_linux_debug_entry_insert("UNPROTECT-nested", __FILE__, __LINE__);
        }

        /* Only release the critical section.  */
        _tx_linux_mutex_release(&_tx_linux_mutex);
    }

    /* Release the critical section.  */
    _tx_linux_mutex_release(&_tx_linux_mutex);

}
