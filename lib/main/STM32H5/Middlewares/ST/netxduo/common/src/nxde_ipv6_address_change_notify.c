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
#if defined(NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY) && defined(FEATURE_NX_IPV6)
#include "nx_ip.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY && FEATURE_NX_IPV6 */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_ipv6_address_change_notify                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the IPv6 address change notify   */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    ip_address_change_notify              Application callback function */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_ip_address_change_notify         Actual IP address change      */
/*                                            notify function             */
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
UINT  _nxde_ipv6_address_change_notify(NX_IP *ip_ptr, VOID (*ip_address_change_notify)(NX_IP *ip_ptr, UINT status, UINT interface_index, UINT address_index, ULONG *ip_address))
{
#if defined(NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY) && defined(FEATURE_NX_IPV6)
UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual IP address change notify function.  */
    status =  _nxd_ipv6_address_change_notify(ip_ptr, ip_address_change_notify);

    /* Return completion status.  */
    return(status);
#else

    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address_change_notify);

    return(NX_NOT_ENABLED);
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY && FEATURE_NX_IPV6 */
}

