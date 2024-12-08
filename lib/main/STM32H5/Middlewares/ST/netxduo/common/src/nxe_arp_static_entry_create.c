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
/**   Address Resolution Protocol (ARP)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_arp.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_arp_static_entry_create                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the ARP static entry create      */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    ip_address                            IP Address to bind to         */
/*    physical_msw                          Physical address MSW          */
/*    physical_lsw                          Physical address LSW          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_arp_static_entry_create          Actual static entry create     */
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
UINT  _nxe_arp_static_entry_create(NX_IP *ip_ptr, ULONG ip_address,
                                   ULONG physical_msw, ULONG physical_lsw)
{

#ifndef NX_DISABLE_IPV4
UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid IP address.  */
    if (!ip_address)
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Determine if the IP address is multicast or directed broadcast.  */
    if (((ip_address & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE) ||
        (ip_address  == NX_IP_LIMITED_BROADCAST))
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Check for invalid physical address.  */
    if ((physical_msw | physical_lsw) == 0)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Check to see if ARP is enabled.  */
    if (!ip_ptr -> nx_ip_arp_allocate)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual ARP static entry create function.  */
    status =  _nx_arp_static_entry_create(ip_ptr, ip_address, physical_msw, physical_lsw);

    /* Return completion status.  */
    return(status);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(physical_msw);
    NX_PARAMETER_NOT_USED(physical_lsw);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

