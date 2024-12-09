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
/** NetX Component                                                        */
/**                                                                       */
/**   Packet Pool Management (Packet)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_packet_pool_info_get                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves information about the specified packet      */
/*    pool.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                              Pool to get information from  */
/*    total_packets                         Destination for total packets */
/*    free_packets                          Destination for free packets  */
/*    empty_pool_requests                   Destination for empty requests*/
/*    empty_pool_suspensions                Destination for empty         */
/*                                            suspensions                 */
/*    invalid_packet_releases               Destination for invalid packet*/
/*                                            release requests            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
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
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_packet_pool_info_get(NX_PACKET_POOL *pool_ptr, ULONG *total_packets, ULONG *free_packets,
                               ULONG *empty_pool_requests, ULONG *empty_pool_suspensions,
                               ULONG *invalid_packet_releases)
{
TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts to get packet pool information.  */
    TX_DISABLE

    /* Determine if pool total packets is wanted.  */
    if (total_packets)
    {

        /* Return the number of total packets in this pool.  */
        *total_packets =  pool_ptr -> nx_packet_pool_total;
    }

    /* Determine if pool free packets is wanted.  */
    if (free_packets)
    {

        /* Return the number of free packets in this pool.  */
        *free_packets =  pool_ptr -> nx_packet_pool_available;
    }

    /* Determine if empty pool requests is wanted.  */
    if (empty_pool_requests)
    {

        /* Return the number of empty pool requests made in this pool.  */
        *empty_pool_requests =  pool_ptr -> nx_packet_pool_empty_requests;
    }

    /* Determine if empty pool suspensions is wanted.  */
    if (empty_pool_suspensions)
    {

        /* Return the number of empty pool suspensions made in this pool.  */
        *empty_pool_suspensions =  pool_ptr -> nx_packet_pool_empty_suspensions;
    }

    /* Determine if invalid packet releases is wanted.  */
    if (invalid_packet_releases)
    {

        /* Return the number of invalid packet releases made in this pool.  */
        *invalid_packet_releases =  pool_ptr -> nx_packet_pool_invalid_releases;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_PACKET_POOL_INFO_GET, pool_ptr, pool_ptr -> nx_packet_pool_total, pool_ptr -> nx_packet_pool_available, pool_ptr -> nx_packet_pool_empty_requests, NX_TRACE_PACKET_EVENTS, 0, 0);

    /* Return completion status.  */
    return(NX_SUCCESS);
}

