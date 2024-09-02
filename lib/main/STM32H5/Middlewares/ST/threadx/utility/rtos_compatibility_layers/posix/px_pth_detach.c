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
/*    pthread_detach                                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*                                                                        */
/*    The pthread_detach() function indicates that system resources for   */
/*    the specified thread should be reclaimed when the thread ends.      */
/*    If the thread is already ended, resources are reclaimed immediately.*/
/*    This routine does not cause the thread to end.                      */
/*    After pthread_detach() has been issued, it is not valid to try to   */
/*    pthread_join() with the target thread.                              */
/*    Eventually, you should call pthread_join() or pthread_detach() for  */
/*    every thread that is created joinable (with a detachstate of        */
/*    PTHREAD_CREATE_JOINABLE)so that the system can reclaim all resources*/
/*    associated with the thread. Failure to join to or detach joinable   */ 
/*    threads will result in memory and other resource leaks until the    */
/*    process ends. If thread doesn't represent a valid undetached thread,*/
/*    pthread_detach() will return ESRCH.                                 */ 
/*                                                                        */   
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread                      pthread handle to the target thread     */
/*                                                                        */
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
INT pthread_detach(pthread_t thread)

{

TX_INTERRUPT_SAVE_AREA

POSIX_TCB    *pthread_ptr;

    TX_DISABLE

    pthread_ptr = posix_tid2tcb(thread);
    if(pthread_ptr==NULL)
    {
        TX_RESTORE
        return(ESRCH);
    }
    if(pthread_ptr->is_detached==TX_TRUE)
    {
        TX_RESTORE
        return (EINVAL);
    }
    pthread_ptr->is_detached = TX_TRUE;
    
    TX_RESTORE
    return(OK);
}
