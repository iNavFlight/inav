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
/*    signal.h                                            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the constants, structures, etc.needed to          */
/*    implement signals functionality for POSIX Users (POSIX)             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      William E. Lamie        Initial Version 6.1.7         */
/*  10-31-2022      Scott Larson            Update pthread_kill argument  */
/*                                            type,                       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef _SIGNAL_H
#define _SIGNAL_H


/* The POSIX wrapper for ThreadX supports a maximum of 32 signals, from 0 
   through 31, inclusive. In this implemenation, signals are NOT queued.  */


/* Define constants for the signal implementation.  */

#define MAX_SIGNALS 32
#define SIGRTMIN    0
#define SIGRTMAX    31
#define SIG_DFL     (void *) 0
#define SIG_IGN     (void *) 0
#define SIG_BLOCK   1
#define SIG_SETMASK 2
#define SIG_UNBLOCK 3


/* Define the typdefs for this signal handling implementation.  */


/* Define the type that holds the desired signals.  */

typedef struct sigset_t_struct
{
    unsigned long   signal_set;
} sigset_t;


/* Define the type that keeps track of information in the POSIX thread control block. POSIX threads are used to simulate the behavior
   of POSIX signals in this implemenation.  */

struct pthread_control_block;
struct pthread_t_variable;

typedef struct signal_info_struct
{

    UINT                            signal_handler;                         /* This is a flag. If TRUE, this thread is being used as a signal handler. If FALSE, it is a regular thread. */
    UINT                            signal_nesting_depth;                   /* A positive value indicates the level of nested signal handling the POSIX thread is currently processing.  */
    sigset_t                        signal_pending;                         /* Bit map of signals pending.                                                                               */ 
    sigset_t                        signal_mask;                            /* Signal mask, bit blocks the signal until cleared.                                                         */ 
    UINT                            saved_thread_state;                     /* Saved ThreadX state of the POSIX thread, at the time of the first signal.                                 */
    struct  pthread_control_block  *base_thread_ptr;                        /* Pointer to the thread associated with the signal.                                                         */ 
    struct  pthread_control_block  *top_signal_thread;                      /* Pointer to the top (most recent) signal thread.                                                           */ 
    struct  pthread_control_block  *next_signal_thread;                     /* Pointer to the next most recent signal thread.                                                            */ 
    void                            (*signal_func[MAX_SIGNALS])(int);       /* Array of signal handlers for this thread.                                                                 */ 
    TX_EVENT_FLAGS_GROUP            signal_event_flags;                     /* ThreadX event flag group used for sigwait                                                                 */ 
    
} signal_info;


/* Define public POSIX routines.  */

int   signal(int signo, void (*func)(int));
int   pthread_kill(ALIGN_TYPE thread, int sig);
int   sigwait(const sigset_t *set, int *sig);
int   sigemptyset(sigset_t *set);
int   sigaddset(sigset_t *set, int signo);
int   sigfillset(sigset_t *set);
int   sigdelset(sigset_t *set, int signo);
int   pthread_sigmask(int how, const sigset_t *newmask, sigset_t *oldmask);


/* Define internal POSIX routines.  */

void  internal_signal_dispatch(ULONG id);


#endif
