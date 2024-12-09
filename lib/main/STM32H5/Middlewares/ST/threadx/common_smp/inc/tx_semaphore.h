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
/**   Semaphore                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_semaphore.h                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX semaphore management component,       */
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

#ifndef TX_SEMAPHORE_H
#define TX_SEMAPHORE_H


/* Define semaphore control specific data definitions.  */

#define TX_SEMAPHORE_ID                         ((ULONG) 0x53454D41)


/* Determine if in-line component initialization is supported by the
   caller.  */
#ifdef TX_INVOKE_INLINE_INITIALIZATION
            /* Yes, in-line initialization is supported, remap the
               semaphore initialization function.  */
#ifndef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO
#define _tx_semaphore_initialize() \
                    _tx_semaphore_created_ptr =                   TX_NULL;     \
                    _tx_semaphore_created_count =                 TX_EMPTY
#else
#define _tx_semaphore_initialize() \
                    _tx_semaphore_created_ptr =                   TX_NULL;     \
                    _tx_semaphore_created_count =                 TX_EMPTY;    \
                    _tx_semaphore_performance_put_count =         ((ULONG) 0); \
                    _tx_semaphore_performance_get_count =         ((ULONG) 0); \
                    _tx_semaphore_performance_suspension_count =  ((ULONG) 0); \
                    _tx_semaphore_performance_timeout_count =     ((ULONG) 0)
#endif
#define TX_SEMAPHORE_INIT
#else
            /* No in-line initialization is supported, use standard
               function call.  */
VOID        _tx_semaphore_initialize(VOID);
#endif


/* Define internal semaphore management function prototypes.  */

VOID        _tx_semaphore_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence);


/* Semaphore management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_SEMAPHORE_INIT
#define SEMAPHORE_DECLARE
#else
#define SEMAPHORE_DECLARE extern
#endif


/* Define the head pointer of the created semaphore list.  */

SEMAPHORE_DECLARE  TX_SEMAPHORE *   _tx_semaphore_created_ptr;


/* Define the variable that holds the number of created semaphores. */

SEMAPHORE_DECLARE  ULONG            _tx_semaphore_created_count;


#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

/* Define the total number of semaphore puts.  */

SEMAPHORE_DECLARE  ULONG            _tx_semaphore_performance_put_count;


/* Define the total number of semaphore gets.  */

SEMAPHORE_DECLARE  ULONG            _tx_semaphore_performance_get_count;


/* Define the total number of semaphore suspensions.  */

SEMAPHORE_DECLARE  ULONG            _tx_semaphore_performance_suspension_count;


/* Define the total number of semaphore timeouts.  */

SEMAPHORE_DECLARE  ULONG            _tx_semaphore_performance_timeout_count;


#endif


/* Define default post semaphore delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_SEMAPHORE_DELETE_PORT_COMPLETION
#define TX_SEMAPHORE_DELETE_PORT_COMPLETION(s)
#endif


#endif

