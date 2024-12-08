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
/**   Internet Protocol (IPv6)                                            */
/**                                                                       */
/**************************************************************************/


#define NX_SOURCE_CODE

/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#if defined(NX_ENABLE_IPV6_MULTICAST) && defined(FEATURE_NX_IPV6)
#include "nx_ip.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS
#endif /* NX_ENABLE_IPV6_MULTICAST && FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_ipv6_multicast_interface_leave                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service checks for errors in the IPv6 multicast leave call.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    group_address                         IPv6 multicast address        */
/*    interface_index                       Index to phyical interface    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    IPv6_Address_Type                     Obtain IPv6 address type      */
/*    _nxd_ipv6_multicast_interface_leave   Actual IPv6 multicast leave   */
/*                                            function                    */
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
UINT  _nxde_ipv6_multicast_interface_leave(NX_IP *ip_ptr, NXD_ADDRESS *group_address, UINT interface_index)
{

#if defined(NX_ENABLE_IPV6_MULTICAST) && defined(FEATURE_NX_IPV6)
UINT status;

    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (group_address == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid input parameter.  */
    if ((IPv6_Address_Type(group_address -> nxd_ip_address.v6) & IPV6_ADDRESS_MULTICAST) != IPV6_ADDRESS_MULTICAST)
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Validate the interface. */
    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual IPv6 enable function.  */
    status =  _nxd_ipv6_multicast_interface_leave(ip_ptr, group_address, interface_index);

    /* Return completion status.  */
    return(status);

#else /* ! NX_ENABLE_IPV6_MULTICAST */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(group_address);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* NX_ENABLE_IPV6_MULTICAST && FEATURE_NX_IPV6 */
}

