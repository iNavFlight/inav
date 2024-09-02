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
/*    pthread_mutex_trylock                               PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function shall be equivalent to pthread_mutex_lock(), except   */
/*    that if the mutex object referenced by mutex is currently locked    */
/*    (by any thread,including the current thread), the call shall return */
/*    immediately. If the mutex type is PTHREAD_MUTEX_RECURSIVE and the   */
/*    mutex is currently owned by the calling thread,the mutex lock count */
/*    shall be incremented by one and the pthread_mutex_trylock()function */
/*    shall immediately return success.                                   */ 
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*    mutex                          Pointer to the mutex object          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     0                             If successful                        */ 
/*     Value                         In case of any error                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                   ThreadX Mutex get service.           */ 
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
INT pthread_mutex_trylock(pthread_mutex_t *mutex)
{

TX_MUTEX    *mutex_ptr;
INT          retval,status;

    /* convert pthread mutex object to ThreadX mutex  */
    mutex_ptr = (TX_MUTEX *)mutex;

    /* Try to get the mutex */
    status = tx_mutex_get( mutex_ptr, TX_NO_WAIT);
   
    switch ( status)
    {
        case TX_SUCCESS:
            retval = OK;
            break;
        
        case TX_DELETED:
            posix_errno  = EINVAL;
	        posix_set_pthread_errno(EINVAL);
            retval = EINVAL;
            break;

        case TX_NOT_AVAILABLE:
            posix_errno  = EBUSY;
	        posix_set_pthread_errno(EBUSY);
            retval = EBUSY;
            break;

        default:
            posix_errno  = EINVAL;
	        posix_set_pthread_errno(EINVAL);
            retval = EINVAL;
            break;
    }
    return (retval);
}
