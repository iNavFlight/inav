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
/*    sem_post                                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function releases/puts back a semaphore token.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    sem                                   semaphore descriptor          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    temp1                                 If successful                 */
/*    ERROR                                 If fails                      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_semaphore_put                      ThreadX Semaphore put         */
/*    posix_internal_error                  Internal posix error          */
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
INT sem_post(sem_t * sem)
{

TX_SEMAPHORE    *TheSem;
UINT             temp1;

    /* Assign a temporary variable for clarity.  */
    TheSem = (TX_SEMAPHORE *)sem;

    if( !TheSem || TheSem->tx_semaphore_id != TX_SEMAPHORE_ID)
    {
        /* Set appropriate errno.  */
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        /* Return ERROR.  */
        return (EINVAL);
    }

    /* Place an instance of the semaphore.  */
    temp1 = tx_semaphore_put(TheSem);

    /* Make sure the semaphore was incremented.  */
    if (temp1 != TX_SUCCESS)
    {

        /* return generic error.  */
        posix_internal_error(159);

        /* Return ERROR.  */
        return (ERROR);
    }

    /* All done.  */
    return(temp1);
}
