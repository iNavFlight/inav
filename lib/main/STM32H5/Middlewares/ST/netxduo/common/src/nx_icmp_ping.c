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
#include "nx_ip.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_ping                                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a suitable interface and calls interface ping.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    ip_address                            IP address to ping            */
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
/*    _nx_ip_route_find                     Find suitable interface       */
/*    _nx_icmp_interface_ping               Send ping packet              */
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
UINT  _nx_icmp_ping(NX_IP *ip_ptr, ULONG ip_address,
                    CHAR *data_ptr, ULONG data_size,
                    NX_PACKET **response_ptr, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
ULONG         next_hop_address;
NX_INTERFACE *interface_ptr = NX_NULL;


    /* Find a suitable interface for sending the ping packet. */
    if (_nx_ip_route_find(ip_ptr, ip_address, &interface_ptr, &next_hop_address) != NX_SUCCESS)
    {

        /* No suitable interface configured. */
        /* Clear the destination pointer.  */
        *response_ptr =  NX_NULL;

        /* Return the error status. */
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Call interface ping service. */
    /*lint -e{644} suppress variable might not be initialized, since "interface_ptr" and "next_hop_address" were initialized in _nx_ip_route_find. */
    return(_nx_icmp_interface_ping(ip_ptr, ip_address, interface_ptr, next_hop_address,
                                   data_ptr, data_size, response_ptr, wait_option));
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(data_ptr);
    NX_PARAMETER_NOT_USED(data_size);
    NX_PARAMETER_NOT_USED(response_ptr);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

