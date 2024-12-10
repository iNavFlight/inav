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
/*    pthread_sigmask                                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function shall examine or change (or both) the calling         */
/*    thread's signal mask.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    how                            how the set will be changed          */
/*    newmask                        pointer to new set of signals        */
/*    oldmask                        pointer to store previous signals    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                        If successful                             */
/*    EINVAL                    If error occurs                           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_set_pthread_errno                                             */
/*    tx_thread_identify                                                  */
/*    pthread_kill                                                        */
/*    _tx_thread_system_preempt_check                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Update pthread_kill argument  */
/*                                            cast,                       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

int   pthread_sigmask(int how, const sigset_t *newmask, sigset_t *oldmask)
{

TX_INTERRUPT_SAVE_AREA

ULONG       blocked_signals;
ULONG       released_signals;
ULONG       signal_number;
ULONG       previous_mask;
POSIX_TCB   *base_thread;
ULONG       reissue_flag;


    /* Check for a valid how parameter.  */
    if ((how != SIG_BLOCK) && (how != SIG_SETMASK) & (how != SIG_UNBLOCK))
    {
    
        /* Return an error.  */
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }

    /* Check for valid signal masks.  */
    if ((newmask == NULL) || (oldmask == NULL))
    {
    
        /* Return an error.  */
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }

    /* Pickup base thread, since the base thread and all signal threads will pend off the same
       event flag group.  */
    base_thread =  (POSIX_TCB *) tx_thread_identify();

    /* Is it non-NULL?  */
    if (!base_thread)
    {

        /* System error!  */
        posix_set_pthread_errno(ESRCH);
        return(EINVAL);
    }

    /* Determine if the current thread is a signal handler thread.  */
    if (base_thread -> signals.signal_handler)
    {
    
        /* Pickup target thread.  */
        base_thread =  base_thread -> signals.base_thread_ptr;
    }

    /* Save the current signal mask for return.  */
    previous_mask =  base_thread -> signals.signal_mask.signal_set;

    /* Now process based on how the mask is to be changed.  */
    if (how == SIG_BLOCK)
    {
    
        /* Simply set the mask to block the signal(s). */
        base_thread -> signals.signal_mask.signal_set =  base_thread -> signals.signal_mask.signal_set | newmask -> signal_set;
    }
    else
    {

        /* Now calculate the set of currently pending signals there are waiting based on the current mask.  */
        blocked_signals = base_thread -> signals.signal_mask.signal_set & base_thread -> signals.signal_pending.signal_set;

        /* Now modify the singal mask correspondingly.  */
        if (how == SIG_UNBLOCK)
        {
        
            /* Clear only the signals specified in the new signal mask.  */
            base_thread -> signals.signal_mask.signal_set =  base_thread -> signals.signal_mask.signal_set & ~(newmask -> signal_set);
        }
        else
        {
        
            /* Simply set the signal mask to the new signal mask value.  */
            base_thread -> signals.signal_mask.signal_set =  newmask -> signal_set;
        }
    
        /* Now determine if there are any signals that need to be activated.  */
        released_signals =  blocked_signals & ~(base_thread -> signals.signal_mask.signal_set);

        /* Are there any signals that need to be activated?  */
        if (released_signals)
        {

            /* Temporarily disable interrupts.  */
            TX_DISABLE

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;
            
            /* Restore interrupts.  */
            TX_RESTORE
        
            /* Set the reissue flag to false.  */
            reissue_flag =  TX_FALSE;
              
            /* Loop to process all the blocked signals.  */
            signal_number = 0;
            while ((released_signals) && (signal_number < 32))
            {
        
                /* Determine if this signal was released.  */
                if (released_signals & 1)
                {
            
                    /* Yes, this signal was released.  We need to make it active again.  */
                    
                    /* Clear the pending bit so the pthread_kill call will not discard the signal (signals are not queued in this implementation).  */
                    base_thread -> signals.signal_pending.signal_set  =  base_thread -> signals.signal_pending.signal_set & ~(((unsigned long) 1) << signal_number);

                    /* Call pthread_kill to reissue the signal.  */
                    pthread_kill((ALIGN_TYPE) base_thread, signal_number);
                    
                    /* Set the reissue flag.  */
                    reissue_flag =  TX_TRUE;
                }
                
                /* Look for next signal.  */
                released_signals =  released_signals >> 1;
                signal_number++;
            }

            /* Temporarily disable interrupts.  */
            TX_DISABLE

            /* Release preemption.  */
            _tx_thread_preempt_disable--;
            
            /* Restore interrupts.  */
            TX_RESTORE

            /* Check for a preemption condition.  */
            _tx_thread_system_preempt_check();
            
            /* Determine if the reissue flag is set.  */
            if (reissue_flag == TX_TRUE)
            {

                /* Relinquish to allow signal thread at same priority to run before we return.  */
                _tx_thread_relinquish();              
            }
        }
    }    

    /* Setup return mask.  */
    oldmask -> signal_set =  previous_mask;

    return(OK);
}
