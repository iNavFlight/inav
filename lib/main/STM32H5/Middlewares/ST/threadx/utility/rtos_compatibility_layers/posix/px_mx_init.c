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
/*    pthread_mutex_init                                   PORTABLE C     */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function shall init the mutex object referenced by mutex with  */
/*    attributes specified by attr.                                       */
/*    If attr is NULL, the default mutex attributes are used; the effect  */
/*    shall be the same as passing the address of a default mutex         */
/*    attributes object. Upon successful initialization,the state of the  */
/*    mutex becomes initialized and unlocked.                             */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*    mutex                          Pointer to a pthread mutex object    */
/*    attr                           Pointer to mutex attributes          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     0                             If successful                        */ 
/*     Value                         In case of any error                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    posix_internal_error           In case of some special errors       */ 
/*    posix_in_thread_context        Check whether called from a thread   */
/*    tx_mutex_create                Create a ThreadX Mutex object        */ 
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
INT pthread_mutex_init(pthread_mutex_t *mutex ,pthread_mutexattr_t *attr)
{

TX_INTERRUPT_SAVE_AREA

TX_MUTEX         *mutex_ptr;
ULONG             status,retval;



    /* Make sure we're calling this routine from a thread context.  */
    if (!posix_in_thread_context())
    {
        /* return POSIX error.  */
        posix_internal_error(444);
    }

    /* Disable interrupts.  */ 
    TX_DISABLE

    /* Check for any pthread_mutexattr_t suggested */
    if (!attr)
    {
        /* no attributes passed so assume default attributes */
        attr = &(posix_default_mutex_attr);
    }
    else
    {
        /* attributes passed , check for validity */
        if  (( (attr->in_use) == TX_FALSE)|| (attr->type!=PTHREAD_MUTEX_RECURSIVE))
        {   
            /* attributes passed is not valid, return with an error */
            /* Restore interrupts.  */ 
            TX_RESTORE
            posix_errno = EINVAL;
	        posix_set_pthread_errno(EINVAL);
            return(EINVAL);
        }
    }

    mutex_ptr = (&(mutex->mutex_info)) ;

    /* Now actually create the mutex */ 
    status = tx_mutex_create(mutex_ptr, "PMTX", TX_INHERIT);
   
    if ( status == TX_SUCCESS)
    {
        mutex->in_use = TX_TRUE;
        retval = OK;   
    }
    else
    {
        mutex->in_use = TX_FALSE;
        posix_errno =  EINVAL;
	    posix_set_pthread_errno(EINVAL);
        retval = EINVAL;
    }
   
    TX_RESTORE
    return(retval); 
}
