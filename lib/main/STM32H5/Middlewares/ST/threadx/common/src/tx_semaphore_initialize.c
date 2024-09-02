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

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_semaphore.h"


#ifndef TX_INLINE_INITIALIZATION

/* Locate semaphore component data in this file.  */

/* Define the head pointer of the created semaphore list.  */

TX_SEMAPHORE *   _tx_semaphore_created_ptr;


/* Define the variable that holds the number of created semaphores. */

ULONG            _tx_semaphore_created_count;


#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

/* Define the total number of semaphore puts.  */

ULONG            _tx_semaphore_performance_put_count;


/* Define the total number of semaphore gets.  */

ULONG            _tx_semaphore_performance_get_count;


/* Define the total number of semaphore suspensions.  */

ULONG            _tx_semaphore_performance_suspension_count;


/* Define the total number of semaphore timeouts.  */

ULONG            _tx_semaphore_performance_timeout_count;

#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_semaphore_initialize                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the semaphore component.                                            */
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
VOID  _tx_semaphore_initialize(VOID)
{

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the head pointer of the created semaphores list and the
       number of semaphores created.  */
    _tx_semaphore_created_ptr =        TX_NULL;
    _tx_semaphore_created_count =      TX_EMPTY;

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

    /* Initialize semaphore performance counters.  */
    _tx_semaphore_performance_put_count =         ((ULONG) 0);
    _tx_semaphore_performance_get_count =         ((ULONG) 0);
    _tx_semaphore_performance_suspension_count =  ((ULONG) 0);
    _tx_semaphore_performance_timeout_count =     ((ULONG) 0);
#endif
#endif
}
#endif
