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
/**   Packet Pool Mangement (Packet)                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */
#include "nx_api.h"
#include "nx_ip.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#include "nx_icmp.h"
#endif /* NX_IPSEC_ENABLE */

#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6  */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_max_payload_size_find                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the maximum payload size at application level   */
/*    that would not cause IPv4 or IPv6 fragmentation.  It takes the      */
/*    layer 4 protocol (TCP vs. UDP), layer 3 protocol (IPv4 vs. IPv6)    */
/*    and IPsec into consideration, and reports back to values:           */
/*    (1) start_offset_ptr: This is the location where enough space       */
/*        for protocol header is reserved for TCP/UDP/IP/IPsec headers;   */
/*    (2) payload_length_ptr: the amount of data application may          */
/*        transfer without exceeding MTU.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    dest_address                          Packet Destination Address    */
/*    if_index                              Interface index for ipv4,     */
/*                                          address index for ipv6.       */
/*    src_port                              Source port number, in host   */
/*                                            byte order.                 */
/*    dest_port                             Destination port number,      */
/*                                            in host byte order.         */
/*    protocol                              Protocol type                 */
/*    start_offset_ptr                      Pointer to the start of data  */
/*                                            for maximum packet payload. */
/*    payload_length_ptr                    Pointer to the computed       */
/*                                            payload size that would not */
/*                                            cause IP fragmentation.     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion Code.              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipsec_sa_egress_lookup            The acutal function that      */
/*                                            performs the SA lookup      */
/*                                            (if IPsec is enable)        */
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
UINT _nx_ip_max_payload_size_find(NX_IP *ip_ptr,
                                  NXD_ADDRESS *dest_address,
                                  UINT if_index,
                                  UINT src_port,
                                  UINT dest_port,
                                  ULONG protocol,
                                  ULONG *start_offset_ptr,
                                  ULONG *payload_length_ptr)

{

ULONG        payload_length = 0;
ULONG        start_offset = 0;

#if defined(NX_IPSEC_ENABLE) || defined(TX_ENABLE_EVENT_TRACE)
NXD_ADDRESS  src_address = {0};
#endif /* (NX_IPSEC_ENABLE) || defined(TX_ENABLE_EVENT_TRACE) */

#ifdef NX_IPSEC_ENABLE
UINT         block_size;
NX_IPSEC_SA *sa = NX_NULL;
UINT         ret;
ULONG        data_offset;
#endif /* NX_IPSEC_ENABLE */

#ifdef TX_ENABLE_EVENT_TRACE
ULONG        src_address_lsw = 0, dst_address_lsw = 0;
#endif /* TX_ENABLE_EVENT_TRACE */

#ifndef NX_DISABLE_IPV4
    /* First check the version. */
    if (dest_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* Computer header space needed for physical layer and IP layer.  */
        start_offset = NX_IPv4_PACKET;

        /* In this case, if_index indicate the interface_index.  */

        /* Check the validity of the if_index.  */
        if ((if_index >= NX_MAX_PHYSICAL_INTERFACES) || (!ip_ptr -> nx_ip_interface[if_index].nx_interface_ip_address))
        {
            return(NX_INVALID_INTERFACE);
        }

#if defined(NX_IPSEC_ENABLE) || defined(TX_ENABLE_EVENT_TRACE)
        /* Record the source address.  */
        src_address.nxd_ip_address.v4 = ip_ptr -> nx_ip_interface[if_index].nx_interface_ip_address;
        src_address.nxd_ip_version = NX_IP_VERSION_V4;
#endif /* NX_IPSEC_ENABLE */

        /* Get the mtu size.  */
        payload_length = ip_ptr -> nx_ip_interface[if_index].nx_interface_ip_mtu_size + NX_PHYSICAL_HEADER;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (dest_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Computer header space needed for physical layer and IP layer.  */
        start_offset = NX_IPv6_PACKET;

        /* In this case, if_index indicate the address_index.  */

        /* Check the validity of the if_index.  */
        if (if_index >= (NX_MAX_IPV6_ADDRESSES + NX_LOOPBACK_IPV6_ENABLED))
        {
            return(NX_IP_ADDRESS_ERROR);
        }

#if defined(NX_IPSEC_ENABLE) || defined(TX_ENABLE_EVENT_TRACE)
        /* Record the source address.  */
        COPY_IPV6_ADDRESS(ip_ptr -> nx_ipv6_address[if_index].nxd_ipv6_address, src_address.nxd_ip_address.v6);
        src_address.nxd_ip_version = NX_IP_VERSION_V6;
#endif /* NX_IPSEC_ENABLE */

        /* Get the mtu size.  */
        payload_length = ip_ptr -> nx_ipv6_address[if_index].nxd_ipv6_address_attached -> nx_interface_ip_mtu_size + NX_PHYSICAL_HEADER;
    }
#endif /* FEATURE_NX_IPV6  */

    if (protocol == NX_PROTOCOL_TCP)
    {
        /* Add TCP header length. */
        start_offset += 20;
    }
    else if (protocol == NX_PROTOCOL_UDP)
    {
        /* Add UDP header length. */
        start_offset += 8;
    }

    /* Now compute the max protocol payload size that would not lead to IP fragmentation
       e.g. keeping payload length at or below the MTU size. */
    payload_length -= start_offset;

#ifdef NX_IPSEC_ENABLE
    ret = NX_IPSEC_TRAFFIC_BYPASS;
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup)
    {
#ifdef FEATURE_NX_IPV6
        if (src_address.nxd_ip_version == NX_IP_VERSION_V4)
        {
#endif /* FEATURE_NX_IPV6  */
            ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr, &src_address, dest_address,
                                                          (UCHAR)protocol, src_port, dest_port,
                                                          &data_offset, (VOID **)&sa, (NX_ICMP_ECHO_REQUEST_TYPE << 8));
#ifdef FEATURE_NX_IPV6
        }
        else
        {
            ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr, &src_address, dest_address,
                                                          (UCHAR)protocol, src_port, dest_port,
                                                          &data_offset, (VOID **)&sa, (NX_ICMPV6_ECHO_REQUEST_TYPE << 8));
        }
