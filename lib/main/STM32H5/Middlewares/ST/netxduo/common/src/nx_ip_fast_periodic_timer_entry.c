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
#include "tx_timer.h"



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_fast_periodic_timer_entry                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles waking up the IP helper thread on a periodic  */
/*    basis for higher-frequency IPv6/TCP events.  This timer is enabled  */
/*    when IPv6 or TCP is enabled.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_address                            IP address in a ULONG         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set event flags to wakeup     */
/*                                            IP helper thread            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX system timer thread                                         */
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
static VOID  _nx_ip_fast_periodic_timer_entry(ULONG ip_address)
{

NX_IP *ip_ptr;


    /* Setup IP pointer.  */
    NX_TIMER_EXTENSION_PTR_GET(ip_ptr, NX_IP, ip_address)

    /* Wakeup this IP's helper thread.  */
    tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_FAST_EVENT, TX_OR);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_fast_periodic_timer_create                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a high resolution timer for driving IPv6 and  */
/*    TCP time-keeping events.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer into IP instance.     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_timer_create                       Set event flags to wakeup     */
/*                                            IP helper thread            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_ipv6_enable                                                      */
/*    nx_tcp_enable                                                       */
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
VOID _nx_ip_fast_periodic_timer_create(NX_IP *ip_ptr)
{

ULONG _nx_ip_fast_timer_rate;

    if (ip_ptr -> nx_ip_fast_periodic_timer_created)
    {
        return;
    }

    _nx_ip_fast_timer_rate =  (NX_IP_PERIODIC_RATE + (NX_IP_FAST_TIMER_RATE - 1)) / NX_IP_FAST_TIMER_RATE;

    /* Create the fast TCP timer.  */
    /*lint -e{923} suppress cast of pointer to ULONG.  */
    tx_timer_create(&(ip_ptr -> nx_ip_fast_periodic_timer), ip_ptr -> nx_ip_name,
                    _nx_ip_fast_periodic_timer_entry, (ULONG)(ALIGN_TYPE)ip_ptr,
                    _nx_ip_fast_timer_rate, _nx_ip_fast_timer_rate, TX_AUTO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(ip_ptr -> nx_ip_fast_periodic_timer), ip_ptr)

    /* Set the flag to indicate that the fast timer has been created. */
    ip_ptr -> nx_ip_fast_periodic_timer_created = 1;
}

