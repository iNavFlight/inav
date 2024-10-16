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
/*    _nxde_ipv6_default_router_number_of_entries_get      PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking for the default router entries*/
/*    get service.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    if_index                              Interface Index               */
/*    num_entries                           Destination for number of     */
/*                                            IPv6 routers on a specified */
/*                                            network interface           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_ipv6_default_router_entries_get                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*                                                                        */
/*    This function cannot be called from ISR.                            */
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
UINT  _nxde_ipv6_default_router_number_of_entries_get(NX_IP *ip_ptr, UINT if_index, UINT *num_entries)
{
#ifdef FEATURE_NX_IPV6

UINT status;


    /* Check for invalid input pointers. */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (num_entries == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Validate the interface. */
    if (if_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual IPv6 default router number get function.  */
    status = _nxd_ipv6_default_router_number_of_entries_get(ip_ptr, if_index, num_entries);

    /* Return completion status.  */
    return(status);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(if_index);
    NX_PARAMETER_NOT_USED(num_entries);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

