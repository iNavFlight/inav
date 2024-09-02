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
/*    clock_gettime                                       PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This subroutine returns the time. The source of the time            */
/*    is the internal Threadx timer variable, which keeps track           */
/*    of the number of ticks of the Threadx timer ISR.                    */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*     clockid_t, tspec                                                   */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     error code                                                         */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_time_get                          ThreadX function               */
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
INT clock_gettime(clockid_t t, struct timespec * tspec)
{
    ULONG tx_time;
    
 if(t==CLOCK_REALTIME)
 {
     tx_time=tx_time_get();

     tspec->tv_sec = tx_time / CPU_TICKS_PER_SECOND;

     tspec->tv_nsec = (ULONG) ((tx_time - tspec->tv_sec*CPU_TICKS_PER_SECOND) * NANOSECONDS_IN_CPU_TICK);
   
     return(OK);
 }
 else return(EINVAL);
    
}
