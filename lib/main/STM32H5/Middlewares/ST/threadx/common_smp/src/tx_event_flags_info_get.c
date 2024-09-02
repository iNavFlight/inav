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

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_event_flags.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_event_flags_info_get                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves information from the specified event flag   */
/*    group.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to event flag group       */
/*    name                              Destination for the event flag    */
/*                                        group name                      */
/*    current_flags                     Current event flags               */
/*    first_suspended                   Destination for pointer of first  */
/*                                        thread suspended on event flags */
/*    suspended_count                   Destination for suspended count   */
/*    next_group                        Destination for pointer to next   */
/*                                        event flag group on the created */
/*                                        list                            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _tx_event_flags_info_get(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR **name, ULONG *current_flags,
                    TX_THREAD **first_suspended, ULONG *suspended_count,
                    TX_EVENT_FLAGS_GROUP **next_group)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_EVENT_FLAGS_INFO_GET, group_ptr, 0, 0, 0, TX_TRACE_EVENT_FLAGS_EVENTS)

    /* Log this kernel call.  */
    TX_EL_EVENT_FLAGS_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the name of the event flag group.  */
    if (name != TX_NULL)
    {

        *name =  group_ptr -> tx_event_flags_group_name;
    }

    /* Retrieve the current event flags in the event flag group.  */
    if (current_flags != TX_NULL)
    {

        /* Pickup the current flags and apply delayed clearing.  */
        *current_flags =  group_ptr -> tx_event_flags_group_current &
                                                        ~group_ptr -> tx_event_flags_group_delayed_clear;
    }

    /* Retrieve the first thread suspended on this event flag group.  */
    if (first_suspended != TX_NULL)
    {

        *first_suspended =  group_ptr -> tx_event_flags_group_suspension_list;
    }

    /* Retrieve the number of threads suspended on this event flag group.  */
    if (suspended_count != TX_NULL)
    {

        *suspended_count =  (ULONG) group_ptr -> tx_event_flags_group_suspended_count;
    }

    /* Retrieve the pointer to the next event flag group created.  */
    if (next_group != TX_NULL)
    {

        *next_group =  group_ptr -> tx_event_flags_group_created_next;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

