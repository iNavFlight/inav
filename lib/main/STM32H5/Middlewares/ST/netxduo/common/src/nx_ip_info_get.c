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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_info_get                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves information about the specified IP          */
/*    instance.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    ip_total_packets_sent                 Destination for number of     */
/*                                            packets sent                */
/*    ip_total_bytes_sent                   Destination for number of     */
/*                                            bytes sent                  */
/*    ip_total_packets_received             Destination for number of     */
/*                                            packets received            */
/*    ip_total_bytes_received               Destination for number of     */
/*                                            bytes received              */
/*    ip_invalid_packets                    Destination for number of     */
/*                                            invalid packets             */
/*    ip_receive_packets_dropped            Destination for number of     */
/*                                            packets dropped             */
/*    ip_receive_checksum_errors            Destination for number of     */
/*                                            checksum errors             */
/*    ip_send_packets_dropped               Destination for number of     */
/*                                            send packets dropped        */
/*    ip_total_fragments_sent               Destination for number of     */
/*                                            fragments sent              */
/*    ip_total_fragments_received           Destination for number of     */
/*                                            fragments received          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT  _nx_ip_info_get(NX_IP *ip_ptr, ULONG *ip_total_packets_sent, ULONG *ip_total_bytes_sent,
                      ULONG *ip_total_packets_received, ULONG *ip_total_bytes_received,
                      ULONG *ip_invalid_packets, ULONG *ip_receive_packets_dropped,
                      ULONG *ip_receive_checksum_errors, ULONG *ip_send_packets_dropped,
                      ULONG *ip_total_fragments_sent, ULONG *ip_total_fragments_received)
{

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_INFO_GET, ip_ptr, ip_ptr -> nx_ip_total_bytes_sent, ip_ptr -> nx_ip_total_bytes_received, ip_ptr -> nx_ip_receive_packets_dropped, NX_TRACE_IP_EVENTS, 0, 0);

    /* Obtain protection on this IP instance.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if IP total packets sent is wanted.  */
    if (ip_total_packets_sent)
    {

        /* Return the number of IP total packets sent by this IP instance.  */
        *ip_total_packets_sent =  ip_ptr -> nx_ip_total_packets_sent;
    }

    /* Determine if IP total bytes sent is wanted.  */
    if (ip_total_bytes_sent)
    {

        /* Return the number of IP total bytes sent by this IP instance.  */
        *ip_total_bytes_sent =  ip_ptr -> nx_ip_total_bytes_sent;
    }

    /* Determine if IP total packets received is wanted.  */
    if (ip_total_packets_received)
    {

        /* Return the number of IP total packets received by this IP instance.  */
        *ip_total_packets_received =  ip_ptr -> nx_ip_total_packets_received;
    }

    /* Determine if IP total bytes received is wanted.  */
    if (ip_total_bytes_received)
    {

        /* Return the number of IP total bytes received by this IP instance.  */
        *ip_total_bytes_received =  ip_ptr -> nx_ip_total_bytes_received;
    }

    /* Determine if IP invalid packets is wanted.  */
    if (ip_invalid_packets)
    {

        /* Return the number of IP invalid packets received by this IP instance.  */
        *ip_invalid_packets =  ip_ptr -> nx_ip_invalid_packets;
    }

    /* Determine if IP receive packets dropped is wanted.  */
    if (ip_receive_packets_dropped)
    {

        /* Return the number of IP receive packets dropped by this IP instance.  */
        *ip_receive_packets_dropped =  ip_ptr -> nx_ip_receive_packets_dropped;
    }

    /* Determine if IP receive checksum errors is wanted.  */
    if (ip_receive_checksum_errors)
    {

        /* Return the number of IP receive checksum errors by this IP instance.  */
        *ip_receive_checksum_errors =  ip_ptr -> nx_ip_receive_checksum_errors;
    }

    /* Determine if IP send packets dropped is wanted.  */
    if (ip_send_packets_dropped)
    {

        /* Return the number of IP send packets dropped by this IP instance.  */
        *ip_send_packets_dropped =  ip_ptr -> nx_ip_send_packets_dropped;
    }

#ifndef NX_DISABLE_FRAGMENTATION
    /* Determine if IP total fragments sent is wanted.  */
    if (ip_total_fragments_sent)
    {

        /* Return the number of IP total fragments sent by this IP instance.  */
        *ip_total_fragments_sent =  ip_ptr -> nx_ip_total_fragments_sent;
    }

    /* Determine if IP total fragments received is wanted.  */
    if (ip_total_fragments_received)
    {

        /* Return the number of IP total fragments received by this IP instance.  */
        *ip_total_fragments_received =  ip_ptr -> nx_ip_total_fragments_received;
    }
#else /* NX_DISABLE_FRAGMENTATION */
    /* Set IP Total Framgnets Sent to zero */
    if (ip_total_fragments_sent)
    {

        *ip_total_fragments_sent =  0;
    }

    /* Set IP Total Framgnets Received to zero */
    if (ip_total_fragments_received)
    {

        *ip_total_fragments_received =  0;
    }

#endif /* NX_DISABLE_FRAGMENTATION */

    /* Release the protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}

