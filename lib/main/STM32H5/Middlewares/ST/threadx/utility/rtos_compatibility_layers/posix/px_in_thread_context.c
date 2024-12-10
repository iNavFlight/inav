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
/** POSIX wrapper for THREADX                                             */ 
/**                                                                       */
/**                                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Include necessary system files.  */

#include "tx_api.h"     /* Threadx API */
#include "pthread.h"    /* Posix API */
#include "px_int.h"     /* Posix helper functions */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    posix_in_thread_context                              PORTABLE C     */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function determines if the system is currently in "thread"     */
/*      context, i.e. not timer routine, not ISR, not idling.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_TRUE | TX_FALSE                                                  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    posix internal code                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
ULONG  posix_in_thread_context(VOID)
{
    /* External variables, defined within ThreadX, needed here. */
    extern    TX_THREAD *     _tx_thread_current_ptr;
    extern    ULONG           _tx_thread_system_state;
#ifndef TX_TIMER_PROCESS_IN_ISR
    extern    TX_THREAD       _tx_timer_thread;
#endif
    
    /*     Return TX_FALSE if any of the following are true:
         - we are in the scheduling loop (not in a thread);
         - we are in an ISR
         - we are in a timer routine
           Return TX_TRUE otherwise (we are in a thread.)
   */

    if (_tx_thread_system_state == TX_INITIALIZE_IN_PROGRESS)
    {

        /* We are calling from initialization, return TRUE.  */
        return(TX_TRUE);
    }
    else if ((!_tx_thread_current_ptr)                  /* Not in a thread */
             || (_tx_thread_system_state)               /* In an ISR       */
#ifndef TX_TIMER_PROCESS_IN_ISR
                                                        /* Timer routine   */
             || (_tx_thread_current_ptr == &_tx_timer_thread) 
#endif
            )
       
    {
        /* We are NOT in thread (thread) context.  */
        return (TX_FALSE);
    }
    else
    {
        /* We ARE in thread (thread) context.  */
        return (TX_TRUE);
    }
}
