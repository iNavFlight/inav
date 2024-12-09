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
/**   Dynamic Host Configuration Protocol (DHCP) Client                   */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_DHCP_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include "nx_api.h"
#ifndef NX_DISABLE_IPV4
#include "nx_system.h"
#include "nx_ip.h"
#include "nx_arp.h"
#include "nxd_dhcp_client.h"
#include "tx_timer.h"

                               
/* Define the DHCP Internal Function.  */
static VOID        _nx_dhcp_thread_entry(ULONG ip_instance);
static UINT        _nx_dhcp_extract_information(NX_DHCP *dhcp_ptr, NX_DHCP_INTERFACE_RECORD *interface_record, UCHAR *dhcp_message, UINT length);
static UINT        _nx_dhcp_get_option_value(UCHAR *bootp_message, UINT option, ULONG *value, UINT length);
static UINT        _nx_dhcp_add_option_value(UCHAR *bootp_message, UINT option, UINT size, ULONG value, UINT *index);
static UINT        _nx_dhcp_add_option_string(UCHAR *bootp_message, UINT option, UINT size, UCHAR *value, UINT *index);
static UINT        _nx_dhcp_add_option_parameter_request(NX_DHCP *dhcp_ptr, UCHAR *bootp_message, UINT *index);
static ULONG       _nx_dhcp_update_timeout(ULONG timeout);
static ULONG       _nx_dhcp_update_renewal_timeout(ULONG timeout);
static UCHAR       *_nx_dhcp_search_buffer(UCHAR *option_message, UINT option, UINT length);
static ULONG       _nx_dhcp_get_data(UCHAR *data, UINT size);
static VOID        _nx_dhcp_store_data(UCHAR *data, UINT size, ULONG value);
static VOID        _nx_dhcp_move_string(UCHAR *dest, UCHAR *source, UINT size);
static UINT        _nx_dhcp_send_request_internal(NX_DHCP *dhcp_ptr, NX_DHCP_INTERFACE_RECORD *interface_record, UINT dhcp_message_type);
static UINT        _nx_dhcp_client_send_with_zero_source_address(NX_DHCP *dhcp_ptr, UINT iface_index, NX_PACKET *packet_ptr);
static ULONG       _nx_dhcp_add_randomize(ULONG timeout);
static VOID        _nx_dhcp_udp_receive_notify(NX_UDP_SOCKET *socket_ptr);
static VOID        _nx_dhcp_packet_process(NX_DHCP *dhcp_ptr, NX_DHCP_INTERFACE_RECORD *dhcp_interface, NX_PACKET *packet_ptr);
static VOID        _nx_dhcp_timeout_entry(ULONG dhcp);
static VOID        _nx_dhcp_timeout_process(NX_DHCP *dhcp_ptr);
static UINT        _nx_dhcp_interface_record_find(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_INTERFACE_RECORD **interface_record);


#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE
static VOID        _nx_dhcp_ip_conflict(NX_IP *ip_ptr, UINT interface_index, ULONG ip_address, ULONG physical_msw, ULONG physical_lsw);
#endif /* NX_DHCP_CLIENT_SEND_ARP_PROBE */



/* Define the Request string that specifies which options are to be added
   to the DHCP Client discover request to the server. Additional options
   (found in nx_dhcp.h) may be added by calling nx_dhcp_user_option_request().  */

UCHAR _nx_dhcp_request_parameters[] = { NX_DHCP_OPTION_SUBNET_MASK,
                                        NX_DHCP_OPTION_GATEWAYS,
                                        NX_DHCP_OPTION_DNS_SVR};

#define NX_DHCP_REQUEST_PARAMETER_SIZE sizeof(_nx_dhcp_request_parameters)

static struct NX_DHCP_STRUCT    *_nx_dhcp_created_ptr;

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_create                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP create function call.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    name_ptr                              DHCP name pointer             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_create                       Actual DHCP create function   */ 
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
UINT  _nxe_dhcp_create(NX_DHCP *dhcp_ptr, NX_IP *ip_ptr, CHAR *name_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (dhcp_ptr == NX_NULL))
    {
    
        return(NX_PTR_ERROR);
    }

    /* Call actual DHCP create service.  */
    status =  _nx_dhcp_create(dhcp_ptr, ip_ptr, name_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_create                                     PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the DHCP structure and associated IP      */ 
/*    instance for DHCP operation.  In doing so, it creates a packet      */ 
/*    pool for DHCP messages, a UDP socket for communication with the     */ 
/*    server, and a DHCP processing thread.                               */ 
/*                                                                        */
/*    The primary interface is automatically enabled for DHCP.  To run    */
/*    DHCP on a different interface, use the nx_dhcp_set_interface_index  */
/*    service.  To run DHCP on multiple interfaces, see                   */
/*    nx_dhcp_interface_enable and nx_dhcp_interface_start                */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    name_ptr                              DHCP name pointer             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_pool_create                 Create the DHCP packet pool   */ 
/*    nx_packet_pool_delete                 Delete the DHCP packet pool   */ 
/*    nx_udp_socket_create                  Create the DHCP UDP socket    */ 
/*    nx_udp_socket_delete                  Delete the DHCP UDP socket    */ 
/*    tx_mutex_create                       Create DHCP mutex             */ 
/*    tx_mutex_delete                       Delete DHCP mutex             */ 
/*    tx_thread_create                      Create DHCP processing thread */ 
/*    tx_timer_create                       Create DHCP timer             */ 
/*    tx_timer_delete                       Delete DHCP timer             */ 
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the code,          */
/*                                            resulting in version 6.1.8  */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), supported*/
/*                                            multiple client instances,  */
/*                                            resulting in version 6.1.10 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), cleaned  */
/*                                            up error check logic, and   */
/*                                            properly terminated thread, */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcp_create(NX_DHCP *dhcp_ptr, NX_IP *ip_ptr, CHAR *name_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT    status;
#ifdef NX_DHCP_CLIENT_ENABLE_HOST_NAME_CHECK
CHAR    *temp_ptr;
UINT    label_length = 0;
#endif /* NX_DHCP_CLIENT_ENABLE_HOST_NAME_CHECK  */


#ifdef NX_DHCP_CLIENT_ENABLE_HOST_NAME_CHECK
  
    /* Set the pointer.  */
    temp_ptr = name_ptr;

    /* The total length of a domain name (host name) is restricted to 255 octets or less.  */
    if (_nx_utility_string_length_check(temp_ptr, NX_NULL, 255))
    {
        return (NX_DHCP_INVALID_NAME);
    }

    /* The labels must follow the rules for ARPANET host names.  They must
       start with a letter, end with a letter or digit, and have as interior
       characters only letters, digits, and hyphen.  There are also some
       restrictions on the length. Labels must be 63 characters or less. 
       RFC 1035, Section 2.3.1, Page8.  */
    while (*temp_ptr)
    {

        /* Is a dot?  */
        if (*temp_ptr == '.')
        {

            /* Current label is end.  */

            /* Labels must be 63 characters or less, and check the continuous dot '..' for host name.  */
            if ((label_length == 0) || (label_length > 63))
                return (NX_DHCP_INVALID_NAME);

            /* End with a letter or digit. Only need to check the Hyphen '-' since
               *(temp_ptr - 1) must be Letter, Hyphen or Digits in 'else'.  */
            if (*(temp_ptr - 1) == '-')
                return (NX_DHCP_INVALID_NAME);

            /* Continue the next lable.  */
            label_length = 0;
        }
        else
        {

            /* Update the lable length.  */
            label_length++;

            /* Letter.  */
            if((((*temp_ptr) | 0x20) >= 'a') && (((*temp_ptr) | 0x20) <= 'z'))
            {

                /* Continue next character.  */
                temp_ptr++;
                continue;
            }

            /* Start with a letter.  */
            if (label_length == 1)
                return (NX_DHCP_INVALID_NAME);

            /* Hyphen or Digits.  */
            if ((*temp_ptr != '-') &&
                ((*temp_ptr < '0') || (*temp_ptr > '9')))
                return (NX_DHCP_INVALID_NAME);
        }

        /* Continue next character.  */
        temp_ptr++;
    }

    /* Check again if the host name have not the 'dot' terminator.  */
    if (*(temp_ptr - 1) != '.')
    {

        /* Labels must be 63 characters or less.  */
        if (label_length > 63)
            return (NX_DHCP_INVALID_NAME);

        /* End with a letter or digit. Only need to check the Hyphen '-' since
            *(temp_ptr - 1) must be Letter, Hyphen or Digits in 'else'.  */
        if (*(temp_ptr - 1) == '-')
            return (NX_DHCP_INVALID_NAME);
    }
#endif /* NX_DHCP_CLIENT_ENABLE_HOST_NAME_CHECK  */

    /* Initialize the DHCP control block to zero.  */
    memset((void *) dhcp_ptr, 0, sizeof(NX_DHCP));
    
    /* Save the IP pointer.  */
    dhcp_ptr -> nx_dhcp_ip_ptr =  ip_ptr;

    /* Save the DHCP name.  */
    dhcp_ptr -> nx_dhcp_name =  name_ptr;

    /* If the host does not intend to supply their own packet pool, create one here. */
#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL

    /* Check the packet payload size, DHCP Client must be prepared to receive a message of up to 576 octets.   
       RFC 2131, Section 2, Page10.  */
    if (NX_DHCP_PACKET_PAYLOAD < (NX_PHYSICAL_HEADER + NX_DHCP_MINIMUM_IP_DATAGRAM)) 
    {

        /* Invalid payload, return error status.  */
        return(NX_DHCP_INVALID_PAYLOAD);
    }

    /* Create the pool and check the status */
    nx_packet_pool_create(&dhcp_ptr -> nx_dhcp_pool, "NetX DHCP Client", NX_DHCP_PACKET_PAYLOAD, 
                          dhcp_ptr -> nx_dhcp_pool_area, NX_DHCP_PACKET_POOL_SIZE);

    /* Set an internal packet pool pointer to the newly created packet pool. */
    dhcp_ptr -> nx_dhcp_packet_pool_ptr = &dhcp_ptr -> nx_dhcp_pool;
                                             
#ifdef NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION

    /* Set the maximum DHCP message size.  */
    dhcp_ptr -> nx_dhcp_max_dhcp_message_size = NX_DHCP_PACKET_PAYLOAD - NX_PHYSICAL_HEADER;
#endif /* NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION  */

#endif /* NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL  */

    /* Create the Socket and check the status */
    nx_udp_socket_create(ip_ptr, &(dhcp_ptr -> nx_dhcp_socket), "NetX DHCP Client",
                         NX_DHCP_TYPE_OF_SERVICE, NX_DHCP_FRAGMENT_OPTION, NX_DHCP_TIME_TO_LIVE, NX_DHCP_QUEUE_DEPTH);

    /* Set the UDP socket receive callback function.  */
    nx_udp_socket_receive_notify(&(dhcp_ptr -> nx_dhcp_socket), _nx_dhcp_udp_receive_notify);

    dhcp_ptr -> nx_dhcp_socket.nx_udp_socket_reserved_ptr = (VOID*)dhcp_ptr;

    /* Create the ThreadX activity timeout timer.  This will be used to periodically check to see if
       a client connection has gone silent and needs to be terminated.  */
    status =  tx_timer_create(&(dhcp_ptr -> nx_dhcp_timer), "DHCP Client Timer", _nx_dhcp_timeout_entry,
                              (ULONG)(ALIGN_TYPE)dhcp_ptr, (NX_DHCP_TIME_INTERVAL), 
                              (NX_DHCP_TIME_INTERVAL), TX_NO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(dhcp_ptr -> nx_dhcp_timer), dhcp_ptr)

    /* Determine if the semaphore creation was successful.  */
    if (status != TX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL 
        /* Delete the packet pool.  */
        nx_packet_pool_delete(dhcp_ptr -> nx_dhcp_packet_pool_ptr);
#endif

        /* No, return error status.  */
        return(status);
    }

    /* Create the DHCP mutex.  */
    status =  tx_mutex_create(&(dhcp_ptr -> nx_dhcp_mutex), "NetX DHCP Client", TX_NO_INHERIT);

    /* Determine if the semaphore creation was successful.  */
    if (status != TX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL 
        /* Delete the packet pool.  */
        nx_packet_pool_delete(dhcp_ptr -> nx_dhcp_packet_pool_ptr);
#endif

        /* Delete the timer.  */
        tx_timer_delete(&(dhcp_ptr -> nx_dhcp_timer));

        /* No, return error status.  */
        return(status);
    }

    /* Create the DHCP processing thread.  */
    status =  tx_thread_create(&(dhcp_ptr -> nx_dhcp_thread), "NetX DHCP Client", _nx_dhcp_thread_entry, (ULONG)(ALIGN_TYPE)dhcp_ptr,
                                dhcp_ptr -> nx_dhcp_thread_stack, NX_DHCP_THREAD_STACK_SIZE, 
                                NX_DHCP_THREAD_PRIORITY, NX_DHCP_THREAD_PRIORITY, 1, TX_DONT_START);

    NX_THREAD_EXTENSION_PTR_SET(&(dhcp_ptr -> nx_dhcp_thread), dhcp_ptr)

    /* Determine if the thread creation was successful.  */
    if (status != TX_SUCCESS)
    {

        /* Delete the mutex.  */
        tx_mutex_delete(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL 
        /* Delete the packet pool.  */
        nx_packet_pool_delete(dhcp_ptr -> nx_dhcp_packet_pool_ptr);
#endif

        /* Delete the timer.  */
        tx_timer_delete(&(dhcp_ptr -> nx_dhcp_timer));

        /* No, return error status.  */
        return(status);
    }

    /* Create a DHCP event flag group. .  */
    status = tx_event_flags_create(&(dhcp_ptr -> nx_dhcp_events), (CHAR *)"DHCP Client Events");

    /* Check for error. */
    if (status != TX_SUCCESS)
    {

        /* First put the thread into TERMINATE state. */
        tx_thread_terminate(&(dhcp_ptr -> nx_dhcp_thread));

        /* Delete the thread.  */
        tx_thread_delete(&(dhcp_ptr -> nx_dhcp_thread));

        /* Delete the mutex.  */
        tx_mutex_delete(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Delete the timer.  */
        tx_timer_delete(&(dhcp_ptr -> nx_dhcp_timer));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL 
        /* Delete the packet pool.  */
        nx_packet_pool_delete(dhcp_ptr -> nx_dhcp_packet_pool_ptr);
#endif

        /* No, return error status.  */
        return(status);
    }

    /* Otherwise, the DHCP initialization was successful.  Place the
       DHCP control block on the list of created DHCP instances.  */
    TX_DISABLE

    /* Update the dhcp structure ID.  */
    dhcp_ptr -> nx_dhcp_id =  NX_DHCP_ID;

    /* Setup this DHCP's created links.  */
    dhcp_ptr -> nx_dhcp_created_next = _nx_dhcp_created_ptr;

    /* Place the new DHCP control block on the head of created DHCPs.  */
    _nx_dhcp_created_ptr = dhcp_ptr;

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Default enable DHCP on the primary interface (0).  */
    _nx_dhcp_interface_enable(dhcp_ptr, 0);

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_clear_broadcast_flag                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP set the clear broadcast */
/*    flag in DHCP Client messages service.                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    clear_flag                            Broadcast flag status         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_broadcast_flag         Actual DHCP Client clear      */
/*                                            broadcast flag service      */ 
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
UINT  _nxe_dhcp_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT clear_flag)
{

UINT    status;

    /* Check for invalid input pointer.  */
    if (dhcp_ptr == NX_NULL)
    {

        return(NX_PTR_ERROR);
    }

    /* Call actual DHCP clear flag service.  */
    status =  _nx_dhcp_clear_broadcast_flag(dhcp_ptr, clear_flag);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_broadcast_flag                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This service uses the clear_flag input to clear or set the broadcast*/
/*    in DHCP Client messages to the DHCP server. By default the broadcast*/
/*    flag is set.                                                        */
/*                                                                        */ 
/*    This function is intended for DHCP Clients whose DHCP messages are  */
/*    routed through a relay agent (router) that will not accept DHCP     */
/*    messages with broadcast replies requested.                          */
/*                                                                        */ 
/*         NX_TRUE        broadcast flag is cleared for all messages      */
/*         NX_FALSE       broadcast flag set only if it normally would be */ 
/*                                                                        */ 
/*    The server reply will actually be broadcast when relayed by the     */
/*    router using this strategy.                                         */
/*                                                                        */ 
/*    If DHCP is enabled on multiple interfaces, this service will set the*/
/*    broadcast flag on all interfaces. To set this flag on a specific    */
/*    interface, use the nx_dhcp_interface_clear_broadcast_flag.          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    clear_flag                            Broadcast flag status         */ 
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
UINT  _nx_dhcp_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT clear_flag)
{

UINT    i;

    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Loop to set the broadcast flag for all interface record enabled.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid == NX_TRUE)
        {

            dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_clear_broadcast = clear_flag;
        }
    }

    /* Release the mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_clear_broadcast_flag            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the interface specific clear     */
/*    broadcast flag service.                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Network interface index       */
/*    clear_flag                            Broadcast flag status         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_clear_broadcast_flag Actual DHCP Client clear    */
/*                                            broadcast flag service      */ 
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
UINT  _nxe_dhcp_interface_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT iface_index, UINT clear_flag)
{

UINT    status;

    /* Check for invalid input pointer.  */
    if (dhcp_ptr == NX_NULL)
    {

        return(NX_PTR_ERROR);
    }

    /* Check interface index.  */
    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Call actual DHCP clear flag service.  */
    status =  _nx_dhcp_interface_clear_broadcast_flag(dhcp_ptr, iface_index, clear_flag);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interace_clear_broadcast_flag              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This service uses the clear_flag input to determine if the DHCP     */
/*    Client on the specified network interface should clear the broadcast*/
/*    flag in its DHCP messages to the DHCP server (normally it sets the  */
/*    broadcast flag.                                                     */
/*                                                                        */ 
/*    This is intended for DHCP Clients whose DHCP messages are routed    */
/*    through arelay agent (router) that will not accept DHCP messages    */
/*    withbroadcast replies requested.                                    */
/*                                                                        */ 
/*         NX_TRUE        broadcast flag is cleared for all messages      */
/*         NX_FALSE       broadcast flag set only if it normally would be */ 
/*                                                                        */ 
/*    The server reply will actually be broadcast when relayed by the     */
/*    router using this strategy.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Network interface index       */
/*    clear_flag                            Broadcast flag status         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_dhcp_interface_record_find        Find Client record for input  */
/*                                              DHCP interface            */
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
UINT  _nx_dhcp_interface_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT iface_index, UINT clear_flag)
{

UINT    status; 
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;
  
    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);
   
    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }
    
    /* Set or clear the option to request unicast/clear broadcast flag. */
    interface_record -> nx_dhcp_clear_broadcast = clear_flag;

    /* Release the mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_packet_pool_set                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the set the DHCP Client   */
