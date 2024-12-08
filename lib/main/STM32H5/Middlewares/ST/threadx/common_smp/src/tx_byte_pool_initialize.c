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
/**   Byte Pool                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_byte_pool.h"


#ifndef TX_INLINE_INITIALIZATION

/* Locate byte pool component data in this file.  */

/* Define the head pointer of the created byte pool list.  */

TX_BYTE_POOL *   _tx_byte_pool_created_ptr;


/* Define the variable that holds the number of created byte pools. */

ULONG            _tx_byte_pool_created_count;


#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

/* Define the total number of allocates.  */

ULONG            _tx_byte_pool_performance_allocate_count;


/* Define the total number of releases.  */

ULONG            _tx_byte_pool_performance_release_count;


/* Define the total number of adjacent memory fragment merges.  */

ULONG            _tx_byte_pool_performance_merge_count;


/* Define the total number of memory fragment splits.  */

ULONG            _tx_byte_pool_performance_split_count;


/* Define the total number of memory fragments searched during allocation.  */

ULONG            _tx_byte_pool_performance_search_count;


/* Define the total number of byte pool suspensions.  */

ULONG            _tx_byte_pool_performance_suspension_count;


/* Define the total number of byte pool timeouts.  */

ULONG            _tx_byte_pool_performance_timeout_count;

#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_byte_pool_initialize                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the byte pool component.                                            */
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
VOID  _tx_byte_pool_initialize(VOID)
{

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the head pointer of the created byte pools list and the
       number of byte pools created.  */
    _tx_byte_pool_created_ptr =        TX_NULL;
    _tx_byte_pool_created_count =      TX_EMPTY;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

    /* Initialize byte pool performance counters.  */
    _tx_byte_pool_performance_allocate_count =    ((ULONG) 0);
    _tx_byte_pool_performance_release_count =     ((ULONG) 0);
    _tx_byte_pool_performance_merge_count =       ((ULONG) 0);
    _tx_byte_pool_performance_split_count =       ((ULONG) 0);
    _tx_byte_pool_performance_search_count =      ((ULONG) 0);
    _tx_byte_pool_performance_suspension_count =  ((ULONG) 0);
    _tx_byte_pool_performance_timeout_count =     ((ULONG) 0);
#endif
#endif
}
#endif
