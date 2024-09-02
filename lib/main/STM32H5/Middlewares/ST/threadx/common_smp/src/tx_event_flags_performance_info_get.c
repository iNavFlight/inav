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
#include "tx_event_flags.h"
#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_event_flags_performance_info_get                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves performance information from the specified  */
/*    event flag group.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    group_ptr                         Pointer to event flag group       */
/*    sets                              Destination for the number of     */
/*                                        event flag sets on this group   */
/*    gets                              Destination for the number of     */
/*                                        event flag gets on this group   */
/*    suspensions                       Destination for the number of     */
/*                                        event flag suspensions on this  */
/*                                        group                           */
/*    timeouts                          Destination for number of timeouts*/
/*                                        on this event flag group        */
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
UINT  _tx_event_flags_performance_info_get(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG *sets, ULONG *gets,
                    ULONG *suspensions, ULONG *timeouts)
{

#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA
UINT                    status;


    /* Determine if this is a legal request.  */
    if (group_ptr == TX_NULL)
    {

        /* Event flags group pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }

    /* Determine if the event group ID is invalid.  */
    else if (group_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID)
    {

        /* Event flags group pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_EVENT_FLAGS_PERFORMANCE_INFO_GET, group_ptr, 0, 0, 0, TX_TRACE_EVENT_FLAGS_EVENTS)

        /* Log this kernel call.  */
        TX_EL_EVENT_FLAGS_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */

        /* Retrieve the number of set operations on this event flag group.  */
        if (sets != TX_NULL)
        {

            *sets =  group_ptr -> tx_event_flags_group_performance_set_count;
        }

        /* Retrieve the number of get operations on this event flag group.  */
        if (gets != TX_NULL)
        {

            *gets =  group_ptr -> tx_event_flags_group__performance_get_count;
        }

        /* Retrieve the number of thread suspensions on this event flag group.  */
        if (suspensions != TX_NULL)
        {

            *suspensions =  group_ptr -> tx_event_flags_group___performance_suspension_count;
        }

        /* Retrieve the number of thread timeouts on this event flag group.  */
        if (timeouts != TX_NULL)
        {

            *timeouts =  group_ptr -> tx_event_flags_group____performance_timeout_count;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return successful completion.  */
        status =  TX_SUCCESS;
    }
#else
UINT                    status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (group_ptr != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (sets != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (gets != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (timeouts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
#endif

    /* Return completion status.  */
    return(status);
}

