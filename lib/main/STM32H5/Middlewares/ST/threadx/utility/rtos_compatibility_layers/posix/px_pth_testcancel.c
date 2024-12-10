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
/*    pthread_testcancel                                  PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    The pthread_testcancel function shall request that thread be        */
/*    canceled if the cancel state is enabled and cancel type is          */
/*    deferred, and a cancellation request has happened in the past.      */
/*    The target thread's cancelability state and type determines when    */
/*    the cancellation takes effect. When the cancellation is acted on,   */
/*    the cancelation cleanup handlers for thread shall be called. When   */
/*    the last cancelation cleanup handler returns, the thread-specific   */
/*    data destructor functions shall be called for thread. When the last */
/*    destructor function returns, thread shall be terminated.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread                      pthread handle to thread to be canceled */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*         posix_destroy_thread                                           */
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
VOID pthread_testcancel(VOID)
{

POSIX_TCB   *pthread_ptr;

    
    /* Get the thread identifier of the calling pthread */ 
    pthread_ptr = posix_tid2tcb(pthread_self()); 

    /* Check if thread was created with cancel enable */
    if ( (pthread_ptr->cancel_state == PTHREAD_CANCEL_ENABLE) &&
           (pthread_ptr->cancel_type==PTHREAD_CANCEL_DEFERRED) &&
           (pthread_ptr->cancel_request==TRUE) )
    {
       
        /* Signal the housekeeping ThreadX thread to cancel (delete) this pthread */ 
        posix_destroy_pthread(pthread_ptr,(VOID *)0);
   }
}

