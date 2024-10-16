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
#include "nx_icmp.h"
#include "nx_packet.h"
#include "nx_ip.h"
#include "tx_thread.h"
#include "nx_icmpv6.h"
#include "nx_ipv6.h"


#ifdef FEATURE_NX_IPV6


#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_interface_ping6                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles ICMPv6 ping requests and calls the associated */
/*    network driver to send it out on the network.  The function will    */
/*    then suspend for the specified time waiting for the ICMP ping       */
/*    response.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    dest_ip_address                       IPv6 address to ping          */
/*    data_ptr                              Pointer to user data to       */
/*                                           include in ping packet       */
/*    data_size                             Size of user data             */
/*    address_index                         Source IPv6 address to use    */
/*    response_ptr                          Pointer to response packet    */
/*    wait_option                           Time out on packet allocate   */
/*                                             and sending packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*    NX_OVERFLOW                           Data exceeds packet payload   */
/*    NX_IPSEC_REJECTED                     IPSec check failed            */
/*    NX_NO_RESPONSE                        No response to ping message   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_icmp_checksum_compute             Computes ICMP checksum        */
/*    _nx_ipv6_packet_send                  Send IPv6 packet function     */
/*    _nx_packet_allocate                   Allocate a packet for the     */
/*                                            ICMP ping request           */
/*    _nx_packet_release                    Release packet on error       */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _tx_thread_system_suspend             Suspend thread                */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmp_ping6                                                      */
/*    Application                           Send ping on the specified    */
/*                                                   interface            */
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
UINT  _nx_icmp_interface_ping6(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, CHAR *data_ptr, ULONG data_size,
                               NXD_IPV6_ADDRESS *ipv6_address, NX_PACKET **response_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
NX_PACKET      *request_ptr;
NX_ICMPV6_ECHO *echo_header_ptr;
ULONG           checksum;
ULONG           sequence;
TX_THREAD      *thread_ptr;

#ifdef TX_ENABLE_EVENT_TRACE
ULONG           ip_address_lsw;
#endif /* TX_ENABLE_EVENT_TRACE */
#ifdef NX_IPSEC_ENABLE
VOID           *sa = NX_NULL;
NXD_ADDRESS     src_addr;
UINT            ret = 0;
#endif /* NX_IPSEC_ENABLE */
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT            compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */


#ifdef TX_ENABLE_EVENT_TRACE
    ip_address_lsw = ip_address -> nxd_ip_address.v6[3];

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ICMP_PING6, ip_ptr, ip_address_lsw, data_ptr, data_size, NX_TRACE_ICMP_EVENTS, 0, 0);
#endif /* TX_ENABLE_EVENT_TRACE */

    /* Clear the destination pointer.  */
    *response_ptr =  NX_NULL;

#ifdef NX_IPSEC_ENABLE

    src_addr.nxd_ip_version = NX_IP_VERSION_V6;
    COPY_IPV6_ADDRESS(ipv6_address -> nxd_ipv6_address, src_addr.nxd_ip_address.v6);

    /* Check if the IPsec is enabled. */
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)
    {

        /* Yes it is. Check for possible SA match. */
        ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr,                                 /* IP ptr    */
                                                      &src_addr,                              /* src_addr  */
                                                      ip_address,                             /* dest_addr */
                                                      NX_PROTOCOL_ICMPV6,                     /* protocol  */
                                                      0,                                      /* src_port  */
                                                      0,                                      /* dest_port */
                                                      NX_NULL, &sa, (NX_ICMPV6_ECHO_REQUEST_TYPE << 8));

        /* Check if the SA rules allow us to send this packet. */
        if (ret == NX_IPSEC_TRAFFIC_BYPASS)
        {

            /* They do. Ok to send the ping6 packet. */
            sa = NX_NULL;
        }
        else if (ret == NX_IPSEC_TRAFFIC_DROP || ret == NX_IPSEC_TRAFFIC_PENDING_IKEV2)
        {

            /* No we cannot. Set the error status and return.*/
            return(NX_IPSEC_REJECTED);
        }
    }