/*    packet pool service.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    packet_pool_ptr                       Pointer to packet pool        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_SUCCESS                            Packet pool successfully set  */
/*    NX_PTR_ERROR                          Invalid pointer input         */ 
/*    NX_NOT_ENABLED                        DHCP client not enabled for   */ 
/*                                             user create packet pool    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_packet_pool_set                                            */ 
/*                                          Actual set packet pool service*/ 
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
UINT  _nxe_dhcp_packet_pool_set(NX_DHCP *dhcp_ptr, NX_PACKET_POOL *packet_pool_ptr)
{

#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL
    NX_PARAMETER_NOT_USED(dhcp_ptr);
    NX_PARAMETER_NOT_USED(packet_pool_ptr);

    /* Client not configured for the user creating the packet pool. Return an error status. */
    return NX_NOT_ENABLED;
#else

UINT  status;


    /* Check for invalid pointer input. */
    if ((dhcp_ptr == NX_NULL) || (packet_pool_ptr == NX_NULL))
    {

        return NX_PTR_ERROR;
    }
                        
    /* Check the packet payload size, DHCP Client must be prepared to receive a message of up to 576 octets.   
       RFC2131, Section2, Page10.  */
    if (packet_pool_ptr -> nx_packet_pool_payload_size < (NX_PHYSICAL_HEADER + NX_DHCP_MINIMUM_IP_DATAGRAM)) 
    {

        /* Invalid payload, return error status.  */
        return(NX_DHCP_INVALID_PAYLOAD);
    }

    status = _nx_dhcp_packet_pool_set(dhcp_ptr, packet_pool_ptr);

    return status;
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_packet_pool_set                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the DHCP Client packet pool by passing in a      */
/*    packet pool pointer to packet pool already create.  The             */
/*    NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL must be set.                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    packet_pool_ptr                       Pointer to packet pool        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*    NX_NOT_ENABLED                        Setting DHCP Client packet    */ 
/*                                             pool not enabled           */ 
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
UINT  _nx_dhcp_packet_pool_set(NX_DHCP *dhcp_ptr, NX_PACKET_POOL *packet_pool_ptr)
{


    /* Determine if the DHCP Client is configured for accepting a packet pool pointer. */
#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL
    NX_PARAMETER_NOT_USED(dhcp_ptr);
    NX_PARAMETER_NOT_USED(packet_pool_ptr);

    /* No, return the error status. */
    return NX_NOT_ENABLED;  
#else

    /* Set the Client packet pool to the supplied packet pool. */
    dhcp_ptr -> nx_dhcp_packet_pool_ptr = packet_pool_ptr;  

#ifdef NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION

    /* Set the maximum DHCP message size.  */
    dhcp_ptr -> nx_dhcp_max_dhcp_message_size = packet_pool_ptr -> nx_packet_pool_payload_size - NX_PHYSICAL_HEADER;    
#endif /* NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION  */

    return NX_SUCCESS;
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_reinitialize                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the reinitialize service. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
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
UINT _nxe_dhcp_reinitialize(NX_DHCP *dhcp_ptr)
{

UINT status;

    if (dhcp_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    /* Call the actual reinitialize service.  */
    status = _nx_dhcp_reinitialize(dhcp_ptr);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_reinitialize                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reinitializes all enabled DHCP interfaces for         */
/*    restarting the DHCP client protocol.  Network parameters in the DHCP*/
/*    Client and IP interface(s) are both cleared, and sets the DHCP state*/
/*    back to the not started state.                                      */
/*                                                                        */ 
/*    To reinitialize the DHCP Client on a specific interface when        */
/*    multiple interfaces are enabled for DHCP, use the                   */
/*    nx_dhcp_interface_reinitialize service.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcp_interface_reinitialize       Reinitialize DHCP interface   */ 
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
UINT _nx_dhcp_reinitialize(NX_DHCP *dhcp_ptr)
{

UINT    i;

    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), NX_WAIT_FOREVER);

    /* Loop to reinitalize the record.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid == NX_TRUE)
        {

            /* Reinitialize the record.  */
           _nx_dhcp_interface_reinitialize(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index);
        }
    }

    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_reinitialize                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the reinitialize service.  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Specify interface to init     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */
/*    NX_INVALID_INTERFACE                  Index exceeds max interface   */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_reinitialize       Actual reinitialize service   */ 
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
UINT _nxe_dhcp_interface_reinitialize(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT status;

    if (dhcp_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return NX_INVALID_INTERFACE;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual reinitialize service.  */
    status = _nx_dhcp_interface_reinitialize(dhcp_ptr, iface_index);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_reinitialize                     PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reinitializes the DHCP instance for restarting the    */
/*    DHCP client state machine and re-running the DHCP protocol on the   */
/*    specified interface.  The IP address and other network parameters in*/
/*    the DHCP Client and in the IP interface are cleared.                */
/*                                                                        */ 
/*    This function also sets the DHCP server IP address to the broadcast */
/*    address and sets the DHCP client state back to the INIT state.      */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Specify interface to init     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find        Find Client record  for input */
/*                                             DHCP interface             */
/*    nx_ip_interface_address_set           Clear IP inteface address of  */
/*                                             the input DHCP interface   */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    nx_ip_gateway_address_clear           Clear the gateway address     */
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcp_interface_reinitialize(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT                      status;
ULONG                     ip_address;
ULONG                     network_mask;
ULONG                     gateway_address;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;

  
    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), NX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Check if have IP address.  */
    if (interface_record -> nx_dhcp_ip_address)
    {

        /* Get the IP address.  */
        nx_ip_interface_address_get(dhcp_ptr -> nx_dhcp_ip_ptr, iface_index, &ip_address, &network_mask);

        /* Check if the IP address is set by DHCP.  */
        if (ip_address == interface_record -> nx_dhcp_ip_address)
        {

            /* Clear the IP address.  */
            nx_ip_interface_address_set(dhcp_ptr -> nx_dhcp_ip_ptr, iface_index, 0, 0);
        }
    }

    /* Check if have gateway address.  */
    if (interface_record -> nx_dhcp_gateway_address)
    {

        /* Get the gateway address.  */
        status = nx_ip_gateway_address_get(dhcp_ptr -> nx_dhcp_ip_ptr, &gateway_address);

        /* Check status.  */
        if ((status == NX_SUCCESS) && (gateway_address == interface_record -> nx_dhcp_gateway_address))
        {

            /* Clear the Gateway/Router IP address.  */
            nx_ip_gateway_address_clear(dhcp_ptr -> nx_dhcp_ip_ptr);
        }
    }

    /* Initialize the client DHCP IP address with the NULL IP address.  */
    interface_record -> nx_dhcp_ip_address =  NX_BOOTP_NO_ADDRESS; 

    /* Initialize the client DHCP server IP address.  */
    interface_record -> nx_dhcp_server_ip =  NX_BOOTP_NO_ADDRESS; 

    /* Clear these DHCP Client network values too.*/
    interface_record -> nx_dhcp_gateway_address = NX_BOOTP_NO_ADDRESS; 
    interface_record -> nx_dhcp_network_mask = NX_BOOTP_NO_ADDRESS;

    /* Clear the flag to skip the discovery step. The host application must
       call the nx_dhcp_request_ip_address to reset the flag and the requested IP address. */
    interface_record -> nx_dhcp_skip_discovery = NX_FALSE;
                                                             
    /* Initialize renew and rebind timeout values to zero.  */
    interface_record -> nx_dhcp_rebind_time = 0;
    interface_record -> nx_dhcp_renewal_time =  0;

    /* Setup for infinite lease time request.  */
    interface_record -> nx_dhcp_lease_time =  NX_DHCP_INFINITE_LEASE;

    /* Reset the seconds field for starting the DHCP request process. */
    interface_record -> nx_dhcp_seconds = 0;

    /* Reset the timeout and retransmission interval.  */
    interface_record -> nx_dhcp_timeout = 0;
    interface_record -> nx_dhcp_rtr_interval = 0;

    /* Set the DHCP state to the initial state.  */
    interface_record -> nx_dhcp_state =  NX_DHCP_STATE_NOT_STARTED;

    /* Release the mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_request_client_ip                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the client ip request     */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    client_request_address                Client's requested IP address */ 
/*    skip_discover_message                 Initialize Client state to    */
/*                                            BOOT (skip discover message)*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
/*    status                                Completion status             */ 
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
UINT  _nxe_dhcp_request_client_ip(NX_DHCP *dhcp_ptr, ULONG client_request_address, UINT skip_discover_message)
{

UINT status;


    /* Check for an invalid DHCP pointer .  */
    if (dhcp_ptr == NX_NULL)
    {
    
        /* Return an error.  */
        return(NX_PTR_ERROR);
    }

    /* Determine that a nonzero IP address is entered. */
    if (client_request_address == NX_BOOTP_NO_ADDRESS)
    {

        return NX_DHCP_INVALID_IP_REQUEST;
    }

    /* Call actual request client IP service.  */
    status = _nx_dhcp_request_client_ip(dhcp_ptr, client_request_address, skip_discover_message);

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_request_client_ip                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the requested client IP address in DHCP Client   */ 
/*    messages, and enables or disables the skip discovery option         */
/*    based on the skip_discover_message input.                           */
/*                                                                        */ 
/*    If multiple interfaces are enabled for DHCP, this function sets the */
/*    requested Client IP on the first valid interface it finds.          */
/*                                                                        */ 
/*    To set the requested Client IP for a specific interface, use the    */
/*    nx_dhcp_interface_request_ip() service.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    client_request_address                Client's requested IP address */ 
/*    skip_discover_message                 To set DHCP Client flag       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_request_client_ip  Interface specific request    */ 
/*                                            IP service                  */
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
UINT  _nx_dhcp_request_client_ip(NX_DHCP *dhcp_ptr, ULONG client_request_address, UINT skip_discover_message)
{

UINT    i;
UINT    status;

    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), NX_WAIT_FOREVER);

    /* Find the DHCP interface record.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid)
        {

            /* Set the request IP.  */  
            status = _nx_dhcp_interface_request_client_ip(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index, 
                                                          client_request_address, skip_discover_message);

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_DHCP_NO_INTERFACES_ENABLED);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_request_client_ip               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the request IP service.    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Index to apply parameter to   */ 
/*    client_request_address                Client's requested IP address */ 
/*    skip_discover_message                 To set DHCP Client flag       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
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
UINT  _nxe_dhcp_interface_request_client_ip(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG client_request_address, UINT skip_discover_message)
{

UINT status;

    if (dhcp_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return NX_INVALID_INTERFACE;
    }

    status = _nx_dhcp_interface_request_client_ip(dhcp_ptr, iface_index, client_request_address, skip_discover_message);

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_request_client_ip                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the requested client IP address in the client    */ 
/*    REQUEST message on the input interface. If the skip_discover_message*/
/*    flag is set, the DISCOVER message is skipped when the DHCP Client   */
/*    (re))starts.                                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Index to apply parameter to   */ 
/*    client_request_address                Client's requested IP address */ 
/*    skip_discover_message                 To set DHCP Client flag       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find        Find Client record for the    */
/*                                            input interface             */
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
UINT  _nx_dhcp_interface_request_client_ip(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG client_request_address, UINT skip_discover_message)
{

UINT    status;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Use input IP address.  */
    interface_record -> nx_dhcp_ip_address =  client_request_address;

    /* Set the flag for skipping the discovery state. */
    interface_record -> nx_dhcp_skip_discovery =  skip_discover_message;

    /* Release the mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return success!  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_delete                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP delete function call.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_delete                       Actual DHCP delete function   */ 
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
UINT  _nxe_dhcp_delete(NX_DHCP *dhcp_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP delete service.  */
    status =  _nx_dhcp_delete(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_delete                                     PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function disables DHCP on all enabled interfaces, deletes the  */
/*    DHCP instance and releases all of its resources. All DHCP and IP    */
/*    interface IP addresses and the IP gateway are cleared on all        */
/*    valid DHCP interfaces.                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    nx_dhcp_interface_disable             Disable DHCP on interface     */ 
/*    nx_packet_pool_delete                 Delete the DHCP packet pool   */ 
/*    nx_udp_socket_delete                  Delete the DHCP UDP socket    */ 
/*    tx_thread_suspend                     Suspend DHCP Thread           */
/*    tx_thread_terminate                   Terminate DHCP thread         */ 
/*    tx_mutex_delete                       Delete DHCP mutex             */
/*    tx_event_flags_delete                 Delete DHCP flag group        */
/*    tx_timer_deactivate                   Stop the DHCP timer           */
/*    tx_timer_delete                       Delete the DHCP timer         */
/*    tx_thread_delete                      Delete DHCP thread            */ 
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
/*  01-31-2022     Yuxin Zhou               Modified comment(s), supported*/
/*                                            multiple client instances,  */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcp_delete(NX_DHCP *dhcp_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT    i;
NX_DHCP *dhcp_previous;

    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Disable all the interfaces enabled for DHCP. */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

       /* Check if this interface (i) is enabled for DHCP. */
       if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) 
       {

           /* Disable the interface record.  */
           _nx_dhcp_interface_disable(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index);
       }
    }

    tx_thread_suspend(&(dhcp_ptr -> nx_dhcp_thread));

    /* Terminate the DHCP processing thread.  */
    tx_thread_terminate(&(dhcp_ptr -> nx_dhcp_thread));

    /* Delete the DHCP processing thread.  */
    tx_thread_delete(&(dhcp_ptr -> nx_dhcp_thread)); 

    /* Delete the DHCP event flags.  */
    tx_event_flags_delete(&(dhcp_ptr -> nx_dhcp_events));

    tx_timer_deactivate(&(dhcp_ptr -> nx_dhcp_timer));

    tx_timer_delete(&(dhcp_ptr -> nx_dhcp_timer));

    /* Unbind the port.  */
    nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket));

    /* Delete the UDP socket.  */
    nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL 
    /* Delete the packet pool.  */
    nx_packet_pool_delete(dhcp_ptr -> nx_dhcp_packet_pool_ptr);
#endif

    /* Get the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));   

    /* Delete the DHCP mutex.  */
    tx_mutex_delete(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Disable interrupts.  */
    TX_DISABLE

    /* Clear the dhcp structure ID. */
    dhcp_ptr -> nx_dhcp_id =  0;

    /* Remove the DHCP instance from the created list.  */
    if (_nx_dhcp_created_ptr == dhcp_ptr)
    {
        _nx_dhcp_created_ptr = _nx_dhcp_created_ptr -> nx_dhcp_created_next;
    }
    else
    {
        for (dhcp_previous = _nx_dhcp_created_ptr;
             dhcp_previous -> nx_dhcp_created_next;
             dhcp_previous = dhcp_previous -> nx_dhcp_created_next)
        {
            if (dhcp_previous -> nx_dhcp_created_next == dhcp_ptr)
            {
                dhcp_previous -> nx_dhcp_created_next  = dhcp_ptr -> nx_dhcp_created_next;
                break;
            }
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                                RELEASE       */ 
/*                                                                        */ 
/*    _nxe_dhcp_force_renew                                PORTABLE C     */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP force renew function    */
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                               Pointer to DHCP instance     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                 Completion status            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_force_renew                   Actual DHCP force renew      */ 
/*                                             function                   */ 
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
UINT  _nxe_dhcp_force_renew(NX_DHCP *dhcp_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP force renew service.  */
    status =  _nx_dhcp_force_renew(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_force_renew                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function forces the DHCP processing thread to renew the lease  */
/*    on all valid (DHCP enabled) interfaces. To force renew on a specific*/
/*    interface if multiple interfaces are enabled for DHCP, use the      */
/*    nx_dhcp_interface_force_renew service.                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
/*    nx_dhcp_interface_force_renew         Force renew on DHCP interface */ 
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
UINT  _nx_dhcp_force_renew(NX_DHCP *dhcp_ptr)
{

UINT    i;


    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */                                       
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state >= NX_DHCP_STATE_BOUND))
        {

            /* Force renew on this interface.  */
            _nx_dhcp_interface_force_renew(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index);  
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    return (NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_force_renew                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function forces the DHCP processing thread to renew the lease  */
/*    on the specified interface.                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to force renew on   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_dhcp_interface_force_renew         Actual force renew service    */ 
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
UINT  _nxe_dhcp_interface_force_renew(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT status;

    if (dhcp_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Call the actual reinitialize service.  */
    status = _nx_dhcp_interface_force_renew(dhcp_ptr, iface_index);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_force_renew                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function allows the DHCP Client to force a renew on a          */
/*    previously obtained lease on the input interface.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface to force renew on   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find       Find record for the interface  */ 
/*    _nx_dhcp_send_request_internal       Send request to DHCP server    */ 
/*    _nx_dhcp_update_renewal_timeout      Update time left on renew lease*/ 
/*    tx_mutex_get                         Obtain protection mutex        */ 
/*    tx_mutex_put                         Release protection mutex       */ 
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
UINT  _nx_dhcp_interface_force_renew(NX_DHCP *dhcp_ptr, UINT iface_index)
{
UINT    status; 
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record for this interface.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Return status.  */
        return(status);
    }

    /* Determine if DHCP is in the Bound state.  */
    if (interface_record -> nx_dhcp_state >= NX_DHCP_STATE_BOUND)
    {

        /* Since the Client is initiating RENEW ('force' renew'), set the state to RENEW. */
        interface_record -> nx_dhcp_state = NX_DHCP_STATE_RENEWING;

        /* Send the renew request. */
        _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPREQUEST);

        /* Reset the renew time remaining (before transition to the REBIND state) and
           the dhcp timeout for retransmission of RENEW request. */
        interface_record -> nx_dhcp_renewal_remain_time = interface_record -> nx_dhcp_rebind_time - interface_record -> nx_dhcp_renewal_time;
        interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_renewal_remain_time);

        /* Record the retransmission interval.  */
        interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;

        /* Determine if the application has specified a routine for DHCP state change notification.  */
        if (dhcp_ptr -> nx_dhcp_state_change_callback)
        {

            /* Yes, call the application's state change notify function with the new state.  */
            (dhcp_ptr -> nx_dhcp_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_state);
        }

        /* Determine if the application has specified a routine for DHCP interface state change notification.  */
        if (dhcp_ptr -> nx_dhcp_interface_state_change_callback)
        {

            /* Yes, call the application's state change notify function with the new state.  */
            (dhcp_ptr -> nx_dhcp_interface_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_interface_index, interface_record -> nx_dhcp_state);
        }
    }
    else
    {

        /* Return a not bound error code.  */
        status =  NX_DHCP_NOT_BOUND;
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return completion status.  */
    return(status);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_decline                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP decline function call.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_decline                      Actual DHCP decline function  */ 
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
UINT  _nxe_dhcp_decline(NX_DHCP *dhcp_ptr)
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP release service.  */
    status =  _nx_dhcp_decline(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_decline                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function declines the IP address(es) assigned to the DHCP      */ 
/*    Client on all valid interfaces.  The DHCP Client state is set back  */ 
/*    to the INIT state and DHCP is automatically restarted.              */ 
/*                                                                        */ 
/*    To decline an IP address on a specific interface where multiple     */
/*    interfaces are enabled for DHCP, use the                            */
/*    nx_dhcp_interface_decline service.                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_decline            Interface specific decline    */
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
UINT  _nx_dhcp_decline(NX_DHCP *dhcp_ptr)
{

UINT    i;

    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);\

    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state >= NX_DHCP_STATE_BOUND))
        {

            /* Decline the address.  */
            _nx_dhcp_interface_decline(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index);
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_decline                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the decline service.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface to decline IP on    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_decline            Actual decline service        */  
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

UINT _nxe_dhcp_interface_decline(NX_DHCP *dhcp_ptr, UINT iface_index)
{
UINT status;

    if (dhcp_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)  
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual decline service.  */
    status = _nx_dhcp_interface_decline(dhcp_ptr, iface_index);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_decline                          PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function declines the IP assigned to the input DHCP Client     */ 
/*    interface. The DHCP Client state is set back to the INIT state and  */
/*    DHCP is automatically restarted.                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface to decline IP on    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find        Find record for the interface */
/*    _nx_dhcp_send_request_internal        Send DHCP request             */ 
/*    _nx_dhcp_interface_reinitialize       Reset IP and DHCP Client      */
/*                                            network parameters          */
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcp_interface_decline(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT                      status; 
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr ->nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Check the record state.  */
    if (interface_record -> nx_dhcp_state < NX_DHCP_STATE_BOUND)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(NX_DHCP_NOT_BOUND);
    }

    /* Send the decline DHCP request.  */
    _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPDECLINE);

    /* Reinitialize DHCP.  */
    _nx_dhcp_interface_reinitialize(dhcp_ptr, iface_index);

    /* Start the DHCP protocol again by setting the state back to INIT. */
    interface_record -> nx_dhcp_state = NX_DHCP_STATE_INIT;

    /* Wait for some time before restarting the configuration process to avoid excessive network traffic in case of looping. RFC2131, Section3.1, Page17.  */
    interface_record -> nx_dhcp_timeout = NX_DHCP_RESTART_WAIT;

    /* Set the retransmission interval.  */
    interface_record -> nx_dhcp_rtr_interval = 0;

    /* Determine if the application has specified a routine for DHCP state change notification.  */
    if (dhcp_ptr -> nx_dhcp_state_change_callback)
    {

        /* Yes, call the application's state change notify function with the new state.  */
        (dhcp_ptr -> nx_dhcp_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_state);
    }

    /* Determine if the application has specified a routine for DHCP interface state change notification.  */
    if (dhcp_ptr -> nx_dhcp_interface_state_change_callback)
    {

        /* Yes, call the application's state change notify function with the new state.  */
        (dhcp_ptr -> nx_dhcp_interface_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_interface_index, interface_record -> nx_dhcp_state);
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_release                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP release function call.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_release                      Actual DHCP release function  */ 
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
UINT  _nxe_dhcp_release(NX_DHCP *dhcp_ptr)
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP release service.  */
    status =  _nx_dhcp_release(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_release                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function releases the IP previously assigned to this DHCP      */ 
/*    instance on all interfaces enabled for DHCP. A subsequent  call to  */
/*    nx_dhcp_start initiates a new request to obtain an IP address.      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_release            Interface specific send       */ 
/*                                            release request service     */
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
UINT _nx_dhcp_release(NX_DHCP *dhcp_ptr)
{

UINT    i;

    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state >= NX_DHCP_STATE_BOUND))
        {

            /* Release the address.  */
            _nx_dhcp_interface_release(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index);
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return.  */
    return (NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_release                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the release service.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to release IP on    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_release            Actual release service        */ 
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
UINT _nxe_dhcp_interface_release(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT status;

    if (dhcp_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual interface release service.  */
    status = _nx_dhcp_interface_release(dhcp_ptr, iface_index);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_release                          PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function releases the IP previously assigned to this DHCP      */ 
/*    interface. The DHCP state is returned back to the NOT STARTED state.*/ 
/*    A subsequent call to _nx_dhcp_interface_start will initiate a new   */ 
/*    request to obtain an IP address.                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface to release IP on    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find       Find record for the interface  */ 
/*    _nx_dhcp_send_request_internal       Send request to DHCP server    */ 
/*    _nx_dhcp_interface_reinitialize      Clear IP and DHCP network      */
/*                                           parameters to restart DHCP   */ 
/*    tx_mutex_get                         Obtain protection mutex        */ 
/*    tx_mutex_put                         Release protection mutex       */ 
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcp_interface_release(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT                     i;
UINT                     status;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        return(status);
    }

    /* Determine if DHCP is in the Boot state.  */
    if (interface_record -> nx_dhcp_state < NX_DHCP_STATE_BOUND)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Return an error code.  */
        return(NX_DHCP_NOT_BOUND);
    }

    /* Send the release DHCP request.  */
    _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPRELEASE);

    /* Reinitialize DHCP. Note: the state is changed to NX_DHCP_STATE_NOT_STARTED in this function. */
    _nx_dhcp_interface_reinitialize(dhcp_ptr, iface_index); 

    /* Determine if the application has specified a routine for DHCP state change notification.  */
    if (dhcp_ptr -> nx_dhcp_state_change_callback)
    {

        /* Yes, call the application's state change notify function with the new state.  */
        (dhcp_ptr -> nx_dhcp_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_state);
    }

    /* Determine if the application has specified a routine for DHCP interface state change notification.  */
    if (dhcp_ptr -> nx_dhcp_interface_state_change_callback)
    {

        /* Yes, call the application's state change notify function with the new state.  */
        (dhcp_ptr -> nx_dhcp_interface_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_interface_index, interface_record -> nx_dhcp_state);
    }

    /* Check if other interfaces are running DHCP.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state != NX_DHCP_STATE_NOT_STARTED))
        {

            /* Yes, other interfaces have started DHCP. We can assume Netx and ThreadX
               resources need to stay activated. */

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(NX_SUCCESS);
        }
    }
           
    /* Yes, stop DHCP Thread. */
    tx_thread_suspend(&(dhcp_ptr -> nx_dhcp_thread));

    /* Deactivate DHCP Timer.  */
    tx_timer_deactivate(&(dhcp_ptr -> nx_dhcp_timer));

    /* Unbind UDP socket.  */
    nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket));

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_start                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP start function call.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_start                        Actual DHCP start function    */ 
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
UINT  _nxe_dhcp_start(NX_DHCP *dhcp_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP start service.  */
    status =  _nx_dhcp_start(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_start                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts DHCP on all interfaces that are enabled for    */
/*    DHCP. If no interfaces have previously been started on DHCP, this   */
/*    function re/starts the DHCP CLient thread, binds the socket and     */
/*    re/starts the timer.                                                */
/*                                                                        */ 
/*    To start DHCP on a specific interface if multiple interfaces are    */
/*    enabled for DHCP, use the nx_dhcp_interface_start service.          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_DHCP_NO_INTERFACES_STARTED         Unable to start any interfaces*/
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_start              Interface specific start DHCP */
/*                                             Client service             */
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
UINT  _nx_dhcp_start(NX_DHCP *dhcp_ptr)
{

UINT    status;
UINT    interfaces_started = 0;
UINT    i;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Check all interfaces to find out which need to start the DHCP protocol.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid == NX_TRUE)
        {

            /* Start DHCP on this interface.   */
             status = _nx_dhcp_interface_start(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index);

             /* Note, not all interfaces are enabled to run, so
                status may not be successful. Skip to the next
                interface. */

             /* Check status.  */
             if ((status == NX_SUCCESS) || (status == NX_DHCP_ALREADY_STARTED))
             {
                 interfaces_started++;
             }
        }
    }

    /* Check if any interfaces started DHCP. */
    if (interfaces_started == 0) 
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Unable to start any interfaces. */
        return NX_DHCP_NO_INTERFACES_STARTED;
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_start                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the interface specific    */ 
/*    DHCP start service.                                                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to start DHCP       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_start              Actual interface start service*/ 
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
UINT  _nxe_dhcp_interface_start(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT status;

    if (dhcp_ptr == NX_NULL) 
    {
        return(NX_PTR_ERROR);
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP interface start service.  */
    status =  _nx_dhcp_interface_start(dhcp_ptr, iface_index);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_start                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function 'starts' the specified interface.  It finds the matching*/
/*  DHCP Client record for that interface and checks that the interface   */
/*  has been enabled for DHCP (see nx_dhcp_interface_enable). It then     */
/*  checks if any interfaces are running DHCP. If not it binds the DHCP   */
/*  socket port, activates the DHCP timer, and resumes the DHCP Client    */
/*  thread.                                                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to start DHCP       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find        Find the Client record for the*/
/*                                              specified  interface      */
/*    nx_udp_socket_bind                    Bind DHCP socket              */ 
/*    nx_udp_socket_unbind                  Unbind DHCP socket            */ 
/*    tx_thread_resume                      Initiate DHCP processing      */ 
/*    tx_timer_activate                     Activate DHCP timer           */
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
UINT  _nx_dhcp_interface_start(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT    i;
UINT    status;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;

    
    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        return(status);
    }

    /* Check if DHCP is already started.  */
    if (interface_record -> nx_dhcp_state != NX_DHCP_STATE_NOT_STARTED)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(NX_DHCP_ALREADY_STARTED); 
    }
                                   
    /* Check if other interface are working.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state != NX_DHCP_STATE_NOT_STARTED))
        {

            /* Yes, other interface have started DHCP.  */
            break;
        }
    }

    /* Check if any interfaces have started DHCP.  */
    if (i == NX_DHCP_CLIENT_MAX_RECORDS) 
    {

        /* Bind the UDP socket to the DHCP Client port.  */
        status =  nx_udp_socket_bind(&(dhcp_ptr -> nx_dhcp_socket), NX_DHCP_CLIENT_UDP_PORT, NX_WAIT_FOREVER);

        /* Check for error */
        if (status != NX_SUCCESS)
        {

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }

        /* Resume the DHCP processing thread.  */
        status = tx_thread_resume(&(dhcp_ptr -> nx_dhcp_thread));

        /* Determine if the resume was successful.  */
        if ((status != TX_SUCCESS) && (status != TX_SUSPEND_LIFTED))
        {

            /* Error, unbind the DHCP socket.  */
            nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket));

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }

        /* Activate DHCP Timer.  */
        status = tx_timer_activate(&(dhcp_ptr -> nx_dhcp_timer));

        /* Determine if the resume was successful.  */
        if (status != NX_SUCCESS)
        {

            /* Error, unbind the DHCP socket.  */
            nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket));

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }
    }

    /* Start DHCP service for this interface record.  */
    /* Start the DHCP protocol again by setting the state back to INIT. */
    interface_record -> nx_dhcp_state = NX_DHCP_STATE_INIT;

    /* The client begins in INIT state and forms a DHCPDISCOVER message.
       The client should wait a random time between one and ten seconds to desynchronize the use of DHCP at startup.  
       RFC2131, Section4.4.1, Page36.  */

    /* Use the minimum value, Wait one second to begain in INIT state and forms a DHCP Discovery message.  */
    interface_record -> nx_dhcp_timeout = NX_IP_PERIODIC_RATE;
    interface_record -> nx_dhcp_rtr_interval = 0;

    /* Determine if the application has specified a routine for DHCP state change notification.  */
    if (dhcp_ptr -> nx_dhcp_state_change_callback)
    {

        /* Yes, call the application's state change notify function with the new state.  */
        (dhcp_ptr -> nx_dhcp_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_state);
    }

    /* Determine if the application has specified a routine for DHCP interface state change notification.  */
    if (dhcp_ptr -> nx_dhcp_interface_state_change_callback)
    {

        /* Yes, call the application's state change notify function with the new state.  */
        (dhcp_ptr -> nx_dhcp_interface_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_interface_index, interface_record -> nx_dhcp_state);
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_enable                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the interface specific    */ 
/*    DHCP enable interface service.                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to enable DHCP      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_add               Actual interface enable service*/ 
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
UINT  _nxe_dhcp_interface_enable(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT status;

    if (dhcp_ptr == NX_NULL) 
    {
        return(NX_PTR_ERROR);
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Call actual DHCP interface enable service.  */
    status = _nx_dhcp_interface_enable(dhcp_ptr, iface_index);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_enable                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function sets the specified interface as enabled to run DHCP.    */
/*  It ensures all DHCP parameters are initialized to zero, and uses the  */
/*  physical interface MAC address to set the unique Client ID used for   */
/*  transactions with the DHCP server.                                    */
/*                                                                        */ 
/*  By default the primary interface is enabled in nx_dhcp_create.  An    */
/*  interface must be enabled before DHCP can be started on it.           */
/*  Thereafter, it is only necessary to call this function on an interface*/
/*  that has been disabled.                                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to enable DHCP      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*    NX_DHCP_NO_RECORDS_AVAILABLE          No more records available to  */
/*                                            enable DHCP on an interface */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
UINT  _nx_dhcp_interface_enable(NX_DHCP *dhcp_ptr, UINT iface_index)   
{
UINT                      i;
ULONG                     client_physical_lsw, client_physical_msw;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), NX_WAIT_FOREVER);

    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid == NX_TRUE)
        {

            /* Check if the interface is already enabled. */
            if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index == iface_index)
            {

                /* Release the DHCP mutex.  */
                 tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
                 return(NX_DHCP_INTERFACE_ALREADY_ENABLED);
            }
        }
        else
        {

            /* Yes, we found an available record.  */
            if (interface_record == NX_NULL)
                interface_record = &dhcp_ptr -> nx_dhcp_interface_record[i];
        }
    }

    /* Check if we found an valid DHCP interface record.  */
    if (interface_record == NX_NULL)
    {

        /* No, release the mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(NX_DHCP_NO_RECORDS_AVAILABLE);
    }

    /* Set this record as valid.  */
    interface_record -> nx_dhcp_record_valid = NX_TRUE;

    /* Set interface index. */  
    interface_record -> nx_dhcp_interface_index = iface_index;

    /* Initialize the client DHCP IP address with the NULL IP address.  */
    interface_record -> nx_dhcp_ip_address =  NX_BOOTP_NO_ADDRESS;

    /* Initialize the client DHCP server IP address.  */
    interface_record -> nx_dhcp_server_ip =  NX_BOOTP_NO_ADDRESS;

    /* Initialize renew and rebind timeout values to zero.  */
    interface_record -> nx_dhcp_rebind_time = 0;
    interface_record -> nx_dhcp_renewal_time =  0;

    /* Setup for infinite lease time request.  */
    interface_record -> nx_dhcp_lease_time =  NX_DHCP_INFINITE_LEASE;

    /* Get the client MAC address from the device interface. */
    client_physical_msw = dhcp_ptr ->  nx_dhcp_ip_ptr -> nx_ip_interface[iface_index].nx_interface_physical_address_msw;
    client_physical_lsw = dhcp_ptr ->  nx_dhcp_ip_ptr -> nx_ip_interface[iface_index].nx_interface_physical_address_lsw;

    /* Generate a 'unique' client transaction ID from the MAC address for each message to the server. */
    interface_record -> nx_dhcp_xid =  (ULONG)(client_physical_msw ^ client_physical_lsw ^ (ULONG)NX_RAND());

    /* Clear the timeout.  */
    interface_record -> nx_dhcp_timeout = 0;

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex)); 
    return(NX_SUCCESS); 
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_disable                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the disable service.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface to disable DHCP     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_disable            Actual disable service        */  
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
UINT _nxe_dhcp_interface_disable(NX_DHCP *dhcp_ptr, UINT iface_index)
{
UINT status;

    if (dhcp_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)  
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual delete service.  */
    status = _nx_dhcp_interface_disable(dhcp_ptr, iface_index);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_disable                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function disables the input interface to run DHCP. The DHCP    */
/*    Client need not be in any state or DHCP to be started on this       */
/*    interface to disable it.                                            */  
/*                                                                        */ 
/*    This function clears the DHCP and IP interface network parameters.  */
/*    Before DHCP can be restarted on this interface, it must be enabled  */
/*    by calling nx_dhcp_interface_enable.                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface to disable DHCP on  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find        Find Client record for input  */
/*                                            interface                   */
/*    _nx_dhcp_interface_reinitialize       Clear DHCP and IP network     */  
/*                                            parameters                  */
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
UINT _nx_dhcp_interface_disable(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT status;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Reinitalize the DHCP. This clears network values in DHCP and IP interfaces  */
    _nx_dhcp_interface_reinitialize(dhcp_ptr, iface_index);

    /* Set the record as invalid.  */
    interface_record -> nx_dhcp_record_valid = NX_FALSE;

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_state_change_notify                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP notify function call.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    dchp_state_change_notify              Application function to call  */ 
/*                                            upon DHCP state change      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_state_change_notify          Actual DHCP notify function   */ 
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
UINT  _nxe_dhcp_state_change_notify(NX_DHCP *dhcp_ptr, VOID (*dhcp_state_change_notify)(NX_DHCP *dhcp_ptr, UCHAR new_state))
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP notify service.  */
    status =  _nx_dhcp_state_change_notify(dhcp_ptr, dhcp_state_change_notify);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_state_change_notify                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up an application notification function that is  */ 
/*    called whenever a DHCP interface enters a new state.  If a NULL     */ 
/*    pointer is supplied, the notification is effectively cancelled.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    dchp_state_change_notify              Application function to call  */ 
/*                                            upon DHCP state change      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
UINT  _nx_dhcp_state_change_notify(NX_DHCP *dhcp_ptr, VOID (*dhcp_state_change_notify)(NX_DHCP *dhcp_ptr, UCHAR new_state))
{


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Setup the DHCP notify function.  */
    dhcp_ptr -> nx_dhcp_state_change_callback =  dhcp_state_change_notify;

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_state_change_notify             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the state change notify call.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    dhcp_state_interface_change_notify    Application function to call  */ 
/*                                            upon DHCP state change      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_state_change_notify Actual DHCP notify function  */ 
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
UINT  _nxe_dhcp_interface_state_change_notify(NX_DHCP *dhcp_ptr, VOID (*dhcp_state_interface_change_notify)(NX_DHCP *dhcp_ptr, UINT iface_index, UCHAR new_state))
{

UINT    status;

    /* Check for invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
    {
        return(NX_PTR_ERROR);
    }
    
    /* Call actual DHCP notify service.  */
    status =  _nx_dhcp_interface_state_change_notify(dhcp_ptr, dhcp_state_interface_change_notify);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_state_change_notify              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up an application notification function that is  */ 
/*    called whenever DHCP on the input interface enters a new state. If  */ 
/*    NULL function pointer is supplied, the notification is  cancelled.  */ 
/*    The interface on which the DHCP state changed is indicated by the   */
/*    iface_index input.                                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    dchp_state_change_notify              interface-specific callback   */ 
/*                                            for DHCP state change       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
UINT  _nx_dhcp_interface_state_change_notify(NX_DHCP *dhcp_ptr, VOID (*dhcp_interface_state_change_notify)(NX_DHCP *dhcp_ptr, UINT iface_index, UCHAR new_state))
{


    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Setup the DHCP notify function.  */
    dhcp_ptr -> nx_dhcp_interface_state_change_callback =  dhcp_interface_state_change_notify;

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_stop                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP stop function call.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_stop                         Actual DHCP stop function     */ 
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
UINT  _nxe_dhcp_stop(NX_DHCP *dhcp_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP stop service.  */
    status =  _nx_dhcp_stop(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_stop                                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function halts DHCP processing on all interfaces enabled for   */ 
/*    DHCP.  The Client state is reset to NOT STARTED, and if no          */
/*    interfaces are currently running DHCP, this function suspends the   */
/*    DHCP Client thread, and unbinds the DHCP socket port.               */
/*                                                                        */ 
/*    To stop DHCP on a specific interface when multiple interfaces are   */
/*    enabled for DHCP, use the nx_dhcp_interface_stop service.           */
/*                                                                        */ 
/*    To restart DHCP Client on a stopped interface, the interface must   */
/*    be reinitialized using either nx_dhcp_reinitialize() to reinitialize*/
/*    all enabled DHCP interfaces, or nx_dhcp_interface_reinitialize() for*/
/*    a specific interface.                                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_stop               Interface specific DHCP stop  */ 
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
UINT  _nx_dhcp_stop(NX_DHCP *dhcp_ptr)
{

UINT i;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Check all interfaces to find out which need to stop the DHCP protocol.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state != NX_DHCP_STATE_NOT_STARTED))
        {

            /* Stop DHCP.  */
            _nx_dhcp_interface_stop(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index);
       }
    }

    /* Stop DHCP Thread. */
    tx_thread_suspend(&(dhcp_ptr -> nx_dhcp_thread));

    /* Deactivate DHCP Timer.  */
    tx_timer_deactivate(&(dhcp_ptr -> nx_dhcp_timer));

    /* Ubind UDP socket.  */
    nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket));

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_stop                           PORTABLE C       */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the interface specific    */ 
/*    DHCP stop service.                                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to stop DHCP        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_stop               Actual interface stop service */ 
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
UINT  _nxe_dhcp_interface_stop(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT status;

    if (dhcp_ptr == NX_NULL) 
    {
        return NX_PTR_ERROR;
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return NX_INVALID_INTERFACE;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
        
    status =  _nx_dhcp_interface_stop(dhcp_ptr, iface_index);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_stop                             PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function stops DHCP processing on the specified interface. It  */ 
/*    sets the DHCP state to the unstarted status. If any state changes   */
/*    occurred it calls the record state change callback.                 */
/*                                                                        */ 
/*    If DHCP is not started on any of the DHCP Client interfaces, this   */
/*    function will suspend the thread, unbind the socket and stop the    */
/*    timer.                                                              */
/*                                                                        */ 
/*    Note: before DHCP Client can be restarted on the interface, the     */
/*    DHCP Client must be reinitialized by calling                        */
/*    nx_dhcp_interface_reinitialize().                                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface to stop DHCP on     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find        Find record assigned to  the  */  
/*                                            input interface             */
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcp_interface_stop(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT                     i;
UINT                     status;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Not found. Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Determine if DHCP is started.  */
    if (interface_record -> nx_dhcp_state == NX_DHCP_STATE_NOT_STARTED)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
   
        /* DHCP is not started so it can't be stopped.  */
        return(NX_DHCP_NOT_STARTED);
    }

    /* Set the state to NX_DHCP_STATE_NOT_STARTED.  */
    interface_record -> nx_dhcp_state = NX_DHCP_STATE_NOT_STARTED;

    /* Determine if the application has specified a routine for DHCP state change notification.  */
    if (dhcp_ptr -> nx_dhcp_state_change_callback)
    {

        /* Yes, call the application's state change notify function with the new state.  */
        (dhcp_ptr -> nx_dhcp_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_state);
    }

    /* Determine if the application has specified a routine for DHCP interface state change notification.  */
    if (dhcp_ptr -> nx_dhcp_interface_state_change_callback)
    {

        /* Yes, call the application's state change notify function with the new state.  */
        (dhcp_ptr -> nx_dhcp_interface_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_interface_index, interface_record->nx_dhcp_state);
    }

    /* Check if other interfaces are running DHCP.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state != NX_DHCP_STATE_NOT_STARTED))
        {

            /* Yes, other interfaces have started DHCP. We can assume Netx and ThreadX
               resources need to stay activated. */

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(NX_SUCCESS);
        }
    }

    /* Yes, stop DHCP Thread. */
    tx_thread_suspend(&(dhcp_ptr -> nx_dhcp_thread));

    /* Deactivate DHCP Timer.  */
    tx_timer_deactivate(&(dhcp_ptr -> nx_dhcp_timer));

    /* Unbind UDP socket.  */
    nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket));

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr->nx_dhcp_mutex));
    return(NX_SUCCESS);

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_user_option_retrieve                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP user option function    */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    option_request                        Option request                */ 
/*    destination_ptr                       Pointer to return buffer      */ 
/*    destination_size                      Size of return buffer (and    */ 
/*                                            modified to reflect how     */ 
/*                                            much information is in the  */ 
/*                                            response)                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_request          Actual DHCP user option       */ 
/*                                            function call               */ 
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
UINT  _nxe_dhcp_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT option_request, UCHAR *destination_ptr, UINT *destination_size)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID) ||
        (destination_ptr == NX_NULL) || (destination_size == NX_NULL))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP user option request service.  */
    status =  _nx_dhcp_user_option_retrieve(dhcp_ptr, option_request, destination_ptr, destination_size);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_retrieve                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function searches the DHCP options buffer to obtain the        */ 
