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
/*    _nxe_arp_info_get                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the ARP information get          */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    arp_requests_sent                     Destination for number of ARP */
/*                                            requests sent               */
/*    arp_requests_received                 Destination for number of ARP */
/*                                            requests received           */
/*    arp_responses_sent                    Destination for number of ARP */
/*                                            responses sent              */
/*    arp_responses_received                Destination for number of ARP */
/*                                            responses received          */
/*    arp_dynamic_entries                   Destination for number of ARP */
/*                                            dynamic entries             */
/*    arp_static_entries                    Destination for number of ARP */
/*                                            static entries              */
/*    arp_aged_entries                      Destination for number of ARP */
/*                                            aged entries                */
/*    arp_invalid_messages                  Destination for number of ARP */
/*                                            invalid messages            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_arp_info_get                      Actual ARP information get    */
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
UINT  _nxe_arp_info_get(NX_IP *ip_ptr, ULONG *arp_requests_sent, ULONG *arp_requests_received,
                        ULONG *arp_responses_sent, ULONG *arp_responses_received,
                        ULONG *arp_dynamic_entries, ULONG *arp_static_entries,
                        ULONG *arp_aged_entries, ULONG *arp_invalid_messages)
{

#ifndef NX_DISABLE_IPV4
UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check to see if ARP is enabled.  */
    if (!ip_ptr -> nx_ip_arp_allocate)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual ARP information get function.  */
    status =  _nx_arp_info_get(ip_ptr, arp_requests_sent, arp_requests_received,
                               arp_responses_sent, arp_responses_received,
                               arp_dynamic_entries, arp_static_entries,
                               arp_aged_entries, arp_invalid_messages);

    /* Return completion status.  */
    return(status);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(arp_requests_sent);
    NX_PARAMETER_NOT_USED(arp_requests_received);
    NX_PARAMETER_NOT_USED(arp_responses_sent);
    NX_PARAMETER_NOT_USED(arp_responses_received);
    NX_PARAMETER_NOT_USED(arp_dynamic_entries);
    NX_PARAMETER_NOT_USED(arp_static_entries);
    NX_PARAMETER_NOT_USED(arp_aged_entries);
    NX_PARAMETER_NOT_USED(arp_invalid_messages);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

