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
/*    pthread_attr_getdetachstate                         PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*     This function returns the detach state attribute from a pthread    */
/*     attributes object specified.The detach state of a thread indicates */
/*     whether the system is allowed to free thread resources when the    */
/*     thread terminates.                                                 */
/*     The detach state specifies one of:                                 */
/*     PTHREAD_CREATE_DETACHED or PTHREAD_CREATE_JOINABLE.                */ 
/*     The default detach state (DEFAULT_DETACHSTATE) is:                 */
/*     PTHREAD_CREATE_JOINABLE.                                           */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*    attr                           Address of the thread attributes     */
/*    detachstate                    Address of variable to contain the   */
/*                                   returned detach state                */ 
/*                                                                        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     0                             if successful                        */ 
/*     Value                         in case of any error                 */ 
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
INT pthread_attr_getdetachstate( pthread_attr_t *attr,INT *detachstate)
{
    /* First check the attribute object is already destroyed? */

    if (attr->inuse == TX_FALSE)
    {
        posix_errno = EINVAL;
	    posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }    
    else
    {
        *detachstate = attr->detach_state ;
        return(OK);
    }
}
