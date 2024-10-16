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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_tcp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_info_get                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves TCP information for the specified IP        */
/*    instance.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to the IP instance    */
/*    tcp_packets_sent                      Destination for number of     */
/*                                            packets sent                */
/*    tcp_bytes_sent                        Destination for number of     */
/*                                            bytes sent                  */
/*    tcp_packets_received                  Destination for number of     */
/*                                            packets received            */
/*    tcp_bytes_received                    Destination for number of     */
/*                                            bytes received              */
/*    tcp_invalid_packets                   Destination for number of     */
/*                                            invalid packets             */
/*    tcp_receive_packets_dropped           Destination for number of     */
/*                                            receive packets dropped     */
/*    tcp_checksum_errors                   Destination for number of     */
/*                                            checksum errors             */
/*    tcp_connections                       Destination for number of     */
/*                                            connections                 */
/*    tcp_disconnections                    Destination for number of     */
/*                                            disconnections              */
/*    tcp_connections_dropped               Destination for number of     */
/*                                            connections dropped         */
/*    tcp_retransmit_packets                Destination for number of     */
/*                                            retransmit packets          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection             */
/*    tx_mutex_put                          Release protection            */
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
UINT  _nx_tcp_info_get(NX_IP *ip_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                       ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                       ULONG *tcp_invalid_packets, ULONG *tcp_receive_packets_dropped,
                       ULONG *tcp_checksum_errors, ULONG *tcp_connections,
                       ULONG *tcp_disconnections, ULONG *tcp_connections_dropped,
                       ULONG *tcp_retransmit_packets)
{

    /* Obtain the IP mutex so we can examine the bound port.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if packets sent is wanted.  */
    if (tcp_packets_sent)
    {

        /* Return the number of packets sent by this IP instance.  */
        *tcp_packets_sent =  ip_ptr -> nx_ip_tcp_packets_sent;
    }

    /* Determine if bytes sent is wanted.  */
    if (tcp_bytes_sent)
    {

        /* Return the number of bytes sent by this IP instance.  */
        *tcp_bytes_sent =  ip_ptr -> nx_ip_tcp_bytes_sent;
    }

    /* Determine if packets received is wanted.  */
    if (tcp_packets_received)
    {

        /* Return the number of packets received by this IP instance.  */
        *tcp_packets_received =  ip_ptr -> nx_ip_tcp_packets_received;
    }

    /* Determine if bytes received is wanted.  */
    if (tcp_bytes_received)
    {

        /* Return the number of bytes received by this IP instance.  */
        *tcp_bytes_received =  ip_ptr -> nx_ip_tcp_bytes_received;
    }

    /* Determine if invalid packets is wanted.  */
    if (tcp_invalid_packets)
    {

        /* Return the number of invalid packets by this IP instance.  */
        *tcp_invalid_packets =  ip_ptr -> nx_ip_tcp_invalid_packets;
    }

    /* Determine if receive packets dropped is wanted.  */
    if (tcp_receive_packets_dropped)
    {

        /* Return the number of receive packets dropped by this IP instance.  */
        *tcp_receive_packets_dropped =  ip_ptr -> nx_ip_tcp_receive_packets_dropped;
    }

    /* Determine if checksum errors is wanted.  */
    if (tcp_checksum_errors)
    {

        /* Return the number of checksum errors by this IP instance.  */
        *tcp_checksum_errors =  ip_ptr -> nx_ip_tcp_checksum_errors;
    }

    /* Determine if connections is wanted.  */
    if (tcp_connections)
    {

        /* Return the number of connections by this IP instance.  */
        *tcp_connections =  ip_ptr -> nx_ip_tcp_connections;
    }

    /* Determine if disconnections is wanted.  */
    if (tcp_disconnections)
    {

        /* Return the number of disconnections by this IP instance.  */
        *tcp_disconnections =  ip_ptr -> nx_ip_tcp_disconnections;
    }

    /* Determine if connections dropped is wanted.  */
    if (tcp_connections_dropped)
    {

        /* Return the number of connections dropped by this IP instance.  */
        *tcp_connections_dropped =  ip_ptr -> nx_ip_tcp_connections_dropped;
    }

    /* Determine if retransmit packets is wanted.  */
    if (tcp_retransmit_packets)
    {

        /* Return the number of retransmit packets by this IP instance.  */
        *tcp_retransmit_packets =  ip_ptr -> nx_ip_tcp_retransmit_packets;
    }

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion status.  */
    return(NX_SUCCESS);
}

