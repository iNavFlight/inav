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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */


#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_process_ns                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function processes an incoming neighbor solicitation  */
/*       message.  In response to a valid NS message, it also sends out   */
/*       a neighbor solicitation, and updates the Neighbor Cache.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                  IP stack instance                           */
/*    packet_ptr              The echo reply packet                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                       Obtain exclusive lock (on table) */
/*    tx_mutex_put                       Release exclusive lock           */
/*    _nx_packet_release                 Release packet back to pool      */
/*    _nx_ipv6_packet_send               Send IPv6 packet to remote host  */
/*    _nx_nd_cache_add                   Add entry to ND cache table      */
/*    _nx_nd_cache_find_entry            Find ND cache entry by IP address*/
/*    _nx_icmpv6_send_queued_packets     Send packets queued waiting for  */
/*                                            physical mapping            */
/*    _nx_icmpv6_validate_neighbor_message                                */
/*                                       Validate received ICMPv6 packet  */
/*    _nx_ip_checksum_compute            Compute NS packet ICMP checksum  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process                 Main ICMPv6 processor            */
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
VOID _nx_icmpv6_process_ns(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_ICMPV6_ND     *nd_ptr;
NX_ICMPV6_OPTION *option_ptr;
USHORT           *mac_addr;
UINT              source_unspecified;
UINT              error;
UINT              option_length;
NX_ICMPV6_HEADER *header_ptr;
NX_IPV6_HEADER   *ipv6_header;
UINT              SLLA_changed = NX_FALSE;
UINT              i;
NXD_IPV6_ADDRESS *interface_addr;
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT              compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
ULONG             dest_address[4];


    /* Initialize local variable: assume source address is specified. */
    source_unspecified = NX_FALSE;

    /* Assume there is no error. */
    error = 0;

    /* Get a pointer to the ICMP message header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_ICMPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Get a pointer to the IPv6 header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

    /* Get a pointer to the Neighbor Discovery message. */
    /*lint -e{929} suppress cast of pointer to pointer, since it is necessary  */
    nd_ptr = (NX_ICMPV6_ND *)header_ptr;

    /* Convert target address to host byte order. */
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(nd_ptr -> nx_icmpv6_nd_targetAddress);

    /* Convert flag field to host byte order. */
    NX_CHANGE_ULONG_ENDIAN(nd_ptr -> nx_icmpv6_nd_flag);


    /* Validate the packet. */
    if (_nx_icmpv6_validate_neighbor_message(packet_ptr) != NX_SUCCESS)
    {
        error = 1;
    }

    /* Find whether or not sender is unspecified.  If sender is unspecified,
       the sender is performing DAD process. */
    if (CHECK_UNSPECIFIED_ADDRESS(ipv6_header -> nx_ip_header_source_ip))
    {

        /* Mark the packet source as nonsolicited. */
        source_unspecified = NX_TRUE;
    }

    /* Find the appropriate interface to send the packet out on, based
       on the destination address. */

    /* Get a pointer to the first ipv6 address in the interface address list. */
    interface_addr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr;

    if (!error)
    {

        /* Loop to match IP addresses.  */
        while (interface_addr != NX_NULL)
        {

            /* Does the current address in the IP interface list match the one in the
               ND message header? */
            if ((CHECK_IPV6_ADDRESSES_SAME(interface_addr -> nxd_ipv6_address,
                                           nd_ptr -> nx_icmpv6_nd_targetAddress)))
            {

                /* We're done matching. */
                break;
            }

            /* Get next IPv6 address in the interface address list.  */
            interface_addr = interface_addr -> nxd_ipv6_address_next;
        }
    }

    if (error || (interface_addr == NX_NULL))
    {

#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid packet error. */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

        /* An error occurred.  Release the packet. */
        _nx_packet_release(packet_ptr);

        return;
    }

    /* Have find a valid address.  */
    packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr = interface_addr;

    /*
     * Once we get here, we have two cases:
     * (1) Sender is in the DAD process
     * (2) Sender wants to find our MAC address.
     *
     * So first, we need to find out whether or not the source IP address is
     *  the unspecified address.
     */

    if (source_unspecified == NX_TRUE)
    {

        /* The sender is in DAD process. */

#ifndef NX_DISABLE_IPV6_DAD
        /* The sender is doing a DAD on the same address as we have... */
        if (interface_addr -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_TENTATIVE)
        {

            /* Our interface address is in tentative state.  Therefore interface
               address is also invalid.  */
            _nx_icmpv6_DAD_failure(ip_ptr, interface_addr);

            _nx_packet_release(packet_ptr);
            return;
        }
#endif
        /* Our address state is not in tentative mode.  That means
           we have a valid address.  In this case, we should send a response */
    }

    /* Get a pointer to the ICMPv6 options. */
    /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
    option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(header_ptr, sizeof(NX_ICMPV6_ND));

    /* We'll need to keep track of option data to parse the options. */
    option_length = (UINT)packet_ptr -> nx_packet_length - (UINT)sizeof(NX_ICMPV6_ND);

    /* Walk through the ICMPv6 options, if any. */
    while (option_length > 0)
    {

        /* Handle the source link-layer address option for a solicited NS request. */
        if (option_ptr -> nx_icmpv6_option_type == ICMPV6_OPTION_TYPE_SRC_LINK_ADDR)
        {

        ND_CACHE_ENTRY *nd_entry;
        UINT            status;


            /* At this point, the source address has been verified to be
               valid (not unspecified.) Therefore we should add
               the source link layer address to our neighbor cache. */

            /* Is the source IP address is not in our ND cache? */

            status = _nx_nd_cache_find_entry(ip_ptr, ipv6_header -> nx_ip_header_source_ip, &nd_entry);

            if (status != NX_SUCCESS)
            {

                /* Yes, this NS fills in the mac address so the LLA is changed. */
                SLLA_changed = NX_TRUE;

                /* No, so we create a cache entry. */
                /*lint -e{929} suppress cast from pointer to pointer, since it is necessary  */
                /*lint -e{826} suppress cast of pointer to pointer, since it is necessary  */
                /*lint -e{740} suppress unusual cast of pointer, since it is necessary  */
                _nx_nd_cache_add(ip_ptr, ipv6_header -> nx_ip_header_source_ip,
                                 interface_addr -> nxd_ipv6_address_attached,
                                 (CHAR *)&option_ptr -> nx_icmpv6_option_data, 0, ND_CACHE_STATE_STALE,
                                 interface_addr, &nd_entry);
            }
            else
            {

            /* Entry already exists.  If the mac address is the same, do not update the entry. Otherwise,
               update the entry and set the state to STALE (RFC2461 7.2.3) */
            ULONG mac_msw, mac_lsw, new_msw, new_lsw;

            /*lint -e{928} suppress cast from pointer to pointer, since it is necessary  */
            UCHAR *new_mac = (UCHAR *)&option_ptr -> nx_icmpv6_option_data;

                /*lint -e{644} suppress variable might not be initialized, since "nd_entry" was initialized in _nx_nd_cache_find_entry. */
                mac_msw = ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[0]) << 8) | (nd_entry -> nx_nd_cache_mac_addr[1]);
                mac_lsw = ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[2]) << 24) | ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[3]) << 16) |
                          ((ULONG)(nd_entry -> nx_nd_cache_mac_addr[4]) << 8) | nd_entry -> nx_nd_cache_mac_addr[5];
                new_msw = ((ULONG)(new_mac[0]) << 8) | (new_mac[1]);
                new_lsw = ((ULONG)(new_mac[2]) << 24) | ((ULONG)(new_mac[3]) << 16) | ((ULONG)(new_mac[4]) << 8) | new_mac[5]; /* lgtm[cpp/overflow-buffer] */
                if ((mac_msw != new_msw) || (mac_lsw != new_lsw)) /* If the new MAC is different from what we have in the table. */
                {

                    /* This NS changes the cache entry mac address, so the LLA is changed. */
                    SLLA_changed = NX_TRUE;

                    /* Set the mac address. */
                    for (i = 0; i < 6; i++)
                    {
                        nd_entry -> nx_nd_cache_mac_addr[i] = new_mac[i];
                    }

                    /* Set the state to STALE.  */
                    nd_entry -> nx_nd_cache_nd_status = ND_CACHE_STATE_STALE;

                    /* Set the interface. */
                    nd_entry -> nx_nd_cache_interface_ptr = interface_addr -> nxd_ipv6_address_attached;
                }

                if (nd_entry -> nx_nd_cache_packet_waiting_head) /* There are packets waiting to be transmitted */
                {

                    /* Ok to transmit the packets now. */
                    _nx_icmpv6_send_queued_packets(ip_ptr, nd_entry);
                }
            }
        }

        option_length -= ((UINT)(option_ptr -> nx_icmpv6_option_length) << 3);

        /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
        option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(option_ptr, ((option_ptr -> nx_icmpv6_option_length) << 3));
    }

    /* We may need to reset a reachable timer if there is no new LLA involved.
       Did we change the cache LLA? */
    if (SLLA_changed == NX_FALSE)
    {

    /* No, so if this cache state is REACHABLE, check if we should
       reset the timer tick after receiving a NS packet from the neighbor.
       RFC 2461 7.2.3. */

    ND_CACHE_ENTRY *nd_entry;

        /* Verify the source of the NS packet in our ND cache, in case
           we have not already done so. */
        if (_nx_nd_cache_find_entry(ip_ptr, ipv6_header -> nx_ip_header_source_ip, &nd_entry) == NX_SUCCESS)
        {

            /* Yes, is it currently in a reachable state? */
            /*lint -e{644} suppress variable might not be initialized, since "nd_entry" was initialized in _nx_nd_cache_find_entry. */
            if (nd_entry -> nx_nd_cache_nd_status == ND_CACHE_STATE_REACHABLE)
            {
                /* Ok to update the timer. */
                nd_entry -> nx_nd_cache_timer_tick = ip_ptr -> nx_ipv6_reachable_timer;
            }
        }
    }

    /*
     * A valid NS has been received.  Prepare a Neighbor Advertisement packet
     *    according to RFC2461 section 7.2.4.
     *
     * Adjust the packet, eliminate the option part.  At this point, the packet prepend_ptr points to the
     *    beginning of the ICMP message. The size of the ICMP message includes ICMP NA and the target
     *    link layer address option field (8 bytes).
     */
    packet_ptr -> nx_packet_append_ptr =
        packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_ICMPV6_ND) + 8;

    packet_ptr -> nx_packet_length = sizeof(NX_ICMPV6_ND) + 8;

    if (source_unspecified == NX_TRUE)
    {

        /* Response to an unsolicited NS: clear the S bit.*/
        nd_ptr -> nx_icmpv6_nd_flag = (0x20000000);
    }
    else
    {
        /* Response to a normal NS: set the S bit.*/
        nd_ptr -> nx_icmpv6_nd_flag = (0x60000000);
    }

    NX_CHANGE_ULONG_ENDIAN(nd_ptr -> nx_icmpv6_nd_flag);

    /* nd_ptr -> targetAddress has been converted to host byte order.
       We need to convert it back to network byte order. */
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(nd_ptr -> nx_icmpv6_nd_targetAddress);

    header_ptr -> nx_icmpv6_header_type = NX_ICMPV6_NEIGHBOR_ADVERTISEMENT_TYPE;
    header_ptr -> nx_icmpv6_header_code = 0;
    header_ptr -> nx_icmpv6_header_checksum = 0;

    if (source_unspecified == NX_TRUE)
    {

        /* Sender uses unspecified address.  So we send NA to all node multicast address. */
        /* RFC2461 7.2.4 */
        dest_address[0] = 0xFF020000;
        dest_address[1] = 0;
        dest_address[2] = 0;
        dest_address[3] = 0x00000001;
    }
    else
    {

        /* Set the packet destination IP address */
        COPY_IPV6_ADDRESS(ipv6_header -> nx_ip_header_source_ip,
                          dest_address);
    }

    /*
        Fill in the options.  Since we are using the same packet,
        the option_ptr is already pointing to the option field.
     */
    /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
    option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(header_ptr, sizeof(NX_ICMPV6_ND));
    option_ptr -> nx_icmpv6_option_type = ICMPV6_OPTION_TYPE_TRG_LINK_ADDR;
    option_ptr -> nx_icmpv6_option_length = 1;
    mac_addr = &option_ptr -> nx_icmpv6_option_data;

    mac_addr[0] = (USHORT)(interface_addr -> nxd_ipv6_address_attached -> nx_interface_physical_address_msw);
    mac_addr[1] = (USHORT)((interface_addr -> nxd_ipv6_address_attached -> nx_interface_physical_address_lsw & 0xFFFF0000) >> 16); /* lgtm[cpp/overflow-buffer] */
    mac_addr[2] = (USHORT)(interface_addr -> nxd_ipv6_address_attached -> nx_interface_physical_address_lsw & 0x0000FFFF); /* lgtm[cpp/overflow-buffer] */

    NX_CHANGE_USHORT_ENDIAN(mac_addr[0]);
    NX_CHANGE_USHORT_ENDIAN(mac_addr[1]); /* lgtm[cpp/overflow-buffer] */
    NX_CHANGE_USHORT_ENDIAN(mac_addr[2]); /* lgtm[cpp/overflow-buffer] */

#ifdef NX_DISABLE_ICMPV6_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV6_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (interface_addr -> nxd_ipv6_address_attached -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {

        /* Compute the checksum. */
        header_ptr -> nx_icmpv6_header_checksum =
            _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_ICMPV6,
                                    (UINT)packet_ptr -> nx_packet_length,
                                    interface_addr -> nxd_ipv6_address,
                                    dest_address);

        /* Write the checksum to the ICMPv6 header. */
        header_ptr -> nx_icmpv6_header_checksum = (USHORT)(~(header_ptr -> nx_icmpv6_header_checksum));

        NX_CHANGE_USHORT_ENDIAN(header_ptr -> nx_icmpv6_header_checksum);
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY  */
    /* Send out the NA reply. */
    _nx_ipv6_packet_send(ip_ptr, packet_ptr,
                         NX_PROTOCOL_ICMPV6,
                         packet_ptr -> nx_packet_length,
                         255 /* NA message must have hop limit 255 */,
                         interface_addr -> nxd_ipv6_address,
                         dest_address);

    /* (Let the driver release the packet.) */
    return;
}

#endif /* FEATURE_NX_IPV6 */

