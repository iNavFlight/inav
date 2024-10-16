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
/*    _tx_event_flags_create                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a group of 32 event flags.  All the flags are */
/*    initially in a cleared state.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to event flags group      */
/*                                        control block                   */
/*    name_ptr                          Pointer to event flags name       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Successful completion status      */
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
UINT  _tx_event_flags_create(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR *name_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_EVENT_FLAGS_GROUP    *next_group;
TX_EVENT_FLAGS_GROUP    *previous_group;


    /* Initialize event flags control block to all zeros.  */
    TX_MEMSET(group_ptr, 0, (sizeof(TX_EVENT_FLAGS_GROUP)));

    /* Setup the basic event flags group fields.  */
    group_ptr -> tx_event_flags_group_name =             name_ptr;

    /* Disable interrupts to put the event flags group on the created list.  */
    TX_DISABLE

    /* Setup the event flags ID to make it valid.  */
    group_ptr -> tx_event_flags_group_id =  TX_EVENT_FLAGS_ID;

    /* Place the group on the list of created event flag groups.  First,
       check for an empty list.  */
    if (_tx_event_flags_created_count == TX_EMPTY)
    {

        /* The created event flags list is empty.  Add event flag group to empty list.  */
        _tx_event_flags_created_ptr =                         group_ptr;
        group_ptr -> tx_event_flags_group_created_next =      group_ptr;
        group_ptr -> tx_event_flags_group_created_previous =  group_ptr;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_group =      _tx_event_flags_created_ptr;
        previous_group =  next_group -> tx_event_flags_group_created_previous;

        /* Place the new event flag group in the list.  */
        next_group -> tx_event_flags_group_created_previous =  group_ptr;
        previous_group -> tx_event_flags_group_created_next =  group_ptr;

        /* Setup this group's created links.  */
        group_ptr -> tx_event_flags_group_created_previous =  previous_group;
        group_ptr -> tx_event_flags_group_created_next =      next_group;
    }

    /* Increment the number of created event flag groups.  */
    _tx_event_flags_created_count++;

    /* Optional event flag group create extended processing.  */
    TX_EVENT_FLAGS_GROUP_CREATE_EXTENSION(group_ptr)

    /* If trace is enabled, register this object.  */
    TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_EVENT_FLAGS, group_ptr, name_ptr, 0, 0)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_EVENT_FLAGS_CREATE, group_ptr, TX_POINTER_TO_ULONG_CONVERT(&next_group), 0, 0, TX_TRACE_EVENT_FLAGS_EVENTS)

    /* Log this kernel call.  */
    TX_EL_EVENT_FLAGS_CREATE_INSERT

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

