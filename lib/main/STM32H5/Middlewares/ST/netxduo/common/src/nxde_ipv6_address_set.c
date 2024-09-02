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
#include "nx_ipv6.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ip.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS
#endif /* FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_ipv6_address_set                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the IP address set               */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    interface_index                       The index to the physical     */
/*                                            interface this address      */
/*                                            belongs to                  */
/*    ip_address                            Pointer to IP address         */
/*                                            structure                   */
/*    prefix_length                         Prefix length                 */
/*    address_index                         Index to the IPv6 address     */
/*                                            table                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    IPv6_Address_Type                     Find IPv6 address type.       */
/*    _nxd_ipv6_address_set                 Actual IP address set function*/
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
UINT  _nxde_ipv6_address_set(NX_IP *ip_ptr, UINT if_index, NXD_ADDRESS *ip_address, ULONG prefix_length, UINT *address_index)
{
#ifdef FEATURE_NX_IPV6

UINT  status;
ULONG AddressType;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid IP addresses.  */

    /* (Do not apply to autoconfigured linklocal addresses) */
    if ((ip_address != NULL) && (prefix_length != 10))
    {

        /* A non null IP address must have the version set. */
        if (ip_address -> nxd_ip_version != NX_IP_VERSION_V6)
        {
            return(NX_IP_ADDRESS_ERROR);
        }
    }

    if ((if_index >= NX_MAX_PHYSICAL_INTERFACES) ||
        (ip_ptr -> nx_ip_interface[if_index].nx_interface_valid != NX_TRUE))
    {
        return(NX_INVALID_INTERFACE);
    }


    /* Make sure the address is unicast address. */
    /* Find out the type of the incoming IP address. */
    if (ip_address)
    {
        AddressType = IPv6_Address_Type(ip_address -> nxd_ip_address.v6);

        if ((AddressType & IPV6_ADDRESS_UNICAST) == 0)
        {
            return(NX_IP_ADDRESS_ERROR);
        }
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual IP address set function.  */
    status =  _nxd_ipv6_address_set(ip_ptr, if_index, ip_address, prefix_length, address_index);

    /* Return completion status.  */
    return(status);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(if_index);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(prefix_length);
    NX_PARAMETER_NOT_USED(address_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

