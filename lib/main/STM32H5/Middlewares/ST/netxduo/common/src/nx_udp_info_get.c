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
/**   User Datagram Protocol (UDP)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_udp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_info_get                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves UDP information from the specified IP       */
/*    instance.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    udp_packets_sent                      Destination to the number of  */
/*                                            packets sent                */
/*    udp_bytes_sent                        Destination to the number of  */
/*                                            bytes sent                  */
/*    udp_packets_received                  Destination to the number of  */
/*                                            packets received            */
/*    udp_bytes_received                    Destination to the number of  */
/*                                            bytes received              */
/*    udp_invalid_packets                   Destination to the number of  */
/*                                            invalid packets             */
/*    udp_receive_packets_dropped           Destination to the number of  */
/*                                            receive packets dropped     */
/*    udp_checksum_errors                   Destination to the number of  */
/*                                            checksum errors             */
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
UINT  _nx_udp_info_get(NX_IP *ip_ptr, ULONG *udp_packets_sent, ULONG *udp_bytes_sent,
                       ULONG *udp_packets_received, ULONG *udp_bytes_received,
                       ULONG *udp_invalid_packets, ULONG *udp_receive_packets_dropped,
                       ULONG *udp_checksum_errors)
{

TX_INTERRUPT_SAVE_AREA


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_INFO_GET, ip_ptr, ip_ptr -> nx_ip_udp_bytes_sent, ip_ptr -> nx_ip_udp_bytes_received, ip_ptr -> nx_ip_udp_invalid_packets, NX_TRACE_UDP_EVENTS, 0, 0);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Determine if packets sent is wanted.  */
    if (udp_packets_sent)
    {

        /* Return the number of packets sent by this IP instance.  */
        *udp_packets_sent =  ip_ptr -> nx_ip_udp_packets_sent;
    }

    /* Determine if bytes sent is wanted.  */
    if (udp_bytes_sent)
    {

        /* Return the number of bytes sent by this IP instance.  */
        *udp_bytes_sent =  ip_ptr -> nx_ip_udp_bytes_sent;
    }

    /* Determine if packets received is wanted.  */
    if (udp_packets_received)
    {

        /* Return the number of packets received by this IP instance.  */
        *udp_packets_received =  ip_ptr -> nx_ip_udp_packets_received;
    }

    /* Determine if bytes received is wanted.  */
    if (udp_bytes_received)
    {

        /* Return the number of bytes received by this IP instance.  */
        *udp_bytes_received =  ip_ptr -> nx_ip_udp_bytes_received;
    }

    /* Determine if invalid packets is wanted.  */
    if (udp_invalid_packets)
    {

        /* Return the number of invalid packets by this IP instance.  */
        *udp_invalid_packets =  ip_ptr -> nx_ip_udp_invalid_packets;
    }

    /* Determine if receive packets dropped is wanted.  */
    if (udp_receive_packets_dropped)
    {

        /* Return the number of receive packets dropped by this IP instance.  */
        *udp_receive_packets_dropped =  ip_ptr -> nx_ip_udp_receive_packets_dropped;
    }

    /* Determine if checksum errors is wanted.  */
    if (udp_checksum_errors)
    {

        /* Return the number of checksum errors by this IP instance.  */
        *udp_checksum_errors =  ip_ptr -> nx_ip_udp_checksum_errors;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

