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
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#ifdef TX_ENABLE_EVENT_TRACE


/* Define the pointer to the start of the trace buffer control structure.   */

TX_TRACE_HEADER                   *_tx_trace_header_ptr;


/* Define the pointer to the start of the trace object registry area in the trace buffer.  */

TX_TRACE_OBJECT_ENTRY             *_tx_trace_registry_start_ptr;


/* Define the pointer to the end of the trace object registry area in the trace buffer.  */

TX_TRACE_OBJECT_ENTRY             *_tx_trace_registry_end_ptr;


/* Define the pointer to the starting entry of the actual trace event area of the trace buffer.  */

TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_start_ptr;


/* Define the pointer to the ending entry of the actual trace event area of the trace buffer.  */

TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_end_ptr;


/* Define the pointer to the current entry of the actual trace event area of the trace buffer.  */

TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_current_ptr;


/* Define the trace event enable bits, where each bit represents a type of event that can be enabled
   or disabled dynamically by the application.  */

ULONG                            _tx_trace_event_enable_bits;


/* Define a counter that is used in environments that don't have a timer source. This counter
   is incremented on each use giving each event a unique timestamp.  */

ULONG                             _tx_trace_simulated_time;


/* Define the function pointer used to call the application when the trace buffer wraps. If NULL,
   the application has not registered a callback function.  */

VOID                             (*_tx_trace_full_notify_function)(VOID *buffer);


/* Define the total number of registry entries.  */

ULONG                             _tx_trace_total_registry_entries;


/* Define a counter that is used to track the number of available registry entries.  */

ULONG                             _tx_trace_available_registry_entries;


/* Define an index that represents the start of the registry search.  */

ULONG                             _tx_trace_registry_search_start;

#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_trace_initialize                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the trace component.                                                */
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
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _tx_trace_initialize(VOID)
{

#ifdef TX_ENABLE_EVENT_TRACE
#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize all the pointers to the trace buffer to NULL.  */
    _tx_trace_header_ptr =          TX_NULL;
    _tx_trace_registry_start_ptr =  TX_NULL;
    _tx_trace_registry_end_ptr =    TX_NULL;
    _tx_trace_buffer_start_ptr =    TX_NULL;
    _tx_trace_buffer_end_ptr =      TX_NULL;
    _tx_trace_buffer_current_ptr =  TX_NULL;
#endif
#endif
}

