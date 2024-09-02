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
/**   Queue                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_queue.h"
#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_queue_performance_info_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves performance information from the specified  */
/*    queue.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    queue_ptr                         Pointer to queue control block    */
/*    messages_sent                     Destination for messages sent     */
/*    messages_received                 Destination for messages received */
/*    empty_suspensions                 Destination for number of empty   */
/*                                        queue suspensions               */
/*    full_suspensions                  Destination for number of full    */
/*                                        queue suspensions               */
/*    full_errors                       Destination for queue full errors */
/*                                        returned - no suspension        */
/*    timeouts                          Destination for number of timeouts*/
/*                                        on this queue                   */
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
UINT  _tx_queue_performance_info_get(TX_QUEUE *queue_ptr, ULONG *messages_sent, ULONG *messages_received,
                    ULONG *empty_suspensions, ULONG *full_suspensions, ULONG *full_errors, ULONG *timeouts)
{

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA
UINT                    status;


    /* Determine if this is a legal request.  */
    if (queue_ptr == TX_NULL)
    {

        /* Queue pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }

    /* Determine if the queue ID is invalid.  */
    else if (queue_ptr -> tx_queue_id != TX_QUEUE_ID)
    {

        /* Queue pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_QUEUE_PERFORMANCE_INFO_GET, queue_ptr, 0, 0, 0, TX_TRACE_QUEUE_EVENTS)

        /* Log this kernel call.  */
        TX_EL_QUEUE_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */

        /* Retrieve the number of messages sent to this queue.  */
        if (messages_sent != TX_NULL)
        {

            *messages_sent =  queue_ptr -> tx_queue_performance_messages_sent_count;
        }

        /* Retrieve the number of messages received from this queue.  */
        if (messages_received != TX_NULL)
        {

            *messages_received =  queue_ptr -> tx_queue_performance_messages_received_count;
        }

        /* Retrieve the number of empty queue suspensions on this queue.  */
        if (empty_suspensions != TX_NULL)
        {

            *empty_suspensions =  queue_ptr -> tx_queue_performance_empty_suspension_count;
        }

        /* Retrieve the number of full queue suspensions on this queue.  */
        if (full_suspensions != TX_NULL)
        {

            *full_suspensions =  queue_ptr -> tx_queue_performance_full_suspension_count;
        }

        /* Retrieve the number of full errors (no suspension!) on this queue.  */
        if (full_errors != TX_NULL)
        {

            *full_errors =  queue_ptr -> tx_queue_performance_full_error_count;
        }

        /* Retrieve the number of timeouts on this queue.  */
        if (timeouts != TX_NULL)
        {

            *timeouts =  queue_ptr -> tx_queue_performance_timeout_count;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return completion status.  */
        status =  TX_SUCCESS;
    }
#else
UINT                    status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (queue_ptr != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (messages_sent != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (messages_received != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (empty_suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (full_suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (full_errors != TX_NULL)
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

