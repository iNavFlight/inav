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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_interface_info_get                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves information related to a specified          */
/*    interface.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    interface_index                       Index to the interface        */
/*    interface_name                        Name of the interface         */
/*    ip_address                            Pointer to interface IP       */
/*                                            address in host byte order  */
/*    network_mask                          Pointer to network mask       */
/*                                            destination, in host        */
/*                                            byte order                  */
/*    mtu_size                              Pointer to storage space      */
/*                                            for MTU                     */
/*    physical_address_msw                  Pointer to storage space for  */
/*                                            device phsyical address, MSW*/
/*    physical_address_lsw                  Pointer to storage space for  */
/*                                            device phsyical address, LSW*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT _nx_ip_interface_info_get(NX_IP *ip_ptr, UINT interface_index, CHAR **interface_name,
                               ULONG *ip_address, ULONG *network_mask, ULONG *mtu_size,
                               ULONG *physical_address_msw, ULONG *physical_address_lsw)
{
NX_INTERFACE *nx_interface;

#ifdef NX_DISABLE_IPV4
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(network_mask);
#endif /* NX_DISABLE_IPV4 */

    nx_interface = &(ip_ptr -> nx_ip_interface[interface_index]);

    if (!nx_interface -> nx_interface_valid)
    {
        return(NX_INVALID_INTERFACE);
    }

    if (interface_name)
    {
        *interface_name = nx_interface -> nx_interface_name;
    }

#ifndef NX_DISABLE_IPV4
    if (ip_address)
    {
        *ip_address = nx_interface -> nx_interface_ip_address;
    }

    if (network_mask)
    {
        *network_mask = nx_interface -> nx_interface_ip_network_mask;
    }
#endif /* !NX_DISABLE_IPV4  */

    if (mtu_size)
    {
        *mtu_size = nx_interface -> nx_interface_ip_mtu_size;
    }

    if (physical_address_msw)
    {
        *physical_address_msw = nx_interface -> nx_interface_physical_address_msw;
    }

    if (physical_address_lsw)
    {
        *physical_address_lsw = nx_interface -> nx_interface_physical_address_lsw;
    }

#ifndef NX_DISABLE_IPV4
    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_INTERFACE_INFO_GET, ip_ptr, nx_interface -> nx_interface_ip_address,
                            nx_interface -> nx_interface_physical_address_msw, nx_interface -> nx_interface_physical_address_lsw,
                            NX_TRACE_IP_EVENTS, 0, 0);
#else
    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_INTERFACE_INFO_GET, ip_ptr, 0,
                            nx_interface -> nx_interface_physical_address_msw, nx_interface -> nx_interface_physical_address_lsw,
                            NX_TRACE_IP_EVENTS, 0, 0);
#endif /* NX_DISABLE_IPV4 */


    return(NX_SUCCESS);
}

