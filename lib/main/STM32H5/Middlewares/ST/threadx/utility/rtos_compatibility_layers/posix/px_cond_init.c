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
/*  pthread_cond_init                                     PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function shall initialize the condition variable referenced by */
/*    cond with attributes referenced by attr. If attr is NULL,the default*/
/*    condition variable attributes shall be used; the effect is the same */
/*    as passing the address of a default condition variable attributes   */
/*    object. Upon successful initialization, the state of the condition  */
/*    variable shall become initialized.                                  */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*     Nothing                                                            */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     cond                               condition variable object       */
/*     attr                               attributes                      */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_semaphore_create                 create a semaphore internal to  */
/*                                        cond variable                   */ 
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
INT pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr)
{

TX_SEMAPHORE   *semaphore_ptr;
UINT            status;
    
    cond->in_use = TX_TRUE;
    /* Get the condition variable's internal semaphore.  */
    /* Simply convert the COndition variable control block into a semaphore  a cast */ 
    semaphore_ptr = (&( cond->cond_semaphore )); 
    /* Now create the internal semaphore for this cond variable */
    status = tx_semaphore_create(semaphore_ptr,"csem",0);
    if (status != TX_SUCCESS)
    {
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }
    else
        return(OK);
}
