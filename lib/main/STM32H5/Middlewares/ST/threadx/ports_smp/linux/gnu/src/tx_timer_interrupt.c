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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_interrupt                               SMP/Linux/GCC     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the hardware timer interrupt.  This         */
/*    processing includes incrementing the system clock and checking for  */
/*    time slice and/or timer expiration.  If either is found, the        */
/*    interrupt context save/restore functions are called along with the  */
/*    expiration functions.                                               */
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
/*    _tx_timer_expiration_process                                        */
/*    _tx_thread_time_slice                                               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    interrupt vector                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID   _tx_timer_interrupt(VOID)
{

UINT    saved_posture;


    /* Get the protection.  */
    saved_posture =  _tx_thread_smp_protect();

    /* Increment the system active counter.  */
    _tx_timer_interrupt_active++;

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("TIMER INTERRUPT", __FILE__, __LINE__);

    /* Increment the system clock.  */
    _tx_timer_system_clock++;

    /* Test for timer expiration.  */
    if (*_tx_timer_current_ptr)
    {

        /* Set expiration flag.  */
        _tx_timer_expired =  TX_TRUE;
    }
    else
    {

        /* No timer expired, increment the timer pointer.  */
        _tx_timer_current_ptr++;

        /* Check for wrap-around.  */
        if (_tx_timer_current_ptr == _tx_timer_list_end)
        {

            /* Wrap to beginning of list.  */
            _tx_timer_current_ptr =  _tx_timer_list_start;
        }
    }

    /* See if anything has expired.  */
    if (_tx_timer_expired)
    {

        /* Did a timer expire?  */
        if (_tx_timer_expired)
        {

            /* Process timer expiration.  */
            _tx_timer_expiration_process();
        }
    }

    /* Call time-slice processing to process time-slice for all threads on each core.  */
    _tx_thread_time_slice();

    /* Increment the system active counter.  */
    _tx_timer_interrupt_active++;

    /* Release the protection.  */
    _tx_thread_smp_unprotect(saved_posture);
}
