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

#include "tx_api.h"             /* Threadx API */
#include "pthread.h"            /* Posix API */
#include "px_int.h"             /* Posix helper functions */
#include "time.h"
#include <limits.h>

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    posix_abs_time_to_rel_ticks                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts the absolute time specified in a POSIX       */
/*    timespec structure into the relative number of timer ticks until    */
/*    that time will occur.                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/
ULONG posix_abs_time_to_rel_ticks(struct timespec *abs_timeout)
{
   ULONG current_ticks, ticks_ns, ticks_sec, timeout_ticks;

   current_ticks = tx_time_get();
   /* convert ns to ticks (will lose any ns < 1 tick) */
   ticks_ns = abs_timeout->tv_nsec / NANOSECONDS_IN_CPU_TICK;
   /* 
    * if ns < 1 tick were lost, bump up to next tick so the delay is never
    * less than what was specified.
    */
   if (ticks_ns * NANOSECONDS_IN_CPU_TICK != abs_timeout->tv_nsec)
   {
      ++ticks_ns;
   }
   ticks_sec = (ULONG) (abs_timeout->tv_sec * CPU_TICKS_PER_SECOND);
   /* add in sec. ticks, subtract current ticks to get relative value. */
   timeout_ticks = ticks_sec + ticks_ns - current_ticks;
   /*
    * Unless a relative timeout of zero was specified, bump up 1 tick to
    * compensate for the fact that there is an unknown time between 0 and
    * < 1 tick until the next tick. We never want the delay to be less than
    * what was requested.
    */
   if (timeout_ticks != 0)
   {
      ++timeout_ticks;
   }
      /*
    * If the absolute time was in the past, then we need to set the
    * relative time to zero; otherwise, we get an essentially infinite timeout.
    */
   if (timeout_ticks > LONG_MAX)
   {
       timeout_ticks = 0;
   }

   return timeout_ticks;
}
