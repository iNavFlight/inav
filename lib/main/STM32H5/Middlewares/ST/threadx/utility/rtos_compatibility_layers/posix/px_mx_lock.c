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
/*    pthread_mutex_lock                                   PORTABLE C     */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function locks the mutex object referenced. If the mutex is    */
/*    already locked, the calling thread shall block until the mutex      */
/*    becomes available. This operation shall return with the mutex object*/
/*    referenced by mutex in the locked state with the calling thread as  */
/*    its owner.                                                          */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*    mutex                          Address of the mutex                 */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     0                             if successful                        */ 
/*     Value                         in case of any error                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*  tx_thread_identify               Get calling thread's pointer         */ 
/*  tx_mutex_get                     ThreadX Mutex Service                */ 
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
INT pthread_mutex_lock(pthread_mutex_t *mutex )
{
    
TX_MUTEX         *mutex_ptr;
TX_THREAD        *thread_ptr;
INT               retval,status;

    mutex_ptr = (TX_MUTEX*)mutex;

    thread_ptr = tx_thread_identify();
    if ( (mutex_ptr->tx_mutex_ownership_count > 0 ) && (thread_ptr == (mutex_ptr->tx_mutex_owner )))
    {
        posix_errno = EDEADLK;
	    posix_set_pthread_errno(EINVAL);
        return (EDEADLK);
    }
    status = tx_mutex_get( mutex_ptr, TX_WAIT_FOREVER);
    switch ( status)
    {
    case TX_SUCCESS:
        retval = OK;
        break;
        
    default:
        posix_errno = EINVAL;
	    posix_set_pthread_errno(EINVAL);
        retval = EINVAL;
        break;
    }
    return(retval);
}
