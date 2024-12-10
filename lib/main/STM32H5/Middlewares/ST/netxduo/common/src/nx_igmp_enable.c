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
/**   Internet Group Management Protocol (IGMP)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_igmp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_igmp_enable                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the IGMP management component for the         */
/*    specified IP instance.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set deferred IGMP enable      */
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
UINT  _nx_igmp_enable(NX_IP *ip_ptr)
{

#ifndef NX_DISABLE_IPV4


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IGMP_ENABLE, ip_ptr, 0, 0, 0, NX_TRACE_IGMP_EVENTS, 0, 0);


#ifndef NX_DISABLE_IGMPV2

    /* Set the IGMP protocol to the default version supported by NetX, IGMPv2. */

    ip_ptr -> nx_ip_igmp_router_version = NX_IGMP_HOST_VERSION_2;

#endif

    /* Setup the IGMP packet receive routine, which enables the IGMP component.  */
    ip_ptr -> nx_ip_igmp_packet_receive =  _nx_igmp_packet_receive;

    /* Setup the IGMP periodic processing routine, which enables the IGMP component.  */
    ip_ptr -> nx_ip_igmp_periodic_processing =  _nx_igmp_periodic_processing;

    /* Setup the IGMP queue processing routine, which processes deferred IGMP
       requests.  */
    ip_ptr -> nx_ip_igmp_queue_process =  _nx_igmp_queue_process;

    /* Wakeup IP helper thread to process the IGMP deferred enable.  */
    tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_IGMP_ENABLE_EVENT, TX_OR);

    /* Return a successful status!  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

