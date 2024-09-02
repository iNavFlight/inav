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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_icmp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_info_get                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the selected ICMP information for the       */
/*    caller.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    pings_sent                            Destination for number of     */
/*                                            pings sent                  */
/*    ping_timeouts                         Destination for number of     */
/*                                            ping timeouts               */
/*    ping_threads_suspended                Destination for number of     */
/*                                            threads suspended on pings  */
/*    ping_responses_received               Destination for number of     */
/*                                            ping responses received     */
/*    icmp_checksum_errors                  Destination for number of     */
/*                                            ICMP checksum errors        */
/*    icmp_unhandled_messages               Destination for number of     */
/*                                            unhandled ICMP messages     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
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
UINT  _nx_icmp_info_get(NX_IP *ip_ptr, ULONG *pings_sent, ULONG *ping_timeouts,
                        ULONG *ping_threads_suspended, ULONG *ping_responses_received,
                        ULONG *icmp_checksum_errors, ULONG *icmp_unhandled_messages)
{

    /* Obtain protection on this IP instance.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if pings sent is wanted.  */
    if (pings_sent)
    {

        /* Return the number of pings sent by this IP instance.  */
        *pings_sent =  ip_ptr -> nx_ip_pings_sent;
    }

    /* Determine if ping timeouts is wanted.  */
    if (ping_timeouts)
    {

        /* Return the number of ping timeouts by this IP instance.  */
        *ping_timeouts =  ip_ptr -> nx_ip_ping_timeouts;
    }

    /* Determine if ping threads suspended is wanted.  */
    if (ping_threads_suspended)
    {

        /* Return the number of ping threads suspended by this IP instance.  */
        *ping_threads_suspended =  ip_ptr -> nx_ip_ping_threads_suspended;
    }

    /* Determine if ping responses received is wanted.  */
    if (ping_responses_received)
    {

        /* Return the number of ping responses received by this IP instance.  */
        *ping_responses_received =  ip_ptr -> nx_ip_ping_responses_received;
    }

    /* Determine if ICMP checksum errors is wanted.  */
    if (icmp_checksum_errors)
    {

        /* Return the number of ICMP checksum errors detected by this IP instance.  */
        *icmp_checksum_errors =  ip_ptr -> nx_ip_icmp_checksum_errors;
    }

    /* Determine if ICMP unhandled messages is wanted.  */
    if (icmp_unhandled_messages)
    {

        /* Return the number of ICMP unhandled messages by this IP instance.  */
        *icmp_unhandled_messages =  ip_ptr -> nx_ip_icmp_unhandled_messages;
    }

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}

