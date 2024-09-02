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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"

/* Determine if in-line initialization is required.  */
#ifdef TX_INLINE_INITIALIZATION
#define TX_INVOKE_INLINE_INITIALIZATION
#endif

#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_semaphore.h"
#include "tx_queue.h"
#include "tx_event_flags.h"
#include "tx_mutex.h"
#include "tx_block_pool.h"
#include "tx_byte_pool.h"


/* Define the unused memory pointer.  The value of the first available
   memory address is placed in this variable in the low-level
   initialization function.  The content of this variable is passed
   to the application's system definition function.  */

VOID     *_tx_initialize_unused_memory;


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_initialize_high_level                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is responsible for initializing all of the other      */
/*    components in the ThreadX real-time kernel.                         */
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
/*    _tx_thread_initialize             Initialize the thread control     */
/*                                        component                       */
/*    _tx_timer_initialize              Initialize the timer control      */
/*                                        component                       */
/*    _tx_semaphore_initialize          Initialize the semaphore control  */
/*                                        component                       */
/*    _tx_queue_initialize              Initialize the queue control      */
/*                                        component                       */
/*    _tx_event_flags_initialize        Initialize the event flags control*/
/*                                        component                       */
/*    _tx_block_pool_initialize         Initialize the block pool control */
/*                                        component                       */
/*    _tx_byte_pool_initialize          Initialize the byte pool control  */
/*                                        component                       */
/*    _tx_mutex_initialize              Initialize the mutex control      */
/*                                        component                       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_kernel_enter       Kernel entry function             */
/*    _tx_initialize_kernel_setup       Early kernel setup function that  */
/*                                        is optionally called by         */
/*                                        compiler's startup code.        */
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
VOID    _tx_initialize_high_level(VOID)
{

    /* Initialize event tracing, if enabled.  */
    TX_TRACE_INITIALIZE

    /* Initialize the event log, if enabled.  */
    TX_EL_INITIALIZE

    /* Call the thread control initialization function.  */
    _tx_thread_initialize();

#ifndef TX_NO_TIMER

    /* Call the timer control initialization function.  */
    _tx_timer_initialize();
#endif

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Call the semaphore initialization function.  */
    _tx_semaphore_initialize();

    /* Call the queue initialization function.  */
    _tx_queue_initialize();

    /* Call the event flag initialization function.  */
    _tx_event_flags_initialize();

    /* Call the block pool initialization function.  */
    _tx_block_pool_initialize();

    /* Call the byte pool initialization function.  */
    _tx_byte_pool_initialize();

    /* Call the mutex initialization function.  */
    _tx_mutex_initialize();
#endif
}

