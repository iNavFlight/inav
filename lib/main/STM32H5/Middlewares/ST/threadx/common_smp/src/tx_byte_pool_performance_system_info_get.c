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

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_byte_pool.h"
#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_byte_pool_performance_system_info_get           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves byte pool performance information.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    allocates                         Destination for total number of   */
/*                                        allocates                       */
/*    releases                          Destination for total number of   */
/*                                        releases                        */
/*    fragments_searched                Destination for total number of   */
/*                                        fragments searched during       */
/*                                        allocation                      */
/*    merges                            Destination for total number of   */
/*                                        adjacent free fragments merged  */
/*    splits                            Destination for total number of   */
/*                                        fragments split during          */
/*                                        allocation                      */
/*    suspensions                       Destination for total number of   */
/*                                        suspensions                     */
/*    timeouts                          Destination for total number of   */
/*                                        timeouts                        */
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
UINT  _tx_byte_pool_performance_system_info_get(ULONG *allocates, ULONG *releases,
                    ULONG *fragments_searched, ULONG *merges, ULONG *splits, ULONG *suspensions, ULONG *timeouts)
{

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BYTE_POOL__PERFORMANCE_SYSTEM_INFO_GET, 0, 0, 0, 0, TX_TRACE_BYTE_POOL_EVENTS)

    /* Log this kernel call.  */
    TX_EL_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the total number of byte pool allocates.  */
    if (allocates != TX_NULL)
    {

        *allocates =  _tx_byte_pool_performance_allocate_count;
    }

    /* Retrieve the total number of byte pool releases.  */
    if (releases != TX_NULL)
    {

        *releases =  _tx_byte_pool_performance_release_count;
    }

    /* Retrieve the total number of byte pool fragments searched.  */
    if (fragments_searched != TX_NULL)
    {

        *fragments_searched =  _tx_byte_pool_performance_search_count;
    }

    /* Retrieve the total number of byte pool fragments merged.  */
    if (merges != TX_NULL)
    {

        *merges =  _tx_byte_pool_performance_merge_count;
    }

    /* Retrieve the total number of byte pool fragment splits.  */
    if (splits != TX_NULL)
    {

        *splits =  _tx_byte_pool_performance_split_count;
    }

    /* Retrieve the total number of byte pool suspensions.  */
    if (suspensions != TX_NULL)
    {

        *suspensions =  _tx_byte_pool_performance_suspension_count;
    }

    /* Retrieve the total number of byte pool timeouts.  */
    if (timeouts != TX_NULL)
    {

        *timeouts =  _tx_byte_pool_performance_timeout_count;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);

#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (allocates != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (releases != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (fragments_searched != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (merges != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (splits != TX_NULL)
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

    /* Return completion status.  */
    return(status);
#endif
}

