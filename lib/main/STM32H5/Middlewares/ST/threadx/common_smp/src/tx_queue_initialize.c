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
/**   Queue                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_queue.h"


#ifndef TX_INLINE_INITIALIZATION

/* Define the head pointer of the created queue list.  */

TX_QUEUE *   _tx_queue_created_ptr;


/* Define the variable that holds the number of created queues. */

ULONG        _tx_queue_created_count;


#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

/* Define the total number of messages sent.  */

ULONG        _tx_queue_performance_messages_sent_count;


/* Define the total number of messages received.  */

ULONG        _tx_queue_performance__messages_received_count;


/* Define the total number of queue empty suspensions.  */

ULONG        _tx_queue_performance_empty_suspension_count;


/* Define the total number of queue full suspensions.  */

ULONG        _tx_queue_performance_full_suspension_count;


/* Define the total number of queue full errors.  */

ULONG        _tx_queue_performance_full_error_count;


/* Define the total number of queue timeouts.  */

ULONG        _tx_queue_performance_timeout_count;

#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_queue_initialize                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the queue component.                                                */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_high_level         High level initialization         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            opt out of function when    */
/*                                            TX_INLINE_INITIALIZATION is */
/*                                            defined,                    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _tx_queue_initialize(VOID)
{

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the head pointer of the created queue list and the
       number of queues created.  */
    _tx_queue_created_ptr =        TX_NULL;
    _tx_queue_created_count =      TX_EMPTY;

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

    /* Initialize the queue performance counters.  */
    _tx_queue_performance_messages_sent_count =       ((ULONG) 0);
    _tx_queue_performance__messages_received_count =  ((ULONG) 0);
    _tx_queue_performance_empty_suspension_count =    ((ULONG) 0);
    _tx_queue_performance_full_suspension_count =     ((ULONG) 0);
    _tx_queue_performance_timeout_count =             ((ULONG) 0);
#endif
#endif
}
#endif
