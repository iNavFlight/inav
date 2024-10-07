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

#include "tx_api.h"    /* Threadx API */
#include "pthread.h"  /* Posix API */
#include "px_int.h"    /* Posix helper functions */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    pthread_self                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*                                                                        */
/*    This function returns thread ID of the calling pthread        .     */
/*                                                                        */
/*                                                                        */   
/*                                                                        */   
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*  Nothing                                                               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    thread_ID               pthread handle of the calling thread        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
pthread_t pthread_self(VOID)
{

TX_THREAD   *thread_ptr;
pthread_t    thread_ID; 

    /* Get the thread identifier of the currently running thread */ 
    thread_ptr = tx_thread_identify(); 

    /* Convert thread identifier to posix thread ID */ 
    
    thread_ID = posix_thread2tid(thread_ptr); 

    /* Determine if this thread is actually the signal thread helper.  */
    if (((POSIX_TCB *) thread_ptr) -> signals.signal_handler)
    {
    
        /* Yes, override the thread_ID with the non-signal thread ID.  */
        thread_ID =  (pthread_t) ((POSIX_TCB *) thread_ptr) -> signals.base_thread_ptr;
    }


    /* All done.  */ 
    return(thread_ID); 
}
