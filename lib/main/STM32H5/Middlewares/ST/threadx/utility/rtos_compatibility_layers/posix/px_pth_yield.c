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
/*    pthread_yield                                       PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This subroutine forces the calling thread to relinquish use of its  */
/*    processor,and to wait in the run queue before it is scheduled again.*/
/*    If the run queue is empty when this subroutine is called, the       */
/*    calling thread is immediately rescheduled.                          */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*     Nothing                                                            */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     None                                                               */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    posix_internal_error                 posix internal error function  */ 
/*    tx_thread_relinquish                 ThreadX function               */
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
VOID pthread_yield(VOID)
{
     /* Make sure we're calling this routine from a thread context.  */
    if (!posix_in_thread_context())
    {
        /* return POSIX error.  */
        posix_internal_error(444);
    }

    /* This relinquishes the CPU.  */
    tx_thread_relinquish();
}
