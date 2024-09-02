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
/*    pthread_cancel                                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    The pthread_cancel function shall request that thread be  canceled. */
/*    The target thread’s cancelability state and type determines when */
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
/*    zero                        If successful                           */
/*    error number                If fails                                */
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
INT pthread_cancel(pthread_t thread)
{

TX_THREAD   *thread_ptr;
POSIX_TCB   *pthread_ptr;

    
    
    
    /* Get the thread identifier of the pthread to be canceled */ 
    thread_ptr = posix_tid2thread(thread); 
    
    if( (thread_ptr->tx_thread_state == TX_COMPLETED) || (thread_ptr->tx_thread_state  == TX_TERMINATED) )
    {
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }
    /* get posix TCB for this pthread */
    pthread_ptr = (POSIX_TCB *)thread_ptr;

   /* Check if target pthread is created with cancel enable */
   if ( pthread_ptr->cancel_state != PTHREAD_CANCEL_ENABLE)
   {
        posix_errno= EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
   }
   if( pthread_ptr->cancel_type==PTHREAD_CANCEL_DEFERRED )
   {
       /* set cancel request for cancelation point */
       pthread_ptr->cancel_request=TRUE;
   }
   else if(pthread_ptr->cancel_type==PTHREAD_CANCEL_ASYNCHRONOUS )
   {
        /* Signal the housekeeping ThreadX thread to cancel (delete) the requested pthread now */ 

       posix_destroy_pthread(pthread_ptr,(VOID *)0); 
   }
   else /* illegal value in pthread_ptr->cancel_type */
   {
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }

   /* Indicate success.  */ 
   return(OK); 
}
