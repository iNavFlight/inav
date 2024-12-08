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
/*    posix_get_new_sem                                   PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to acquire a ThreadX semaphore from          */
/*    the POSIX semaphore pool.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    tx_sem                                ThreadX semaphore pointer     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    POSIX internal Code                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
TX_SEMAPHORE  * posix_get_new_sem(VOID)
{

ULONG               i;
TX_SEMAPHORE       *tx_sem;
sem_t              *sem_ptr;

    /* Loop through the list of semaphores -  */
    /* try to find one that is not "in use"  */
    /* Start out pessimistic - assume we won't find a match.  */
    tx_sem = (TX_SEMAPHORE *)0;

    /* Search the semaphore pool for an available semaphore.  */
    for (i = 0, sem_ptr = &(posix_sem_pool[0]);
             i < SEM_NSEMS_MAX;
             i++, sem_ptr++)
    {
        /* Make sure the semaphore is not already "in use".  */
        if (sem_ptr->in_use == TX_FALSE)
        {

            /* Found one!  */ 
            tx_sem = MAKE_TX_SEM(sem_ptr);

            /* This semaphore is now in use.  */
            sem_ptr->in_use = TX_TRUE;

            break;
        }
    }
    return(tx_sem);
}
