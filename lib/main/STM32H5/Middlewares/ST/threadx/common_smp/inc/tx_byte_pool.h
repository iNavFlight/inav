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
/**   Byte Memory                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_byte_pool.h                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX byte memory management component,     */
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

#ifndef TX_BYTE_POOL_H
#define TX_BYTE_POOL_H


/* Define byte memory control specific data definitions.  */

#define TX_BYTE_POOL_ID                         ((ULONG) 0x42595445)

#ifndef TX_BYTE_BLOCK_FREE
#define TX_BYTE_BLOCK_FREE                      ((ULONG) 0xFFFFEEEEUL)
#endif

#ifndef TX_BYTE_BLOCK_MIN
#define TX_BYTE_BLOCK_MIN                       ((ULONG) 20)
#endif

#ifndef TX_BYTE_POOL_MIN
#define TX_BYTE_POOL_MIN                        ((ULONG) 100)
#endif


/* Determine if in-line component initialization is supported by the
   caller.  */

#ifdef TX_INVOKE_INLINE_INITIALIZATION

/* Yes, in-line initialization is supported, remap the byte memory pool
   initialization function.  */

#ifndef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO
#define _tx_byte_pool_initialize() \
                    _tx_byte_pool_created_ptr =                   TX_NULL;      \
                    _tx_byte_pool_created_count =                 TX_EMPTY
#else
#define _tx_byte_pool_initialize() \
                    _tx_byte_pool_created_ptr =                   TX_NULL;      \
                    _tx_byte_pool_created_count =                 TX_EMPTY;     \
                    _tx_byte_pool_performance_allocate_count =    ((ULONG) 0);  \
                    _tx_byte_pool_performance_release_count =     ((ULONG) 0);  \
                    _tx_byte_pool_performance_merge_count =       ((ULONG) 0);  \
                    _tx_byte_pool_performance_split_count =       ((ULONG) 0);  \
                    _tx_byte_pool_performance_search_count =      ((ULONG) 0);  \
                    _tx_byte_pool_performance_suspension_count =  ((ULONG) 0);  \
                    _tx_byte_pool_performance_timeout_count =     ((ULONG) 0)
#endif
#define TX_BYTE_POOL_INIT
#else

/* No in-line initialization is supported, use standard function call.  */
VOID        _tx_byte_pool_initialize(VOID);
#endif


/* Define internal byte memory pool management function prototypes.  */

UCHAR       *_tx_byte_pool_search(TX_BYTE_POOL *pool_ptr, ULONG memory_size);
VOID        _tx_byte_pool_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence);


/* Byte pool management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef TX_BYTE_POOL_INIT
#define BYTE_POOL_DECLARE
#else
#define BYTE_POOL_DECLARE extern
#endif


/* Define the head pointer of the created byte pool list.  */

BYTE_POOL_DECLARE  TX_BYTE_POOL *   _tx_byte_pool_created_ptr;


/* Define the variable that holds the number of created byte pools. */

BYTE_POOL_DECLARE  ULONG            _tx_byte_pool_created_count;


#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

/* Define the total number of allocates.  */

BYTE_POOL_DECLARE  ULONG            _tx_byte_pool_performance_allocate_count;


/* Define the total number of releases.  */

BYTE_POOL_DECLARE  ULONG            _tx_byte_pool_performance_release_count;


/* Define the total number of adjacent memory fragment merges.  */

BYTE_POOL_DECLARE  ULONG            _tx_byte_pool_performance_merge_count;


/* Define the total number of memory fragment splits.  */

BYTE_POOL_DECLARE  ULONG            _tx_byte_pool_performance_split_count;


/* Define the total number of memory fragments searched during allocation.  */

BYTE_POOL_DECLARE  ULONG            _tx_byte_pool_performance_search_count;


/* Define the total number of byte pool suspensions.  */

BYTE_POOL_DECLARE  ULONG            _tx_byte_pool_performance_suspension_count;


/* Define the total number of byte pool timeouts.  */

BYTE_POOL_DECLARE  ULONG            _tx_byte_pool_performance_timeout_count;


#endif


/* Define default post byte pool delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_BYTE_POOL_DELETE_PORT_COMPLETION
#define TX_BYTE_POOL_DELETE_PORT_COMPLETION(p)
#endif


#endif