/*    specified option. If successful, the option is placed in the        */ 
/*    supplied destination string.                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    option_request                        Option request                */ 
/*    destination_ptr                       Pointer to return buffer      */ 
/*    destination_size                      Size of return buffer (and    */ 
/*                                            modified to reflect how     */ 
/*                                            much information is in the  */ 
/*                                            response)                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_DHCP_NO_INTERFACES_ENABLED         No DHCP interfaces enabled    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_user_option_retrieve                             */
/*                                          Search DHCP interface options */ 
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
UINT  _nx_dhcp_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT option_request, UCHAR *destination_ptr, UINT *destination_size)
{
 
UINT    i;
UINT    status;

    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), NX_WAIT_FOREVER);

    /* Find the DHCP interface record.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state >= NX_DHCP_STATE_BOUND))
        {

            /* Retrieve the user option.  */
            status = _nx_dhcp_interface_user_option_retrieve(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index, 
                                                             option_request, destination_ptr, destination_size);
                                 
            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status); 
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_DHCP_NO_INTERFACES_ENABLED);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_user_option_request                       PORTABLE C      */ 
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP user option function    */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    option_code                           Option code                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_request          Actual DHCP user option       */ 
/*                                            request function call       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_dhcp_user_option_request(NX_DHCP *dhcp_ptr, UINT option_code)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP interface user option request service.  */
    status =  _nx_dhcp_user_option_request(dhcp_ptr, option_code);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_request                        PORTABLE C      */ 
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function requests the additional user option.                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    option_code                           Option code                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcp_user_option_request(NX_DHCP *dhcp_ptr, UINT option_code)
{
UINT i;


    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Check if the default option array already has it.  */
    for (i = 0; i < NX_DHCP_REQUEST_PARAMETER_SIZE; i++)
    {
        if (_nx_dhcp_request_parameters[i] == option_code)
        {
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(NX_DUPLICATED_ENTRY);
        }
    }

    /* Check if the user option array already has it.  */
    for (i = 0; i < dhcp_ptr -> nx_dhcp_user_request_parameter_size; i++)
    {
        if (dhcp_ptr -> nx_dhcp_user_request_parameter[i] == option_code)
        {
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(NX_DUPLICATED_ENTRY);
        }
    }

    /* Check if there is space to add option.  */
    if (dhcp_ptr -> nx_dhcp_user_request_parameter_size >= NX_DHCP_CLIENT_MAX_USER_REQUEST_PARAMETER)
    {
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(NX_NO_MORE_ENTRIES);
    }

    /* Add the option.  */
    dhcp_ptr -> nx_dhcp_user_request_parameter[dhcp_ptr -> nx_dhcp_user_request_parameter_size++] = (UCHAR)option_code;

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_user_option_retrieve            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the DHCP user option function    */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface index               */
/*    option_request                        Option request                */ 
/*    destination_ptr                       Pointer to return buffer      */ 
/*    destination_size                      Size of return buffer (and    */ 
/*                                            modified to reflect how     */ 
/*                                            much information is in the  */ 
/*                                            response)                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_request          Actual DHCP user option       */ 
/*                                            function call               */ 
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
UINT  _nxe_dhcp_interface_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT iface_index, UINT option_request, UCHAR *destination_ptr, UINT *destination_size)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_ptr -> nx_dhcp_id != NX_DHCP_ID) ||
        (destination_ptr == NX_NULL) || (destination_size == NX_NULL))
        return(NX_PTR_ERROR);
                                  
    /* Check interface index.  */
    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP interface user option request service.  */
    status =  _nx_dhcp_interface_user_option_retrieve(dhcp_ptr, iface_index, option_request, destination_ptr, destination_size);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_user_option_retrieve             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function searches the DHCP options buffer to obtain the        */ 
