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


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_context_save                             Linux/GNU       */ 
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
/*    tx_linux_mutex_lock                                                 */ 
/*    _tx_linux_thread_suspend                                            */ 
/*    tx_linux_mutex_unlock                                               */ 
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

    /* Debug entry.  */
    _tx_linux_debug_entry_insert("CONTEXT_SAVE", __FILE__, __LINE__);

    /* Lock mutex to ensure other threads are not playing with
       the core ThreadX data structures.  */
    tx_linux_mutex_lock(_tx_linux_mutex);

    /* If an application thread is running, suspend it to simulate preemption. */
    if ((_tx_thread_current_ptr) && (_tx_thread_system_state == 0))
    {

        /* Debug entry.  */
        _tx_linux_debug_entry_insert("CONTEXT_SAVE-suspend_thread", __FILE__, __LINE__);

        /* Yes, this is the first interrupt and an application thread is running...
           suspend it!  */
        _tx_linux_thread_suspend(_tx_thread_current_ptr -> tx_thread_linux_thread_id);

        /* Indicate that this thread was suspended asynchronously.  */
        _tx_thread_current_ptr -> tx_thread_linux_suspension_type =  1;
    }

    /* Increment the nested interrupt condition.  */
    _tx_thread_system_state++;

    /* Unlock linux mutex. */
    tx_linux_mutex_unlock(_tx_linux_mutex);
}

