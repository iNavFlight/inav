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
/**   Dynamic Host Configuration Protocol (DHCP) Server                   */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_DHCP_SERVER_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include    "nx_api.h"
#ifndef NX_DISABLE_IPV4
#include    "nx_ip.h"
#include    "nx_udp.h" 
#include    "nxd_dhcp_server.h"
#include    "tx_timer.h"
#include    "nx_packet.h"
#include    "nx_system.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS

#ifdef EL_PRINTF_ENABLE
#define EL_PRINTF      printf
#endif                                                                                               
                                
/* Define the DHCP Internal Function.  */
static VOID        _nx_dhcp_server_thread_entry(ULONG ip_instance);
static VOID        _nx_dhcp_slow_periodic_timer_entry(ULONG info);
static VOID        _nx_dhcp_fast_periodic_timer_entry(ULONG info); 
static UINT        _nx_dhcp_server_packet_process(NX_DHCP_SERVER *dhcp_ptr, NX_PACKET *packet_ptr);
static UINT        _nx_dhcp_respond_to_client_message(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr);
static UINT        _nx_dhcp_server_extract_information(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT **dhcp_client_ptr, NX_PACKET *packet_ptr, UINT iface_index);
static UINT        _nx_dhcp_process_option_data(NX_DHCP_CLIENT *dhcp_ptr, CHAR *buffer, UCHAR value, UINT get_option_data, UINT size);
static UINT        _nx_dhcp_add_option(UCHAR *bootp_message, UINT option, UINT size, ULONG value, UINT *index);
static UINT        _nx_dhcp_add_requested_option(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, UCHAR *buffer, UINT option, UINT *index);
static UINT        _nx_dhcp_set_server_options(NX_DHCP_SERVER *dhcp_ptr, CHAR *buffer, UINT buffer_length);
static UINT        _nx_dhcp_load_server_options(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr, UCHAR *buffer, UINT option_type, UINT *index);
static UINT        _nx_dhcp_parse_next_option(CHAR **buffer, UINT *digit, UINT length);
static UINT        _nx_dhcp_server_get_data(UCHAR *data, UINT size, ULONG *value);
static VOID        _nx_dhcp_server_store_data(UCHAR *data, UINT size, ULONG value);
static UINT        _nx_dhcp_clear_client_session(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr);
static UINT        _nx_dhcp_validate_client_message(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr);
static UINT        _nx_dhcp_find_client_record_by_chaddr(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, ULONG client_mac_msw, ULONG client_mac_lsw,NX_DHCP_CLIENT **dhcp_client_ptr, UINT add_on);
static UINT        _nx_dhcp_find_client_record_by_ip_address(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT **dhcp_client_ptr, UINT iface_index, ULONG assigned_ip_address);
static UINT        _nx_dhcp_server_assign_ip_address(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr);
static UINT        _nx_dhcp_find_interface_table_ip_address(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, ULONG ip_address, NX_DHCP_INTERFACE_IP_ADDRESS **return_interface_address);
static UINT        _nx_dhcp_update_assignable_ip_address(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr, ULONG ip_address, UINT assign_status); 
static UINT        _nx_dhcp_find_ip_address_owner(NX_DHCP_INTERFACE_IP_ADDRESS *iface_owner, NX_DHCP_CLIENT *client_record_ptr, UINT *assigned_to_client);
static UINT        _nx_dhcp_record_ip_address_owner(NX_DHCP_INTERFACE_IP_ADDRESS *iface_owner, NX_DHCP_CLIENT *client_record_ptr, UINT lease_time);
static UINT        _nx_dhcp_clear_ip_address_owner(NX_DHCP_INTERFACE_IP_ADDRESS *iface_owner);
static VOID        _nx_dhcp_server_socket_receive_notify(NX_UDP_SOCKET *socket_ptr);


/* To enable dhcp server output, define TESTOUTPUT. */
/* #define     TESTOUTPUT          1  */ 

/* To enable packet dump, define PACKET_DUMP. */
/* #define     PACKET_DUMP         1  */

#ifdef      TESTOUTPUT

static int  add_client = 0; 

/* Define how often to print the server table of current clients. 
   For no output set to zero.  For low volume, set to 1;
   for very high volume, set higher so as not to overwelm the processor. */

#define     TRACE_NTH_CLIENT_PACKET    1

#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_server_create                             PORTABLE C      */ 
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
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    stack_ptr                             Pointer to free memory        */
/*    stack_size                            Size of DHCP server stack     */
/*    name_ptr                              DHCP name pointer             */
/*    packet_pool                           Server packet pool for sending*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_server_create                Actual DHCP create function   */ 
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

