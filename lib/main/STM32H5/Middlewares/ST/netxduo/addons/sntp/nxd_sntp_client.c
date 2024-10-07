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
/** NetX SNTP Client Component                                            */
/**                                                                       */
/**   Simple Network Time Protocol (SNTP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nxd_sntp_client.c                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Simple Network Time Protocol (SNTP)      */
/*    Client component, including all data types and external references. */
/*    It is assumed that tx_api.h, tx_port.h, nx_api.h, and nx_port.h,    */
/*    have already been included.                                         */
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
               
#define NX_SNTP_SOURCE_CODE

/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

#include "nx_api.h"
#include "nx_udp.h" 

#include "nx_ipv4.h"
#include "nxd_sntp_client.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif


#include <ctype.h> 

extern  TX_THREAD           *_tx_thread_current_ptr; 
extern  TX_THREAD           _tx_timer_thread; 
extern  volatile ULONG      _tx_thread_system_state;

TX_EVENT_FLAGS_GROUP        nx_sntp_client_events;

/* Define internal time variables for offsets between
   receipt of SNTP packet and application to 
   SNTP Client local time. */
static ULONG send_timerticks = 0;
static ULONG receive_timerticks = 0;
static ULONG process_timerticks = 0;


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_create                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the SNTP client create     */ 
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    ip_ptr                            Pointer to client IP instance     */ 
/*    iface_index                       Index of SNTP network interface   */
/*    packet_pool_ptr                   Pointer to client packet pool     */
/*    socket_ptr                        Pointer to client UDP socket      */
/*    leap_second_handler               Pointer to leap second handler    */
/*    random_number_generator           Random number generator callback  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    NX_INVALID_INTERFACE               Invalid network interface        */
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_create            Actual SNTP client create service */ 
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
UINT  _nxe_sntp_client_create(NX_SNTP_CLIENT *client_ptr, NX_IP *ip_ptr, UINT iface_index, NX_PACKET_POOL *packet_pool_ptr,   
                                UINT (*leap_second_handler)(NX_SNTP_CLIENT *client_ptr, UINT indicator), 
                                UINT (*kiss_of_death_handler)(NX_SNTP_CLIENT *client_ptr, UINT code),
                                VOID (random_number_generator)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, ULONG *rand))
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (client_ptr == NX_NULL) || (packet_pool_ptr == NX_NULL))
    {

        /* Return error status.  */
       return(NX_PTR_ERROR);
    }

    /* Check for invalid network interface input. */
    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {

        return NX_INVALID_INTERFACE;
    }

    /* Call the actual client create service.  */
    status = _nx_sntp_client_create(client_ptr, ip_ptr, iface_index, packet_pool_ptr,   
                                    leap_second_handler,  kiss_of_death_handler,  random_number_generator);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_create                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a SNTP client with the input NetX and SNTP    */