/*    specified option. If successful, the option is placed in the        */ 
/*    supplied destination string.                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface index               */
/*    option_request                        Option request                */ 
/*    destination_ptr                       Pointer to return buffer      */ 
/*    destination_size                      Size of return buffer (and    */ 
/*                                            modified to reflect how     */ 
/*                                            much information is in the  */ 
/*                                            response)                   */ 
/*    memcpy                                Copy specified area of memory */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    destination_size                      Size of returned option       */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_search_buffer                Search the buffer             */ 
/*    _nx_dhcp_interface_record_find        Find record of input interface*/
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcp_interface_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT iface_index, UINT option_request, UCHAR *destination_ptr, UINT *destination_size)
{

UCHAR                    *buffer_ptr;
UINT                      size;    
UINT                      status; 
UINT                      i;
ULONG                    *long_ptr;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;

             
    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }                             

    /* Check state.  */
    if (interface_record -> nx_dhcp_state < NX_DHCP_STATE_BOUND)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Return an error.  */
        return(NX_DHCP_NOT_BOUND);
    }

    /* Setup a pointer to the previous buffer.  */
    buffer_ptr =  _nx_dhcp_search_buffer(interface_record -> nx_dhcp_options_buffer, 
                                         option_request, interface_record -> nx_dhcp_options_size);

    /* Determine if the option was found.  */
    if (!buffer_ptr)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Return an error.  */
        return(NX_DHCP_PARSE_ERROR);
    }

    /* Calculate the size of the option.  */
    size =  (UINT) *buffer_ptr;    

    /* Determine if the destination is large enough.  */
    if (size > *destination_size)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Server did not respond.  */
        return(NX_DHCP_DEST_TO_SMALL);
    }

    /* Skip the option size field.  */
    buffer_ptr++;

    /* Check to see which option data is requested: */
    switch (option_request)
    {
        case NX_DHCP_OPTION_SUBNET_MASK:
        case NX_DHCP_OPTION_TIME_OFFSET:
        case NX_DHCP_OPTION_GATEWAYS:
        case NX_DHCP_OPTION_TIMESVR:
        case NX_DHCP_OPTION_DNS_SVR:
        case NX_DHCP_OPTION_NTP_SVR:
        case NX_DHCP_OPTION_DHCP_LEASE:
        case NX_DHCP_OPTION_DHCP_SERVER:
        case NX_DHCP_OPTION_RENEWAL:
        case NX_DHCP_OPTION_REBIND:
        {

            /* The length of these options must always be a multiple of 4 octets.  */
            /* Store the value as host byte order.  */
            long_ptr = (ULONG *) destination_ptr;

            /* Loop to set the long value.  */
            for (i = 0; i + 4 <= size;)
            {

                /* Set the long value.  */
                *long_ptr = _nx_dhcp_get_data(buffer_ptr + i, 4);

                /* Update the pointer.  */
                long_ptr ++;
                i += 4;
            }
            break;
        }
        default:
        {

            /* Directly copy the data into destination buffer.  */
            memcpy(destination_ptr, buffer_ptr, size); /* Use case of memcpy is verified. */
            break;
        }
    }

    /* Return the actual option size.  */
    *destination_size =  size;

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_user_option_convert                        PORTABLE C     */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the option convert service.*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    source_ptr                            Source of string area         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    value                                 Pointer to conversion value   */ 
/*    NULL                                  Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_convert         Actual option convert service  */ 
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
ULONG  _nxe_dhcp_user_option_convert(UCHAR *source_ptr)
{

ULONG   temp;

    /* Check for invalid input. */
    if (source_ptr == NX_NULL)
    {
        return NX_NULL;
    }

    /* Pickup the ULONG.  */
    temp =  _nx_dhcp_user_option_convert(source_ptr);

    /* Return the ULONG.  */
    return(temp);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_convert                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts the four bytes pointed to by the input       */ 
/*    string pointer to an ULONG and returns the value.                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    source_ptr                            Source of string area         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    value                                 Pointer to conversion value   */ 
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
ULONG  _nx_dhcp_user_option_convert(UCHAR *source_ptr)
{

ULONG   temp;


    /* Pickup the ULONG.  */
    temp =  (((ULONG) *(source_ptr))   << 24) | 
            (((ULONG) *(source_ptr+1)) << 16) | 
            (((ULONG) *(source_ptr+2)) << 8)  | 
             ((ULONG) *(source_ptr+3));

    /* Return the ULONG.  */
    return(temp);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_user_option_add_callback_set               PORTABLE C     */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the user option add        */ 
/*    callback set service.                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    dhcp_user_option_add                  Pointer to application's      */ 
/*                                            option add function         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_add_callback_set Actual user option callback   */ 
/*                                            set service                 */ 
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
UINT  _nxe_dhcp_user_option_add_callback_set(NX_DHCP *dhcp_ptr, UINT (*dhcp_user_option_add)(NX_DHCP *dhcp_ptr, UINT iface_index, UINT message_type,
                                                                                             UCHAR *user_option_ptr, UINT *user_option_length))
{

UINT    status;

    /* Check for invalid input. */
    if ((dhcp_ptr == NX_NULL) || (dhcp_user_option_add == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual DHCP user option callback set service.  */
    status =  _nx_dhcp_user_option_add_callback_set(dhcp_ptr, dhcp_user_option_add);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_user_option_add_callback_set               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the user option add callback.                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    dhcp_user_option_add                  Pointer to application's      */ 
/*                                            option add function         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
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
UINT  _nx_dhcp_user_option_add_callback_set(NX_DHCP *dhcp_ptr, UINT (*dhcp_user_option_add)(NX_DHCP *dhcp_ptr, UINT iface_index, UINT message_type,
                                                                                             UCHAR *user_option_ptr, UINT *user_option_length))
{

    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Set the callback.  */
    dhcp_ptr -> nx_dhcp_user_option_add = dhcp_user_option_add;

    /* Release the mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_udp_receive_notify                         PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is called when the IP thread task detects a UDP packet*/
/*    has been received on this socket. It allows the DHCP Client to make */
/*    a non blocking nx_udp_socket_receive call.                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Pointer to DHCP socket        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   tx_event_flags_set                    Set event packet is received   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    IP thread task                                                      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), supported*/
/*                                            multiple client instances,  */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID _nx_dhcp_udp_receive_notify(NX_UDP_SOCKET *socket_ptr)
{

NX_DHCP *dhcp_ptr;

    dhcp_ptr = (NX_DHCP *)(socket_ptr -> nx_udp_socket_reserved_ptr);

    /* Set the data received event flag.  */
    tx_event_flags_set(&(dhcp_ptr -> nx_dhcp_events), NX_DHCP_CLIENT_RECEIVE_EVENT, TX_OR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_timeout_entry                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function is called when the DHCP Client timer expires. It      */
/*    loops through all valid interfaces that have started DHCP, and      */
/*    updates their DHCP timeout.  If an interface timeout has expired, it*/
/*    processes that interface depending on which DHCP state it is in.    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   tx_event_flags_set                    Set an event that timer expired*/
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX timer                                                       */ 
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
VOID _nx_dhcp_timeout_entry(ULONG dhcp)
{

NX_DHCP     *dhcp_ptr;


    /* Setup DHCP pointer.  */
    NX_TIMER_EXTENSION_PTR_GET(dhcp_ptr, NX_DHCP, dhcp)

    /* Set the data event flag.  */
    tx_event_flags_set(&(dhcp_ptr -> nx_dhcp_events), NX_DHCP_CLIENT_TIMER_EVENT, TX_OR);
}

                 
     
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_thread_entry                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the processing thread for the DHCP Client.         */
/*    Its processing consists of executing a while-forever loop that      */ 
/*    checks for events (e.g. packet receive, timer expiration). For      */ 
/*    received packets, some initial packet validation is done before     */
/*    calling _nx_dhcp_packet_process.                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_instance                         Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_get                    Check for events              */
/*    _nx_udp_socket_receive                Retrieve packet from socket   */
/*    _nx_dhcp_interface_record_find        Find record of this interface */
/*    tx_mutex_get                          Get the DHCP mutex            */ 
/*    tx_mutex_put                          Release the DHCP mutex        */ 
/*    _nx_dhcp_packet_process               Process received packet       */ 
/*    _nx_dhcp_timout_entry                 Process timer expiration      */
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
static VOID  _nx_dhcp_thread_entry(ULONG dhcp_instance)
{

NX_DHCP                  *dhcp_ptr;
NX_PACKET                *packet_ptr;
ULONG                     events;
UINT                      status;
UINT                      iface_index;
UINT                      source_port;
ULONG                     source_ip_address;
UINT                      protocol;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;

    /* Setup the DHCP pointer.  */
    NX_THREAD_EXTENSION_PTR_GET(dhcp_ptr, NX_DHCP, dhcp_instance)

    /* Obtain the DHCP mutex before processing an.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Enter the DHCP Client task loop.  */
    do
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Wait for a DHCP client activity.  */
        tx_event_flags_get(&(dhcp_ptr -> nx_dhcp_events), NX_DHCP_CLIENT_ALL_EVENTS, 
                           TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

        /* Obtain the DHCP mutex before processing an.  */
        tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

        /* Check for DHCP data received event.  */
        if  (events & NX_DHCP_CLIENT_RECEIVE_EVENT)
        {

            /* Loop to receive DHCP message.  */
            while(1)
            {

                /* Check for an incoming DHCP packet with non blocking option. */
                status = _nx_udp_socket_receive(&dhcp_ptr -> nx_dhcp_socket, &packet_ptr, NX_NO_WAIT);

                /* Check for packet receive errors. */
                if (status != NX_SUCCESS)
                {
                    break;
                }

                /* Find the source IP address, port, interface this packet is on. */
                status = nx_udp_packet_info_extract(packet_ptr, &source_ip_address, &protocol, &source_port, &iface_index);

                /* Check status.  */
                if (status != NX_SUCCESS) 
                {

                    nx_packet_release(packet_ptr); 
                    continue;
                }

                /* Find the interface record.  */
                status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

                /* Check status.  */
                if (status != NX_SUCCESS)
                {

                    /* Release the original packet . */
                    nx_packet_release(packet_ptr);
                    continue;
                }
                
                /* Process DHCP packet.  */
                _nx_dhcp_packet_process(dhcp_ptr, interface_record, packet_ptr);
            }
        }

        /* Timer event.  */
        if (events & NX_DHCP_CLIENT_TIMER_EVENT)
        {
            _nx_dhcp_timeout_process(dhcp_ptr);
        }

#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE

        /* IP address conflict event.  */
        if (events & NX_DHCP_CLIENT_CONFLICT_EVENT)
        {

            /* Loop to check the interface.  */
            for (iface_index = 0; iface_index < NX_MAX_PHYSICAL_INTERFACES; iface_index++)
            {

                /* Check the flag.  */
                if (dhcp_ptr -> nx_dhcp_interface_conflict_flag == 0)
                {
                    break;
                }

                /* Check if IP address conflict for this interface.  */
                if (dhcp_ptr -> nx_dhcp_interface_conflict_flag & ((UINT)(1 << iface_index)))
                {

                    /* Handle notice of address conflict event. Let the server know we
                       did not get assigned a unique IP address. */
                    _nx_dhcp_interface_decline(dhcp_ptr, iface_index);

                    /* Clear the flag.  */
                    dhcp_ptr -> nx_dhcp_interface_conflict_flag &= (UINT)(~(1 << iface_index));
                }
            }
        }
#endif /* NX_DHCP_CLIENT_SEND_ARP_PROBE  */

    } while (1);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_packet_process                             PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is called when the DHCP Client thread is notified of a*/
/*    receive event. If verifies the packet is intended for this host,    */
/*    and checks the transaction ID from the Server to match its ID,      */
/*    processing the packet based on DHCP state.                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    interface_record                      Pointer to DHCP interface     */
/*    packet_ptr                            Pointer to received packet    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_find_matching_record          Find Client record matching   */
/*                                            the packet interface        */
/*   nx_packet_allocate                     Allocate new packet from DHCP */
/*                                            Client packet pool          */
/*   nx_packet_release                      Release packet back to DHCP   */
/*                                            Client packet pool          */
/*   nx_packet_data_extract_offset          Copy data to new packet buffer*/
/*   _nx_dhcp_get_option_value              Get data for input option     */
/*   _nx_dhcp_extract_information           Extract basic info from packet*/
/*   _nx_dhcp_send_request_internal         Send DHCP message             */
/*   nx_ip_interface_address_set            Set IP interface address      */
/*   nx_ip_gateway_address_set              Set IP gateway address        */
/*   _nx_dhcp_interface_reinitialize        Clear DHCP interface data     */
/*                                            for restarting DHCP         */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    nx_dhcp_thread_entry                  DHCP Client thread entry fcn  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_dhcp_packet_process(NX_DHCP *dhcp_ptr, NX_DHCP_INTERFACE_RECORD *interface_record, NX_PACKET *packet_ptr)
{

UINT        status;
NX_PACKET   *new_packet_ptr;
ULONG       dhcp_type;
UCHAR       *work_ptr;
ULONG       bytes_copied;
UINT        offset;
NX_IP       *ip_ptr;
UINT        iface_index;
ULONG       packet_client_mac_msw, packet_client_mac_lsw;
ULONG       dhcp_client_mac_msw, dhcp_client_mac_lsw;
UINT        original_state;
UCHAR       *buffer;
#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE
ULONG       probing_delay;
#endif

    /* Set the IP pointer and interface index.  */
    ip_ptr = dhcp_ptr -> nx_dhcp_ip_ptr;
    iface_index = interface_record -> nx_dhcp_interface_index;

    /* Check for valid packet length.  */
    if (packet_ptr -> nx_packet_length <= NX_BOOTP_OFFSET_OPTIONS)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        /* Return.  */
        return;
    }

    /* Copy the received packet (datagram) over to a packet from the DHCP Client pool and release
       the packet back to receive packet pool as soon as possible. */
    status =  nx_packet_allocate(dhcp_ptr -> nx_dhcp_packet_pool_ptr, &new_packet_ptr, NX_IPv4_UDP_PACKET, NX_NO_WAIT);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {

        /* Release the original packet. */
        nx_packet_release(packet_ptr);

        /* Error allocating packet, return error status.  */
        return;
    }

    /* Verify the incoming packet does not exceed our DHCP Client packet payload. */
    if ((ULONG)(new_packet_ptr -> nx_packet_data_end - new_packet_ptr -> nx_packet_prepend_ptr) < ((packet_ptr) -> nx_packet_length))
    {

        /* Release the newly allocated packet . */
        nx_packet_release(new_packet_ptr);
        
        /* Release the original packet. */
        nx_packet_release(packet_ptr);

        return;
    }

    /* Initialize the offset to the beginning of the packet buffer. */
    offset = 0;
    status = nx_packet_data_extract_offset(packet_ptr, offset, (VOID *)new_packet_ptr -> nx_packet_prepend_ptr, (packet_ptr) -> nx_packet_length, &bytes_copied);
    NX_ASSERT((status == NX_SUCCESS) && (bytes_copied > 0));

    /* Update the new packet with the bytes copied.  For chained packets, this will reflect the total
       'datagram' length. */
    new_packet_ptr -> nx_packet_length = bytes_copied; 


    /* Now we can release the original packet. */
    nx_packet_release(packet_ptr);

    /* Set the interface index and MAC address.  */
    dhcp_client_mac_msw = ip_ptr -> nx_ip_interface[iface_index].nx_interface_physical_address_msw;
    dhcp_client_mac_lsw = ip_ptr -> nx_ip_interface[iface_index].nx_interface_physical_address_lsw;

    /* Set work_ptr.  */
    work_ptr = new_packet_ptr -> nx_packet_prepend_ptr + NX_BOOTP_OFFSET_CLIENT_HW;

    /* Pickup the target MAC address in the DHCP message.  */
    packet_client_mac_msw = (((ULONG)work_ptr[0]) << 8) | ((ULONG)work_ptr[1]);
    packet_client_mac_lsw = (((ULONG)work_ptr[2]) << 24) |
                            (((ULONG)work_ptr[3]) << 16) |
                            (((ULONG)work_ptr[4]) << 8) |
                            ((ULONG)work_ptr[5]);

    /* Determine if the  MAC address matches ours.  */
    if ((packet_client_mac_msw != dhcp_client_mac_msw) || (packet_client_mac_lsw != dhcp_client_mac_lsw))
    {

        /* Release the allocated packet. */
        nx_packet_release(new_packet_ptr);
        return;
    }

    /* Check if XIDs match.  */
    if (_nx_dhcp_get_data(new_packet_ptr -> nx_packet_prepend_ptr + NX_BOOTP_OFFSET_XID, 4) != interface_record -> nx_dhcp_xid)
    {

        /* Release the original packet . */
        nx_packet_release(new_packet_ptr);

        /* Error with XID data, return error status.  */
        return;
    }

    /* Save the original state for the state change callback; after this point we will likely change it. */                          
    original_state  = interface_record -> nx_dhcp_state;

    /* The action depends on the current state of the dhcp client. */
    switch (interface_record -> nx_dhcp_state)
    {

        case NX_DHCP_STATE_SELECTING:
        {
    
            /* Set up a buffer pointer.  */
            buffer =  new_packet_ptr -> nx_packet_prepend_ptr;

            /* Get what type of DHCP message it is. */
            status = _nx_dhcp_get_option_value(buffer, NX_DHCP_OPTION_DHCP_TYPE, &dhcp_type, new_packet_ptr -> nx_packet_length);

            /* Determine if it is an Offer.  */        
            if ((status == NX_SUCCESS) && (dhcp_type == NX_DHCP_TYPE_DHCPOFFER))
            {

                /* Yes, a valid Offer is received!  */

                /* Increment the number of offers received.  */
                interface_record -> nx_dhcp_offers_received++;

                /* Update the DHCP Client interface parameters (IP address, server IP, lease, renewal and rebind times */
                if (_nx_dhcp_extract_information(dhcp_ptr, interface_record, buffer, new_packet_ptr -> nx_packet_length))
                    break;

                /* Send the DHCP Request to accept the offer.  */
                _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPREQUEST);

                /* Reset the initial timeout to NX_DHCP_MIN_RETRANS_TIMEOUT seconds  */
                interface_record -> nx_dhcp_rtr_interval = NX_DHCP_MIN_RETRANS_TIMEOUT;
                interface_record -> nx_dhcp_timeout = interface_record -> nx_dhcp_rtr_interval;

                /* This will modify the timeout by up to +/- 1 second as recommended by RFC 2131, Section 4.1, Page 24. */
                interface_record -> nx_dhcp_timeout = _nx_dhcp_add_randomize(interface_record -> nx_dhcp_timeout);

                /* Update the state to Requesting state.  */
                interface_record -> nx_dhcp_state = NX_DHCP_STATE_REQUESTING;
           }

           /* Let the timeout processing handle retransmissions. We're done here */
           break;
        }

        case NX_DHCP_STATE_REQUESTING:
        {

#ifdef NX_DHCP_ENABLE_BOOTP

            /* Initialize the value of dhcp type since we do not care for BOOTP. */
            dhcp_type = NX_DHCP_TYPE_DHCPACK;

            /* Also initialize status to success since we won't make the get option call. */               
            status = NX_SUCCESS;
#endif

            /* Setup buffer pointer.  */
            buffer = new_packet_ptr -> nx_packet_prepend_ptr;

#ifndef NX_DHCP_ENABLE_BOOTP
            /* There is a valid DHCP response, see if it is an ACK.  */
            status = _nx_dhcp_get_option_value(buffer, NX_DHCP_OPTION_DHCP_TYPE, &dhcp_type, new_packet_ptr ->nx_packet_length);
#endif
            /* Proceed to processing the server response?   */
            if (status == NX_SUCCESS)
            {

                /* Yes, check and see if it is an ACK back to our previous request.  */
                if (dhcp_type == NX_DHCP_TYPE_DHCPACK)
                {

                    /* Increment the number of ACKs received.  */
                    interface_record -> nx_dhcp_acks_received++;

                    /* Either we got an ACK or we are using BOOTP.  */

                    /* Update the parameters (IP address, server IP, lease, renewal and rebind times */
                    if (_nx_dhcp_extract_information(dhcp_ptr, interface_record, buffer, new_packet_ptr -> nx_packet_length))
                        break;

                    /* If the host is configured to send an ARP probe to verify Client address is 
                       not in use, do so now. */

#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE

                    /* Change to the Address Probing state.  */
                    interface_record -> nx_dhcp_state =  NX_DHCP_STATE_ADDRESS_PROBING;

                    /* Initalize the time for probing.  */        
                    probing_delay = (ULONG)NX_RAND() % (NX_DHCP_ARP_PROBE_WAIT);

                    /* Check the probing_delay for timer interval.  */
                    if (probing_delay)
                        interface_record -> nx_dhcp_timeout = probing_delay;
                    else
                        interface_record -> nx_dhcp_timeout = 1;

                    /* Set the probing count.  */
                    interface_record -> nx_dhcp_probe_count = NX_DHCP_ARP_PROBE_NUM;

                    /* Setup the handler to indicate the we want collision notification.  */
                    ip_ptr -> nx_ip_interface[iface_index].nx_interface_ip_conflict_notify_handler = _nx_dhcp_ip_conflict;

#else    /* NX_DHCP_CLIENT_SEND_ARP_PROBE not defined: */

                    nx_ip_interface_address_set(ip_ptr, iface_index, 
                                                interface_record -> nx_dhcp_ip_address,
                                                interface_record -> nx_dhcp_network_mask);

                    /* Check if the gateway address is valid.  */
                    if (interface_record -> nx_dhcp_gateway_address)
                    {

                        /* Set the gateway address.  */
                        nx_ip_gateway_address_set(dhcp_ptr -> nx_dhcp_ip_ptr, interface_record -> nx_dhcp_gateway_address);
                    }

                    /* No ARP probe performed. OK to change to the Bound state.  */
                    interface_record -> nx_dhcp_state =  NX_DHCP_STATE_BOUND;

#ifdef NX_DHCP_ENABLE_BOOTP
                    /* BOOTP does not use timeouts.  For the life of this DHCP Client application, keep the same IP address. */
                    interface_record -> nx_dhcp_timeout = NX_WAIT_FOREVER; 
#else
                    /* Set the renewal time received from the server.  */
                    interface_record -> nx_dhcp_timeout = interface_record -> nx_dhcp_renewal_time;
#endif /* NX_DHCP_ENABLE_BOOTP  */

#endif /* NX_DHCP_CLIENT_SEND_ARP_PROBE*/

                    break;
                }
                else if (dhcp_type == NX_DHCP_TYPE_DHCPNACK)
                {
                                                 
                    /* Increment the number of NACKs received.  */
                    interface_record -> nx_dhcp_nacks_received++;

                    /* Reinitialize DHCP.  */
                    _nx_dhcp_interface_reinitialize(dhcp_ptr, interface_record -> nx_dhcp_interface_index);

                    /* Restart DHCP service for this interface record.  */

                    /* Start the DHCP protocol again by setting the state back to INIT. */
                    interface_record -> nx_dhcp_state = NX_DHCP_STATE_INIT;

                    /* The client begins in INIT state and forms a DHCPDISCOVER message.
                       The client should wait a random time between one and ten seconds to desynchronize the use of DHCP at startup.  
                       RFC2131, Section4.4.1, Page36.  */

                    /* Use the minimum value, Wait one second to begain in INIT state and forms a DHCP Discovery message.  */
                    interface_record -> nx_dhcp_timeout = NX_IP_PERIODIC_RATE;
                    interface_record -> nx_dhcp_rtr_interval = 0;
                }
            }
            break;
        }

        case NX_DHCP_STATE_BOUND:
        {

            /* Silently discard all received packets in the BOUND state, RFC2131, Section 4.4 Figure 5  */

            break;
        }

        case NX_DHCP_STATE_RENEWING:
        {

            /* Setup the buffer pointer.  */
            buffer =  new_packet_ptr -> nx_packet_prepend_ptr;

            /* Check the server response if it accepts are renewal.  */
            status = _nx_dhcp_get_option_value(buffer, NX_DHCP_OPTION_DHCP_TYPE, &dhcp_type, new_packet_ptr ->nx_packet_length);

            /* Was the option retrieved?  */
            if (status == NX_SUCCESS)
            {

                /* Yes, Check for an ACK.  */
                if (dhcp_type == NX_DHCP_TYPE_DHCPACK)
                {

                    /* Increment the number of ACKs received.  */
                    interface_record -> nx_dhcp_acks_received++;

                    /* Update the parameters (IP address, server IP, lease, renewal and rebind times */
                    if (_nx_dhcp_extract_information(dhcp_ptr, interface_record, buffer, new_packet_ptr -> nx_packet_length))
                        break;
                    
                    /* Set the IP address and gateway address from the value extracted from the Server's DHCP response. */
                    nx_ip_interface_address_set(ip_ptr, iface_index, 
                                                interface_record -> nx_dhcp_ip_address,  
                                                interface_record -> nx_dhcp_network_mask);

                    /* Check if the gateway address is valid.  */
                    if (interface_record -> nx_dhcp_gateway_address)
                    {

                        /* Set the gateway address.  */
                        nx_ip_gateway_address_set(dhcp_ptr -> nx_dhcp_ip_ptr, interface_record -> nx_dhcp_gateway_address);
                    }

                    /* Lease has been renewed, set the countdown timer back to the renewal time and go back 
                       to the Bound state*/
                    interface_record -> nx_dhcp_timeout = interface_record -> nx_dhcp_renewal_time;

                    /* Change the state back to bound.  */
                    interface_record -> nx_dhcp_state =  NX_DHCP_STATE_BOUND;

                }
                else if (dhcp_type == NX_DHCP_TYPE_DHCPNACK)
                {

                    /* Increment the number of NACKs received.  */
                    interface_record -> nx_dhcp_nacks_received++;

                    /* Reinitialize DHCP.  */
                    _nx_dhcp_interface_reinitialize(dhcp_ptr, interface_record -> nx_dhcp_interface_index);

                    /* Restart DHCP service for this interface record.  */

                    /* Start the DHCP protocol again by setting the state back to INIT. */
                    interface_record -> nx_dhcp_state = NX_DHCP_STATE_INIT;

                    /* The client begins in INIT state and forms a DHCPDISCOVER message.
                       The client should wait a random time between one and ten seconds to desynchronize the use of DHCP at startup.  
                       RFC2131, Section4.4.1, Page36.  */

                    /* Use the minimum value, Wait one second to begain in INIT state and forms a DHCP Discovery message.  */
                    interface_record -> nx_dhcp_timeout = NX_IP_PERIODIC_RATE;
                    interface_record -> nx_dhcp_rtr_interval = 0;
                }
            }
            break;
        }

        case NX_DHCP_STATE_REBINDING:
        {

            /* Setup buffer pointer.  */
            buffer =  new_packet_ptr -> nx_packet_prepend_ptr;

            /* There is a valid DHCP response, pickup the type of response.  */
            status = _nx_dhcp_get_option_value(buffer, NX_DHCP_OPTION_DHCP_TYPE, &dhcp_type, new_packet_ptr ->nx_packet_length);

            /* Valid response?  */
            if (status == NX_SUCCESS)
            {

                /* Is it an ACK response?  */
                if (dhcp_type == NX_DHCP_TYPE_DHCPACK)
                {

                    /* Increment the number of ACKs received.  */
                    interface_record -> nx_dhcp_acks_received++;

                    /* Update the parameters (IP address, server IP, lease, renewal and rebind times */
                    if (_nx_dhcp_extract_information(dhcp_ptr, interface_record, buffer, new_packet_ptr -> nx_packet_length))
                        break;

                    /* Set the IP address and gateway address from the value extracted from the Server's DHCP response. */
                    nx_ip_interface_address_set(ip_ptr, iface_index, 
                                                interface_record -> nx_dhcp_ip_address,  
                                                interface_record -> nx_dhcp_network_mask);

                    /* Check if the gateway address is valid.  */
                    if (interface_record -> nx_dhcp_gateway_address)
                    {

                        /* Set the gateway address.  */
                        nx_ip_gateway_address_set(dhcp_ptr -> nx_dhcp_ip_ptr, interface_record -> nx_dhcp_gateway_address);
                    }

                    /* Lease has been renewed, set the countdown timer back to the renewal time and go back 
                       to the Bound state.  */
                    interface_record -> nx_dhcp_timeout = interface_record -> nx_dhcp_renewal_time;

                    /* Change to bound state.  */
                    interface_record -> nx_dhcp_state = NX_DHCP_STATE_BOUND;
                }
                else if (dhcp_type == NX_DHCP_TYPE_DHCPNACK)
                {

                    /* Increment the number of NACKs received.  */
                    interface_record -> nx_dhcp_nacks_received++;

                    /* Reinitialize DHCP.  */
                    _nx_dhcp_interface_reinitialize(dhcp_ptr, interface_record -> nx_dhcp_interface_index);

                    /* Restart DHCP service for this interface record.  */

                    /* Start the DHCP protocol again by setting the state back to INIT. */
                    interface_record -> nx_dhcp_state = NX_DHCP_STATE_INIT;

                    /* The client begins in INIT state and forms a DHCPDISCOVER message.
                       The client should wait a random time between one and ten seconds to desynchronize the use of DHCP at startup.  
                       RFC2131, Section4.4.1, Page36.  */

                    /* Use the minimum value, Wait one second to begain in INIT state and forms a DHCP Discovery message.  */
                    interface_record -> nx_dhcp_timeout = NX_IP_PERIODIC_RATE;
                    interface_record -> nx_dhcp_rtr_interval = 0;
                }
            }
            break;
        }

        default:
            break;

    } /* End of switch case */

    /* Release the packet.  */
    nx_packet_release(new_packet_ptr);

    /* Check if the state is changed.  */
    if (original_state != interface_record -> nx_dhcp_state)
    {

        /* Determine if the application has specified a routine for DHCP state change notification.  */
        if (dhcp_ptr -> nx_dhcp_state_change_callback)
        {

            /* Yes, call the application's state change notify function with the new state.  */
            (dhcp_ptr -> nx_dhcp_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_state);
        }

        /* Determine if the application has specified a routine for DHCP interface state change notification.  */
        if (dhcp_ptr -> nx_dhcp_interface_state_change_callback)
        {

            /* Yes, call the application's state change notify function with the new state.  */
            (dhcp_ptr -> nx_dhcp_interface_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_interface_index, interface_record -> nx_dhcp_state);
        }
    }

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_timeout_process                            PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is by the DHCP Client when it checks for a timer      */
/*    expiration event. It checks all the DHCP interface records who have */
/*    started DHCP and updates the time remaining on their timeout.       */
/*    If a timeout has expired, this function then processes it according */
/*    to the DHCP state it is in.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_send_request_internal         Send a request to DHCP server */
/*   _nx_dhcp_update_timeout                Reset the DHCP timeout        */
/*   _nx_dhcp_add_randomize                 Modify timeout by random value*/
/*   _nx_dhcp_update_renewal_timeout        Set the retransmission timeout*/
/*   _nx_arp_probe_send                     Send ARP probe for IP address */
/*                                            uniqueness if ARP probe     */
/*                                            enabled                     */
/*   nx_ip_interface_address_set            Set the IP interface address  */
/*   nx_ip_gateway_address_set              Set the IP gateway address    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_thread_entry                                               */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
VOID _nx_dhcp_timeout_process(NX_DHCP *dhcp_ptr)
{

UINT            i;
UINT            original_state;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;
#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE
ULONG            probing_delay;
NX_IP           *ip_ptr; 


    /* Pickup the associated IP pointer.  */
    ip_ptr = dhcp_ptr -> nx_dhcp_ip_ptr;

#endif /* NX_DHCP_CLIENT_SEND_ARP_PROBE  */

    /* Update the timeout on both interfaces. Check what needs to be done
       if a timeout expires, based on Client state. */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

         /* Check if the DHCP Client is active on this interface. */      
         if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid == NX_FALSE)
             continue;

         /* Set the interface reocrd pointer.  */
         interface_record = &dhcp_ptr -> nx_dhcp_interface_record[i];

         /* Update the count.  */
         interface_record -> nx_dhcp_seconds ++;

        /* Check the timer.  */
        if (interface_record -> nx_dhcp_timeout != 0)
        {

            /* Apply the timer interval to the current DHCP Client timeout.  */
            if (interface_record -> nx_dhcp_timeout > NX_DHCP_TIME_INTERVAL)
            {

                /* Update the timeout.  */
                interface_record -> nx_dhcp_timeout -= (ULONG)NX_DHCP_TIME_INTERVAL;
            }
            else
            {

                /* The DHCP Client timeout has expired. */
                interface_record -> nx_dhcp_timeout = 0; 

                /* Save the current state for state change callback. */
                original_state = interface_record -> nx_dhcp_state;

                /* Process according to what state the Client is in. */
                switch (interface_record -> nx_dhcp_state)
                {

                    case NX_DHCP_STATE_INIT:
                    {

                        /* Reset the seconds field for starting the DHCP address acquistiion. */
                        interface_record -> nx_dhcp_seconds = 0;

                        /* Initial state when there is no address.  Send a DHCPDISCOVER message
                           to find a DHCP server and switch to the SELECTING state.
                           Initial timeout is NX_DHCP_MIN_RETRANS_TIMEOUT seconds. */
#ifndef NX_DHCP_ENABLE_BOOTP
                        /* Only if the DHCP Client is requesting an IP address and is configured to skip the Discovery message. */
                        if ((interface_record -> nx_dhcp_ip_address != NX_BOOTP_NO_ADDRESS) &&
                            (interface_record -> nx_dhcp_skip_discovery))
                        {

                            /* Send out the DHCP request.  */
                            _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPREQUEST);

                            /* And change to the Requesting state. */
                            interface_record -> nx_dhcp_state = NX_DHCP_STATE_REQUESTING;
                        }
                        else
                        {

                            /* Send out the DHCP request.  */
                            _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPDISCOVER);

                            /* And change to the Selecting state. */
                            interface_record -> nx_dhcp_state = NX_DHCP_STATE_SELECTING;
                        }
#else
                        /* Send the BOOTP Request message.  */
                        _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_BOOT_REQUEST);

                        /* And change to the Requesting state. */
                        interface_record -> nx_dhcp_state = NX_DHCP_STATE_REQUESTING;
#endif

                        /* Check if the retransmission interval is zero.  */
                        if (interface_record -> nx_dhcp_rtr_interval == 0)
                        {

                            /* Set the interval to min retransmission timeout.  */
                            interface_record -> nx_dhcp_rtr_interval = NX_DHCP_MIN_RETRANS_TIMEOUT;
                        }
                        else
                        {

                            /* Record the retransmission interval for next retransmission.  */
                            interface_record -> nx_dhcp_rtr_interval = _nx_dhcp_update_timeout(interface_record -> nx_dhcp_rtr_interval);
                        }

                        /* Update the timeout for next retransmission.  */
                        interface_record -> nx_dhcp_timeout = interface_record -> nx_dhcp_rtr_interval; 

                        /* This will modify the timeout by up to +/- 1 second as recommended by RFC 2131, Section 4.1, Page 24. */
                        interface_record -> nx_dhcp_timeout = _nx_dhcp_add_randomize(interface_record -> nx_dhcp_timeout);

                        break;
                    }

                    case NX_DHCP_STATE_SELECTING:
                    {

#ifndef NX_DHCP_ENABLE_BOOTP
                        /* Retransmit the Discover message.  */
                        _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPDISCOVER);
#else
                        /* Retransmit the BOOTP Request message.  */ 
                        _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_BOOT_REQUEST);
#endif

                        /* Update the retransmision interval.   */
                        interface_record -> nx_dhcp_rtr_interval = _nx_dhcp_update_timeout(interface_record -> nx_dhcp_rtr_interval);

                        /* Update the timeout for next retransmission.  */
                        interface_record -> nx_dhcp_timeout = interface_record -> nx_dhcp_rtr_interval;

                        /* This will modify the timeout by up to +/- 1 second as recommended by RFC 2131, Section 4.1, Page 24. */
                        interface_record -> nx_dhcp_timeout = _nx_dhcp_add_randomize(interface_record -> nx_dhcp_timeout);

                        break;
                    }

                    case NX_DHCP_STATE_REQUESTING:
                    {

#ifndef NX_DHCP_ENABLE_BOOTP
                        /* Send a DHCP request. */
                        _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPREQUEST);
#else
                        /* Send a BOOTP request. */
                        _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_BOOT_REQUEST);
#endif
                                                
                        /* Update the retransmision interval.   */
                        interface_record->nx_dhcp_rtr_interval = _nx_dhcp_update_timeout(interface_record -> nx_dhcp_rtr_interval);

                        /* Reset the timeout for next retransmision. */
                        interface_record -> nx_dhcp_timeout = interface_record->nx_dhcp_rtr_interval;

                        /* This will modify the timeout by up to +/- 1 second as recommended by RFC 2131, Section 4.1, Page 24. */
                        interface_record -> nx_dhcp_timeout = _nx_dhcp_add_randomize(interface_record -> nx_dhcp_timeout);

                        break;
                    }

                    case NX_DHCP_STATE_ADDRESS_PROBING:
                    {

#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE

                        /* Send the ARP probe.  */
                        _nx_arp_probe_send(ip_ptr, interface_record -> nx_dhcp_interface_index, interface_record -> nx_dhcp_ip_address);

                        /* Decrease the probe count.  */
                        interface_record -> nx_dhcp_probe_count--;

                        /* Check the probe count.  */
                        if (interface_record -> nx_dhcp_probe_count)
                        {

                            /* Calculate the delay time.  */
                            probing_delay = (ULONG)NX_RAND() % (NX_DHCP_ARP_PROBE_MAX);

                            /* Determine if this is less than the minimum.  */
                            if (probing_delay < NX_DHCP_ARP_PROBE_MIN)
                            {

                                /* Set the delay to the minimum.  */
                                probing_delay = NX_DHCP_ARP_PROBE_MIN;
                            }

                            /* Check the probing_delay for timer interval.  */
                            if (probing_delay)
                                interface_record -> nx_dhcp_timeout = probing_delay;
                            else
                                interface_record -> nx_dhcp_timeout = 1;
                        }
                        else
                        {

                            /* No address conflict.  */
                            ip_ptr -> nx_ip_interface[interface_record -> nx_dhcp_interface_index].nx_interface_ip_conflict_notify_handler = NX_NULL;

                            /* Set the IP address.  */
                            nx_ip_interface_address_set(ip_ptr, interface_record -> nx_dhcp_interface_index,
                                                        interface_record -> nx_dhcp_ip_address, interface_record -> nx_dhcp_network_mask);

                            /* Check if the gateway address is valid.  */
                            if (interface_record -> nx_dhcp_gateway_address)
                            {

                                /* Set the gateway address.  */
                                nx_ip_gateway_address_set(dhcp_ptr -> nx_dhcp_ip_ptr, interface_record -> nx_dhcp_gateway_address);
                            }

                            /* Change to the Bound state.  */
                            interface_record -> nx_dhcp_state = NX_DHCP_STATE_BOUND;

 #ifdef NX_DHCP_ENABLE_BOOTP
                            /* BOOTP does not use timeouts.  For the life of this DHCP Client application, keep the same IP address. */
                            interface_record -> nx_dhcp_timeout = NX_WAIT_FOREVER; 
#else
                            /* Set the renewal time received from the server.  */
                            interface_record -> nx_dhcp_timeout = interface_record -> nx_dhcp_renewal_time;
#endif /* NX_DHCP_ENABLE_BOOTP  */
                        }

#endif /* NX_DHCP_CLIENT_SEND_ARP_PROBE */

                        break;
                    }

                    case NX_DHCP_STATE_BOUND:
                    {

                        /* Reset the seconds field for starting the DHCP request process. */
                        interface_record -> nx_dhcp_seconds = 0;

                        /* The lease has timed out. Time to renew.  */

                        /* And change to the Renewing state. */
                        interface_record -> nx_dhcp_state = NX_DHCP_STATE_RENEWING;

                        /* Send the renewal request.  */
                        _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPREQUEST);

                        /* Set the time remaining based on RFC 2131 when T1 expires. */
                        interface_record -> nx_dhcp_renewal_remain_time = interface_record -> nx_dhcp_rebind_time - interface_record -> nx_dhcp_renewal_time;
                        interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_renewal_remain_time);

                        /* Record the retransmission interval.  */
                        interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;

                        break;
                    }

                    case NX_DHCP_STATE_RENEWING:
                    {

                        /* Check if we have reached the end of the renewal time.  */
                        if (interface_record -> nx_dhcp_renewal_remain_time >= interface_record -> nx_dhcp_rtr_interval)
                        {
                            interface_record -> nx_dhcp_renewal_remain_time -= interface_record -> nx_dhcp_rtr_interval;
                        }
                        else
                        {
                            interface_record -> nx_dhcp_renewal_remain_time = 0;
                        }

                        /* Update the timeout for renew retranmission.  */
                        interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_renewal_remain_time);
                                                     
                        /* Check if we are at the limit on retransmission.  */
                        if (interface_record -> nx_dhcp_timeout == 0)
                        {

                            /* And change to the Rebinding state. */
                            interface_record -> nx_dhcp_state = NX_DHCP_STATE_REBINDING;

                            /* Send the rebind request.  */
                            _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPREQUEST);

                            /* Calculate the rebind time based on the RFC 2131. */
                            interface_record -> nx_dhcp_rebind_remain_time = interface_record -> nx_dhcp_lease_time - interface_record -> nx_dhcp_rebind_time;
                                                                                        
                            /* Calculate the timeout for the response.  */
                            interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_rebind_remain_time);

                            /* Record the retransmission interval.  */
                            interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;
                        }
                        else
                        {

                            /* Retransmit the Renewing message and wait again */
                            _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPREQUEST);

                            /* Record the retransmission interval.  */
                            interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;
                        }

                        break;
                    }

                    case NX_DHCP_STATE_REBINDING:
                    {

                        /* No response yet, the response must have timed out, 
                            update the timeout and check if we have reached the 
                            end of the rebinding time.  */
                        if (interface_record -> nx_dhcp_rebind_remain_time >= interface_record -> nx_dhcp_rtr_interval)
                        {
                            interface_record -> nx_dhcp_rebind_remain_time -= interface_record -> nx_dhcp_rtr_interval;
                        }
                        else
                        {
                            interface_record -> nx_dhcp_rebind_remain_time = 0;
                        }

                        /* Update the timeout for renew retranmission.  */
                        interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_rebind_remain_time);
                                                     
                        /* Check if we are at the limit on retransmission.  */
                        if (interface_record -> nx_dhcp_timeout == 0)
                        {

                            /* Timeout. Restart DHCP service for this interface record.  */

                            /* Reinitialize DHCP.  */
                            _nx_dhcp_interface_reinitialize(dhcp_ptr, interface_record -> nx_dhcp_interface_index);

                            /* Start the DHCP protocol again by setting the state back to INIT. */
                            interface_record -> nx_dhcp_state = NX_DHCP_STATE_INIT;

                            /* The client begins in INIT state and forms a DHCPDISCOVER message.
                               The client should wait a random time between one and ten seconds to desynchronize the use of DHCP at startup.  
                               RFC2131, Section4.4.1, Page36.  */

                            /* Use the minimum value, Wait one second to begain in INIT state and forms a DHCP Discovery message.  */
                            interface_record -> nx_dhcp_timeout = NX_IP_PERIODIC_RATE;
                            interface_record -> nx_dhcp_rtr_interval = 0;
                        }
                        else
                        {

                            /* Retransmit the Renewing message and wait again */
                            _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, NX_DHCP_TYPE_DHCPREQUEST);

                            /* Record the retransmission interval.  */
                            interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;
                        }
                        break;
                    }

                    default:
                    {

                        break;
                    }
                }

                /* Check if the state is changed.  */
                if (original_state != interface_record -> nx_dhcp_state)
                {

                    /* Determine if the application has specified a routine for DHCP state change notification.  */
                    if (dhcp_ptr -> nx_dhcp_state_change_callback)
                    {

                        /* Yes, call the application's state change notify function with the new state.  */
                        (dhcp_ptr -> nx_dhcp_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_state);
                    }

                    /* Determine if the application has specified a routine for DHCP interface state change notification.  */
                    if (dhcp_ptr -> nx_dhcp_interface_state_change_callback)
                    {

                        /* Yes, call the application's state change notify function with the new state.  */
                        (dhcp_ptr -> nx_dhcp_interface_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_interface_index, interface_record -> nx_dhcp_state);
                    }
                }

            } /* End of switch statement. */ 
        }

    } /* Try the next interface record.  */ 
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_send_request                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the DHCP client send      */
/*    request service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    dhcp_message_type                     Type of DHCP message to send , */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxe_dhcp_send_request                Actual send request service   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application thread                                                  */ 
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
UINT  _nxe_dhcp_send_request(NX_DHCP *dhcp_ptr, UINT dhcp_message_type)
{

UINT status;


    /* Check for invalid input. */
    if (!dhcp_ptr || (dhcp_message_type == 0) || (dhcp_message_type > NX_DHCP_TYPE_DHCPFORCERENEW))
    {
        return (NX_PTR_ERROR);
    }

    /* Call the actual service and return completion status. */
    status = _nx_dhcp_send_request(dhcp_ptr, dhcp_message_type);

    return(status);

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_send_request                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends the specified request to the first DHCP enabled */
/*    interface found. To send a RELEASE or DECLINE message, use the      */
/*    nx_dhcp_release/nx_dhcp_interface_release or nx_dhcp_decline/       */
/*    nx_dhcp_interface_decline respectively.                             */
/*                                                                        */
/*    To send a request on a specific DHCP interface if multiple          */
/*    interfaces are DHCP enabled, use the                                */
/*    nx_dhcp_interface_send_request service.                             */
/*                                                                        */
/*    Note: Except for an INFORM REQUEST message, the application should  */
/*    not need to send DHCP messages out independently of the DHCP Client */
/*    processing thread. It is not recommended to use this function once  */
/*    the DHCP Client has started until it is BOUND.                      */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    dhcp_message_type                     Type of DHCP message to send  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*    NX_DHCP_NO_INTERFACES_ENABLED         No DHCP interface enabled     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_send_request       Send DHCP request to server   */ 
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcp_send_request(NX_DHCP *dhcp_ptr, UINT dhcp_message_type)
{

UINT  status;
UINT  i;

    /* Obtain the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), NX_WAIT_FOREVER);

    /* Find the DHCP interface record.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid)
        {

            /* Set the request on only the first (only) valid interface.  */  
            status = _nx_dhcp_interface_send_request(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index,  dhcp_message_type);

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_DHCP_NO_INTERFACES_ENABLED);

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_send_request                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the send request service.  */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to send message on  */
/*    dhcp_message_type                     DHCP messages to send:        */ 
/*                                              NX_DHCP_TYPE_DHCPDECLINE  */ 
/*                                              NX_DHCP_TYPE_DHCPRELEASE  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_send_request       Actual send the DHCP request  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_dhcp_interface_send_request(NX_DHCP *dhcp_ptr, UINT iface_index, UINT dhcp_message_type)
{

UINT status;


    if (dhcp_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)  
    {
        return(NX_INVALID_INTERFACE);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status = _nx_dhcp_interface_send_request(dhcp_ptr, iface_index, dhcp_message_type);
    
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_send_request                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function allows the host application to send a specific request*/ 
/*    on the specified interface. The interface must be enabled for DHCP. */
/*                                                                        */
/*    To send a RELEASE or DECLINE message on a specific interface, use   */
/*    the nx_dhcp_interface_release or nx_dhcp_interface_decline          */
/*    respectively.                                                       */
/*                                                                        */
/*    Note: Except for an INFORM REQUEST message, the application should  */
/*    not need to send DHCP messages out independently of the DHCP Client */
/*    processing thread. It is not recommended to use this function once  */
/*    the DHCP Client has started until it is BOUND.                      */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to send message on  */
/*    dhcp_message_type                     Type of DHCP message to send, */ 
/*                                              NX_DHCP_TYPE_DHCPDECLINE  */ 
/*                                              NX_DHCP_TYPE_DHCPRELEASE  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_DHCP_INVALID_MESSAGE               Message type not allowed      */ 
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find        Find Client record for the    */
/*                                            specified interface         */
/*    _nx_dhcp_send_request_internal        Send the DHCP request         */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcp_interface_send_request(NX_DHCP *dhcp_ptr, UINT iface_index, UINT dhcp_message_type)
{

UINT                      status;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        return(status);
    }

    /* If the message is a RELEASE or DECLINE request, the host should call nx_dhcp_release
       or nx_dhcp_decline respectively. */
    if ((dhcp_message_type == NX_DHCP_TYPE_DHCPRELEASE) || (dhcp_message_type == NX_DHCP_TYPE_DHCPDECLINE))
    {

        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        return NX_DHCP_INVALID_MESSAGE;
    }

    /* The DHCP INFORM message is independent of the Client thread task activity. */
    if (dhcp_message_type != NX_DHCP_TYPE_DHCPINFORM)
    {

        /* Determine if DHCP is started.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[iface_index].nx_dhcp_state == NX_DHCP_STATE_NOT_STARTED)
        {

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

            /* DHCP is not started so it should not 'send' a request to the server.  */
            return(NX_DHCP_NOT_STARTED);
        }    
    }


    status = _nx_dhcp_send_request_internal(dhcp_ptr, interface_record, dhcp_message_type); 

    /* Release the DHCP mutex.  */
    tx_mutex_put(&dhcp_ptr -> nx_dhcp_mutex);

    return status;

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_send_request_internal                      PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a DHCP request to the server.  Any additional   */ 
/*    options are appended to the request structure for certain types of  */ 
/*    DHCP requests.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to send message on  */ 
/*    dhcp_message_type                     Type of DHCP message to send  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a DHCP packet        */ 
/*    nx_packet_release                     Release DHCP packet           */ 
/*    nx_udp_socket_send                    Send DHCP packet              */ 
/*    _nx_dhcp_store_data                   Write data into message       */
/*    _nx_dhcp_add_option_value             Add an option to the request  */ 
/*    _nx_dhcp_add_option_string            Add an option string to the   */ 
/*                                            request                     */ 
/*    _nx_dhcp_add_option_parameter_request Add a parameter request option*/ 
/*    nx_udp_socket_interface_send          Send packet out on interface  */
/*    _nx_dhcp_client_send_with_zero_source_address                       */
/*                                          Send broadcast packet with    */ 
/*                                            zero source IP address      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_send_request       Send request on specified     */ 
/*                                             interface                  */
/*    _nx_dhcp_interface_force_renew        Send Force Renew message      */
/*    _nx_dhcp_interface_decline            Send DECLINE message          */
/*    _nx_dhcp_interface_release            Send RELEASE message          */ 
/*    _nx_dhcp_packet_process               Process received packets      */
/*    _nx_dhcp_timeout_process              Process timer expirations     */
#ifdef NX_DHCP_CLIENT_RESTORE_STATE
/*    _nx_dhcp_resume                       Resume the DHCP Client thread */ 
#endif
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), supported*/
/*                                            adding additional request   */
/*                                            option in parameter request,*/
/*                                            resulting in version 6.1.8  */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), corrected*/
/*                                            the logic of adding server  */
/*                                            identifier option,          */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dhcp_send_request_internal(NX_DHCP *dhcp_ptr, NX_DHCP_INTERFACE_RECORD *interface_record, UINT dhcp_message_type)
{

NX_PACKET       *packet_ptr;
UCHAR           *buffer;
ULONG           targetIP;
UINT            status;
ULONG           dhcp_client_mac_msw;
ULONG           dhcp_client_mac_lsw;
UINT            iface_index;
UINT            index = 0;
UCHAR          *user_option_ptr;
UINT            user_option_length;
UINT            name_length;


    /* Set the interface idnex.  */
    iface_index = interface_record -> nx_dhcp_interface_index;

    /* Allocate a DHCP packet.  */
    status =  nx_packet_allocate(dhcp_ptr -> nx_dhcp_packet_pool_ptr, &packet_ptr, NX_IPv4_UDP_PACKET, NX_NO_WAIT);

    /* Was the packet allocation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Increment the DHCP internal error counter.  */
        interface_record -> nx_dhcp_internal_errors++;

        /* Return status.  */
        return(status);
    }

    /* Set the interface index and MAC address.  */
    dhcp_client_mac_msw = dhcp_ptr -> nx_dhcp_ip_ptr -> nx_ip_interface[iface_index].nx_interface_physical_address_msw;
    dhcp_client_mac_lsw = dhcp_ptr -> nx_dhcp_ip_ptr -> nx_ip_interface[iface_index].nx_interface_physical_address_lsw;

    /* Setup the buffer pointer.  */
    buffer =  packet_ptr -> nx_packet_prepend_ptr;

    /* Clear the buffer out... just in case.  */
    memset((void *) buffer, 0, NX_BOOTP_OFFSET_END);
    
    /* Setup the standard BootP fields.  */
    buffer[NX_BOOTP_OFFSET_OP] =        NX_BOOTP_OP_REQUEST;
    buffer[NX_BOOTP_OFFSET_HTYPE] =     NX_BOOTP_TYPE_ETHERNET;     
    buffer[NX_BOOTP_OFFSET_HLEN] =      NX_BOOTP_HLEN_ETHERNET;
    buffer[NX_BOOTP_OFFSET_HOPS] =      0;
    buffer[NX_BOOTP_OFFSET_SERVER_NM] = 0;  
    buffer[NX_BOOTP_OFFSET_BOOT_FILE] = 0;  

    /* Setup the 'Xid' field.  */
    _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_XID, 4, interface_record -> nx_dhcp_xid);

    /* Set the 'secs' field according to RFC2131, Secion4.4.1, Page37, Table5. */ 
    if ((dhcp_message_type == NX_DHCP_TYPE_DHCPDECLINE) || (dhcp_message_type == NX_DHCP_TYPE_DHCPRELEASE))
    {
        _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_SECS, 2, 0);
    }
    else
    {
        _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_SECS, 2, interface_record -> nx_dhcp_seconds);
    }

    /* Set the broadcast flag according to RFC2131, Secion4.4.1, Page38, Table5.  */

    /* Set the broadcast flag to 0 for DHCP Decline and DHCP Release.  */
    if ((dhcp_message_type == NX_DHCP_TYPE_DHCPDECLINE) || (dhcp_message_type == NX_DHCP_TYPE_DHCPRELEASE))
    {

        /* Request the response be sent unicast.  */
        _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_FLAGS, 1, NX_BOOTP_FLAGS_UNICAST);
    }

    /* Set the 'broadcast' flag according to user requirement for DHCP Discover, DHCP Request and DHCP Inform.  */
    else if (interface_record -> nx_dhcp_clear_broadcast == NX_TRUE)
    {

        /* Request the response be sent unicast.  */
        _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_FLAGS, 1, NX_BOOTP_FLAGS_UNICAST);
    }
    else
    {

        /* Request the response be sent broadcast.  */
        _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_FLAGS, 1, NX_BOOTP_FLAGS_BROADCAST);
    }

    /* RFC 2131 4.4.1: Do not set the Client IP ("ciaddr" field) address...*/
    if (dhcp_message_type != NX_DHCP_TYPE_DHCPINFORM)
    {
        _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_CLIENT_IP, 4, NX_BOOTP_NO_ADDRESS);
    }
    /* ...unless this is an INFORM REQUEST message. */
    else
    {
        _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_CLIENT_IP, 4, interface_record -> nx_dhcp_ip_address);
    }

    _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_YOUR_IP, 4, NX_BOOTP_NO_ADDRESS);   
    _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_SERVER_IP, 4, NX_BOOTP_NO_ADDRESS); 
    _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_GATEWAY_IP, 4, NX_BOOTP_NO_ADDRESS);
    _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_CLIENT_HW, 2, dhcp_client_mac_msw);
    _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_CLIENT_HW + 2, 4, dhcp_client_mac_lsw);

