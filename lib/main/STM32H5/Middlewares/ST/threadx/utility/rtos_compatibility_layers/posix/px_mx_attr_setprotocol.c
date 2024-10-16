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
/*    pthread_mutexattr_setprotocol                       PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function shall sets the mutex protocol attribute.              */
/*    The protocol of a mutex is contained in the protocol attribute of   */
/*    the mutex  attributes                                               */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*    attr                           Pointer to the mutex attributes      */
/*    protocol                       mutex protocol                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     0                             If successful                        */ 
/*     Value                         In case of any error                 */ 
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
INT  pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, INT protocol)
{

    /* First check the attribute object is already destroyed? */
    if (attr->in_use == TX_FALSE)
    {
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }
    else
    {
        if (protocol == PTHREAD_PRIO_INHERIT )
        {   
            attr->protocol  = protocol;             
            return(OK);
        }
        else
        {
            posix_errno = ENOSYS;
	        posix_set_pthread_errno(ENOSYS);
            return(ENOSYS);
        }
    }
}
