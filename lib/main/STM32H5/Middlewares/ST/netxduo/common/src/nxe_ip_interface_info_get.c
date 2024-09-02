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

/* Bring in externs for caller checking code */
NX_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ip_interface_info_get                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking for nx_ip_interface_info_get  */
/*    service call.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    interface_index                       Index to the interface        */
/*    interface_name                        Name of the interface         */
/*    ip_address                            Pointer to IP address         */
/*                                            destination in host byte    */
/*                                            order                       */
/*    network_mask                          Pointer to network mask       */
/*                                            destination, in host byte   */
/*                                            order                       */
/*    mtu_size                              Pointer to storage space for  */
/*                                            MTU                         */
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
/*    _nx_ip_interface_info_get             Actual IP interface info get  */
/*                                            service call.               */
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

UINT _nxe_ip_interface_info_get(NX_IP *ip_ptr, UINT interface_index, CHAR **interface_name,
                                ULONG *ip_address, ULONG *network_mask, ULONG *mtu_size,
                                ULONG *physical_address_msw, ULONG *physical_address_lsw)
{
UINT status;

    if (ip_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (ip_ptr -> nx_ip_id != NX_IP_ID)
    {
        return(NX_PTR_ERROR);
    }

    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return(NX_INVALID_INTERFACE);
    }

    NX_INIT_AND_THREADS_CALLER_CHECKING

    status = _nx_ip_interface_info_get(ip_ptr, interface_index, interface_name, ip_address, network_mask,
                                       mtu_size, physical_address_msw, physical_address_lsw);

    return(status);
}

