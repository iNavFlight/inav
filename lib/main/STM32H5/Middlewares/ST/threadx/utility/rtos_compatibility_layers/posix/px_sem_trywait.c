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
/*    sem_trywait                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine locks(takes) a semaphore if the semaphore is not       */
/*    currently locked.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    *sem                      Pointer to Semaphore                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                        If successful                             */
/*    ERROR                     If error occurs                           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_internal_error      Returns generic error                     */
/*    tx_semaphore_get          ThreadX Semaphore get                     */
/*    tx_thread_identify        Returns currently running thread          */
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
INT sem_trywait(sem_t * sem)
{

TX_SEMAPHORE          * TheSem;
INT                    retval;
INT                    pxerror;

     /* Make sure we're calling this routine from a thread context.  */
    if (!tx_thread_identify() )
    {
        /* No error in POSIX.  */
        posix_internal_error(242);

        /* Return error.  */
        return (ERROR);
    }

     /* Get ThreadX semaphore.  */
    TheSem = (TX_SEMAPHORE *)sem;

    /* Check for an invalid semaphore pointer.  */
    if ((!TheSem) || (TheSem -> tx_semaphore_id != TX_SEMAPHORE_ID))
    {
        /* error in POSIX.  */
        posix_errno = EINVAL;
	    posix_set_pthread_errno(EINVAL);
        return (EINVAL);
    }
    else
        retval = tx_semaphore_get(TheSem,TX_NO_WAIT);

    switch(retval)
    {

        case TX_SUCCESS :

            sem->refCnt -= 1;
            pxerror = OK;
            break;

        case TX_NO_INSTANCE:

            posix_errno = EAGAIN;
	        posix_set_pthread_errno(EAGAIN);
            pxerror = ERROR;
            break;

        case  TX_DELETED :
        case  TX_WAIT_ABORTED :
        case  TX_SEMAPHORE_ERROR :
        case  TX_WAIT_ERROR :

            /* No error in POSIX, give default.  */
            posix_errno = EINVAL;
	        posix_set_pthread_errno(EINVAL);
            pxerror = ERROR;
            break;
    }
    return(pxerror);
}
