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
/*    _nxde_ipv6_address_get                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the IP address get function      */
/*    call.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    address_index                         Index to the IPv6 address     */
/*                                            table                       */
/*    ip_address                            Pointer to IP address         */
/*                                            structure to be filled      */
/*    prefix_length                         Pointer to prefix length      */
/*                                            to return                   */
/*    interface_index                       The index to the physical     */
/*                                            interface this address      */
/*                                            belongs to                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_ipv6_address_get                 Actual IPv6 address get       */
/*                                            function                    */
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
UINT  _nxde_ipv6_address_get(NX_IP *ip_ptr, UINT address_index, NXD_ADDRESS *ip_address, ULONG *prefix_length, UINT *interface_index)
{
#ifdef FEATURE_NX_IPV6

UINT status;


    /* Check for invalid IP pointer.  */
    if (ip_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for other invalid input.  */
    if ((ip_ptr -> nx_ip_id != NX_IP_ID) || (ip_address == NX_NULL) || (prefix_length == NX_NULL) || (interface_index == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check the validity of the address index.  */
    if (address_index >= (NX_MAX_IPV6_ADDRESSES + NX_LOOPBACK_IPV6_ENABLED))
    {
        return(NX_NO_INTERFACE_ADDRESS);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual IP address get function.  */
    status =  _nxd_ipv6_address_get(ip_ptr, address_index, ip_address, prefix_length, interface_index);

    /* Return completion status.  */
    return(status);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(address_index);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(prefix_length);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

