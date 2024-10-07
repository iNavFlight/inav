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
#include "pthread.h"   /* Posix API */
#include "px_int.h"    /* Posix helper functions */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    pthread_setcanceltype                               PORTABLE C      */ 
/*                                                           6.1.7        */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */
/*    The pthread_setcancelstate()function shall atomically both get the  */
/*    calling thread's cancelability type to the indicated type and       */
/*    return the previous cancelability type at the location referenced   */
/*    by oldtype. Legal values for type are PTHREAD_CANCEL_DEFERRED and   */
/*    PTHREAD_CANCEL_ASYNCHRONOUS.                                        */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*    type                           New cancelability type to be set     */
/*    oldtype                        Pointer to old cancelability type    */
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
INT pthread_setcanceltype (INT type, INT *oldtype)
{
TX_INTERRUPT_SAVE_AREA

TX_THREAD     *thread_ptr;
POSIX_TCB     *pthread_ptr;

    /* First check for validity of the new cancel type to be set  */
    if ( ( type  == PTHREAD_CANCEL_DEFERRED ) || ( type == PTHREAD_CANCEL_ASYNCHRONOUS ) ) 
    {
        TX_DISABLE

            /* Get the thread identifier of the currently running thread */ 
            thread_ptr = tx_thread_identify(); 
            /* get posix TCB for this pthread */
            pthread_ptr = (POSIX_TCB *)thread_ptr;
            *oldtype = pthread_ptr->cancel_type;
            pthread_ptr->cancel_type = type;

        TX_RESTORE

        return(OK);
    }
    else
    {
        posix_errno  = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(EINVAL);
    }
}    