UINT  _nxe_dhcp_server_create(NX_DHCP_SERVER *dhcp_ptr, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, 
                              CHAR *name_ptr, NX_PACKET_POOL *packet_pool)
{

UINT    status;


    /* Check for pointer invalid input.  */
    if ((dhcp_ptr == NX_NULL) || (packet_pool == NX_NULL) ||(stack_ptr == NX_NULL) ||(ip_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    if (stack_size == 0)
    {
        return(NX_DHCP_PARAMETER_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual DHCP create service.  */
    status =  _nx_dhcp_server_create(dhcp_ptr, ip_ptr, stack_ptr, stack_size, name_ptr, packet_pool);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_create                              PORTABLE C      */  
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the DHCP Server with necessary components.*/ 
/*    It creates a packet pool for sending Server DHCP messages and a     */ 
/*    thread task for the DHCP server operation. It also sets the standard*/
/*    DHCP options (e.g. lease time etc) for granting IP addresses.       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    stack_ptr                             Pointer to server thread on   */
/*                                                    the stack           */
/*    stack_size                            Size of DHCP server stack     */
/*    name_ptr                              DHCP name pointer             */
/*    packet_pool                           Server packet pool for sending*/
/*                                            DHCP replies to client      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*    NX_DHCP_INADEQUATE_PACKET_POOL_PAYLOAD                              */
/*                                          Packet payload too small error*/ 
/*    NX_DHCP_NO_SERVER_OPTION_LIST         Missing option list error     */
/*                                                                        */ 
/*    status                                Completion status of socket   */ 
/*                                               and thread create calls  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_create                  Create the DHCP UDP socket    */ 
/*    nx_udp_socket_delete                  Delete the DHCP UDP socket    */ 
/*    tx_thread_create                      Create DHCP processing thread */ 
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
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcp_server_create(NX_DHCP_SERVER *dhcp_ptr, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, 
                             CHAR *name_ptr, NX_PACKET_POOL *packet_pool_ptr)
{

UINT  status;
UINT  timer_ticks;
UINT  i, j;


    /* Initialize the DHCP control block to zero.  */
    memset((void *) dhcp_ptr, 0, sizeof(NX_DHCP_SERVER));

    /* Check the Server packet pool is at least large enough for a full sized DHCP message as per
       RFC 2131, plus header data. */
    if (packet_pool_ptr -> nx_packet_pool_payload_size < NX_DHCP_MINIMUM_PACKET_PAYLOAD)
    {

        /* Return the error status. */
        return(NX_DHCP_INADEQUATE_PACKET_POOL_PAYLOAD);
    }

    /* Set the packet pool pointer.  */
    dhcp_ptr -> nx_dhcp_packet_pool_ptr = packet_pool_ptr;

    /* Save the DHCP name.  */
    dhcp_ptr -> nx_dhcp_name = name_ptr;

    /* Set the DHCP Server attributes.  */
    dhcp_ptr -> nx_dhcp_ip_ptr = ip_ptr;

    /* Loop through all the interface tables and clear table memory. */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Clear all the table's IP address entries. */
        for (j = 0; j < NX_DHCP_IP_ADDRESS_MAX_LIST_SIZE; j++)
        {

            memset(&dhcp_ptr -> nx_dhcp_interface_table[i].nx_dhcp_ip_address_list[j], 
                   0, sizeof(NX_DHCP_INTERFACE_IP_ADDRESS));
        }

        /* Clear the DHCP interface itself. */
        memset(&dhcp_ptr -> nx_dhcp_interface_table[i], 0, sizeof(NX_DHCP_INTERFACE_TABLE));
    }

    /* Loop through the entire client record table. */
    for (i = 0; i < NX_DHCP_CLIENT_RECORD_TABLE_SIZE; i++)
    {

        /* Clear the client record. */
        memset(&dhcp_ptr -> client_records[i], 0, sizeof(NX_DHCP_CLIENT));
    }

    /* Verify the application has defined a server option list. */
    if (sizeof(NX_DHCP_SERVER_OPTION_LIST) == 1)
    {

        /* No, return the error status. */
        return(NX_DHCP_NO_SERVER_OPTION_LIST);
    }

    /* Create the list of DHCP options the server can provide for the client. */
    status = _nx_dhcp_set_server_options(dhcp_ptr, (CHAR *)NX_DHCP_SERVER_OPTION_LIST, sizeof(NX_DHCP_SERVER_OPTION_LIST) - 1);

    /* Was the option list set successful?  */
    if (status != NX_SUCCESS)
    {

        /* No, return error status.  */
        return(status);
    }

    /* Update the dhcp structure ID.  */
    dhcp_ptr -> nx_dhcp_id =  NX_DHCP_SERVER_ID;

    /* Create the Socket and check the status */
    status = nx_udp_socket_create(ip_ptr, &(dhcp_ptr -> nx_dhcp_socket), "NetX DHCP Server Socket",
                       NX_DHCP_TYPE_OF_SERVICE, NX_DHCP_FRAGMENT_OPTION, NX_DHCP_TIME_TO_LIVE, NX_DHCP_QUEUE_DEPTH);

    /* Was the socket creation successful?  */
    if (status != NX_SUCCESS)
    {

        /* No, return error status.  */
        return(status);
    }

    /* Save the DHCP instance pointer in the socket. */
    dhcp_ptr -> nx_dhcp_socket.nx_udp_socket_reserved_ptr =  (void *) dhcp_ptr;

    /* Set the DHCP request handler for packets received on the DHCP server socket. */
    status = nx_udp_socket_receive_notify(&(dhcp_ptr -> nx_dhcp_socket), _nx_dhcp_server_socket_receive_notify);

    /* Was the socket receive_notify set successful?  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

        /* No, return error status.  */
        return status;
    }

    /* Create the DHCP processing thread.  */
    status =  tx_thread_create(&(dhcp_ptr -> nx_dhcp_server_thread), "NetX DHCP Server Thread", 
                               _nx_dhcp_server_thread_entry, (ULONG)(ALIGN_TYPE)dhcp_ptr,
                               stack_ptr, stack_size, NX_DHCP_SERVER_THREAD_PRIORITY, 
                               NX_DHCP_SERVER_THREAD_PRIORITY, 1, TX_DONT_START);

    NX_THREAD_EXTENSION_PTR_SET(&(dhcp_ptr -> nx_dhcp_server_thread), dhcp_ptr)

    /* Determine if the thread creation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

        /* No, return error status.  */
        return(status);
    }

    /* Create the DHCP mutex.  */
    status = tx_mutex_create(&dhcp_ptr -> nx_dhcp_mutex, "DHCP Server Mutex", TX_NO_INHERIT);

    /* Determine if the thread creation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

        /* Delete DHCP thead.  */
        tx_thread_delete(&(dhcp_ptr -> nx_dhcp_server_thread));

        /* No, return error status.  */
        return(status);
    }

    /* Convert seconds to timer ticks. */
    timer_ticks = NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL * NX_IP_PERIODIC_RATE;

    /* Create the timer for Client DHCP session. This will keep track of when leases expire 
       and when a client session has timed out. */
    status = tx_timer_create(&(dhcp_ptr -> nx_dhcp_slow_periodic_timer), "DHCP Server IP Lease Timer", 
                             _nx_dhcp_slow_periodic_timer_entry, (ULONG)(ALIGN_TYPE)dhcp_ptr, 
                             timer_ticks, timer_ticks, TX_NO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(dhcp_ptr -> nx_dhcp_slow_periodic_timer), dhcp_ptr)

    /* Convert seconds to timer ticks. */
    timer_ticks = NX_DHCP_FAST_PERIODIC_TIME_INTERVAL * NX_IP_PERIODIC_RATE;

    /* Create the timer for Client DHCP session. This will keep track of when leases expire 
       and when a client session has timed out. */
    status += tx_timer_create(&(dhcp_ptr -> nx_dhcp_fast_periodic_timer), "DHCP Server Session Timer", 
                              _nx_dhcp_fast_periodic_timer_entry, (ULONG)(ALIGN_TYPE)dhcp_ptr, 
                              timer_ticks, timer_ticks, TX_NO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(dhcp_ptr -> nx_dhcp_fast_periodic_timer), dhcp_ptr)

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

        /* Delete DHCP thead.  */
        tx_thread_delete(&(dhcp_ptr -> nx_dhcp_server_thread));

        /* Delete ThreadX resources. */
        tx_mutex_delete(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Delete DHCP timer.  */
        tx_timer_delete(&(dhcp_ptr -> nx_dhcp_slow_periodic_timer));
        tx_timer_delete(&(dhcp_ptr -> nx_dhcp_fast_periodic_timer));

        /* Return the error status. */
        return(status);
    }

    /* Create the DHCP event flag instance.  */
    status = tx_event_flags_create(&dhcp_ptr -> nx_dhcp_server_events, "DHCP Server Events");

    /* Check for error. */
    if (status != TX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcp_ptr -> nx_dhcp_socket));

        /* Delete DHCP thread.  */
        tx_thread_delete(&(dhcp_ptr -> nx_dhcp_server_thread));

        /* Delete ThreadX resources. */
        tx_mutex_delete(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Delete DHCP Timer.  */
        tx_timer_delete(&(dhcp_ptr -> nx_dhcp_slow_periodic_timer));

        tx_timer_delete(&(dhcp_ptr -> nx_dhcp_fast_periodic_timer));

        /* Return the error status. */
        return(status);
    }

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_fast_periodic_timer_entry                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function watches the session timeout on each active client     */
/*    session. WHen it does it clears session data from the client record */ 
/*    and resets the client state to INIT. If a iP address had been       */
/*    assigned, it is returned back to the server pool.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    info                                 Generic pointer to DHCP server */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     tx_event_flags_set                   Adds a fast periodic event    */
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
static VOID  _nx_dhcp_fast_periodic_timer_entry(ULONG info)
{

NX_DHCP_SERVER   *dhcp_ptr;


    /* Setup DHCP pointer.  */
    NX_TIMER_EXTENSION_PTR_GET(dhcp_ptr, NX_DHCP_SERVER, info)

    /* Signal the DHCP Server fast periodic event.  */
    tx_event_flags_set(&(dhcp_ptr -> nx_dhcp_server_events), NX_DHCP_SERVER_FAST_PERIODIC_EVENT, TX_OR);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_slow_periodic_timer_entry                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function watches lease timeout on assigned IP address in the   */
/*    server database.  When one has expired, the address is returned to  */
/*    the available pool and the Client is reset to the INIT state.       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    info                                 Generic pointer to DHCP server */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*     tx_event_flags_set                   Adds a slow periodic event    */
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
static VOID  _nx_dhcp_slow_periodic_timer_entry(ULONG info)
{

NX_DHCP_SERVER                  *dhcp_ptr;


    /* Setup DHCP pointer.  */
    NX_TIMER_EXTENSION_PTR_GET(dhcp_ptr, NX_DHCP_SERVER, info)

    /* Signal the DHCP Server slow periodic event.  */
    tx_event_flags_set(&(dhcp_ptr -> nx_dhcp_server_events), NX_DHCP_SERVER_SLOW_PERIODIC_EVENT, TX_OR);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_socket_receive_notify               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is notified by NetX Duo when a packet arrives in the  */
/*    server socket. It sets a flag which the DHCP server thread          */
/*    detect so it will know to process the incoming packet .             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Pointer to Server Socket      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*     tx_event_flags_set                   Adds a packet receive event   */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Threads                                                             */ 
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
VOID  _nx_dhcp_server_socket_receive_notify(NX_UDP_SOCKET *socket_ptr)
{

NX_DHCP_SERVER  *dhcp_server_ptr;


    /* Get a pointer to the DHCP server.  */
    dhcp_server_ptr =  (NX_DHCP_SERVER *) socket_ptr -> nx_udp_socket_reserved_ptr;

    /* Signal the DHCP Server it has a UDP packet on its socket receive queue.  */
    tx_event_flags_set(&(dhcp_server_ptr -> nx_dhcp_server_events), NX_DHCP_SERVER_RECEIVE_EVENT, TX_OR);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_server_delete                             PORTABLE C      */ 
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
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_server_delete                Actual DHCP delete function   */ 
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
UINT  _nxe_dhcp_server_delete(NX_DHCP_SERVER *dhcp_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if (dhcp_ptr == NX_NULL)
        return(NX_PTR_ERROR);

    if (dhcp_ptr -> nx_dhcp_id != NX_DHCP_SERVER_ID)
    {
        return(NX_DHCP_PARAMETER_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP delete service.  */
    status =  _nx_dhcp_server_delete(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_delete                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the DHCP server   and releases all of its     */ 
/*    resources.                                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_unbind                  Free the UDP socket port      */ 
/*    nx_udp_socket_delete                  Delete the DHCP UDP socket    */ 
/*    tx_thread_terminate                   Terminate DHCP thread         */ 
/*    tx_thread_terminate                   Terminate DHCP thread         */ 
/*    tx_thread_delete                      Delete DHCP thread            */ 
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcp_server_delete(NX_DHCP_SERVER *dhcp_ptr)
{


    /* Determine if DHCP is stopped.  */
    if (dhcp_ptr -> nx_dhcp_started != NX_FALSE)
    {
   
        /* Stop the DHCP server.  */
        _nx_dhcp_server_stop(dhcp_ptr);
    }    

    /* Delete the UDP socket.  */
    nx_udp_socket_delete(&(dhcp_ptr  -> nx_dhcp_socket));

    /* Delete the session ("fast") timer. */
    tx_timer_delete(&(dhcp_ptr -> nx_dhcp_fast_periodic_timer));

    /* Delete the IP lease ("slow") timer. */
    tx_timer_delete(&(dhcp_ptr -> nx_dhcp_slow_periodic_timer));

    /* Suspend the DHCP processing thread.  */
    tx_thread_suspend(&(dhcp_ptr -> nx_dhcp_server_thread));

    /* Terminate the DHCP processing thread.  */
    tx_thread_terminate(&(dhcp_ptr -> nx_dhcp_server_thread));

    /* Delete the DHCP processing thread.  */
    tx_thread_delete(&(dhcp_ptr -> nx_dhcp_server_thread));

    /* Delete the mutexes.  */
    tx_mutex_delete(&(dhcp_ptr -> nx_dhcp_mutex)); 

    /* Delete the flag event.  */
    tx_event_flags_delete(&(dhcp_ptr -> nx_dhcp_server_events));

    /* Clear the dhcp structure ID. */
    dhcp_ptr -> nx_dhcp_id =  0;

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_create_server_ip_address_list             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the IP address list create*/
/*    service.                                                            */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    dhcp_ptr                        Pointer to DHCP Server              */ 
/*    iface_index                     IP interface index to specify the   */
/*                                       DHCP server interface to use     */ 
/*    start_ip_address                Starting IP address                 */
/*    end_ip_address                  Ending IP address                   */
/*    addresses_added                 Pointer to addresses added counter  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                      Successful completion status        */ 
/*    NX_PTR_ERROR                    Invalid pointer input               */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_create_server_ip_address_list                              */ 
/*                                    Actual IP address create service    */
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*     Application code                                                   */ 
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
UINT _nxe_dhcp_create_server_ip_address_list(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, 
                                             ULONG start_ip_address, ULONG end_ip_address, UINT *addresses_added)
{

UINT            status;


    /* Check for invalid input.  */
    if ((dhcp_ptr == NX_NULL) || (addresses_added == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid non pointer input. */
    if ((start_ip_address > end_ip_address) ||
        (iface_index >= NX_MAX_PHYSICAL_INTERFACES) ||
        (start_ip_address == NX_DHCP_NO_ADDRESS))
    {

        return(NX_DHCP_INVALID_IP_ADDRESS);                                           
    }

    status = _nx_dhcp_create_server_ip_address_list(dhcp_ptr, iface_index, start_ip_address, end_ip_address, addresses_added);

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_create_server_ip_address_list              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function creates a list of available IP addresses for DHCP      */
/*   clients on the specified interface. The caller supplies a start and  */
/*   end IP address and all addresses in between are flagged as available.*/  
/*   The start ip address is assumed to be the first entry in the list,   */
/*   not appended to a previously started list. All the IP addresses  may */
/*   not fit in the server interface table; the number actually added is  */
/*   written to the 'addresses added' variable. The caller should check   */
/*   this output.                                                         */
/*                                                                        */ 
/*   If some addresses within the specified range need to be reserved or  */
/*   statically assigned the host application needs to set their lease to */
/*   infinity (0xffffffff) and assigned status is set. This function also */
/*   add the interface subnet, router, dns server for specified index.    */  
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                           Pointer to DHCP server           */ 
/*    iface_index                        Index specifying server interface*/
/*    start_ip_address                   Beginning IP address in list.    */
/*    end_ip_address                     Ending IP address in list.       */
/*    addresses_added                    Number of addresses added to list*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP list creation      */
/*    NX_DHCP_SERVER_BAD_INTERFACE_INDEX Invalid interface index input    */ 
/*    NX_DHCP_INVALID_IP_ADDRESS_LIST    Invalid IP list parameter input  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     _nx_dhcp_update_assignable_ip_address                              */ 
/*                                        Updates interface table entry   */
/*                                        with IP address status and owner*/
/*     _nx_dhcp_clear_client_session      Clears client record of session */ 
/*                                           data from most recent client */
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
UINT  _nx_dhcp_create_server_ip_address_list(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, 
                                            ULONG start_ip_address, ULONG end_ip_address,
                                            UINT *addresses_added)
{

UINT                         i;
ULONG                        next_ip_address;
NX_IP                        *ip_ptr;
NX_DHCP_INTERFACE_IP_ADDRESS *ip_address_entry_ptr;
NX_DHCP_INTERFACE_TABLE      *dhcp_interface_table_ptr;


    /* Check for an invalid interface index. */
    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return NX_DHCP_SERVER_BAD_INTERFACE_INDEX;
    }

    /* Obtain DHCP Server mutex protection,. */
    tx_mutex_get(&dhcp_ptr -> nx_dhcp_mutex, NX_WAIT_FOREVER);

    *addresses_added = 0;
    next_ip_address = start_ip_address;

    /* Set local pointer to the list being created for convenience. */
    dhcp_interface_table_ptr = &dhcp_ptr -> nx_dhcp_interface_table[iface_index];

    /* Fill in the interface table with DHCP attributes for the specified interface. */

    /* Set up a local variable for convenience. */
    ip_ptr = dhcp_ptr -> nx_dhcp_ip_ptr;

    /* Assign the IP interface specified by the IP interface index to the DHCP interface. */
    dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_incoming_interface = &(ip_ptr -> nx_ip_interface[iface_index]);

    /* Set the server ip address based on the specified interface. */
    dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_server_ip_address = ip_ptr -> nx_ip_interface[iface_index].nx_interface_ip_address;

    /* Set the dns server, subnet mask, and router (default gateway) for this interface. */
    dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_dns_ip_address = ip_ptr -> nx_ip_interface[iface_index].nx_interface_ip_address;
    dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_subnet_mask = ip_ptr -> nx_ip_interface[iface_index].nx_interface_ip_network_mask;     
    dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_router_ip_address = ip_ptr -> nx_ip_gateway_address;
    

    /* Make sure the start/end address subnet match the interface subnet. */
    if ((dhcp_interface_table_ptr -> nx_dhcp_subnet_mask & start_ip_address) != 
        (dhcp_interface_table_ptr -> nx_dhcp_subnet_mask & dhcp_interface_table_ptr -> nx_dhcp_server_ip_address))
    {

        /* Release DHCP Server mutex.  */
        tx_mutex_put(&dhcp_ptr -> nx_dhcp_mutex);

        return(NX_DHCP_SERVER_BAD_INTERFACE_INDEX);
    }
    if ((dhcp_interface_table_ptr -> nx_dhcp_subnet_mask & end_ip_address) != 
        (dhcp_interface_table_ptr -> nx_dhcp_subnet_mask & dhcp_interface_table_ptr -> nx_dhcp_server_ip_address))
    {

        /* Release DHCP Server mutex.  */
        tx_mutex_put(&dhcp_ptr -> nx_dhcp_mutex);

        return(NX_DHCP_SERVER_BAD_INTERFACE_INDEX);
    }

    /* Zero out the list size. */
    dhcp_interface_table_ptr -> nx_dhcp_address_list_size = 0;

    /* Check for invalid list parameters. */
    if ((start_ip_address > end_ip_address) || !start_ip_address || !end_ip_address)
    {

        /* Release DHCP Server mutex.  */
        tx_mutex_put(&dhcp_ptr -> nx_dhcp_mutex);

        return(NX_DHCP_INVALID_IP_ADDRESS_LIST);
    }

    /* Clear out existing entries and start adding IP addresses at the beginning of the list. */
    i = 0;

    /* Fit as many IP addresses in the specified range as will fit in the table. */
    while (i < NX_DHCP_IP_ADDRESS_MAX_LIST_SIZE && next_ip_address <= end_ip_address)
    {

        /* Set local pointer to the next entry for convenience. */
        ip_address_entry_ptr = &dhcp_interface_table_ptr -> nx_dhcp_ip_address_list[i];

        /* Clear existing entry; client identifier and available status 
           must be initialized to NULL and not assigned respectively.  */
        memset(ip_address_entry_ptr, 0, sizeof(NX_DHCP_INTERFACE_IP_ADDRESS));

        /* Add the next IP address to the list. */
        ip_address_entry_ptr -> nx_assignable_ip_address = next_ip_address;

        /* Increase the list size. */
        dhcp_interface_table_ptr -> nx_dhcp_address_list_size++;

        next_ip_address++;
        i++;
    }

    /* Set the actual number of available addresses added to the table. */
    *addresses_added = dhcp_interface_table_ptr -> nx_dhcp_address_list_size;

    /* Release DHCP Server mutex.  */
    tx_mutex_put(&dhcp_ptr -> nx_dhcp_mutex);

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_set_interface_network_parameters          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking services for the set interface */
/*   network parameters service.                                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                           Pointer to DHCP server           */ 
/*    iface_index                        Index specifying server interface*/
/*    subnet_mask                        Network mask for DHCP clients    */
/*    default_gateway_address            Router/default gateway for client*/
/*    dns_server_address                 DNS server for DHCP clients      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */
/*    NX_PTR_ERROR                       Invalid interface index input    */ 
/*    NX_DHCP_INVALID_NETWORK_PARAMETERS Invalid network parameter input  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     _nx_dhcp_update_assignable_ip_address                              */ 
/*                                        Updates interface table entry   */
/*                                        with IP address status and owner*/
/*     _nx_dhcp_clear_client_session      Clears client record of session */ 
/*                                           data from most recent client */
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
UINT  _nxe_dhcp_set_interface_network_parameters(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, 
                                            ULONG subnet_mask, ULONG default_gateway_address,
                                            ULONG dns_server_address)
{
UINT status;

    /* Check for invalid pointer input. */
    if (dhcp_ptr == NX_NULL) 
    {
        return(NX_PTR_ERROR);
    }

    /* Check for non pointer input. */
    if ((subnet_mask == 0) || (default_gateway_address == 0) || (dns_server_address == 0))
    {
        return(NX_DHCP_INVALID_NETWORK_PARAMETERS);
    }

    /* Call the actual service. */
    status = _nx_dhcp_set_interface_network_parameters(dhcp_ptr, iface_index, 
                                            subnet_mask, default_gateway_address,
                                            dns_server_address);
    /* Return completion status. */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_set_interface_network_parameters           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function sets the DHCP server default options for 'network      */  
/*   critical parameters:' gateway, dns server and network mask for the   */
/*   specified interface.                                                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                           Pointer to DHCP server           */ 
/*    iface_index                        Index specifying server interface*/
/*    subnet_mask                        Network mask for DHCP clients    */
/*    default_gateway_address            Router/default gateway for client*/
/*    dns_server_address                 DNS server for DHCP clients      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Valid parameters received        */
/*    NX_DHCP_SERVER_BAD_INTERFACE_INDEX Invalid interface index input    */ 
/*    NX_DHCP_INVALID_NETWORK_PARAMETERS Invalid network parameter input  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     None                                                               */
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
UINT  _nx_dhcp_set_interface_network_parameters(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, 
                                            ULONG subnet_mask, ULONG default_gateway_address,
                                            ULONG dns_server_address)
{

    /* Check for invalid non pointer input. */
    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {

        return(NX_DHCP_SERVER_BAD_INTERFACE_INDEX);                                           
    }

    /* The default gateway should be on the same network as the dhcp server interface. */
    if ((default_gateway_address & subnet_mask) != 
        (dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_server_ip_address & 
                        dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_subnet_mask))
    {
        return(NX_DHCP_INVALID_NETWORK_PARAMETERS);
    }

    /* Obtain DHCP Server mutex protection,. */
    tx_mutex_get(&dhcp_ptr -> nx_dhcp_mutex, NX_WAIT_FOREVER);

    /* Set the subnet, dns server, and router address for this interface. */
    dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_dns_ip_address = dns_server_address;
    
    dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_subnet_mask = subnet_mask;
    
    dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_router_ip_address = default_gateway_address;

    /* Release DHCP Server mutex.  */
    tx_mutex_put(&dhcp_ptr -> nx_dhcp_mutex);

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_server_start                              PORTABLE C      */ 
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
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_start                        Actual DHCP start function    */ 
/*    nx_udp_socket_bind                    Bind the DHCP UDP socket      */ 
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
UINT  _nxe_dhcp_server_start(NX_DHCP_SERVER *dhcp_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if (dhcp_ptr == NX_NULL)
        return(NX_PTR_ERROR);

    if (dhcp_ptr -> nx_dhcp_id != NX_DHCP_SERVER_ID)
    {
        return(NX_DHCP_PARAMETER_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual DHCP start service.  */
    status =  _nx_dhcp_server_start(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_start                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initiates the DHCP processing thread.                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_bind                    Bind DHCP socket              */ 
/*    nx_udp_socket_unbind                  Unbind DHCP socket            */ 
/*    tx_thread_resume                      Initiate DHCP processing      */ 
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
UINT  _nx_dhcp_server_start(NX_DHCP_SERVER *dhcp_ptr)
{

UINT  status;


    /* Determine if DHCP has already been started.  */
    if (dhcp_ptr -> nx_dhcp_started)
    {

        /* Error DHCP has already been started.  */

        /* Return completion status.  */
        return(NX_DHCP_SERVER_ALREADY_STARTED);
    }

    dhcp_ptr -> nx_dhcp_started = NX_TRUE;

    /* Bind the UDP socket to the DHCP Socket port.  */
    status =  nx_udp_socket_bind(&(dhcp_ptr -> nx_dhcp_socket), NX_DHCP_SERVER_UDP_PORT, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {

        /* Error, unbind the DHCP socket.  */
        nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket));

        /* Set status to DHCP error code.  */
        return(status);   
    }

    /* Start the session ("fast") timer. */
    tx_timer_activate(&(dhcp_ptr -> nx_dhcp_fast_periodic_timer));

    /* Start the IP lease ("slow") timer. */
    tx_timer_activate(&(dhcp_ptr -> nx_dhcp_slow_periodic_timer));

    /* Start the DHCP server thread.  */
    tx_thread_resume(&(dhcp_ptr -> nx_dhcp_server_thread));


    /* Return completion status.  */
    return(NX_SUCCESS);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_server_stop                               PORTABLE C      */ 
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
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
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
UINT  _nxe_dhcp_server_stop(NX_DHCP_SERVER *dhcp_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if (dhcp_ptr == NX_NULL)
        return(NX_PTR_ERROR);

    if (dhcp_ptr -> nx_dhcp_id != NX_DHCP_SERVER_ID)
    {
        return(NX_DHCP_PARAMETER_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP stop service.  */
    status =  _nx_dhcp_server_stop(dhcp_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_stop                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function halts the DHCP processing thread.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_unbind                  Unbind the DHCP UDP socket    */ 
/*    tx_thread_preemption_change           Change the thread preemption  */ 
/*    tx_thread_suspend                     Suspend DHCP processing       */ 
/*    tx_thread_wait_abort                  Remove any thread suspension  */
/*    tx_timer_deactivate                   Deactivate timer              */ 
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
UINT  _nx_dhcp_server_stop(NX_DHCP_SERVER *dhcp_ptr)
{

UINT    current_preemption;


    /* Determine if DHCP is started.  */
    if (dhcp_ptr -> nx_dhcp_started == NX_FALSE)
    {
   
        /* DHCP is not started so it can't be stopped.  */
        return(NX_DHCP_SERVER_NOT_STARTED);
    }    

    /* Obtain mutex protection, which is to say don't interrupt a DHCP server
       if it is processing a packet from a DHCP client. */
    tx_mutex_get(&dhcp_ptr -> nx_dhcp_mutex, NX_WAIT_FOREVER);

    /* Stop the session ("fast") timer. */
    tx_timer_deactivate(&(dhcp_ptr -> nx_dhcp_fast_periodic_timer));

    /* Stop the IP lease ("slow") timer. */
    tx_timer_deactivate(&(dhcp_ptr -> nx_dhcp_slow_periodic_timer));

    /* Clear the started flag here to ensure other threads can't issue a stop while
       this stop is in progress.  */
    dhcp_ptr -> nx_dhcp_started =  NX_FALSE;

    /* Disable preemption for critical section.  */
    tx_thread_preemption_change(tx_thread_identify(), 0, &current_preemption);

    /* Suspend the DHCP thread.  */
    tx_thread_suspend(&(dhcp_ptr -> nx_dhcp_server_thread));

    /* Unbind the port.  */
    nx_udp_socket_unbind(&(dhcp_ptr -> nx_dhcp_socket));

    /* Restore preemption.  */
    tx_thread_preemption_change(tx_thread_identify(), current_preemption, &current_preemption);

    tx_mutex_put(&dhcp_ptr -> nx_dhcp_mutex);


    /* Return completion status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_thread_entry                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is top level server processing thread for the DHCP    */
/*    service. It listens for and processes client requests in an infinite*/ 
/*    loop.                                                               */
/*                                                                        */ 
/*    Note that the current implementation expects a Client to  initiate  */
/*    a session with DISCOVER message rather than using the option of     */
/*    skipping to the REQUEST message.                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_instance                         Pointer to the DHCP server    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_process                      Main DHCP processing function */ 
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
static VOID  _nx_dhcp_server_thread_entry(ULONG info)
{

NX_DHCP_SERVER                  *dhcp_ptr;
ULONG                           dhcp_events;
NX_DHCP_INTERFACE_TABLE         *iface_table_ptr;
NX_DHCP_INTERFACE_IP_ADDRESS    *iface_address_ptr;
NX_DHCP_CLIENT                  *dhcp_client_ptr;
UINT                            iface_index, i;
UINT                            status;
NX_PACKET                       *packet_ptr;


    /* Setup the DHCP pointer.  */
    NX_THREAD_EXTENSION_PTR_GET(dhcp_ptr, NX_DHCP_SERVER, info)

    /* Enter while loop.  */
    while(1)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcp_ptr -> nx_dhcp_mutex));

        /* Pick up IP event flags.  */
        tx_event_flags_get(&dhcp_ptr -> nx_dhcp_server_events, NX_DHCP_SERVER_ALL_EVENTS, 
                           TX_OR_CLEAR, &dhcp_events, TX_WAIT_FOREVER);

        /* Obtain the DHCP mutex before processing the DHCP event.  */
        tx_mutex_get(&(dhcp_ptr -> nx_dhcp_mutex), TX_WAIT_FOREVER);

        /* Check events.  */

        /* Packet receive event.  */
        if (dhcp_events & NX_DHCP_SERVER_RECEIVE_EVENT)
        {

            /* Loop to receive DHCP message.  */
            while(1)
            {

                /* Check for an incoming DHCP packet with non blocking option. */
                status = _nx_udp_socket_receive(&dhcp_ptr -> nx_dhcp_socket, &packet_ptr, NX_NO_WAIT);

                /* Check for packet receive errors. */
                if (status != NX_SUCCESS)
                {

#ifdef EL_PRINTF_ENABLE
                    EL_PRINTF("DHCPserv: Error receiving DHCP packet 0x%x\n", status);
#endif
                    break;
                }

                /* Process DHCP packet.  */
                _nx_dhcp_server_packet_process(dhcp_ptr, packet_ptr);
            }
        }

        /* FAST periodic event.  */
        if (dhcp_events & NX_DHCP_SERVER_FAST_PERIODIC_EVENT)
        {

            /* Check the clients in active session with the server. */
            for (i = 0; i < NX_DHCP_CLIENT_RECORD_TABLE_SIZE; i++)
            {

                /* Create a local pointer for convenience. */
                dhcp_client_ptr = &dhcp_ptr -> client_records[i];

                /* Skip empty records. */
                if ((dhcp_client_ptr -> nx_dhcp_client_mac_lsw == 0) && (dhcp_client_ptr -> nx_dhcp_client_mac_msw == 0))
                    continue;

                /* Skip clients not in active session with the Server, static members, 
                or client in the bound state. */
                if (dhcp_client_ptr -> nx_dhcp_session_timeout == TX_WAIT_FOREVER)
                    continue;
                if (dhcp_client_ptr -> nx_dhcp_client_state == NX_DHCP_STATE_BOUND)
                    continue;

                /* Skip clients whose session timeout is not set e.g. previous session
                   failed and no longer configured with this server. */
                if (dhcp_client_ptr -> nx_dhcp_session_timeout == 0)
                    continue;
                if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address == NX_DHCP_NO_ADDRESS)
                    continue;

                /* Check how much time is left before decreasing the time remaining on the session timeout. */
                if (dhcp_client_ptr -> nx_dhcp_session_timeout >= NX_DHCP_FAST_PERIODIC_TIME_INTERVAL)
                {
                    /* Ok to decrement the full amount. */
                    dhcp_client_ptr -> nx_dhcp_session_timeout -= NX_DHCP_FAST_PERIODIC_TIME_INTERVAL;
                }
                else
                {

                    /* Set to zero. Time's up! */
                    dhcp_client_ptr -> nx_dhcp_session_timeout = 0;
                }

                /* Has time expired on this session? */
                if (dhcp_client_ptr -> nx_dhcp_session_timeout == 0)
                {

                    /* Yes, the client has bailed on the session, intentionally or otherwise. If it wants to 
                       get an IP address it must start again. Reset status to 'INIT'. */
                    dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;

#ifdef EL_PRINTF_ENABLE
                    EL_PRINTF("DHCPserv: client session has timed out. Clear session data\n");
#endif

                    /* Does the client have an assigned IP address? */
                    if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address)
                    {

                        /* Yes, return it back to the available pool. */
                        _nx_dhcp_update_assignable_ip_address(dhcp_ptr, dhcp_client_ptr, 
                                                              dhcp_client_ptr -> nx_dhcp_assigned_ip_address, 
                                                              NX_DHCP_ADDRESS_STATUS_MARK_AVAILABLE);

                        /* The assigned address must be removed. */
                        dhcp_client_ptr -> nx_dhcp_assigned_ip_address = NX_DHCP_NO_ADDRESS;

                        /* Clear the session data, including any data read from last client DHCP message. */
                        _nx_dhcp_clear_client_session(dhcp_ptr, dhcp_client_ptr);
                    }
                }
            }
        }

        /* Slow periodic event.  */
        if (dhcp_events & NX_DHCP_SERVER_SLOW_PERIODIC_EVENT)
        {

            for (iface_index = 0; iface_index < NX_MAX_PHYSICAL_INTERFACES; iface_index++)
            {

                /* Set a local pointer for convenience. */
                iface_table_ptr = &dhcp_ptr -> nx_dhcp_interface_table[iface_index];

                /* Check the interface addresses in the table for lease expiration. */
                for (i = 0; i < iface_table_ptr -> nx_dhcp_address_list_size; i++)
                {

                    iface_address_ptr = &iface_table_ptr -> nx_dhcp_ip_address_list[i];

                    /* Skip all the entries who are not assigned or assigned indefinately (static). */ 
                    if(iface_address_ptr -> assigned == NX_FALSE)
                        continue;
                    if(iface_address_ptr -> lease_time == TX_WAIT_FOREVER)
                        continue;

                    /* Check how much time is left before decreasing the time remaining on the lease timeout. */
                    if (iface_address_ptr -> lease_time >= NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL)
                    {

                        /* Ok to decrement the full amount. */
                        iface_address_ptr -> lease_time -= NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL;
                    }
                    else
                    {

                        /* Set to zero. Time's up! */
                        iface_address_ptr -> lease_time = 0;
                    }

                    /* Has time expired on this lease? */
                    if (iface_address_ptr -> lease_time == 0)
                    {

                        /* Yes, make this address available. */

                        /* Clear the 'owner' field. */
                        _nx_dhcp_clear_ip_address_owner(iface_address_ptr);

                        /* Look up the client in the server database by its assigned address. */
                        _nx_dhcp_find_client_record_by_ip_address(dhcp_ptr, &dhcp_client_ptr, iface_index, 
                                                                  iface_address_ptr -> nx_assignable_ip_address);

                        /* Did we find it? */
                        if (dhcp_client_ptr != NX_NULL)
                        {

                            /* Remove the assigned IP address and reset the Client to the INIT state. */
                            dhcp_client_ptr -> nx_dhcp_assigned_ip_address = NX_DHCP_NO_ADDRESS;
                            dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;

#ifdef EL_PRINTF_ENABLE
                            EL_PRINTF("DHCPserv: client IP lease expired. Clear client data, release IP address\n");
#endif

                            /* Clear all session data if there is any for this client. */
                            _nx_dhcp_clear_client_session(dhcp_ptr, dhcp_client_ptr);

                            /* No, we should have this client in the server client records table but we don't. 
                            That's all we can do right now. */
                        }
                    }
                }
            }
        }
    };

    /* We only break out of this loop in the event of the server stopping
       or a fatal error occurring.*/
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_respond_to_client_message                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function prepares a DHCP message response and sends it back to */
/*    the  Client.  Most of the information is obtained from specified    */
/*    input or from client session record.                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                             Pointer to DHCP Server         */ 
/*    dhcp_client_ptr                      Pointer to client to respond to*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                                Completion status from various*/ 
/*                                              NetX calls                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a DHCP packet        */ 
/*    nx_packet_release                     Release DHCP packet           */ 
/*    nx_udp_socket_send                    Send DHCP packet              */ 
/*    _nx_dhcp_add_option                   Add an option to the request  */ 
/*    _nx_dhcp_add_requested_option         Adds non standard option to   */
/*                                                 server reply to client */
/*    _nx_dhcp_load_server_options          Load option data from server  */
/*                                             option list                */
/*    _nx_dhcp_server_store_data            Store data into buffer        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_dhcp_listen_for_messages           Service Client DHCP messages  */
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
static UINT  _nx_dhcp_respond_to_client_message(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr)
{

NX_PACKET               *packet_ptr;
UCHAR                   *buffer;
UINT                     status;
UINT                     destination_ip_address;
UINT                     destination_port;
UINT                     index = 0;


#ifdef EL_PRINTF_ENABLE
    /* Set local variables for convenience. */
    EL_PRINTF("DHCPserv: respond to client message on interface %d\n", dhcp_client_ptr -> nx_dhcp_client_iface_index);
#endif

    /* Allocate a DHCP packet.  */
    status =  nx_packet_allocate(dhcp_ptr -> nx_dhcp_packet_pool_ptr, &packet_ptr, NX_IPv4_UDP_PACKET, NX_DHCP_PACKET_ALLOCATE_TIMEOUT);

    /* Was the packet allocation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Return status.  */
        return(status);
    }

    /* Setup the packet buffer pointer.  */
    buffer =  packet_ptr -> nx_packet_prepend_ptr;

    /* Clear the buffer memory.  */
    memset((void *) buffer, 0, NX_DHCP_OFFSET_END);

    /* Setup the standard DHCP fields.  */
    buffer[NX_DHCP_OFFSET_OP] =        NX_DHCP_OP_REPLY;
    buffer[NX_DHCP_OFFSET_HTYPE] =     (UCHAR)dhcp_client_ptr -> nx_dhcp_client_hwtype;      
    buffer[NX_DHCP_OFFSET_HLEN] =      (UCHAR)dhcp_client_ptr -> nx_dhcp_client_hwlen;  
    buffer[NX_DHCP_OFFSET_HOPS] =      0;
    buffer[NX_DHCP_OFFSET_BOOT_FILE] = 0;                          
   
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_XID, 4, dhcp_client_ptr -> nx_dhcp_xid);       
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_SECS, 2, 0);        
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_CLIENT_IP, 4, NX_DHCP_NO_ADDRESS);
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_SERVER_IP, 4, NX_DHCP_NO_ADDRESS); 
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_RELAY_IP, 4, NX_DHCP_NO_ADDRESS);
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_FLAGS, 1, dhcp_client_ptr -> nx_dhcp_broadcast_flag_set);  
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_CLIENT_HW, 2, dhcp_client_ptr -> nx_dhcp_client_mac_msw);
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_CLIENT_HW + 2, 4, dhcp_client_ptr -> nx_dhcp_client_mac_lsw);
    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_VENDOR, 4, NX_DHCP_MAGIC_COOKIE);
                                                
    /* Update the index.  */
    index = NX_DHCP_OFFSET_OPTIONS; 

    /* Add the list of options that go out with ALL server messages. */
    status = _nx_dhcp_load_server_options(dhcp_ptr, dhcp_client_ptr, buffer, NX_DHCP_OPTIONS_FOR_ALL_REPLIES, &index);

    /* Unless the server is sending a NACK, there are more options to add for the server reply. */
    if (dhcp_client_ptr -> nx_dhcp_response_type_to_client != NX_DHCP_TYPE_DHCPNACK)
    {

        /* Add the list of options that go out with server ACK replies (standard network parameters). */
        _nx_dhcp_load_server_options(dhcp_ptr, dhcp_client_ptr, buffer, NX_DHCP_OPTIONS_FOR_GENERIC_ACK, &index);
    
        /* Determine if there are any message specific options to add. */
        switch (dhcp_client_ptr -> nx_dhcp_message_type)
        {
    
            case NX_DHCP_TYPE_DHCPDISCOVER:

                /* Are we returning an OFFER to the Client? */
                if (dhcp_client_ptr -> nx_dhcp_response_type_to_client == NX_DHCP_TYPE_DHCPOFFER)
                {

                    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_YOUR_IP, 4, dhcp_client_ptr -> nx_dhcp_your_ip_address);   

                    /* Add the list of options that go out with server OFFER replies. */
                    _nx_dhcp_load_server_options(dhcp_ptr, dhcp_client_ptr, buffer, NX_DHCP_OPTIONS_FOR_REPLY_TO_OFFER, &index);
                }
    
                /* Increment the number of Discovery messages received.  */
                dhcp_ptr -> nx_dhcp_discoveries_received++;
                break;
    
                
            case NX_DHCP_TYPE_DHCPREQUEST:

                /* Are we returning an ACK to the Client? */
                if (dhcp_client_ptr -> nx_dhcp_response_type_to_client == NX_DHCP_TYPE_DHCPACK)
                {

                    _nx_dhcp_server_store_data(buffer + NX_DHCP_OFFSET_YOUR_IP, 4, dhcp_client_ptr -> nx_dhcp_assigned_ip_address);   

                    /* Add the list of options that go out with server ACKs replies. */
                    _nx_dhcp_load_server_options(dhcp_ptr, dhcp_client_ptr, buffer, NX_DHCP_OPTIONS_FOR_REPLY_TO_REQUEST, &index);
                }
    
                /* Increment the number of Request messages received.  */
                dhcp_ptr -> nx_dhcp_requests_received++;
                break;

            /* Note: DHCP Servers should not reply to NX_DHCP_REPLY_TO_RELEASE or NX_DHCP_REPLY_TO_DECLINE messages. */
            case NX_DHCP_TYPE_DHCPRELEASE:
                dhcp_ptr -> nx_dhcp_releases_received++;
                break;
    
            case NX_DHCP_TYPE_DHCPDECLINE:
                dhcp_ptr -> nx_dhcp_declines_received++;
                break;
    
            case NX_DHCP_TYPE_DHCPINFORM:
    
                /* At this time the server sends just standard network parameters to INFORM requests.  
                   so no additional options to add. */
    
                /* Increment the number of Inform messages received.  */
                dhcp_ptr -> nx_dhcp_informs_received++;
                break;
    
            /* No other DHCP messages are supported by NetX DHCP Server at this time. */
            default:
                break;
        }
    
        /* Determine which of the remaining client requested options the server has information for. */
        _nx_dhcp_load_server_options(dhcp_ptr, dhcp_client_ptr, buffer, NX_DHCP_OPTIONS_REQUESTED_BY_CLIENT, &index);       
    }

    /* Setup the packet pointers.  */   
    /* Added the END option.  */
    *(buffer + index) = NX_DHCP_SERVER_OPTION_END;
    index ++;

    /* Check the option length.  */
    if (index > NX_DHCP_OFFSET_END)
    {
        packet_ptr -> nx_packet_length = index;
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + index;
    }
    else
    {   
        packet_ptr -> nx_packet_length = NX_DHCP_OFFSET_END;
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_END;
    }

    /* DHCP Server set the correct destination address and port to send resposne. RFC2131, Section4.1, Page23.  
       1. If the 'giaddr' field in a DHCP message from a client is non-zero, the server sends any return messages to the 'DHCP server' port on the
          BOOTP relay agent whose address appears in 'giaddr'.
       2. If the 'giaddr'field is zero and the 'ciaddr' field is nonzero, then the server unicasts DHCPOFFER and DHCPACK messages to the address in 'ciaddr'.
       3. If 'giaddr' is zero and 'ciaddr' is zero, and the broadcast bit is set, then the server broadcasts DHCPOFFER and DHCPACK messages to 0xffffffff.
       4. If the broadcast bit is not set and 'giaddr' is zero and 'ciaddr' is zero, then the server unicasts DHCPOFFER and DHCPACK messages to the client's hardware address and 'yiaddr' address.
       5. In all cases, when 'giaddr' is zero, the server broadcasts any DHCPNAK messages to 0xffffffff.  */
                                                    
    /* Check the 'giaddr' field. */
    if (dhcp_client_ptr -> nx_dhcp_relay_ip_address)
    {                

        /* Set the destination address and port. */
        destination_ip_address = dhcp_client_ptr -> nx_dhcp_relay_ip_address;
        destination_port = NX_DHCP_SERVER_UDP_PORT;
    }
    else
    {
        /* Check the DHCP response message type.  */
        if (dhcp_client_ptr -> nx_dhcp_response_type_to_client != NX_DHCP_TYPE_DHCPNACK)
        {

            /* Check the 'ciaddr' field*/
            if (dhcp_client_ptr -> nx_dhcp_clientip_address)
            {    
                destination_ip_address = dhcp_client_ptr -> nx_dhcp_clientip_address;
            }
            /* Check the broadcast bit.  */
            else if (dhcp_client_ptr -> nx_dhcp_broadcast_flag_set & NX_DHCP_FLAGS_BROADCAST)
            {                     
                destination_ip_address = NX_DHCP_BC_ADDRESS;
            }
            else
            {                             
                destination_ip_address = dhcp_client_ptr -> nx_dhcp_your_ip_address;       

                /* Add the dynamic arp entry.  */
                status = nx_arp_dynamic_entry_set(dhcp_ptr -> nx_dhcp_ip_ptr, destination_ip_address, dhcp_client_ptr -> nx_dhcp_client_mac_msw, dhcp_client_ptr -> nx_dhcp_client_mac_lsw);

                /* Check the status.  */
                if (status)
                {
                    nx_packet_release(packet_ptr);

                    return(status);
                }
            }
        }
        else
        {                 
            destination_ip_address = NX_DHCP_BC_ADDRESS;
        } 

        /* Set the destination port.  */
        destination_port = NX_DHCP_CLIENT_UDP_PORT;
    }              

#ifdef EL_PRINTF_ENABLE
    EL_PRINTF("DHCPserv: DHCP reply packet destination 0x%x\n", destination_ip_address); 
    EL_PRINTF("DHCPserv: DHCP reply buffer size (bytes) %d\n", packet_ptr -> nx_packet_length); 
    EL_PRINTF("DHCPserv: DHCP reply packet payload size (bytes) %d\n\n", packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_payload_size);
#endif

#ifdef PACKET_DUMP
{

    ULONG *work_ptr;
    ULONG  uword;
    UINT i = 0;
    UINT index = 0;
    UINT length;

#ifdef EL_PRINTF_ENABLE
    EL_PRINTF("DHCPserv: server reply to client - packet dump:\n");
#endif

    work_ptr = (ULONG *)packet_ptr -> nx_packet_prepend_ptr;
    length = ((packet_ptr -> nx_packet_length + sizeof(ULONG) - 1)/sizeof(ULONG));
                     
    while(i < length)
    {
        uword = *work_ptr;

        if (index == 10)
        {

#ifdef EL_PRINTF_ENABLE
            EL_PRINTF("\n");
#endif
            index = 0;
        }

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF(" %08x", uword);
#endif
        work_ptr++;
        index++;
        i++;
    }
#ifdef EL_PRINTF_ENABLE
    EL_PRINTF("\n\n");
#endif
}
#endif  /* #ifdef PACKET_DUMP */

    /* Send the packet.  */
    status =  nx_udp_socket_interface_send(&(dhcp_ptr -> nx_dhcp_socket), packet_ptr, destination_ip_address, destination_port, dhcp_client_ptr -> nx_dhcp_client_iface_index);

    if (status != NX_SUCCESS) 
    {
        nx_packet_release(packet_ptr);
    }

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_client_session                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function clears data from the current client session (e.g. read*/
/*    from the most recent client message. It does remove anything in the */
/*    interface table associated with this client's subnet, nor change the*/
/*    client state.                                                       */
/*                                                                        */ 
/*    Permanent client info such as client host name, client mac address  */
/*    assigned IP address, or lease time is not cleared (left to the      */
/*    caller).                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    dhcp_client_ptr                       Pointer to DHCP Client        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_SUCCESS                            Successful outcome            */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_dhcp_listen_for_messages           Handle Client DHCP messages   */
/*   _nx_dhcp_fast_periodic_timer_entry     Session timer entry function  */
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
static UINT  _nx_dhcp_clear_client_session(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr)
{

UINT i;

    NX_PARAMETER_NOT_USED(dhcp_ptr);

    dhcp_client_ptr -> nx_dhcp_message_type = 0;
    dhcp_client_ptr -> nx_dhcp_response_type_to_client = 0; 
    dhcp_client_ptr -> nx_dhcp_xid = 0;
    dhcp_client_ptr -> nx_dhcp_requested_ip_address = 0x0;                                             
    dhcp_client_ptr -> nx_dhcp_requested_lease_time = 0x0;
    dhcp_client_ptr -> nx_dhcp_your_ip_address = 0x0;                                             
    dhcp_client_ptr -> nx_dhcp_clientip_address = 0x0;                                             
    dhcp_client_ptr -> nx_dhcp_server_id = 0x0; 
    dhcp_client_ptr -> nx_dhcp_clientrec_server_ip = 0x0;                                             
    dhcp_client_ptr -> nx_dhcp_broadcast_flag_set = NX_DHCP_FLAGS_BROADCAST;       
    dhcp_client_ptr -> nx_dhcp_session_timeout = 0;
    dhcp_client_ptr -> nx_dhcp_destination_ip_address = 0; 
    dhcp_client_ptr -> nx_dhcp_source_ip_address = 0; 

    /* Clear out the option data. */
    dhcp_client_ptr -> nx_dhcp_client_option_count = 0;    
    for (i = 0; i < NX_DHCP_CLIENT_OPTIONS_MAX; i++)
    {
         dhcp_client_ptr -> nx_dhcp_user_options[0] = 0;
    }

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcp_clear_client_record                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the clear client record   */
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    dhcp_client_ptr                       Pointer to DHCP Client        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_SUCCESS                            Successful outcome            */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_client_record     Actual clear client record service */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   Host Application                                                     */
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
UINT  _nxe_dhcp_clear_client_record(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr)
{

UINT status;


    /* Check for invalid input.  */
    if ((dhcp_ptr == NX_NULL) || (dhcp_client_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCP clear client record service.  */
    status =  _nx_dhcp_clear_client_record(dhcp_ptr, dhcp_client_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_client_record                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function clears the entire client record from the server table.*/
/*    It also frees up the client's IP address in the server IP address   */
/*    table.                                                              */
/*                                                                        */ 
/*    The DHCP server does not use this function, it is strictly for the  */
/*    host application to remove clients from the table (e.g. to free up  */
/*    room for new clients by removing old 'stale' client entries no      */
/*    longer configured with the server.                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    dhcp_client_ptr                       Pointer to DHCP Client        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_SUCCESS                            Successful outcome            */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcp_clear_ip_address_owner     Clear IP address owner          */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   Host Application                                                     */
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
UINT  _nx_dhcp_clear_client_record(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr)
{

UINT                     i;
NX_DHCP_INTERFACE_TABLE  *iface_table_ptr;


    if (dhcp_client_ptr == 0x0)
    {
        /* Benign error. Return success. */
        return(NX_SUCCESS);
    }

    /* Obtain DHCP Server mutex protection,. */
    tx_mutex_get(&dhcp_ptr -> nx_dhcp_mutex, NX_WAIT_FOREVER);

    /* Set local pointer for convenience. */
    iface_table_ptr = &dhcp_ptr -> nx_dhcp_interface_table[dhcp_client_ptr -> nx_dhcp_client_iface_index];

    i = 0;

    /* Does the client have an assigned IP address? */
    if (iface_table_ptr -> nx_dhcp_ip_address_list[i].nx_assignable_ip_address)
    {
    
        /* Yes, We need to free up the Client's assigned IP address in the server database. */
        while (i < iface_table_ptr -> nx_dhcp_address_list_size)
        {
    
            /* Find the interface table entry by matching IP address. */
            if (iface_table_ptr -> nx_dhcp_ip_address_list[i].nx_assignable_ip_address == 
                dhcp_client_ptr -> nx_dhcp_assigned_ip_address)
            {

                /* Clear the owner information.  */
                _nx_dhcp_clear_ip_address_owner(&(iface_table_ptr -> nx_dhcp_ip_address_list[i]));

                /* Address now available for DHCP client. */
                break;
            }
            i++;
        }
    }

    /* Ok to clear the Client record. */
    memset(dhcp_client_ptr, 0, sizeof(NX_DHCP_CLIENT));

    /* Update the total number of dhcp clients. */
    if (dhcp_ptr -> nx_dhcp_number_clients > 0)
    {

        dhcp_ptr -> nx_dhcp_number_clients--;
    } 

    /* Release DHCP Server mutex.  */
    tx_mutex_put(&dhcp_ptr -> nx_dhcp_mutex);

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_load_server_options                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function adds the specified options in the server's DHCP option  */
/*  list to the DHCP server response to the Client message. There is a    */
/*  required set of options that go out with all server messages, as well */
/*  as a standard (user configurable) set of options for all 'ACK'        */
/*  messages.  Lastly, there are certain options that are specific to the */
/*  client message request e.g. RENEW vs OFFER etc                        */
/*                                                                        */
/*  There is no support for responding to specific client options in this */
/*  revision.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                            Pointer to DHCP Server          */
/*    dhcp_client_ptr                     Pointer to client session record*/
/*    buffer                              Packet buffer to load options to*/
/*    option_type                         Category of options to load     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Successful completion status   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_add_option                 Add option to response (specify */
/*                                                 option and data to add)*/ 
/*    _nx_dhcp_add_requested_option       Add option to response          */
/*                                            (specify option and client  */
/*                                            interface)                  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_respond_to_dhcp_message      Create and send response back */ 
/*                                               DHCP Client              */
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
static UINT  _nx_dhcp_load_server_options(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr, UCHAR *buffer, UINT option_type, UINT *index)
{

UINT i;
UINT iface_index;
UINT renew_time; 
UINT rebind_time; 
UINT lease_time; 


    /* Compute default lease, renew and rebind times to offer the client. */
    lease_time = dhcp_client_ptr -> nx_dhcp_requested_lease_time;

#if (NX_DHCP_DEFAULT_LEASE_TIME >= 0xFFFFFFFF)
    if (lease_time == 0)
#else
    if ((lease_time == 0) || (lease_time > NX_DHCP_DEFAULT_LEASE_TIME))
#endif
    {
        lease_time = NX_DHCP_DEFAULT_LEASE_TIME;
    }

    renew_time = lease_time/2; 
    rebind_time = 7 * (lease_time/8); 

    /* Set a local pointer for convenience. */
    iface_index = dhcp_client_ptr -> nx_dhcp_client_iface_index;

    /* Determine what set of options to apply based on caller input. */
    if (option_type == NX_DHCP_OPTIONS_FOR_ALL_REPLIES)
    {

        /* This is the generic set of options for ALL server replies. */

        /* Set the DHCP message type the server is sending back to client.  */
        _nx_dhcp_add_option(buffer, NX_DHCP_SERVER_OPTION_DHCP_TYPE, NX_DHCP_SERVER_OPTION_DHCP_TYPE_SIZE, 
                            dhcp_client_ptr -> nx_dhcp_response_type_to_client, index);

        /* Add the server identifier to all messages to the client. */
        _nx_dhcp_add_option(buffer, NX_DHCP_SERVER_OPTION_DHCP_SERVER_ID, NX_DHCP_SERVER_OPTION_DHCP_SERVER_SIZE, 
                            dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_server_ip_address, index);

        return(NX_SUCCESS); 
    }
    else if (option_type == NX_DHCP_OPTIONS_FOR_REPLY_TO_OFFER)
    {

        /* Offer an IP lease. */
        _nx_dhcp_add_option(buffer, NX_DHCP_SERVER_OPTION_DHCP_LEASE, NX_DHCP_SERVER_OPTION_DHCP_LEASE_SIZE, lease_time, index);
        /* With the computed renew time. */
        _nx_dhcp_add_option(buffer, NX_DHCP_SERVER_OPTION_RENEWAL, NX_DHCP_SERVER_OPTION_RENEWAL_SIZE, renew_time, index);
        /* And rebind time. */
        _nx_dhcp_add_option(buffer, NX_DHCP_SERVER_OPTION_REBIND, NX_DHCP_SERVER_OPTION_REBIND_SIZE, rebind_time, index);
    }
    else if (option_type == NX_DHCP_OPTIONS_FOR_REPLY_TO_REQUEST)
    {

        /* Confirm the assigned IP lease, renew and rebind times. */
        _nx_dhcp_add_option(buffer, NX_DHCP_SERVER_OPTION_DHCP_LEASE, NX_DHCP_SERVER_OPTION_DHCP_LEASE_SIZE, lease_time, index);
        _nx_dhcp_add_option(buffer, NX_DHCP_SERVER_OPTION_RENEWAL, NX_DHCP_SERVER_OPTION_RENEWAL_SIZE, renew_time, index);
        _nx_dhcp_add_option(buffer, NX_DHCP_SERVER_OPTION_REBIND, NX_DHCP_SERVER_OPTION_REBIND_SIZE, rebind_time, index);
    }
    else if (option_type == NX_DHCP_OPTIONS_FOR_REPLY_TO_INFORM)
    {

        /* The NetX DHCP Server reply to Inform messages includes the standard server option 
           list which is automatically included. */        
    }
    else if (option_type == NX_DHCP_OPTIONS_FOR_GENERIC_ACK)
    {

        /* Add the standard DHCP server information (e.g. subnet, router IP etc) which are
           appended to the required server options. */
        for (i= NX_DHCP_REQUIRED_SERVER_OPTION_SIZE; i < dhcp_ptr -> nx_dhcp_server_option_count; i++)
        {

            /* Append this DHCP option to the client message. */
            _nx_dhcp_add_requested_option(dhcp_ptr,  dhcp_client_ptr -> nx_dhcp_client_iface_index,
                                                   buffer, dhcp_ptr -> nx_dhcp_server_options[i], index);
        }
    }
    else if (option_type == NX_DHCP_OPTIONS_REQUESTED_BY_CLIENT)
    {

        /* The NetX DHSP Server does not (yet) support this feature in the current release. */       
        
        /* Clear the current DHCP Client's requested option list. */
        for (i = 0; i < dhcp_client_ptr -> nx_dhcp_client_option_count; i++)
        {
    
            dhcp_client_ptr -> nx_dhcp_user_options[i] = 0;
        }
    
        dhcp_client_ptr -> nx_dhcp_client_option_count = 0;
    }

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_set_default_server_options                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function parses a set of options from the supplied buffer into   */
/*  the server's list of options to supply data to the DHCP Client. If the*/
/*  server is configured to use default option data, it will use this list*/
/*  of options to compose the response to the Client.                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                             Pointer to DHCP Server         */ 
/*    buffer                               Pointer to option list         */ 
/*    size                                 Size of option list buffer     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*    status                                Actual error status           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_server_create               Create the DHCP Server instance*/
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
static UINT  _nx_dhcp_set_server_options(NX_DHCP_SERVER *dhcp_ptr, CHAR *buffer, UINT buffer_size)
{

UINT j;
UINT digit;
UINT status;
CHAR *work_ptr;


    j = 0;
    /* Search through the text for all options to load. */
    while(buffer_size && (j < NX_DHCP_SERVER_OPTION_LIST_SIZE)) 
    {

        /* Check if we're off the end of the buffer. */
        if (buffer == 0x0)
        {
            break;
        }

        work_ptr = buffer;

        /* This should advance the buffer past the digit.*/
        status = _nx_dhcp_parse_next_option(&buffer, &digit, buffer_size);
        if (status)
        {

#ifdef EL_PRINTF_ENABLE
            EL_PRINTF("DHCPserv: Unable to set server DHCP option data list. Status 0x%x\n", status);
#endif

            return(status);
        }

        buffer_size -= (UINT)(buffer - work_ptr);

        /* Load the parsed option into the server 'option list.' */
        dhcp_ptr -> nx_dhcp_server_options[j] = digit;
        j++;
    }

    /* Update the number of server options in the list. */
    dhcp_ptr -> nx_dhcp_server_option_count = j;

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_parse_next_option                          PORTABLE C      */ 
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function parses space or comma separated numeric data from the   */
/*  supplied buffer, for example, the server option list.                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    option_list                           Pointer to buffer to parse    */ 
/*    option                                Data parsed from buffer       */ 
/*    length                                Size of option list buffer    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*    NX_DHCP_INTERNAL_OPTION_PARSE_ERROR   Parsing error status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_utility_string_to_uint           Convert ascii number to integer*/
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_set_default_server_options  Creates the server option list */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            obsolescent functions,      */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dhcp_parse_next_option(CHAR **option_list, UINT *option, UINT length)
{

CHAR c;
UINT j;
UINT buffer_index;
CHAR num_string[3];  
CHAR *buffer;


    /* Check for invalid input. */
    if ((option_list == 0x0) || (option == 0) || (length == 0))
    {
        return(NX_DHCP_INTERNAL_OPTION_PARSE_ERROR);
    }

    buffer = *option_list;
    memset(&num_string[0], 0, 3);

    /* Initialize parsed option to undefined option (sorry, zero is taken by the PAD option
       which I personally think was a bad programming decision). */
    *option = 999;
    j = 0;
    buffer_index = 0;

    /* Get the first character in the buffer. */
    c = *buffer++;

    /* Get the first digit (non separator character)*/
    while ((c == ' ') || (c == ','))
    {
    
        c = *buffer++;
        j++;
    }

    /* Now parse the next characters until the end of the buffer or next separator character. */
    while ((j < length) && (buffer_index < 3)) 
    {

        /* Check for separator marking the end of the option string. */
        if ((c == ' ') || (c == ',') || (c == 0x0))
            break;

        /* Check for an invalid digit or out of range option. */
        if (c < 0x30 || c > 0x39 || buffer_index > 2)
        {
            return(NX_DHCP_BAD_OPTION_LIST_ERROR);
        }

        num_string[buffer_index] = c;
        j++;

        /* Append the character ('digit') into a number buffer. */
        c = *buffer++;
        buffer_index++;
    }

    *option_list += j;

    /* Convert the number string to an actual unsigned integer. */
    return(_nx_utility_string_to_uint(num_string, buffer_index, option));
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_packet_process                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes a Client DHCP message and extracts          */
/*    information from a client DHCP message to create or update the      */
/*    client record.  It handles the DHCP message based on type, sets     */
/*    a timer on the client record if a response is expected from the     */
/*    client, and waits for another DHCP packet.                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Message handled successfully   */ 
/*    NO_PACKET                            No packet received             */
/*    NX_DHCP_SERVER_BAD_INTERFACE_INDEX   Packet interface not recognized*/
/*    status                               Actual completion outcome      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release DHCP packet           */ 
/*    nx_udp_socket_receive                 Receive DHCP packet           */ 
/*    _nx_dhcp_server_extract_information   Extract DHCP packet info      */ 
/*    _nx_dhcp_validate_client_message      Process the Client message    */
/*    _nx_dhcp_server_assign_ip_address     Assign IP address to Client   */
/*    _nx_dhcp_respond_to_client_message    Create and send response back */
/*    _nx_dhcp_clear_client_session         Clears client session data    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_server_thread_entry          DHCP Server thread task       */ 
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
static UINT  _nx_dhcp_server_packet_process(NX_DHCP_SERVER *dhcp_ptr, NX_PACKET *packet_ptr)
{

UINT            status;
UINT            iface_index;
NX_DHCP_CLIENT  *dhcp_client_ptr = NX_NULL;
NX_PACKET       *new_packet_ptr;   
ULONG           bytes_copied = 0;
ULONG           offset;

#ifdef EL_PRINTF_ENABLE
    EL_PRINTF("\n");
    EL_PRINTF("DHCPserv: Waiting to receive next packet\n");
#endif

    /* Check for minimum sized DHCP packet (e.g. just the DHCP header and 0 bytes of option data). */
    if (packet_ptr -> nx_packet_length <  NX_DHCP_HEADER_SIZE) 
    {
#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: Client DHCP packet is malformed or empty. Invalid DHCP packet.\n");
#endif              

        /* No; Release the original packet. */
        nx_packet_release(packet_ptr);

        /* Return successful completion status. */
        return(NX_DHCP_MALFORMED_DHCP_PACKET);
    }
                       
    /* Get the interface index.  */
     nx_udp_packet_info_extract(packet_ptr, NX_NULL, NX_NULL, NX_NULL, &iface_index);

    /* Does the DHCP server have a table for this packet interface? */
    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: No interface found for DHCP packet\n");
#endif
                                      
        /* Release the original packet. */
        nx_packet_release(packet_ptr);

        /* No, return the error status. */
        return(NX_DHCP_SERVER_BAD_INTERFACE_INDEX);
    }
          
    /* We will copy the received packet (datagram) over to a packet from the DHCP Server pool and release
       the packet from the receive packet pool as soon as possible. */
    status =  nx_packet_allocate(dhcp_ptr -> nx_dhcp_packet_pool_ptr, &new_packet_ptr, NX_IPv4_UDP_PACKET, NX_DHCP_PACKET_ALLOCATE_TIMEOUT);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
                                      
        /* Release the original packet. */
        nx_packet_release(packet_ptr);

        /* Error allocating packet, return error status.  */
        return(status);
    }

    /* Verify the incoming packet does not exceed our DHCP Server packet payload. */
    if ((ULONG)(new_packet_ptr -> nx_packet_data_end - new_packet_ptr -> nx_packet_prepend_ptr) < (packet_ptr -> nx_packet_length))
    {

        /* Release the original packet. */
        nx_packet_release(packet_ptr);

        /* Release the newly allocated packet and return an error. */
        nx_packet_release(new_packet_ptr);

        return(NX_DHCP_INADEQUATE_PACKET_POOL_PAYLOAD);
    }
    
    /* Update the prepend pointer to make sure that the IP header and UDP header also are copied into new packet. */
    packet_ptr -> nx_packet_prepend_ptr -= 28;
    packet_ptr -> nx_packet_length += 28;

    /* Initialize the offset to the beginning of the packet buffer. */ 
    offset = 0;
    status = nx_packet_data_extract_offset(packet_ptr, offset, (VOID *)new_packet_ptr -> nx_packet_prepend_ptr, packet_ptr -> nx_packet_length, &bytes_copied);

    /* Check status.  */
    if ((status != NX_SUCCESS) || (bytes_copied != packet_ptr -> nx_packet_length))
    {
                                    
        /* Release the original packet. */
        nx_packet_release(packet_ptr);

        /* Release the allocated packet we'll never send. */
        nx_packet_release(new_packet_ptr);

        /* Error extracting packet buffer, return error status.  */
        return(status);
    }       

    /* Update the new packet with the bytes copied, and removed.  
       For chained packets, this will reflect the total 'datagram' length. */
    new_packet_ptr -> nx_packet_prepend_ptr += 28;
    new_packet_ptr -> nx_packet_length = bytes_copied - 28;
    new_packet_ptr -> nx_packet_append_ptr = new_packet_ptr -> nx_packet_prepend_ptr + new_packet_ptr -> nx_packet_length;

    /* Now we can release the original packet. */
    nx_packet_release(packet_ptr);

    /* Extract the DHCP specific information from the packet. This will create new record or update existing
       client record in the server database. */
    status = _nx_dhcp_server_extract_information(dhcp_ptr, &dhcp_client_ptr, new_packet_ptr, iface_index);

#ifdef PACKET_DUMP
{

    ULONG *work_ptr;
    ULONG  uword;
    UINT i = 0;
    UINT index = 0;
    UINT length;
    
#ifdef EL_PRINTF_ENABLE
    EL_PRINTF("DHCPserv: received client packet dump:\n");
#endif
                  
    /* Get a pointer to the IP header and adjust the packet length for IP and UDP header size. */
    work_ptr = (ULONG *)(packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_UDP_HEADER) - sizeof(NX_IPV4_HEADER));
    length = ((packet_ptr -> nx_packet_length + sizeof(ULONG) - 1)/sizeof(ULONG)) + sizeof(NX_UDP_HEADER) + sizeof(NX_IPV4_HEADER);

    while(i < length)
    {
        uword = *work_ptr;

        if (index == 10)
        {

#ifdef EL_PRINTF_ENABLE
            EL_PRINTF("\n");
#endif
            index = 0;
        }
#ifdef EL_PRINTF_ENABLE
        EL_PRINTF(" %08x", uword);
#endif
        work_ptr++;
        index++;
        i++;
    }
#ifdef EL_PRINTF_ENABLE
    EL_PRINTF("\n\n");
#endif
}
#endif  /* #ifdef PACKET_DUMP */

    /* We are done with this packet now regardless of the success of the above operations. */
    nx_packet_release(new_packet_ptr);

    /* Check if we have an invalid client record. */
    if ((status != NX_SUCCESS) && dhcp_client_ptr)
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: Error extracting DHCP packet (invalid): 0x%x\n", status);
#endif

       /* Client record was created but there was a problem with the packet
         or client message. Since we're not sure what got written to the Client record,
         remove packet data out completely. */
        _nx_dhcp_clear_client_record(dhcp_ptr, dhcp_client_ptr);
    }

    /* Check for general errors or failure to produce a client record. */
    if ((status != NX_SUCCESS) || !dhcp_client_ptr)
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: Internal error extracting DHCP packet: 0x%x\n", status);
#endif

        /* Return the error from extracting packet data. */
        return(status);
    }

    /* Clear any previous response type in the client record. */
    dhcp_client_ptr -> nx_dhcp_response_type_to_client = 0;

    /* Validate the client message including how the server should respond to it. */
    status = _nx_dhcp_validate_client_message(dhcp_ptr, dhcp_client_ptr);

    /* Check for errors. */
    if (status != NX_SUCCESS)
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: DHCP packet failed validation: 0x%x\n", status);
#endif

        /* Return all other errors as completion status. Do not send a message to the client. */
        return(status);
    }
  
    /* Assign/confirm the assigned IP address of the Client if the server is not returning
       a NACK, responding to an INFORM message, or not responding at all. */
    if ((dhcp_client_ptr -> nx_dhcp_response_type_to_client != NX_DHCP_TYPE_DHCPNACK) &&
        (dhcp_client_ptr -> nx_dhcp_response_type_to_client != NX_DHCP_TYPE_DHCPSILENT) &&
        (dhcp_client_ptr -> nx_dhcp_message_type != NX_DHCP_TYPE_DHCPINFORM))
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: assigning IP address to DHCP client...\n");
#endif

        /* Assign/confirm an IP address to the client. If the client is renewing or rebinding
           this will verify they sent the correct IP address and reset the lease time. */ 
        status = _nx_dhcp_server_assign_ip_address(dhcp_ptr, dhcp_client_ptr);

        /* Has the server run out of available addresses? Handle this 'error' 
          condition separately.  */
        if (status == NX_DHCP_NO_AVAILABLE_IP_ADDRESSES)
        {

#ifdef EL_PRINTF_ENABLE
            EL_PRINTF("DHCPserv: Unable to assign IP address; none available\n");
#endif

            /* Actually as yet, there is no callback to the host application 
               for handling this situation. 

               The server should still send a NACK to the Client that it cannot 
               provide what it requests. */
        }
        /* Check for errors other than running out of available IP addresses. */
        else if (status != NX_SUCCESS)
        {

#ifdef EL_PRINTF_ENABLE
            EL_PRINTF("DHCPserv: Unable to assign IP address. Status: 0x%x\n", status);
#endif

            /* Return all other errors as completion status. Do not send a message to the client. */
            return(status);
        }
    }

#ifdef EL_PRINTF_ENABLE
    if (dhcp_client_ptr -> nx_dhcp_response_type_to_client == NX_DHCP_TYPE_DHCPOFFER)
        EL_PRINTF("DHCPserv: response to client: OFFER\n");
    else if (dhcp_client_ptr -> nx_dhcp_response_type_to_client == NX_DHCP_TYPE_DHCPACK)
        EL_PRINTF("DHCPserv: response to client: ACK\n");
    else if (dhcp_client_ptr -> nx_dhcp_response_type_to_client == NX_DHCP_TYPE_DHCPNACK)
        EL_PRINTF("DHCPserv: response to client: NACK\n");
    else if (dhcp_client_ptr -> nx_dhcp_response_type_to_client == NX_DHCP_TYPE_DHCPSILENT)
        EL_PRINTF("DHCPserv: response to client: SILENT (no response)\n");
#endif
     

    /* Is there a response to send to the Client? */
    if (dhcp_client_ptr -> nx_dhcp_response_type_to_client != NX_DHCP_TYPE_DHCPSILENT)
    {

        /* Yes; use the session data in the client record to compose the server reply. */
        status = _nx_dhcp_respond_to_client_message(dhcp_ptr, dhcp_client_ptr); 

        /* Check for error. */
        if (status != NX_SUCCESS)
        {

#ifdef EL_PRINTF_ENABLE
            EL_PRINTF("DHCPserv: Error sending response to DHCP client. Status: 0x%x\n", status);
#endif

            /* Return error completion status. Error sending a message to client. */
            return(status);
        }

        /* Update the client state if an ACK was sent successfully. */
        if (dhcp_client_ptr -> nx_dhcp_response_type_to_client == NX_DHCP_TYPE_DHCPACK)
        {

            /* Was the client was in a requesting, renewing or rebinding state? */
            if ((dhcp_client_ptr -> nx_dhcp_client_state == NX_DHCP_STATE_RENEWING)   ||
                (dhcp_client_ptr -> nx_dhcp_client_state == NX_DHCP_STATE_REBINDING)  ||
                (dhcp_client_ptr -> nx_dhcp_client_state == NX_DHCP_STATE_REQUESTING))
            {

                /* Yes, the client should now be bound to an IP address. */
                dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_BOUND;
            }
        }
    }

    /* 
       If the Client is BOUND, the session is over. Clear the session including 
       the sesison timeout. 

       If the Client is still in the BOOT or INIT state something was wrong with their 
       message or else the server had a problem.  Clear the record and they
       can start a new session with a retransmission of their request. 
    */

    /* Do we need to clear the session data depending? */
    if (dhcp_client_ptr -> nx_dhcp_client_state == NX_DHCP_STATE_BOOT ||
        dhcp_client_ptr -> nx_dhcp_client_state == NX_DHCP_STATE_INIT ||
        dhcp_client_ptr -> nx_dhcp_client_state == NX_DHCP_STATE_BOUND)
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: client session completed. Clear session data.\n");
#endif

        /* Yes, clear all data from the session. Client assigned IP address
           and hardware address are not cleared.*/
        _nx_dhcp_clear_client_session(dhcp_ptr,dhcp_client_ptr);
    }
    else
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: Waiting for DHCP client response...\n");
#endif

       /* No; assume the Client is Renewing, rebinding, or selecting, 
          to set the session timeout to await the next Client response;
          (not finished with this DHCP session yet). */

        /* Assume we have received at least one message from the Client, so 
           reset the timer on the client session timeout. */
        dhcp_client_ptr -> nx_dhcp_session_timeout = NX_DHCP_CLIENT_SESSION_TIMEOUT;
    }


/* Output Server session data if enabled.  */
#ifdef TESTOUTPUT
    add_client++;

    /* Only print out the session table depending on trace interval. */
    if ((TRACE_NTH_CLIENT_PACKET > 0) && (add_client % TRACE_NTH_CLIENT_PACKET == 0))     
    {
    
        UINT i;
    
#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("\nClient\t\tAssigned\tLease\t\tSession\tState\n");
#endif
        for (i = 0; i < dhcp_ptr -> nx_dhcp_number_clients; i++)
        {
            NX_DHCP_CLIENT *client_ptr;
    
            client_ptr = &(dhcp_ptr -> client_records[i]);
    
#ifdef EL_PRINTF_ENABLE
            EL_PRINTF("0x%x 0x%x\t0x%08x\t0x%08x\t%d\t%d\n",
                   client_ptr -> nx_dhcp_client_mac_msw,
                   client_ptr -> nx_dhcp_client_mac_lsw,
                   client_ptr -> nx_dhcp_assigned_ip_address,
                   client_ptr -> nx_dhcp_requested_lease_time,
                   client_ptr -> nx_dhcp_session_timeout,
                   client_ptr -> nx_dhcp_client_state);
#endif
        }
    
#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("\nHost\tIP address\tAssigned?  Lease\n");
        EL_PRINTF("Interface 0\n");
        for (i = 0; i < dhcp_ptr -> nx_dhcp_interface_table[0].nx_dhcp_address_list_size; i++)
        {
            NX_DHCP_INTERFACE_IP_ADDRESS *interface_address_ptr;
    
            interface_address_ptr = &(dhcp_ptr -> nx_dhcp_interface_table[0].nx_dhcp_ip_address_list[i]);
            EL_PRINTF("0x%x 0x%x\t0x%x\t%d\t  0x%x\n",
                   interface_address_ptr -> owner_mac_msw,
                   interface_address_ptr -> owner_mac_lsw,
                   interface_address_ptr -> nx_assignable_ip_address,
                   interface_address_ptr -> assigned,
                   interface_address_ptr -> lease_time);
        }
    
        EL_PRINTF("\nInterface 1\n");
        for (i = 0; i < dhcp_ptr -> nx_dhcp_interface_table[1].nx_dhcp_address_list_size; i++)
        {
            NX_DHCP_INTERFACE_IP_ADDRESS *interface_address_ptr;
    
            interface_address_ptr = &(dhcp_ptr -> nx_dhcp_interface_table[1].nx_dhcp_ip_address_list[i]);
            EL_PRINTF("0x%x 0x%x\t0x%x\t%d\t  0x%x\n",
                   interface_address_ptr -> owner_mac_msw,
                   interface_address_ptr -> owner_mac_lsw,
                   interface_address_ptr -> nx_assignable_ip_address,
                   interface_address_ptr -> assigned,
                   interface_address_ptr -> lease_time);
        }
#endif
    }
#endif

    /* Return actual outcome status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_find_client_record_by_ip_address           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function looks up a client record using the client's last known*/
/*    assigned IP address.                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP server        */ 
/*    dhcp_client_ptr                       Pointer to DHCP client        */
/*    iface_index                           Client network interface index*/
/*    assigned_ip_address                   Client's assigned IP address  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Successful completion          */ 
/*    NX_DHCP_CLIENT_RECORD_NOT_FOUND      Client record not found in     */
/*                                             Server database            */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_client_record         Remove Client record from      */
/*                                             Server database            */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_slow_periodic_timer_entry    Update IP lease time outs     */ 
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
UINT  _nx_dhcp_find_client_record_by_ip_address(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT **dhcp_client_ptr, 
                                               UINT iface_index, ULONG assigned_ip_address)
{

UINT            i;
NX_DHCP_CLIENT  *client_record_ptr;


    /* Initialize the search results to unsuccessful. */
    *dhcp_client_ptr = NX_NULL;

    i = 0;

    /* Records are not necessarily added and deleted sequentially, 
       so search the whole table until a match is found. */
    while (i < NX_DHCP_CLIENT_RECORD_TABLE_SIZE) 
    {
        /* Set local pointer for convenience. */
        client_record_ptr = &dhcp_ptr -> client_records[i];

        /* Check the mac address for a match. */
        if (client_record_ptr -> nx_dhcp_assigned_ip_address == assigned_ip_address)
        {

            /* Verify the client packet interface matches the interface on record. */
            if (client_record_ptr -> nx_dhcp_client_iface_index == iface_index)
            {
                /* Return the client record location. */
                *dhcp_client_ptr = client_record_ptr;

                return(NX_SUCCESS);
            }
            else
            {

                /* It appears the client has changed its subnet location but not hardware type e.g. 
                   same mac address/hardware type but not interface. Clear the old record. */

                /* This will remove the client record both in the
                   server's client table and in the IP address database. */
                _nx_dhcp_clear_client_record(dhcp_ptr, client_record_ptr);

                /* Search through the rest of the server records for
                   another instance of this client. Either way, if not found
                   with the expected interface a null pointer is returned,
                   or new record created depending on the caller. */
            }
        }
        i++;
    }

    return(NX_DHCP_CLIENT_RECORD_NOT_FOUND);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_find_client_record_by_chaddr               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function looks up a client record by the client hardware mac   */
/*    address.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    iface_index                           Client interface index        */
/*    client_mac_msw                        MSB of client hardware address*/
/*    client_mac_lsw                        LSB of client hardware address*/
/*    dhcp_client_ptr                       Pointer to client record entry*/
/*    add_on                                Option to add if not found    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Message handled successfully   */ 
/*    NX_DHCP_CLIENT_TABLE_FULL            No more room in Client table   */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_client_record          Removes client record from    */
/*                                              server table              */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_server_extract_information    Extract DHCP info from Client */ 
/*                                             message                    */
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
static UINT  _nx_dhcp_find_client_record_by_chaddr(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, ULONG client_mac_msw, 
                                 ULONG client_mac_lsw, NX_DHCP_CLIENT **dhcp_client_ptr, UINT add_on)
{

UINT            i;
UINT            available_index;
NX_DHCP_CLIENT  *client_record_ptr;


    /* Initialize the search results to unsuccessful. */
    *dhcp_client_ptr = NX_NULL;

    /* Initialize an available slot in the table as outside the boundary (e.g. no slots available). */
    available_index = NX_DHCP_CLIENT_RECORD_TABLE_SIZE;

    /* Records are not necessarily added and deleted sequentially, 
       so search the whole table until a match is found. */
    i = 0;
    while (i < NX_DHCP_CLIENT_RECORD_TABLE_SIZE) 
    {

        /* Set local pointer for convenience. */
        client_record_ptr = &dhcp_ptr -> client_records[i];

        /* Skip empty records. Assume a client with no mac address is empty. */
        if ((client_record_ptr -> nx_dhcp_client_mac_msw == 0) && (client_record_ptr -> nx_dhcp_client_mac_lsw == 0))
        {

            /* Flag the first empty record in case we need to add the current client to the table. */
            if (i < available_index)
                available_index = i;

            i++;
            continue;
        }

        /* Check the mac address of each record for a match. */
        if ((client_record_ptr -> nx_dhcp_client_mac_msw == client_mac_msw) &&
            (client_record_ptr -> nx_dhcp_client_mac_lsw == client_mac_lsw))
        {

            /* Verify the client packet interface matches the interface on record. */
            if (client_record_ptr -> nx_dhcp_client_iface_index == iface_index)
            {
                /* Return the client record location. */
                *dhcp_client_ptr = client_record_ptr;

                return(NX_SUCCESS);
            }
            else
            {

                /* It appears the client has changed its location (subnet) but not hardware type e.g. same mac 
                   address/hware type but not interface. Remove the client record entirely and
                   free up any assigned IP address in the server database. */
                _nx_dhcp_clear_client_record(dhcp_ptr, client_record_ptr);

                /* Continue searching through the rest of the server records for
                   another instance of this client. Either way, if not found
                   with the expected interface a null pointer is returned,
                   or new record created depending on the caller. */
            }
        }

        i++;
    }

    /* Not found. Create a record for this client? */
    if (add_on == NX_FALSE)
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: Client HW address not found. Do not add to server database\n");
#endif

        /* No, we're done then. */
        return(NX_SUCCESS);
    }

    /* Check if there is available room in the table for a new client. */
    if (available_index >= NX_DHCP_CLIENT_RECORD_TABLE_SIZE)
    {

        /* No, we cannot add this client so the server's table. */
        return(NX_DHCP_CLIENT_TABLE_FULL);
    }

    /* Set local pointer to an available slot. */
    client_record_ptr = &dhcp_ptr -> client_records[available_index];

    /* Add this client to the server's total number of clients. */
    dhcp_ptr -> nx_dhcp_number_clients++;

    /* Add the supplied information to the new client record. */    
    client_record_ptr -> nx_dhcp_client_mac_msw = client_mac_msw;
    client_record_ptr -> nx_dhcp_client_mac_lsw = client_mac_lsw;
    client_record_ptr -> nx_dhcp_client_iface_index = iface_index;

    /* Initialize the client state as the init state. */
    client_record_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;

    /* Return the location of the newly created client record. */
    *dhcp_client_ptr = client_record_ptr; 

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_find_interface_table_ip_address            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function looks up a table entry by IP address in the specified */
/*    server interface table. If not found, a NULL entry pointer is       */
/*    returned but successful search completion.                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    iface_index                           Network interface index       */
/*    ip_address                            IP address to look up         */
/*    return_interface_address              Pointer to table entry        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Message handled successfully   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_client_record          Removes client record from    */
/*                                              server table              */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_server_extract_information   Extract DHCP info from Client */ 
/*                                             message                    */
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
static UINT  _nx_dhcp_find_interface_table_ip_address(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, ULONG ip_address, 
                                              NX_DHCP_INTERFACE_IP_ADDRESS **return_interface_address)
{

UINT i;
NX_DHCP_INTERFACE_TABLE *dhcp_interface_table_ptr;         


    /* Initialize search results to NULL (not found). */
    *return_interface_address = NX_NULL;

    /* Set a local varible to the IP address list for this client. */
    dhcp_interface_table_ptr = &(dhcp_ptr -> nx_dhcp_interface_table[iface_index]);

    /* Yes, search for an available ip address in the IP list for this interface */
    for(i = 0; i < dhcp_interface_table_ptr -> nx_dhcp_address_list_size; i++)
    {

        /* Is this address a match? */
        if (dhcp_interface_table_ptr -> nx_dhcp_ip_address_list[i].nx_assignable_ip_address == ip_address)
        {
            /* Yes, set a pointer to the location and return. */
            *return_interface_address =  &dhcp_interface_table_ptr -> nx_dhcp_ip_address_list[i];

            /* And we're done! */
            return(NX_SUCCESS);
        }
    }

    /* Not found, so return null pointer and successful search status. */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_update_assignable_ip_address               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the IP address status in the server interface */
/*    table address for the current client. This will return an error     */
/*    status if the IP address being updated is not owned by the client.  */
/*                                                                        */ 
/*    Note: No changes are made to the associated client record.          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    dhcp_client_ptr                       Pointer to DHCP client        */
/*    ip_address                            IP address to update          */
/*    assign_status                         Status to assign              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Entry updated successfully     */ 
/*    NX_DHCP_IP_ADDRESS_NOT_FOUND         IP address not found           */
/*    NX_DHCP_IP_ADDRESS_ASSIGNED_TO_OTHER IP address not assigned to     */
/*                                                  current client        */
/*    NX_DHCP_INVALID_UPDATE_ADDRESS_CMD   Unknown status input           */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_find_interface_table_ip_address                            */
/*                                        Find IP address in server       */
/*                                              interface table           */
/*    _nx_dhcp_find_ip_address_owner      Verify Client is address owner  */
/*    _nx_dhcp_clear_ip_address_owner     Clear IP address owner          */
/*    _nx_dhcp_record_ip_address_owner    Sets Client as IP address owner */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_validate_client_message         Process DHCP Client message*/ 
/*    _nx_dhcp_fast_periodic_timer_entry       Timer on client sessions   */
/*    _nx_dhcp_slow_periodic_timer_entry       Timer on IP address leases */
/*                                             message                    */
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
static UINT  _nx_dhcp_update_assignable_ip_address(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr, 
                                           ULONG ip_address, UINT assign_status) 
{

UINT iface_index; 
UINT assigned_to_client;
UINT lease_time;
NX_DHCP_INTERFACE_IP_ADDRESS    *interface_address_ptr;


    /* Create a local variable for convenience. */
    iface_index = dhcp_client_ptr -> nx_dhcp_client_iface_index;

    /* Look up the IP address in the server interface IP address table. */
    _nx_dhcp_find_interface_table_ip_address(dhcp_ptr, iface_index, ip_address, &interface_address_ptr);

    /* Was it found? */
    if (interface_address_ptr == 0x0)
    {

        /* No, return an error status address not found. */
        return(NX_DHCP_IP_ADDRESS_NOT_FOUND);
    }

    /* Is the Client releasing IP address?  */
    if (assign_status == NX_DHCP_ADDRESS_STATUS_MARK_AVAILABLE)
    {

        /* Check if owner of this IP address matches with this client's record.  */
        _nx_dhcp_find_ip_address_owner(interface_address_ptr, dhcp_client_ptr, &assigned_to_client);

        /* Is the IP address owned (leased) to this client? */
        if (assigned_to_client == NX_FALSE)
        {

            /* No, this IP address is assigned to another host. We cannot change
               the IP address status. */
            return(NX_DHCP_IP_ADDRESS_ASSIGNED_TO_OTHER);
        }

        /* Yes, clear the owner information.  */
        _nx_dhcp_clear_ip_address_owner(interface_address_ptr);
    }

    /* Was the IP address assigned externally from the DHCP process?  */
    else if (assign_status == NX_DHCP_ADDRESS_STATUS_ASSIGNED_EXT)
    {

        /* Check the lease time.  */
        if (interface_address_ptr -> lease_time)
        { 
            /* If this already has a lease time (because we assigned it most likely) don't change it. */
            lease_time = interface_address_ptr -> lease_time; 
        }
        else
        {  
            /* Otherwise, make this lease time infinity e.g. the time out will not expire on it.*/
             lease_time = NX_WAIT_FOREVER;
        }

        /*  Set the current client as the owner.  */
        _nx_dhcp_record_ip_address_owner(interface_address_ptr, dhcp_client_ptr, lease_time);
    }

    /* Is the client informing us the IP address is already in use (e.g. it has
       sent the server a DECLINE message) or accepting another DHCP server's offer? */
    else if (assign_status == NX_DHCP_ADDRESS_STATUS_ASSIGNED_OTHER)
    {

        /* Yes; Is the client is declining the IP address? */
        if (dhcp_client_ptr -> nx_dhcp_message_type == NX_DHCP_TYPE_DHCPDECLINE)
        {

            /* Yes, remove the Client as owner of this IP lease.
               Record owner information with null 'owner' ID since we don't know who the owner is.  */
            _nx_dhcp_record_ip_address_owner(interface_address_ptr, NX_NULL, NX_WAIT_FOREVER);
        }
        else
        {

            /* Record owner information.  */
            _nx_dhcp_record_ip_address_owner(interface_address_ptr, dhcp_client_ptr, NX_WAIT_FOREVER);
        }
    }
    else
    { 

        /* Received an invalid update address command. */
        return(NX_DHCP_INVALID_UPDATE_ADDRESS_CMD);
    }
    
    /* Successful completion! */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_find_ip_address_owner                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finds the owner of the specified IP address and checks*/
/*    it against the specified client record to insure the client is the  */
/*    owner of the IP address. The result of the search is updated in the */
/*    assigned_to_client pointer.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    iface_owner                           Pointer to table entry        */ 
/*    dhcp_client_ptr                       Pointer to DHCP client        */
/*    assign_status                         Status to assign              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Table searched successfully   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_update_assignable_ip_address Update IP address status in   */ 
/*                                            the server database         */
/*    _nx_dhcp_server_assign_ip_address     Assign an IP address to the   */
/*                                            current client              */ 
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
static UINT  _nx_dhcp_find_ip_address_owner(NX_DHCP_INTERFACE_IP_ADDRESS *iface_owner, NX_DHCP_CLIENT *client_record_ptr, UINT *assigned_to_client)
{


    /* Initialize the value.  */
    *assigned_to_client = NX_FALSE;

    /* Compare the owner in the interface table entry with the client record mac address. */
    if ((iface_owner -> owner_hwtype == client_record_ptr -> nx_dhcp_client_hwtype) &&
        (iface_owner -> owner_mac_msw == client_record_ptr -> nx_dhcp_client_mac_msw) &&
        (iface_owner -> owner_mac_lsw == client_record_ptr -> nx_dhcp_client_mac_lsw))
    {

        /* They match; This verifies the IP address is assigned to this client. */
        *assigned_to_client = NX_TRUE;
    }

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_record_ip_address_owner                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function fills in the address owner in the interface table from*/
/*    the specified client record.                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    iface_owner                           Pointer to table entry        */ 
/*    client_record_ptr                     Pointer to DHCP client        */
/*    lease_time                            Lease duration in secs        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Table searched successfully   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_update_assignable_ip_address Update IP address status in   */ 
/*                                            the server database         */
/*    _nx_dhcp_server_assign_ip_address     Assign an IP address to the   */
/*                                            current client              */ 
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
static UINT  _nx_dhcp_record_ip_address_owner(NX_DHCP_INTERFACE_IP_ADDRESS *iface_owner, NX_DHCP_CLIENT *client_record_ptr, UINT lease_time)
{

    /* Check the client_record_ptr.  */
    if (client_record_ptr)
    {

        /* Parse the mac address and hardware type from the client record. */
        iface_owner -> owner_hwtype = client_record_ptr -> nx_dhcp_client_hwtype;
        iface_owner -> owner_mac_msw = client_record_ptr -> nx_dhcp_client_mac_msw; 
        iface_owner -> owner_mac_lsw = client_record_ptr -> nx_dhcp_client_mac_lsw;
    }
    else
    {

        /* We don't know who the owner is, set the owner information as zero.  */
        iface_owner -> owner_hwtype = 0;
        iface_owner -> owner_mac_msw = 0; 
        iface_owner -> owner_mac_lsw = 0;;
    }

    /* Record the lease time.  */
    iface_owner -> lease_time = lease_time;

    /* Set the assigned status.  */
    iface_owner -> assigned = NX_TRUE;

    /* Return.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_ip_address_owner                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function clears the address owner information.                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    iface_owner                           Pointer to table entry        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                                                          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_slow_periodic_timer_entry    Update IP lease time outs     */ 
/*    _nx_dhcp_clear_client_record          Clear Client record           */
/*    _nx_dhcp_update_assignable_ip_address Update IP address status in   */ 
/*                                                 the server database    */
/*    _nx_dhcp_server_assign_ip_address     Assign an IP address to the   */
/*                                                  current client        */ 
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
static UINT  _nx_dhcp_clear_ip_address_owner(NX_DHCP_INTERFACE_IP_ADDRESS *iface_owner)
{

    /* Clear the owner information.  */
    iface_owner -> owner_hwtype = 0;
    iface_owner -> owner_mac_msw = 0;
    iface_owner -> owner_mac_lsw = 0;

    /* Clear the lease time.  */
    iface_owner -> lease_time = 0;

    /* Clear the assigned status.  */
    iface_owner -> assigned = NX_FALSE;

    /* Return.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_validate_client_message                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function verifies the client message is a valid DHCP request,  */
/*    determines if an IP address need to be assigned, what state the DHCP*/
/*    Client should be advanced to, and what the server DHCP response type*/
/*    should be.  in the client record the specified client record.       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_ptr                              Pointer to DHCP Server        */ 
/*    client_record_ptr                     Pointer to DHCP client        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Table searched successfully    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_update_assignable_ip_address Update IP address status in    */ 
/*                                              the server database       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_listen_for_messages         Listen for, process and respond*/
/*                                            to DHCP Client messages     */
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
static UINT  _nx_dhcp_validate_client_message(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr)
{

ULONG                   server_ip_address;
UINT                    iface_index;
NX_DHCP_INTERFACE_TABLE *dhcp_interface_table_ptr;


    /* Create a local variable for the client's interface for convenience. */
    iface_index = dhcp_client_ptr -> nx_dhcp_client_iface_index;
    dhcp_interface_table_ptr = &dhcp_ptr -> nx_dhcp_interface_table[iface_index];

    if (dhcp_client_ptr -> nx_dhcp_xid == 0x0)
    {
    
#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: DHCP packet missing transaction ID. Reject the packet silently. \n");
#endif

        /* Return the request denied message to clients and return. */
        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPSILENT;
        return(NX_SUCCESS);
    }

    /* Client messages should never fill in the "Your (Client) IP" or "Next Server IP" fields. */
    else if ((dhcp_client_ptr -> nx_dhcp_your_ip_address !=  NX_DHCP_NO_ADDRESS) ||
        (dhcp_client_ptr -> nx_dhcp_clientrec_server_ip !=  NX_DHCP_NO_ADDRESS))
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: NACK! 'Your IP'/'Server IP' fields should not be filled in.\n");
#endif

        /* Return the 'request denied' message to the client. */
        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
    }

    /* Check for missing required fields. */
    else if ((dhcp_client_ptr -> nx_dhcp_client_hwtype == 0) ||
             (dhcp_client_ptr -> nx_dhcp_client_hwlen == 0))
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: NACK! DHCP packet missing MAC and/or hardware type\n");
#endif

        /* Return the request denied message to clients and return. */
        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
    }

    else
    {
    
        /* Do validation specific the for message type. This section will also
           advance the Client DHCP state and set the server response message type.
        */
        switch (dhcp_client_ptr -> nx_dhcp_message_type)
        {
    
    
            case NX_DHCP_TYPE_DHCPDISCOVER:
            {
    
#ifdef EL_PRINTF_ENABLE
                EL_PRINTF("DHCPserv: Received DISCOVER message\n");
#endif
    
                /* Determine if server needs to assign an IP address based on assigned IP address.  
                   e.g. If this appears to be a retransmission of a previous IP address DISOVERY request, 
                   only assign another IP address if the client appears not to have been assigned one already.*/
                /* Initialize response back to be an offer pending validation check. */
                dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPOFFER;  
    
                /* Check for invalid IP address field(s). */ 
                if (dhcp_client_ptr -> nx_dhcp_clientip_address != NX_DHCP_NO_ADDRESS)
                {
    
#ifdef EL_PRINTF_ENABLE
                    EL_PRINTF("DHCPserv: NACK! Client IP address field must not be filled in.\n");
#endif
    
                    /* Let the client know it has an invalid request. */
                    dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
                }
                else
                {
                
                    /* DHCP client state advances to SElECTING state. */
                    dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_SELECTING;
                }

                break;
            }
    
            case NX_DHCP_TYPE_DHCPREQUEST:       
            {
    
    
               /* This is a more complex message since a client request can be generated from 
                  4 different states: boot, selecting, renew, rebind. This requires that 
                  we determine the Client state based on what's previously known about 
                  the client and which fields are filled in the Request message. */
    
                /* Get the DHCP server IP address for the client package interface. */
                server_ip_address = dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_server_ip_address;
    
                /* Get the index of the client packet interface. */
                iface_index = dhcp_client_ptr -> nx_dhcp_client_iface_index;
    
                /* If the client does not specify a server id but is requesting an IP address...  */
                if ((dhcp_client_ptr -> nx_dhcp_server_id == 0) &&
                    (dhcp_client_ptr -> nx_dhcp_requested_ip_address != NX_DHCP_NO_ADDRESS))
                {
    
#ifdef EL_PRINTF_ENABLE
                    EL_PRINTF("DHCPserv: Received REQUEST (BOOT state) message\n");
#endif
                    /* ...Then it is in the boot state as per RFC 2131 4.3.2 (requests
                       an IP address but is skipping the DISCO message). */
    
                    /* This client may NOT specify a Client IP address e.g. that field must
                       be left blank. */ 
                    if (dhcp_client_ptr -> nx_dhcp_clientip_address != NX_DHCP_NO_ADDRESS)
                    {
    
#ifdef EL_PRINTF_ENABLE
                        EL_PRINTF("DHCPserv: NACK! REQUEST message may not specify Client IP address.\n");
#endif
    
                        /* Let the client know it has an invalid request. */
                        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
                        break;
                    }
    
                    /* Server SHOULD send a DHCPNAK message to the client if the 'requested IP address' is incorrect, or is on the wrong network. 
                       RFC 2131, Section 4.3.2, Page31.  */
                    if ((dhcp_client_ptr -> nx_dhcp_assigned_ip_address != 0) &&
                        (dhcp_client_ptr -> nx_dhcp_assigned_ip_address != dhcp_client_ptr -> nx_dhcp_requested_ip_address))
                    {
    
                        ULONG ip_address_assigned = dhcp_client_ptr -> nx_dhcp_assigned_ip_address;
    
#ifdef EL_PRINTF_ENABLE
                        EL_PRINTF("DHCPserv: NACK! REQUEST message contains incorrect requested IP address. \n");
#endif
    
                        /* No; invalid request. Return the IP address tentatively assigned to 
                          the client back to the available IP address pool. */
    
                        _nx_dhcp_update_assignable_ip_address(dhcp_ptr, dhcp_client_ptr, ip_address_assigned,
                                                       NX_DHCP_ADDRESS_STATUS_MARK_AVAILABLE); 
    
                        /* Clear requested IP lease time because the client is not granted an IP address lease. */                    
                        dhcp_client_ptr -> nx_dhcp_requested_lease_time = 0;
    
                        /* Let the client know it made an invalid request. */
                        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
                        break;
                    }

                    /* Set the client state to the BOOT state */
                    dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_BOOT;

                    /* Indicate the server will grant the IP lease. */ 
                    dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPACK;

                    /* Advance the client state to requesting (waiting for an ACK). */
                    dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_REQUESTING;
                    break;
                }
                /* Else if the client is specifying a server id, and a requested IP address... */
                else if (dhcp_client_ptr -> nx_dhcp_server_id &&
                        (dhcp_client_ptr -> nx_dhcp_requested_ip_address != NX_DHCP_NO_ADDRESS))
                {
    
#ifdef EL_PRINTF_ENABLE
                    EL_PRINTF("DHCPserv: Received REQUEST (SELECT state) message\n");
#endif
    
                    /* ...Then the client is in the SELECT state as per RFC 2131 4.3.2 (sent a 
                      DISCOVERY message and is responding to a server OFFER message). */
    
                    /* Client may NOT specify a Client IP address. */ 
                    if (dhcp_client_ptr -> nx_dhcp_clientip_address != NX_DHCP_NO_ADDRESS)
                    {
    
#ifdef EL_PRINTF_ENABLE
                        EL_PRINTF("DHCPserv: NACK! REQUEST message may not specify Client IP address.\n");
#endif
    
                        /* Let the client know it has an invalid request. */
                        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
                        break;
                    }
    
                    /* Check if the Client has chosen this server's offer (e.g.server ID option in the
                       REQUEST message). */
                    if (dhcp_client_ptr -> nx_dhcp_server_id != server_ip_address) 
                    {
    
                        ULONG  ip_address_assigned;
    
                        /* We are not. */
    
                        /* Is the Client accepting the same IP address this server offered? */
                        if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address == dhcp_client_ptr -> nx_dhcp_requested_ip_address)
                        {
    
                            /* Yes, so do not change the client record IP address and IP address
                               status in the server interface table as assigned.  */
                            ip_address_assigned = dhcp_client_ptr -> nx_dhcp_assigned_ip_address;
                        }
                        else
                        {
    
                            /* No, it is accepting a different IP address from another server. */
                            ip_address_assigned = dhcp_client_ptr -> nx_dhcp_requested_ip_address;
                            
                            /* So update the client record's assigned address to the one it is accepting
                               from another server. */
                            dhcp_client_ptr -> nx_dhcp_assigned_ip_address = dhcp_client_ptr -> nx_dhcp_requested_ip_address;
                        }
    
                        /* Update the server interface table with the information.  */
                        _nx_dhcp_update_assignable_ip_address(dhcp_ptr, dhcp_client_ptr, ip_address_assigned,
                                                                       NX_DHCP_ADDRESS_STATUS_ASSIGNED_OTHER); 
    
#ifdef EL_PRINTF_ENABLE
                        EL_PRINTF("DHCPserv: SILENCE! REQUEST message indicates Client accepting another server IP.\n");
#endif
    
                        /* If this client wants an IP address from this server it must start 
                           over at the init state. */
                        dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;
    
                        /* No need to respond. */
                        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPSILENT; 
        
                        return(NX_SUCCESS);
                    }
    
                    /* Make sure the Client did in fact receive an OFFER from this server (e.g. has 
                       been tentatively assigned an IP address from us). */
                    if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address == NX_DHCP_NO_ADDRESS)
                    {
    
#ifdef EL_PRINTF_ENABLE
                        EL_PRINTF("DHCPserv: NACK! REQUEST message does not indicate valid assigned IP address.\n");
#endif
    
                        /* It did not.  This is an invalid request. Let the client know. */
                        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
    
                        /* Clear requested IP lease times because the client was not assigned an IP address. */                    
                        dhcp_client_ptr -> nx_dhcp_requested_lease_time = 0;
    
                        /* Nothing more to do! */
                        break;
                    }
    
                    /* Does the Client's requested IP address match the "Your IP" field in the server OFFER 
                       previously sent to this client (e.g. Client's assigned IP address)? */
                    else if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address != dhcp_client_ptr -> nx_dhcp_requested_ip_address)
                    {
    
                        ULONG ip_address_assigned = dhcp_client_ptr -> nx_dhcp_assigned_ip_address;
    
#ifdef EL_PRINTF_ENABLE
                        EL_PRINTF("DHCPserv: NACK! REQUEST message contains requested IP address. \n");
#endif
    
                        /* No; invalid request. Return the IP address tentatively assigned to 
                          the client back to the available IP address pool. */
    
                        _nx_dhcp_update_assignable_ip_address(dhcp_ptr, dhcp_client_ptr, ip_address_assigned,
                                                       NX_DHCP_ADDRESS_STATUS_MARK_AVAILABLE); 
    
                        /* Clear requested IP lease time because the client is not granted an IP address lease. */                    
                        dhcp_client_ptr -> nx_dhcp_requested_lease_time = 0;
    
                        /* Let the client know it made an invalid request. */
                        dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
                        break;
                    }
                                                
                    /* The Client accepts this Server's offer. We will grant the client the IP lease now. */
                    dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPACK;
    
                    /* UPdate the client state to REQUESTING (waiting on an ACK). */
                    dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_REQUESTING; 
                }
    
               /* Is this a RENEW or REBIND request? If so the server ID and requested IP options must 
                  NOT be filled in as per RFC 2131 4.3.2. */
               else if ((dhcp_client_ptr -> nx_dhcp_server_id == NX_DHCP_NO_ADDRESS) &&
                       (dhcp_client_ptr -> nx_dhcp_requested_ip_address == NX_DHCP_NO_ADDRESS))
               {
    
#ifdef EL_PRINTF_ENABLE
                   EL_PRINTF("DHCPserv: Received REQUEST (RENEW/REBIND state) message\n");
#endif
    
                   /* Yes, this is a renew/rebind request! */
    
                   /* Extending an IP lease is a administration decision vs RFC mandate. The
                      NetX DHCP Server automatically extends IP leases. */
    
                   /* Go ahead and ACK the renew request pending any DHCP violations. */
                   dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPACK;
    
                   /* The "Client IP" address field MUST be filled in. */
                   if (dhcp_client_ptr -> nx_dhcp_clientip_address == NX_DHCP_NO_ADDRESS)
                   {
    
#ifdef EL_PRINTF_ENABLE
                       EL_PRINTF("DHCPserv: NACK! REQUEST message no 'Client IP' address specified. \n");
#endif
    
                       /* This is an invalid renew/rebind request. Reject the request but
                          do not change or advance the Client state (SHOULD be bound). */
                       dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
    
                       break;
                   }
    
                   /* The "Client IP" address subnet should match the Client packet source IP address subnet. */
                   if ((dhcp_interface_table_ptr -> nx_dhcp_subnet_mask & dhcp_client_ptr -> nx_dhcp_clientip_address) != 
                           (dhcp_interface_table_ptr -> nx_dhcp_subnet_mask & dhcp_interface_table_ptr -> nx_dhcp_server_ip_address))
                   {
#ifdef EL_PRINTF_ENABLE
                       EL_PRINTF("DHCPserv: NACK! REQUEST message; the 'Client IP' address subnet does not match the DHCP server subnet. \n");
#endif
    
                       /* This is an invalid renew/rebind request. Reject the request but
                          do not change or advance the Client state (SHOULD be bound). */
                       dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
                       break;
                   }
    
                   /* Is there an assigned IP address in the Client record?  */
                   if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address == NX_DHCP_NO_ADDRESS)
                   {
    
#ifdef EL_PRINTF_ENABLE
                       EL_PRINTF("DHCPserv: NACK! REQUEST message; server has no record of assigned IP address.\n");
#endif
    
                       /* No; possibly assigned by another DHCP server or outside of DHCP protocol. 
                          Reject the renewal request. */
                       dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
    
                       /* This client needs to start the configuration process with this
                          server from the INIT start if it wants an IP address from it. */
                       dhcp_client_ptr -> nx_dhcp_client_state  = NX_DHCP_STATE_INIT;
                       break;
                   }
    
                   /* Determine if this is a renew or a rebind request. */
                   if (dhcp_client_ptr -> nx_dhcp_destination_ip_address != NX_DHCP_BC_ADDRESS)
                   {
    
#ifdef EL_PRINTF_ENABLE
                       EL_PRINTF("DHCPserv: Received REQUEST (renewing) message\n");
#endif
    
                       /* Renewing: DHCP clients' renew requests must be unicast. */
                       dhcp_client_ptr -> nx_dhcp_client_state  = NX_DHCP_STATE_RENEWING;
                   }
                   else
                   {
    
#ifdef EL_PRINTF_ENABLE
                       EL_PRINTF("DHCPserv: Received REQUEST (rebinding) message\n");
#endif
    
                      /* Rebinding DHCP clients' rebind request must be broadcast. */
                       dhcp_client_ptr -> nx_dhcp_client_state  = NX_DHCP_STATE_REBINDING;
                   }
                }
                else
                {
    
#ifdef EL_PRINTF_ENABLE
                    EL_PRINTF("DHCPserv: NACK! Received unknown REQUEST message type.\n");
#endif
    
                    /* Do not respond to unknown requests. This is kind of a dealer's choice
                       because it is not specified in the RFC 2132 how to handle undefined
                       request IDs or message types. */
                    dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPSILENT;
                    return(NX_SUCCESS);
                }                 
    
                break;
            }
    
            case NX_DHCP_TYPE_DHCPRELEASE:
            {
    
#ifdef EL_PRINTF_ENABLE
                EL_PRINTF("DHCPserv: Received RELEASE response type.\n");
#endif
    
    
                /* The Client is releasing it's IP address back to the server. A client is not
                   required to do this; usually it indicates the client may have moved on the
                   network. 
    
                   Clear the client record and mark the IP address as available. If the request proves
                   invalid, this server will ignore it and let the Client's assigned IP lease expire.  */
    
                /* Regardless what happens the server does not send a reply to a RELEASE message. */
                dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPSILENT;
    
                /* Was an address previously assigned? */
                if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address == NX_DHCP_NO_ADDRESS)
                {
    
                    /* No; nothing to do here. */
                    return(NX_SUCCESS);
                }
    
                /* The client IP field must be filled in for RELEASE requests. Was it? */
                if (dhcp_client_ptr -> nx_dhcp_clientip_address == NX_DHCP_NO_ADDRESS)
                {
    
                    /* No; This is an invalid release request.  */
                    return(NX_SUCCESS);
                }
    
                /* Does client IP field match what this Server assigned the Client? */
                if (dhcp_client_ptr -> nx_dhcp_clientip_address != dhcp_client_ptr -> nx_dhcp_assigned_ip_address)
                {
    
                    /* No; Not sure how this happened but let the existing assigned
                       IP address lease expire. Ignore this message. */
                    return(NX_SUCCESS);
                }
    
                /* A RELEASE request must be unicasted to the server. Was this a broadcast
                   message? */
                if (dhcp_client_ptr -> nx_dhcp_destination_ip_address == NX_DHCP_BC_ADDRESS)
                {
    
                    /* Yes, so not a valid RELEASE request. */
                    return(NX_SUCCESS);
                }
                else
                {
    
                    UINT index = dhcp_client_ptr -> nx_dhcp_client_iface_index;
                    ULONG server_ip = (&dhcp_ptr -> nx_dhcp_interface_table[index]) ->nx_dhcp_server_ip_address;
    
                    /* Check that we are the intended server. */
                    if (dhcp_client_ptr -> nx_dhcp_destination_ip_address != server_ip)
                    {
    
                        /* We are not. Not a valid Release request. */
                        return(NX_SUCCESS);
                     }
                }
    
                /* Legitimate release request: return the IP address to the available pool, 
                   and clear the client session. */
    
                /* Release the client's assigned IP address (in the "CI-ADDR" field). */
                _nx_dhcp_update_assignable_ip_address(dhcp_ptr, dhcp_client_ptr, 
                                                      dhcp_client_ptr -> nx_dhcp_assigned_ip_address, 
                                                      NX_DHCP_ADDRESS_STATUS_MARK_AVAILABLE);                 
    
                /* Clear the Client's assigned IP address. */
                dhcp_client_ptr -> nx_dhcp_assigned_ip_address = NX_DHCP_NO_ADDRESS;
    
                /* If this client wants to configure its network parameters with us it 
                   must start at the INIT state. */
                dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;
    
    
                break;
            }
            
            case NX_DHCP_TYPE_DHCPDECLINE:
            {
    
#ifdef EL_PRINTF_ENABLE
                EL_PRINTF("DHCPserv: Received DECLINE response type.\n");
#endif
    
                /* The Client is informing us an IP address we assigned to it is apparently already
                   in use by another host.  It needs to restart in the INIT/BOOT state for this
                   server to assign it another IP address. */
                dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;
    
                /* Regardless what happens do not send a reply. */
                dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPSILENT;
    
                /* If no address previously assigned, silently ignore this message. */
                if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address == NX_DHCP_NO_ADDRESS) 
                {
                
                    /* This client never had an address assigned by this server. Do nothing. */
                    return(NX_SUCCESS);
                }
    
                /* A DECLINE message must specify the requested address. */
                if (dhcp_client_ptr -> nx_dhcp_requested_ip_address == NX_DHCP_NO_ADDRESS)
                {
    
                    /* This is an invalid decline request. Reject it silently. */
                    return(NX_SUCCESS);
                }
    
                /* A Decline request is supposed to be unicasted to the server. If it is not,
                   ignore it. */
                if (dhcp_client_ptr -> nx_dhcp_destination_ip_address != NX_DHCP_BC_ADDRESS)
                {
    
                    return(NX_SUCCESS);
                }
                else
                {
    
                    UINT index = dhcp_client_ptr -> nx_dhcp_client_iface_index;
                    ULONG server_ip = (&dhcp_ptr -> nx_dhcp_interface_table[index]) ->nx_dhcp_server_ip_address;
    
                    /* Check that we are the intended server. */
                    if (dhcp_client_ptr -> nx_dhcp_server_id != server_ip)
                    {
    
                        /* We are not. Ignore the request. */
                        return(NX_SUCCESS);
                     }
                }
    
                /* Release the client's assigned IP address. */
                _nx_dhcp_update_assignable_ip_address(dhcp_ptr, dhcp_client_ptr, 
                                                               dhcp_client_ptr -> nx_dhcp_assigned_ip_address, 
                                                               NX_DHCP_ADDRESS_STATUS_ASSIGNED_OTHER); 
    
                /* Clear the Client's assigned IP address. */
                dhcp_client_ptr -> nx_dhcp_assigned_ip_address = NX_DHCP_NO_ADDRESS;
    
                break;
            }
    
            case NX_DHCP_TYPE_DHCPINFORM:
            {
    
#ifdef EL_PRINTF_ENABLE
                  EL_PRINTF("DHCPserv: Received INFORM response type.\n");
#endif
    
    
                /* A DHCP client sends an INFORM message when it has been assigned an IP address 
                   with another server or outside DHCP but still needs local configuration data. 
                   The DHCP server must update its database with their address (specified in the "CI-ADDR" field) 
                   as no longer available. When the server forms a reply ACK, it must leave the 
                   'Your IP' and lease time fields blank as per RFC 2131 3.4. */
    
                 
                /* A DHCP server must respond (ACK) to INFORM messages unless the message
                   is invalid. */
                dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPACK;  
                
                /* The INFORM message must have the Client IP address filled in to be valid. */
                if (dhcp_client_ptr -> nx_dhcp_clientip_address == NX_DHCP_NO_ADDRESS)
                {
    
                    /* Invalid message. */
                    dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;  
                }
    
                /* It cannot have a requested IP option in the options. */
                if (dhcp_client_ptr -> nx_dhcp_requested_ip_address != NX_DHCP_NO_ADDRESS)
                {
    
                    /* Invalid message. */
                    dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;  
                }
    
                /* Make sure the server interface table is up to date for this IP address. */
                _nx_dhcp_update_assignable_ip_address(dhcp_ptr, dhcp_client_ptr, dhcp_client_ptr -> nx_dhcp_clientip_address, 
                                                      NX_DHCP_ADDRESS_STATUS_ASSIGNED_EXT);                 
    
                /* And update the client record assigned IP address in case it is not set yet. */
                dhcp_client_ptr -> nx_dhcp_assigned_ip_address = dhcp_client_ptr -> nx_dhcp_clientip_address;
    
                /* Has client IP address leased (bound) from this server? */
                if (dhcp_client_ptr -> nx_dhcp_client_state != NX_DHCP_STATE_BOUND)
                {
                
                    /* No, so if this client needs to configure an IP address assigned 
                       with this server it must start at the INIT state. */
                    dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;
                }
    
                break;
            }
            default:
            {
    
#ifdef EL_PRINTF_ENABLE
                 EL_PRINTF("DHCPserv: NACK! Received unknown response type.\n");
 #endif

                 dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPSILENT;

                 return(NX_SUCCESS);
    
                /* No further processing to do. Leave the client state unchanged, whatever it was/is. */
            }
    
        } /* switch case on client message*/
    }

    /* Were there were any conditions requiring a NACK response? */
    if (dhcp_client_ptr -> nx_dhcp_response_type_to_client == NX_DHCP_TYPE_DHCPNACK)
    {
        /* Yes, the caller knows to send the NACK message. No processing to do. */
        return NX_SUCCESS;
    }

    /* If this is not a DISCOVERY or REQUEST message, we are done! */
    if ((dhcp_client_ptr -> nx_dhcp_message_type != NX_DHCP_TYPE_DHCPDISCOVER) &&
        (dhcp_client_ptr -> nx_dhcp_message_type != NX_DHCP_TYPE_DHCPREQUEST))
    {

        return(NX_SUCCESS);
    }

    /* If the boot/init state client did not request an address, we are done. */
    if (dhcp_client_ptr -> nx_dhcp_requested_ip_address == NX_DHCP_NO_ADDRESS)
    {

        return(NX_SUCCESS);
    }

    /* Check if the Client has requested a valid IP address. This is only permitted
       with a DISCO or REQUEST messages while the Client is in either the BOOT/INIT 
       or SELECT states. 

        As per RFC 2131 4.3.1, the DHCP server should assign an IP address in order of preference as follows:
        1) the client currently is bound to
        2) client ip address in client records (e.g. previously assigned to)
        3) the currently requested IP
        4) the server will assign one. */
     
    /* Find the currently binding source address in the server's database. */
    if (dhcp_client_ptr -> nx_dhcp_source_ip_address != NX_DHCP_NO_ADDRESS)
    {

        /* Assign this as the Client's IP address. */
        dhcp_client_ptr -> nx_dhcp_assigned_ip_address = dhcp_client_ptr -> nx_dhcp_source_ip_address;
    }

    /* Next best option: Has the client previously been assigned an IP address? */
    else if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address) 
    {

        /* Yes, let's leave it assigned to it. */
        return(NX_SUCCESS);
    }
    else
    {
    
        /* Last option: Set the client's requested IP address as the one to assign. */
        dhcp_client_ptr -> nx_dhcp_assigned_ip_address = dhcp_client_ptr -> nx_dhcp_requested_ip_address;
    }

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_assign_ip_address                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This service finds an available IP address for the client if none is  */
/*  provided, and updates the client's record with the new IP address. It */ 
/*  updates the interface table entry with current client data (lease     */
/*  time, owner).                                                         */
/*                                                                        */ 
/*  If there already is an assigned IP address on the client record, this */
/*  function will verify the IP address has not been assigned to another  */
/*  client before setting the current client as owner.  If it has, this   */
/*  function will look for another available IP address for  the client.  */
/*                                                                        */ 
/*  Once an address is assigned, this function updates the fields in the  */
/*  interface pointer entry, and the "Your IP address" in its message back*/
/*  to the client.                                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_cptr                             Pointer to DHCP Server        */
/*    dhcp_client_ptr                       Pointer to client record      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion         */ 
/*    NX_DHCP_NO_AVAILABLE_IP_ADDRESSES     No available address to assign*/ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_find_interface_table_ip_address                            */ 
/*                                          Look up IP address in         */
/*                                             server interface table     */
/*    _nx_dhcp_find_ip_address_owner        Look up owner of an IP address*/ 
/*                                             in server interfce table   */
/*    _nx_dhcp_record_ip_address_owner      Set the client as IP address  */
/*                                              owner in interface table  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_validate_client_message      Check Client DHCP messages for*/ 
/*                                              proper data and determine */
/*                                              server response           */
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
static UINT  _nx_dhcp_server_assign_ip_address(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr)
{

NX_DHCP_INTERFACE_TABLE         *dhcp_interface_table_ptr;
NX_DHCP_INTERFACE_IP_ADDRESS    *interface_address_ptr;
UINT                            assigned_to_client;
UINT                            assigned_ip;
UINT                            lease_time;


    /* Set a flag on the outcome of finding an available address. */
    assigned_ip = NX_FALSE;

    /* Compute the client lease time based on client's requested 
       lease time and server default lease time. */
    lease_time = dhcp_client_ptr -> nx_dhcp_requested_lease_time;


#if (NX_DHCP_DEFAULT_LEASE_TIME >= 0xFFFFFFFF)
    if (lease_time == 0)
#else
    if ((lease_time == 0) || (lease_time > NX_DHCP_DEFAULT_LEASE_TIME))
#endif
    {
        lease_time = NX_DHCP_DEFAULT_LEASE_TIME;
    }


    interface_address_ptr = (NX_DHCP_INTERFACE_IP_ADDRESS *)NX_NULL;

    /* Does the Client have a candidate already assigned? */
    if (dhcp_client_ptr -> nx_dhcp_assigned_ip_address)
    {

        /* The client does have one assigned, but we need to verify it is available
           or already assigned to the client. */

        /* Look for this IP address in the specified interface IP address list (table). */
        _nx_dhcp_find_interface_table_ip_address(dhcp_ptr, dhcp_client_ptr -> nx_dhcp_client_iface_index, 
                                 dhcp_client_ptr -> nx_dhcp_assigned_ip_address, &interface_address_ptr);

        /* Was the ip address found? */
        if (interface_address_ptr)
        {

            /* Yes; Is it assigned to anyone yet? */
            if (interface_address_ptr -> assigned == NX_FALSE)
            {
                /* No, We will assign this one further below. */
            }
            else
            {

                /* Determine who if is assigned to.  */
                _nx_dhcp_find_ip_address_owner(interface_address_ptr, dhcp_client_ptr, &assigned_to_client);

                /* Assigned to this client? */
                if (assigned_to_client)
                {


                    /* Yes, so we're done here, except for checking a few fields. */

                    /* Renew the lease time. */
                    interface_address_ptr -> lease_time = lease_time;

                    /* Set the Your IP address field for the server response message. */
                    dhcp_client_ptr -> nx_dhcp_your_ip_address = interface_address_ptr -> nx_assignable_ip_address;

                    return(NX_SUCCESS);
                }

#ifdef EL_PRINTF_ENABLE
                EL_PRINTF("DHCPserv: NACK! Cannot assign requested IP address (already assigned)\n");
#endif

                /* No, assigned to someone else.  We cannot provide this so return a NACK. */
                dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
                dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;

                /* Clear the client's assigned IP address since it is assigned to someone else. */
                dhcp_client_ptr -> nx_dhcp_assigned_ip_address = NX_DHCP_NO_ADDRESS;
                
                return(NX_SUCCESS);
            }
        }
        else
        {

            /* If this is a NOT discovery message, the server can not very well
                assign a different IP address becaause an ACK reply cannot contain
                different information. */
            if (dhcp_client_ptr -> nx_dhcp_message_type != NX_DHCP_TYPE_DHCPDISCOVER)
            {

#ifdef EL_PRINTF_ENABLE
                EL_PRINTF("DHCPserv: NACK! Cannot assign requested IP address (not owned by server)\n");
#endif

                /* Apparently the client has asked for an IP address either not 
                   in the server database or on another network.  We cannot provide this
                   so return a NACK. */
                dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;
                dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;
    
                /* Clear the client record's 'assigned' IP address since none was assigned. */
                dhcp_client_ptr -> nx_dhcp_assigned_ip_address = NX_DHCP_NO_ADDRESS;
    
                /* Since the client did not get assigned an IP address
                   make sure the requested lease time is reset to zero. */
                dhcp_client_ptr -> nx_dhcp_requested_lease_time = 0;
    
                return(NX_SUCCESS);
            }

            /* For a Discovery request, the server will go ahead and find an 
               available IP address to offer the client. */
        }
    }

    /* If the client doesn't have an address assigned yet, find one in the server table not 
       yet assigned. */
    if (interface_address_ptr == 0x0)
    {

        UINT i;

        /* Set a local varible to the IP address list for this client. */
        dhcp_interface_table_ptr = &(dhcp_ptr -> nx_dhcp_interface_table[dhcp_client_ptr -> nx_dhcp_client_iface_index]);
    
        /* Yes, search for an available ip address in the IP list for this interface */
        for(i = 0; i < dhcp_interface_table_ptr -> nx_dhcp_address_list_size; i++)
        {
    
            /* Set a local pointer variable to the next address. */
            interface_address_ptr = &dhcp_interface_table_ptr -> nx_dhcp_ip_address_list[i];
    
            /* Is this address assigned already? */
            if (interface_address_ptr -> assigned == NX_FALSE)
            {

                /* No, but it is now! */

                /* Indicate the search was successful. */
                assigned_ip = NX_TRUE;

                break;
            }
        }

        /* Check if we were able to find an available IP address. */
        if (assigned_ip == NX_FALSE)
        {

#ifdef EL_PRINTF_ENABLE
            EL_PRINTF("DHCPserv: NACK! Cannot assign requested IP address (not found in server IP list)\n");
#endif


            /* The server is not obliged to respond, but it should let the client
               know if it cannot provide what it requests. */
            dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;
            dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;

            /* Since the client did not get assigned an IP address
               make sure the requested lease time is reset to zero. */
            dhcp_client_ptr -> nx_dhcp_requested_lease_time = 0;

            /* Make sure the client record's 'assigned' IP address ir removed since it was not assigned. */
            dhcp_client_ptr -> nx_dhcp_assigned_ip_address = NX_DHCP_NO_ADDRESS;

            /* No, return the error status. */
            return(NX_DHCP_NO_AVAILABLE_IP_ADDRESSES);
        }
    }

    /* Was the assigned ip address available? */
    if (interface_address_ptr)
    {

        /* Set the client as the IP address owner in the server interface table. */
        _nx_dhcp_record_ip_address_owner(interface_address_ptr, dhcp_client_ptr, lease_time);

        /* Set the client's assigned IP address. */
        dhcp_client_ptr -> nx_dhcp_assigned_ip_address = interface_address_ptr -> nx_assignable_ip_address;

        /* Set the Your IP address field for the server response message. */
        dhcp_client_ptr -> nx_dhcp_your_ip_address = interface_address_ptr -> nx_assignable_ip_address;

        /* We are done! */
        return(NX_SUCCESS);
    }

    /* If we're here we were not able to assign an IP address. */

    /* The server is not obliged to respond, but it should let the client
       know if it cannot provide what it requests. */
    dhcp_client_ptr -> nx_dhcp_client_state = NX_DHCP_STATE_INIT;

#ifdef EL_PRINTF_ENABLE
    EL_PRINTF("DHCPserv: NACK! Server cannot assign requested IP address\n");
#endif

    dhcp_client_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPNACK;

    /* Make sure the client record's 'assigned' IP address ir removed since it was not assigned. */
    dhcp_client_ptr -> nx_dhcp_assigned_ip_address = NX_DHCP_NO_ADDRESS;

    /* Since the client did not get assigned an IP address
       make sure the requested lease time is reset to zero. */
    dhcp_client_ptr -> nx_dhcp_requested_lease_time = 0;

    return(NX_DHCP_NO_AVAILABLE_IP_ADDRESSES); 
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_extract_information                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts information from a DHCP Client packet and    */
/*    updates the server's client database. If no client record exists for*/
/*    the client this function will create one for it.  It does some      */
/*    message validation but primary extracts DHCP header and option      */
/*    fields to the client record.                                        */
/*                                                                        */
/*    If an internal error is encountered parsing the packet buffer, this */
/*    function returns the error status.  If it finds the client DHCP     */
/*    message invalid, it returns successful completion but sets the DHCP */
/*    server to respond with either NACK or silence.                      */
/*                                                                        */
/*    When this function is finished, the caller releases the packet.     */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_cptr                             Pointer to DHCP Server        */
/*    dhcp_client_ptr                       Pointer to client record      */
/*    packet_ptr                            Pointer to DHCP client packet */
/*    iface_index                           Interface index packet was    */
/*                                                  received on           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion         */ 
/*    status                                Actual completion status      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_find_client_record_by_chaddr Find Client record using the  */
/*                                            parsed mac address          */ 
/*    _nx_dhcp_server_get_data              Parse DHCP data from buffer   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_listen_for_messages          Process DHCP Client messages  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed the issue of read     */
/*                                            and write overflow,         */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dhcp_server_extract_information(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT **dhcp_client_ptr, 
                                          NX_PACKET *packet_ptr, UINT iface_index)
{

UINT            status;
ULONG           value;
UINT            size = 0;
ULONG           xid;
UCHAR           *work_ptr;
ULONG           client_mac_msw;
ULONG           client_mac_lsw;
NX_IPV4_HEADER  *ip_header_ptr;
NX_DHCP_CLIENT  *temp_client_rec_ptr;


    /* Initialize the DHCP Client pointer to null pending successful data extraction. */
    *dhcp_client_ptr = NX_NULL;

    /* Set up UCHAR pointer to HW address field to extract client hardware address
       which we will use as the client's unique identifier, since not all clients may use
       Option 61/Client Identifier.  */
    work_ptr = packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_CLIENT_HW;

    /* Pickup the MSW of the MAC address.  */
    client_mac_msw = (((ULONG) work_ptr[0]) << 8)  | ((ULONG) work_ptr[1]);
    client_mac_lsw = (((ULONG) work_ptr[2]) << 24) |
                     (((ULONG) work_ptr[3]) << 16) |
                     (((ULONG) work_ptr[4]) << 8)  |
                     ((ULONG) work_ptr[5]);

    /* Check the client hardware (mac address) field is filled in. */
    if ((client_mac_msw == 0) && (client_mac_lsw == 0))
    {
    
        return(NX_DHCP_INVALID_HW_ADDRESS);
    }

    /* Look up the client record by IP address and interface index in Client Records table. */
    status = _nx_dhcp_find_client_record_by_chaddr(dhcp_ptr, iface_index, client_mac_msw, 
                                                   client_mac_lsw, &temp_client_rec_ptr, NX_TRUE);


    /* Check for error during search. */
    if ((status != NX_SUCCESS) ||  !temp_client_rec_ptr)
    {
        return(status);
    }


    /* Note: No need to check for NULL pointer because we asked to add the
       record if no match is found. */


    /* Get the client's current binding IP address from the packet interface. */
    ip_header_ptr = (NX_IPV4_HEADER *)(packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_UDP_HEADER) - sizeof(NX_IPV4_HEADER));

    temp_client_rec_ptr -> nx_dhcp_source_ip_address = ip_header_ptr -> nx_ip_header_source_ip;
    temp_client_rec_ptr -> nx_dhcp_destination_ip_address = ip_header_ptr -> nx_ip_header_destination_ip;

    /* Get the message type. */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_OP, 1, &value);
    if (status)
    {
        return(status);
    }

    /* Check this is a 'request' DHCP Client message (not to be confused with the REQUEST  
       message type. */
    if (value != NX_DHCP_OP_REQUEST)
    {

#ifdef EL_PRINTF_ENABLE
        EL_PRINTF("DHCPserv: SILENCE! Client DHCP packet not coded as a BOOT Request. Invalid request.\n");
#endif

        /* Set the server to ignore the 'reply'. */
        temp_client_rec_ptr -> nx_dhcp_response_type_to_client = NX_DHCP_TYPE_DHCPSILENT;

        /* Return successful completion status. */
        return(NX_SUCCESS);
    }

    /* Get the hardware type. */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_HTYPE, 1, &value);
    if (status)
    {
        return(status);
    }

    /* This is the Client interface type which is used in the default client identifier tag. */ 
    temp_client_rec_ptr -> nx_dhcp_client_hwtype = value;

    /* Get the hardware length. */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_HLEN, 1, &value);
    if (status)
    {
        return(status);
    }

    /* This is the size of the hardware address. Used in the default client identifier tag. */ 
    temp_client_rec_ptr -> nx_dhcp_client_hwlen = value;                        

    /* Extract the message ID. */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_XID, 4, &xid);
    if (status)
    {
        return(status);
    }

    /* Save the client xid to use in the server's reply back. */
    temp_client_rec_ptr -> nx_dhcp_xid = xid;
                                                   
    /* Check if response to client should unicast or (default) broadcast.  */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_FLAGS, 1, &value);
    if (status)
    {
        return(status);
    }

    /* Get the Client's preference for unicast vs broadcast. */
    temp_client_rec_ptr -> nx_dhcp_broadcast_flag_set = (value & NX_DHCP_FLAGS_BROADCAST) ? 0x80 : 0x0;

    /* Extract the Requested Client IP address, if there is one. */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_CLIENT_IP, 4, &value);
    if (status)
    {
        return(status);
    }

    /* This is the Client IP address reported by the Client. */ 
    temp_client_rec_ptr -> nx_dhcp_clientip_address = value;            

    /* Update the IP address assigned to the Client "Your/client IP address".  */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_YOUR_IP, 4, &value);
    if (status)
    {
        return(status);
    }

    /* This is the "Your IP address" which only the server should fill in. Should be zero in 
       client messages. */
    temp_client_rec_ptr -> nx_dhcp_your_ip_address =  value;

    /* Extract the "Next Server IP address".     */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_SERVER_IP, 4, &value);
    if (status)
    {
        return(status);
    }
    temp_client_rec_ptr -> nx_dhcp_clientrec_server_ip =  value;   

    /* Store relay IP address for DHCP server (on another subnet) if there is one. */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_RELAY_IP, 4, &value);
    if (status)
    {
        return(status);
    }
    temp_client_rec_ptr -> nx_dhcp_relay_ip_address =  value;

    /* Look for the marker indicating option data (magic cookie).  */
    status = _nx_dhcp_server_get_data(packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_VENDOR, 4, &value);
    if (status)
    {
        return(status);
    }

    /* Note we don't store the magic cookie value. */
   
    /* Are there user options?  */
    if (value == NX_DHCP_MAGIC_COOKIE)
    {
        /* Yes there are user options in the Client message. */

        /* Move up the buffer pointer past the magic cookie thing. */
        work_ptr = packet_ptr -> nx_packet_prepend_ptr + NX_DHCP_OFFSET_VENDOR + 4;

        /* Extract all the client option data. */
        while (*work_ptr != 0xFF)   
        {

            /* Guard against a missing "END" marker by checking if we are at the end of the DHCP packet data. */
            if (work_ptr + 2 > packet_ptr -> nx_packet_append_ptr)
            {

                /* Yes, Client must have sent a DHCP message with improperly terminated option 
                   data (or Client is using a super large DHCP packet). */
                return(NX_DHCP_IMPROPERLY_TERMINATED_OPTION);
            }

            /* Get the next option. */
            _nx_dhcp_server_get_data(work_ptr, 1, &value);

            /* Move up the buffer pointer to the option length. */
            work_ptr++;

            /* Get the option length. */
            _nx_dhcp_server_get_data(work_ptr, 1, (ULONG *)&size);   

            /* Move up the buffer pointer to the next option. */
            work_ptr++;

            /* Validate the size. */
            if (work_ptr + size > packet_ptr -> nx_packet_append_ptr)
            {
                return(NX_DHCP_IMPROPERLY_TERMINATED_OPTION);
            }

            /* Is this the client ID option? */
            if (value != NX_DHCP_SERVER_OPTION_CLIENT_ID)
            {

                /* Check if there is enough space to store client requested options. */
                if (temp_client_rec_ptr -> nx_dhcp_client_option_count < NX_DHCP_CLIENT_OPTIONS_MAX)
                {

                    /* Process as any other option. */
                    _nx_dhcp_process_option_data(temp_client_rec_ptr, (CHAR *)work_ptr, (UCHAR)value, NX_TRUE, size);
                }
            }

            /* Move up the buffer pointer past the current option data size to the next option. */
            work_ptr += size;
        }
    }

    /* Return the location of the client record. */
    *dhcp_client_ptr = temp_client_rec_ptr;

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_get_option_data                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine extracts data from the specified location and adds the */ 
/*    Client's requested option to the Client record.  The caller can ask */
/*    for the option data as well to be extracted and stored to client    */
/*    record.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_client_ptr                      Pointer to DHCP Client         */ 
/*    buffer                               Pointer to data buffer         */ 
/*    option                               Option from Client DHCP packet */ 
/*    size                                 Size of data buffer            */ 
/*    get_option_data                      Indicate if server wants option*/
/*                                           data stored in client record */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                               Error status extracting data   */ 
/*    NX_SUCCESS                           Successful completion          */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_server_get_data             Extract data from specified    */
/*                                                  buffer location       */ 
/*    memcpy                               Copy specified area of memory  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_server_extract_information   Extract DHCP data from packet */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            fixed the issue of infinite */
/*                                            recursion,                  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT  _nx_dhcp_process_option_data(NX_DHCP_CLIENT *dhcp_client_ptr, CHAR *buffer, UCHAR option, UINT get_option_data, UINT size)
{

UINT  status;
ULONG option_value = 0;

    /* Do we parse option data for this option? */
    if (get_option_data)
    {

        /* Yes, store to local variable to be applied to the Client record below. */
        status = _nx_dhcp_server_get_data((UCHAR *)buffer, size, &option_value);
        if (status)
        {
            return(status);
        }
    }

    switch (option)
    {

        case 1: 
            /* Subnet mask */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            if (get_option_data)
            {
                dhcp_client_ptr -> nx_dhcp_subnet_mask = option_value;
            }
            dhcp_client_ptr -> nx_dhcp_client_option_count++;
            break;

        case 3:
            /* Router or gateway */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            if (get_option_data)
            {
                dhcp_client_ptr -> nx_dhcp_router_ip_address = option_value;
            }
            dhcp_client_ptr -> nx_dhcp_client_option_count++;
            break;

        case 6:
            /* Domain name server */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            if (get_option_data)
            {
                dhcp_client_ptr -> nx_dhcp_dns_ip_address = option_value;      
            }
            dhcp_client_ptr -> nx_dhcp_client_option_count++;
            break;


        case 12: 
            /* Client host name */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            if (get_option_data)
            {

                /* Only store as much of the host name as will fit in the host name buffer. */
                size  = (size <= NX_DHCP_CLIENT_HOSTNAME_MAX) ? size : NX_DHCP_CLIENT_HOSTNAME_MAX;

                /* Clear the memory buffer before writing the host name to it. */
                memset(&dhcp_client_ptr -> nx_dhcp_client_name[0], 0, NX_DHCP_CLIENT_HOSTNAME_MAX);
                memcpy(&dhcp_client_ptr -> nx_dhcp_client_name[0], buffer, size); /* Use case of memcpy is verified. */
            }
            dhcp_client_ptr -> nx_dhcp_client_option_count++;
            break;

        case 50:

            /* Client's requested IP address */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            if (get_option_data)
            {

                dhcp_client_ptr -> nx_dhcp_requested_ip_address = option_value;
            }
            dhcp_client_ptr -> nx_dhcp_client_option_count++;
            break;

        case 51:

            /* Client's requested IP address lease time */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            if (get_option_data)
            {

                dhcp_client_ptr -> nx_dhcp_requested_lease_time = option_value;
            }
            dhcp_client_ptr -> nx_dhcp_client_option_count++;
            break;

        case 53:
            /* Message type */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            dhcp_client_ptr -> nx_dhcp_message_type = (UCHAR)option_value;
            dhcp_client_ptr -> nx_dhcp_client_option_count++;
            break;

        case 54:    
            /* Server ID by which server identifies itself to client(usually server IP address). 
               The Client includes this option in a REQUEST message to identify which server
               it wants to lease an IP address from. */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            if (get_option_data)
            {

                dhcp_client_ptr -> nx_dhcp_server_id = option_value;
            }
            dhcp_client_ptr -> nx_dhcp_client_option_count++;
            break;

        case 55:

            /* Check if there is enough space to store all the client requested options. */
            if (dhcp_client_ptr -> nx_dhcp_client_option_count + size > NX_DHCP_CLIENT_OPTIONS_MAX)
            {
                size = NX_DHCP_CLIENT_OPTIONS_MAX - dhcp_client_ptr -> nx_dhcp_client_option_count;
            }

            /* Update the client record with that option. */
            memcpy(&(dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count]), buffer, size); /* Use case of memcpy is verified. */
            dhcp_client_ptr -> nx_dhcp_client_option_count += size;
            break;

        case 61:    

            /* Note Client ID is handled separately. */
            dhcp_client_ptr -> nx_dhcp_user_options[dhcp_client_ptr -> nx_dhcp_client_option_count] = option;
            dhcp_client_ptr -> nx_dhcp_client_option_count++;

            break;

        case 116: /* Auto configuration option */
        case 60:  /* Vendor class Identifier. */  

            /* Not supported at this time. */
            break;

        default:
            break;
    }
    
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_add_requested_option                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine receives options and which interface the DHCP client is*/
/*    on and adds the server information if there is any for that option  */
/*    into the packet reply buffer.                                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_message                         Pointer to message buffer      */ 
/*    iface_index                          Index to server interface table*/ 
/*    buffer                               Pointer to buffer to write to  */
/*    option                               Option to add                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_add_option                   Adds the actual option to the */ 
/*                                             DHCP message buffer        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_respond_to_dhcp_message     Prepare and send out DHCP      */
/*                                                 response to client     */ 
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
static UINT  _nx_dhcp_add_requested_option(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, UCHAR *buffer, UINT option, UINT *index)
{

UINT status = NX_SUCCESS;

    /* The NetX DHCP server provides a limited list of DHCP options for ACK responses. These do NOT 
       include the server's 'required' options. */
    switch (option)
    {

        case 1: /* Subnet mask  */
            status = _nx_dhcp_add_option(buffer, option, NX_DHCP_SERVER_OPTION_SUBNET_MASK_SIZE, 
                                         dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_subnet_mask, index);
            break;

        case 3:  /* Router IP address */  
                                                                            
                                                                            
            status = _nx_dhcp_add_option(buffer, option, NX_DHCP_SERVER_OPTION_ROUTER_SIZE, 
                                         dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_router_ip_address, index);
            break;

        case 6:  /* DNS IP address */
            status = _nx_dhcp_add_option(buffer, option, NX_DHCP_SERVER_OPTION_ADDRESS_SIZE, 
                                         dhcp_ptr -> nx_dhcp_interface_table[iface_index].nx_dhcp_dns_ip_address, index);
            break;

        /* The NetX DHCP server does not reply to other options at this time. */
        default: 
            break;
    }

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_add_option                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This routine adds a DHCP option data to the DHCP message in the     */ 
/*    supplied buffer.  Adding the option includes adding the option code,*/ 
/*    length of the data and option data value.                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcp_message                          Pointer to message buffer     */ 
/*    option                                Option to add                 */ 
/*    value                                 Value of Option to add        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_server_store_data            Store data value              */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_process                      Process the DHCP state machine*/ 
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
static UINT  _nx_dhcp_add_option(UCHAR *dhcp_message, UINT option, UINT size, ULONG value, UINT *index)
{
                     
                  
    /* Store the option.  */
    *(dhcp_message + (*index)) = (UCHAR)option;
    (*index) ++;

    /* Store the option size.  */
    *(dhcp_message + (*index)) = (UCHAR)size; 
    (*index) ++;

    /* Store the option value.  */
    _nx_dhcp_server_store_data(dhcp_message + (*index), size, value);
    (*index) += size;    

    /* Return a successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_get_data                            PORTABLE C      */ 
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
/*    value                                 Data value retrieved          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion         */ 
/*    NX_DHCP_PARAMETER_ERROR               Invalid parameter input       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcp_get_response                 Get response from server      */ 
/*    _nx_dhcp_server_extract_information   Extract server information    */ 
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
static UINT  _nx_dhcp_server_get_data(UCHAR *data, UINT size, ULONG *value)
{


    /* Check for invalid buffer parameters. */
    if ((size == 0) || (data == 0x0) || (value == 0))
    {
        return(NX_DHCP_PARAMETER_ERROR);  
    }

    /* Initialize value to zero. */
    *value = 0;

    /* Store each byte of data from buffer into result.  */
    while (size > 0)
    {

        /* Build return value.  */
        *value = (*value << 8) | *data++;
        size--;
    }

    /* Return value.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcp_server_store_data                          PORTABLE C      */ 
/*                                                           6.1          */
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
/*    _nx_dhcp_send_request                 Send DHCP request             */ 
/*    _nx_dhcp_add_option                   Add a DHCP option             */ 
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
static VOID  _nx_dhcp_server_store_data(UCHAR *data, UINT size, ULONG value)
{

    /* Make sure that data is left justified.  */
    switch (size)
    {
    
        case 1:

            value <<= 24;
            break;

        case 2:

            value <<= 16;
            break;

        case 3:
      
            value <<= 8;
            break;

        default:
            break;
    }

    /* Store the value.  */
    while (size-- > 0)
    {

        *data = (UCHAR)((value >> 24) & 0xff);
        data++;
        value <<= 8;
    }
}

#endif /* NX_DISABLE_IPV4 */