#ifndef NX_DHCP_ENABLE_BOOTP        
    /* Update the index.  */
    index = NX_BOOTP_OFFSET_OPTIONS; 

    /*  A BOOTP Client should not request DHCP option data. */
    _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_VENDOR, 4, NX_BOOTP_MAGIC_COOKIE);

    /* Add the actual DHCP request.  */
    _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_TYPE, NX_DHCP_OPTION_DHCP_TYPE_SIZE, dhcp_message_type, &index);
#endif

    /* Determine if any additional options need to be added relative to the DHCP message type.
       RFC 2131, Table 5: Fields and options used by DHCP Clients.  */
    switch (dhcp_message_type)
    {

#ifdef NX_DHCP_ENABLE_BOOTP

        case NX_DHCP_TYPE_BOOT_REQUEST:

            _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_CLIENT_IP, 4, interface_record -> nx_dhcp_ip_address);
            _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_SERVER_IP, 4, interface_record -> nx_dhcp_server_ip); 

            break;
#endif

        case NX_DHCP_TYPE_DHCPDISCOVER:


            /* Determine if we have a valid IP address.  */
            if ((interface_record -> nx_dhcp_ip_address != NX_BOOTP_NO_ADDRESS) && 
                (interface_record -> nx_dhcp_ip_address != NX_BOOTP_BC_ADDRESS))
            {

                /* Add a IP request option if we have a valid IP address */
                _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_IP_REQ, NX_DHCP_OPTION_DHCP_IP_REQ_SIZE, 
                                          interface_record -> nx_dhcp_ip_address, &index);
            }

            /* Add an option request for an infinite lease.  */
            _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_LEASE, NX_DHCP_OPTION_DHCP_LEASE_SIZE, NX_DHCP_INFINITE_LEASE, &index);

            /* Add the system name */
            if (dhcp_ptr -> nx_dhcp_name)
            {

                /* Check name length.  */
                if (_nx_utility_string_length_check(dhcp_ptr -> nx_dhcp_name, &name_length, 255))
                {
                    nx_packet_release(packet_ptr);
                    return(NX_DHCP_INVALID_NAME);
                }

                _nx_dhcp_add_option_string(buffer, NX_DHCP_OPTION_HOST_NAME, name_length, 
                                           (UCHAR *) dhcp_ptr -> nx_dhcp_name, &index);
            }

            /* Add parameter request option.  */
            _nx_dhcp_add_option_parameter_request(dhcp_ptr, buffer, &index);
                             
#ifdef NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION

            /* Add an option to specify the maximum length DHCP message that DHCP Client is willing to accept.  
               RFC2132, Section9.10, Page28.  */    
            _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_MAX_DHCP_MESSAGE, 2, dhcp_ptr -> nx_dhcp_max_dhcp_message_size, &index);
#endif

            /* Increment the number of Discovery messages sent.  */
            interface_record -> nx_dhcp_discoveries_sent++;
            break;
      
        case NX_DHCP_TYPE_DHCPREQUEST:

            /* Add the system name */
            if (dhcp_ptr -> nx_dhcp_name)
            {

                /* Check name length.  */
                if (_nx_utility_string_length_check(dhcp_ptr -> nx_dhcp_name, &name_length, 255))
                {
                    nx_packet_release(packet_ptr);
                    return(NX_DHCP_INVALID_NAME);
                }

                _nx_dhcp_add_option_string(buffer, NX_DHCP_OPTION_HOST_NAME, name_length, (UCHAR *) dhcp_ptr -> nx_dhcp_name, &index);
            }

            /* Determine if we have a valid IP address. Must not include if Renewing or Rebinding RCV 2131 4.3.2.  */
            if ((interface_record -> nx_dhcp_ip_address != NX_BOOTP_NO_ADDRESS) && 
                (interface_record -> nx_dhcp_ip_address != NX_BOOTP_BC_ADDRESS) &&
                (interface_record -> nx_dhcp_state != NX_DHCP_STATE_RENEWING) && 
                (interface_record -> nx_dhcp_state != NX_DHCP_STATE_REBINDING))
            {

                /* Add an IP request option if we have a valid IP address.  */
                _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_IP_REQ, NX_DHCP_OPTION_DHCP_IP_REQ_SIZE, 
                                          interface_record -> nx_dhcp_ip_address, &index);
            }

            /* Add a request for an infinite lease if we haven't already set the timers.  */
            if ((interface_record -> nx_dhcp_rebind_time == 0) || 
                (interface_record -> nx_dhcp_renewal_time == 0))
            {

                /* Add the infinite lease option.  */
                _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_LEASE, NX_DHCP_OPTION_DHCP_LEASE_SIZE, NX_DHCP_INFINITE_LEASE, &index);
            }

            /* Should add server ID if not renewing.  */
            if ((interface_record -> nx_dhcp_state != NX_DHCP_STATE_RENEWING) &&
                (interface_record -> nx_dhcp_state != NX_DHCP_STATE_REBINDING) && 
                (interface_record -> nx_dhcp_server_ip != NX_BOOTP_BC_ADDRESS) && 
                (interface_record -> nx_dhcp_server_ip != NX_BOOTP_NO_ADDRESS)
               )
            {

                /* Add Server identifier option.  */
                _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_SERVER, NX_DHCP_OPTION_DHCP_SERVER_SIZE, 
                                          interface_record -> nx_dhcp_server_ip, &index);
            }
            else if ((interface_record -> nx_dhcp_state == NX_DHCP_STATE_RENEWING) || 
                     (interface_record -> nx_dhcp_state == NX_DHCP_STATE_REBINDING))

            {

                /* Ensure the renewal message fields are correct.  */
                _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_CLIENT_IP, 4, interface_record -> nx_dhcp_ip_address);
            }

            /* Add parameter request option.  */
            _nx_dhcp_add_option_parameter_request(dhcp_ptr, buffer, &index);

