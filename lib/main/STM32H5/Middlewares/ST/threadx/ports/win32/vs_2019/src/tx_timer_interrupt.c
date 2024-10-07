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


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"
#include "tx_thread.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_timer_interrupt                               Win32/Visual      */ 
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
/*    _tx_thread_time_slice                 Time slice interrupted thread */ 
/*    _tx_timer_expiration_process          Timer expiration processing   */ 
/*    _tx_win32_critical_section_obtain     Obtain critical section       */ 
/*    _tx_win32_critical_section_release    Release critical section      */ 
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


    /* Enter critical section to ensure other threads are not playing with
       the core ThreadX data structures.  */
    _tx_win32_critical_section_obtain(&_tx_win32_critical_section);

    /* Debug entry.  */
    _tx_win32_debug_entry_insert("TIMER INTERRUPT", __FILE__, __LINE__);

    /* Increment the system clock.  */
    _tx_timer_system_clock++;

    /* Test for time-slice expiration.  */
    if (_tx_timer_time_slice)
    {

        /* Decrement the time_slice.  */
        _tx_timer_time_slice--;

        /* Check for expiration.  */
        if (_tx_timer_time_slice == 0)
        {

           /* Set the time-slice expired flag.  */
           _tx_timer_expired_time_slice =  TX_TRUE;
        }
    }

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
    if ((_tx_timer_expired_time_slice) || (_tx_timer_expired))
    {

        /* Did a timer expire?  */
        if (_tx_timer_expired)
        {

            /* Process timer expiration.  */
            _tx_timer_expiration_process();
        }

        /* Did time slice expire?  */
        if (_tx_timer_expired_time_slice)
        {

            /* Time slice interrupted thread.  */
            _tx_thread_time_slice();
        }
    }

    /* Exit Win32 critical section.  */
    _tx_win32_critical_section_release(&_tx_win32_critical_section);
}

