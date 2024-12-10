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
/*    pthread_mutex_unlock                                PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function shall release the mutex object referenced by mutex.   */
/*    The manner in which a mutex is released is dependent upon the mutex */
/*    type attribute. If there are threads blocked on the mutex object    */
/*    referenced by mutex when pthread_mutex_unlock() is called,resulting */
/*    in the mutex becoming available,the scheduling policy shall         */
/*    determine which thread shall acquire the mutex.                     */
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
/*    tx_mutex_put                   ThreadX Mutex service                */ 
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
INT pthread_mutex_unlock(pthread_mutex_t *mutex )
{
    
TX_MUTEX        *mutex_ptr;
INT              retval,status;


    /* convert pthread mutex object to ThreadX mutex  */
    mutex_ptr = (TX_MUTEX*)mutex;
    status = tx_mutex_put( mutex_ptr);
    switch ( status)
    {
        case TX_SUCCESS:
            retval = OK;
            break;
        
        case TX_MUTEX_ERROR:
            posix_errno  = EINVAL;
	        posix_set_pthread_errno(EINVAL);
            retval = EINVAL;
            break;

        case TX_NOT_OWNED:
            posix_errno  = EPERM;
	        posix_set_pthread_errno(EPERM);
	    retval = EPERM;
            break;

        default:
            posix_errno  = EINVAL;
	        posix_set_pthread_errno(EINVAL);
            retval = EINVAL;
            break;
    }
    return (retval);
}
