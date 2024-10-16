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
/*    sem_destroy                                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function destroys a nameless semphore                          */
/*    Waiting threads will be rescheduled                                 */
/*    No memory will be released                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    sem                           Semaphore pointer                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    EINVAL                        if error occurs                       */
/*    EBUSY                         if threads were waiting               */
/*    NO_ERROR                      successful                            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_semaphore_delete           deletes actual THreadx semaphore      */
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
INT sem_destroy(sem_t *sem)
 {
    INT result;

    if((sem==NULL)|| ( sem->in_use == TX_FALSE))
    {
        result = EINVAL; /* general error */

    }
    else 
    {
        if(sem->sem.tx_semaphore_suspended_count > 0 ) result = EBUSY;
        else result = NO_ERROR;
        tx_semaphore_delete(&(sem->sem));
        sem->in_use = TX_FALSE;
    }


    return result;
}
