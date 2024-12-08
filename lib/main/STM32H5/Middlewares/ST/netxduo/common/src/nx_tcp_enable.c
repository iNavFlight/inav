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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_tcp.h"
#include "nx_ip.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_enable                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the TCP management component for the          */
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
/*    tx_timer_create                       Create fast TCP timer         */
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
UINT  _nx_tcp_enable(NX_IP *ip_ptr)
{

UINT                         i;
struct NX_TCP_LISTEN_STRUCT *listen_ptr;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_ENABLE, ip_ptr, 0, 0, 0, NX_TRACE_TCP_EVENTS, 0, 0);
    /* Place all server listen request structures on the available list.   */

    /* Setup a pointer to the first listen.  */
    listen_ptr =  &(ip_ptr -> nx_ip_tcp_server_listen_reqs[0]);

    /* Setup the available listen requests head pointer.  */
    ip_ptr -> nx_ip_tcp_available_listen_requests =  listen_ptr;

    /* Loop through the listen requests and link them on the available list.  */
    for (i = 0; i < NX_MAX_LISTEN_REQUESTS; i++)
    {

        /* Link listen request to next listen request.  */
        listen_ptr -> nx_tcp_listen_next =  listen_ptr + 1;

        /* Determine if we need to move to the next listen request.  */
        if (i < (NX_MAX_LISTEN_REQUESTS - 1))
        {
            listen_ptr++;
        }
    }

    /* Make sure the last listen request has a NULL pointer.  */
    listen_ptr -> nx_tcp_listen_next =  NX_NULL;

    /* Set the TCP packet queue processing function.  */
    ip_ptr -> nx_ip_tcp_queue_process =  _nx_tcp_queue_process;

    /* Set the TCP periodic processing function.  */
    ip_ptr -> nx_ip_tcp_periodic_processing =  _nx_tcp_periodic_processing;

    /* Set the TCP fast periodic processing function.  */
    ip_ptr -> nx_ip_tcp_fast_periodic_processing =  _nx_tcp_fast_periodic_processing;

    /* Set the TCP deferred cleanup check function.  */
    ip_ptr -> nx_tcp_deferred_cleanup_check =  _nx_tcp_deferred_cleanup_check;

    /* Setup base timer variables.  */
    _nx_tcp_fast_timer_rate =       (NX_IP_PERIODIC_RATE + (NX_TCP_FAST_TIMER_RATE - 1)) / NX_TCP_FAST_TIMER_RATE;
    _nx_tcp_ack_timer_rate =        (NX_IP_PERIODIC_RATE + (NX_TCP_ACK_TIMER_RATE - 1)) / NX_TCP_ACK_TIMER_RATE;

    /*lint -e{778} suppress constant expression, since NX_TCP_TRANSMIT_TIMER_RATE can be redefined. */
    /*lint -e{835} -e{845} suppress operating on zero. */
    _nx_tcp_transmit_timer_rate =   (NX_IP_PERIODIC_RATE + (NX_TCP_TRANSMIT_TIMER_RATE - 1)) / NX_TCP_TRANSMIT_TIMER_RATE;

    _nx_tcp_2MSL_timer_rate = 2 * NX_IP_PERIODIC_RATE * NX_TCP_MAXIMUM_SEGMENT_LIFETIME;

    _nx_ip_fast_periodic_timer_create(ip_ptr);

    /* Set the TCP packet receive function in the IP structure to indicate
       we are ready to receive TCP packets.  */
    ip_ptr -> nx_ip_tcp_packet_receive =  _nx_tcp_packet_receive;

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

