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
/**   Event Flags                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_event_flags.h                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX event flags management component,     */
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

#ifndef TX_EVENT_FLAGS_H
#define TX_EVENT_FLAGS_H


/* Define event flags control specific data definitions.  */

#define TX_EVENT_FLAGS_ID                       ((ULONG) 0x4456444E)
#define TX_EVENT_FLAGS_AND_MASK                 ((UINT) 0x2)
#define TX_EVENT_FLAGS_CLEAR_MASK               ((UINT) 0x1)


/* Determine if in-line component initialization is supported by the
   caller.  */
#ifdef TX_INVOKE_INLINE_INITIALIZATION

/* Yes, in-line initialization is supported, remap the event flag initialization
   function.  */

#ifndef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO
#define _tx_event_flags_initialize() \
                    _tx_event_flags_created_ptr =                   TX_NULL;     \
                    _tx_event_flags_created_count =                 TX_EMPTY
#else
#define _tx_event_flags_initialize() \
                    _tx_event_flags_created_ptr =                   TX_NULL;     \
                    _tx_event_flags_created_count =                 TX_EMPTY;    \
                    _tx_event_flags_performance_set_count =         ((ULONG) 0); \
                    _tx_event_flags_performance_get_count =         ((ULONG) 0); \
                    _tx_event_flags_performance_suspension_count =  ((ULONG) 0); \
                    _tx_event_flags_performance_timeout_count =     ((ULONG) 0)
#endif
#define TX_EVENT_FLAGS_INIT
#else

/* No in-line initialization is supported, use standard function call.  */
VOID        _tx_event_flags_initialize(VOID);
#endif


/* Define internal event flags management function prototypes.  */

VOID        _tx_event_flags_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence);


/* Event flags management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_EVENT_FLAGS_INIT
#define EVENT_FLAGS_DECLARE
#else
#define EVENT_FLAGS_DECLARE extern
#endif


/* Define the head pointer of the created event flags list.  */

EVENT_FLAGS_DECLARE  TX_EVENT_FLAGS_GROUP * _tx_event_flags_created_ptr;


/* Define the variable that holds the number of created event flag groups. */

EVENT_FLAGS_DECLARE  ULONG                  _tx_event_flags_created_count;


#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO

/* Define the total number of event flag sets.  */

EVENT_FLAGS_DECLARE  ULONG                  _tx_event_flags_performance_set_count;


/* Define the total number of event flag gets.  */

EVENT_FLAGS_DECLARE  ULONG                  _tx_event_flags_performance_get_count;


/* Define the total number of event flag suspensions.  */

EVENT_FLAGS_DECLARE  ULONG                  _tx_event_flags_performance_suspension_count;


/* Define the total number of event flag timeouts.  */

EVENT_FLAGS_DECLARE  ULONG                  _tx_event_flags_performance_timeout_count;


#endif

/* Define default post event flag group delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_EVENT_FLAGS_GROUP_DELETE_PORT_COMPLETION
#define TX_EVENT_FLAGS_GROUP_DELETE_PORT_COMPLETION(g)
#endif


#endif

