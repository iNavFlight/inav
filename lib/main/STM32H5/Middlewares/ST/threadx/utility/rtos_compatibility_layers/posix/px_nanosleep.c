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
/**************************************************************************/
/**************************************************************************/

/* Include necessary system files.  */

#include "tx_api.h"     /* Threadx API */
#include "pthread.h"    /* Posix API */
#include "px_int.h"     /* Posix helper functions */
#include <limits.h>

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*  nanosleep                                              PORTABLE C     */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function shall cause the current thread to be suspended from   */
/*    execution until the time interval specified by the req argument has */
/*    elapsed.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*   req                          The number of real-time (as opposed     */
/*                                to CPU-time) seconds and nanoseconds to */
/*                                suspend the calling thread.             */
/*   rem                          Points to a structure to receive the    */
/*                                remaining time if the function is       */
/*                                interrupted by a signal. This pointer   */
/*                                may be NULL.                            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*   zero                         If the function returns because the     */
/*                                requested time has elapsed.             */
/*                                                                        */
/*   -1                           If this functions fails if req argument */
/*                                specified a nanosecond value greater    */
/*                                than or equal to 1 billion.             */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_sleep             ThreadX thread sleep service            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Fix bounds check,             */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
INT nanosleep(struct timespec *req, struct timespec *rem)
{

ULONG    timer_ticks;

    /* Check for valid inputs - the nanosecond value must be less than 1 billion
       and not roll over when converting to ThreadX timer ticks. */
    if ( (!req) || (req->tv_nsec > 999999999) || ((timer_ticks = (req->tv_sec * CPU_TICKS_PER_SECOND + req->tv_nsec/NANOSECONDS_IN_CPU_TICK)) < req->tv_sec) )
    {
        posix_errno = EINVAL;
        posix_set_pthread_errno(EINVAL);
        return(ERROR);
    }

    /* Add padding of 1 so that the thread will sleep no less than the specified time, 
       except in the case that timer_ticks is ULONG_MAX */
    if(timer_ticks != ULONG_MAX)
    {
        timer_ticks = timer_ticks + 1;
    }

    /* Now call ThreadX thread sleep service. */
    tx_thread_sleep(timer_ticks);
    
    /* Sleep completed. */
    if (rem)
    {
        rem->tv_nsec =  0;
        rem->tv_sec = 0;
    }
    return(OK);
}
