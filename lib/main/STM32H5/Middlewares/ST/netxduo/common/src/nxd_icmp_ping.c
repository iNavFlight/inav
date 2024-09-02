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
#include "nx_icmpv6.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_icmp_ping                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Depending on the destination IP address, this function performs     */
/*    ICMP ping or ICMPv6 ping.                                           */
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
/*    NX_SUCCESS                            Successful completion         */
/*    NX_NOT_SUPPORTED                      IPv6 or ICMP not enabled      */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_icmp_ping                         IPv4 ICMP ping routine.       */
/*    _nx_icmp_ping6                        IPv6 ICMP ping routine.       */
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
UINT  _nxd_icmp_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, CHAR *data_ptr, ULONG data_size,
                     NX_PACKET **response_ptr, ULONG wait_option)
{

UINT status = NX_NOT_SUPPORTED;

#ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        status = _nx_icmp_ping(ip_ptr, ip_address -> nxd_ip_address.v4, data_ptr, data_size,
                               response_ptr, wait_option);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        status = _nx_icmp_ping6(ip_ptr, ip_address, data_ptr, data_size,
                                response_ptr, wait_option);
    }
#endif /* FEATURE_NX_IPV6 */

    return(status);
}

