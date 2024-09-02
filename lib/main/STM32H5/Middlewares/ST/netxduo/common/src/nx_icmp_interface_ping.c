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

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_interface_ping                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function builds an ICMP ping request packet and calls the      */
/*    associated driver to send it out on the network.  The function will */
/*    then suspend for the specified time waiting for the ICMP ping       */
/*    response.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    ip_address                            IP address to ping            */
/*    interface_ptr                         Pointer to interface          */
/*    next_hop_address                      Next hop address              */
/*    data_ptr                              User Data pointer             */
/*    data_size                             Size of User Data             */
/*    response_ptr                          Pointer to Response Packet    */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_checksum_compute               Computer checksum             */
/*    _nx_ip_packet_send                    IP packet send function       */
/*    _nx_packet_allocate                   Allocate a packet for the     */
/*                                            ICMP ping request           */
/*    _nx_packet_release                    Release packet to packet pool */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _tx_thread_system_suspend             Suspend thread                */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmp_ping                                                       */
/*    _nxd_icmp_source_ping                                               */
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
UINT  _nx_icmp_interface_ping(NX_IP *ip_ptr, ULONG ip_address,
                              NX_INTERFACE *interface_ptr, ULONG next_hop_address,
                              CHAR *data_ptr, ULONG data_size,
                              NX_PACKET **response_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
NX_PACKET      *request_ptr;
NX_ICMP_HEADER *header_ptr;
ULONG           checksum;
#if defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
ULONG           compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
ULONG           sequence;
TX_THREAD      *thread_ptr;
ULONG           data_offset;

#ifdef NX_IPSEC_ENABLE
VOID           *sa = NX_NULL;
NXD_ADDRESS     src_addr;
NXD_ADDRESS     dest_addr;
UINT            ret = 0;
#endif /* NX_IPSEC_ENABLE */


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ICMP_PING, ip_ptr, ip_address, data_ptr, data_size, NX_TRACE_ICMP_EVENTS, 0, 0);

    /* Clear the destination pointer.  */
    *response_ptr =  NX_NULL;
    data_offset = 0;

#ifdef NX_IPSEC_ENABLE
    /* Create address data for finding an IPSec SA match. */
    src_addr.nxd_ip_version = NX_IP_VERSION_V4;
    src_addr.nxd_ip_address.v4 = interface_ptr -> nx_interface_ip_address;
    dest_addr.nxd_ip_version = NX_IP_VERSION_V4;
    dest_addr.nxd_ip_address.v4 = ip_address;

    /* Check if IPsec is enabled. */
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)
    {
        /* It is. Check for possible SA match. */
        ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr,                                    /* IP ptr */
                                                      &src_addr,                                 /* src_addr */
                                                      &dest_addr,                                /* dest_addr */
                                                      NX_PROTOCOL_ICMP,                          /* protocol */
                                                      0,                                         /* src_port */
                                                      0,                                         /* dest_port */
                                                      &data_offset, &sa, (NX_ICMP_ECHO_REQUEST_TYPE << 8));

        /* Does our IPSec SA allow this ping packet to pass through? */
        if (ret == NX_IPSEC_TRAFFIC_BYPASS)
        {

            /* Yes, ok to pass through without encryption or authentication. */
            sa = NX_NULL;
            data_offset = 0;
        }
        /* Does our IPSec SA indicate this packet may not be sent?  */
        else if (ret == NX_IPSEC_TRAFFIC_DROP || ret == NX_IPSEC_TRAFFIC_PENDING_IKEV2)
        {

            /* Return the error status. */
            return(NX_IPSEC_REJECTED);
        }
    }

