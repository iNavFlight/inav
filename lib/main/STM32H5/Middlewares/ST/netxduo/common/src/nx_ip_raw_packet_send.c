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
/*    _nx_ip_raw_packet_send                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for raw packet send enabled on the input IP    */
/*    instance, then finds a valid interface for sending the packet. If   */
/*    those requirements are met it sends a raw IP packet out the         */
/*    appropriate interface.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    destination_ip                        Destination IP address        */
/*    type_of_service                       Type of service for packet    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_route_find                     Find out-going interface and  */
/*                                            next hop                    */
/*    nx_ip_packet_send                     Core IP packet send service   */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
UINT  _nx_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                             ULONG destination_ip, ULONG type_of_service)
{

#ifndef NX_DISABLE_IPV4
ULONG next_hop_address = NX_NULL;

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_RAW_PACKET_SEND, ip_ptr, destination_ip, type_of_service, packet_ptr, NX_TRACE_IP_EVENTS, 0, 0);

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Make sure a valid interface exists for the destination. */
    if (_nx_ip_route_find(ip_ptr, destination_ip, &packet_ptr -> nx_packet_address.nx_packet_interface_ptr, &next_hop_address) != NX_SUCCESS)
    {

        /* Release protection. */
        tx_mutex_put(&ip_ptr -> nx_ip_protection);

        /* There is not; return the error status. */
        return(NX_IP_ADDRESS_ERROR);
    }
    else
    {

        /* Yes, a valid interface exists. Send the packet!  */
        /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized as long as return value is NX_SUCCESS. */
        _nx_ip_packet_send(ip_ptr, packet_ptr, destination_ip, type_of_service, NX_IP_TIME_TO_LIVE, NX_IP_RAW, NX_FRAGMENT_OKAY, next_hop_address);
    }

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status!  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(packet_ptr);
    NX_PARAMETER_NOT_USED(destination_ip);
    NX_PARAMETER_NOT_USED(type_of_service);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