#ifdef NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION

            /* Add an option to specify the maximum length DHCP message that DHCP Client is willing to accept.  
               RFC2132, Section9.10, Page28.  */    
            _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_MAX_DHCP_MESSAGE, 2, dhcp_ptr -> nx_dhcp_max_dhcp_message_size, &index);
#endif

            /* Increment the number of Request messages sent.  */
            interface_record -> nx_dhcp_requests_sent++;
            break;

        case NX_DHCP_TYPE_DHCPDECLINE:      

            /* Does the Client have a nonzero requested address it is declining? */
            if ((interface_record -> nx_dhcp_ip_address != NX_BOOTP_NO_ADDRESS) && 
                (interface_record -> nx_dhcp_ip_address != NX_BOOTP_BC_ADDRESS))
            {

                /* Yes; add Request IP address option.  */
                _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_IP_REQ, NX_DHCP_OPTION_DHCP_IP_REQ_SIZE, 
                                          interface_record -> nx_dhcp_ip_address, &index);
            }     

            /* Add Server identifier option.  */
            _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_SERVER, NX_DHCP_OPTION_DHCP_SERVER_SIZE, 
                                      interface_record -> nx_dhcp_server_ip, &index);

            break;

        case NX_DHCP_TYPE_DHCPRELEASE:       

            /* Added the 'ciaddr', Indicate the IP address being released.  */
            _nx_dhcp_store_data(buffer + NX_BOOTP_OFFSET_CLIENT_IP, 4, interface_record -> nx_dhcp_ip_address);
                                       
            /* Add Server identifier option.  */
            _nx_dhcp_add_option_value(buffer, NX_DHCP_OPTION_DHCP_SERVER, NX_DHCP_OPTION_DHCP_SERVER_SIZE, 
                                      interface_record -> nx_dhcp_server_ip, &index);

            /* Increment the number of Release messages sent.  */
            interface_record -> nx_dhcp_releases_sent++;
            break;

        case NX_DHCP_TYPE_DHCPINFORM:

            /* Add the system name */
            if (dhcp_ptr -> nx_dhcp_name)
            {

                /* Check name length.  */
                if (_nx_utility_string_length_check(dhcp_ptr -> nx_dhcp_name, &name_length, 255))
                {
                    nx_packet_release(packet_ptr);
                    return(NX_DHCP_INVALID_NAME);
                }

                _nx_dhcp_add_option_string(buffer, NX_DHCP_OPTION_HOST_NAME, name_length, (UCHAR *) dhcp_ptr -> nx_dhcp_name, &index);
            }

            /* Add parameter request option.  */
            _nx_dhcp_add_option_parameter_request(dhcp_ptr, buffer, &index);

            /* Increment the number of Inform messages sent.  */
            interface_record -> nx_dhcp_informs_sent++;

            break;

        default:
            break;
    }

    /* Add any user supplied options to the buffer.  */
    if (dhcp_ptr -> nx_dhcp_user_option_add)
    {

        /* Set the pointer for adding user option.  */
        user_option_ptr = buffer + index;

        /* Calculate the available length for user options. Minus 1 to add the END option.  */
        user_option_length = (UINT)(packet_ptr -> nx_packet_data_end - user_option_ptr - 1);

        /* Add the specific DHCP option user wanted.  */
        if (dhcp_ptr -> nx_dhcp_user_option_add(dhcp_ptr, iface_index, dhcp_message_type, user_option_ptr, &user_option_length) == NX_TRUE)
        {

            /* Update the index to include the user options.  */
            index += user_option_length;
        }
        else
        {

            /* Invalid user options. Release the packet.  */
            nx_packet_release(packet_ptr);
            return(NX_DHCP_UNKNOWN_OPTION);
        }
    }

    /* Setup the packet pointers.  */
    packet_ptr -> nx_packet_length =      NX_BOOTP_OFFSET_END;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_BOOTP_OFFSET_END;

#ifndef NX_DHCP_ENABLE_BOOTP

    /* Added the END option.  */
    *(buffer + index) = NX_DHCP_OPTION_END;
    index ++;

    /* Check the option length.  */
    if (index > NX_BOOTP_OFFSET_END)
    {
        packet_ptr -> nx_packet_length = index;
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + index;
    }
#endif

    /* Set the target address according to RFC2131, Section4.3.6, Page33, Table4 and Section4.4.4, Page40.  
       DHCP Request for renewing and DHCP Release message must be unicast.  */
    if (((dhcp_message_type == NX_DHCP_TYPE_DHCPREQUEST) && (interface_record -> nx_dhcp_state == NX_DHCP_STATE_RENEWING)) ||
        (dhcp_message_type == NX_DHCP_TYPE_DHCPRELEASE))
    {

        /* Use the current server's IP address.  */
        targetIP = interface_record -> nx_dhcp_server_ip;
    }
    else
    {

        /* Set the server target IP address to broadcast.  */
        targetIP = NX_BOOTP_BC_ADDRESS;
    }

    /* DHCP messages broadcast by a client prior to that client obtaining
       its IP address must have the source address field in the IP header
       set to 0. RFC2131, Section4.1, Page23.  */
    if ((dhcp_message_type == NX_DHCP_TYPE_DHCPDISCOVER) ||
        ((dhcp_message_type == NX_DHCP_TYPE_DHCPREQUEST) && (interface_record -> nx_dhcp_state < NX_DHCP_STATE_BOUND)))
    {

        /* Call function to send the special packet with zero source address.*/
        status = _nx_dhcp_client_send_with_zero_source_address(dhcp_ptr, iface_index, packet_ptr);
    }
    else
    {

        /* Send the packet.  */
        status = nx_udp_socket_interface_send(&(dhcp_ptr -> nx_dhcp_socket), packet_ptr, targetIP, NX_DHCP_SERVER_UDP_PORT, iface_index);
    }

    /* If an error is detected, release the packet. */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_client_send_with_zero_source_address       PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds the UDP and IP header with zero source address,*/ 
/*    then sends the packet to the appropriate link driver.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface to send message on  */ 
/*    packet_ptr                            Pointer to packet to send     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (ip_link_driver)                      User supplied link driver     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_send_request_internal        Send DHCP Request             */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported new ip filter,    */
/*                                            resulting in version 6.1.8  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            set the IP header pointer,  */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            and solve inconformity with */
/*                                            udp socket send and ip      */
/*                                            header add,                 */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dhcp_client_send_with_zero_source_address(NX_DHCP *dhcp_ptr, UINT iface_index, NX_PACKET *packet_ptr)
{

NX_IP          *ip_ptr;
NX_UDP_SOCKET  *socket_ptr;
NX_UDP_HEADER  *udp_header_ptr;
NX_IPV4_HEADER *ip_header_ptr;
NX_INTERFACE   *interface_ptr;
ULONG           ip_src_addr, ip_dest_addr;
#if defined(NX_DISABLE_UDP_TX_CHECKSUM) || defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
UINT            compute_checksum = 1;
#endif /* defined(NX_DISABLE_UDP_TX_CHECKSUM) || defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) */
ULONG           checksum;
ULONG           val;
NX_IP_DRIVER    driver_request;
      
    /* Set up the pointer to the associated IP instance.  */
    ip_ptr = dhcp_ptr -> nx_dhcp_ip_ptr;

    /* Set up the pointer to the associated socket.  */
    socket_ptr = &dhcp_ptr -> nx_dhcp_socket;

    /* Set up the pointer to the interface.  */
    interface_ptr = &(ip_ptr -> nx_ip_interface[iface_index]);
    packet_ptr -> nx_packet_ip_interface = interface_ptr;

    /* Set up the address.  */
    ip_src_addr = NX_BOOTP_NO_ADDRESS;
    ip_dest_addr = NX_BOOTP_BC_ADDRESS;

    /* Check the interface.  */
    if ((!interface_ptr -> nx_interface_valid) || (!interface_ptr -> nx_interface_link_up))
    {

        /* None found; return the error status. */
        return(NX_INVALID_INTERFACE);
    }

    /* Build UDP header.  */

    /* Prepend the UDP header to the packet.  First, make room for the UDP header.  */
    packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_UDP_HEADER);

    /* Set the correct IP version. */
    packet_ptr -> nx_packet_ip_version = NX_IP_VERSION_V4;
    
#ifndef NX_DISABLE_UDP_INFO
    /* Increment the total UDP packets sent count.  */
    ip_ptr -> nx_ip_udp_packets_sent++;

    /* Increment the total UDP bytes sent.  */
    ip_ptr -> nx_ip_udp_bytes_sent +=  packet_ptr -> nx_packet_length;

    /* Increment the total UDP packets sent count for this socket.  */
    socket_ptr -> nx_udp_socket_packets_sent++;

    /* Increment the total UDP bytes sent for this socket.  */
    socket_ptr -> nx_udp_socket_bytes_sent +=  packet_ptr -> nx_packet_length;
#endif

    /* Increase the packet length.  */
    packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length + (ULONG)sizeof(NX_UDP_HEADER);

    /* Setup the UDP header pointer.  */
    udp_header_ptr =  (NX_UDP_HEADER *) packet_ptr -> nx_packet_prepend_ptr;

    /* Build the first 32-bit word of the UDP header.  */
    udp_header_ptr -> nx_udp_header_word_0 = (((ULONG)socket_ptr -> nx_udp_socket_port ) << NX_SHIFT_BY_16) | (ULONG) NX_DHCP_SERVER_UDP_PORT;

    /* Build the second 32-bit word of the UDP header.  */
    udp_header_ptr -> nx_udp_header_word_1 =  (packet_ptr -> nx_packet_length << NX_SHIFT_BY_16);

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the UDP header.  */
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);

#ifdef NX_DISABLE_UDP_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_UDP_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM)
        compute_checksum = 0;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#if defined(NX_DISABLE_UDP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_UDP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) */
    {
        /* Yes, we need to compute the UDP checksum.  */
        checksum = _nx_ip_checksum_compute(packet_ptr,
                                           NX_PROTOCOL_UDP,
                                           (UINT)packet_ptr -> nx_packet_length,
                                           &ip_src_addr,
                                           &ip_dest_addr);

        checksum = ~checksum & NX_LOWER_16_MASK;

        /* If the computed checksum is zero, it will be transmitted as all ones. */
        /* RFC 768, page 2. */
        if (checksum == 0)
            checksum = 0xFFFF;

        NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);

        udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 | checksum;

        NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
        /* Set CHECKSUM flag so the driver would invoke the HW checksum. */
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY  */

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Build the IP header.  */

    /* Prepend the IP header to the packet.  First, make room for the IP header.  */
    packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr - 20;

    /* Increase the packet length.  */
    packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length + 20;

    /* Setup the IP header pointer.  */
    ip_header_ptr =  (NX_IPV4_HEADER *) packet_ptr -> nx_packet_prepend_ptr; 
    packet_ptr -> nx_packet_ip_header = packet_ptr -> nx_packet_prepend_ptr;
    packet_ptr -> nx_packet_ip_header_length = sizeof(NX_IPV4_HEADER);

    /* Build the first 32-bit word of the IP header.  */
    ip_header_ptr -> nx_ip_header_word_0 =  (NX_IP_VERSION | socket_ptr -> nx_udp_socket_type_of_service | (0xFFFF & packet_ptr -> nx_packet_length));

    /* Build the second 32-bit word of the IP header.  */
    ip_header_ptr -> nx_ip_header_word_1 =  (ip_ptr -> nx_ip_packet_id++ << NX_SHIFT_BY_16) | socket_ptr -> nx_udp_socket_fragment_enable;

    /* Build the third 32-bit word of the IP header.  */
    ip_header_ptr -> nx_ip_header_word_2 =  ((socket_ptr -> nx_udp_socket_time_to_live << NX_IP_TIME_TO_LIVE_SHIFT) | NX_IP_UDP);

    /* Place the source IP address in the IP header.  */
    ip_header_ptr -> nx_ip_header_source_ip =  ip_src_addr;

    /* Place the destination IP address in the IP header.  */
    ip_header_ptr -> nx_ip_header_destination_ip =  ip_dest_addr;

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the IP header.  */
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_1);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_2);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_source_ip);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);

#ifdef NX_DISABLE_IP_TX_CHECKSUM
    compute_checksum = 0;
#elif defined(NX_ENABLE_INTERFACE_CAPABILITY)
    /* Re-initialize the value back to the default initial value (i.e. 1) */
    compute_checksum = 1;
#endif /* defined(NX_DISABLE_IP_TX_CHECKSUM) */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM)
        compute_checksum = 0;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#if defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) */
    {
        checksum = _nx_ip_checksum_compute(packet_ptr, NX_IP_VERSION_V4, 20, NULL, NULL);

        val = (ULONG)(~checksum);
        val = val & NX_LOWER_16_MASK;

        /* Convert to network byte order. */
        NX_CHANGE_ULONG_ENDIAN(val);

        /* Now store the checksum in the IP header.  */
        ip_header_ptr -> nx_ip_header_word_2 =  ip_header_ptr -> nx_ip_header_word_2 | val;
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    
#ifdef NX_ENABLE_IP_PACKET_FILTER
    /* Check if the IP packet filter is set.  */
    if (ip_ptr -> nx_ip_packet_filter)
    {

        /* Yes, call the IP packet filter routine.  */
        if ((ip_ptr -> nx_ip_packet_filter((VOID *)(ip_header_ptr), NX_IP_PACKET_OUT)) != NX_SUCCESS)
        {

            /* Release mutex protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return a not successful status.  */
            return(NX_NOT_SUCCESSFUL);
        }
    }

    /* Check if the IP packet filter extended is set. */
    if (ip_ptr -> nx_ip_packet_filter_extended)
    {

        /* Yes, call the IP packet filter extended routine. */
        if (ip_ptr -> nx_ip_packet_filter_extended(ip_ptr, packet_ptr, NX_IP_PACKET_OUT) != NX_SUCCESS)
        {

            /* Release mutex protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return a not successful status.  */
            return(NX_NOT_SUCCESSFUL);
        }
    }
#endif /* NX_ENABLE_IP_PACKET_FILTER */

    /* Build the driver request.  */
    driver_request.nx_ip_driver_ptr =                   ip_ptr;
    driver_request.nx_ip_driver_packet =                packet_ptr;
    driver_request.nx_ip_driver_interface =             packet_ptr -> nx_packet_ip_interface;
    driver_request.nx_ip_driver_command =               NX_LINK_PACKET_BROADCAST;
    driver_request.nx_ip_driver_physical_address_msw =  0xFFFFUL;
    driver_request.nx_ip_driver_physical_address_lsw =  0xFFFFFFFFUL;

    /* Determine if fragmentation is needed.  */
    if (packet_ptr -> nx_packet_length > packet_ptr -> nx_packet_ip_interface -> nx_interface_ip_mtu_size)
    {

#ifndef NX_DISABLE_FRAGMENTATION
        /* Check the DF bit flag.  */
        if ((ip_ptr -> nx_ip_fragment_processing) && (socket_ptr -> nx_udp_socket_fragment_enable != NX_DONT_FRAGMENT))
        {

            /* Fragmentation is needed, call the IP fragment processing routine.  */
            (ip_ptr -> nx_ip_fragment_processing) (&driver_request);

            /* Release mutex protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return a successful status.  */
            return(NX_SUCCESS);
        }
        else
#endif /* NX_DISABLE_FRAGMENTATION */
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP send packets dropped count.  */
            ip_ptr -> nx_ip_send_packets_dropped++;
#endif

            /* Release mutex protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return a not successful status.  */
            return(NX_NOT_SUCCESSFUL);
        }
    }

#ifndef NX_DISABLE_IP_INFO

    /* Increment the IP packet sent count.  */
    ip_ptr -> nx_ip_total_packets_sent++;

    /* Increment the IP bytes sent count.  */
    ip_ptr -> nx_ip_total_bytes_sent +=  packet_ptr -> nx_packet_length - 20;
#endif

    /* Broadcast packet.  */
    (packet_ptr -> nx_packet_ip_interface -> nx_interface_link_driver_entry) (&driver_request);

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}