#endif /* NX_IPSEC_ENABLE */

    /* Determine if the size of the data and the ICMP header is larger than
       the packet payload area.  */

    /* Allocate a packet to place the ICMP echo request message in.  */
    status =  _nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool, &request_ptr,
                                  NX_ICMP_PACKET + sizeof(NX_ICMPV6_ECHO), wait_option);
    if (status)
    {

        /* Error getting packet, so just get out!  */
        return(status);
    }

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, request_ptr);

    /* Mark the packet as IPv6 */
    /*lint -e{644} suppress variable might not be initialized, since "request_ptr" was initialized in _nx_packet_allocate. */
    request_ptr -> nx_packet_ip_version = NX_IP_VERSION_V6;

#ifdef NX_IPSEC_ENABLE

    request_ptr -> nx_packet_ipsec_sa_ptr = sa;

#endif /* NX_IPSEC_ENABLE */

    /* Copy the data into the packet payload area.  */
    status = _nx_packet_data_append(request_ptr, (VOID *)data_ptr, data_size, ip_ptr -> nx_ip_default_packet_pool, wait_option);

    /* Check return status. */
    if (status)
    {

        /* Release the packet.  */
        _nx_packet_release(request_ptr);

        /* Error, the data area is too big for the default packet payload.  */
        return(NX_OVERFLOW);
    }

    /* Set the outgoing address.  */
    request_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr =  ipv6_address;

#ifndef NX_DISABLE_ICMP_INFO
    /* Increment the ICMP ping count.  */
    ip_ptr -> nx_ip_pings_sent++;
#endif

#ifdef TX_ENABLE_EVENT_TRACE
    /* If trace is enabled, insert this event into the trace buffer.  */
/*     NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ICMP_SEND, ip_ptr, dest_ip_byte3, request_ptr, (((ULONG) NX_ICMP_ECHO_REQUEST_TYPE) << 24), NX_TRACE_INTERNAL_EVENTS, 0, 0) */
#endif /* TX_ENABLE_EVENT_TRACE */

    /* Calculate the ICMP echo request message size and store it in the
       packet header.  */
    request_ptr -> nx_packet_length += (ULONG)sizeof(NX_ICMPV6_ECHO);

    /* Adjust the nx_packet_prepend_ptr for ICMP header. */
    request_ptr -> nx_packet_prepend_ptr -= sizeof(NX_ICMPV6_ECHO);

    /* Build the ICMP request packet.  */

    /* Setup the pointer to the message area.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    echo_header_ptr =  (NX_ICMPV6_ECHO *)request_ptr -> nx_packet_prepend_ptr;

    /* Write the ICMP type into the message.  Use the lower 16-bits of the IP address for
       the ICMP identifier.  */
    echo_header_ptr -> nx_icmpv6_echo_header.nx_icmpv6_header_type = NX_ICMPV6_ECHO_REQUEST_TYPE;
    echo_header_ptr -> nx_icmpv6_echo_header.nx_icmpv6_header_code = 0;
    echo_header_ptr -> nx_icmpv6_echo_header.nx_icmpv6_header_checksum = 0;

    echo_header_ptr -> nx_icmpv6_echo_identifier = (USHORT)(ipv6_address -> nxd_ipv6_address[3] & NX_LOWER_16_MASK);

    NX_CHANGE_USHORT_ENDIAN(echo_header_ptr -> nx_icmpv6_echo_identifier);

    sequence = (ip_ptr -> nx_ip_icmp_sequence++ & NX_LOWER_16_MASK);
    echo_header_ptr -> nx_icmpv6_echo_sequence_num = (USHORT)(sequence);

    NX_CHANGE_USHORT_ENDIAN(echo_header_ptr -> nx_icmpv6_echo_sequence_num);

#ifdef NX_DISABLE_ICMPV6_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV6_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (ipv6_address -> nxd_ipv6_address_attached -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#ifdef NX_IPSEC_ENABLE
    if (sa != NX_NULL)
    {
        if (((NX_IPSEC_SA *)sa) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE)
        {
            compute_checksum = 1;
        }
    }

#endif /* NX_IPSEC_ENABLE */

