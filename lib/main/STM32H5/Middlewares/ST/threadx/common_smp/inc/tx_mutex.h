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
/**   Mutex                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_mutex.h                                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX mutex management component,           */
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

#ifndef TX_MUTEX_H
#define TX_MUTEX_H


/* Define mutex control specific data definitions.  */

#define TX_MUTEX_ID                             ((ULONG) 0x4D555445)


/* Determine if in-line component initialization is supported by the
   caller.  */

#ifdef TX_INVOKE_INLINE_INITIALIZATION

/* Yes, in-line initialization is supported, remap the mutex initialization
   function.  */

#ifndef TX_MUTEX_ENABLE_PERFORMANCE_INFO
#define _tx_mutex_initialize() \
                    _tx_mutex_created_ptr =                             TX_NULL;      \
                    _tx_mutex_created_count =                           TX_EMPTY
#else
#define _tx_mutex_initialize() \
                    _tx_mutex_created_ptr =                             TX_NULL;      \
                    _tx_mutex_created_count =                           TX_EMPTY;     \
                    _tx_mutex_performance_put_count =                   ((ULONG) 0);  \
                    _tx_mutex_performance_get_count =                   ((ULONG) 0);  \
                    _tx_mutex_performance_suspension_count =            ((ULONG) 0);  \
                    _tx_mutex_performance_timeout_count =               ((ULONG) 0);  \
                    _tx_mutex_performance_priority_inversion_count =    ((ULONG) 0);  \
                    _tx_mutex_performance__priority_inheritance_count = ((ULONG) 0)
#endif
#define TX_MUTEX_INIT
#else

/* No in-line initialization is supported, use standard function call.  */
VOID        _tx_mutex_initialize(VOID);
#endif


/* Define internal mutex management function prototypes.  */

VOID        _tx_mutex_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence);
VOID        _tx_mutex_thread_release(TX_THREAD *thread_ptr);
VOID        _tx_mutex_priority_change(TX_THREAD *thread_ptr, UINT new_priority);


/* Mutex management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_MUTEX_INIT
#define MUTEX_DECLARE
#else
#define MUTEX_DECLARE extern
#endif


/* Define the head pointer of the created mutex list.  */

MUTEX_DECLARE  TX_MUTEX *   _tx_mutex_created_ptr;


/* Define the variable that holds the number of created mutexes. */

MUTEX_DECLARE  ULONG        _tx_mutex_created_count;


#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

/* Define the total number of mutex puts.  */

MUTEX_DECLARE  ULONG        _tx_mutex_performance_put_count;


/* Define the total number of mutex gets.  */

MUTEX_DECLARE  ULONG        _tx_mutex_performance_get_count;


/* Define the total number of mutex suspensions.  */

MUTEX_DECLARE  ULONG        _tx_mutex_performance_suspension_count;


/* Define the total number of mutex timeouts.  */

MUTEX_DECLARE  ULONG        _tx_mutex_performance_timeout_count;


/* Define the total number of priority inversions.  */

MUTEX_DECLARE  ULONG        _tx_mutex_performance_priority_inversion_count;


/* Define the total number of priority inheritance conditions.  */

MUTEX_DECLARE  ULONG        _tx_mutex_performance__priority_inheritance_count;


#endif


/* Define default post mutex delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_MUTEX_DELETE_PORT_COMPLETION
#define TX_MUTEX_DELETE_PORT_COMPLETION(m)
#endif


#endif
