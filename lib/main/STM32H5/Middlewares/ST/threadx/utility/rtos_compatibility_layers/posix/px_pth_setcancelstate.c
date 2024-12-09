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
/*    pthread_setcancelstate                              PORTABLE C      */ 
/*                                                           6.1.7        */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */ 
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    The pthread_setcancelstate()function shall atomically both set the  */
/*    calling thread’s cancelability state to the indicated state and  */
/*    return the previous cancelability state at the location referenced  */
/*    by oldstate.Legal values for state are PTHREAD_CANCEL_ENABLE and    */
/*    PTHREAD_CANCEL_DISABLE.                                             */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*    state                          New cancelability state to be set    */
/*    oldstate                       Pointer to old cancelability state   */
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
INT pthread_setcancelstate (INT state, INT *oldstate)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD     *thread_ptr;
POSIX_TCB     *pthread_ptr;

    /* First check for validity of the new cancel state to be set  */
    if ( (state  == PTHREAD_CANCEL_ENABLE) || (state == PTHREAD_CANCEL_DISABLE) ) 
    {
        TX_DISABLE

            /* Get the thread identifier of the currently running thread */ 
            thread_ptr = tx_thread_identify(); 
            /* get posix TCB for this pthread */
            pthread_ptr = (POSIX_TCB *)thread_ptr;
            *oldstate = pthread_ptr->cancel_state;
            pthread_ptr->cancel_state = state;

        TX_RESTORE

        return(OK);
    }
    else
    {
        posix_errno  = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return (EINVAL);
    }
}    
