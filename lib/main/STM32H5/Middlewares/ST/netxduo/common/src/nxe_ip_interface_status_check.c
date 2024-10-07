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

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ip_interface_status_check                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the IP interface status check    */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    interface_index                       Index to the interface        */
/*    needed_status                         Status needed request         */
/*    actual_status                         Pointer to return status area */
/*    wait_option                           Maximum suspension time       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_interface_status_check         Actual IP interface status    */
/*                                             check function             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT  _nxe_ip_interface_status_check(NX_IP *ip_ptr, UINT interface_index, ULONG needed_status,
                                     ULONG *actual_status, ULONG wait_option)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (actual_status == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid interface index. */
    if ((interface_index >= NX_MAX_PHYSICAL_INTERFACES) ||
        (ip_ptr -> nx_ip_interface[interface_index].nx_interface_valid) == 0)
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for valid options.  */
    if (needed_status &
        ~((NX_IP_INITIALIZE_DONE | NX_IP_LINK_ENABLED | NX_IP_UDP_ENABLED | NX_IP_TCP_ENABLED | NX_IP_INTERFACE_LINK_ENABLED)
#ifndef NX_DISABLE_IPV4
          | (NX_IP_ADDRESS_RESOLVED | NX_IP_ARP_ENABLED | NX_IP_RARP_COMPLETE | NX_IP_IGMP_ENABLED)
#endif /* !NX_DISABLE_IPV4  */
         ))
    {
        return(NX_OPTION_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual IP interface status check function.  */
    status =  _nx_ip_interface_status_check(ip_ptr, interface_index, needed_status, actual_status, wait_option);

    /* Return completion status.  */
    return(status);
}