#endif /* FEATURE_NX_IPV6  */
    }
    if (ret == NX_IPSEC_TRAFFIC_PROTECT)
    {
        /* Add IPsec protocol header layer. */
        start_offset += sa -> nx_ipsec_sa_head_encap_size;
        payload_length -= sa -> nx_ipsec_sa_head_encap_size;
        /* Align the payload_length to the SA algorithm block size. */
        if (sa -> nx_ipsec_sa_protocol == NX_PROTOCOL_NEXT_HEADER_ENCAP_SECURITY)
        {
            /* The next block computes the authentication trailer block size in the ESP combined mode. */
            /* Payload length should exclude this trailer size.  */
            if (sa -> nx_ipsec_sa_integrity_method != NX_NULL)
            {
                block_size = sa -> nx_ipsec_sa_integrity_method -> nx_crypto_block_size_in_bytes;
                payload_length -= block_size;
            }

            /* Compute the the actual data payload size.  */
            if ((sa -> nx_ipsec_sa_encryption_method != NX_NULL) &&
                (sa -> nx_ipsec_sa_encryption_method -> nx_crypto_algorithm != NX_CRYPTO_NONE))
            {
                /* Assume block size is 4, for ENCRYPTION_NULL algorithm. */
                block_size = 4;
                if (sa -> nx_ipsec_sa_encryption_method -> nx_crypto_algorithm != NX_CRYPTO_ENCRYPTION_NULL)
                {
                    block_size = sa -> nx_ipsec_sa_encryption_method -> nx_crypto_block_size_in_bytes;
                }

                /* Round the payload size to multiple of block size. */
                payload_length = payload_length / block_size;
                payload_length = payload_length * block_size;
                payload_length -= NX_IPSEC_ESP_TRAIL_LEN;
            }
        }
        /* No need to update payload_length if sa_protocol is AH. */
    }
#else
    NX_PARAMETER_NOT_USED(src_port);
    NX_PARAMETER_NOT_USED(dest_port);
#endif /* NX_IPSEC_ENABLE */

    /* Return payload_length and start_offset values. */

    if (payload_length_ptr)
    {
        *payload_length_ptr = payload_length;
    }

    if (start_offset_ptr)
    {
        *start_offset_ptr = start_offset;
    }

#ifdef TX_ENABLE_EVENT_TRACE
#ifndef NX_DISABLE_IPV4
    if (dest_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        src_address_lsw = src_address.nxd_ip_address.v4;
        dst_address_lsw = dest_address -> nxd_ip_address.v4;
    }
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
    if (dest_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        src_address_lsw = src_address.nxd_ip_address.v6[3];
        dst_address_lsw = dest_address -> nxd_ip_address.v6[3];
    }
#endif /* FEATURE_NX_IPV6 */

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_IP_MAX_PAYLOAD_SIZE_FIND,  src_address_lsw, dst_address_lsw, payload_length, start_offset,  NX_TRACE_PACKET_EVENTS, 0, 0);

#endif /* TX_ENABLE_EVENT_TRACE */

    return(NX_SUCCESS);
}

