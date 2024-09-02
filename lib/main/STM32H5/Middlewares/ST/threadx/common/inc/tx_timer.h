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
/** ThreadX Component                                                     */
/**                                                                       */
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_timer.h                                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX timer management component, including */
/*    data types and external references.  It is assumed that tx_api.h    */
/*    and tx_port.h have already been included.                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef TX_TIMER_H
#define TX_TIMER_H


/* Define timer management specific data definitions.  */

#define TX_TIMER_ID                             ((ULONG) 0x4154494D)
#define TX_TIMER_ENTRIES                        ((ULONG) 32)


/* Define internal timer management function prototypes.  */

VOID        _tx_timer_expiration_process(VOID);
VOID        _tx_timer_initialize(VOID);
VOID        _tx_timer_system_activate(TX_TIMER_INTERNAL *timer_ptr);
VOID        _tx_timer_system_deactivate(TX_TIMER_INTERNAL *timer_ptr);
VOID        _tx_timer_thread_entry(ULONG timer_thread_input);


/* Timer management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_TIMER_INIT
#define TIMER_DECLARE
#else
#define TIMER_DECLARE extern
#endif


/* Define the system clock value that is continually incremented by the
   periodic timer interrupt processing.  */

TIMER_DECLARE volatile ULONG    _tx_timer_system_clock;


/* Define the current time slice value.  If non-zero, a time-slice is active.
   Otherwise, the time_slice is not active.  */

TIMER_DECLARE ULONG             _tx_timer_time_slice;


/* Define the time-slice expiration flag.  This is used to indicate that a time-slice
   has happened.  */

TIMER_DECLARE UINT              _tx_timer_expired_time_slice;


/* Define the thread and application timer entry list.  This list provides a direct access
   method for insertion of times less than TX_TIMER_ENTRIES.  */

TIMER_DECLARE TX_TIMER_INTERNAL *_tx_timer_list[TX_TIMER_ENTRIES];


/* Define the boundary pointers to the list.  These are setup to easily manage
   wrapping the list.  */

TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_list_start;
TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_list_end;


/* Define the current timer pointer in the list.  This pointer is moved sequentially
   through the timer list by the timer interrupt handler.  */

TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_current_ptr;


/* Define the timer expiration flag.  This is used to indicate that a timer
   has expired.  */

TIMER_DECLARE UINT              _tx_timer_expired;


/* Define the created timer list head pointer.  */

TIMER_DECLARE TX_TIMER          *_tx_timer_created_ptr;


/* Define the created timer count.  */

TIMER_DECLARE ULONG             _tx_timer_created_count;


/* Define the pointer to the timer that has expired and is being processed.  */

TIMER_DECLARE TX_TIMER_INTERNAL *_tx_timer_expired_timer_ptr;


#ifndef TX_TIMER_PROCESS_IN_ISR

/* Define the timer thread's control block.  */

TIMER_DECLARE TX_THREAD         _tx_timer_thread;


/* Define the variable that holds the timer thread's starting stack address.  */

TIMER_DECLARE VOID              *_tx_timer_stack_start;


/* Define the variable that holds the timer thread's stack size.  */

TIMER_DECLARE ULONG             _tx_timer_stack_size;


/* Define the variable that holds the timer thread's priority.  */

TIMER_DECLARE UINT              _tx_timer_priority;

/* Define the system timer thread's stack.   The default size is defined
   in tx_port.h.  */

TIMER_DECLARE ULONG             _tx_timer_thread_stack_area[(((UINT) TX_TIMER_THREAD_STACK_SIZE)+((sizeof(ULONG)) - ((UINT) 1)))/sizeof(ULONG)];

#else


/* Define the busy flag that will prevent nested timer ISR processing.  */

TIMER_DECLARE UINT              _tx_timer_processing_active;

#endif

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

/* Define the total number of timer activations.  */

TIMER_DECLARE  ULONG            _tx_timer_performance_activate_count;


/* Define the total number of timer reactivations.  */

TIMER_DECLARE  ULONG            _tx_timer_performance_reactivate_count;


/* Define the total number of timer deactivations.  */

TIMER_DECLARE  ULONG            _tx_timer_performance_deactivate_count;


/* Define the total number of timer expirations.  */

TIMER_DECLARE  ULONG            _tx_timer_performance_expiration_count;


/* Define the total number of timer expiration adjustments. These are required
   if the expiration time is greater than the size of the timer list. In such
   cases, the timer is placed at the end of the list and then reactivated
   as many times as necessary to finally achieve the resulting timeout. */

TIMER_DECLARE  ULONG            _tx_timer_performance__expiration_adjust_count;


#endif


/* Define default post timer delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_TIMER_DELETE_PORT_COMPLETION
#define TX_TIMER_DELETE_PORT_COMPLETION(t)
#endif


#endif
