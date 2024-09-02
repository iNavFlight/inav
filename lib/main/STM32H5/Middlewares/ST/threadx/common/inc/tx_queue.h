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


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_queue.h                                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX queue management component,           */
/*    including all data types and external references.  It is assumed    */
/*    that tx_api.h and tx_port.h have already been included.             */
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

#ifndef TX_QUEUE_H
#define TX_QUEUE_H


/* Define queue control specific data definitions.  */

#define TX_QUEUE_ID                             ((ULONG) 0x51554555)


/* Determine if in-line component initialization is supported by the
   caller.  */
#ifdef TX_INVOKE_INLINE_INITIALIZATION

/* Yes, in-line initialization is supported, remap the queue initialization
   function.  */

#ifndef TX_QUEUE_ENABLE_PERFORMANCE_INFO
#define _tx_queue_initialize() \
                    _tx_queue_created_ptr =                          TX_NULL;     \
                    _tx_queue_created_count =                        TX_EMPTY
#else
#define _tx_queue_initialize() \
                    _tx_queue_created_ptr =                          TX_NULL;     \
                    _tx_queue_created_count =                        TX_EMPTY;    \
                    _tx_queue_performance_messages_sent_count =      ((ULONG) 0); \
                    _tx_queue_performance__messages_received_count = ((ULONG) 0); \
                    _tx_queue_performance_empty_suspension_count =   ((ULONG) 0); \
                    _tx_queue_performance_full_suspension_count =    ((ULONG) 0); \
                    _tx_queue_performance_timeout_count =            ((ULONG) 0)
#endif
#define TX_QUEUE_INIT
#else

/* No in-line initialization is supported, use standard function call.  */
VOID        _tx_queue_initialize(VOID);
#endif


/* Define the message copy macro. Note that the source and destination
   pointers must be modified since they are used subsequently.  */

#ifndef TX_QUEUE_MESSAGE_COPY
#define TX_QUEUE_MESSAGE_COPY(s, d, z)          \
                    *(d)++ = *(s)++;            \
                    if ((z) > ((UINT) 1))       \
                    {                           \
                        while (--(z))           \
                        {                       \
                            *(d)++ =  *(s)++;   \
                         }                      \
                    }
#endif


/* Define internal queue management function prototypes.  */

VOID        _tx_queue_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence);


/* Queue management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_QUEUE_INIT
#define QUEUE_DECLARE
#else
#define QUEUE_DECLARE extern
#endif


/* Define the head pointer of the created queue list.  */

QUEUE_DECLARE  TX_QUEUE *   _tx_queue_created_ptr;


/* Define the variable that holds the number of created queues. */

QUEUE_DECLARE  ULONG        _tx_queue_created_count;


#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

/* Define the total number of messages sent.  */

QUEUE_DECLARE  ULONG        _tx_queue_performance_messages_sent_count;


/* Define the total number of messages received.  */

QUEUE_DECLARE  ULONG        _tx_queue_performance__messages_received_count;


/* Define the total number of queue empty suspensions.  */

QUEUE_DECLARE  ULONG        _tx_queue_performance_empty_suspension_count;


/* Define the total number of queue full suspensions.  */

QUEUE_DECLARE  ULONG        _tx_queue_performance_full_suspension_count;


/* Define the total number of queue full errors.  */

QUEUE_DECLARE  ULONG        _tx_queue_performance_full_error_count;


/* Define the total number of queue timeouts.  */

QUEUE_DECLARE  ULONG        _tx_queue_performance_timeout_count;


#endif


/* Define default post queue delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_QUEUE_DELETE_PORT_COMPLETION
#define TX_QUEUE_DELETE_PORT_COMPLETION(q)
#endif


#endif

