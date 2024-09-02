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
/*    sem_close                                           PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine is called to indicate that the calling thread is       */
/*    finished with the specified named semaphore, sem.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    sem                           type of semaphore                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                            If successful                         */
/*    ERROR                         If error occurs                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_internal_error          Generic error handler                 */
/*    posix_sem_reset               Resets the semaphore structure        */
/*    tx_thread_identify            Returns currently running thread      */
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
INT  sem_close(sem_t  * sem)
{

TX_SEMAPHORE    * TheSem;
TX_THREAD       * Thethread;

    /* Make sure we're calling this routine from a thread context.  */
    Thethread = tx_thread_identify();

    if (!Thethread)
    {
        /* Return appropriate error.  */
        posix_internal_error(240);

        /* there is no error defined for this in POSIX, hence give default.  */
        return(ERROR);
    }

    /* Get ThreadX semaphore.  */
    TheSem = (TX_SEMAPHORE * )sem;

    /* Check for an invalid semaphore pointer.  */
    if ((!TheSem) || (TheSem -> tx_semaphore_id != TX_SEMAPHORE_ID))
    {

        /* Return appropriate error.  */
        posix_errno = EINVAL;
	    posix_set_pthread_errno(EINVAL);

        /* return error.  */
        return(ERROR);
    }
    if(sem)
        sem->count -= 1;

    if(! (sem->count) )
    {

        if(sem->unlink_flag == TX_TRUE)
        {
            posix_sem_reset(sem );
            sem = NULL;
        }
    }
    return(OK);
}
