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
/*    sem_wait                                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function locks (takes) a semaphore.                            */
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
/*    tx_thread_identify        To check whether calling from a thread    */
/*    tx_semaphore_get          ThreadX Semaphore get                     */
/*    posix_internal_error      Returns a generic error                   */
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
INT  sem_wait( sem_t * sem )
{

TX_SEMAPHORE         * TheSem;


    /* Make sure we're calling this routine from a thread context.  */
    if (! tx_thread_identify())
    {
        /* No wait when called from ISR.  */
        posix_internal_error(242);

        /* Return Error.  */
        return (ERROR);
    }

    /* get ThreadX semaphore.  */
    TheSem = (TX_SEMAPHORE *)sem;

    /* Check for an invalid semaphore pointer.  */
    if ((!TheSem) || (TheSem -> tx_semaphore_id != TX_SEMAPHORE_ID))
    {
        /* error in POSIX.  */
        posix_errno = EINVAL;
	    posix_set_pthread_errno(EINVAL);

        /* Return error.  */
        return (EINVAL);
    }
    else
    {
        /* Takes the semaphore.  */
        if(tx_semaphore_get(TheSem,TX_WAIT_FOREVER))
        {
            /* Return general error.  */
            posix_internal_error(246);

            /* Return error.  */
            return(ERROR);
        }

        return (OK);
    }
}
