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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_trace_user_event_insert                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function inserts a user-defined event into the trace buffer.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    event_id                              User Event ID                 */
/*    info_field_1                          First information field       */
/*    info_field_2                          First information field       */
/*    info_field_3                          First information field       */
/*    info_field_4                          First information field       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
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
UINT  _tx_trace_user_event_insert(ULONG event_id, ULONG info_field_1, ULONG info_field_2, ULONG info_field_3, ULONG info_field_4)
{

#ifdef TX_ENABLE_EVENT_TRACE

TX_INTERRUPT_SAVE_AREA

UINT            status;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Determine if trace is disabled.  */
    if (_tx_trace_buffer_current_ptr == TX_NULL)
    {

        /* Yes, trace is already disabled.  */
        status =  TX_NOT_DONE;
    }
    else
    {

        /* Insert this event into the trace buffer.  */
#ifdef TX_MISRA_ENABLE
        TX_TRACE_IN_LINE_INSERT(event_id, TX_ULONG_TO_POINTER_CONVERT(info_field_1), info_field_2, info_field_3, info_field_4, ((ULONG) TX_TRACE_USER_EVENTS))
#else
        TX_TRACE_IN_LINE_INSERT(event_id, info_field_1, info_field_2, info_field_3, info_field_4, TX_TRACE_USER_EVENTS)
#endif

        /* Return successful status.  */
        status =  TX_SUCCESS;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(status);

#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (event_id != ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (info_field_1 != ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (info_field_2 != ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (info_field_3 != ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (info_field_4 != ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }

    /* Return completion status.  */
    return(status);
#endif
}

