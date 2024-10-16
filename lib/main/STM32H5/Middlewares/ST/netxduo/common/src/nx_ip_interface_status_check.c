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



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_interface_status_check                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function polls the specified interface for the link state using*/
/*    thread sleep for the necessary conditions n the IP instance. Where  */
/*    the requested status exists only at the IP instance, for example    */
/*    NX_IP_INITIALIZE_DONE this service supplies the IP setting for that */
/*    status.                                                             */
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
/*    (ip_link_driver)                      User supplied link driver     */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    tx_thread_sleep                       Sleep until events are        */
/*                                            satisfied                   */
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
UINT  _nx_ip_interface_status_check(NX_IP *ip_ptr, UINT interface_index, ULONG needed_status,
                                    ULONG *actual_status, ULONG wait_option)
{

ULONG                  current_status;
NX_IP_DRIVER           driver_request;
ULONG                  return_value;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_STATUS_CHECK, ip_ptr, needed_status, 0, wait_option, NX_TRACE_IP_EVENTS, &trace_event, &trace_timestamp);

    /* Loop to keep checking for the proper status bits.  */
    for (;;)
    {

        /* Clear the current status.  */
        current_status =  0;

        /*  Process according to the status option specified.  */

        if (needed_status & NX_IP_INITIALIZE_DONE)
        {

            /* Check for initialization complete.  */
            if (ip_ptr -> nx_ip_initialize_done)
            {

                /* Yes, set the appropriate bit in the current status.  */
                current_status =  current_status | NX_IP_INITIALIZE_DONE;
            }
        }

#ifndef NX_DISABLE_IPV4
        if (needed_status & NX_IP_ADDRESS_RESOLVED)
        {

            /* Check for a non-zero IP address.  */
            if (ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_address)
            {

                /* Yes, set the appropriate bit in the current status.  */
                current_status =  current_status | NX_IP_ADDRESS_RESOLVED;
            }
        }

        if (needed_status & NX_IP_ARP_ENABLED)
        {

            /* Check for ARP being enabled.  */
            if (ip_ptr -> nx_ip_arp_periodic_update)
            {
                /* Yes, set the appropriate bit in the current status.  */
                current_status =  current_status | NX_IP_ARP_ENABLED;
            }
        }

        if (needed_status & NX_IP_RARP_COMPLETE)
        {

            /* This is effectively the same as the IP address resolved...  */

            /* Check for a non-zero IP address.  */
            if (ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_address)
            {

                /* Yes, set the appropriate bit in the current status.  */
                current_status =  current_status | NX_IP_RARP_COMPLETE;
            }
        }

        if (needed_status & NX_IP_IGMP_ENABLED)
        {

            /* Check for IGMP being enabled.  */
            if (ip_ptr -> nx_ip_igmp_packet_receive)
            {
                /* Yes, set the appropriate bit in the current status.  */
                current_status =  current_status | NX_IP_IGMP_ENABLED;
            }
        }
#endif /* !NX_DISABLE_IPV4  */

        if (needed_status & NX_IP_LINK_ENABLED)
        {

            /* Get mutex protection.  */
            tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

            /* Build the driver request structure.  */
            driver_request.nx_ip_driver_ptr =         ip_ptr;
            driver_request.nx_ip_driver_command =     NX_LINK_GET_STATUS;
            driver_request.nx_ip_driver_return_ptr =  &return_value;
            driver_request.nx_ip_driver_interface  =  &(ip_ptr -> nx_ip_interface[interface_index]);

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_GET_STATUS, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Call link level driver.  */
            (ip_ptr -> nx_ip_interface[interface_index].nx_interface_link_driver_entry)(&driver_request);

            /* If the driver does not recognize this keyword, we fall back to reading the IP link status.*/
            /*lint -e{644} suppress variable might not be initialized, since "nx_ip_driver_status" was initialized in nx_interface_link_driver_entry. */
            if (driver_request.nx_ip_driver_status != NX_SUCCESS)
            {
                if (driver_request.nx_ip_driver_status == NX_UNHANDLED_COMMAND)
                {
                    if (ip_ptr -> nx_ip_interface[interface_index].nx_interface_link_up)
                    {
                        current_status = current_status | NX_IP_LINK_ENABLED;
                    }
                }
            }
            else
            {

                /* Check for a link up condition.  */
                /*lint -e{644} suppress variable might not be initialized, since "return_value" was initialized in nx_interface_link_driver_entry. */
                if (return_value == NX_TRUE)
                {

                    /* Yes, set the appropriate bit in the current status.  */
                    current_status =  current_status | NX_IP_LINK_ENABLED;
                }
            }

            /* Release mutex protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        }

        if (needed_status &  NX_IP_UDP_ENABLED)
        {

            /* Check for UDP being enabled.  */
            if (ip_ptr -> nx_ip_udp_packet_receive)
            {
                /* Yes, set the appropriate bit in the current status.  */
                current_status =  current_status | NX_IP_UDP_ENABLED;
            }
        }

        if (needed_status & NX_IP_TCP_ENABLED)
        {

            /* Check for TCP being enabled.  */
            if (ip_ptr -> nx_ip_tcp_packet_receive)
            {
                /* Yes, set the appropriate bit in the current status.  */
                current_status =  current_status | NX_IP_TCP_ENABLED;
            }
        }

        if (needed_status & NX_IP_INTERFACE_LINK_ENABLED)
        {

            /* Get mutex protection.  */
            tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

            /* Build the driver request structure.  */
            driver_request.nx_ip_driver_ptr =         ip_ptr;
            driver_request.nx_ip_driver_command =     NX_LINK_GET_STATUS;
            driver_request.nx_ip_driver_return_ptr =  &return_value;
            driver_request.nx_ip_driver_interface  =  &(ip_ptr -> nx_ip_interface[interface_index]);

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_GET_STATUS, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Call link level driver.  */
            (ip_ptr -> nx_ip_interface[interface_index].nx_interface_link_driver_entry)(&driver_request);

            /* If the driver does not recognize this keyword, we fall back to reading the IP link status.*/
            /*lint -e{644} suppress variable might not be initialized, since "nx_ip_driver_status" was initialized in nx_interface_link_driver_entry. */
            if (driver_request.nx_ip_driver_status != NX_SUCCESS)
            {
                if (driver_request.nx_ip_driver_status == NX_UNHANDLED_COMMAND)
                {
                    if (ip_ptr -> nx_ip_interface[interface_index].nx_interface_link_up)
                    {
                        current_status = current_status | NX_IP_INTERFACE_LINK_ENABLED;
                    }
                }
            }
            else
            {

                /* Check for a link up condition.  */
                /*lint -e{644} suppress variable might not be initialized, since "return_value" was initialized in nx_interface_link_driver_entry. */
                if (return_value == NX_TRUE)
                {

                    /* Yes, set the appropriate bit in the current status.  */
                    current_status =  current_status | NX_IP_INTERFACE_LINK_ENABLED;
                }
            }

            /* Release mutex protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        }

        /* Determine if current status is the same.  If so, break out
           of this polling loop.  */
        if (current_status == needed_status)
        {
            break;
        }

        /* Check for suspension request.  */
        if (wait_option)
        {

            /* Decrease the wait time and sleep.  */
            if (wait_option > NX_IP_STATUS_CHECK_WAIT_TIME)
            {
                wait_option -= NX_IP_STATUS_CHECK_WAIT_TIME;
            }
            else
            {
                wait_option = 0;
            }

            /* Sleep for a tick and check again.  */
            tx_thread_sleep(NX_IP_STATUS_CHECK_WAIT_TIME);
        }
        else
        {

            /* Get out of the loop.  */
            break;
        }
    }

    /* Place the current status in the return destination.  */
    *actual_status =  current_status;

    /* Update the trace event with the status.  */
    NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_IP_STATUS_CHECK, 0, 0, current_status, 0);

    /* Determine what status to return.  */
    if (needed_status == current_status)
    {

        /* Return a success.  */
        return(NX_SUCCESS);
    }
    else
    {

        /* Return an error.  */
        return(NX_NOT_SUCCESSFUL);
    }
}

