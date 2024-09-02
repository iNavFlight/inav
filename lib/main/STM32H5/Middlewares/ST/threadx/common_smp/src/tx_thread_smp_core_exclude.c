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
/**   Thread - High Level SMP Support                                     */
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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_smp_core_exclude                        PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allows the application to exclude one or more cores   */
/*    from executing the specified thread.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to the thread         */
/*    exclusion_map                         Bit map of exclusion list,    */
/*                                            where bit 0 set means that  */
/*                                            this thread cannot run on   */
/*                                            core0, etc.                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_smp_rebalance_execute_list Build execution list          */
/*    _tx_thread_system_return              System return                 */
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
UINT  _tx_thread_smp_core_exclude(TX_THREAD *thread_ptr, ULONG exclusion_map)
{

TX_INTERRUPT_SAVE_AREA

UINT            core_index;
UINT            new_mapped_core;
ULONG           mapped_core;
ULONG           available_cores;
UINT            restore_needed;
UINT            status;


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* First, make sure the thread pointer is valid.  */
    if (thread_ptr == TX_NULL)
    {

        /* Return pointer error.  */
        status =  TX_THREAD_ERROR;
    }

    /* Check for valid ID.  */
    else if (thread_ptr -> tx_thread_id != TX_THREAD_ID)
    {

        /* Return pointer error.  */
        status =  TX_THREAD_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Set the restore needed flag.  */
        restore_needed =  TX_TRUE;

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

        /* Debug entry.  */
        _tx_thread_smp_debug_entry_insert(2, 0, thread_ptr);
#endif

        /* Build the bitmap for the last mapped core.  */
        mapped_core =  (((ULONG) 1) << thread_ptr -> tx_thread_smp_core_mapped);

        /* Calculate the available cores map.   */
        available_cores =  (~exclusion_map) & ((ULONG) TX_THREAD_SMP_CORE_MASK);

        /* Save the excluded and available cores.  */
        thread_ptr -> tx_thread_smp_cores_excluded =  exclusion_map;
        thread_ptr -> tx_thread_smp_cores_allowed =   available_cores;

        /* Determine if this is within the now available cores.  */
        if ((mapped_core & available_cores) == ((ULONG) 0))
        {

            /* Determine if there are any cores available.  */
            if (available_cores == ((ULONG) 0))
            {

                /* No cores are available, simply set the last running core to 0.  */
                thread_ptr -> tx_thread_smp_core_mapped =  ((UINT) 0);
            }
            else
            {

                /* No, we need set the last mapped core to a valid core.  */
                TX_LOWEST_SET_BIT_CALCULATE(available_cores, new_mapped_core)

                /* Now setup the last core mapped.  */
                thread_ptr -> tx_thread_smp_core_mapped =  new_mapped_core;
            }
        }

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

        /* Determine if the thread is in a ready state.  */
        if (thread_ptr -> tx_thread_state != TX_READY)
        {


            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(3, 0, thread_ptr);

        }
#endif

        /* Determine if the thread is ready.  */
        if (thread_ptr -> tx_thread_state == TX_READY)
        {

            /* Pickup the index.  */
            core_index =  TX_SMP_CORE_ID;

            /* Call the rebalance routine. This routine maps cores and ready threads.  */
            _tx_thread_smp_rebalance_execute_list(core_index);

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(3, 0, thread_ptr);
#endif

            /* Determine if this thread needs to return to the system.  */

            /* Is there a difference between the current and execute thread pointers?  */
            if (_tx_thread_execute_ptr[core_index] != _tx_thread_current_ptr[core_index])
            {

                /* Yes, check to see if we are at the thread level.  */
                if (_tx_thread_system_state[core_index] == ((ULONG) 0))
                {

                    /* At the thread level, check for the preempt disable flag being set.  */
                    if (_tx_thread_preempt_disable == ((UINT) 0))
                    {

#ifndef TX_NOT_INTERRUPTABLE

                        /* Increment the preempt disable flag in order to keep the protection.  */
                        _tx_thread_preempt_disable++;

                        /* Restore interrupts.  */
                        TX_RESTORE
#endif

                        /* Transfer control to the system so the scheduler can execute
                           the next thread.  */
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
        }

        /* Determine if the protection still needs to be restored.  */
        if (restore_needed == TX_TRUE)
        {

            /* Restore interrupts.  */
            TX_RESTORE
        }
    }

    /* Return status.  */
    return(status);
}

