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
#include "nx_igmp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_igmp_info_get                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves various IGMP information associated with    */
/*    the specified IP instance.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    igmp_reports_sent                     Destination for the number    */
/*                                            of IGMP reports sent        */
/*    igmp_queries_received                 Destination for the number    */
/*                                            of IGMP queries received    */
/*    igmp_checksum_errors                  Destination for the number    */
/*                                            of IGMP checksum errors     */
/*    current_groups_joined                 Destination for the number    */
/*                                            of IGMP multicast groups    */
/*                                            currently joined            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
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
UINT  _nx_igmp_info_get(NX_IP *ip_ptr, ULONG *igmp_reports_sent, ULONG *igmp_queries_received,
                        ULONG *igmp_checksum_errors, ULONG *current_groups_joined)
{

#ifndef NX_DISABLE_IPV4


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IGMP_INFO_GET, ip_ptr, ip_ptr -> nx_ip_igmp_reports_sent, ip_ptr -> nx_ip_igmp_queries_received, ip_ptr -> nx_ip_igmp_groups_joined, NX_TRACE_IGMP_EVENTS, 0, 0);

    /* Obtain protection on this IP instance.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if IGMP reports sent is wanted.  */
    if (igmp_reports_sent)
    {

        /* Return the number of IGMP reports sent by this IP instance.  */
        *igmp_reports_sent =  ip_ptr -> nx_ip_igmp_reports_sent;
    }

    /* Determine if IGMP queries received is wanted.  */
    if (igmp_queries_received)
    {

        /* Return the number of IGMP queries received by this IP instance.  */
        *igmp_queries_received =  ip_ptr -> nx_ip_igmp_queries_received;
    }

    /* Determine if IGMP checksum errors is wanted.  */
    if (igmp_checksum_errors)
    {

        /* Return the number of IGMP checksum errors by this IP instance.  */
        *igmp_checksum_errors =  ip_ptr -> nx_ip_igmp_checksum_errors;
    }

    /* Determine if the number of IGMP multicast groups joined is wanted.  */
    if (current_groups_joined)
    {

        /* Return the number of IGMP multicast groups joined is wanted.  */
        *current_groups_joined =  ip_ptr -> nx_ip_igmp_groups_joined;
    }

    /* Release the protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status!  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(igmp_reports_sent);
    NX_PARAMETER_NOT_USED(igmp_queries_received);
    NX_PARAMETER_NOT_USED(igmp_checksum_errors);
    NX_PARAMETER_NOT_USED(current_groups_joined);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

