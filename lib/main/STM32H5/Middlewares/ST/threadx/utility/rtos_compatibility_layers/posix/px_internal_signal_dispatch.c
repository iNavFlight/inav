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
#include "tx_thread.h"  /* Internal ThreadX thread management.  */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    internal_signal_dispatch                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     Signal handler thread.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    id                             Signal                               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*     none                                                               */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_internal_error                  Generic error Handler         */
/*    tx_thread_identify                                                  */
/*    tx_event_flags_set                                                  */
/*    tx_thread_resume                                                    */
/*    posix_destroy_pthread                                               */
/*    tx_thread_suspend                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Internal Code                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
void  internal_signal_dispatch(ULONG id)
{

TX_INTERRUPT_SAVE_AREA

POSIX_TCB   *signal_thread;
POSIX_TCB   *target_thread;
VOID        (*handler)(int);


    /* Determine if the desired signal is valid.  */
    if (id > SIGRTMAX)
    {
    
        /* System error!  */
        posix_internal_error(444);
        return;
    }

    /* Pickup signal thread.  */
    signal_thread =  (POSIX_TCB *) tx_thread_identify();

    /* Is it non-NULL?  */
    if (!signal_thread)
    {

        /* System error!  */
        posix_internal_error(444);
        return;
    }

    /* Pickup target thread.  */
    target_thread =  signal_thread -> signals.base_thread_ptr;

    /* Pickup signal handler function pointer.  */
    handler =  target_thread -> signals.signal_func[id];

    /* See if there is a signal handler setup for this signal.  */
    if (handler)
    {
    
        /* Yes, there is a signal handler - call it!  */
        (handler)((int) id);
    }

    /* Set the event flag corresponding the signal.  */
    tx_event_flags_set(&(target_thread -> signals.signal_event_flags), (((ULONG) 1) << id), TX_OR);

    /* Ensure the flag is left in a clear state.  */
    tx_event_flags_set(&(target_thread -> signals.signal_event_flags), ~(((ULONG) 1) << id), TX_AND);

    /* Now we need to clear this signal and terminate this signal handler thread.  */
    
    /* Disable interrupts.  */
    TX_DISABLE
   
    /* Clear this signal from the pending list.  */
    target_thread -> signals.signal_pending.signal_set =  target_thread -> signals.signal_pending.signal_set & ~(((unsigned long) 1) << id);
    
    /* Decrement the signal nesting count.  */
    target_thread -> signals.signal_nesting_depth--;
    
    /* Is this the last nested signal leaving?  */
    if (target_thread -> signals.signal_nesting_depth == 0)
    {
    
        /* Clear the top signal thread link and resume the target thread.  */
        target_thread -> signals.top_signal_thread =  NULL;
        
        /* Restore interrupts.  */
        TX_RESTORE
        
        /* Resume the target thread.  */
        tx_thread_resume((TX_THREAD *) target_thread);
    }
    else
    {
    
        /* Otherwise, there are more signal threads still active.   */
        
        /* Setup the new top signal thread pointer.  */
        target_thread -> signals.top_signal_thread =  signal_thread -> signals.next_signal_thread;

        /* Restore interrupts.  */
        TX_RESTORE
        
        /* Resume the signal handler thread.  */
        tx_thread_resume((TX_THREAD *) signal_thread -> signals.next_signal_thread);
    }

    /* Now we need to mark this signal thread for destruction.  */
    posix_destroy_pthread(signal_thread,(VOID *) 0);
    
    /* Self-suspend the current signal thread.  */
    tx_thread_suspend((TX_THREAD *) signal_thread); 
}
