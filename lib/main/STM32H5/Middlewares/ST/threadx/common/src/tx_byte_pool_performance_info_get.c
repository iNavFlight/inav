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
/*    _tx_byte_pool_performance_info_get                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves performance information from the specified  */
/*    byte pool.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to byte pool control block*/
/*    allocates                         Destination for number of         */
/*                                        allocates on this pool          */
/*    releases                          Destination for number of         */
/*                                        releases on this pool           */
/*    fragments_searched                Destination for number of         */
/*                                        fragments searched during       */
/*                                        allocation                      */
/*    merges                            Destination for number of adjacent*/
/*                                        free fragments merged           */
/*    splits                            Destination for number of         */
/*                                        fragments split during          */
/*                                        allocation                      */
/*    suspensions                       Destination for number of         */
/*                                        suspensions on this pool        */
/*    timeouts                          Destination for number of timeouts*/
/*                                        on this byte pool               */
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
UINT  _tx_byte_pool_performance_info_get(TX_BYTE_POOL *pool_ptr, ULONG *allocates, ULONG *releases,
                    ULONG *fragments_searched, ULONG *merges, ULONG *splits, ULONG *suspensions, ULONG *timeouts)
{

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA

UINT        status;


    /* Determine if this is a legal request.  */
    if (pool_ptr == TX_NULL)
    {

        /* Byte pool pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }

    /* Determine if the pool ID is invalid.  */
    else if (pool_ptr -> tx_byte_pool_id != TX_BYTE_POOL_ID)
    {

        /* Byte pool pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_BYTE_POOL_PERFORMANCE_INFO_GET, pool_ptr, 0, 0, 0, TX_TRACE_BYTE_POOL_EVENTS)

        /* Log this kernel call.  */
        TX_EL_BYTE_POOL_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */

        /* Retrieve the number of allocates on this byte pool.  */
        if (allocates != TX_NULL)
        {

            *allocates =  pool_ptr -> tx_byte_pool_performance_allocate_count;
        }

        /* Retrieve the number of releases on this byte pool.  */
        if (releases != TX_NULL)
        {

            *releases =  pool_ptr -> tx_byte_pool_performance_release_count;
        }

        /* Retrieve the number of fragments searched in this byte pool.  */
        if (fragments_searched != TX_NULL)
        {

            *fragments_searched =  pool_ptr -> tx_byte_pool_performance_search_count;
        }

        /* Retrieve the number of fragments merged on this byte pool.  */
        if (merges != TX_NULL)
        {

            *merges =  pool_ptr -> tx_byte_pool_performance_merge_count;
        }

        /* Retrieve the number of fragment splits on this byte pool.  */
        if (splits != TX_NULL)
        {

            *splits =  pool_ptr -> tx_byte_pool_performance_split_count;
        }

        /* Retrieve the number of suspensions on this byte pool.  */
        if (suspensions != TX_NULL)
        {

            *suspensions =  pool_ptr -> tx_byte_pool_performance_suspension_count;
        }

        /* Retrieve the number of timeouts on this byte pool.  */
        if (timeouts != TX_NULL)
        {

            *timeouts =  pool_ptr -> tx_byte_pool_performance_timeout_count;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return completion status.  */
        status =  TX_SUCCESS;
    }

    /* Return completion status.  */
    return(status);
#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (pool_ptr != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (allocates != TX_NULL)
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

