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

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"


/* Check for the TX_NO_TIMER option. When defined, do not define all of the
   timer component global variables.  */

#ifndef TX_NO_TIMER


/* Define the system clock value that is continually incremented by the
   periodic timer interrupt processing.  */

volatile ULONG    _tx_timer_system_clock;


/* Define count to detect when timer interrupt is active.  */

ULONG             _tx_timer_interrupt_active;


/* Define the time-slice expiration flag.  This is used to indicate that a time-slice
   has happened.  */

UINT              _tx_timer_expired_time_slice;


/* Define the thread and application timer entry list.  This list provides a direct access
   method for insertion of times less than TX_TIMER_ENTRIES.  */

TX_TIMER_INTERNAL *_tx_timer_list[TX_TIMER_ENTRIES];


/* Define the boundary pointers to the list.  These are setup to easily manage
   wrapping the list.  */

TX_TIMER_INTERNAL **_tx_timer_list_start;
TX_TIMER_INTERNAL **_tx_timer_list_end;


/* Define the current timer pointer in the list.  This pointer is moved sequentially
   through the timer list by the timer interrupt handler.  */

TX_TIMER_INTERNAL **_tx_timer_current_ptr;


/* Define the timer expiration flag.  This is used to indicate that a timer
   has expired.  */

UINT              _tx_timer_expired;


/* Define the created timer list head pointer.  */

TX_TIMER          *_tx_timer_created_ptr;


/* Define the created timer count.  */

ULONG             _tx_timer_created_count;


/* Define the pointer to the timer that has expired and is being processed.  */

TX_TIMER_INTERNAL *_tx_timer_expired_timer_ptr;


#ifndef TX_TIMER_PROCESS_IN_ISR

/* Define the timer thread's control block.  */

TX_THREAD         _tx_timer_thread;


/* Define the variable that holds the timer thread's starting stack address.  */

VOID              *_tx_timer_stack_start;


/* Define the variable that holds the timer thread's stack size.  */

ULONG             _tx_timer_stack_size;


/* Define the variable that holds the timer thread's priority.  */

UINT              _tx_timer_priority;

/* Define the system timer thread's stack.   The default size is defined
   in tx_port.h.  */

ULONG             _tx_timer_thread_stack_area[(((UINT) TX_TIMER_THREAD_STACK_SIZE)+((sizeof(ULONG)) - ((UINT) 1)))/sizeof(ULONG)];

#else


/* Define the busy flag that will prevent nested timer ISR processing.  */

UINT              _tx_timer_processing_active;

#endif

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

/* Define the total number of timer activations.  */

ULONG            _tx_timer_performance_activate_count;


/* Define the total number of timer reactivations.  */

ULONG            _tx_timer_performance_reactivate_count;


/* Define the total number of timer deactivations.  */

ULONG            _tx_timer_performance_deactivate_count;


/* Define the total number of timer expirations.  */

ULONG            _tx_timer_performance_expiration_count;


/* Define the total number of timer expiration adjustments. These are required
   if the expiration time is greater than the size of the timer list. In such
   cases, the timer is placed at the end of the list and then reactivated
   as many times as necessary to finally achieve the resulting timeout. */

ULONG            _tx_timer_performance__expiration_adjust_count;

#endif
#endif


/* Define the current time slice value.  If non-zero, a time-slice is active.
   Otherwise, the time_slice is not active.  There is one of these entries
   per core.  */

ULONG             _tx_timer_time_slice[TX_THREAD_SMP_MAX_CORES];


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_initialize                               PORTABLE SMP     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the clock control component.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_create                 Create the system timer thread    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_high_level         High level initialization         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     William E. Lamie         Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _tx_timer_initialize(VOID)
{
#ifndef TX_NO_TIMER
#ifndef TX_TIMER_PROCESS_IN_ISR
UINT    status;
#endif

#ifndef TX_DISABLE_REDUNDANT_CLEARING


    /* Initialize the system clock to 0.  */
    _tx_timer_system_clock =      ((ULONG) 0);

    /* Initialize timer interrupt active count.  */
    _tx_timer_interrupt_active =  ((ULONG) 0);

    /* Initialize the time-slice array to 0 to make sure everything is disabled.  */
    TX_MEMSET(&_tx_timer_time_slice[0], 0, (sizeof(_tx_timer_time_slice)));

    /* Clear the expired flags.  */
    _tx_timer_expired_time_slice =  TX_FALSE;
    _tx_timer_expired =             TX_FALSE;

    /* Set the currently expired timer being processed pointer to NULL.  */
    _tx_timer_expired_timer_ptr =  TX_NULL;

    /* Initialize the thread and application timer management control structures.  */

    /* First, initialize the timer list.  */
    TX_MEMSET(&_tx_timer_list[0], 0, sizeof(_tx_timer_list));
#endif

    /* Initialize all of the list pointers.  */
    _tx_timer_list_start =   &_tx_timer_list[0];
    _tx_timer_current_ptr =  &_tx_timer_list[0];

    /* Set the timer list end pointer to one past the actual timer list.  This is done
       to make the timer interrupt handling in assembly language a little easier.  */
    _tx_timer_list_end =     &_tx_timer_list[TX_TIMER_ENTRIES-((ULONG) 1)];
    _tx_timer_list_end =     TX_TIMER_POINTER_ADD(_tx_timer_list_end, ((ULONG) 1));

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Setup the variables associated with the system timer thread's stack and
       priority.  */
    _tx_timer_stack_start =  (VOID *) &_tx_timer_thread_stack_area[0];
    _tx_timer_stack_size =   ((ULONG) TX_TIMER_THREAD_STACK_SIZE);
    _tx_timer_priority =     ((UINT) TX_TIMER_THREAD_PRIORITY);

    /* Create the system timer thread.  This thread processes all of the timer
       expirations and reschedules.  Its stack and priority are defined in the
       low-level initialization component.  */
    do
    {

        /* Create the system timer thread.  */
        status =  _tx_thread_create(&_tx_timer_thread,
                                    TX_CONST_CHAR_TO_CHAR_POINTER_CONVERT("System Timer Thread"),
                                    _tx_timer_thread_entry,
                                    (ULONG) TX_TIMER_ID,  _tx_timer_stack_start, _tx_timer_stack_size,
                                    _tx_timer_priority, _tx_timer_priority, TX_NO_TIME_SLICE, TX_DONT_START);

        /* Define timer initialize extension.  */
        TX_TIMER_INITIALIZE_EXTENSION(status)

    } while (status != TX_SUCCESS);

#else

    /* Clear the timer interrupt processing active flag.  */
    _tx_timer_processing_active =  TX_FALSE;
#endif

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the head pointer of the created application timer list.  */
    _tx_timer_created_ptr =  TX_NULL;

    /* Set the created count to zero.  */
    _tx_timer_created_count =  TX_EMPTY;

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

    /* Initialize timer performance counters.  */
    _tx_timer_performance_activate_count =           ((ULONG) 0);
    _tx_timer_performance_reactivate_count =         ((ULONG) 0);
    _tx_timer_performance_deactivate_count =         ((ULONG) 0);
    _tx_timer_performance_expiration_count =         ((ULONG) 0);
    _tx_timer_performance__expiration_adjust_count =  ((ULONG) 0);
#endif
#endif
#endif
}

