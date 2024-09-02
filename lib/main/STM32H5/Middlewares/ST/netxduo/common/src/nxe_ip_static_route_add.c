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
#include "nx_ip.h"

#if !defined(NX_DISABLE_IPV4) && defined(NX_ENABLE_IP_STATIC_ROUTING)
/* Bring in externs for caller checking code.  */
NX_CALLER_CHECKING_EXTERNS
#endif /* !NX_DISABLE_IPV4 && NX_ENABLE_IP_STATIC_ROUTING */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ip_static_route_add                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deltes static routing entry from the routing table.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    network_address                       network address, in network   */
/*                                            byte order.                 */
/*    net_mask                              Network Mask, in network      */
/*                                            byte order.                 */
/*    next_hop                              Next Hop address, in network  */
/*                                            byte order.                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_staic_route_add                Actual IP static route        */
/*                                            add function.               */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  NOTE:                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _nxe_ip_static_route_add(NX_IP *ip_ptr, ULONG network_address,
                               ULONG net_mask, ULONG next_hop)
{
#if !defined(NX_DISABLE_IPV4) && defined(NX_ENABLE_IP_STATIC_ROUTING)

UINT status;

    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual IP forwarding disable function.  */
    status =  _nx_ip_static_route_add(ip_ptr, network_address, net_mask, next_hop);

    /* Return completion status.  */
    return(status);

#else /* !NX_DISABLE_IPV4 && NX_ENABLE_IP_STATIC_ROUTING */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(network_address);
    NX_PARAMETER_NOT_USED(net_mask);
    NX_PARAMETER_NOT_USED(next_hop);

    return(NX_NOT_SUPPORTED);

#endif /* !NX_DISABLE_IPV4 && NX_ENABLE_IP_STATIC_ROUTING */
}

