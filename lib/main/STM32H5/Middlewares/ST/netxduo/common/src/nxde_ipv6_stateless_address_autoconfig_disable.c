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
#if defined(FEATURE_NX_IPV6) && defined(NX_IPV6_STATELESS_AUTOCONFIG_CONTROL)
#include "nx_ip.h"

/* Bring in externs for caller checking code.  */
NX_CALLER_CHECKING_EXTERNS

#endif /* FEATURE_NX_IPV6 && NX_IPV6_STATELESS_AUTOCONFIG_CONTROL */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_ipv6_stateless_address_autoconfig_disable     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking on the IPv6 stateless address */
/*    auto configuratoin disable service                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    interface_index                       The interface to operate on   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nxd_ipv6_stateless_address_autoconfig_disable                       */
/*                                                                        */
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
UINT  _nxde_ipv6_stateless_address_autoconfig_disable(NX_IP *ip_ptr, UINT interface_index)
{

#if defined(FEATURE_NX_IPV6) && defined(NX_IPV6_STATELESS_AUTOCONFIG_CONTROL)
UINT status;

    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Validate the interface. */
    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual IPv6 stateless address autoconfig disable function.  */
    status =  _nxd_ipv6_stateless_address_autoconfig_disable(ip_ptr, interface_index);

    /* Return completion status.  */
    return(status);

#else
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 && NX_IPV6_STATELESS_AUTOCONFIG_CONTROL */
}

