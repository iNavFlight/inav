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
/*    pthread_exit                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*                                                                        */
/*     The pthread_exit() function terminates the calling thread, making  */
/*     its exit status available to any waiting threads.Normally,a thread */
/*     terminates by returning from the start routine that was specified  */
/*     in the pthread_create() call which started it.                     */
/*     An implicit call to pthread_exit() occurs when any thread returns  */
/*     from its start routine. (With the exception of the initial thread, */
/*     at which time an implicit call to exit() occurs).                  */
/*     The pthread_exit() function provides an interface similar to exit()*/
/*     but on a per-thread basis.                                         */
/*                                                                        */   
/*     pthread_exit() does not return.                                    */                 
/*                                                                        */   
/*                                                                        */
/*  INPUT                                                                 */
/*       value_ptr                 exit parameter                         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*     none                        pthread_exit() does not return.        */
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
VOID pthread_exit(void *value_ptr)
{

TX_THREAD   *thread_ptr;
POSIX_TCB   *pthread_ptr;

    /* Get the thread identifier of the currently running thread */ 
    thread_ptr = tx_thread_identify(); 
    /* get posix TCB for this pthread */
    pthread_ptr = (POSIX_TCB *)thread_ptr;
        
    /* Signal the housekeeping ThreadX thread to delete the requested pthread */ 
    posix_destroy_pthread(pthread_ptr,value_ptr); 

    /* Indicate success.  */ 
    return; 

}
