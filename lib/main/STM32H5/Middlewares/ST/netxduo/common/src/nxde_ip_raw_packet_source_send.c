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

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_ip_raw_packet_source_send                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function does error checking for the raw IP packet send serivce*/
/*    out the specified IP source.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    destination_ip                        Destination IP address        */
/*    address_index                         Index to the IPv6 address     */
/*    protocol                              Value for the protocol field  */
/*    ttl                                   Value for ttl or hop limit    */
/*    tos                                   Value for tos or traffic      */
/*                                            class and flow label        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_NOT_ENABLED                        Raw packet send not enabled   */
/*    NX_IP_ADDRESS_ERROR                   Invalid input address         */
/*    NX_INVALID_INTERFACE                  Invalid interface input       */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nxd_ip_raw_packet_source_send         Actual raw packet send service*/
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
UINT  _nxde_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                      NXD_ADDRESS *destination_ip, UINT address_index, ULONG protocol, UINT ttl, ULONG tos)
{

UINT status;

    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (packet_ptr == NX_NULL) || (destination_ip == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Determine if raw IP packet sending/receiving is enabled.  */
    if (!ip_ptr -> nx_ip_raw_ip_processing)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check that the destination address version is either IPv4 or IPv6. */
    if ((destination_ip -> nxd_ip_version != NX_IP_VERSION_V4) &&
        (destination_ip -> nxd_ip_version != NX_IP_VERSION_V6))
    {
        return(NX_IP_ADDRESS_ERROR);
    }

#ifndef NX_DISABLE_IPV4
    if (destination_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        if (destination_ip -> nxd_ip_address.v4 == 0)
        {
            return(NX_IP_ADDRESS_ERROR);
        }

        /* Check for valid interface. */
        if (address_index >= NX_MAX_IP_INTERFACES)
        {
            return(NX_INVALID_INTERFACE);
        }

        /* Check for an invalid packet prepend pointer for IPv4 packet.  */
        /*lint -e{946} suppress pointer subtraction, since it is necessary. */
        if ((packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV4_HEADER)) < packet_ptr -> nx_packet_data_start)
        {
            return(NX_UNDERFLOW);
        }
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (destination_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* destination_ip -> nxd_ip_version == NX_IP_VERSION_V6.  */
        /* Check for valid destination address. */
        if (CHECK_UNSPECIFIED_ADDRESS(&destination_ip -> nxd_ip_address.v6[0]))
        {
            return(NX_IP_ADDRESS_ERROR);
        }

        /* Check for valid interface. */
        if (address_index >= (NX_MAX_IPV6_ADDRESSES + NX_LOOPBACK_IPV6_ENABLED))
        {
            return(NX_IP_ADDRESS_ERROR);
        }

        /* Check for an invalid packet prepend pointer for IPv6 packet.  */
        /*lint -e{946} suppress pointer subtraction, since it is necessary. */
        if ((packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV6_HEADER)) < packet_ptr -> nx_packet_data_start)
        {
            return(NX_UNDERFLOW);
        }
    }
#endif /* FEATURE_NX_IPV6 */

    /* Check for an invalid packet append pointer.  */
    /*lint -e{946} suppress pointer subtraction, since it is necessary. */
    if (packet_ptr -> nx_packet_append_ptr > packet_ptr -> nx_packet_data_end)
    {
        return(NX_OVERFLOW);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nxd_ip_raw_packet_source_send(ip_ptr, packet_ptr, destination_ip, address_index, protocol, ttl, tos);

    /* Return completion status. */
    return(status);
}

