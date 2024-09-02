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
/*    sem_unlink                                         PORTABLE C       */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine removes the string from the semaphore name table ,and  */
/*    marks the corresponding semaphore for destruction.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    name                   Semaphore name.                              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    OK                     If successful                                */
/*    ERROR                  IF fails                                     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    posix_find_sem         checks the name of semaphore with the names  */
/*                            of the already created semaphore.           */
/*    posix_sem_reset        Resets the semaphore structure               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Remove double parenthesis,    */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
INT sem_unlink(const CHAR * name)
{


struct POSIX_SEMAPHORE_STRUCT          * sem;
ULONG                                    len;


    /* Checking for the invalid length.  */
    len = strlen(name);

    if(len > SEM_NAME_MAX)
    {
        /* Return appropriate error.  */
        posix_errno=ENAMETOOLONG;
        posix_set_pthread_errno(ENAMETOOLONG);
        /* Return error.  */
        return(ERROR);
    }

    if(!(sem=posix_find_sem(name)))
    {
        /* Set error in global variable.  */
        posix_errno = ENOENT;
	    posix_set_pthread_errno(ENOENT);

        /* Return Error.  */
        return(ERROR); 
    }
    if(sem)

        sem->unlink_flag =TX_TRUE;

    /* Check for the count.  */
    if(sem->count == 0)
    {
        posix_sem_reset(sem );
        sem = NULL;
    }

    return(OK);
}
