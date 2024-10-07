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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_free_port_find                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the first available TCP port, starting from the */
/*    supplied port.  If no available ports are found, an error is        */
/*    returned.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    port                                  Starting port                 */
/*    free_port_ptr                         Pointer to return free port   */
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
UINT  _nx_tcp_free_port_find(NX_IP *ip_ptr, UINT port, UINT *free_port_ptr)
{

UINT                   index;
UINT                   bound;
UINT                   starting_port;
NX_TCP_SOCKET         *search_ptr;
NX_TCP_SOCKET         *end_ptr;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_FREE_PORT_FIND, ip_ptr, port, 0, 0, NX_TRACE_TCP_EVENTS, &trace_event, &trace_timestamp);

    /* Save the original port.  */
    starting_port =  port;

    /* Loop through the TCP ports until a free entry is found.  */
    do
    {

        /* Calculate the hash index in the TCP port array of the associated IP instance.  */
        index =  (UINT)((port + (port >> 8)) & NX_TCP_PORT_TABLE_MASK);

        /* Obtain the IP mutex so we can figure out whether or not the port has already
           been bound to.  */
        tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

        /* Pickup the head of the TCP ports bound list.  */
        search_ptr =  ip_ptr -> nx_ip_tcp_port_table[index];

        /* Set the bound flag to false.  */
        bound =  NX_FALSE;

        /* Determine if we need to perform a list search.  */
        if (search_ptr)
        {

            /* Walk through the circular list of TCP sockets that are already
               bound.  */
            end_ptr =     search_ptr;
            do
            {

                /* Determine if this entry is the same as the requested port.  */
                if (search_ptr -> nx_tcp_socket_port == port)
                {

                    /* Set the bound flag.  */
                    bound =  NX_TRUE;

                    /* Get out of the loop.  */
                    break;
                }

                /* Move to the next entry in the list.  */
                search_ptr =  search_ptr -> nx_tcp_socket_bound_next;
            } while (search_ptr != end_ptr);
        }

#ifdef NX_NAT_ENABLE
        if (bound == NX_FALSE)
        {

            /* Check if this IP interface has a NAT service. */
            if (ip_ptr -> nx_ip_nat_port_verify)
            {

                /* Yes, so check the port by NAT handler. If NAT does not use this port, allow NetX to use it.  */
                bound = (ip_ptr -> nx_ip_nat_port_verify)(ip_ptr, NX_PROTOCOL_TCP, port);
            }
        }
#endif

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Determine if the port is available.  */
        if (!bound)
        {

            /* Setup the return port number.  */
            *free_port_ptr =  port;

            /* Update the trace event with the status.  */
            NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_TCP_FREE_PORT_FIND, 0, 0, port, 0);

            /* Return success.  */
            return(NX_SUCCESS);
        }

        /* Move to the next port.  */
        port++;

        /* Determine if we need to wrap.  */
        if (port > NX_MAX_PORT)
        {

            /* Yes, we need to wrap around.  */
            port =  NX_SEARCH_PORT_START;
        }
    } while (starting_port != port);

    /* A free port was not found, return an error.  */
    return(NX_NO_FREE_PORTS);
}