#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {
        /* Compute the checksum of the ICMP packet.  */
        checksum =  _nx_ip_checksum_compute(request_ptr, NX_PROTOCOL_ICMPV6,
                                            (UINT)request_ptr -> nx_packet_length,
                                            ipv6_address -> nxd_ipv6_address,
                                            ip_address -> nxd_ip_address.v6);

        echo_header_ptr -> nx_icmpv6_echo_header.nx_icmpv6_header_checksum = (USHORT) ~checksum;

        /* Endian swap */
        NX_CHANGE_USHORT_ENDIAN(echo_header_ptr -> nx_icmpv6_echo_header.nx_icmpv6_header_checksum);
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        request_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM;
    }
#endif
    /* Obtain the IP internal mutex to prevent against possible suspension later in the
       call to IP packet send.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Pickup thread pointer.  */
    thread_ptr =  _tx_thread_current_ptr;

    /* Determine if the request specifies suspension.  */
    if (wait_option)
    {

        /* Prepare for suspension of this thread.  */

        /* Setup cleanup routine pointer.  */
        thread_ptr -> tx_thread_suspend_cleanup =  _nx_icmp_cleanup;

        thread_ptr -> tx_thread_suspend_status =   NX_NO_RESPONSE;

        /* Setup cleanup information, i.e. this pool control
           block.  */
        thread_ptr -> tx_thread_suspend_control_block =  (void *)ip_ptr;

        /* Save the return packet pointer address as well.  */
        thread_ptr -> tx_thread_additional_suspend_info =  (void *)response_ptr;

        /* Save the sequence number so this can be matched up with an ICMP
           response later.  */
        thread_ptr -> tx_thread_suspend_info =  sequence;

        /* Setup suspension list.  */
        if (ip_ptr -> nx_ip_icmp_ping_suspension_list)
        {

            /* This list is not NULL, add current thread to the end. */
            thread_ptr -> tx_thread_suspended_next =
                ip_ptr -> nx_ip_icmp_ping_suspension_list;

            thread_ptr -> tx_thread_suspended_previous =
                (ip_ptr -> nx_ip_icmp_ping_suspension_list) -> tx_thread_suspended_previous;

            ((ip_ptr -> nx_ip_icmp_ping_suspension_list) -> tx_thread_suspended_previous) -> tx_thread_suspended_next =
                thread_ptr;

            (ip_ptr -> nx_ip_icmp_ping_suspension_list) -> tx_thread_suspended_previous =   thread_ptr;
        }
        else
        {

            /* No other threads are suspended.  Setup the head pointer and
               just setup this threads pointers to itself.  */
            ip_ptr -> nx_ip_icmp_ping_suspension_list  =   thread_ptr;
            thread_ptr -> tx_thread_suspended_next     =   thread_ptr;
            thread_ptr -> tx_thread_suspended_previous =   thread_ptr;
        }

        /* Increment the suspended thread count.  */
        ip_ptr -> nx_ip_icmp_ping_suspended_count++;

        /* Set the state to suspended.  */
        thread_ptr -> tx_thread_state =  TX_TCP_IP;

        /* Set the suspending flag.  */
        thread_ptr -> tx_thread_suspending =  TX_TRUE;

        /* Save the timeout value.  */
        thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Send the ICMP packet to the IP component.  */
    _nx_ipv6_packet_send(ip_ptr, request_ptr, NX_PROTOCOL_ICMPV6,
                         request_ptr -> nx_packet_length, ip_ptr -> nx_ipv6_hop_limit,
                         ipv6_address -> nxd_ipv6_address,
                         ip_address -> nxd_ip_address.v6);

    /* If wait option is requested, suspend the thread.  */
    if (wait_option)
    {

        /* Release the protection on the ARP list.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Call actual thread suspension routine.  */
        _tx_thread_system_suspend(thread_ptr);

        if (thread_ptr -> tx_thread_suspend_status == NX_SUCCESS)
        {

            /* Add debug information. */
            NX_PACKET_DEBUG(__FILE__, __LINE__, *response_ptr);
        }

        /* Return the status from the thread control block.  */
        return(thread_ptr -> tx_thread_suspend_status);
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Release preemption disable.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Release the protection mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();

        /* Immediate return, return error completion.  */
        return(NX_NO_RESPONSE);
    }
}

#endif /* FEATURE_NX_IPV6 */