/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_extract_information                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts important information from the server's      */
/*    response DHCP message.                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    interface_record                      Pointer to DHCP interface     */
/*    dhcp_message                          Pointer to DHCP message       */
/*    length                                Size of DHCP message buffer   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_ip_address_set               Set the DHCP IP address       */ 
/*    nx_ip_gateway_address_set             Set the Gateway address       */ 
/*    _nx_dhcp_get_option_value             Get DHCP option from buffer   */ 
/*    _nx_dhcp_get_data                     Get data from buffer          */ 
/*    memcpy                                Copy specified area of memory */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_packet_process               Data received handler         */ 
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
static UINT  _nx_dhcp_extract_information(NX_DHCP *dhcp_ptr, NX_DHCP_INTERFACE_RECORD *interface_record, UCHAR *dhcp_message, UINT length)
{

ULONG       value;


    /* Extract the IP address.  */
    value =  _nx_dhcp_get_data(dhcp_message + NX_BOOTP_OFFSET_YOUR_IP, 4);

    /* Determine if it is valid.  */
    if ((value != NX_BOOTP_NO_ADDRESS) &&
        (((value & NX_IP_CLASS_A_MASK) == NX_IP_CLASS_A_TYPE) ||
         ((value & NX_IP_CLASS_B_MASK) == NX_IP_CLASS_B_TYPE) ||
         ((value & NX_IP_CLASS_C_MASK) == NX_IP_CLASS_C_TYPE)))
    {

        /* Store the IP address.  */
        interface_record -> nx_dhcp_ip_address =  value;
    }
    else
    {
        return(NX_DHCP_BAD_IP_ADDRESS);
    }
                       
    /* Determine if there is a subnet mask. Note a DHCP Server receiving a BOOTP request
       may send DHCP option data for subnet masks as per RFC 1534 Section 2.  */
    if (_nx_dhcp_get_option_value(dhcp_message, NX_DHCP_OPTION_SUBNET_MASK, &value, length) == NX_SUCCESS)
    {

        /* Make sure there is a valid IP address too.  */
        if (value != NX_BOOTP_NO_ADDRESS)
        {

            interface_record -> nx_dhcp_network_mask =  value;
        }
        else
        {
            ULONG ip_address;

            /* No valid network mask info supplied; use the current network mask if any. Don't
               care about current IP address for now. */
            nx_ip_interface_address_get(dhcp_ptr -> nx_dhcp_ip_ptr, 
                                        interface_record -> nx_dhcp_interface_index, 
                                        &ip_address, 
                                        &(interface_record -> nx_dhcp_network_mask));
        }
    }

#ifdef NX_DHCP_ENABLE_BOOTP

    /* Update the IP address.  */
    value =  _nx_dhcp_get_data(dhcp_message + NX_BOOTP_OFFSET_SERVER_IP, 4);

    /* Determine if it is valid.  */
    if (value != NX_BOOTP_NO_ADDRESS)
    {

        /* Store the IP address.  */
        interface_record -> nx_dhcp_server_ip =  value;
    }

    /* Update the  IP address.  */
    value =  _nx_dhcp_get_data(dhcp_message + NX_BOOTP_OFFSET_GATEWAY_IP, 4);

    /* Determine if it is valid.  */
    if ((value != NX_BOOTP_NO_ADDRESS) &&
        (((value & NX_IP_CLASS_A_MASK) == NX_IP_CLASS_A_TYPE) ||
         ((value & NX_IP_CLASS_B_MASK) == NX_IP_CLASS_B_TYPE) ||
         ((value & NX_IP_CLASS_C_MASK) == NX_IP_CLASS_C_TYPE)))
    {

        /* Store the gateway/Router IP address to the Client record.  */
        interface_record -> nx_dhcp_gateway_address = value;
    }
    else
    {

        /* The gateway may be sent as an option: See note above about BOOTP Clients
           parsing DHCP option data. */

        /* Determine if the IP gateway/router IP address is present.  */
        if (_nx_dhcp_get_option_value(dhcp_message, NX_DHCP_OPTION_GATEWAYS, &value, length) == NX_SUCCESS)
        {

            /* Determine if it is valid.  */
            if ((value != NX_BOOTP_NO_ADDRESS) &&
                (((value & NX_IP_CLASS_A_MASK) == NX_IP_CLASS_A_TYPE) ||
                 ((value & NX_IP_CLASS_B_MASK) == NX_IP_CLASS_B_TYPE) ||
                 ((value & NX_IP_CLASS_C_MASK) == NX_IP_CLASS_C_TYPE)))
            {

                /* Store the gateway/Router IP address to the Client record.  */
                interface_record -> nx_dhcp_gateway_address = value;
            }
            else
            {
                return(NX_DHCP_BAD_IP_ADDRESS);
            }
        }
    }

#else  

    /* NX_DHCP_ENABLE_BOOTP  not defined */

    /* Overwrite the server ID if there is a DHCP option for Server ID */
    if (_nx_dhcp_get_option_value(dhcp_message, NX_DHCP_OPTION_DHCP_SERVER, &value, length) == NX_SUCCESS)
    {

        /* Determine if it is valid.  */
        if ((value != NX_BOOTP_NO_ADDRESS) &&
            (((value & NX_IP_CLASS_A_MASK) == NX_IP_CLASS_A_TYPE) ||
             ((value & NX_IP_CLASS_B_MASK) == NX_IP_CLASS_B_TYPE) ||
             ((value & NX_IP_CLASS_C_MASK) == NX_IP_CLASS_C_TYPE)))
        {

            /* Store the server IP address.  */
            interface_record -> nx_dhcp_server_ip = value;
        }
        else
        {
            return(NX_DHCP_BAD_IP_ADDRESS);
        }
    }

    /* Get the lease time.  */
    if (_nx_dhcp_get_option_value(dhcp_message, NX_DHCP_OPTION_DHCP_LEASE, &value, length) == NX_SUCCESS)
    {

        /* Check for an infinite lease. */
        if (value == 0xFFFFFFFF)
        {
            /* Store the 'infinite' lease time . */
            interface_record -> nx_dhcp_lease_time = value;
            interface_record -> nx_dhcp_renewal_time = value;
            interface_record -> nx_dhcp_rebind_time = value;
        }
        else
        {            

            /* Store the lease time in timer ticks.  */
            interface_record -> nx_dhcp_lease_time =  value * (ULONG)NX_IP_PERIODIC_RATE;
    
            /* Set the renew and rebind times.  */
            interface_record -> nx_dhcp_renewal_time = interface_record -> nx_dhcp_lease_time / 2;
            interface_record -> nx_dhcp_rebind_time =  interface_record -> nx_dhcp_lease_time - (interface_record -> nx_dhcp_lease_time / 8);
        }
    }
 
    /* Overwrite the renew and rebind times with the specified values if the options are present.  */
    if ((_nx_dhcp_get_option_value(dhcp_message, NX_DHCP_OPTION_RENEWAL, &value, length) == NX_SUCCESS) && 
        (value <= interface_record -> nx_dhcp_lease_time))
    {       

        /* Check for an infinite lease. */
        if (value == 0xFFFFFFFF)
        {
            /* Set the 'infinite least time.  */
            interface_record -> nx_dhcp_renewal_time = value;
        }
        else
        {

            /* Store the renewal time in timer ticks  */
            interface_record -> nx_dhcp_renewal_time =  value * (ULONG)NX_IP_PERIODIC_RATE;
        }
    }
  
    /* Determine if there is a rebind time.  */
    if (_nx_dhcp_get_option_value(dhcp_message, NX_DHCP_OPTION_REBIND, &value, length) == NX_SUCCESS)
    {

        /* Check for an infinite lease. */
        if (value == 0xFFFFFFFF)
        {

            /* Set the 'infinite least time.  */
            interface_record -> nx_dhcp_rebind_time = value;
        }
        else
        {

            /* Convert to timer ticks. */
            value = value * (ULONG)NX_IP_PERIODIC_RATE;

            /* Sanity check*/
            if ((value <= interface_record -> nx_dhcp_lease_time) && 
                (value >= interface_record -> nx_dhcp_renewal_time))
            {
        
                /* Store the rebind time.  */
                interface_record -> nx_dhcp_rebind_time =  value;
            }
        }
    }

    /* Determine if this is an ACK from a server response, which can only happen from a handful of states.  */
    if ((interface_record -> nx_dhcp_state == NX_DHCP_STATE_REQUESTING) ||
        (interface_record -> nx_dhcp_state == NX_DHCP_STATE_RENEWING) ||
        (interface_record -> nx_dhcp_state == NX_DHCP_STATE_REBINDING))
    {

        /* Determine if the IP gateway/router IP address is present.  */
        if (_nx_dhcp_get_option_value(dhcp_message, NX_DHCP_OPTION_GATEWAYS, &value, length) == NX_SUCCESS)
        {

            /* Determine if it is valid.  */
            if ((value != NX_BOOTP_NO_ADDRESS) &&
                (((value & NX_IP_CLASS_A_MASK) == NX_IP_CLASS_A_TYPE) ||
                 ((value & NX_IP_CLASS_B_MASK) == NX_IP_CLASS_B_TYPE) ||
                 ((value & NX_IP_CLASS_C_MASK) == NX_IP_CLASS_C_TYPE)))
            {

                /* Store the gateway/Router IP address to the Client record.  */
                interface_record -> nx_dhcp_gateway_address = value;
            }
            else
            {
                return(NX_DHCP_BAD_IP_ADDRESS);
            }
        }
    }

     /* Check the DHCP options size.  */
     if ((length - NX_BOOTP_OFFSET_OPTIONS) > NX_DHCP_OPTIONS_BUFFER_SIZE)
         interface_record -> nx_dhcp_options_size = NX_DHCP_OPTIONS_BUFFER_SIZE;
     else
         interface_record -> nx_dhcp_options_size = length - NX_BOOTP_OFFSET_OPTIONS;

     /* Copy the DHCP options into DHCP Client options buffer.  */
     memcpy(interface_record -> nx_dhcp_options_buffer, /* Use case of memcpy is verified. */
            &dhcp_message[NX_BOOTP_OFFSET_OPTIONS], interface_record -> nx_dhcp_options_size);

#endif  /* NX_DHCP_ENABLE_BOOTP */

    return (NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_get_option_value                           PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function searches through a buffer containing a BootP message  */ 
/*    for a DHCP option parameter and gets the value of that option if    */ 
/*    it exists. The function is restricted to options that are less      */ 
/*    than 4 octets (bytes) in size.                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    bootp_message                         Pointer to option buffer      */ 
/*    option                                Option requested              */ 
/*    value                                 Pointer to return value var   */ 
/*    length                                Size of option buffer         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_search_buffer                Search the buffer             */ 
/*    _nx_dhcp_get_data                     Get data from buffer          */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_packet_process               Data received handler         */ 
/*    _nx_dhcp_extract_information          Extract info from server      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dhcp_get_option_value(UCHAR *bootp_message, UINT option, ULONG *value, UINT length)
{

UCHAR *data;
UCHAR *option_message;
UINT   option_length;


    /* Setup buffer pointer.  */
    option_message = &bootp_message[NX_BOOTP_OFFSET_OPTIONS];
    option_length = length - NX_BOOTP_OFFSET_OPTIONS;

    /* There is no need to check whether the option is PAD or END here since no caller will pass these 2 options
       and for denfensive purpose, the function below could check them and guarantee appropriate behaviour */

    /* Search the buffer for the option.  */
    data =  _nx_dhcp_search_buffer(option_message, option, option_length);

    /* Check to see if the option was found.  */
    if (data != NX_NULL)
    {

        /* Check for the proper size.  */
        if (*data > 4)
        {

            /* Check for the gateway option.  */
            if (option == NX_DHCP_OPTION_GATEWAYS)
            {

                /* Pickup the first gateway address.  */
                *value =  _nx_dhcp_get_data(data + 1, 4);

                /* For now, just disregard any additional gateway addresses.  */
                return(NX_SUCCESS);
            }
            else
            {

                /* Invalid size, return error.  */
                return(NX_SIZE_ERROR);
            }
        }
        else
        {

            /* Get the actual value.  */
            *value = _nx_dhcp_get_data(data + 1, *data);
            return(NX_SUCCESS);  
        }
    }

    /* Return an error if not found.  */
    return(NX_OPTION_ERROR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_add_option_value                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine adds a DHCP vendor option value to the BootP message   */ 
/*    in the supplied buffer.  Adding the option includes adding the      */ 
/*    option code, length and option data value.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    bootp_message                         Pointer to message buffer     */ 
/*    option                                Option to add                 */ 
/*    value                                 Value of Option to add        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_store_data                   Store data value              */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_send_request_internal        Send DHCP request             */ 
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
UINT  _nx_dhcp_add_option_value(UCHAR *bootp_message, UINT option, UINT size, ULONG value, UINT *index)
{


    /* Store the option.  */
    *(bootp_message + (*index)) = (UCHAR)option;
    (*index) ++;

    /* Store the option size.  */
    *(bootp_message + (*index)) = (UCHAR)size; 
    (*index) ++;

    /* Store the option value.  */
    _nx_dhcp_store_data(bootp_message + (*index), size, value);
    (*index) += size;    

    /* Return a successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_add_option_string                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine adds a DHCP option string to the BootP message in      */ 
/*    supplied buffer.  Adding the option includes adding the option      */ 
/*    code, length and option string.                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    bootp_message                         Pointer to message buffer     */ 
/*    option                                Option to add                 */ 
/*    size                                  Size of option string         */ 
/*    value                                 Option string pointer         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_move_string                  Store option string           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_send_request_internal       Internal DHCP message send     */ 
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
static UINT  _nx_dhcp_add_option_string(UCHAR *bootp_message, UINT option, UINT size, UCHAR *value, UINT *index)
{                                              
                    
    /* Store the option.  */
    *(bootp_message + (*index)) = (UCHAR)option;
    (*index) ++;

    /* Store the option size.  */
    *(bootp_message + (*index)) = (UCHAR)size; 
    (*index) ++;

    /* Store the option value.  */
    _nx_dhcp_move_string(bootp_message + (*index), value, size);
    (*index) += size;    

    /* Return a successful completion.  */
    return(NX_SUCCESS);      
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_add_option_parameter_request               PORTABLE C      */ 
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine adds a DHCP parameter request option to the BootP      */ 
/*    message in supplied buffer. Adding the option includes adding the   */ 
/*    option code, length and option value.                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    bootp_message                         Pointer to message buffer     */ 
/*    index                                 Index to write data           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_move_string                  Store option string           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_send_request_internal       Internal DHCP message send     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dhcp_add_option_parameter_request(NX_DHCP *dhcp_ptr, UCHAR *bootp_message, UINT *index)
{                                              
                    
    /* Store the option.  */
    *(bootp_message + (*index)) = NX_DHCP_OPTION_DHCP_PARAMETERS;
    (*index) ++;

    /* Store the option size.  */
    *(bootp_message + (*index)) = (UCHAR)(NX_DHCP_REQUEST_PARAMETER_SIZE + dhcp_ptr -> nx_dhcp_user_request_parameter_size); 
    (*index) ++;

    /* Store the option value.  */
    _nx_dhcp_move_string(bootp_message + (*index), _nx_dhcp_request_parameters, NX_DHCP_REQUEST_PARAMETER_SIZE);
    (*index) += (UINT)NX_DHCP_REQUEST_PARAMETER_SIZE;

    /* Check if there are additional user options.  */
    if (dhcp_ptr -> nx_dhcp_user_request_parameter_size)
    {
        _nx_dhcp_move_string(bootp_message + (*index), dhcp_ptr -> nx_dhcp_user_request_parameter, dhcp_ptr -> nx_dhcp_user_request_parameter_size);
        (*index) += (UCHAR)dhcp_ptr -> nx_dhcp_user_request_parameter_size;
    }

    /* Return a successful completion.  */
    return(NX_SUCCESS);      
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_add_randomize                              PORTABLE C      */  
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine adds randomized variance to the input timeout up to    */
/*    +/- one second.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timeout                               Timeout to randomize          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    ULONG                                 Modified timeout value        */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    nx_dhcp_process                       Process the current state of  */
/*                                           the DHCP Client state machine*/ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
static ULONG _nx_dhcp_add_randomize(ULONG timeout)
{

ULONG adjustment;

    /* Uniform random number chosen from the range -1 to +1 second as recommended by RFC2131, Section4.1, Page24. */

    /* Calculate random time adjustment in timer ticks from the range 0 to NX_IP_PERIODIC_RATE * 2.  */
    adjustment = (ULONG)NX_RAND() % ((NX_IP_PERIODIC_RATE << 1) + 1);

    /* Check for adjustment.  */
    if (adjustment < NX_IP_PERIODIC_RATE)
    {

        /* Updated timeout, minus NX_IP_PERIODIC_RATE - adjustment.  */

        /* Check for timeout.  */
        if (timeout > (NX_IP_PERIODIC_RATE - adjustment))
            timeout -= (ULONG)(NX_IP_PERIODIC_RATE - adjustment);
        else
            timeout = 1; /* Set 1 here since the minmum tick for timeout shall be larger than 0 */
    }
    else
    {

        /* Updated timeout, add adjustment- NX_IP_PERIODIC_RATE.  */
        timeout += (ULONG)(adjustment - NX_IP_PERIODIC_RATE);
    }

    return timeout;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_update_timeout                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the DHCP timeout for retransmission. When the */
/*    current timeout expires, this function doubles the timeout, but     */
/*    limits the timeout value to NX_DHCP_MAX_RETRANS_TIMEOUT (seconds).  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timeout                               The current Timeout value     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    timeout                               The updated timeout           */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_convert_delay_to_ticks       Convert the delay to ticks    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_timeout_process               Timer expiration handler     */ 
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
static ULONG _nx_dhcp_update_timeout(ULONG timeout)
{

    /* Timed out, double the timeout, limited to NX_DHCP_MAX_RETRANS_TIMEOUT */
    if ((2 * timeout) >= NX_DHCP_MAX_RETRANS_TIMEOUT)
    {

        /* Set the timeout as NX_DHCP_MAX_RETRANS_TIMEOUT.  */
        timeout = NX_DHCP_MAX_RETRANS_TIMEOUT;  
    }
    else
    {

        /* Double timeout value.  */
        timeout = timeout * 2;
    }

    /* Return the sequence timeout.  */
    return(timeout);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_update_renewal_timeout                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the DHCP timeout when trying to renew a       */ 
/*    lease according to the RFC. When the current period has timed out,  */ 
/*    this function halves the timeout, limiting the minimum timeout      */ 
/*    value to NX_DHCP_MIN_RENEW_TIMEOUT seconds.                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timeout                               The current Timeout value     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    timeout                               Renewal time remaining        */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_client_interface_update_time_remaining                     */
/*                                          Apply input time elapsed to   */
/*                                            the DHCP Client record      */
/*    _nx_dhcp_timeout_process              Timer expiration handler      */ 
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
static ULONG _nx_dhcp_update_renewal_timeout(ULONG timeout)
{

    /* check if the timeout is non zero */
    if (timeout != 0)
    {

        /* Timed out, halve the timeout, limited to NX_DHCP_MIN_RENEW_TIMEOUT or
          the remaining timeout if it is less than NX_DHCP_MIN_RENEW_TIMEOUT */
        if (timeout > NX_DHCP_MIN_RENEW_TIMEOUT)
        {

            /* Timeout can still decrease, either
               force it to the minimum or halve it */
            if (timeout > (2 * NX_DHCP_MIN_RENEW_TIMEOUT ))
            {

                /* Halve timeout.  */
                timeout = timeout / 2;
            }
            else
            {

                /* set timeout to minimum.  */
                timeout =  NX_DHCP_MIN_RENEW_TIMEOUT ;
            }
        }
    }

    /* Return the sequence timeout.  */
    return(timeout);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_search_buffer                              PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine searches through a buffer containing a BootP message   */ 
/*    for a DHCP option parameter and returns a pointer to the byte       */ 
/*    containing the size of the data section of that option if it        */ 
/*    exists. If the option cannot be found then function returns NULL.   */ 
/*    If the option has no data (PAD and END) then the pointer is not     */ 
/*    usable.                                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    option_message                        Pointer to option buffer area */ 
/*    option                                Option to search for          */ 
/*    length                                Length of search buffer       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    pointer                               Pointer to found option       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_get_option_value             Get the value of an option    */ 
/*    _nx_dhcp_get_option_data              Get the string of an option   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
static UCHAR  *_nx_dhcp_search_buffer(UCHAR *option_message, UINT option, UINT length)
{

UCHAR   *data;
UINT    i;
UINT    size;

    /* Setup buffer pointer.  */
    data = option_message;
    i = 0;

    /* Search as long as there are valid options.   */
    while (i < length - 1)
    {
        /* Jump out when it reaches END option */
        if (*data == NX_DHCP_OPTION_END)
        {
            break;
        }

        /* Simply skip any padding */
        else if (*data == NX_DHCP_OPTION_PAD)
        {

            data++;
            i++;
        }

        /* On a match, return a pointer to the size.  */
        else if (*data == option)
        {

            size = *(data + 1);

            /* Check if the option data is in the packet.  */
            if ((i + size + 1) > length)
                return(NX_NULL);

            /* Return a pointer to the option size byte.  */
            return(data + 1);
        }

        /* Otherwise skip the option by adding the size to the pointer.  */
        else
        {

            size = *(++data);

            /* skip the data plus the size byte */
            data += size + 1;
            i += size + 1;
        }
    }

    /* Return NULL to indicate the option was not found.  */
    return(NX_NULL);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_get_data                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine gets a data value from a buffer, assuming the data is  */ 
/*    stored in standard Network format (big endian). Up to 4 bytes of    */ 
/*    data are used, if there are more than 4 bytes, only the lower 4     */ 
/*    bytes are returned.                                                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    data                                  Pointer to buffer data        */ 
/*    size                                  Size of data value            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    value                                 Data value retrieved          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_get_response                 Get response from server      */ 
/*    _nx_dhcp_extract_information          Extract server information    */ 
/*    _nx_dhcp_get_option_value             Retrieve option value         */ 
/*    _nx_dhcp_update_address_list          Update address list           */ 
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
static ULONG  _nx_dhcp_get_data(UCHAR *data, UINT size)
{

ULONG   value = 0;

   
    /* Process the data retrieval request.  */
    while (size-- > 0)
    {

        /* Build return value.  */
        value = (value << 8) | *data++;
    }

    /* Return value.  */
    return(value);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_store_data                                 PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function stores a data value in a buffer in standard Network   */ 
/*    format (big endian) where the destination may be unaligned. Up to   */ 
/*    4 bytes of data are stored, if the size is larger than 4 bytes, the */ 
/*    remaining bytes are set to zero.                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    data                                  Pointer to buffer data        */ 
/*    size                                  Size of data value            */ 
/*    value                                 Value to store                */ 
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
/*    _nx_dhcp_send_request_internal        Send DHCP request             */ 
/*    _nx_dhcp_add_option_value             Add a DHCP option             */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
static VOID  _nx_dhcp_store_data(UCHAR *data, UINT size, ULONG value)
{
    /* Store the value.  */
    while (size-- > 0)
    {
        *(data + size) = (UCHAR)(value & 0xff);
        value >>= 8;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_move_string                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function stores a sequence of data bytes to a buffer.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dest                                  Pointer to destination buffer */ 
/*    source                                Pointer to source buffer      */ 
/*    size                                  Number of bytes to move       */ 
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
/*    _nx_dhcp_add_option_string            Add option string             */ 
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
static VOID  _nx_dhcp_move_string(UCHAR *dest, UCHAR *source, UINT size)
{

    /* Loop to copy all bytes.  */
    while (size-- > 0)
    {
        
        /* Copy a byte.  */
        *dest++ = *source++;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_server_address_get                         PORTABLE C     */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the DHCP server get       */
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                             Pointer to DHCP Client task    */ 
/*    server_address                       Pointer to DHCP server address */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                         Invalid pointer input          */ 
/*    status                               Actual completion status       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_server_address_get         Actual get DHCP IP server       */ 
/*                                           address service              */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT _nxe_dhcp_server_address_get(NX_DHCP *dhcp_ptr, ULONG *server_address)
{

UINT status ;


    if ((dhcp_ptr == NX_NULL) || (server_address == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status = _nx_dhcp_server_address_get(dhcp_ptr, server_address);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_address_get                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the DHCP Client's DHCP server IP address on */
/*    the first DHCP enabled interface found.                             */
/*                                                                        */ 
/*    Note that the caller should only call this service when the Client  */
/*    is bound to an IP address from a DHCP server. See the               */
/*    nx_dhcp_state_change_notify for notification of state changes.      */
/*                                                                        */ 
/*    If multiple interfaces are enabled for DHCP, use                    */
/*    nx_dhcp_interface_server_address_get() to get the server IP address */
/*    on a specified interface.                                           */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Client task   */ 
/*    server_address                        Pointer to DHCP server address*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*    NX_DHCP_NO_INTERFACES_ENABLED         No interfaces enabled for DHCP*/
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_server_address_get Interface specific server IP  */
/*                                            address get service         */ 
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT _nx_dhcp_server_address_get(NX_DHCP *dhcp_ptr, ULONG *server_address)
{

UINT i;
UINT status;

    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), NX_WAIT_FOREVER);

    /* Find the DHCP interface record.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check the interface record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state >= NX_DHCP_STATE_BOUND))
        {

            /* Get the server address of first valid record.  */
            status = _nx_dhcp_interface_server_address_get(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index, server_address);

            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

            return(status);
        }
    }

    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    return(NX_DHCP_NO_INTERFACES_ENABLED);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_interface_server_address_get              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the DHCP server get       */
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                             Pointer to DHCP Client task    */ 
/*    iface_index                          Interface of the Server        */
/*    server_address                       Pointer to DHCP server address */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_server_address_get Actual get server IP address  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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

UINT _nxe_dhcp_interface_server_address_get(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG *server_address)
{

UINT status;

    if ((dhcp_ptr == NX_NULL) || (server_address == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES) 
    {
        return NX_INVALID_INTERFACE;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual reinitialize service.  */
    status = _nx_dhcp_interface_server_address_get(dhcp_ptr, iface_index, server_address);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_server_address_get               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the DHCP Client's DHCP server IP address on */
/*    the specified interface.                                            */
/*                                                                        */ 
/*    Note that the caller should only call this service when the Client  */
/*    is in the bound state. See nx_dhcp_state_change_notify for          */
/*    notification when the Client state changes.                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                             Pointer to DHCP Client task    */ 
/*    iface_index                          Interface of the DHCP server   */
/*    server_address                       Pointer to DHCP server address */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                               Actual completion status       */ 
/*    NX_DHCP_NOT_BOUND                    Client state not Bound         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_dhcp_interface_record_find        Find record for the interface */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT _nx_dhcp_interface_server_address_get(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG *server_address)
{

UINT    status; 
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;

    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        return(status);
    }

    /* Check the record state.  */
    if (interface_record -> nx_dhcp_state >= NX_DHCP_STATE_BOUND)
    {

        /* Set the server IP address from the DHCP Client instance. */
        *server_address = interface_record -> nx_dhcp_server_ip;
        status = NX_SUCCESS;
    }
    else
        status = NX_DHCP_NOT_BOUND;

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    return status;
}


#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_ip_conflict                                PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function notifies the DHCP instance that a conflict was        */ 
/*    detected by the NetX ARP receive packet handling routine.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_ptr                                IP instance pointer           */ 
/*    iface_index                           IP Interface Index            */
/*    ip_address                            IP Address to bind to         */ 
/*    physical_msw                          Physical address MSW          */ 
/*    physical_lsw                          Physical address LSW          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set event flags               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX                                                                */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), supported*/
/*                                            multiple client instances,  */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_dhcp_ip_conflict(NX_IP *ip_ptr, UINT iface_index, ULONG ip_address, ULONG physical_msw, ULONG physical_lsw)
{

TX_INTERRUPT_SAVE_AREA

NX_DHCP *dhcp_ptr;

    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(physical_msw);
    NX_PARAMETER_NOT_USED(physical_lsw);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Find the DHCP client.  */
    for (dhcp_ptr = _nx_dhcp_created_ptr; dhcp_ptr; dhcp_ptr = dhcp_ptr -> nx_dhcp_created_next)
    {
        if (dhcp_ptr -> nx_dhcp_ip_ptr == ip_ptr)
        {

            /* Set the interface index.  */
            dhcp_ptr -> nx_dhcp_interface_conflict_flag |= (UINT)(1 << iface_index);

            /* Set the address conflict event flag.  */
            tx_event_flags_set(&(_nx_dhcp_created_ptr -> nx_dhcp_events), NX_DHCP_CLIENT_CONFLICT_EVENT, TX_OR);

            break;
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE

}
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_set_interface_index                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function does error checking for the set interface index call. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    interface_index                       Interface the DHCP Client task*/
/*                                                is associated with      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_set_interface_index           Actual set interface index   */ 
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
UINT _nxe_dhcp_set_interface_index(NX_DHCP *dhcp_ptr, UINT interface_index)
{

UINT status;


    /* Check for invalid pointer input. */
    if (dhcp_ptr == NX_NULL)
    {

        return(NX_PTR_ERROR);
    }
    /* Check for invalid non pointer input. */
    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {

        return(NX_INVALID_INTERFACE);
    }

    /* Call the actual set DHCP Client interface service */
    status = _nx_dhcp_set_interface_index(dhcp_ptr,  interface_index);

    /* Return completion status. */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_set_interface_index                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables DHCP on the specified interface. This function*/
/*    should be called before the DHCP Client is started. The intended use*/
/*    of this function is for DHCP to run on only one interface. To enable*/
/*    multiple interfaces for DHCP, use the nx_dhcp_interface_enable      */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface to enable DHCP on   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful Completion status  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_interface_enable              Enable DHCP on interface      */ 
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
UINT _nx_dhcp_set_interface_index(NX_DHCP *dhcp_ptr, UINT iface_index)
{

UINT    i;
UINT    status;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), NX_WAIT_FOREVER);

    /* Invalidate all the interfaces enabled for DHCP. */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Invalidate this interface.  */
        dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid = NX_FALSE;
        
        /* Change the state.  */
        dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state = NX_DHCP_STATE_NOT_STARTED; 
    }

    /* Enable DHCP on this interface.  */
    status = _nx_dhcp_interface_enable(dhcp_ptr, iface_index);

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_interface_record_find                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    Find the Client record that is assigned to the input interface      */ 
/*    index, and return a pointer to that record. The interface index must*/
/*    match the interface index of the Client record, and the interface   */
/*    must be enabled for DHCP.  If no matching records are found, an     */
/*    error status is returned, and the interface record pointer is NULL. */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    interface_index                       Interface to find             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion         */
/*    NX_DHCP_INTERFACE_NOT_ENABLED         No matching record found      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*       None                                                             */ 
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
static UINT _nx_dhcp_interface_record_find(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_INTERFACE_RECORD **interface_record)
{

UINT i;

    /* Find which DHCP Client interface record is assigned the input interface. */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++) 
    {

        /* Check if this record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid == NX_FALSE) 
            continue;

        /* Check if the interface index matches.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index == iface_index)
        {

            /* Yes, we found the record.  */
            *interface_record = &dhcp_ptr -> nx_dhcp_interface_record[i];

            /* Return.  */
            return (NX_SUCCESS);
        }
    }

    /* No matching record found.  */
    return (NX_DHCP_INTERFACE_NOT_ENABLED);
}


#ifdef NX_DHCP_CLIENT_RESTORE_STATE   
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_client_get_record                          PORTABLE C     */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get DHCP Client record */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    client_record_ptr                     Pointer to memory to save     */
/*                                             Client record to           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    status                                Completion status from        */
/*                                            internal DHCP calls         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_create_record                                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_dhcp_client_get_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *client_record_ptr)
{

UINT status;

    if ((dhcp_ptr == NX_NULL) || (client_record_ptr == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    status = _nx_dhcp_client_get_record(dhcp_ptr, client_record_ptr);

    return status; 
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_client_get_record                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a DHCP Client record for restoring the Client */
/*    state between power cycles or idle (sleep) mode.  It copies the     */
/*    first DHCP enabled Client record found to the supplied client record*/
/*    pointer. If DHCP is enabled on multiple interfaces, use             */
/*    nx_dhcp_client_interface_record_get to get a record of a specific   */
/*    interface.                                                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    client_record_ptr                     Pointer to Client record      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_DHCP_NO_INTERFACES_ENABLED         No DHCP interface enabled     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_interface_get_record   Get record for specified index*/ 
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcp_client_get_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *client_record_ptr)
{

UINT i;
UINT status;


    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the DHCP interface record.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check if the interface record is valid.  */
        if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid) &&
            (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state >= NX_DHCP_STATE_BOUND))
        {

            /* Find a record of the current state of the DHCP CLient. */
            status = _nx_dhcp_client_interface_get_record(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index, client_record_ptr);

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_DHCP_NO_INTERFACES_ENABLED);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_client_interface_get_record               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get DHCP Client        */
/*    interface record service.                                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface index               */
/*    client_record_ptr                     Pointer to memory to save     */
/*                                             Client record to           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_interface_create_record                              */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_dhcp_client_interface_get_record(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_CLIENT_RECORD *client_record_ptr)
{

UINT status;

    /* Check for invalid input pointers.  */
    if ((dhcp_ptr == NX_NULL) || (client_record_ptr == NX_NULL))
    {
        return NX_PTR_ERROR;
    }  

    /* Check interface index.  */
    if (iface_index >= NX_DHCP_CLIENT_MAX_RECORDS) 
    {
        return NX_INVALID_INTERFACE;
    }

    status = _nx_dhcp_client_interface_get_record(dhcp_ptr, iface_index, client_record_ptr);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_client_interface_get_record                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a DHCP Client record for restoring the Client */
/*    state between power cycles or idle (sleep) mode.  It then copies the*/
/*    Client record to the supplied client record pointer.                */
/*                                                                        */ 
/*    The DHCP Client state should be restored from a Client record saved */
/*    to memory (client_record_ptr).                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */
/*    iface_index                           Interface index               */
/*    client_record_ptr                     Pointer to memory to save     */
/*                                             Client record to           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_DHCP_NOT_BOUND                     Client not Bound to address   */
/*    status                                Actual completion status      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_interface_record_find         Find interface record matching*/
/*                                             the input interface        */ 
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcp_client_interface_get_record(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_CLIENT_RECORD *client_record_ptr)
{

UINT                      status;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Check the state.  */
    if (interface_record -> nx_dhcp_state < NX_DHCP_STATE_BOUND)
    {

        /* The DHCP Client is not bound to an IP address. Cannot create a record for restoring Client 
           state if the Client not bound to an IP address. */

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return NX_DHCP_NOT_BOUND;
    }

    /* Clear memory before filling with data. */
    memset(client_record_ptr, 0, sizeof(NX_DHCP_CLIENT_RECORD));

    /* Set the record.  */
    client_record_ptr -> nx_dhcp_state = interface_record -> nx_dhcp_state;
    client_record_ptr -> nx_dhcp_ip_address = interface_record -> nx_dhcp_ip_address;           /* Server assigned IP Address                               */ 
    client_record_ptr -> nx_dhcp_network_mask = interface_record -> nx_dhcp_network_mask;       /* Server assigned network mask                             */  
    client_record_ptr -> nx_dhcp_gateway_address = interface_record -> nx_dhcp_gateway_address;
    client_record_ptr -> nx_dhcp_server_ip = interface_record -> nx_dhcp_server_ip;
    client_record_ptr -> nx_dhcp_timeout = interface_record -> nx_dhcp_timeout;
    client_record_ptr -> nx_dhcp_lease_time = interface_record -> nx_dhcp_lease_time ;
    client_record_ptr -> nx_dhcp_renewal_time = interface_record -> nx_dhcp_renewal_time;
    client_record_ptr -> nx_dhcp_rebind_time = interface_record -> nx_dhcp_rebind_time;
    client_record_ptr -> nx_dhcp_renewal_remain_time = interface_record -> nx_dhcp_renewal_remain_time;
    client_record_ptr -> nx_dhcp_rebind_remain_time = interface_record -> nx_dhcp_rebind_remain_time;   
    client_record_ptr -> nx_dhcp_interface_index = interface_record -> nx_dhcp_interface_index;

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_client_restore_record                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the client restore service.*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    client_record_ptr                     Pointer to previously saved   */
/*                                             Client record              */
/*    time_elapsed                          Time elapsed in timer ticks   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERRPR                          Invalid pointer input         */
/*    status                                NetX completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_restore_record         Actual restore record service */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_dhcp_client_restore_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed)
{

UINT status;


    if ((dhcp_ptr == NX_NULL) || (client_record_ptr == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    /* Note: zero time elapsed is an acceptable value. */

    status = _nx_dhcp_client_restore_record(dhcp_ptr, client_record_ptr, time_elapsed);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_client_restore_record                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the DHCP Client state of the first DHCP       */
/*    enabled interface found with the supplied DHCP Client record        */
/*    pointed to by the client_record_ptr pointer. It then updates the    */
/*    time out parameters of the DHCP Client with the time_elapsed        */
/*    input (in timer ticks).                                             */
/*                                                                        */ 
/*    If DHCP is enabled on multiple interfaces, use                      */
/*    nx_dhcp_client_interface_restore_record to restore a record to a    */
/*    specific interface.                                                 */
/*                                                                        */ 
/*    This function is intended for restoring Client state between reboots*/
/*    and assumes the DHCP Client state was previously obtained and stored*/
/*    in non volatile memory before power down.                           */
/*                                                                        */ 
/*    Note: his should not be called in addition to _nx_dhcp_client_update*/
/*    _time_remaining.  _nx_dhcp_client_restore_ calls that function,     */
/*    so calling t_nx_dhcp_client_update_time_remaining separately would  */
/*    effectively apply the time elapsed a second time.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    client_record_ptr                     Pointer to previously saved   */
/*                                             Client record              */
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_DHCP_NO_INTERFACES_ENABLED         No interfaces enabled for DHCP*/
/*    status                                NetX completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_interface_restore_record                             */
/*                                          Interface specific restore    */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcp_client_restore_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed)
{

UINT    i;
UINT    status;


    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the DHCP interface record.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid)
        {
                              
            /* Create a record of the current state of the DHCP Client. */
            status = _nx_dhcp_client_interface_restore_record(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index, 
                                                              client_record_ptr, time_elapsed);

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }
    }
    
    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_DHCP_NO_INTERFACES_ENABLED);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_client_interface_restore_record           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the client restore service.*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    iface_index                           Interface index               */
/*    client_record_ptr                     Pointer to previously saved   */
/*                                             Client record              */
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERRPR                          Invalid pointer input         */
/*    status                                NetX completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_interface_restore_record                             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_dhcp_client_interface_restore_record(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((dhcp_ptr == NX_NULL) || (client_record_ptr == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    /* Check interface index.  */
    if (iface_index >= NX_DHCP_CLIENT_MAX_RECORDS) 
    {
        return NX_INVALID_INTERFACE;
    }

    status = _nx_dhcp_client_interface_restore_record(dhcp_ptr, iface_index, client_record_ptr, time_elapsed);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_client_interface_restore_record            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the DHCP Client state with the input DHCP     */
/*    Client record for the specified interface. It copies data from the  */
/*    Client record to the interface racord, and updates the time out     */
/*    parameters of the DHCP Client with the time_elapsed inpur (in timer */
/*    ticks).                                                             */
/*                                                                        */ 
/*    This function is intended for restoring Client state between reboots*/
/*    and assumes the DHCP Client record was previously obtained and      */
/*    stored in non volatile memory before power down.                    */
/*                                                                        */ 
/*    Note: this should not be called in addition to                      */
/*    nx_dhcp_client_update_time_remaining.                               */
/*    nx_dhcp_client_interface_restore_record calls that function, so     */
/*    calling nx_dhcp_client_update_time_remaining would apply the time   */
/*    elapsed a second time.                                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    client_record_ptr                     Pointer to previously saved   */
/*                                             Client record              */
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                NetX completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*   _nx_dhcp_interface_record_find         Find client record for        */
/*                                            specified interface         */
/*    nx_ip_interface_address_set           Set IP interface address      */
/*   _nx_dhcp_client_update_time_remaining  Update time remaining for time*/ 
/*                                            elapsed while powered down  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            restored the gateway        */
/*                                            address,                    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcp_client_interface_restore_record(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed)
{

UINT    status;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;

    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    interface_record -> nx_dhcp_state = client_record_ptr -> nx_dhcp_state;
    interface_record -> nx_dhcp_gateway_address = client_record_ptr -> nx_dhcp_gateway_address;
    interface_record -> nx_dhcp_server_ip = client_record_ptr -> nx_dhcp_server_ip;
    interface_record -> nx_dhcp_timeout = client_record_ptr -> nx_dhcp_timeout;
    interface_record -> nx_dhcp_lease_time = client_record_ptr -> nx_dhcp_lease_time ;
    interface_record -> nx_dhcp_renewal_time = client_record_ptr -> nx_dhcp_renewal_time;
    interface_record -> nx_dhcp_rebind_time = client_record_ptr -> nx_dhcp_rebind_time;
    interface_record -> nx_dhcp_renewal_remain_time = client_record_ptr -> nx_dhcp_renewal_remain_time;
    interface_record -> nx_dhcp_rebind_remain_time = client_record_ptr -> nx_dhcp_rebind_remain_time;

    /* Restore directly from the client record. */
    interface_record -> nx_dhcp_ip_address = client_record_ptr -> nx_dhcp_ip_address;
    interface_record -> nx_dhcp_network_mask = client_record_ptr -> nx_dhcp_network_mask; 
    interface_record -> nx_dhcp_interface_index = iface_index;

    /* Update the IP instance with saved network parameters.*/
    status = nx_ip_interface_address_set(dhcp_ptr -> nx_dhcp_ip_ptr, iface_index, interface_record -> nx_dhcp_ip_address, interface_record -> nx_dhcp_network_mask);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return status;
    }

    /* Check if the gateway address is valid.  */
    if (interface_record -> nx_dhcp_gateway_address)
    {

        /* Set the gateway address.  */
        status = nx_ip_gateway_address_set(dhcp_ptr -> nx_dhcp_ip_ptr, interface_record -> nx_dhcp_gateway_address);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }
    }

    /* Now apply the time elapsed to update the DHCP Client time remaining on its lease and current DHCP state. */
    status = _nx_dhcp_client_interface_update_time_remaining(dhcp_ptr, interface_record-> nx_dhcp_interface_index, time_elapsed);

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr->nx_dhcp_mutex));

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_client_update_time_remaining              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the update time remaining  */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual Completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_update_time_remaining  Update DHCP timeout for time  */
/*                                            elapsed while powered down  */
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
UINT  _nxe_dhcp_client_update_time_remaining(NX_DHCP *dhcp_ptr, ULONG time_elapsed)
{

UINT status;


    if (dhcp_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    status = _nx_dhcp_client_update_time_remaining(dhcp_ptr, time_elapsed);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_client_update_time_remaining               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function applies the input time_elapsed to the DHCP Client     */
/*    time remaining and determines if a state change occurred meanwhile. */
/*    Note that time_elapsed is in timer ticks.                           */
/*                                                                        */ 
/*    There is no need to call this function if also calling              */
/*    _nx_dhcp_client_restore_record which applies the elapsed time.      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*    NX_DHCP_NO_INTERFACES_ENABLED         No interface enabled for DHCP */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_interface_update_time_remaining                      */
/*                                          Apply the elapsed time to the */
/*                                            specified DHCP timeout      */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_client_restore_record                                      */ 
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
UINT  _nx_dhcp_client_update_time_remaining(NX_DHCP *dhcp_ptr, ULONG time_elapsed)
{

UINT i;
UINT status;

                                        
    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the DHCP interface record.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid)
        {

            /* Create a record of the current state of the DHCP CLient. */
            status = _nx_dhcp_client_interface_update_time_remaining(dhcp_ptr, dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index, time_elapsed);

            /* Release the DHCP mutex.  */
            tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
            return(status);
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
    return(NX_DHCP_NO_INTERFACES_ENABLED);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_client_interface_update_time_remaining    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the update time remaining  */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_INTERFACE                  Invalid interface index       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_interface_update_time_remaining                      */
/*                                          Actual update time service    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_dhcp_client_interface_update_time_remaining(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG time_elapsed)
{

UINT status;


    /* Check for invalid input pointers.  */
    if (dhcp_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    /* Check interface index.  */
    if (iface_index >= NX_DHCP_CLIENT_MAX_RECORDS) 
    {
        return NX_INVALID_INTERFACE;
    }

    status = _nx_dhcp_client_interface_update_time_remaining(dhcp_ptr, iface_index, time_elapsed);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_client_interface_update_time_remaining     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function applies the input time_elapsed data to the DHCP Client*/
/*    state and determines what state it should be in (e.g. BOUND,        */
/*    IP address. Note, time_elapsed is in timer ticks. If called from the*/
/*    application it is assumed the application device is sleeping, not   */
/*    powering up.  If the device is waking up from sleeping, it does not */
/*    need to restore the DHCP client state but only to update the time   */
/*    elapsed.                                                            */
/*                                                                        */ 
/*    Therefore this should not be called in addition to                  */
/*    _nx_dhcp_client_restore_record.  The restore record service handles */
/*    updating time remaining, such that calling this service would       */
/*    effectively apply the time elapsed a second time.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful Completion status  */ 
/*    status                                Actual completion status      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */ 
/*    tx_mutex_put                          Release protection mutex      */ 
/*   _nx_dhcp_update_renewal_timeout        Update time remaining on lease*/
/*   _nx_dhcp_interface_record_find         Find record for the interface */
/*   _nx_dhcp_interface_reinitialize        Clear DHCP and IP network     */
/*                                            parameters                  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcp_client_interface_update_time_remaining(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG time_elapsed)
{

UINT                      status;
UINT                      accrued_time_adjusted = 0;
UINT                      time_accrued;
UINT                      original_state;
NX_DHCP_INTERFACE_RECORD *interface_record = NX_NULL;


    /* Obtain DHCP Client protection mutex. */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Find the interface record.  */
    status = _nx_dhcp_interface_record_find(dhcp_ptr, iface_index, &interface_record);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Get the original state.  */
    original_state = interface_record -> nx_dhcp_state;

    /* Compute the time since the lease was assigned before we suspended the Client.  */
    if (interface_record -> nx_dhcp_state == NX_DHCP_STATE_BOUND) 
    {
        if (interface_record -> nx_dhcp_renewal_time >= interface_record -> nx_dhcp_timeout)
        {
            /* Since the Client is in a BOUND state, we know the timeout is less than T2 
               or nx_dhcp_renew_time. */
            time_accrued = interface_record -> nx_dhcp_renewal_time - interface_record -> nx_dhcp_timeout;
        }
        else
        {
            /* Invalid DHCP timeout. If it is in the BOUND state, set to the renewal time. */
            time_accrued = interface_record -> nx_dhcp_renewal_time;
        }
    }
    else if (interface_record -> nx_dhcp_state == NX_DHCP_STATE_RENEWING)
    {

        /* Since the Client is in the RENEWING state, the total time on the lease is the
           lease rebind time - the time out in the RENEW state: */
        time_accrued = interface_record -> nx_dhcp_rebind_time - interface_record -> nx_dhcp_renewal_remain_time;

        /* Note if dhcp_ptr -> nx_dhcp_rebind_time < dhcp_ptr -> nx_dhcp_renewal_remain_time,
           the CLient either received invalid DHCP parameters or there is an internal error.
           The renewal time should never exceed the rebind time.  */
    }
    else if (interface_record -> nx_dhcp_state == NX_DHCP_STATE_REBINDING)
    {

        /* Since the Client is in the REBINDING state, the total time on the lease is the
           lease  time - the  time remaining in the REBIND state: */
        time_accrued = interface_record -> nx_dhcp_lease_time - interface_record -> nx_dhcp_rebind_remain_time;

        /* Note if dhcp_ptr -> nx_dhcp_lease_time < dhcp_ptr -> nx_dhcp_rebind_remain_time,
           the CLient either received invalid DHCP parameters or there is an internal error.
           The rebind time should never exceed the lease time.  */
    }
    else
    {
        /* Expired! */
        accrued_time_adjusted = 0xFFFFFFFF;
    }

    /* Adjust the time accrued to include the interval while the Client was suspended. */
    if (accrued_time_adjusted != 0xFFFFFFFF)
    {
        accrued_time_adjusted = time_accrued + time_elapsed;
    }

    /* Determine if the DHCP Client needs to renew. */
    if (accrued_time_adjusted < interface_record -> nx_dhcp_renewal_time)
    {

        /* Not yet.  Update the nx_dhcp_timeout for the time elapsed and we're done. */
        interface_record -> nx_dhcp_timeout -= time_elapsed;
                                                    
        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* We are done. Ok to resume the Client. */
        return NX_SUCCESS;
    }

    /* Determine if the Client should renew. */
    else if (accrued_time_adjusted < interface_record -> nx_dhcp_rebind_time)
    {

        /* Yes; it has not reached the expiration on the renew period.  */

        /* Check if it is already in the RENEW state. */
        if (interface_record -> nx_dhcp_state == NX_DHCP_STATE_RENEWING)
        {

            /* Yes it is, so do nothing. Let the Client continue renewing. */

            /* Update the time remaining for the Client to renew its lease.   */
            interface_record -> nx_dhcp_renewal_remain_time -= time_elapsed;

            /* Reset the timeout based on the updated time remaining to renew. */
            interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_renewal_remain_time); 

            /* Record the retransmission interval.  */
            interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;
        }
        else
        {
        
            /* No it wasn't so update the Client for the RENEW state. */
    
            /* Compute how many seconds into the renew period the Client is (total time on lease over and above
               the time to start renewing. */
            if (accrued_time_adjusted == interface_record -> nx_dhcp_renewal_time)
            {

                interface_record -> nx_dhcp_renewal_remain_time = interface_record -> nx_dhcp_rebind_time - interface_record -> nx_dhcp_renewal_time;
            }
            else
            {
            
                interface_record -> nx_dhcp_renewal_remain_time = interface_record -> nx_dhcp_rebind_time - accrued_time_adjusted;
            }

            /* Set the DHCP timeout for being in the RENEW phase. */
            interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_renewal_remain_time);

            /* Record the retransmission interval.  */
            interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;

            /* And change to the Renewing state. */
            interface_record -> nx_dhcp_state = NX_DHCP_STATE_RENEWING;
        }

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* We are done. Ok to resume the Client. */
        return NX_SUCCESS;
    }

    /* Determine if it is time for the Client to rebind (e.g. check the lease has not completely expired). */
    else if (accrued_time_adjusted < interface_record -> nx_dhcp_lease_time)
    {

        /* Check if it is already in the REBIND state. */
        if (interface_record -> nx_dhcp_state == NX_DHCP_STATE_REBINDING)
        {
            /* Yes it is, so do nothing. Let the Client complete its rebind task*/

            /* Update the time remaining for the Client to rebind an IP address.   */
            interface_record -> nx_dhcp_rebind_remain_time -= time_elapsed;

            /* Reset the timeout based on the updated time remaining to rebind. */
            interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_rebind_remain_time);
 
            /* Record the retransmission interval.  */
            interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;
        }
        else
        {
        
            /* No it wasn't so update the Client for the REBIND state. */
    
            /* Compute how many seconds into the rebind period the Client is (total time on lease over and above
               the time to start rebinding. */
            if (accrued_time_adjusted == interface_record -> nx_dhcp_rebind_time)
            {

                interface_record -> nx_dhcp_rebind_remain_time = interface_record -> nx_dhcp_lease_time - interface_record -> nx_dhcp_rebind_time;
            }
            else
            {
            
                interface_record -> nx_dhcp_rebind_remain_time = interface_record -> nx_dhcp_lease_time - accrued_time_adjusted;
            }

            /* Set the DHCP timeout for being in the RENEW phase. */
            interface_record -> nx_dhcp_timeout = _nx_dhcp_update_renewal_timeout(interface_record -> nx_dhcp_rebind_remain_time); 

            /* Record the retransmission interval.  */
            interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;

            /* Record the retransmission interval.  */
            interface_record -> nx_dhcp_rtr_interval = interface_record -> nx_dhcp_timeout;

            /* And change to the Rebinding state. */
            interface_record -> nx_dhcp_state = NX_DHCP_STATE_REBINDING;
        }

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* We are done. Ok to resume the Client. */
        return NX_SUCCESS;
    }
    else
    {

        /* Clear the existing DHCP Client network parameters for restarting in the INIT state. */
        _nx_dhcp_interface_reinitialize(dhcp_ptr, iface_index); 

        /* Start the DHCP protocol again by setting the state back to INIT. */
        interface_record -> nx_dhcp_state = NX_DHCP_STATE_INIT;

        /* The client begins in INIT state and forms a DHCPDISCOVER message.
           The client should wait a random time between one and ten seconds to desynchronize the use of DHCP at startup.  
           RFC2131, Section4.4.1, Page36.  */

        /* Use the minimum value, Wait one second to begain in INIT state and forms a DHCP Discovery message.  */
        interface_record -> nx_dhcp_timeout = NX_IP_PERIODIC_RATE;
        interface_record -> nx_dhcp_rtr_interval = 0;
    }
                                 
    /* Check if the state is changed.  */
    if (original_state != interface_record -> nx_dhcp_state)
    {

        /* Determine if the application has specified a routine for DHCP state change notification.  */
        if (dhcp_ptr -> nx_dhcp_state_change_callback)
        {

            /* Yes, call the application's state change notify function with the new state.  */
            (dhcp_ptr -> nx_dhcp_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_state);
        }

        /* Determine if the application has specified a routine for DHCP interface state change notification.  */
        if (dhcp_ptr -> nx_dhcp_interface_state_change_callback)
        {

            /* Yes, call the application's state change notify function with the new state.  */
            (dhcp_ptr -> nx_dhcp_interface_state_change_callback)(dhcp_ptr, interface_record -> nx_dhcp_interface_index, interface_record -> nx_dhcp_state);
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr->nx_dhcp_mutex));

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_resume                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the Client resume service.*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */ 
/*    status                                Actual completion  status     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_resume                        Actual Client resume service  */ 
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
UINT  _nxe_dhcp_resume(NX_DHCP *dhcp_ptr)
{

UINT status;


    if (dhcp_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    status = _nx_dhcp_resume(dhcp_ptr);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_resume                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resumes the DHCP Client thread and timer.  It then    */
/*    checks the state on all DHCP enabled interfaces if renewing or      */
/*    rebinding.  If so it sends a REQUEST to the DHCP server.            */
/*                                                                        */ 
/*    The DHCP Client application can then call the                       */
/*    nx_dhcp_client_udpate_remaining_time() service to update the time   */
/*    remaining on the client lease, before calling nx_dhcp_resume.       */
/*                                                                        */ 
/*    This function does not change the Client state, lease timeout or    */
/*    time remaining on the current lease.  It requires the               */
/*    NX_DHCP_CLIENT_RESTORE_STATE option to be enabled.                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful Completion status  */ 
/*    NX_DHCP_ALREADY_STARTED               DHCP Client thread started    */ 
/*    status                                NetX or ThreadX completion    */
/*                                             status                     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_send_request_internal         Send message to DHCP server   */
/*   nx_udp_socket_bind                     Bind UDP socket to port       */
/*   tx_thread_resume                       Resume the DHCP Client thread */
/*   tx_mutex_get                           Obtain mutex protection       */ 
/*   tx_mutex_put                           Release mutex protectiob      */
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
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcp_resume(NX_DHCP *dhcp_ptr)
{

UINT    i;
UINT    status;
NX_IP   *ip_ptr;
ULONG   client_physical_msw;
ULONG   client_physical_lsw;
UINT    iface_index;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Resume the DHCP processing thread.  */
    status = tx_thread_resume(&(dhcp_ptr -> nx_dhcp_thread));

    /* Determine if the resume was successful.  */
    if ((status != TX_SUCCESS) && (status != TX_SUSPEND_LIFTED))
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Active the timer.  */
    status = tx_timer_activate(&(dhcp_ptr -> nx_dhcp_timer));

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Bind the UDP socket to the DHCP Client port.  */
    status =  nx_udp_socket_bind(&(dhcp_ptr -> nx_dhcp_socket), NX_DHCP_CLIENT_UDP_PORT, NX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));
        return(status);
    }

    /* Set IP pointer.  */
    ip_ptr = dhcp_ptr -> nx_dhcp_ip_ptr;

    /* Loop to process interface records.  */
    for (i = 0; i < NX_DHCP_CLIENT_MAX_RECORDS; i++)
    {

        /* Check which interface record is valid.  */
        if (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_record_valid)
        {

            /* Get the interface index.  */
            iface_index = dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_interface_index;

            /* Get the client MAC address from the device interface. */
            client_physical_msw = ip_ptr -> nx_ip_interface[iface_index].nx_interface_physical_address_msw;
            client_physical_lsw = ip_ptr -> nx_ip_interface[iface_index].nx_interface_physical_address_lsw;

            /* Generate a 'unique' client transaction ID from the MAC address for each message to the server. */
            dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_xid = client_physical_msw ^ client_physical_lsw ^ (ULONG)NX_RAND();
                                         
            /* If the DHCP Client is renewing or rebinding on being resumed, send a DHCP request. */
            if ((dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state == NX_DHCP_STATE_RENEWING) || 
                (dhcp_ptr -> nx_dhcp_interface_record[i].nx_dhcp_state == NX_DHCP_STATE_REBINDING))
            {

                /* Transmit the request message.  */
                _nx_dhcp_send_request_internal(dhcp_ptr, &dhcp_ptr -> nx_dhcp_interface_record[i], NX_DHCP_TYPE_DHCPREQUEST);
            }
        }
    }

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_suspend                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the Client suspend        */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful Completion status  */ 
/*    status                                Actual completion  status     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_suspend                       Actual Client suspend service */ 
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
UINT  _nxe_dhcp_suspend(NX_DHCP *dhcp_ptr)
{

UINT status;


    if (dhcp_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

   status = _nx_dhcp_suspend(dhcp_ptr);
   return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_suspend                                   PORTABLE C       */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function suspends the DHCP Client task thread and unbinds the  */
/*    socket port. The intented use of this service is to combine with the*/
/*    resume service such that the calling application can 'pause' the    */
/*    DHCP Client for a certain amount of time, and resume it in the same */
/*    state.  The DHCP Client application can then call the               */
/*    nx_dhcp_client_udpate_remaining_time service to update the time     */
/*    remaining on the client lease, then nx_dhcp_resume to actually      */
/*    resume the DHCP Client task.                                        */
/*                                                                        */ 
/*    This function does not change the Client state, lease timeout or    */
/*    time remaining on the current lease.  It requires the               */
/*    NX_DHCP_CLIENT_RESTORE_STATE option to be enabled.                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP instance      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful Completion status  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   nx_udp_socket_unbind                   Unbind port from UDP socket   */
/*   tx_timer_deactivate                    Stop the timer                */
/*   tx_thread_suspend                      Suspend the DHCP Client thread*/
/*   tx_mutex_get                           Obtain mutex protection       */ 
/*   tx_mutex_put                           Release mutex protection      */
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
UINT  _nx_dhcp_suspend(NX_DHCP *dhcp_ptr)
{


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

    /* Suspend the DHCP thread.  */
    tx_thread_suspend(&(dhcp_ptr -> nx_dhcp_thread));

    /* Deactive the timer.  */
    tx_timer_deactivate(&(dhcp_ptr -> nx_dhcp_timer));

    /* Unbind the port.  */
    nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket)); 

    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);
}

#endif /* NX_DHCP_CLIENT_RESTORE_STATE */    
#endif /* NX_DISABLE_IPV4 */