#endif /* NX_IPSEC_ENABLE */

    /* Allocate a packet to place the ICMP echo request message in.  */
    /*lint -e{845} suppress argument to operator '+' is certain to be 0, since "data_offset" can be non-zero when NX_IPSEC_ENABLE is defined. */
    status =  _nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool, &request_ptr,
                                  (ULONG)(NX_IPv4_ICMP_PACKET + data_offset + NX_ICMP_HEADER_SIZE), wait_option);
    if (status)
    {

        /* Error getting packet, so just get out!  */
        return(status);
    }

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, request_ptr);

    /* Copy the data into the packet payload area.  */
    /*lint -e{644} suppress variable might not be initialized, since "request_ptr" was initialized in _nx_packet_allocate. */
    status = _nx_packet_data_append(request_ptr, (VOID *)data_ptr, data_size, ip_ptr -> nx_ip_default_packet_pool, wait_option);

    /* Check return status. */
    if (status)
    {

        /* Release the packet.  */
        _nx_packet_release(request_ptr);

        /* Error, the data area is too big for the default packet payload.  */
        return(status);
    }

    /* Store outgoing interface. */
    request_ptr -> nx_packet_address.nx_packet_interface_ptr = interface_ptr;

#ifdef NX_IPSEC_ENABLE
    /* Store SA. */
    request_ptr -> nx_packet_ipsec_sa_ptr = sa;
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_ICMP_INFO
    /* Increment the ICMP ping count.  */
    ip_ptr -> nx_ip_pings_sent++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ICMP_SEND, ip_ptr, ip_address, request_ptr, (((ULONG)NX_ICMP_ECHO_REQUEST_TYPE) << 24), NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Calculate the ICMP echo request message size and store it in the
       packet header.  */
    request_ptr -> nx_packet_length += (ULONG)NX_ICMP_HEADER_SIZE;

    /* Adjust the nx_packet_prepend_ptr for ICMP header. */
    request_ptr -> nx_packet_prepend_ptr -= NX_ICMP_HEADER_SIZE;

    /* Build the ICMP request packet.  */

    /* Setup the pointer to the message area.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_ICMP_HEADER *)request_ptr -> nx_packet_prepend_ptr;

    /* Write the ICMP type into the message.  Use the lower 16-bits of the IP address for
       the ICMP identifier.  */
    header_ptr -> nx_icmp_header_word_0 =  (ULONG)(NX_ICMP_ECHO_REQUEST_TYPE << 24);
    sequence =                             (ip_ptr -> nx_ip_icmp_sequence++ & NX_LOWER_16_MASK);
    header_ptr -> nx_icmp_header_word_1 =  (ULONG)(request_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address << 16) | sequence;

    /* If NX_LITTLE_ENDIAN is defined, the headers need to be swapped to match
       that of the data area.  */
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_icmp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_icmp_header_word_1);

#ifdef NX_DISABLE_ICMPV4_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV4_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (request_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#ifdef NX_IPSEC_ENABLE
    if ((sa != NX_NULL) && (((NX_IPSEC_SA *)sa) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
    {
        compute_checksum = 1;
    }
#endif

#if defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {

        /* Compute the checksum of the ICMP packet.  */
        checksum = _nx_ip_checksum_compute(request_ptr, NX_IP_ICMP,
                                           (UINT)request_ptr -> nx_packet_length,
                                           /* ICMPV4 checksum does not include
                                              src/dest addresses */
                                           NX_NULL, NX_NULL);

        /* If NX_LITTLE_ENDIAN is defined, the headers need to be swapped back so
           we can place the checksum in the ICMP header.  */
        NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_icmp_header_word_0);

        /* Place the checksum into the first header word.  */
        header_ptr -> nx_icmp_header_word_0 =  header_ptr -> nx_icmp_header_word_0 | (~checksum & NX_LOWER_16_MASK);

        /* If NX_LITTLE_ENDIAN is defined, the first header word needs to be swapped
           back.  */
        NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_icmp_header_word_0);
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        request_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

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
            ip_ptr -> nx_ip_icmp_ping_suspension_list =    thread_ptr;
            thread_ptr -> tx_thread_suspended_next =       thread_ptr;
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
    /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
    _nx_ip_packet_send(ip_ptr, request_ptr, ip_address,
                       NX_IP_NORMAL, NX_IP_TIME_TO_LIVE, NX_IP_ICMP, NX_FRAGMENT_OKAY, next_hop_address);

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
#endif /* !NX_DISABLE_IPV4  */

