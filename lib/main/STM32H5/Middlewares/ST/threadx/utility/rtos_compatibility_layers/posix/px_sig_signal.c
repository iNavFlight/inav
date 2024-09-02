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
/*    signal                                              PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function assigns a function ("signal handler") pointed to by   */
/*    func to be invoked when signal signo occurs.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    set                            Pointer to set of signals            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                        If successful                             */
/*    ERROR                     If error occurs                           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_in_thread_context                                             */
/*    posix_internal_error                                                */
/*    posix_set_pthread_errno                                             */
/*    tx_thread_identify                                                  */
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
int  signal(int signo, void (*func)(int))
{

POSIX_TCB   *current_thread;


    /* Make sure we're calling this routine from a thread context.  */
    if (!posix_in_thread_context())
    {
        /* return POSIX error.  */
        posix_internal_error(444);
        return(444);
    }

    /* Determine if the desired signal is valid.  */
    if ((signo < 0) || (signo > SIGRTMAX))
    {
    
        /* Return an error.  */
        posix_set_pthread_errno(EINVAL);
        return(ERROR);
    }

    /* Now pickup the current thread pointer.  */
    current_thread =  (POSIX_TCB *) tx_thread_identify();
    
    /* Now index into the array of signal handlers and insert this one.  */
    current_thread -> signals.signal_func[signo] =  func;
    
    /* Return success!  */
    return(OK);
}
