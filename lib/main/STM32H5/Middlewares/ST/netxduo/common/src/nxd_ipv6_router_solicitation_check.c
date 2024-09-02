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
#include "nx_nd_cache.h"
#include "nx_icmpv6.h"


#ifdef FEATURE_NX_IPV6

#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_router_solicitation_check                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    At every time tick, this function decrement the router solicitation */
/*    counter.   When the counter reaches zero, the stack sends out       */
/*    router solicitation.                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    router_address                        The specific gateway address  */
/*                                            to search for.              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_icmpv6_send_rs                   Send router solicitation packet*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_ip_thread_entry                    Handle IP thread task events  */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    Caller must obtain nx_ip_protection mutex before calling this       */
/*    function.                                                           */
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
void _nxd_ipv6_router_solicitation_check(NX_IP *ip_ptr)
{
UINT i;

    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {
        if (ip_ptr -> nx_ip_interface[i].nx_interface_valid == NX_TRUE)
        {

            /* Check if max number of router solicitation messages have been sent. */
            if (ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_count != 0)
            {

                /* Check on count down timer for sending out next router solicitation message. */
                ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_timer--;
                if (ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_timer == 0)
                {
                    if (_nx_icmpv6_send_rs(ip_ptr, i) &&
                        (ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_count ==
                         ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_max))
                    {

                        /* Initial RS is not sent successfully. */
                        /* Try it next round. */
                        ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_timer = 1;
                    }
                    else
                    {
                        ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_count--;
                        ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_timer = ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_interval;
                    }
                }
            }
        }
    }
}
#endif /* NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */
#endif /* FEATURE_NX_IPV6 */

