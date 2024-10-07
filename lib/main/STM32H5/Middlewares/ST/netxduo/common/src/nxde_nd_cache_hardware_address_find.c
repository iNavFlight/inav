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
/**   Neighbor Discovery Cache                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_nd_cache.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ip.h"
#include "nx_ipv6.h"

/* Bring in externs for caller checking code.  */
NX_CALLER_CHECKING_EXTERNS


#endif /* FEATURE_NX_IPV6 */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_nd_cache_hardware_address_find                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking on the service for finding a  */
/*    neighbor discovery (ND) cache entry service.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    ip_address                            Pointer to the IP address to  */
/*                                            search for                  */
/*    physical_msw                          Physical address, most        */
/*                                            signifcant word             */
/*    physical_lsw                          Physical address, least       */
/*                                            signifcant word             */
/*    interface_index                       Pointer to interface through  */
/*                                            which the neighbor can be   */
/*                                            reached.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_nd_cache_hardware_address_find  Find entry by IP address       */
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
UINT  _nxde_nd_cache_hardware_address_find(NX_IP *ip_ptr,
                                           NXD_ADDRESS *ip_address,
                                           ULONG *physical_msw,
                                           ULONG *physical_lsw,
                                           UINT *interface_index)
{
#ifdef FEATURE_NX_IPV6

    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid IP and MAC address input*/
    if ((ip_address == NX_NULL) || (physical_msw == NX_NULL) || (physical_lsw == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check the address is an IPv6 address. */
    if (ip_address -> nxd_ip_version != NX_IP_VERSION_V6)
    {
        return(NX_INVALID_PARAMETERS);
    }

    if (interface_index == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service and return completion status. */
    return(_nxd_nd_cache_hardware_address_find(ip_ptr, ip_address, physical_msw, physical_lsw, interface_index));

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(physical_msw);
    NX_PARAMETER_NOT_USED(physical_lsw);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

