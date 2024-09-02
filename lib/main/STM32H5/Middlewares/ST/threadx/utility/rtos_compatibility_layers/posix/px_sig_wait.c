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
/*    sigwait                                             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function selects a pending signal from set, atomically         */
/*    clears it from the system's set of pending signals, and returns the */
/*    signal number in the location referenced by sig.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    set                            Pointer to set of signals            */
/*    sig                            Pointer to returned signal number    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                        If successful                             */
/*    EINVAL                    If error occurs                           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_identify                                                  */
/*    posix_internal_error                                                */
/*    pthread_sigmask                                                     */
/*    tx_event_flags_get                                                  */
/*    TX_LOWEST_SET_BIT_CALCULATE                                         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
int   sigwait(const sigset_t *set, int *sig)
{

UINT        status;
ULONG       signal_bit_map;
ULONG       signal_number;
ULONG       saved_mask;
ULONG       changed_mask;
ULONG       pending_signals;
sigset_t    original_set;
POSIX_TCB   *base_thread;


    /* Pickup base thread, since the base thread and all signal threads will pend off the same
       event flag group.  */
    base_thread =  (POSIX_TCB *) tx_thread_identify();

    /* Is it non-NULL?  */
    if (!base_thread)
    {

        /* System error!  */
        posix_internal_error(444);
        return(EINVAL);
    }

    /* Determine if the current thread is a signal handler thread.  */
    if (base_thread -> signals.signal_handler)
    {
    
        /* Pickup target thread.  */
        base_thread =  base_thread -> signals.base_thread_ptr;
    }

    /* Initialize the saved and changed mask values to zero.  */
    saved_mask =  0;
    changed_mask =   0;

    /* Determine if there are any pending signals that are pertinent to this request.  */
    pending_signals =  base_thread -> signals.signal_mask.signal_set & base_thread -> signals.signal_pending.signal_set & set -> signal_set;

    /* Are there any.  */
    if (pending_signals)
    {
    
        /* Yes, there are signals being masked currently that would satisfy this request. */
   
        /* Save the current mask.  */
        saved_mask =  base_thread -> signals.signal_mask.signal_set;

        /* Calculate the changed mask.  */
        changed_mask =  saved_mask & ~(set -> signal_set);

        /* Call pthread_sigmask to temporarily unblock these signals which will release them as well.  */
        pthread_sigmask(SIG_UNBLOCK, set, &original_set);
        
        /* Now determine if the changed mask is still in effect, i.e., there wasn't a pthread_sigmask call from any subsequent signal handlers.  */
        if (base_thread -> signals.signal_mask.signal_set == changed_mask)
        {
        
            /* Yes, restore the previous signal mask.  */
            base_thread -> signals.signal_mask.signal_set =  saved_mask;
        }
    
        /* Derived the signal number from the bit map.  */
        TX_LOWEST_SET_BIT_CALCULATE(pending_signals, signal_number);
        
        /* Return the signal number.  */
        *sig =  (int) signal_number;
        
        /* Return success!  */
        return(OK);
    }
    
    /* Determine if there are any signals that have to be temporarily cleared.  */
    if (base_thread -> signals.signal_mask.signal_set & set -> signal_set)
    {
    
        /* Yes, there are signals being masked needed to satisfy this request. */
   
        /* Save the current mask.  */
        saved_mask =  base_thread -> signals.signal_mask.signal_set;

        /* Calculate the changed mask.  */
        changed_mask =  saved_mask & ~(set -> signal_set);

        /* Apply the changed signal mask.  */
        base_thread -> signals.signal_mask.signal_set =  changed_mask;
    }
    
    /* Suspend on the signal specified by the input.  */
    status =  tx_event_flags_get(&(base_thread -> signals.signal_event_flags), (ULONG) set -> signal_set, TX_OR_CLEAR, &signal_bit_map, TX_WAIT_FOREVER);

    /* Determine if we need to restore the signal mask.  */
    if ((saved_mask) && (changed_mask == base_thread -> signals.signal_mask.signal_set))
    {
    
        /* Yes, the signal mask should be restored.  */
        base_thread -> signals.signal_mask.signal_set =  saved_mask;
    }

    /* Check for successful status.  */
    if (status == TX_SUCCESS)
    {
    
        /* Derived the signal number from the bit map.  */
        TX_LOWEST_SET_BIT_CALCULATE(signal_bit_map, signal_number);
        
        /* Return the signal number.  */
        *sig =  (int) signal_number;
        
        /* Return success!  */
        return(OK);
    }
    else
    {
    
        /* Return error!  */
        return(EINVAL);
    }
}
