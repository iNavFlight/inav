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
#include "nx_ipv6.h"
#include "nx_packet.h"

/* Bring in externs for caller checking code.  */


NX_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_ip_raw_packet_send                             PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a raw IP packet through the specified IPv6      */
/*    interface.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    destination_ip                        Destination IP address        */
/*    protocol                              Value for the protocol field  */
/*    ttl                                   Value for ttl or hop limit    */
/*    tos                                   Value for tos or traffic      */
/*                                            class and flow label        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_NOT_ENABLED                        Raw IP not enabled            */
/*    NX_IP_ADDRESS_ERROR                   Invalid address version       */
/*    NX_INVALID_PARAMETERS                 Invalid protocol specified    */
/*    NX_UNDERFLOW                          Invalid packet header         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_ip_raw_packet_source_send        Actual raw packet send        */
/*                                             function.                  */
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
UINT  _nxde_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET **packet_ptr_ptr,
                               NXD_ADDRESS *destination_ip, ULONG protocol, UINT ttl, ULONG tos)
{


NX_PACKET *packet_ptr;


    /* Setup packet pointer.  */
    packet_ptr =  *packet_ptr_ptr;

    /* Check for invalid input pointers.  */
    /* Cast the ULONG into a packet pointer. Since this is exactly what we wish to do, disable the lint warning with the following comment:  */
    /*lint -e{923} suppress cast of ULONG to pointer.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (packet_ptr == NX_NULL) ||
        (packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != ((NX_PACKET *)NX_PACKET_ALLOCATED)))
    {

        return(NX_PTR_ERROR);
    }

    /* Check to see if IP raw packet processing is enabled.  */
    if (!ip_ptr -> nx_ip_raw_ip_processing)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for invalid IP address.  */
    if (!destination_ip || ((destination_ip -> nxd_ip_version != NX_IP_VERSION_V6) &&
                            (destination_ip -> nxd_ip_version != NX_IP_VERSION_V4)))
    {

        return(NX_IP_ADDRESS_ERROR);
    }

    /* Check for valid protocol.  */
    if ((protocol & 0x000000FF) != protocol)
    {
        return(NX_INVALID_PARAMETERS);
    }

#ifndef NX_DISABLE_IPV4
    if (destination_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* Check for an invalid packet prepend pointer for IPv4 packet.  */
        /*lint -e{946} suppress pointer subtraction, since it is necessary. */
        if ((packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV4_HEADER)) < packet_ptr -> nx_packet_data_start)
        {
            return(NX_UNDERFLOW);
        }
        if (destination_ip -> nxd_ip_address.v4 == 0)
        {
            return(NX_IP_ADDRESS_ERROR);
        }
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (destination_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Check for an invalid packet prepend pointer for IPv6 packet.  */
        /*lint -e{946} suppress pointer subtraction, since it is necessary. */
        if ((packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV6_HEADER)) < packet_ptr -> nx_packet_data_start)
        {
            return(NX_UNDERFLOW);
        }
        if (CHECK_UNSPECIFIED_ADDRESS(&destination_ip -> nxd_ip_address.v6[0]))
        {
            return(NX_IP_ADDRESS_ERROR);
        }
    }
#endif /* FEATURE_NX_IPV6  */


    /* Check for an invalid packet append pointer.  */
    /*lint -e{946} suppress pointer subtraction, since it is necessary. */
    if (packet_ptr -> nx_packet_append_ptr > packet_ptr -> nx_packet_data_end)
    {
        return(NX_OVERFLOW);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    _nxd_ip_raw_packet_source_send(ip_ptr, packet_ptr, destination_ip, 0, protocol, ttl, tos);

    /* Now clear the application's packet pointer so it can't be accidentally
       used again by the application.  This is only done when error checking is
       enabled.  */
    *packet_ptr_ptr =  NX_NULL;

    /* Return completion status.  */
    return(NX_SUCCESS);
}