/*    parameters. Note the host application must initialize and start the */
/*    SNTP client using the nx_sntp_broadcast/unicast_initialize and      */
/*    nx_sntp_run_broadcast/unicast services to receive SNTP time updates.*/ 
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    ip_ptr                            Pointer to client IP instance     */ 
/*    iface_index                       Index of SNTP network interface   */
/*    packet_pool_ptr                   Pointer to client packet pool     */
/*    socket_ptr                        Pointer to client UDP socket      */
/*    leap_second_handler               Pointer to leap second handler    */
/*    random_number_generator           Random number generator callback  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_SNTP_INSUFFICIENT_PACKET_PAYLOAD                                 */
/*                                      Invalid packet pool payload       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_timer_create                  Create ThreadX timer service       */
/*    tx_timer_delete                  Delete ThreadX timer service       */
/*    tx_mutex_create                  Create ThreadX mutex service       */ 
/*    tx_mutex_delete                  Delete ThreadX mutex service       */
/*    nx_udp_socket_create             Create the SNTP Client socket      */ 
/*    nx_udp_socket_delete             Delete the SNTP Client socket      */
/*    nx_udp_socket_bind               Bind the SNTP Client to SNTP port  */
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
UINT  _nx_sntp_client_create(NX_SNTP_CLIENT *client_ptr, NX_IP *ip_ptr, UINT iface_index, NX_PACKET_POOL *packet_pool_ptr,    
                                UINT (*leap_second_handler)(NX_SNTP_CLIENT *client_ptr, UINT indicator), 
                                UINT (*kiss_of_death_handler)(NX_SNTP_CLIENT *client_ptr, UINT code),
                                VOID (random_number_generator)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, ULONG *rand))
{

UINT status;


    /* Null the members of NX_SNTP_CLIENT.  */
    memset(client_ptr, 0, sizeof(NX_SNTP_CLIENT));

    /* Set the Client ID to indicate the SNTP client thread is ready.  */
    client_ptr -> nx_sntp_client_id = NXD_SNTP_ID;

    /* Set the IP instance.  */
    client_ptr -> nx_sntp_client_ip_ptr   =  ip_ptr;

    /* Set the SNTP network interface. */
    client_ptr -> nx_sntp_client_interface_index = iface_index;

    /* Check for minimal packet size requirement. */
#ifndef NX_DISABLE_IPV4
    if  (packet_pool_ptr -> nx_packet_pool_payload_size < 
            (sizeof(NX_IPV4_HEADER) + sizeof(NX_UDP_HEADER) + NX_SNTP_CLIENT_PACKET_DATA_SIZE))
    {

        return NX_SNTP_INSUFFICIENT_PACKET_PAYLOAD;
    }
#endif /* NX_DISABLE_IPV4 */


    /* This only assures IPv4 packets will work. We do not know yet if the Client expects to receive
       SNTP packets over ipv6 networks. When sending packets, the SNTP Client will have set its
       SNTP server from which the IP type will be determined.  However, for receiving packets,
       it is up to the host to ensure its packet payload will be large enough for IPv6 packets:

             sizeof(NX_IPV6_HEADER) + sizeof(NX_UDP_HEADER) + NX_SNTP_CLIENT_PACKET_DATA_SIZE)
                  40 bytes                   8 bytes                    48 bytes
    */ 

    client_ptr -> nx_sntp_client_packet_pool_ptr =  packet_pool_ptr;

    /* Create a udp socket to receive SNTP time data. */
    status =  nx_udp_socket_create(client_ptr -> nx_sntp_client_ip_ptr, &(client_ptr -> nx_sntp_client_udp_socket), 
                                   NX_SNTP_CLIENT_UDP_SOCKET_NAME, NX_IP_NORMAL, 
                                   NX_FRAGMENT_OKAY, NX_SNTP_CLIENT_TIME_TO_LIVE, 
                                   NX_SNTP_CLIENT_MAX_QUEUE_DEPTH);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Register a receive notify callback.  */
    status = nx_udp_socket_receive_notify(&(client_ptr -> nx_sntp_client_udp_socket), _nx_sntp_client_receive_notify);

    /* Check for errors.  */
    if (status != NX_SUCCESS)
    {

        nx_udp_socket_delete(&(client_ptr -> nx_sntp_client_udp_socket));

        return status;
    }

    /* Set callback for leap second handler (optional).  */
    client_ptr -> leap_second_handler = leap_second_handler;            

    /* Set callback for kiss of death server packet handler (optional).  */
    client_ptr -> kiss_of_death_handler = kiss_of_death_handler;  

    /* Set callback for random number generator (Only applicable for unicast clients  
       configured for random wait on startup).  */
    client_ptr -> random_number_generator = random_number_generator;            

    /* Create the SNTP update timeout timer.  */
    status =  tx_timer_create(&client_ptr -> nx_sntp_update_timer, "SNTP Client Update Timer",
              _nx_sntp_client_update_timeout_entry, (ULONG)client_ptr, 
              (NX_IP_PERIODIC_RATE * NX_SNTP_UPDATE_TIMEOUT_INTERVAL), 
              (NX_IP_PERIODIC_RATE * NX_SNTP_UPDATE_TIMEOUT_INTERVAL), TX_NO_ACTIVATE);

    /* Check for error.  */
    if (status != TX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(client_ptr -> nx_sntp_client_udp_socket));

        /* Return error status.  */
        return(status);
    }

    /* Create the SNTP mutex.  */
    status =  tx_mutex_create(&(client_ptr -> nx_sntp_client_mutex), "NetX SNTP Client Mutex", TX_NO_INHERIT);

    /* Determine if the semaphore creation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(client_ptr -> nx_sntp_client_udp_socket));

        /* Delete the timer.  */
        tx_timer_delete(&(client_ptr -> nx_sntp_update_timer));

        /* No, return error status.  */
        return(status);
    }

        /* Create the SNTP event flag group.   */
    status =  tx_event_flags_create(&nx_sntp_client_events, "NetX SNTP Client Events");
    
    /* Check the return status.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(client_ptr -> nx_sntp_client_udp_socket));

        /* Delete the timer.  */
        tx_timer_delete(&(client_ptr -> nx_sntp_update_timer));

        /* Delete the Client mutex. */
        tx_mutex_delete(&client_ptr -> nx_sntp_client_mutex);

        /* Error present, return error code.  */
        return(status);
    }

    /* Create the SNTP Client processing thread.  */
    status =  tx_thread_create(&(client_ptr -> nx_sntp_client_thread), "NetX SNTP Client", _nx_sntp_client_thread_entry, (ULONG)client_ptr,
                        client_ptr -> nx_sntp_client_thread_stack, NX_SNTP_CLIENT_THREAD_STACK_SIZE, 
                        NX_SNTP_CLIENT_THREAD_PRIORITY, NX_SNTP_CLIENT_PREEMPTION_THRESHOLD, 1, TX_DONT_START);

    /* Determine if the thread creation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the mutex.  */
        tx_mutex_delete(&(client_ptr -> nx_sntp_client_mutex));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(client_ptr -> nx_sntp_client_udp_socket));

        /* Delete the timer.  */
        tx_timer_delete(&(client_ptr -> nx_sntp_update_timer));

        /* Delete the event flag group.  */
        tx_event_flags_delete(&nx_sntp_client_events);

        /* No, return error status.  */
        return(status);
    }

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_delete                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors on the client delete service.       */ 
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                         Pointer to SNTP Client           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_delete             Actual SNTP client delete service*/ 
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
UINT  _nxe_sntp_client_delete(NX_SNTP_CLIENT *client_ptr)
{

UINT status;


    /* Check for the validity of input parameter.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_sntp_client_id != NXD_SNTP_ID))
    {

        /* Return error status.  */
        return(NX_PTR_ERROR);
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual client create service.  */
    status =  _nx_sntp_client_delete(client_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_delete                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a previously created SNTP Client and releases */
/*    ThreadX and NetX resources held by the Client.                      */
/*                                                                        */ 
/*    Note that it is the application's responsibility to delete/release  */
/*    the Client packet pool.                                             */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to Client struct          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_timer_delete                  Delete ThreadX timer service       */
/*    tx_mutex_delete                  Delete ThreadX mutex service       */
/*    tx_timer_deactivate              Deschedule ThreadX timer           */ 
/*    nx_udp_socket_unbind             Release NetX UDP socket port       */
/*    nx_udp_socket_delete             Delete NetX UDP socket             */ 
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
UINT  _nx_sntp_client_delete(NX_SNTP_CLIENT *client_ptr)
{

    /* Suspend the SNTP Client thread.  */
    tx_thread_suspend(&client_ptr -> nx_sntp_client_thread);

    /* Terminate SNTP Client thread. */
    tx_thread_terminate(&client_ptr -> nx_sntp_client_thread);

    /* Delete SNTP Client thread.  */
    tx_thread_delete(&client_ptr -> nx_sntp_client_thread);

    /* Delete the Client mutex. */
    tx_mutex_delete(&client_ptr -> nx_sntp_client_mutex);

    /* Deactivate SNTP update timer.  */
    tx_timer_deactivate(&(client_ptr -> nx_sntp_update_timer));

    /* Delete the update timer.  */
    tx_timer_delete(&(client_ptr -> nx_sntp_update_timer));

    /* Make sure the socket is in an unbound state.  */
    nx_udp_socket_unbind(&(client_ptr -> nx_sntp_client_udp_socket));

    /* Ok to delete the UDP socket.  */
    nx_udp_socket_delete(&(client_ptr -> nx_sntp_client_udp_socket));

    /* Delete the Client event flags group. */
    tx_event_flags_delete(&nx_sntp_client_events);

    /* Return success status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_update_timeout_entry                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function decrements the Client's time remaining since it last  */
/*    received a time update.                                             */
/*                                                                        */
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                           Pointer to Client              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                               ThreadX timer callback        */
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
VOID  _nx_sntp_client_update_timeout_entry(ULONG info)
{

NX_SNTP_CLIENT *client_ptr;


    client_ptr = (NX_SNTP_CLIENT *)info;

    /* Is the Client's time remaining large enough to decrement by the timeout interval?  */ 
    if (client_ptr -> nx_sntp_update_time_remaining >= (NX_IP_PERIODIC_RATE * NX_SNTP_UPDATE_TIMEOUT_INTERVAL))
    {

        /* Yes; ok to decrement time remaining.  */
        client_ptr -> nx_sntp_update_time_remaining -= (NX_IP_PERIODIC_RATE * NX_SNTP_UPDATE_TIMEOUT_INTERVAL);
    }
    else
    {
        /* No, set the time remaining to NULL.  */
        client_ptr -> nx_sntp_update_time_remaining = 0;
    }

    /* Update time elapsed. */
    client_ptr -> nx_sntp_client_local_ntp_time_elapsed += NX_SNTP_UPDATE_TIMEOUT_INTERVAL;

    /* Is the client operating in unicast? */
    if (client_ptr -> nx_sntp_client_protocol_mode == UNICAST_MODE)
    {

        /* Yes; Update the time remaining till it is time to poll the SNTP server. */
        if (client_ptr -> nx_sntp_client_unicast_poll_interval >= (NX_IP_PERIODIC_RATE * NX_SNTP_UPDATE_TIMEOUT_INTERVAL))
        {
            client_ptr -> nx_sntp_client_unicast_poll_interval -= (NX_IP_PERIODIC_RATE * NX_SNTP_UPDATE_TIMEOUT_INTERVAL);
        }
        else
        {

            /* Time is up. Client should send out another update request. */
            client_ptr -> nx_sntp_client_unicast_poll_interval = 0;
        }
    }
    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_create_time_request_packet          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts NTP time data from the input buffer into a   */
/*    unicast time data and copies into the supplied packet buffer.       */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                           Pointer to Client              */
/*    packet_ptr                           Pointer to time update packet  */ 
/*    time_message_ptr                     Pointer to time request        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Successful completion status   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_send_unicast_request Send unicast request to server */
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
UINT  _nx_sntp_client_create_time_request_packet(NX_SNTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, 
                                                 NX_SNTP_TIME_MESSAGE *time_message_ptr)
{
    NX_PARAMETER_NOT_USED(client_ptr);

    if (40u > ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
    {
        return(NX_SIZE_ERROR);
    }

    /* Clear packet data. */
    memset(packet_ptr -> nx_packet_append_ptr, 0, 40);

    *(packet_ptr -> nx_packet_append_ptr) = (UCHAR)(time_message_ptr -> flags);

    /* Skip to transmit time; all other fields are not used for a client unicast request.  */
    packet_ptr -> nx_packet_append_ptr += 40;
    packet_ptr -> nx_packet_length += 40;

    /* Copy the transmit time stamp from time request into the packet buffer.  */
    time_message_ptr -> transmit_time.seconds = time_message_ptr -> transmit_time_stamp[0];
    time_message_ptr -> transmit_time.fraction = time_message_ptr -> transmit_time_stamp[1];

    /* Now copy the data to the buffer. */
    *((ULONG*)(packet_ptr -> nx_packet_append_ptr)) = (ULONG)(time_message_ptr -> transmit_time.seconds);
    *((ULONG*)(packet_ptr -> nx_packet_append_ptr + 4)) = (ULONG)(time_message_ptr -> transmit_time.fraction);

    /* Change endian to network order. */
    NX_CHANGE_ULONG_ENDIAN(*((ULONG*)(packet_ptr -> nx_packet_append_ptr)));
    NX_CHANGE_ULONG_ENDIAN(*((ULONG*)(packet_ptr -> nx_packet_append_ptr + 4)));

    packet_ptr -> nx_packet_append_ptr += 8;
    packet_ptr -> nx_packet_length += 8;

    /* Return completion status.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_sntp_client_initialize_unicast                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the service that          */
/*    initializes the Client for unicast service.                         */ 
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client struct           */ 
/*    unicast_time_server              Pointer to unicast server          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     None                                                               */ 
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
UINT  _nxde_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *unicast_time_server)
{

UINT  status;


    /* Check pointer input.  */
    if ((client_ptr == NX_NULL) || (unicast_time_server == NX_NULL))
    {

        /* Return error status.  */
        return NX_PTR_ERROR;
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual Client for initialize unicast service.  */
    status = _nxd_sntp_client_initialize_unicast(client_ptr, unicast_time_server);

    /* Return completion status.  */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_sntp_client_initialize_unicast                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up the Client to operate in unicast mode with the*/
/*    supplied IPv4 or IPv6 SNTP server as the primary (current) server.  */
/*                                                                        */ 
/*    For adding IPv4 and IPv6 SNTP servers to the unicast server list,   */
/*    use the nxd_sntp_client_duo_add_unicast_server_to_list service.     */ 
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client struct           */ 
/*    unicast_poll_interval            Interval between update requests   */
/*    unicast_time_server              Pointer to unicast server          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_SNTP_PARAM_ERROR               Invalid server list input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     memset                           Copy data to area of memory       */ 
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
UINT  _nxd_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *unicast_time_server)
{


    /* Set the Client polling interval.  */
    client_ptr -> nx_sntp_client_unicast_poll_interval = (NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL  * NX_IP_PERIODIC_RATE); 

    /* Initialize the number of times we've increased the backoff rate to zero. */
    client_ptr -> nx_sntp_client_backoff_count = 0;

    /* Clear the Client's current server IP.  */
    memset(&client_ptr -> nx_sntp_server_ip_address, 0, sizeof(NXD_ADDRESS));

#ifdef FEATURE_NX_IPV6
    /* Set as the Client's unicast server.  */
    COPY_NXD_ADDRESS(unicast_time_server, &client_ptr -> nx_sntp_unicast_time_server);

    /* Set as the Client's current SNTP server.  */
    COPY_NXD_ADDRESS(unicast_time_server, &client_ptr -> nx_sntp_server_ip_address);
#else
    /* Set as the Client's unicast server.  */
    client_ptr -> nx_sntp_unicast_time_server.nxd_ip_version = NX_IP_VERSION_V4;
    client_ptr -> nx_sntp_unicast_time_server.nxd_ip_address.v4 = unicast_time_server -> nxd_ip_address.v4;

    /* Set as the Client's current SNTP server.  */
    client_ptr -> nx_sntp_server_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
    client_ptr -> nx_sntp_server_ip_address.nxd_ip_address.v4 = unicast_time_server -> nxd_ip_address.v4;

#endif /* FEATURE_NX_IPV6 */


    /* Set the Client operational mode to unicast mode.  */
    client_ptr -> nx_sntp_client_protocol_mode = UNICAST_MODE;

    /* Indicate the Client task is ready to run! */
    client_ptr -> nx_sntp_client_unicast_initialized = NX_TRUE;

    /* Initialize the server status as good (receiving updates). */
    client_ptr -> nx_sntp_valid_server_status = NX_TRUE;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_initialize_unicast                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors on the initialize Client for unicast*/
/*    service.                                                            */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client struct           */ 
/*    unicast_time_server              SNTP unicast server to use         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                     Invalid pointer input              */
/*    NX_INVALID_PARAMETERS            Invalid non pointer input          */ 
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
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
UINT  _nxe_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, ULONG unicast_time_server)
{

#ifndef NX_DISABLE_IPV4
UINT status;

    /* Check input parameters.  */
    if (client_ptr == NX_NULL)
    {

        /* Return error status.  */
        return NX_PTR_ERROR;
    }

    if (unicast_time_server == 0x0)
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING


    /* Call the actual Client for initialize unicast service.  */
    status = _nx_sntp_client_initialize_unicast(client_ptr, unicast_time_server);

    /* Return completion status.  */
    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(unicast_time_server);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_initialize_unicast                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts the input address to the NetX Duo address    */
/*    format and calls the 'duo' equivalent service, nxd_sntp_client_     */
/*    _initialize_unicast.                                                */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client struct           */ 
/*    unicast_time_server              SNTP unicast server to use         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*     _nxd_sntp_client_initialize_unicast                                */
/*                                      Duo initialize unicast service    */
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
UINT  _nx_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, ULONG unicast_time_server)
{

#ifndef NX_DISABLE_IPV4
UINT  status;
NXD_ADDRESS duo_unicast_time_server;


        
    duo_unicast_time_server.nxd_ip_address.v4 = unicast_time_server;
    duo_unicast_time_server.nxd_ip_version = NX_IP_VERSION_V4;

    status = _nxd_sntp_client_initialize_unicast(client_ptr, &duo_unicast_time_server);

    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(unicast_time_server);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_run_unicast                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the run unicast service.   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                     Invalid pointer input              */ 
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*    _nx_sntp_client_run_unicast      Actual run unicast service         */
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
UINT  _nxe_sntp_client_run_unicast(NX_SNTP_CLIENT *client_ptr)
{

UINT status;

    /* Check for invalid pointer input.  */
    if (client_ptr == NX_NULL)
    {

        /* Return pointer error.  */
        return NX_PTR_ERROR;
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual run unicast service.  */
    status = _nx_sntp_client_run_unicast(client_ptr);

    /* Return actual completion status.  */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_run_unicast                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts the SNTP Client for unicast SNTP processing    */
/*    by activating the SNTP timer and main processing thread. The Client */
/*    is checked for being initialized for unicast SNTP and if it is      */
/*    not, the function returns an error status.                          */
/*                                                                        */
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SNTP_CLIENT_NOT_INITIALIZED   Client initialized flag not set    */ 
/*    NX_SNTP_CLIENT_ALREADY_STARTED   Client already started             */
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_timer_activate                Start the ThreadX timer            */ 
/*    tx_thread_resume                 Resume the specified thread        */ 
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
UINT  _nx_sntp_client_run_unicast(NX_SNTP_CLIENT *client_ptr)
{

UINT    status;
ULONG   startup_ticks;


    status = tx_mutex_get(&client_ptr -> nx_sntp_client_mutex, TX_WAIT_FOREVER);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Determine if SNTP has already been started.  */
    if (client_ptr -> nx_sntp_client_started)
    {

        /* Error SNTP has already been started.  */

        /* Release the SNTP mutex.  */
        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        /* Return completion status.  */
        return(NX_SNTP_CLIENT_ALREADY_STARTED);
    }

    /* Verify the client is ready to start receiving time data.  */
    if (client_ptr -> nx_sntp_client_unicast_initialized == NX_FALSE)
    {

        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        /* Return the error condition.  */
        return NX_SNTP_CLIENT_NOT_INITIALIZED;
    }

    /* Should the first time request sleep for a random timeout?  */
    if ((NX_SNTP_CLIENT_RANDOMIZE_ON_STARTUP == NX_TRUE) && (client_ptr -> random_number_generator))
    {

        /* Yes, generate a random number of ticks to sleep.*/
       (client_ptr -> random_number_generator)(client_ptr, &startup_ticks); 
    
       /* Sleep for the random length.  */  
       tx_thread_sleep(startup_ticks);
    }

    /* Set status that the first update is expected. */
    client_ptr -> nx_sntp_client_first_update_pending = NX_TRUE;

    /* Initialize missed broadcasts and bad time updates from server.  */
    client_ptr -> nx_sntp_client_invalid_time_updates = 0;


    /* Set the maximum timeout to the full count.  */
    client_ptr -> nx_sntp_update_time_remaining = (NX_SNTP_CLIENT_MAX_TIME_LAPSE * NX_IP_PERIODIC_RATE);

    /* Activate SNTP update timer.  */
    status = tx_timer_activate(&(client_ptr -> nx_sntp_update_timer));

    /* Check for error. Ignore timer is already active error.  */
    if (status != TX_SUCCESS && status != TX_ACTIVATE_ERROR)
    {

        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        /* Return the error condition.  */
        return status;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&(client_ptr -> nx_sntp_client_udp_socket), NX_SNTP_CLIENT_UDP_PORT, NX_WAIT_FOREVER);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

        /* Deactivate SNTP update timer.  */
        tx_timer_deactivate(&(client_ptr -> nx_sntp_update_timer));

        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        return status;
    }

    status = tx_thread_resume(&(client_ptr -> nx_sntp_client_thread));

    /* Start the SNTP Client thread. */
    if (status != NX_SUCCESS)
    {

        /* Release the socket port. */
        nx_udp_socket_unbind(&(client_ptr -> nx_sntp_client_udp_socket));

        /* Deactivate SNTP update timer.  */
        tx_timer_deactivate(&(client_ptr -> nx_sntp_update_timer));

        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        return status;
    }

    /* The client thread is successfully started, so set the started flag to true. */
    client_ptr -> nx_sntp_client_started = NX_TRUE;

    /* Clear the sleep flag.  */
    client_ptr -> nx_sntp_client_sleep_flag =  NX_FALSE;

    /* Set poll interval to 0 to send request immediately. */
    client_ptr -> nx_sntp_client_unicast_poll_interval = 0;

    tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_run_unicast                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function checks if the SNTP client has received a packet, and if*/
/*   so processes it for valid SNTP data and resets the timeouts          */
/*   for sending (polling) and receiving updates. If no packet received it*/
/*   checks if the timeout to receive an update has expired.  If so, it   */
/*   sends out another SNTP request.                                      */
/*                                                                        */ 
/*   When a timeout expires, the poll interval is increased as per        */
/*   RFC guidelines until it exceeds the maximum time without an update.  */
/*   An error flag is set, and it is up to the host application to        */
/*   change its unicast SNTP server.                                      */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_reset_current_time_message                          */
/*                                    Save time update before next update */
/*    _nx_sntp_client_receive_time_update                                 */
/*                                    Receive server update packets       */
/*    _nx_sntp_client_process_time_data                                   */
/*                                    Process time data in server update  */
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
VOID  _nx_sntp_client_process_unicast(NX_SNTP_CLIENT *client_ptr)
{

UINT    status;
ULONG   sntp_events;     


    /* Check for a receive event. */
    status = tx_event_flags_get(&nx_sntp_client_events, NX_SNTP_CLIENT_RECEIVE_EVENT, TX_OR_CLEAR, &sntp_events, TX_NO_WAIT);
                
    if (status == NX_SUCCESS)
    {

        /* Listen for a server update packet.  */
        status = _nx_sntp_client_receive_time_update(client_ptr, TX_NO_WAIT);

        if (status == NX_SUCCESS)
        {
        
            /* Process the server update packet and apply to local time if valid.  */
            status =  _nx_sntp_client_process_update_packet(client_ptr);

            /* Check for error. */
            if (status != NX_SUCCESS)
            {
                 return;
            }

             /* Reset the Client poll timeout to the original poll interval.  */
             client_ptr -> nx_sntp_client_unicast_poll_interval = (NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL  * NX_IP_PERIODIC_RATE); 

             /* Reset the Client timeout for maximum time lapse without a valid update. */
             client_ptr -> nx_sntp_update_time_remaining = (NX_SNTP_CLIENT_MAX_TIME_LAPSE * NX_IP_PERIODIC_RATE); 

             /* Indicate the client has received at least one valid time update with the current server. */
             client_ptr -> nx_sntp_client_first_update_pending = NX_FALSE;

             client_ptr -> nx_sntp_client_backoff_count = 0;
             /* Set the server status as good (receiving updates). */
             client_ptr -> nx_sntp_valid_server_status = NX_TRUE;

             return;
        }
    }

    /* Has the timeout expired on the maximum lapse without a valid update?  */
    if (client_ptr -> nx_sntp_update_time_remaining == 0)
    {

        /* Yes, it has. Set the server status as no longer valid. */
        client_ptr -> nx_sntp_valid_server_status = NX_FALSE;
    }

    /* Is it time to send another SNTP request? */
    if (client_ptr -> nx_sntp_client_unicast_poll_interval == 0)
    {

        
        /* Save the last server message before the next update.  We need this for comparing time data. */
        _nx_sntp_client_reset_current_time_message(client_ptr);

        /* Create and send a unicast request .  */
        _nx_sntp_client_send_unicast_request(client_ptr);


        /* Check if we have received an update since the previous unicast poll. */
        if ((NX_SNTP_CLIENT_MAX_TIME_LAPSE * NX_IP_PERIODIC_RATE - client_ptr -> nx_sntp_update_time_remaining) > 
            (NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL  * NX_IP_PERIODIC_RATE))
        {

            /* No we have not. Increase the count of times we've increased the back off rate. */
            client_ptr -> nx_sntp_client_backoff_count++;

            client_ptr -> nx_sntp_client_unicast_poll_interval = 
                NX_IP_PERIODIC_RATE * NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL * NX_SNTP_CLIENT_EXP_BACKOFF_RATE * client_ptr -> nx_sntp_client_backoff_count; 
        }
        else
        {
        
            /* Reset the polling interval to it's normal value. */
            client_ptr -> nx_sntp_client_unicast_poll_interval = NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL  * NX_IP_PERIODIC_RATE;
        }

        /* Check that the poll interval does not exceed the maximum lapse without a valid update. */
        if (client_ptr -> nx_sntp_client_unicast_poll_interval > (NX_SNTP_CLIENT_MAX_TIME_LAPSE * NX_IP_PERIODIC_RATE))
        {

            /* Set the poll interval equal to that lapse. */
            client_ptr -> nx_sntp_client_unicast_poll_interval = (NX_SNTP_CLIENT_MAX_TIME_LAPSE * NX_IP_PERIODIC_RATE);
        }
    }

    /* Nothing to do but wait...*/
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_sntp_client_initialize_broadcast              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the initialize Client for  */
/*    IPv4 broadcast time updates service.                                */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*    multicast_server_address         Multicast server address           */
/*    broadcast_time_server            Broadcast server address           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                     Invalid pointer input              */ 
/*    NX_SNTP_PARAM_ERROR              Invalid non pointer input          */ 
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*     _nxd_sntp_client_initialize_duo_broadcast                          */ 
/*                                     Initialize broadcast Client service*/ 
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
UINT  _nxde_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *multicast_server_address, NXD_ADDRESS *broadcast_server_address)
{

UINT status;


    /* Check for invalid pointer input.  */
    if (client_ptr == NX_NULL)
    {

        /* Return pointer error.  */
        return NX_PTR_ERROR;
    }

    /* Check for missing broadcast server input. */
    if ((multicast_server_address == NX_NULL) && (broadcast_server_address == NX_NULL))
    {

        /* Return pointer error.  */
        return NX_PTR_ERROR;
    }

    /* Check for illegal broadcast address in IPv6. */
    if ((broadcast_server_address != NX_NULL) && (broadcast_server_address -> nxd_ip_version == NX_IP_VERSION_V6))
    {

        return NX_SNTP_PARAM_ERROR;
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual initialize client for broadcast time update service.  */
    status = _nxd_sntp_client_initialize_broadcast(client_ptr, multicast_server_address, broadcast_server_address);

    /* Return completion status.  */
    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_sntp_client_initialize_broadcast               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up the Client to operate in broadcast mode with  */
/*    either an IPv6 or an IPv4 broadcast server.                         */
/*                                                                        */
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*    multicast_server_address         Multicast server address           */
/*    broadcast_time_server            Broadcast server address           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
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
UINT  _nxd_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *multicast_server_address, NXD_ADDRESS *broadcast_server_address)
{


    /* Clear Client's current server IP.  */
    memset(&client_ptr -> nx_sntp_server_ip_address, 0, sizeof(NXD_ADDRESS));

    /* Was a multicast IP was supplied?  */
    if (multicast_server_address != NX_NULL)
    {

#ifdef FEATURE_NX_IPV6
        /* Set the Client multicast server.  */
        COPY_NXD_ADDRESS(multicast_server_address, &client_ptr -> nx_sntp_multicast_server_address);

        /* Set this as the Client's current SNTP server. */
        COPY_NXD_ADDRESS(multicast_server_address, &client_ptr -> nx_sntp_server_ip_address);
#else
        /* Set the Client multicast server.  */
        client_ptr -> nx_sntp_multicast_server_address.nxd_ip_version = NX_IP_VERSION_V4;
        client_ptr -> nx_sntp_multicast_server_address.nxd_ip_address.v4 = multicast_server_address -> nxd_ip_address.v4;

        /* Set this as the Client's current SNTP server. */
        client_ptr -> nx_sntp_server_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
        client_ptr -> nx_sntp_server_ip_address.nxd_ip_address.v4 = multicast_server_address -> nxd_ip_address.v4;

#endif /* FEATURE_NX_IPV6 */
    }

    /* No multicast address supplied, so was a broadcast address specified?  This must be an IPv4 address
       since IPv6 does not support broadcast. */
    else if ((broadcast_server_address != NX_NULL) && (broadcast_server_address -> nxd_ip_version == NX_IP_VERSION_V4))
    {
        

#ifdef FEATURE_NX_IPV6
        /* Set the Client broadcast server.  */
        COPY_NXD_ADDRESS(broadcast_server_address, &client_ptr -> nx_sntp_broadcast_time_server);

        /* Set this as the Client's current SNTP server. */
        COPY_NXD_ADDRESS(broadcast_server_address, &client_ptr -> nx_sntp_server_ip_address);
#else
        /* Set the Client broadcast server.  */
        client_ptr -> nx_sntp_broadcast_time_server.nxd_ip_version = NX_IP_VERSION_V4;
        client_ptr -> nx_sntp_broadcast_time_server.nxd_ip_address.v4 = broadcast_server_address -> nxd_ip_address.v4;

        /* Set this as the Client's current SNTP server. */
        client_ptr -> nx_sntp_server_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
        client_ptr -> nx_sntp_server_ip_address.nxd_ip_address.v4 = broadcast_server_address -> nxd_ip_address.v4;

#endif /* FEATURE_NX_IPV6 */
    }

    /* Set client to work in broadcast (non unicast) mode.  */
    client_ptr -> nx_sntp_client_protocol_mode = BROADCAST_MODE;

    /* Indicate the client task is ready to run! */
    client_ptr -> nx_sntp_client_broadcast_initialized = NX_TRUE;

    /* Initialize the server status as good (receiving updates) even though it has
       not received one with the current server. */
    client_ptr -> nx_sntp_valid_server_status = NX_TRUE;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_initialize_broadcast               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the initialize Client for  */
/*    broadcast time updates service. Note that broadcast and multicast   */
/*    services are only available for IPv4 SNTP communcations.  For       */
/*    multicast service, the host IP instance must be enabled for IGMP.   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*    multicast_server_address         Server multicast address           */
/*    broadcast_server_aaddres         Broadcast server address           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                     Invalid pointer input              */ 
/*    NX_INVALID_PARAMETERS            Invalid non pointer input          */ 
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*     _nx_sntp_client_initialize_broadcast                               */ 
/*                                     Actual initialize broadcast service*/ 
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
UINT  _nxe_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, ULONG multicast_server_address, ULONG broadcast_server_address)
{

#ifndef NX_DISABLE_IPV4
UINT status;


    /* Check for invalid pointer input.  */
    if (client_ptr == NX_NULL)
    {

        /* Return pointer error.  */
        return NX_PTR_ERROR;
    }

    if ((multicast_server_address == 0x0) && (broadcast_server_address == 0x0))
    {

        return NX_INVALID_PARAMETERS;
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual initialize client for broadcast time update service.  */
    status = _nx_sntp_client_initialize_broadcast(client_ptr, multicast_server_address, broadcast_server_address);

    /* Return completion status.  */
    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(multicast_server_address);
    NX_PARAMETER_NOT_USED(broadcast_server_address);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_initialize_broadcast                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts the IPv4 addresses to the NetX Duo address   */
/*    format and calls the 'duo' equivalent service, nxd_sntp_client_     */
/*    _initialize_broadcast.                                              */
/*                                                                        */
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*    multicast_server_address         Server multicast address           */
/*    broadcast_server_address         Broadcast server address           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*     _nxd_sntp_client_initialize_broadcast                              */
/*                                      Duo initialize broadcast service  */
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
UINT  _nx_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, ULONG multicast_server_address, ULONG broadcast_server_address)
{

#ifndef NX_DISABLE_IPV4
UINT        status;
NXD_ADDRESS server_address;


    /* Was a multicast IP was supplied?  */
    if (multicast_server_address != 0x0)
    {

        server_address.nxd_ip_address.v4 = multicast_server_address;
        server_address.nxd_ip_version = NX_IP_VERSION_V4;
        status =  _nxd_sntp_client_initialize_broadcast(client_ptr, &server_address, NX_NULL);
    }

    /* Was a broadcast SNTP server specified?  */
    else if (broadcast_server_address != 0x0)
    {
        
        server_address.nxd_ip_address.v4 = broadcast_server_address;
        server_address.nxd_ip_version = NX_IP_VERSION_V4;
        status =  _nxd_sntp_client_initialize_broadcast(client_ptr, NX_NULL, &server_address);
    }
    else
        return NX_INVALID_PARAMETERS;

    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(multicast_server_address);
    NX_PARAMETER_NOT_USED(broadcast_server_address);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_stop                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the SNTP client thread stop*/ 
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                           Pointer to SNTP Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_sntp_client_sto                    Actual stop SNTP service      */ 
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
UINT  _nxe_sntp_client_stop(NX_SNTP_CLIENT *client_ptr)
{

UINT status;


    /* Check for invalid pointer input.  */
    if (client_ptr == NX_NULL)
    {

        /* Return pointer error.  */
        return NX_PTR_ERROR;
    }

    status = _nx_sntp_client_stop(client_ptr);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_stop                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function halts the SNTP Client thread and returns the SNTP     */
/*    Client to an initial state.                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                           Pointer to SNTP Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_unbind                  Unbind the SNTP UDP socket    */ 
/*    tx_thread_preemption_change           Change the thread preemption  */ 
/*    tx_thread_suspend                     Suspend SNTP processing       */ 
/*    tx_thread_wait_abort                  Remove any thread suspension  */
/*    tx_mutex_get                          Get the SNTP mutex            */ 
/*    tx_mutex_put                          Release the SNTP mutex        */ 
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
UINT  _nx_sntp_client_stop(NX_SNTP_CLIENT *client_ptr)
{

UINT    current_preemption;


    /* Get the SNTP mutex.  */
    tx_mutex_get(&(client_ptr -> nx_sntp_client_mutex), TX_WAIT_FOREVER);

    /* Determine if the SNTP client is started.  */
    if (client_ptr -> nx_sntp_client_started == NX_FALSE)
    {

        /* Release the SNTP mutex.  */
        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        /* SNTP client is not started so it can't be stopped.  */
        return(NX_SNTP_CLIENT_NOT_STARTED);
    }    

    /* Clear the started flag here to ensure other threads can't issue a stop while
       this stop is in progress.  */
    client_ptr -> nx_sntp_client_started =  NX_FALSE;

    /* Indicate the client needs to be re-initialized to run again. */
    client_ptr -> nx_sntp_client_unicast_initialized = NX_FALSE;
    client_ptr -> nx_sntp_client_broadcast_initialized = NX_FALSE;

    /* Disable preemption for critical section.  */
    tx_thread_preemption_change(tx_thread_identify(), 0, &current_preemption);

    /* Loop to wait for the SNTP Client thread to be in a position to be stopped.  */
    while (client_ptr -> nx_sntp_client_sleep_flag != NX_TRUE)
    {

        /* Release the SNTP mutex.  */
        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        /* Sleep temporarily. */
        tx_thread_sleep(1);

        /* Get the SNTP mutex.  */
        tx_mutex_get(&(client_ptr -> nx_sntp_client_mutex), TX_WAIT_FOREVER);
    }

    /* Clear the sleep flag.  */
    client_ptr -> nx_sntp_client_sleep_flag =  NX_FALSE;

    /* Get rid of any previous server message.  */
    memset(&(client_ptr -> nx_sntp_previous_server_time_message), 0, sizeof(NX_SNTP_TIME_MESSAGE));

    /* Get rid of the current server message if there is one.  */
    memset(&(client_ptr -> nx_sntp_current_server_time_message), 0, sizeof(NX_SNTP_TIME_MESSAGE));

    /* Deactivate SNTP update timer.  */
    tx_timer_deactivate(&(client_ptr -> nx_sntp_update_timer));

    /* Suspend the SNTP Client thread.  */
    tx_thread_suspend(&(client_ptr -> nx_sntp_client_thread));

    /* Abort the wait on the SNTP Client thread.  */
    tx_thread_wait_abort(&(client_ptr -> nx_sntp_client_thread));

    /* Unbind the port.  */
    nx_udp_socket_unbind(&(client_ptr -> nx_sntp_client_udp_socket));

    /* Restore preemption.  */
    tx_thread_preemption_change(tx_thread_identify(), current_preemption, &current_preemption);

    /* Release the SNTP mutex.  */
    tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_run_broadcast                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the run broadcast service. */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                          Pointer to Client               */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_run_broadcast       Actual run broadcast service    */
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
UINT  _nxe_sntp_client_run_broadcast(NX_SNTP_CLIENT *client_ptr)
{

UINT status;


    /* Check for pointer error.  */
    if (client_ptr == NX_NULL)
    {

        /* Return error status.  */
        return NX_PTR_ERROR;
    }

    /* Verify this is called from thread context only.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual run broadcast service.  */    
    status = _nx_sntp_client_run_broadcast(client_ptr);

    /* Return completion status.  */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_run_broadcast                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts the SNTP Client for broadcast SNTP processing  */
/*    by activating the SNTP timer and main processing thread. The Client */
/*    is checked for being initialized for broadcast SNTP and if it is    */
/*    not, the function returns an error status.                          */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SNTP_CLIENT_NOT_INITIALIZED   Client initialized flag not set    */ 
/*    NX_SNTP_CLIENT_ALREADY_STARTED   Client already started             */
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_timer_activate                Start the ThreadX timer            */ 
/*    tx_thread_resume                 Resume the specified thread        */ 
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
UINT  _nx_sntp_client_run_broadcast(NX_SNTP_CLIENT *client_ptr)
{

UINT    status;


    /* Get the SNTP mutex.  */
    tx_mutex_get(&(client_ptr -> nx_sntp_client_mutex), TX_WAIT_FOREVER);

    /* Determine if SNTP has already been started.  */
    if (client_ptr -> nx_sntp_client_started)
    {

        /* Error SNTP has already been started.  */

        /* Release the SNTP mutex.  */
        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        /* Return completion status.  */
        return(NX_SNTP_CLIENT_ALREADY_STARTED);
    }

    /* Verify the client is ready to start receiving time data.  */
    if (client_ptr -> nx_sntp_client_broadcast_initialized == NX_FALSE)
    {

        tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

        /* Return error condition.  */
        return NX_SNTP_CLIENT_NOT_INITIALIZED;
    }


    /* Set the maximum timeout to the full count.  */
    client_ptr -> nx_sntp_update_time_remaining = (NX_SNTP_CLIENT_MAX_TIME_LAPSE * NX_IP_PERIODIC_RATE);

    /* Activate the SNTP update timer.  */
    status = tx_timer_activate(&(client_ptr -> nx_sntp_update_timer)); 

    /* Check for error. Ignore timer is already active error.  */
    if (status != TX_SUCCESS && status != TX_ACTIVATE_ERROR)
    {

        tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

        /* Return the error condition.  */
        return status;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&(client_ptr -> nx_sntp_client_udp_socket), NX_SNTP_CLIENT_UDP_PORT, NX_WAIT_FOREVER);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

        /* Deactivate SNTP update timer.  */
        tx_timer_deactivate(&(client_ptr -> nx_sntp_update_timer));

        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        return status;
    }

    /* Set status that the first update is expected. */
    client_ptr -> nx_sntp_client_first_update_pending = NX_TRUE;

    status = tx_thread_resume(&(client_ptr -> nx_sntp_client_thread));

    if (status != NX_SUCCESS)
    {

        /* Release the socket port. */
        nx_udp_socket_unbind(&(client_ptr -> nx_sntp_client_udp_socket));

        /* Deactivate SNTP update timer.  */
        tx_timer_deactivate(&(client_ptr -> nx_sntp_update_timer));

        tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

        return status;
    }

    /* Clear the sleep flag.  */
    client_ptr -> nx_sntp_client_sleep_flag =  NX_FALSE;

    client_ptr -> nx_sntp_client_started = NX_TRUE;

    tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_process_broadcast                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function checks if the SNTP client has received a packet, and if*/
/*   so processes it for valid SNTP data and resets the timeouts          */
/*   for receiving the next update.                                       */
/*                                                                        */ 
/*   If the invalid packet update count exceeds the maximum allowed       */
/*   updates, or no valid packet is received within the maximum interval, */
/*   an error flag is set, and it is up to the host application to change */
/*   its broadcast SNTP server.                                           */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_reset_current_time_message                          */
/*                                    Save time update before next update */
/*    _nx_sntp_client_receive_time_update                                 */
/*                                    Receive server update packets       */
/*    _nx_sntp_client_process_time_data                                   */
/*                                    Process time data in server update  */
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
VOID _nx_sntp_client_process_broadcast(NX_SNTP_CLIENT *client_ptr)
{

UINT  status;
ULONG sntp_events;


    /* Check for a receive event. */
    status = tx_event_flags_get(&nx_sntp_client_events, NX_SNTP_CLIENT_RECEIVE_EVENT, TX_OR_CLEAR, &sntp_events, TX_NO_WAIT);
                
    if (status == NX_SUCCESS)
    {

        /* Save this server time update before receiving the next server time update.  */
        _nx_sntp_client_reset_current_time_message(client_ptr);

        /* Listen for a server update packet.  */
        status = _nx_sntp_client_receive_time_update(client_ptr,  TX_NO_WAIT);

        if (status == NX_SUCCESS)
        {
        
            /* Process the server update packet and apply to local time if valid.  */
            status =  _nx_sntp_client_process_update_packet(client_ptr);

            /* Check for error. */
            if (status != NX_SUCCESS)
            {

                /* Have we exceeded the max number of invalid or missed updates from this server? */
                if (client_ptr -> nx_sntp_client_invalid_time_updates > NX_SNTP_CLIENT_INVALID_UPDATE_LIMIT)
                {

                        /* Yes, set the server status as no longer valid. */
                        client_ptr -> nx_sntp_valid_server_status = NX_FALSE;
                }

                return;
            }

            /* Reset the Client timeout for receiving broadcast updates, zero out invalid update count.  */
            client_ptr -> nx_sntp_update_time_remaining = (NX_SNTP_CLIENT_MAX_TIME_LAPSE * NX_IP_PERIODIC_RATE);

            client_ptr -> nx_sntp_client_invalid_time_updates = 0;

            /* Set the server status as good (receiving updates). */
            client_ptr -> nx_sntp_valid_server_status = NX_TRUE;

            /* Indicate the client has received at least one valid time update with the current server. */
            client_ptr -> nx_sntp_client_first_update_pending = NX_FALSE;

            return;
        }
    }

    /* Has the timeout for receiving the next time update expired? */
    if (client_ptr -> nx_sntp_update_time_remaining == 0)
    {

        /* Yes, set the server status as no longer valid. */
        client_ptr -> nx_sntp_valid_server_status = NX_FALSE;
    }

    /* Nothing to do but wait...*/
    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_send_unicast_request                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function allocates a packet from the Client packet pool,creates*/
/*    a time request time message, transfers the request message into the */
/*    packet buffer, and sends the packet via the Client UDP socket.  It  */
/*    then releases the packet back to the packet pool.                   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                          Clear specified area of memory      */
/*    nx_packet_allocate              Allocate packet from packet pool    */
/*    nx_packet_release               Release packet back to packet pool  */
/*    nx_udp_socket_send              Transmit packet via UDP socket      */
/*    _nx_sntp_client_utility_convert_time_to_UCHAR                       */
/*                                    Copy NX_SNTP_TIME data into a       */
/*                                          UCHAR[4] time stamp field     */
/*    _nx_sntp_client_create_time_request_packet                          */
/*                                    Create packet with time request data*/ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_run_unicast   Send and receive unicast time data    */
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
UINT  _nx_sntp_client_send_unicast_request(NX_SNTP_CLIENT *client_ptr)
{

NX_PACKET   *packet_ptr;
UINT        status;
NX_SNTP_TIME_MESSAGE *unicast_request;
NX_SNTP_TIME local_time;


    /* Lock access to the SNTP Client while we create a unicast request. */
    tx_mutex_get(&(client_ptr -> nx_sntp_client_mutex), TX_WAIT_FOREVER);

    unicast_request = &(client_ptr -> nx_sntp_current_time_message_request);

    /* Clear the time request.  */
    memset(unicast_request, 0, sizeof(NX_SNTP_TIME_MESSAGE));

    /* Add client version to the time request flags.  */
    unicast_request -> flags = NX_SNTP_CLIENT_NTP_VERSION << 3;

    /* Set the Client association (mode) in the last three bits of the flag field.  */
    unicast_request -> flags |= PROTOCOL_MODE_CLIENT;

    /* Release the mutex before a potentially blocking call. */
    tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

    /* Allocate a packet from the Client pool.  */
    status =  nx_packet_allocate(client_ptr -> nx_sntp_client_packet_pool_ptr, &packet_ptr, NX_UDP_PACKET, NX_SNTP_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return status;
    }

    /* Reacquire the lock to SNTP Client while we process request. */
    tx_mutex_get(&(client_ptr -> nx_sntp_client_mutex), TX_WAIT_FOREVER);

    /* Check for minimal packet size requirement. */


    /* IPv6 packets require more space for the IPv6 header. */
    if (client_ptr -> nx_sntp_server_ip_address.nxd_ip_version == NX_IP_VERSION_V6)
    {

#ifndef FEATURE_NX_IPV6
        nx_packet_release(packet_ptr);

        /* Release the mutex. */
        tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

        return NX_SNTP_INVALID_IP_ADDRESS;
#else

        if  (client_ptr -> nx_sntp_client_packet_pool_ptr -> nx_packet_pool_payload_size < 
                (sizeof(NX_IPV6_HEADER) + sizeof(NX_UDP_HEADER) + NX_SNTP_CLIENT_PACKET_DATA_SIZE))
        {

            nx_packet_release(packet_ptr);

            /* Release the mutex. */
            tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

            return NX_SNTP_INSUFFICIENT_PACKET_PAYLOAD;
        }
#endif /* FEATURE_NX_IPV6 */
    }
    else  /* IPv4 */
    {

#ifdef NX_DISABLE_IPV4
        nx_packet_release(packet_ptr);

        /* Release the mutex. */
        tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

        return NX_SNTP_INVALID_IP_ADDRESS;
#else
        
        if  (client_ptr -> nx_sntp_client_packet_pool_ptr -> nx_packet_pool_payload_size < 
                (sizeof(NX_IPV4_HEADER) + sizeof(NX_UDP_HEADER) + NX_SNTP_CLIENT_PACKET_DATA_SIZE))
        {
    
            nx_packet_release(packet_ptr);

            /* Release the mutex. */
            tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

            return NX_SNTP_INSUFFICIENT_PACKET_PAYLOAD;
        }
#endif /* NX_DISABLE_IPV4 */
    }


    /* Convert the local time into the request's transmit time stamp field.  */
    local_time.seconds = client_ptr -> nx_sntp_client_local_ntp_time.seconds +
        client_ptr -> nx_sntp_client_local_ntp_time_elapsed;
    local_time.fraction = client_ptr -> nx_sntp_client_local_ntp_time.fraction;
    _nx_sntp_client_utility_convert_time_to_UCHAR(&local_time, unicast_request,  TRANSMIT_TIME);

    /* Create the request packet with the unicast request message.  */
    status = _nx_sntp_client_create_time_request_packet(client_ptr, packet_ptr, unicast_request);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);


        /* Release the mutex. */
        tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

        /* Return error status.  */
        return status;
    }

    /* Release the lock. we're done with the SNTP Client data. */
    tx_mutex_put(&client_ptr -> nx_sntp_client_mutex);

    /* Send the time request packet.  */
    status =  nxd_udp_socket_send(&(client_ptr -> nx_sntp_client_udp_socket), packet_ptr, &client_ptr -> nx_sntp_server_ip_address, NX_SNTP_SERVER_UDP_PORT);


    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return status;
    }

    send_timerticks = tx_time_get();

    /* Return completion status.  */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_receive_time_update                 PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function receives UDP data on the Client socket with specified */
/*    wait (timeout option).  Received packets are checked for valid      */
/*    and sender IP and port.  Packet data is then extracted and stored   */
/*    in the specified time message. The packet is released regardless of */
/*    validity of packet received.                                        */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to Client                 */ 
/*    timeout                           Wait option to receive packet     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_receive             Listen for packets on UDP socket  */ 
/*    nx_packet_release                 Release packet back to packet pool*/ 
/*    tx_mutex_get                      Obtain exclusive lock on list     */
/*    memset                            Clear specified area of memory    */
/*    _nx_sntp_client_add_server_ULONG_to_list                            */
/*                                      Add sender address to Client list */
/*    _nx_sntp_client_utility_convert_LONG_to_IP                          */
/*                                     Convert IP to string for display   */
/*    _nx_sntp_client_extract_time_message_from_packet                    */
/*                                     Extract packet data to time message*/
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_run_unicast     Send and receive server time updates*/
/*    _nx_sntp_client_run_broadcast   Listen for server update broadcasts */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added support for disabling */
/*                                            message check,              */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the port check,   */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_sntp_client_receive_time_update(NX_SNTP_CLIENT *client_ptr, ULONG timeout)
{

UINT            status;
UINT            length;
NX_PACKET       *receive_packet;
#ifndef NX_SNTP_CLIENT_MESSAGE_CHECK_DISABLE
UINT            sender_port;
NX_UDP_HEADER   *udp_header_ptr;
NXD_ADDRESS     source_ip_address, destination_ip_address;
#endif /* NX_SNTP_CLIENT_MESSAGE_CHECK_DISABLE */

    /* Loop to receive packets. */
    for(;;)
    {

        /* Indicate the SNTP Client can be stopped while the SNTP client waits for a packet.  */
        client_ptr -> nx_sntp_client_sleep_flag =  NX_TRUE;

        /* Wait to receive packets on the Client UDP socket.  */
        status = nx_udp_socket_receive(&(client_ptr -> nx_sntp_client_udp_socket), &receive_packet, timeout * NX_IP_PERIODIC_RATE);

        /* Indicate the SNTP Client can not be stopped.  */
        client_ptr -> nx_sntp_client_sleep_flag =  NX_FALSE;

        /* Check for error.  */
        if (status != NX_SUCCESS)
        {

            /* Return error status.  */
            return status;
        }

#ifndef NX_SNTP_CLIENT_MESSAGE_CHECK_DISABLE
        /* Check the sender port in the UDP packet header.  */  
        udp_header_ptr = (NX_UDP_HEADER *)(receive_packet -> nx_packet_prepend_ptr - sizeof(NX_UDP_HEADER));
        sender_port = (UINT)udp_header_ptr -> nx_udp_header_word_0 >> NX_SHIFT_BY_16; 

        /* Check if this is the server port the Client expects.  */
        if (sender_port != NX_SNTP_SERVER_UDP_PORT) 
        {

            /* No, reject the packet.  */
            nx_packet_release(receive_packet);

            continue;
        }


        /* If this an IPv6 packet? An SNTP client with an IPv6 SNTP server will only accept IPv6 packets.   */
        if ((receive_packet -> nx_packet_ip_version == NX_IP_VERSION_V6) && 
            (client_ptr -> nx_sntp_server_ip_address.nxd_ip_version == NX_IP_VERSION_V6))
        {

#ifndef FEATURE_NX_IPV6
            nx_packet_release(receive_packet);
            continue;
#else
            NX_IPV6_HEADER *ipv6_header_ptr;

            /* Pick up the IPv6 header from the packet. */
            ipv6_header_ptr =  (NX_IPV6_HEADER *) (receive_packet -> nx_packet_prepend_ptr - sizeof(NX_UDP_HEADER) - sizeof(NX_IPV6_HEADER));

            /* Copy sender address into a local variable. */
            COPY_IPV6_ADDRESS(ipv6_header_ptr -> nx_ip_header_source_ip, &(source_ip_address.nxd_ip_address.v6[0]));
            source_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

            /* Copy destination address into a local variable. */
            COPY_IPV6_ADDRESS(ipv6_header_ptr -> nx_ip_header_destination_ip, &(destination_ip_address.nxd_ip_address.v6[0]));
            destination_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

            if (client_ptr -> nx_sntp_client_protocol_mode == UNICAST_MODE)
            {

                /* Does the source address match the expected server address?  */
                if (!CHECK_IPV6_ADDRESSES_SAME(&client_ptr -> nx_sntp_server_ip_address.nxd_ip_address.v6[0], &source_ip_address.nxd_ip_address.v6[0]))
                {
        
                    /* No further need for the receive packet. Release back to the client pool.  */
                    nx_packet_release(receive_packet);
        
                    /* Set update time to null.  */
                    memset(&(client_ptr -> nx_sntp_server_update_time), 0, sizeof(NX_SNTP_TIME));
        
                    continue;
                }
            }
            else /* Multicast mode */
            {

                /* Define the all hosts multicast address. */
                NXD_ADDRESS All_Hosts_Multicast;
                All_Hosts_Multicast.nxd_ip_version = NX_IP_VERSION_V6;
                All_Hosts_Multicast.nxd_ip_address.v6[0] = 0xff020000;  
                All_Hosts_Multicast.nxd_ip_address.v6[1] = 0x00000000;
                All_Hosts_Multicast.nxd_ip_address.v6[2] = 0x00000000;
                All_Hosts_Multicast.nxd_ip_address.v6[3] = 0x00000001;

                /* Does the source address match the expected server address )?  */
                if (!CHECK_IPV6_ADDRESSES_SAME(&client_ptr -> nx_sntp_server_ip_address.nxd_ip_address.v6[0], &source_ip_address.nxd_ip_address.v6[0]))
                {
        
                    /* No further need for the receive packet. Release back to the client pool.  */
                    nx_packet_release(receive_packet);
        
                    /* Set update time to null.  */
                    memset(&(client_ptr -> nx_sntp_server_update_time), 0, sizeof(NX_SNTP_TIME));
        
                    continue;
                }

                /* Does the destination address match the expected server address (all hosts address FF02::1)?  */
                if (!CHECK_IPV6_ADDRESSES_SAME(&destination_ip_address.nxd_ip_address.v6[0], &All_Hosts_Multicast.nxd_ip_address.v6[0]))
                {
        
                    /* No further need for the receive packet. Release back to the client pool.  */
                    nx_packet_release(receive_packet);
        
                    /* Set update time to null.  */
                    memset(&(client_ptr -> nx_sntp_server_update_time), 0, sizeof(NX_SNTP_TIME));
        
                    continue;
                }

            }
#endif  /* FEATURE_NX_IPV6 */

        }
        /* Otherwise is this an IPv4 packet and is the SNTP client set for IPv4? */
        else if ((receive_packet -> nx_packet_ip_version == NX_IP_VERSION_V4) && 
                 (client_ptr -> nx_sntp_server_ip_address.nxd_ip_version == NX_IP_VERSION_V4))
        {
#ifdef NX_DISABLE_IPV4
            nx_packet_release(receive_packet);
            continue;
#else

            NX_IPV4_HEADER *ipv4_header_ptr;

            ipv4_header_ptr =  (NX_IPV4_HEADER *) (receive_packet -> nx_packet_prepend_ptr - sizeof(NX_UDP_HEADER) - sizeof(NX_IPV4_HEADER));

            /* Copy into a local variable. */
            source_ip_address.nxd_ip_address.v4 =  ipv4_header_ptr -> nx_ip_header_source_ip;
            source_ip_address.nxd_ip_version = NX_IP_VERSION_V4;

            /* Copy into a local variable. */
            destination_ip_address.nxd_ip_address.v4 =  ipv4_header_ptr -> nx_ip_header_destination_ip;
            destination_ip_address.nxd_ip_version = NX_IP_VERSION_V4;

            if (client_ptr -> nx_sntp_client_protocol_mode == UNICAST_MODE)
            {
                /* Compare the sender address with the Client's current sntp server. */
                if (source_ip_address.nxd_ip_address.v4 != client_ptr -> nx_sntp_server_ip_address.nxd_ip_address.v4)
                {
                    /* This is an untrusted server or the client had a broadcast server already,
                       reject this packet and return an error condition.  */
        
                    /* No further need for the receive packet. Release back to the client pool.  */
                    nx_packet_release(receive_packet);
        
                    continue;
                }
            }
            else /* BROADCAST_MODE */
            {

                ULONG network_mask;
                UINT  iface_index;

                /* Compare the sender address with the Client's current sntp server. */
                if (source_ip_address.nxd_ip_address.v4 != client_ptr -> nx_sntp_server_ip_address.nxd_ip_address.v4)
                {
                
                    /* This is an untrusted server or the client had a broadcast server already,
                       reject this packet and return an error condition.  */

                    /* No further need for the receive packet. Release back to the client pool.  */
                    nx_packet_release(receive_packet);

                    continue;
                }

                /* Set a local variable on the SNTP network index. */
                iface_index = client_ptr -> nx_sntp_client_interface_index;

                /* Set a local variable to the network mask. */
                network_mask = client_ptr -> nx_sntp_client_ip_ptr -> nx_ip_interface[iface_index].nx_interface_ip_network_mask;

                /* Now verify this is a broadcast packet. */
                if ((destination_ip_address.nxd_ip_address.v4 & ~network_mask) != ~network_mask)
                {                                        
                    /* This is not a broadcast packet on the local network.  */
        
                    /* No further need for the receive packet. Release back to the client pool.  */
                    nx_packet_release(receive_packet);
        
                    continue;
                }
            }
#endif /* NX_DISABLE_IPV4 */
        }
        else
        {
            /* Not interested, discard the packet. */
            nx_packet_release(receive_packet);

            continue;
        }

#endif /* NX_SNTP_CLIENT_MESSAGE_CHECK_DISABLE */

        /* Check that the packet has the proper length for an NTP message.  */
        length = receive_packet -> nx_packet_length;

        /* Verify the packet data contains at least the minimum time update data,
           but no more than the max size with authentication data appended.  */

        if ((length < NX_SNTP_TIME_MESSAGE_MIN_SIZE) || (length > NX_SNTP_TIME_MESSAGE_MAX_SIZE))
        {

            /* No further need for the receive packet. Release back to the client pool.  */
            nx_packet_release(receive_packet);

            continue;
        }

        memset(&(client_ptr -> nx_sntp_current_server_time_message), 0, sizeof(NX_SNTP_TIME_MESSAGE));

        /* Extract time message data from packet data into time message.  */
        _nx_sntp_client_extract_time_message_from_packet(client_ptr, receive_packet);

        /* No further need for the receive packet. Release back to the client pool.  */
        nx_packet_release(receive_packet);

        return NX_SUCCESS;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_extract_time_message_from_packet    PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts time data from an SNTP  server packet and    */
/*    copies the information into the specified time message buffer.      */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                          Pointer to SNTP Client          */
/*    packet_ptr                          Pointer to server packet        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcpy                              Copy data to area of memory     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_receive_time_update Process SNTP server packets     */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), corrected*/
/*                                            the Reference Identifier,  */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_sntp_client_extract_time_message_from_packet(NX_SNTP_CLIENT *client_ptr, NX_PACKET *packet_ptr) 
{

ULONG   *ntp_word_0;    
NX_SNTP_TIME_MESSAGE *time_message_ptr;


    time_message_ptr = &(client_ptr -> nx_sntp_current_server_time_message);

    process_timerticks = tx_time_get();

    memset(time_message_ptr, 0, sizeof(NX_SNTP_TIME_MESSAGE));

    /* Pickup the pointer to the head of the UDP packet.  */
    ntp_word_0 = (ULONG *)packet_ptr -> nx_packet_prepend_ptr;   
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);

    /* Extract fields from the first 32 bits of the time message.  */
    time_message_ptr -> flags = (*ntp_word_0 & 0xFF000000UL) >> 24;

    time_message_ptr -> peer_clock_stratum = (*ntp_word_0 & 0x00FF0000UL) >> 16; 

    time_message_ptr -> peer_poll_interval = (*ntp_word_0 & 0x0000FF00UL) >> 8;

    time_message_ptr -> peer_clock_precision =  (*ntp_word_0 & 0x00000000FFUL);

    /* Advance to the next 32 bit field and extract the root delay field.  */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);

    time_message_ptr -> root_delay = (ULONG)(*ntp_word_0);

    /* Advance to the next 32 bit field and extract the clock dispersion field.  */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);

    time_message_ptr -> clock_dispersion= *ntp_word_0;    

    /* Advance to the next 32 bit field and extract the reference clock ID field.  */
    ntp_word_0++;
    memcpy(time_message_ptr -> reference_clock_id, ntp_word_0, 4); /* Use case of memcpy is verified. */

    /* Advance to the next field (64 bit field) and extract the reference time stamp field.  */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);
    time_message_ptr -> reference_clock_update_time_stamp[0] = *ntp_word_0;

    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);
    time_message_ptr -> reference_clock_update_time_stamp[1] = *ntp_word_0;

    /* If a non zero reference time was supplied, separate out seconds and fraction.  */
    if (_nx_sntp_client_utility_is_zero_data((UCHAR *)(time_message_ptr -> reference_clock_update_time_stamp), 8) == NX_FALSE)
    {

        time_message_ptr -> reference_clock_update_time.seconds = time_message_ptr -> reference_clock_update_time_stamp[0];

        time_message_ptr -> reference_clock_update_time.fraction = time_message_ptr -> reference_clock_update_time_stamp[1];
    }

    /* Copy the first 32 bits into the originate time stamp seconds field. */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);
    time_message_ptr -> originate_time_stamp[0] = *ntp_word_0;

    /* Copy the next 32 bits into the originate time stamp fraction field. */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);
    time_message_ptr -> originate_time_stamp[1] = *ntp_word_0;

    /* If an originate time was supplied, separate out seconds and fraction.  */
    if (_nx_sntp_client_utility_is_zero_data((UCHAR *)(time_message_ptr -> originate_time_stamp), 8) == NX_FALSE)
    {

        time_message_ptr -> originate_time.seconds = time_message_ptr -> originate_time_stamp[0];

        time_message_ptr -> originate_time.fraction = time_message_ptr -> originate_time_stamp[1];
    }


    /* Copy the first 32 bits into the receive time stamp seconds field. */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);
    time_message_ptr -> receive_time_stamp[0] = *ntp_word_0;

    /* Copy the next 32 bits into the receive time stamp fraction field. */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);
    time_message_ptr -> receive_time_stamp[1] = *ntp_word_0;

    /* If an receive time was supplied, separate out seconds and fraction.  */
    if (_nx_sntp_client_utility_is_zero_data((UCHAR *)(time_message_ptr -> receive_time_stamp), 8) == NX_FALSE)
    {

        time_message_ptr -> receive_time.seconds = time_message_ptr -> receive_time_stamp[0];
        time_message_ptr -> receive_time.fraction = time_message_ptr -> receive_time_stamp[1];
    }

    /* Copy the first 32 bits into the transmit time stamp seconds field. */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);
    time_message_ptr -> transmit_time_stamp[0] = *ntp_word_0;

    /* Copy the next 32 bits into the transmit time stamp fraction field. */
    ntp_word_0++;
    NX_CHANGE_ULONG_ENDIAN(*ntp_word_0);
    time_message_ptr -> transmit_time_stamp[1] = *ntp_word_0;

    /* If an transmit time was supplied, separate out seconds and fraction.  */
    if (_nx_sntp_client_utility_is_zero_data((UCHAR *)(time_message_ptr -> transmit_time_stamp), 8) == NX_FALSE)
    {

        time_message_ptr -> transmit_time.seconds = time_message_ptr -> transmit_time_stamp[0];

        time_message_ptr -> transmit_time.fraction = time_message_ptr -> transmit_time_stamp[1];

    }

    /* Return completion status.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_reset_current_time_message          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function saves the current time message from the server to     */
/*    the previous time message field, and clears the current time message*/
/*    field.                                                              */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                          Pointer to Client               */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                              Clear specified area of memory  */ 
/*    memcpy                              Copy data to area of memory     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_receive_time_update Process received update packets */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_sntp_client_reset_current_time_message(NX_SNTP_CLIENT *client_ptr)
{

    /* Does the client have a non NULL current server time update?  */
    if (client_ptr -> nx_sntp_current_server_time_message.flags)
    {

        /* Yes, copy to the previous time update.  */
        memcpy(&client_ptr -> nx_sntp_previous_server_time_message, &client_ptr -> nx_sntp_current_server_time_message, sizeof(NX_SNTP_TIME_MESSAGE)); /* Use case of memcpy is verified. */

        /* Clear the current time update data.  */
        memset(&(client_ptr -> nx_sntp_current_server_time_message), 0, sizeof(NX_SNTP_TIME_MESSAGE));

    }

    /* Clear the current time update request.  */
    memset(&(client_ptr -> nx_sntp_current_time_message_request), 0, sizeof(NX_SNTP_TIME_MESSAGE));

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_process_update_packet               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes SNTP time data received in a server update  */
/*    packet, applies sanity checks, calculates round trip time, and      */
/*    updates the Client time with the SNTP server time.                  */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                          Pointer to Client               */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_SNTP_BAD_SERVER_ROOT_DISPERSION  Server dispersion too high      */
/*    receive_status                      Actual packet receive status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcmp                              Copy data to area of memory     */ 
/*    _nx_sntp_client_apply_sanity_checks Apply sanity checks to update   */ 
/*    _nx_sntp_client_check_server_clock_dispersion                       */
/*                                        Check server clock dispersion   */
/*    _nx_sntp_client_calculate_roundtrip                                 */
/*                                        Compute roundtrip time to server*/
/*   _nx_sntp_client_process_time_data    Apply server time to local time */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_process_unicast     Process unicast SNTP data       */
/*    _nx_sntp_client_process_broadcast   Process broadcast SNTP data     */
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
UINT  _nx_sntp_client_process_update_packet(NX_SNTP_CLIENT *client_ptr)
{

UINT    status;

    /* Apply client's set of sanity checks on the data.  */
    status = _nx_sntp_client_apply_sanity_checks(client_ptr);     

    /* Check for sanity check errors.  */
    if (status != NX_SUCCESS)
    {

        /* Count these  as bad updates from the server.  */
        client_ptr -> nx_sntp_client_invalid_time_updates++;

        /* Ok to continue Client task.  */
        return status;
    }

    /* Check server clock dispersion (if any reported) in first update 
       from current server for unicast clients.  */
    if ((client_ptr -> nx_sntp_client_first_update_pending == NX_TRUE) && (client_ptr -> nx_sntp_client_protocol_mode == UNICAST_MODE)) 
    {

        status = _nx_sntp_client_check_server_clock_dispersion(client_ptr);
    
        /* Is the server clock variance acceptable?  */
        if (status != NX_SUCCESS)
        {

            /* Return the error status.  */
            return status;
        }
    }

    /* At this point we have received a valid server response.  */

    /* Clear the count of (consecutive) bad time updates received from this server.  */
    client_ptr -> nx_sntp_client_invalid_time_updates = 0;

    /* Apply server time to local device time.  */
    status = _nx_sntp_client_process_time_data(client_ptr); 

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error condition*/
        return status;
    }

    /* If the Client is configured with a time update callback, call it now. */
    if (client_ptr -> nx_sntp_client_time_update_notify)
    {

        client_ptr -> nx_sntp_client_time_update_notify(&(client_ptr -> nx_sntp_current_server_time_message), 
                                                        &(client_ptr -> nx_sntp_client_local_ntp_time)); 
    }

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_duplicate_update_check              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs a comparison of time values between two      */
/*    time messages to verify they are not duplicates.  Duplicate updates */
/*    can be a form of denial of service/clogging attack on a host.       */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    timeA_msg_ptr                       Pointer to first time update    */ 
/*    timeB_msg_ptr                       Pointer to second time update   */
/*    is_a_dupe                           Result of comparison            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcmp                              Copy data to area of memory     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_apply_sanity_checks Verify received SNTP data       */
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
UINT  _nx_sntp_client_duplicate_update_check(NX_SNTP_TIME_MESSAGE *timeA_msg_ptr, 
                                            NX_SNTP_TIME_MESSAGE *timeB_msg_ptr, 
                                            UINT *is_a_dupe)
{

    *is_a_dupe = NX_FALSE;

    if ( (!memcmp(timeA_msg_ptr -> transmit_time_stamp, 
                  timeB_msg_ptr -> transmit_time_stamp, 8))                               &&
         (!memcmp(timeA_msg_ptr -> receive_time_stamp, 
                  timeB_msg_ptr -> receive_time_stamp, 8))                                &&
         (!memcmp(timeA_msg_ptr -> originate_time_stamp, 
                  timeB_msg_ptr -> originate_time_stamp, 8))                              &&
         (!memcmp(timeA_msg_ptr -> reference_clock_update_time_stamp, 
                  timeB_msg_ptr -> reference_clock_update_time_stamp, 8))                 &&
         (!memcmp(timeA_msg_ptr -> reference_clock_id,                                    
                  timeB_msg_ptr -> reference_clock_id, 4))                                &&
         (timeA_msg_ptr -> root_delay == timeB_msg_ptr -> root_delay)                     &&
         (timeA_msg_ptr -> clock_dispersion == timeB_msg_ptr -> clock_dispersion)
       )
    {

        *is_a_dupe = NX_TRUE;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_apply_sanity_checks                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for invalid SNTP data received from server.    */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SNTP_DUPE_SERVER_PACKET       Duplicate server packet received   */ 
/*    NX_SNTP_INVALID_SERVER_MODE      Server mode won't work with Client */ 
/*    NX_SNTP_INVALID_NTP_VERSION      Server NTP version won't work with */
/*                                       Client NTP version.              */
/*    NX_SNTP_SERVER_CLOCK_NOT_SYNC    Server is not ready to send updates*/
/*    NX_SNTP_INVALID_SERVER_STRATUM   Server stratum not acceptable      */
/*    NX_SNTP_NULL_SERVER_TIMESTAMP    Invalid or NULL timestamp in update*/
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcmp                           Copy data into area of memory      */
/*    _nx_sntp_client_duplicate_update_check                              */ 
/*                                     Check for duplicate updates service*/
/*    _nx_sntp_client_utility_get_msec_diff                               */
/*                                     Check for time difference in msecs */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_run_broadcast    Receive broadcast SNTP updates     */ 
/*    _nx_sntp_client_run_unicast      Send and receive SNTP updates      */ 
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
UINT  _nx_sntp_client_apply_sanity_checks(NX_SNTP_CLIENT *client_ptr)
{

    
UINT    status;
UINT    mode;
UINT    leap_second_indicator;
UINT    server_ntp_version;
UINT    is_a_dupe_check;
ULONG   msecs;   
NX_SNTP_TIME_MESSAGE *server_time_msg_ptr;

    server_time_msg_ptr = &(client_ptr -> nx_sntp_current_server_time_message);

    /* Was a previous response received from the server?  */
    if (client_ptr -> nx_sntp_previous_server_time_message.flags)
    {

        /* Yes; perform a duplicate packet check.  */
        status = _nx_sntp_client_duplicate_update_check(&(client_ptr -> nx_sntp_current_server_time_message),
                                                        &(client_ptr -> nx_sntp_previous_server_time_message),
                                                        &is_a_dupe_check);
        /* Check for error.  */
        if (status != NX_SUCCESS)
        {

            /* Return error status.  */
            return status;
        }

        /* Is this a duplicate packet (e.g. possible 'clogging' attack)?  */
        if (is_a_dupe_check)
        {

            return NX_SNTP_DUPE_SERVER_PACKET;
        }
    }

    /* Get the association type from the server.  */
    mode = server_time_msg_ptr ->flags & 0x07;

    /* Is the server of the correct association for a unicast client?  */
    if ((mode != PROTOCOL_MODE_SERVER_UNICAST) && (client_ptr -> nx_sntp_client_protocol_mode == UNICAST_MODE))
    {

        /* Return error status.  */
        return NX_SNTP_INVALID_SERVER_MODE;
    }
    /* Is the server of the correct association for a broadcast client?  */
    else if ((mode != PROTOCOL_MODE_SERVER_BROADCAST) && (client_ptr -> nx_sntp_client_protocol_mode == BROADCAST_MODE))
    {

        /* Return error status.  */
        return NX_SNTP_INVALID_SERVER_MODE;
    }

    /* Extract server's NTP version from the flags field.  */
    server_ntp_version = (server_time_msg_ptr -> flags & 0x38) >> 3;

    /* As per NTP protocol, check if the server reply has the same SNTP version that 
      the Client sent in its unicast request.  */
    if ((server_ntp_version != NX_SNTP_CLIENT_NTP_VERSION) && (client_ptr -> nx_sntp_client_protocol_mode == UNICAST_MODE))
    {

        /* Return the error condition.  */
        return NX_SNTP_INVALID_NTP_VERSION;
    }
    /* If broadcast, verify the server SNTP version is compatible with the Client.  */
    else if ((server_ntp_version < NX_SNTP_CLIENT_MIN_NTP_VERSION) && (client_ptr -> nx_sntp_client_protocol_mode == BROADCAST_MODE))
    {
        
        /* Return the error condition.  */
        return NX_SNTP_INVALID_NTP_VERSION;
    }

    /* Get the leap second indicator from the flags field.  */
    leap_second_indicator = (server_time_msg_ptr -> flags & 0xC0) >> 6;

    /* Is the server clock not yet synchronized?  */
    if (leap_second_indicator == 3)
    {
        /* Return the error condition.  */
        return NX_SNTP_SERVER_CLOCK_NOT_SYNC;
    }

    /* Has the server sent a 'kiss of death' (stratum = 0) packet?  */
    if (server_time_msg_ptr -> peer_clock_stratum == 0) 
    {

        /* Does the Client have a kiss of death handler?  */
        if (client_ptr -> kiss_of_death_handler)
        {
            UINT code_id;

            /* Convert the message to code.*/
            _nx_sntp_client_utility_convert_refID_KOD_code(server_time_msg_ptr -> reference_clock_id, &code_id);

            /* Yes, let the handler deal with this server response.  */
            status = (client_ptr -> kiss_of_death_handler)(client_ptr, code_id);

            /* Is it ok to continue with this server?  */
            if (status != NX_SUCCESS)
            {

                /* No, it is not. Return the error condition.  */
                return status;
            }
        }
        else
        {
            /* It is not ok to continue. Return the error condition. */
            return NX_SNTP_KOD_SERVER_NOT_AVAILABLE;
        }
    }

    /* Is the server stratum a valid stratum ?  */
    else if (server_time_msg_ptr -> peer_clock_stratum & STRATUM_RESERVED)
    {

        /* Return error status.  */
        return NX_SNTP_INVALID_SERVER_STRATUM;

    }
    /* Does the server stratum acceptable to the Client?  */
    else if (server_time_msg_ptr -> peer_clock_stratum > NX_SNTP_CLIENT_MIN_SERVER_STRATUM)
    {

        /* Return error status.  */
        return NX_SNTP_INVALID_SERVER_STRATUM;
    }

    /* Is the Client operating in unicast mode?  */
    if (client_ptr -> nx_sntp_client_protocol_mode == UNICAST_MODE)
    {
    UINT pos_diff = NX_TRUE;

        /* Yes; Are any server time stamps NULL?  */

        /* Special case: the client has not set its local time. Only check server's receive and transmit time. */
        if (client_ptr -> nx_sntp_client_local_ntp_time.seconds == 0)
        {
            if ((_nx_sntp_client_utility_is_zero_data((UCHAR *)(server_time_msg_ptr -> receive_time_stamp), 8) == NX_TRUE) ||
                (_nx_sntp_client_utility_is_zero_data((UCHAR *)(server_time_msg_ptr -> transmit_time_stamp), 8) == NX_TRUE))
            {

                /* Return error status.  */
                return NX_SNTP_INVALID_TIMESTAMP;
            }

        }
        else
        {
            /* Check all server time stamps to be non null.  */
            if ((_nx_sntp_client_utility_is_zero_data((UCHAR *)(server_time_msg_ptr -> originate_time_stamp), 8) == NX_TRUE) || 
                (_nx_sntp_client_utility_is_zero_data((UCHAR *)(server_time_msg_ptr -> receive_time_stamp), 8) == NX_TRUE) ||
                (_nx_sntp_client_utility_is_zero_data((UCHAR *)(server_time_msg_ptr -> transmit_time_stamp), 8) == NX_TRUE))
            {
    
                /* Return error status.  */
                return NX_SNTP_INVALID_TIMESTAMP;
            }

        }

        /* Get the time difference  server transmit time - server receive time.  */
        status = _nx_sntp_client_utility_get_msec_diff(&(server_time_msg_ptr -> receive_time), 
                                                       &(server_time_msg_ptr -> transmit_time), 
                                                       &msecs, &pos_diff);

        /* Is the server transmit time <= server receive time [physically not possible]?  */
        if (status != NX_SUCCESS)
        {

            /* Yes; Return the error condition.  */
            return status;        
        }
        else if (pos_diff == NX_FALSE)
        {

            /* treat as an error in this case. */
            return NX_SNTP_INVALID_TIMESTAMP;
        }
    }
    /* Client is in broadcast mode.  */
    else
    {

        /* Is the broadcast server transmit time stamp is NULL?  */
        if (_nx_sntp_client_utility_is_zero_data((UCHAR *)(server_time_msg_ptr -> transmit_time_stamp), 8) == NX_TRUE)

        {

            /* Return the error status.  */
            return NX_SNTP_INVALID_TIMESTAMP;
        }
    }

    /* Is there an impending leap second adjustment?  */
    if ((leap_second_indicator == 1) || (leap_second_indicator == 2))
    {

        /* Does the Client have a leap second handler?*/
        if (client_ptr -> leap_second_handler)
        {

            /* Yes, invoke the leap second handler for impending leap second event.  */
            (client_ptr -> leap_second_handler)(client_ptr, leap_second_indicator);
        }
    }
 
    /* Return successful completion status.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_check_server_clock_dispersion       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks server clock dispersion extracted from server  */
/*    time message in 32 bit representation:                              */
/*        sum of 1/(2 ^ bit) of bits 16-31 in the time message format     */
/*    and compares it against the Client's dispersion tolerance.          */
/*                                                                        */ 
/*    If NX_SNTP_CLIENT_MAX_ROOT_DISPERSION is set to zero, the function  */
/*    simply returns a successful result but does no calculations.        */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                         Pointer to Client                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         dispersion is acceptable         */ 
/*    NX_SNTP_BAD_SERVER_ROOT_DISPERSION dispersion is too large          */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_process_update_packet                               */
/*                                    Process time data in update packets */
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

UINT  _nx_sntp_client_check_server_clock_dispersion(NX_SNTP_CLIENT *client_ptr)
{

#if (NX_SNTP_CLIENT_MAX_ROOT_DISPERSION != 0)
UINT    mask = 0x8000;
UINT    bit = 1;
ULONG   divisor;
ULONG   dispersion_usecs;

    /* Check for zero clock dispersion reported in server update.  */
    if (client_ptr -> nx_sntp_current_server_time_message.clock_dispersion == 0)
    {

        /* Return successful status.  */
        return NX_SUCCESS; 
    }

    /* Get the most significant 16 bits of dispersion field and convert to microseconds.*/
    dispersion_usecs = (client_ptr -> nx_sntp_current_server_time_message.clock_dispersion >> 16) * 1000000;

    /* Compare seconds in the most significant 16 bits against the Client dispersion tolerance.  */
    if (dispersion_usecs > NX_SNTP_CLIENT_MAX_ROOT_DISPERSION)
    {

        /* Yes, indicate that server clock dispersion exceeds tolerance.  */
        return NX_SNTP_BAD_SERVER_ROOT_DISPERSION;
    }

    /* Compare fraction from least significant 16 bits; sum of (1 / 2 ^ bit field) of lower 16 bits.  */

    /* Get the lower 16 bits.  */
    client_ptr -> nx_sntp_current_server_time_message.clock_dispersion =  
        client_ptr -> nx_sntp_current_server_time_message.clock_dispersion & 0xFFFF;

    /* Compute the dispersion.  */
    while(mask)
    {
        if (mask & client_ptr -> nx_sntp_current_server_time_message.clock_dispersion)
        {
            divisor = (ULONG)(1 << bit);
            if ((divisor < 1000000) && (divisor > 0))
            {
                dispersion_usecs += 1000000/divisor;                
            }
        }
        bit++;
        mask >>= 1;
    }

    /* Is the computed clock dispersion more than the max tolerance?  */
    if (dispersion_usecs > NX_SNTP_CLIENT_MAX_ROOT_DISPERSION)
    {
        /* Yes, indicate that server clock dispersion exceeds tolerance.  */
        return NX_SNTP_BAD_SERVER_ROOT_DISPERSION;
    }
#else
    NX_PARAMETER_NOT_USED(client_ptr);
#endif /* (NX_SNTP_CLIENT_MAX_ROOT_DISPERSION != 0) */

    /* Return successful computation.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_thread_entry                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the processing thread for the SNTP Client. It      */
/*    executes periodic SNTP tasks with SNTP Client mutex protection, then*/
/*    briefly sleeps to allow the host application to call SNTP Client    */
/*    services.                                                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    sntp_instance                         Pointer to SNTP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_process               Main SNTP processing function */ 
/*    tx_thread_sleep                       Sleep for specified time      */ 
/*    tx_mutex_get                          Get the SNTP mutex            */ 
/*    tx_mutex_put                          Release the SNTP mutex        */ 
/*    tx_thread_preemption_change           Change thread preemption      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
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
VOID  _nx_sntp_client_thread_entry(ULONG sntp_instance)
{

NX_SNTP_CLIENT *client_ptr;
UINT    status;
UINT    current_preemption;


    /* Setup the SNTP pointer.  */
    client_ptr =    (NX_SNTP_CLIENT *) sntp_instance;

    /* Enter while loop.  */
    do
    {
        
        /* Loop for obtaining the SNTP mutex.  */
        do
        {

            /* Get the SNTP mutex.  */
            status =  tx_mutex_get(&(client_ptr -> nx_sntp_client_mutex), TX_WAIT_FOREVER);

        } while (status != TX_SUCCESS);

        /* Perform periodic tasks for the SNTP Client.  */
        _nx_sntp_client_process(client_ptr);

         /* Release the SNTP mutex.  */
        tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

        /* Disable preemption for critical section.  */
        tx_thread_preemption_change(tx_thread_identify(), 0, &current_preemption);

        /* Indicate the SNTP Client process is idle. */
        client_ptr -> nx_sntp_client_sleep_flag =  NX_TRUE;

        /* Sleep for timer interval.  */
        tx_thread_sleep(NX_SNTP_CLIENT_SLEEP_INTERVAL);

        /* Clear flag to indicate SNTP thread is not in a position to be stopped.  */
        client_ptr -> nx_sntp_client_sleep_flag =  NX_FALSE;

        /* Restore original preemption.  */
        tx_thread_preemption_change(tx_thread_identify(), current_preemption, &current_preemption);


    } while (1);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_receive_notify                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the socket receive callback for the SNTP Client  */
/*    UDP socket.  It sets a flag for the SNTP Client thread to know there*/
/*    is a packet waiting to be processed.                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                           Pointer to Client UDP socket   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Sets a receive event          */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
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
VOID  _nx_sntp_client_receive_notify(NX_UDP_SOCKET *socket_ptr)
{
    NX_PARAMETER_NOT_USED(socket_ptr);

    /* Wakeup all threads that are attempting to perform a receive or that had their select satisfied.  */
    tx_event_flags_set(&nx_sntp_client_events, NX_SNTP_CLIENT_RECEIVE_EVENT, TX_OR);

    receive_timerticks = tx_time_get();

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_process                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is called periodically by the SNTP client thread, and */
/*    depending on the CLient mode (unicast or broadcast) calls the       */
/*    unicast or broadcast process function.                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to the SNTP client    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_process_unicast       Process unicast tasks         */ 
/*    _nx_sntp_client_process_broadcast     Process broadcast tasks       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
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
VOID  _nx_sntp_client_process(NX_SNTP_CLIENT *client_ptr)
{


    /* Is the client using unicast or unicast to receive updates? */
    if (client_ptr -> nx_sntp_client_protocol_mode == UNICAST_MODE)
    {
        /* Call the unicast process function. */
        _nx_sntp_client_process_unicast(client_ptr);
    }
    else
    {

        /* Call the broadcast process function. */
        _nx_sntp_client_process_broadcast(client_ptr);
    }

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_process_time_data                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function applies the server time data to client local time taking*/
/*  into account the round trip time.  Time updates are checked for a max */
/*  time offset set by the Client max_time_adjustment parameter.          */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to Client                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_SNTP_INVALID_SERVER_UPDATE_TIME No time of server update recorded*/ 
/*    status                            Actual completion status          */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                           Clear specified area of memory     */ 
/*    memcpy                           Copy data to area of memory        */
/*    _nx_sntp_client_utility_add_msecs_to_ntp_time                       */
/*                                     Add msec to NTP time fraction field*/
/*    _nx_sntp_client_utility_get_msec_diff                               */
/*                                     Compute time difference in msecs   */
/*    _nx_sntp_client_utility_add_msecs_to_ntp_time                       */
/*                                     Add time in msecs to an NTP time   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_run_broadcast   Listen and process broadcast updates*/
/*    _nx_sntp_client_process_update_packet                               */
/*                                    Process update SNTP packets         */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_sntp_client_process_time_data(NX_SNTP_CLIENT *client_ptr)
{

UINT            status;
UINT            ignore_max_adjustment_limit;
ULONG           elapsed_msecs_difference;
UINT            adjustment;
NX_SNTP_TIME    local_time;


    /* Copy the received time update to the update time just received from the server. */
    memcpy(&client_ptr -> nx_sntp_server_update_time, &client_ptr -> nx_sntp_current_server_time_message.transmit_time, sizeof(NX_SNTP_TIME)); /* Use case of memcpy is verified. */

    /* Check if the client is configured for round trip time calculation.  */
    if (NX_SNTP_CLIENT_RTT_REQUIRED == NX_TRUE)
    {
    
        /* Compute roundtrip delay. */
        _nx_sntp_client_calculate_roundtrip(&(client_ptr -> nx_sntp_client_roundtrip_time_msec));

        /* Has the client computed a valid round trip time?  */
        if (client_ptr -> nx_sntp_client_roundtrip_time_msec)
        {
    
            /* Yes, Add 1/2 round trip to server's reported time.  */
            status =_nx_sntp_client_utility_add_msecs_to_ntp_time(&(client_ptr -> nx_sntp_current_server_time_message.transmit_time), 
                                                                   ((client_ptr -> nx_sntp_client_roundtrip_time_msec) / 2));

            if (status != NX_SUCCESS)
            {
    
                /* Cannot use this time update. */
                return status;
            }
        }
    }

    /* Is this the first update?  */
    if (client_ptr -> nx_sntp_client_first_update_pending == NX_TRUE)
    {
        /* Check Client configuration if we ignore max adjustment limit on first update.  */
        ignore_max_adjustment_limit = NX_SNTP_CLIENT_IGNORE_MAX_ADJUST_STARTUP;    
    }
    else
        ignore_max_adjustment_limit = NX_FALSE;

    /* If not ignoring the max adjustment, deal with time difference between client and server. */
    if (ignore_max_adjustment_limit == NX_FALSE)
    {
    UINT pos_diff = NX_TRUE;

        /* Compute difference of server update packet minus the client's local time. It is reasonable
           to assume that the Client time is 'behind' the server time because it is updated by the 
           server time.  */
        local_time.seconds = client_ptr -> nx_sntp_client_local_ntp_time.seconds +
            client_ptr -> nx_sntp_client_local_ntp_time_elapsed;
        local_time.fraction = client_ptr -> nx_sntp_client_local_ntp_time.fraction;
        status = _nx_sntp_client_utility_get_msec_diff(&local_time, 
                                                       &(client_ptr -> nx_sntp_server_update_time), 
                                                       &elapsed_msecs_difference, &pos_diff);


        /* Note that a positive difference vs negative difference is not an error. */
        if (status != NX_SUCCESS)
        {

            /* Cannot use this time update. */
            return status;
        }

        /* Is the difference less than the client's minimum adjustment?  */  
        if (elapsed_msecs_difference > NX_SNTP_CLIENT_MIN_TIME_ADJUSTMENT)
        {
    
            /* The adjustment is larger than the Client's minimum adjustment. */

            /* Set the local clock to the server time only if 1)we are ignoring the maximum adjustment on startup, 
               or 2) the difference is within the Client's max time adjustment.  */
            if (elapsed_msecs_difference > NX_SNTP_CLIENT_MAX_TIME_ADJUSTMENT)
            {
                /* Cannot use this time update. */
                return NX_SNTP_INVALID_TIME;
            }
        }
    }

    /* Ok to update client local time. */
    memcpy(&client_ptr -> nx_sntp_client_local_ntp_time, &client_ptr -> nx_sntp_current_server_time_message.transmit_time, sizeof(NX_SNTP_TIME)); /* Use case of memcpy is verified. */
    client_ptr -> nx_sntp_client_local_ntp_time_elapsed = 0;

     /* Apply a correction to server's time for internal SNTP Client delays e.g. periodic task intervals. */
    adjustment = ((process_timerticks - receive_timerticks) * 1000) / NX_IP_PERIODIC_RATE; 

    status = _nx_sntp_client_utility_add_msecs_to_ntp_time(&(client_ptr -> nx_sntp_client_local_ntp_time), (LONG)adjustment); 

    /* Done processing time update. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_calculate_roundtrip                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function computes roundtrip based on the elapsed time from      */
/*   sending the unicast request to receiving the SNTP response.          */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    roundtrip_time                    round trip time computation       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Valid time computations result    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_process_update_packet                               */ 
/*                                      Process server update packet      */
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
UINT  _nx_sntp_client_calculate_roundtrip(LONG *roundtrip_time)
{

ULONG x;


    /* Initialize invalid results. */
    *roundtrip_time = 0;

    /* Compute the roundtrip as the time the packet left the SNTP Client
       to the time it received a response from the SNTP server. */

    /* Check for wrapped timer value. */
    if (send_timerticks > receive_timerticks)
    {
        /* The time has wrapped.  */
        x = 0xFFFFFFFF - send_timerticks;
        *roundtrip_time = (LONG)(receive_timerticks + x);
    }
    else
    {
         *roundtrip_time = (LONG)(receive_timerticks - send_timerticks);
    }

    /* Convert to milliseconds. */
    *roundtrip_time = (LONG)((ULONG)(*roundtrip_time) * NX_SNTP_MILLISECONDS_PER_TICK);

     /* Return successful completion.  */
     return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_get_local_time                      PORTABLE C     */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking for the get local time service.*/
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SNTP Client            */
/*    seconds                           Pointer to SNTP seconds           */ 
/*    fraction                          Local time fraction component     */
/*    buffer                            Pointer for time in string format */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_get_local_time    Get local time service            */ 
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
UINT  _nxe_sntp_client_get_local_time(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer)
{

UINT status;


    /* Check input pointer parameters.  */
    if ((client_ptr == NX_NULL) || (seconds == NX_NULL) || (fraction == NX_NULL))
    {

        /* Return pointer error.  */
        return NX_PTR_ERROR;
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING
    
    /* Call the actual service.  */
    status = _nx_sntp_client_get_local_time(client_ptr, seconds, fraction, buffer); 

    /* Return completion status.  */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_get_local_time                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function retrieves the current SNTP Client local time and       */
/*   returns the data in seconds and fractions, and if a non zero         */
/*   buffer pointer is supplied, a string containing the data in ASCII.   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SNTP Client            */
/*    seconds                           Pointer to SNTP seconds           */ 
/*    fraction                          Local time fraction component     */
/*    buffer                            Pointer for time in string format */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_utility_string_length_check       Check string length           */ 
/*    _nx_sntp_client_get_local_time_extended                             */ 
/*                                          Get local time service        */
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
UINT  _nx_sntp_client_get_local_time(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer) 
{

UINT  status;
    
    status = _nx_sntp_client_get_local_time_extended(client_ptr, seconds, fraction, buffer, NX_MAX_STRING_LENGTH);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_get_local_time_extended             PORTABLE C     */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking for the get extended local     */
/*   time service.                                                        */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SNTP Client            */
/*    seconds                           Pointer to SNTP seconds           */ 
/*    fraction                          Local time fraction component     */
/*    buffer                            Pointer for time in string format */
/*    buffer_size                       Size of buffer                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_get_local_time_extended                             */ 
/*                                      Get extended local time service   */
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
UINT  _nxe_sntp_client_get_local_time_extended(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer, UINT buffer_size)
{

UINT status;


    /* Check input pointer parameters.  */
    if ((client_ptr == NX_NULL) || (seconds == NX_NULL) || (fraction == NX_NULL))
    {

        /* Return pointer error.  */
        return NX_PTR_ERROR;
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING
    
    /* Call the actual service.  */
    status = _nx_sntp_client_get_local_time_extended(client_ptr, seconds, fraction, buffer, buffer_size); 

    /* Return completion status.  */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_get_local_time_extended             PORTABLE C      */ 
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function retrieves the current SNTP Client local time and       */
/*   returns the data in seconds and fractions, and if a non zero         */
/*   buffer pointer is supplied, a string containing the data in ASCII.   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SNTP Client            */
/*    seconds                           Pointer to SNTP seconds           */ 
/*    fraction                          Local time fraction component     */
/*    buffer                            Pointer for time in string format */
/*    buffer_size                       Size of buffer                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_fraction_to_usecs                           */ 
/*                                      Convert NTP fraction to usecs     */
/*    _nx_utility_uint_to_string        Converts number to ascii text     */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memmove use cases, */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            improved the logic of       */
/*                                            converting number to string,*/
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_sntp_client_get_local_time_extended(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer, UINT buffer_size) 
{

ULONG usecs;
UINT offset = 0;
UINT length = 0;
    
    *seconds = client_ptr -> nx_sntp_client_local_ntp_time.seconds;
    *fraction = client_ptr -> nx_sntp_client_local_ntp_time.fraction;

    if (buffer != NX_NULL)
    {

        /* Convert SNTP fraction component into microseconds.  */
        _nx_sntp_client_utility_fraction_to_usecs(client_ptr -> nx_sntp_client_local_ntp_time.fraction, &usecs);

        /* Decrease length for terminal zero. */
        buffer_size--;

        /* Create a string with just the time.  */
        /* Format: "Time: %lu.%06lu sec.\r\n" */
        if (buffer_size < 6)
        {
            return(NX_SIZE_ERROR);
        }
        buffer[offset++] = 'T';
        buffer[offset++] = 'i';
        buffer[offset++] = 'm';
        buffer[offset++] = 'e';
        buffer[offset++] = ':';
        buffer[offset++] = ' ';
        length = _nx_utility_uint_to_string(client_ptr -> nx_sntp_client_local_ntp_time.seconds,
                                            10, &buffer[offset], buffer_size - offset);
        if (length == 0)
        {
            return(NX_SIZE_ERROR);
        }
        offset += length;
        if ((buffer_size - offset) < 14)
        {
            return(NX_SIZE_ERROR);
        }
        buffer[offset++] = '.';
        length = _nx_utility_uint_to_string(usecs, 10, &buffer[offset], buffer_size - offset);
        if (length == 0)
        {
            return(NX_SIZE_ERROR);
        }

        if (length < 6)
        {

            /* Append zeroes. */
            memmove(&buffer[offset + (6 - length)], &buffer[offset], length); /* Use case of memmove is verified.  */
            memset(&buffer[offset], '0', (6 - length));
        }

        offset += 6;
        buffer[offset++] = ' ';
        buffer[offset++] = 's';
        buffer[offset++] = 'e';
        buffer[offset++] = 'c';
        buffer[offset++] = '.';
        buffer[offset++] = '\r';
        buffer[offset++] = '\n';
        buffer[offset] = '\0';               
    }
    
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_convert_time_to_UCHAR       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts time from the ULONG seconds and msecs in the */
/*    NX_SNTP_TIME data to the 32 bit UCHAR seconds and fraction fields in*/
/*    the NTP time message.  The caller specifies which time stamp in the */
/*    NTP time message (transmit,receive, origination, or reference clock)*/
/*    and that field is converted over.                                   */ 
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    time_ptr                        Pointer to NX_SNTP_TIME time        */
/*    time_message_ptr                Pointer to NTP (UCHAR) time         */
/*    which_stamp                     Which time stamp to convert         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SNTP_PARAM_ERROR             Invalid time stamp requested        */ 
/*    NX_SUCCESS                      Successful completion status        */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                         Clear specified area of memory       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_send_unicast_request                                */ 
/*                                   Create a time request and transmit it*/
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
UINT  _nx_sntp_client_utility_convert_time_to_UCHAR(NX_SNTP_TIME *time_ptr, 
                                     NX_SNTP_TIME_MESSAGE *time_message_ptr, UINT which_stamp)
{

ULONG *buffer;

    /* Copy the buffer to the requested time stamp field.  */
    switch (which_stamp)
    {

        case REFERENCE_TIME:
            buffer = time_message_ptr -> reference_clock_update_time_stamp;
        break;

        case ORIGINATE_TIME:
            buffer = time_message_ptr -> originate_time_stamp;
        break;

        case RECEIVE_TIME:
            buffer = time_message_ptr -> receive_time_stamp;
        break;

        case TRANSMIT_TIME:
            buffer = time_message_ptr -> transmit_time_stamp;
        break;

        default:
            /* Invalid time stamp. Return as error.  */
            return NX_SNTP_PARAM_ERROR;
    }

    /* Copy NX_SNTP_TIME seconds and fraction to the buffer.  */
    *(buffer) = time_ptr -> seconds;
    *(buffer + 1) = time_ptr -> fraction;

    /* Return successful completion.  */
    return NX_SUCCESS;
}

/* The conventional civil timescale used in most parts of the world is based on Coordinated Universal Time (UTC), 
   which replaced Greenwich Mean Time (GMT) many years ago. UTC is based on International Atomic Time (TAI), 
   which is derived from hundreds of cesium oscillators in the national standards laboratories of many regions. 
   Deviations of UTC from TAI are implemented in the form of leap seconds, which occur at intervals from a 
   few months to serveral years. */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_convert_seconds_to_date     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function computes the month, day, and time in an NTP time based  */
/*  on the known number of seconds at 1/1/1999 since 1/1/1900 (the NTP    */
/*  time epoch) and the current NTP time.  The caller must indicated the  */
/*  year of the NTP time to convert to simplify the computation.  The data*/
/*  is written to an NX_SNTP_DATE_TIME object from which the call can     */
/*  extract date and time data, or call the NetX SNTP API to display the  */
/*  data/time string.                                                     */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    current_NTP_time_ptr              Pointer to NTP time               */ 
/*    current_year                      Year in the NTP time data         */
/*    current_date_time_ptr             Pointer to date time object       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful completion status     */ 
/*    NX_SNTP_ERROR_CONVERTING_DATETIME  Internal error converting time   */
/*    NX_SNTP_UNABLE_TO_CONVERT_DATETIME Insufficient data for converting */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_convert_fraction_to_msecs                   */
/*                                      Converts the 32 bit fraction field*/ 
/*                                      in an NTP time to milliseconds    */
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed leap year calculation,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_sntp_client_utility_convert_seconds_to_date(NX_SNTP_TIME *current_NTP_time_ptr, UINT current_year, 
                                                     NX_SNTP_DATE_TIME *current_date_time_ptr)
{

    /* Check if there is a base number of seconds set. */
#if (NTP_SECONDS_AT_01011999 == 0)

    /* No, so we cannot convert time into months and years. */
    return NX_SNTP_UNABLE_TO_CONVERT_DATETIME;
#else

UINT seconds_diff;
UINT years_diff;
UINT leapyears_diff;
UINT leaps;
UINT seconds_of_year;
UINT seconds_into_currentyear;
UINT seconds_into_currentmonth;
UINT seconds_into_currentday;
UINT seconds_into_currenthour;

    memset(current_date_time_ptr, 0, sizeof(NX_SNTP_DATE_TIME));

    do
    {

        current_date_time_ptr -> year = current_year;

        seconds_diff = current_NTP_time_ptr -> seconds - NTP_SECONDS_AT_01011999;

        years_diff = current_year - 1999;
    
        /* Figure out number of leap years since 1999 not including the current year. */
        leapyears_diff = (current_year - 1 - 1996) >> 2;
    
        /* Determine if this is a leap year. */
        leaps = (current_year - 1996) & 3;
    
        /* Check if this is a leap year. */
        if (leaps == 0 )
        {
            /* It is! */
            current_date_time_ptr -> leap_year = NX_TRUE;      
            seconds_of_year = SECONDS_PER_LEAPYEAR;
        }
        else
        {

            /* Not a leap year. Clear the leap year flag. */
            current_date_time_ptr -> leap_year = NX_FALSE;
            seconds_of_year = SECONDS_PER_NONLEAPYEAR;
        }

        /* Compute number of seconds into the current year e.g. as of 01/01 at midnite by subtracting
           the total number of seconds since 1/1/1999 up to the end of the previous year.  Remember to compute
           the leapyear seconds at a different rate. */
        seconds_into_currentyear = seconds_diff - ((years_diff - leapyears_diff) * SECONDS_PER_NONLEAPYEAR)
                                                - (leapyears_diff * SECONDS_PER_LEAPYEAR); 

        current_year++;

    }while(seconds_into_currentyear > seconds_of_year);

    /* Initialize month to January till we find out what the month is. */
    current_date_time_ptr -> month = JANUARY;

    while (1)
    {

        /* Does the number of seconds goes past January? */
        if (seconds_into_currentyear >= SEC_IN_JAN)
        {

            /* Yes, reset month to Feb and subtract seconds in January. */
            current_date_time_ptr -> month = FEBRUARY;
            seconds_into_currentyear -= SEC_IN_JAN;        
        }
        /* No, we're done going month to month. */
        else break;

        /* Handle February differently because it is a leap month. */
        if (current_date_time_ptr -> leap_year)
        {

            /* This is a leap year so Feb has 29 days. */
            if (seconds_into_currentyear >= SEC_IN_LEAPFEB)
            {
                current_date_time_ptr -> month = MARCH;
                seconds_into_currentyear -= SEC_IN_LEAPFEB;        
            }
            else break;

        }
        else 
        {
            /* Not a leap year, Feb has the usual 28 days. */
            if (seconds_into_currentyear >= SEC_IN_NONLEAPFEB)
            {
                current_date_time_ptr -> month = MARCH;
                seconds_into_currentyear -= SEC_IN_NONLEAPFEB;        
            }        
            else break;

        }

        /* Repeat for each month for the rest of the year. */

        if (seconds_into_currentyear >= SEC_IN_MAR)
        {
            current_date_time_ptr -> month = APRIL;
            seconds_into_currentyear -= SEC_IN_MAR;        
        }
        else break;

        if (seconds_into_currentyear >= SEC_IN_APR)
        {
            current_date_time_ptr -> month = MAY;
            seconds_into_currentyear -= SEC_IN_APR;        
        }
        else break;
    
        if (seconds_into_currentyear >= SEC_IN_MAY)
        {
            current_date_time_ptr -> month = JUNE;
            seconds_into_currentyear -= SEC_IN_MAY;        
        }
        else break;
    
        if (seconds_into_currentyear >= SEC_IN_JUN)
        {
            current_date_time_ptr -> month = JULY;
            seconds_into_currentyear -= SEC_IN_JUN;        
        }
        else break;
    
        if (seconds_into_currentyear >= SEC_IN_JUL)
        {
            current_date_time_ptr -> month = AUGUST;
            seconds_into_currentyear -= SEC_IN_JUL;        
        }
        else break;
    
        if (seconds_into_currentyear >= SEC_IN_AUG)
        {
            current_date_time_ptr -> month = SEPTEMBER;
            seconds_into_currentyear -= SEC_IN_AUG;        
        }
        else break;
    
        if (seconds_into_currentyear >= SEC_IN_SEP)
        {
            current_date_time_ptr -> month = OCTOBER;
            seconds_into_currentyear -= SEC_IN_SEP;        
        }
        else break;
    
        if (seconds_into_currentyear >= SEC_IN_OCT)
        {
            current_date_time_ptr -> month = NOVEMBER;
            seconds_into_currentyear -= SEC_IN_OCT;        
        }
        else break;
    
        if (seconds_into_currentyear >= SEC_IN_NOV)
        {
            current_date_time_ptr -> month = DECEMBER;
            seconds_into_currentyear -= SEC_IN_NOV;        
        }
        else break;

        /* We should not have more than the seconds in december or there is an error. */
        if (seconds_into_currentyear > SEC_IN_DEC)
        {

            /* Return the error status. */
            return NX_SNTP_ERROR_CONVERTING_DATETIME;
        }
        else break;
    }

    /* Time is now in the current month. */
    seconds_into_currentmonth = seconds_into_currentyear;

    /* Compute how many complete days the time goes into the current month and 
       add one for the current day. */
    current_date_time_ptr -> day = seconds_into_currentmonth/SECONDS_PER_DAY + 1;

    /* Compute the number of seconds into the current day. */
    seconds_into_currentday = seconds_into_currentmonth % SECONDS_PER_DAY;

    /* Compute the number of complete hours into the current day we are. */
    current_date_time_ptr -> hour = seconds_into_currentday/SECONDS_PER_HOUR;

    /* Compute the number of seconds into the current hour. */
    seconds_into_currenthour = seconds_into_currentday % SECONDS_PER_HOUR;

    /* Break this down into minutes. */
    current_date_time_ptr -> minute = seconds_into_currenthour/SECONDS_PER_MINUTE;

    /* Finally the remainder is the seconds. */
    current_date_time_ptr -> second = seconds_into_currenthour % SECONDS_PER_MINUTE;

    /* Convert time fraction field into milliseconds.  */
    _nx_sntp_client_utility_convert_fraction_to_msecs((ULONG *)(&(current_date_time_ptr -> millisecond)), current_NTP_time_ptr);

    return NX_SUCCESS;
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_utility_display_date_time          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function performs error checking services on the display date    */
/*  time service.                                                         */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SNTP Client            */
/*    buffer                            Pointer to string buffer          */ 
/*    length                            Size of the string buffer         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                     Invalid pointer input              */ 
/*    NX_SNTP_PARAM_ERROR              Invalid non pointer input          */ 
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_display_date_time                           */
/*                                     Actual display date/time service   */
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
UINT _nxe_sntp_client_utility_display_date_time(NX_SNTP_CLIENT *client_ptr, CHAR *buffer, UINT length)
{

UINT status;


    /* Check for invalid pointer input.  */
    if ((client_ptr == NX_NULL) || (buffer == NX_NULL))
    {

        /* Return pointer error.  */
        return NX_PTR_ERROR;
    }

    /* Check for invalid parameter input.  */
    if (length == 0)
    {

        /* Return parameter error.  */
        return NX_SNTP_PARAM_ERROR;
    }

    /* Call the actual display time service.  */
    status = _nx_sntp_client_utility_display_date_time(client_ptr, buffer, length);

    /* Return completion status.  */
    return status;
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_request_unicast_time               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the service that          */
/*    forwards a unicast request from the SNTP Client application.        */ 
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client struct           */ 
/*    wait_option                      Time to wait for response (ticks)  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */
/*    status                            Actual completion status          */ 
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
UINT _nxe_sntp_client_request_unicast_time(NX_SNTP_CLIENT *client_ptr, UINT wait_option)
{

UINT status;

    /* Check for invalid pointer input. */
    if (client_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    status = _nx_sntp_client_request_unicast_time(client_ptr, wait_option);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_request_unicast_time                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a unicast request regardless if the SNTP Client */
/*    is configured for unicast or broadcast mode. After sending the      */
/*    request, the function waits for the specified time for a response.  */
/*                                                                        */ 
/*    If received, the SNTP Server response is processed as it normally is*/
/*    for valid SNTP data, and applied to the SNTP Client's notion of time*/
/*    "local time".                                                       */
/*                                                                        */ 
/*    This will not interfere with subsequent unicast requests in unicast */
/*    mode, or the processing of periodic updates in broadcast mode.      */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to Client struct           */ 
/*    wait_option                      Time to wait for response (ticks)  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */
/*    status                            Actual completion status          */ 
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
UINT _nx_sntp_client_request_unicast_time(NX_SNTP_CLIENT *client_ptr, UINT wait_option)
{

UINT status;


    /* Make sure the client is started. */
    if (!client_ptr -> nx_sntp_client_started)
    {
        return NX_SNTP_CLIENT_NOT_STARTED;
    }

    /* Create and send a unicast request.  */
    status = _nx_sntp_client_send_unicast_request(client_ptr);

    if (status != NX_SUCCESS)
    {
        return status;
    }

    /* Wait to receive a response. */
    status = _nx_sntp_client_receive_time_update(client_ptr, wait_option);

    /* If we got a valid SNTP packet, process the data. */
    if (status == NX_SUCCESS)
    {

        /* Process the server update packet and apply to local time if valid.  */
        status =  _nx_sntp_client_process_update_packet(client_ptr);
    }

    tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));
    /* Return completion status.  */
    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_display_date_time           PORTABLE C      */ 
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function converts an NTP time data into a month-date-year time,  */
/*  including seconds and second fraction string. It is intended as a     */
/*  human readable representation of NTP time.  The caller must supply a  */
/*  pointer to a buffer large enough (40 chars is enough) to hold the     */
/*  string and define the NX_SNTP_CURRENT_YEAR parameter, usually as the  */
/*  current year.                                                         */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SNTP Client            */
/*    buffer                            Pointer to string buffer          */ 
/*    length                            Size of the string buffer         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_SNTP_INVALID_DATETIME_BUFFER   Buffer not large enough           */ 
/*    NX_SNTP_ERROR_CONVERTING_DATETIME Internal error converting NTP time*/
/*    status                            Convert seconds completion status */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*    _nx_sntp_client_utility_convert_seconds_to_date                     */ 
/*                                      Converts seconds to year, month   */
/*    _nx_utility_uint_to_string        Converts number to ascii text     */ 
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            improved the logic of       */
/*                                            converting number to string,*/
/*                                            resulting in version 6.1.8  */
/*  10-31-2022     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            the typo of August string,  */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_sntp_client_utility_display_date_time(NX_SNTP_CLIENT *client_ptr, CHAR *buffer, UINT length)
{

UINT                status;
UINT                offset;
UINT                return_length;
NX_SNTP_DATE_TIME   DisplayTime;
const CHAR         *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


#ifndef  NX_SNTP_CURRENT_YEAR  
    return NX_SNTP_ERROR_CONVERTING_DATETIME;
#else

    /* Verify the client has set a local time. */
    if (client_ptr -> nx_sntp_client_local_ntp_time.seconds == 0)
    {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }

    status = _nx_sntp_client_utility_convert_seconds_to_date(&(client_ptr -> nx_sntp_client_local_ntp_time), NX_SNTP_CURRENT_YEAR, &DisplayTime);
    if (status != NX_SUCCESS)
    {
        return status;
    }

    /* Check if we have a long enough buffer. */
    if (length < 5) 
    {

        /* Return the error status. */
        return NX_SNTP_INVALID_DATETIME_BUFFER;
    }

    /* Decrease length for terminal zero. */
    length--;

    /* Substitute numeric month to name of the month. */
    if ((DisplayTime.month < JANUARY) || (DisplayTime.month > DECEMBER))
    {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[0] = months[DisplayTime.month - JANUARY][0];
    buffer[1] = months[DisplayTime.month - JANUARY][1];
    buffer[2] = months[DisplayTime.month - JANUARY][2];
    buffer[3] = ' ';
    offset = 4;

    /* Write in the rest of the data as numeric from the Date Time objext. */
    return_length = _nx_utility_uint_to_string(DisplayTime.day, 10, &buffer[offset], length - offset);
    offset += return_length;
    if ((return_length == 0) || ((length - offset) < 2))
    {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = ',';
    buffer[offset++] = ' ';
    return_length = _nx_utility_uint_to_string(DisplayTime.year, 10, &buffer[offset], length - offset);
    offset += return_length;
    if ((return_length == 0) || ((length - offset) < 1))
    {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = ' ';
    return_length = _nx_utility_uint_to_string(DisplayTime.hour, 10, &buffer[offset], length - offset);
    offset += return_length;
    if ((return_length == 0) || ((length - offset) < 1))
    {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = ':';
    return_length = _nx_utility_uint_to_string(DisplayTime.minute, 10, &buffer[offset], length - offset);
    offset += return_length;
    if ((return_length == 0) || ((length - offset) < 1))
    {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = ':';
    return_length = _nx_utility_uint_to_string(DisplayTime.second, 10, &buffer[offset], length - offset);
    offset += return_length;
    if ((return_length == 0) || ((length - offset) < 1))
    {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = '.';
    return_length = _nx_utility_uint_to_string(DisplayTime.millisecond, 10, &buffer[offset], length - offset);
    offset += return_length;
    if ((return_length == 0) || ((length - offset) < 5))
    {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = ' ';
    buffer[offset++] = 'U';
    buffer[offset++] = 'T';
    buffer[offset++] = 'C';
    buffer[offset++] = ' ';
    buffer[offset] = '\0';

#endif

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_add_msecs_to_ntp_time       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function adds msecs (not necessarily a positive value and not   */
/*   limited to less than a second) to an NTP time value. Msecs cannot be */
/*   added directly to the fraction field in the NTP time field because   */
/*   this field is represented in fixed point notation.  Arithmetic       */ 
/*   overflow and loss of sign errors as a result of adding numbers are   */ 
/*   handled as errors.                                                   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    timeA_ptr                        Pointer to NTP time operand        */
/*    msecs_to_add                     Time (msecs) to add                */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    NX_SNTP_OVERFLOW_ERROR           Overflow result adding numbers     */ 
/*    NX_SNTP_INVALID_TIME             An invalid result (e.g. negative   */
/*                                        time) from adding numbers       */
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_sntp_client_process_time_data  Apply server time to local time   */ 
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

UINT  _nx_sntp_client_utility_add_msecs_to_ntp_time(NX_SNTP_TIME *timeA_ptr, LONG msecs_to_add) 
{

UINT  status;
ULONG timeA_usec;
LONG  seconds;
LONG  usecs;


    /* Separate msecs_to_add into seconds and milliseconds.  */
    seconds = msecs_to_add / 1000;
    usecs = (msecs_to_add % 1000) * 1000;

    /* Are we adding a positive number?  */
    if (msecs_to_add > 0)
    {
        /* Yes; check for overflow before trying to add seconds to the TimeA operand.  */        
        status = _nx_sntp_client_utility_addition_overflow_check(timeA_ptr -> seconds, (ULONG)seconds);

        /* Check for error (overflow).  */
        if (status != NX_SUCCESS)
        {

            /* Return the error condition.  */
            return status;
        }
    }
    /* Check if a negative number larger than the timeA operand is being added (creates negative time!)*/
    else if (timeA_ptr -> seconds < (ULONG)abs(seconds))
    {

        /* Yes; return the error condition.  */
        return NX_SNTP_INVALID_TIME;
    }

    /* Ok to add seconds to the NTP time seconds.  */
    timeA_ptr -> seconds += (ULONG)seconds;

    /* Next get the usecs from the timeA operand (always positive and < 1000000).  */
    _nx_sntp_client_utility_fraction_to_usecs(timeA_ptr -> fraction, &timeA_usec);

    /* In case usecs is < 0, we might have to perform a carry.  */
    if ((usecs + (LONG)timeA_usec) < 0)
    {
        /* Perform a carry by subtracting a second from timeA seconds...*/
        timeA_ptr -> seconds--;

        /* And adding it to the usecs of timeA.  */
        timeA_usec += 1000000;
    }

    /* OK to add the usecs up.  */
    usecs += (LONG)timeA_usec;

    /* Check for a positive carry over into seconds.  */
    if (usecs >= 1000000)
    {
        /* Yes there's a carry; check for possibility of overflow 
           before adding carry (unlikely for another 30 years).  */
        if (timeA_ptr -> seconds == 0xFFFFFFFF)
        {

            return NX_SNTP_OVERFLOW_ERROR;
        }

        /* OK to increment the seconds.  */
        timeA_ptr -> seconds++;

        /* Set milliseconds to remainder.  */
        usecs = usecs % 1000000; 
    }

    /* Convert usecs to the fixed point notation fraction and store in TimeA fraction.  */
    status = _nx_sntp_client_utility_usecs_to_fraction((ULONG)usecs, &(timeA_ptr ->fraction));

    /* Return completion status.  */
    return status;    
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_receiving_updates                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking for the get SNTP get receive   */
/*   status service.                                                      */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to SNTP client instance    */
/*    receive_status                   Pointer to server status           */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                     Invalid input status               */ 
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_receiving_updates                                   */ 
/*                                     Actual get server status service   */
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
UINT _nxe_sntp_client_receiving_updates(NX_SNTP_CLIENT *client_ptr, UINT *receive_status)
{

UINT status;

    /* Check for the validity of input parameter.  */
    if ((client_ptr == NX_NULL) || (receive_status == NX_NULL))
    {

        /* Return error status.  */
        return(NX_PTR_ERROR);
    }

    status = _nx_sntp_client_receiving_updates(client_ptr, receive_status);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_receiving_updates                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function returns the status of the Client SNTP server. If the   */
/*   client has not received a valid update within the NX_SNTP_CLIENT_MAX_*/
/*   _TIME_LAPSE interval or if the number of invalid updates received by */
/*   the client exceeds the NX_SNTP_CLIENT_INVALID_UPDATE_LIMIT limit,    */
/*   the status is set to NX_FALSE.  If the Client has not yet received   */
/*   its first valid update from the current SNTP server, status is set to*/
/*   false.                                                               */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to SNTP client instance    */
/*    receive_status                   Pointer to receive_status          */
/*                                     NX_FALSE: not receiving updates    */
/*                                     NX_TRUE: receiving valid updates   */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                     Invalid input status               */ 
/*    status                           Actual completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                     Get the SNTP mutex                 */ 
/*    tx_mutex_put                     Release the SNTP mutex             */ 
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
UINT _nx_sntp_client_receiving_updates(NX_SNTP_CLIENT *client_ptr, UINT *receive_status)
{


    /* Get the SNTP mutex.  */
    tx_mutex_get(&(client_ptr -> nx_sntp_client_mutex), TX_WAIT_FOREVER);

    /* Verify the client's SNTP server is valid, and the Client has received at least one valid udpate from it. */
    *receive_status = ((client_ptr -> nx_sntp_valid_server_status == NX_TRUE) && (client_ptr -> nx_sntp_client_first_update_pending == NX_FALSE));

    /* Release the SNTP mutex.  */
    tx_mutex_put(&(client_ptr -> nx_sntp_client_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);  

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_set_local_time                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking for the set client local time  */
/*   service.                                                             */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to SNTP Client             */
/*    seconds                          Local time seconds component       */
/*    fraction                         Local time fraction component      */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                           Actual completion status           */  
/*    NX_PTR_ERROR                     Invalid pointer input              */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_set_local_time  Actual set client local time service*/
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
UINT  _nxe_sntp_client_set_local_time(NX_SNTP_CLIENT *client_ptr, ULONG seconds, ULONG fraction) 
{

UINT status;


    /* Check for invalid input. */
   if (client_ptr == NX_NULL)
   {
       return NX_PTR_ERROR;
   }

   /* Call the actual service. */
   status = _nx_sntp_client_set_local_time(client_ptr, seconds, fraction);

   /* Return completion status. */
   return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_set_local_time                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function takes the seconds and fraction input from the caller   */
/*   (or more accurately the independent time clock source, and applies it*/
/*   to the SNTP client local time.                                       */
/*                                                                        */
/*   In between SNTP server updates, it is expected that the SNTP Client  */
/*   host application will update the SNTP client local time from the     */
/*   independent time source (e.g. real time clock on board) and then     */
/*   use the SNTP Server time updates to correct the local time for drifts*/
/*   from the correct time.                                               */
/*                                                                        */ 
/*   It can also set the SNTP Client's base time before starting up the   */
/*   SNTP Client. If the host application cannot obtain a base time, the  */
/*   SNTP Client will take the first SNTP update as the absolute time. If */
/*   the host application does have a real time clock or independent time */
/*   keeper, the SNTP client can set a large enough max adjustment that   */
/*   any Server time udpate will be accepted to the SNTP Client. This     */
/*   leaves the SNTP Client completely dependent on the network and SNTP  */
/*   Server, plus it is vulnerable to rogue SNTP packets.                 */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to SNTP Client             */
/*    seconds                          Local time seconds component       */
/*    fraction                         Local time fraction component      */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
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
UINT  _nx_sntp_client_set_local_time(NX_SNTP_CLIENT *client_ptr, ULONG seconds, ULONG fraction) 
{


    client_ptr -> nx_sntp_client_local_ntp_time.seconds = seconds;
    client_ptr -> nx_sntp_client_local_ntp_time.fraction = fraction;
    client_ptr -> nx_sntp_client_local_ntp_time_elapsed = 0;

    /* Return completion status.  */
    return NX_SUCCESS;    
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_set_time_update_notify             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the set time update       */
/*    callback service.                                                   */
/*                                                                        */
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to Client struct          */ 
/*    time_update_cb                    Pointer to callback when Client   */
/*                                        receives an SNTP update         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_set_time_update_notify                              */ 
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
UINT    _nxe_sntp_client_set_time_update_notify(NX_SNTP_CLIENT *client_ptr, 
                       VOID (time_update_cb)(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time))
{

UINT status;


    /* Check for valid input. */
    if ((client_ptr == NX_NULL) || (time_update_cb == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    status = _nx_sntp_client_set_time_update_notify(client_ptr, time_update_cb);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_set_time_update_notify              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function notifies the application of a valid SNTP time update. */
/*                                                                        */
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to Client struct          */
/*    time_update_cb                    Pointer to callback when Client   */
/*                                        receives an SNTP update         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
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
UINT    _nx_sntp_client_set_time_update_notify(NX_SNTP_CLIENT *client_ptr, 
                             VOID (time_update_cb)(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time))
{


    client_ptr -> nx_sntp_client_time_update_notify = time_update_cb;

    return NX_SUCCESS;
}




/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_get_msec_diff               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function computes the difference in milliseconds between two   */ 
/*    NTP times, receiving an NTP packet, and transmitting it back.       */
/*    The logic calculates the difference in the seconds component and    */
/*    converts it to milliseconds.  It converts the fraction to useconds  */
/*    and calculates that difference. Useconds are rounded to the nearest */
/*    millisecond.  This logic assumes the transmit time occurs after     */
/*    receive time.                                                       */
/*                                                                        */ 
/*    The net difference is the difference in milliseconds from the       */
/*    fractions added to (or subtracted from) the difference in           */
/*    milliseconds from the seconds component.                            */
/*                                                                        */ 
/*    Note that the conversion of useconds to milliseconds may result     */
/*    in the two times' difference to be zero, when they are actually     */
/*    different by useconds.                                              */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    timeReceived_ptr                 NTP time of received message       */
/*    timeTransmit_ptr                 NTP time of transmitted message    */
/*    total_difference_msecs           Millseconds of difference in time  */
/*    pos_diff                         True if Transmit Time >=           */
/*                                          Receive Time                  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Valid transmit/receive times       */ 
/*    NX_SNTP_OVERFLOW_ERROR           Overflow result                    */
/*    NX_SNTP_INVALID_TIME             Transmit time<receive time seconds */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_convert_fraction_to_msecs                   */ 
/*                                      Convert fraction to milliseconds  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_apply_sanity_checks                                 */ 
/*                                     Apply sanity checks to time data   */
/*    _nx_sntp_client_process_time_data                                   */ 
/*                                     Apply server time to local time    */
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
UINT  _nx_sntp_client_utility_get_msec_diff(NX_SNTP_TIME *timeReceived_ptr, NX_SNTP_TIME *timeTransmit_ptr, ULONG *total_difference_msecs, UINT *pos_diff)
{

ULONG    usecsReceived, usecsTransmit;
ULONG    msecsReceived, msecsTransmit;
ULONG    seconds_difference_in_msecs;
ULONG    temp;  

 
    /* Check for overflow with a very large positive difference result.  */
    if (timeReceived_ptr -> seconds > timeTransmit_ptr -> seconds)
    {
        
        /* Use a temporary variable to store the difference in the seconds.  */
        temp = timeReceived_ptr -> seconds - timeTransmit_ptr -> seconds;
        *pos_diff = NX_FALSE;
    }
    else
    {

        /* Reverse the operand to get the absolute difference in the seconds.  */
        temp = timeTransmit_ptr -> seconds -timeReceived_ptr -> seconds;
        *pos_diff = NX_TRUE;
    }

    /* Check for overflow when converting seconds to milliseconds
       (0x3E8 = 1000).  */
    if (temp > (0xFFFFFFFF / 0x3E8))
    {

        /* Return error status.  */
        return NX_SNTP_OVERFLOW_ERROR;
    }
    
    /* Convert to msecs.  */
    seconds_difference_in_msecs = temp * 1000;

    /* Convert the Received time fraction to usecs and msecs.  */
    _nx_sntp_client_utility_fraction_to_usecs(timeReceived_ptr -> fraction, &usecsReceived);

    msecsReceived = usecsReceived / 1000;

    if (usecsReceived % 1000 >= 500)
        msecsReceived++;

    /* Convert the Transmit Time fraction to usecs and msecs.  */
    _nx_sntp_client_utility_fraction_to_usecs(timeTransmit_ptr -> fraction, &usecsTransmit);

    msecsTransmit = usecsTransmit / 1000;

    if (usecsTransmit % 1000 >= 500)
        msecsTransmit++;

    /* Get the difference of the two time stamps' fraction in milliseconds. */
    if (timeReceived_ptr -> seconds == timeTransmit_ptr -> seconds)
    {

        /* Determine the absolute difference in the millisecond component. */
        if (usecsTransmit >= usecsReceived) 
        {

            *total_difference_msecs = msecsTransmit - msecsReceived;
        }
        else
        {

            /* Transmit time usecs < Received time usecs. */
            *pos_diff = NX_FALSE;
            *total_difference_msecs = msecsReceived - msecsTransmit;
        }
    }
    else
    {

        /* Consider the case where the transmit time seconds is greater. */
        if (timeTransmit_ptr -> seconds > timeReceived_ptr -> seconds)
        {

            if ( usecsTransmit >= usecsReceived)
            {
    
                /*  This will add to the total milliseconds' difference. */
               *total_difference_msecs = seconds_difference_in_msecs + (msecsTransmit - msecsReceived);
            }
            else /* (usecsReceived > usecsTransmit) */
            {
    
                /* This will subtract from the total milliseconds' difference . */
                *total_difference_msecs = seconds_difference_in_msecs - (msecsReceived - msecsTransmit);
            }
        }

        /* Consider the case where the transmit time seconds is less. */
        else
        {

            if (usecsReceived >= usecsTransmit)
            {
    
                /*  This will add to the total milliseconds' difference. */
               *total_difference_msecs = seconds_difference_in_msecs + (msecsReceived - msecsTransmit);
            }
            else /* (usecsTransmit > usecsReceived) */
            {
    
                /* This will subtract from the total milliseconds' difference . */
                *total_difference_msecs = seconds_difference_in_msecs - (msecsTransmit - msecsReceived);
            }
        }
    }

    /* Return successful completion status.  */
    return NX_SUCCESS;

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_is_zero_data                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function tests each byte (UCHAR) of data to be non zero.  The   */
/*   return value indicates if the entire data is zero.                   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    data                               Pointer to first byte of data    */
/*    size                               Number of bytes in data          */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_TRUE                            Each byte of data is zero        */ 
/*    NX_FALSE                           At least one byte is non zero    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_sntp_client_apply_sanity_checks Checks SNTP server reply validity*/
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
UINT _nx_sntp_client_utility_is_zero_data(UCHAR *data, UINT size)
{

UINT i;
UINT is_zero;


    /* Initialize local variables. */
    i = 0;
    is_zero = NX_TRUE;
    
    while(i < size)
    {

        if (*data != 0x0)
        {

            is_zero = NX_FALSE;
            break;            
        }

        data += sizeof(UCHAR);
        i++;
    }

    return is_zero;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_convert_fraction_to_msecs   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function converts the fraction in an NTP time to milliseconds.  */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    milliseconds                     Pointer to milliseconds converted  */
/*    time_ptr                         Pointer to an NTP time             */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion              */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_fraction_to_usecs                           */ 
/*                                     Convert fraction to usecs          */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_sntp_client_utility_display_NTP_time                             */ 
/*                                     Display NTP time in seconds        */
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
UINT  _nx_sntp_client_utility_convert_fraction_to_msecs(ULONG *milliseconds, NX_SNTP_TIME *time_ptr)
{

ULONG usecs;


    /* Convert to usecs first.  */
    _nx_sntp_client_utility_fraction_to_usecs(time_ptr ->fraction, &usecs);

    /* Then convert to milliseconds.  */
    *milliseconds = usecs / 1000;

    /* Round up if greater than 500 usecs left over*/
    if (usecs % 1000 >= 500)
    {

        (*milliseconds)++;
    }

    /* Return successful completion.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_utility_usecs_to_fraction          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the utility to convert      */
/*   microseconds to fraction.                                            */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    usecs                            Microseconds to convert            */
/*    fraction                         Pointer to converted fraction      */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SNTP_INVALID_TIME             Invalid SNTP data input            */ 
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_usecs_to_fraction                           */ 
/*                                     Actual usecs conversion service    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   Application code                                                     */
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
UINT    _nxe_sntp_client_utility_usecs_to_fraction(ULONG usecs, ULONG *fraction)
{

UINT status;


    if ((usecs == 0) || (fraction == NX_NULL))
    {

        return NX_SNTP_INVALID_TIME;
    }

    status = _nx_sntp_client_utility_usecs_to_fraction(usecs, fraction);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_usecs_to_fraction           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function converts microseconds to a time stamp fraction.  It is */
/*   primarily an intermediary function used in the process of converting */
/*   millliseconds to fixed point time fraction data.                     */
/*                                                                        */ 
/*   This conversion scheme is limited to microseconds less than 1000000. */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    usecs                            Microseconds to convert            */
/*    fraction                         Fraction to converted fraction     */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    NX_SNTP_OVERFLOW_ERROR           Overflow error status              */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_sntp_client_utility_msecs_to_fraction                            */ 
/*                                     Convert milliseconds to fixed point*/
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
UINT  _nx_sntp_client_utility_usecs_to_fraction(ULONG usecs, ULONG *fraction)
{

ULONG _frac = usecs * 3962;
 
    
    *fraction = (usecs * 4294) + (_frac >> 12);

    if((_frac & 4095) >= 2048)
        *fraction = *fraction + 1;

    /* Successful completion.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_utility_msecs_to_fraction          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the utility to convert      */
/*   milliseconds to fraction.                                            */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    msecs                            Milliseconds to convert            */
/*    fraction                         Pointer to converted fraction      */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SNTP_INVALID_TIME             Invalid SNTP data input            */ 
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_msecs_to_fraction                           */ 
/*                                     Actual msecs conversion service    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   Application code                                                     */
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
UINT    _nxe_sntp_client_utility_msecs_to_fraction(ULONG msecs, ULONG *fraction)
{

UINT status;


    if ((msecs == 0) || (fraction == NX_NULL))
    {

        return NX_SNTP_INVALID_TIME;
    }

    status = _nx_sntp_client_utility_msecs_to_fraction(msecs, fraction);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_msecs_to_fraction           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function converts milliseconds to fixed point notation used in  */
/*   the NTP time fraction field. This will not accept msecs >= 1000      */
/*   because that number cannot be represented in an NTP time fraction.   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    msecs                            Milliseconds to convert            */
/*    fraction                         Pointer to converted fraction      */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    NX_SNTP_OVERFLOW_ERROR           Overflow result                    */ 
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_usecs_to_fraction                           */ 
/*                                     Convert usecs to fixed point       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_sntp_client_utility_add_msecs_to_ntp_time                        */ 
/*                                     Add msecs to an NTP time data      */
/*   _nx_sntp_client_utility_add_NTPtime                                  */ 
/*                                     Add two NTP time fields            */
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
UINT  _nx_sntp_client_utility_msecs_to_fraction(ULONG msecs, ULONG *fraction)
{

UINT    status;
ULONG   usecs;


    /* Check for possible overflow.  */
    if (msecs > 1000)
    {

        /* Return error condition.  */
        return NX_SNTP_OVERFLOW_ERROR;
    }

    /* Convert msecs to usecs first.  */
    usecs = msecs * 1000;

    /* Convert usecs to fraction.  */
    status = _nx_sntp_client_utility_usecs_to_fraction(usecs, fraction);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error status.  */
        return status;
    }

    /* Successful completion.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_sntp_client_utility_fraction_to_usecs          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the utility to convert      */
/*   fraction to microseconds.                                            */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    fraction                         Fraction to convert                */
/*    usecs                            Pointer to converted microseconds  */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_fraction_to_usecs                           */ 
/*                                     Actual usecs conversion service    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   Application code                                                     */
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
UINT    _nxe_sntp_client_utility_fraction_to_usecs(ULONG fraction, ULONG *usecs)
{

UINT status;


    if (usecs == NX_NULL)
    {
        return NX_SNTP_INVALID_TIME;
    }

    status = _nx_sntp_client_utility_fraction_to_usecs(fraction, usecs);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_fraction_to_usecs           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function converts a time stamp fraction to microseconds.  It is */
/*   primarily an intermediary function used in the process of converting */
/*   fixed point time fraction data to msecs.                             */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    fraction                         Fraction to convert to usecs       */
/*    usecs                            Pointer to ucsecs from fraction    */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_sntp_client_get_local_time_extended                              */ 
/*                                     Get extended local time            */ 
/*   _nx_sntp_client_utility_convert_fraction_to_msecs                    */ 
/*                                     Convert time fraction to msecs     */
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
UINT  _nx_sntp_client_utility_fraction_to_usecs(ULONG fraction, ULONG *usecs) 
{

ULONG value, segment;
int i;
    
    value = 0;

    for(i = 2; i < 32; i+=6)
    {
        segment = (fraction >> i) & 0x3F;
        
        if((value & 0x3F) >= 32)
            value = (value  >> 6) + segment * 15625 + 1;
        else
            value = (value  >> 6) + segment * 15625;
    } 
    *usecs = value;

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_convert_refID_KOD_code      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*  This function converts the reference ID field in a NTP time message   */
/*  data to a known Kiss of Death reference_id.                           */
/*                                                                        */
/*  Note the Kiss of Death reference_id list is subject to change.  This  */
/*  is current as of 4/01/2007.                                           */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    reference_id                 Pointer to the reference ID to convert */
/*    code_id                      Pointer to converted code              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                   Successful completion status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcmp                       Copy data to specified area of memory  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code (e.g. the Client kiss of death handler callback)   */ 
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
UINT  _nx_sntp_client_utility_convert_refID_KOD_code(UCHAR *reference_id, UINT *code_id)
{

/* This is the internal list of Kiss of Death codes and their meaning.  */


    /* Compare the reference_id to each known reference_id and if it matches return the reference_id ID.  */
    if (!memcmp(reference_id, ANYCAST, 4))
    {
        *code_id = NX_SNTP_KOD_ANYCAST;
    }
    else if (!memcmp(reference_id, AUTH_FAIL, 4))
    {
        *code_id = NX_SNTP_KOD_AUTH_FAIL;
    }
    else if (!memcmp(reference_id, AUTOKEY_FAIL, 4))
    {
        *code_id = NX_SNTP_KOD_AUTOKEY_FAIL;
    }
    else if (!memcmp(reference_id, BROADCAST, 4))
    {
        *code_id = NX_SNTP_KOD_BROADCAST;
    }
    else if (!memcmp(reference_id, CRYP_FAIL, 4))
    {
        *code_id = NX_SNTP_KOD_CRYP_FAIL;
    }
    else if (!memcmp(reference_id, DENY, 4))
    {
        *code_id = NX_SNTP_KOD_DENY;
    }
    else if (!memcmp(reference_id, DROP, 4))
    {
        *code_id = NX_SNTP_KOD_DROP;
    }
    else if (!memcmp(reference_id, DENY_POLICY, 4))
    {
        *code_id = NX_SNTP_KOD_DENY_POLICY;
    }
    else if (!memcmp(reference_id, NOT_INIT, 4))
    {
        *code_id = NX_SNTP_KOD_NOT_INIT;
    }
    else if (!memcmp(reference_id, MANYCAST, 4))
    {
        *code_id = NX_SNTP_KOD_MANYCAST;
    }
    else if (!memcmp(reference_id, NO_KEY, 4))
    {
        *code_id = NX_SNTP_KOD_NO_KEY;
    }
    else if (!memcmp(reference_id, RATE, 4))
    {
        *code_id = NX_SNTP_KOD_RATE;
    }
    else if (!memcmp(reference_id, RMOT, 4))
    {
        *code_id = NX_SNTP_KOD_RMOT;
    }
    else if (!memcmp(reference_id, STEP, 4))
    {
        *code_id = NX_SNTP_KOD_STEP;
    }
    else
    {
        /* Set reference_id ID to generic KOD 'other' reference_id.  */
        *code_id = NX_SNTP_KISS_OF_DEATH_PACKET;
    }

    /* Return successful completion.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_addition_overflow_check     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs a simple platform independent check for       */
/*   overflow when adding two operands.                                   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    temp1                            First addition operand             */
/*    temp2                            Second addition operand            */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SNTP_OVERFLOW_ERROR           Overflow result adding time        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_sntp_client_utility_add_msecs_to_ntp_time                       */ 
/*                                     Add msecs to an NTP time           */
/*    _nx_sntp_client_utility_add_NTPtime                                 */ 
/*                                     Add two NTP times                  */
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
UINT  _nx_sntp_client_utility_addition_overflow_check(ULONG temp1, ULONG temp2)
{

ULONG sum_lower_16, sum_upper_16;
UINT  carry;


    carry = 0;   

    /* Add the lower 16 bits.  */
    sum_lower_16 = (temp1 & NX_LOWER_16_MASK) + (temp2 & NX_LOWER_16_MASK);

    /* Check for carry.  */
    if (sum_lower_16 & NX_CARRY_BIT)
    {

        /* Use variable to add the carry to the upper 16 bits.*/
        carry = 1;
    }

    /* Add the upper 16 bits.  */
    sum_upper_16 = (temp1 >> NX_SHIFT_BY_16) + (temp2 >> NX_SHIFT_BY_16) + carry; 
    
    /* Check for carry.  */
    if (sum_upper_16 & NX_CARRY_BIT)
    {

        /* This indicates an overflow. Return as an error.  */
        return NX_SNTP_OVERFLOW_ERROR;
    }

    /* Ok to add operands!*/
    return NX_SUCCESS;
}