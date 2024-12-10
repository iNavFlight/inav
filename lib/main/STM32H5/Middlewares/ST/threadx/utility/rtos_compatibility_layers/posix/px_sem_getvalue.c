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
/*    sem_getvalue                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine updates the location referenced by the sval argument   */
/*    to have the value of the semaphore referenced by sem without        */
/*    affecting the state of the semaphore                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    *sem                  pointer to Semaphore.                         */
/*    *sval                 Buffer by which the value is returned.        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                    If successful                                 */
/*    ERROR                 IF fails                                      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
INT sem_getvalue(sem_t * sem,ULONG * sval)
{

TX_INTERRUPT_SAVE_AREA

TX_SEMAPHORE   *TheSem;

    /* get ThreadX semaphore.  */ 
    TheSem = (TX_SEMAPHORE *)sem;

    /* First, check for an invalid semaphore pointer.  */
    if ((!TheSem) || (TheSem -> tx_semaphore_id != TX_SEMAPHORE_ID))
    {
        /* Return appropriate error.  */
        posix_errno=EINVAL;
	    posix_set_pthread_errno(EINVAL);
        /* Return Error.  */
        return(EINVAL);
    }

    /* Disable interrupts.  */
    TX_DISABLE

    * sval = TheSem ->tx_semaphore_count;

    /* Restore Interrupts.  */
    TX_RESTORE

    return(OK);
}
