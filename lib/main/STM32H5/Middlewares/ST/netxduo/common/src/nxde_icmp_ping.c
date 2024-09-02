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
#include "nx_ip.h"
#include "nx_icmp.h"


/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_icmp_ping                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the dual stack (IPv4 or IPv6 ICMP*/
/*    ping function call.                                                 */
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
/*    _nxd_icmp_ping                        Actual ICMP ping function     */
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
UINT  _nxde_icmp_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address,
                      CHAR *data_ptr, ULONG data_size,
                      NX_PACKET **response_ptr, ULONG wait_option)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (response_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid IP address.  */
    if (ip_address == NX_NULL)
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Check for invalid version. */
    if ((ip_address -> nxd_ip_version != NX_IP_VERSION_V4) &&
        (ip_address -> nxd_ip_version != NX_IP_VERSION_V6))
    {

        return(NX_IP_ADDRESS_ERROR);
    }

#ifndef NX_DISABLE_IPV4
    /* Check to see if ICMP is enabled.  */
    /* Cast the function pointer into a ULONG. Since this is exactly what we wish to do, disable the lint warning with the following comment:  */
    /*lint -e{923} suppress cast of pointer to ULONG.  */
    if ((ip_address -> nxd_ip_version == NX_IP_VERSION_V4) &&
        ((ALIGN_TYPE)ip_ptr -> nx_ip_icmpv4_packet_process == NX_NULL))
    {

        return(NX_NOT_ENABLED);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6

    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Cast the function pointer into a ULONG. Since this is exactly what we wish to do, disable the lint warning with the following comment:  */
        /*lint -e{923} suppress cast of pointer to ULONG.  */
        if (((ALIGN_TYPE)ip_ptr -> nx_ip_icmpv6_packet_process == NX_NULL) ||
            ((ALIGN_TYPE)ip_ptr -> nx_ipv6_packet_receive == NX_NULL))
        {

            return(NX_NOT_ENABLED);
        }
    }

#endif /* FEATURE_NX_IPV6 */

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual ICMP ping function.  */
    status =  _nxd_icmp_ping(ip_ptr, ip_address, data_ptr, data_size,
                             response_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}

