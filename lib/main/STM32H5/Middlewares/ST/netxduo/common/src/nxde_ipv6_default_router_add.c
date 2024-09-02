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
/**   Internet Protocol version 6 Default Router Table (IPv6 router)      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ip.h"
#include "nx_nd_cache.h"

/* Bring in externs for caller checking code.  */
NX_CALLER_CHECKING_EXTERNS

#endif /* FEATURE_NX_IPV6 */



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_default_router_add                         PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking for the default router add    */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    router_addr                           Destination Network addr      */
/*    router_lifetime                       Life time information         */
/*    interface_index                       Index to the interface        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*    NX_INVALID_PARAMETERS                 Invalid IP version            */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid Interface Index       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_ipv6_default_router_add                                        */
/*                                          Actual function that adds     */
/*                                          an entry to the router table  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
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
UINT  _nxde_ipv6_default_router_add(NX_IP *ip_ptr, NXD_ADDRESS *router_addr,
                                    ULONG router_lifetime, UINT interface_index)


{
#ifdef FEATURE_NX_IPV6
    /* Check for invalid input pointers. */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (router_addr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for valid IP version. */
    if (router_addr -> nxd_ip_version != NX_IP_VERSION_V6)
    {
        return(NX_INVALID_PARAMETERS);
    }

    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Make sure the interface is valid. */
    if (ip_ptr -> nx_ip_interface[interface_index].nx_interface_valid != NX_TRUE)
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call the actual service and return completion status. */
    return(_nxd_ipv6_default_router_add(ip_ptr, router_addr, router_lifetime, interface_index));

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(router_addr);
    NX_PARAMETER_NOT_USED(router_lifetime);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

