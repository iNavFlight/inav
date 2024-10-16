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
/**   ThreadX Component                                                   */
/**                                                                       */
/**   POSIX Compliancy Wrapper (POSIX)                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  EKP DEFINITIONS                                        RELEASE        */
/*                                                                        */
/*    tx_px_time.h                                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the constants, structures, etc. needed to         */
/*    implement time related functionality for the Evacuation Kit         */
/*    for POSIX Users (POSIX)                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/

#ifndef _TX_PX_TIME_H
#define _TX_PX_TIME_H

#ifndef _TIME_T
#define _TIME_T
typedef  ULONG  time_t;
#endif

typedef INT clockid_t;

struct timespec 
{
    time_t    tv_sec;                /* time in terms of seconds */
    ULONG     tv_nsec;               /* remaining time in terms of nano seconds*/
} ;

struct itimerspec
{
    struct timespec  it_interval ;         /* Timer period. */
    struct timespec  it_value;             /* Timer expiration. */ 
};

#define CLOCK_REALTIME  1
#define TIMER_ABSTIME   1
#define CLOCK_MONOTONIC 2

INT clock_settime(clockid_t, const struct timespec *);
INT clock_gettime(clockid_t, struct timespec *);
INT clock_getres(clockid_t, struct timespec *);


INT nanosleep(struct timespec *req, struct timespec *rem);
UINT sleep(ULONG seconds);

#endif


