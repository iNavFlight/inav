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

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_mutex.h"


#ifndef TX_INLINE_INITIALIZATION

/* Locate mutex component data in this file.  */

/* Define the head pointer of the created mutex list.  */

TX_MUTEX *   _tx_mutex_created_ptr;


/* Define the variable that holds the number of created mutexes. */

ULONG        _tx_mutex_created_count;


#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

/* Define the total number of mutex puts.  */

ULONG        _tx_mutex_performance_put_count;


/* Define the total number of mutex gets.  */

ULONG        _tx_mutex_performance_get_count;


/* Define the total number of mutex suspensions.  */

ULONG        _tx_mutex_performance_suspension_count;


/* Define the total number of mutex timeouts.  */

ULONG        _tx_mutex_performance_timeout_count;


/* Define the total number of priority inversions.  */

ULONG        _tx_mutex_performance_priority_inversion_count;


/* Define the total number of priority inheritance conditions.  */

ULONG        _tx_mutex_performance__priority_inheritance_count;

#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_mutex_initialize                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the mutex component.                                                */
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
VOID  _tx_mutex_initialize(VOID)
{

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the head pointer of the created mutexes list and the
       number of mutexes created.  */
    _tx_mutex_created_ptr =        TX_NULL;
    _tx_mutex_created_count =      TX_EMPTY;

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

    /* Initialize the mutex performance counters.  */
    _tx_mutex_performance_put_count =                   ((ULONG) 0);
    _tx_mutex_performance_get_count =                   ((ULONG) 0);
    _tx_mutex_performance_suspension_count =            ((ULONG) 0);
    _tx_mutex_performance_timeout_count =               ((ULONG) 0);
    _tx_mutex_performance_priority_inversion_count =    ((ULONG) 0);
    _tx_mutex_performance__priority_inheritance_count =  ((ULONG) 0);
#endif
#endif
}
#endif
