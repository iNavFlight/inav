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
/*    clock_settime                                        PORTABLE C     */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This subroutine sets the internal Threadx timer tick variable       */
/*    to the time specificed in the tspec variable after conversion       */
/*    if tspec->tv_sec is 0 the clock with be set to the value of         */
/*    tspec->tv_nsec                                                      */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */
/*    clockid_t, tspec                                                    */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     error code                                                         */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_time_set                          ThreadX function               */
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
INT clock_settime(clockid_t t, const struct timespec * tspec)
{
    ULONG tx_time;
    
 if(t==CLOCK_REALTIME)
 {
     tx_time=(ULONG)((tspec->tv_sec * CPU_TICKS_PER_SECOND) + (tspec->tv_nsec / NANOSECONDS_IN_CPU_TICK));
     tx_time_set( tx_time);
     return(OK);
 }
 else return(EINVAL);
}
