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
/**   Dynamic Host Configuration Protocol over IPv6 (DHCPv6 Server)       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_DHCPV6_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include    <stdio.h> 
#include    "nx_api.h"
#include    "nx_system.h"
#include    "nx_ip.h"
#include    "nx_ipv6.h"
#include    "nx_udp.h"
#include    "nxd_dhcpv6_server.h"
#include    "tx_timer.h"

#ifdef FEATURE_NX_IPV6

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_server_create                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX create dhcpv6     */
/*    server service.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 server        */ 
/*    ip_ptr                              Pointer to server IP instance   */ 
/*    name_ptr                            DHCPV6 name pointer             */ 
/*    packet_pool_ptr                     Pointer to server packet pool   */ 
/*    stack_ptr                           Pointer to free memory          */
/*    stack_size                          Size of server stack memory     */
/*    dhcpv6_address_declined_handler     Declined address request handler*/
/*    dhcpv6_option_request_handler       Option request handler          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*    NX_DHCPV6_PARAM_ERROR               Invalid non pointer input       */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_create           DHCPV6 server create service     */ 
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
UINT  _nxe_dhcpv6_server_create(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_IP *ip_ptr, CHAR *name_ptr, 
                       NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr, ULONG stack_size,
                       VOID (*dhcpv6_address_declined_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UINT message),
                       VOID (*dhcpv6_option_request_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, UCHAR *buffer_ptr, UINT *index))
{

UINT    status;


    /* Check for invalid pointer input.  */
    if (!dhcpv6_server_ptr ||!ip_ptr || !packet_pool_ptr || !stack_ptr)
    {
    
        return NX_PTR_ERROR;
    }

    /* Check for invalid non pointer input. */
    if ((ip_ptr -> nx_ip_id != NX_IP_ID) || (stack_size < TX_MINIMUM_STACK))
    {

        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Call actual DHCPV6 server create service.  */
    status = _nx_dhcpv6_server_create(dhcpv6_server_ptr, ip_ptr, name_ptr, packet_pool_ptr, stack_ptr, stack_size,
                                       dhcpv6_address_declined_handler, dhcpv6_option_request_handler);                        

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_create                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates the DHCPv6 Client instance with a Netx packet */ 
/*    pool, processing thread, and various flag event queues, timers and  */
/*    mutexes necessary for DHCPv6 Client operations.                     */ 
/*                                                                        */ 
/*    Note: User is encouraged to call nx_dhcpv6_server_option_request    */ 
/*    _handler_set to set the option request handler.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 Server        */ 
/*    ip_ptr                              Pointer to Server IP instance   */ 
/*    name_ptr                            DHCPV6 name pointer             */ 
/*    packet_pool_ptr                     Pointer to Server packet pool   */ 
/*    stack_ptr                           Pointer to free memory          */
/*    stack_size                          Size of server stack memory     */
/*    dhcpv6_address_declined_handler     Declined address request handler*/
/*    dhcpv6_option_request_handler       Option request handler          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_create                Create ThreadX flag event queue*/
/*    tx_mutex_create                      Create mutex lock on resource  */
/*    nx_packet_pool_delete                Delete the DHCPV6 packet pool  */ 
/*    nx_udp_socket_create                 Create the DHCPV6 UDP socket   */ 
/*    nx_udp_socket_delete                 Delete the DHCPV6 UDP socket   */ 
/*    tx_mutex_create                      Create DHCPV6 mutex            */ 
/*    tx_mutex_delete                      Delete DHCPV6 mutex            */ 
/*    tx_thread_create                     Create DHCPV6 processing thread*/ 
/*    tx_thread_delete                     Delete DHCPV6 processing thread*/ 
/*    tx_timer_create                      Create DHCPV6 timer            */ 
/*    tx_timer_delete                      Delete DHCPV6 timer            */ 
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
UINT  _nx_dhcpv6_server_create(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_IP *ip_ptr, CHAR *name_ptr, 
                       NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr,  ULONG stack_size,
                       VOID (*dhcpv6_address_declined_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UINT message),
                       VOID (*dhcpv6_option_request_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, UCHAR *buffer_ptr, UINT *index))
{

UINT                i, status;
NX_DHCPV6_CLIENT    *dhcpv6_client_ptr;


    /* Initialize the DHCPV6 control block to zero.  */
    memset((void *) dhcpv6_server_ptr, 0, sizeof(NX_DHCPV6_SERVER));

    /* Default the server global index to index 1. If otherwise, the host
       application must set the DHCPv6 interface before it can start
       the Server (nx_dhcpv6_server_interface_set). */
    dhcpv6_server_ptr -> nx_dhcpv6_server_ga_address_index = 1;

    /* Link the DHCPv6 server with the IP instance. */
    dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr =  ip_ptr;

    /* Set the DHCPV6 name.  */
    dhcpv6_server_ptr -> nx_dhcpv6_server_name =  name_ptr;

    /* Set the server packet pool. */
    dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr = packet_pool_ptr;

    /* Create the IP timer event flag instance.  */
    status = tx_event_flags_create(&dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events, "DHCPv6 Server Timer Events Queue");

    /* Check for error. */
    if (status != TX_SUCCESS)
    {

        return status;
    }

    /* Create the DHCPV6 mutex for the Server instance.  */
    status =  tx_mutex_create(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex), "DHCPV6 Server Mutex", TX_NO_INHERIT);

    /* Determine if the mutexes creation was successful.  */
    if (status)
    {

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events);

        /* No, return error status.  */
        return status;
    }

    /* Create the DHCPV6 processing thread. */
    status =  tx_thread_create(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread), "NetX DHCPV6 Server", _nx_dhcpv6_server_thread_entry, 
                               (ULONG)(ALIGN_TYPE)dhcpv6_server_ptr, stack_ptr, stack_size, 
                               NX_DHCPV6_SERVER_THREAD_PRIORITY, NX_DHCPV6_SERVER_THREAD_PRIORITY, 1, TX_DONT_START);

    NX_THREAD_EXTENSION_PTR_SET(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread), dhcpv6_server_ptr)

    /* Determine if the thread creation was successful. */
    if (status)
    {

        /* Delete the server protection mutex.  */
        tx_mutex_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex));

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events);

        /* No, return error status.  */
        return status;
    }

    /* Create the DHCPV6 timer for keeping track of IP lease time expiration.  */
    status =  tx_timer_create(&(dhcpv6_server_ptr -> nx_dhcpv6_lease_timer), "NetX DHCPV6 Server Lease timer",
                              _nx_dhcpv6_server_lease_timeout_entry, (ULONG)(ALIGN_TYPE)dhcpv6_server_ptr,
                              (NX_DHCPV6_IP_LEASE_TIMER_INTERVAL * NX_DHCPV6_SERVER_TICKS_PER_SECOND) , 
                              (NX_DHCPV6_IP_LEASE_TIMER_INTERVAL * NX_DHCPV6_SERVER_TICKS_PER_SECOND), 
                              TX_NO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(dhcpv6_server_ptr -> nx_dhcpv6_lease_timer), dhcpv6_server_ptr)

    if (status != NX_SUCCESS)
    {

        /* Delete the DHCPv6 Server thread. */
        tx_thread_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread));

        /* Delete the server protection mutex.  */
        tx_mutex_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex));

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events);
    }

    /* Create the DHCPV6 timer for keeping track of the DHCPv6 Client session time.  */
    status =  tx_timer_create(&(dhcpv6_server_ptr -> nx_dhcpv6_session_timer), "NetX DHCPV6 Session Duration timer",
                              _nx_dhcpv6_server_session_timeout_entry, (ULONG)(ALIGN_TYPE)dhcpv6_server_ptr,
                              (NX_DHCPV6_SESSION_TIMER_INTERVAL * NX_DHCPV6_SERVER_TICKS_PER_SECOND),
                              (NX_DHCPV6_SESSION_TIMER_INTERVAL * NX_DHCPV6_SERVER_TICKS_PER_SECOND), 
                              TX_NO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(dhcpv6_server_ptr -> nx_dhcpv6_session_timer), dhcpv6_server_ptr)

    if (status != NX_SUCCESS)
    {

        /* Delete the server lease timer. */
        tx_timer_delete( &(dhcpv6_server_ptr -> nx_dhcpv6_lease_timer));

        /* Delete the DHCPv6 Server thread. */
        tx_thread_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread));

        /* Delete the server protection mutex.  */
        tx_mutex_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex));

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events);
    }

    /*  Initialize the DHCPv6 client control blocks */
    for (i = 0; i < NX_DHCPV6_MAX_CLIENTS; i++)
    {

        /* Set a local pointer for convenience. */
        dhcpv6_client_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_clients[i];

        /* Clear the client record. */
        memset(dhcpv6_client_ptr, 0, sizeof(NX_DHCPV6_CLIENT));
    }

    /* Determine if the timers creation were successful.  */
    if (status)
    {

        /* Delete the server lease timer. */
        tx_timer_delete( &(dhcpv6_server_ptr -> nx_dhcpv6_lease_timer));

        /* Delete the server serssion timer. */
        tx_timer_delete( &(dhcpv6_server_ptr -> nx_dhcpv6_session_timer));

        /* Delete the DHCPv6 Server thread. */
        tx_thread_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread));

        /* Delete the server protection mutex.  */
        tx_mutex_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex));

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events);

        /* Return error status. */
        return status;
    }

    /* Save the DHCPV6 instance pointer in the socket. */
    dhcpv6_server_ptr -> nx_dhcpv6_server_socket.nx_udp_socket_reserved_ptr =  (void *) dhcpv6_server_ptr;

    /* Set the DHCPv6 Client ID.  */
    dhcpv6_server_ptr -> nx_dhcpv6_id =  NX_DHCPV6_SERVER_ID;

    /* Set the server preference for clients using multicast only for all request message types.  */
    dhcpv6_server_ptr -> nx_dhcpv6_server_multicast_only = NX_TRUE;

    /* Assign the various handlers (server error messages, state change and expired/deprecated addresses). */
    dhcpv6_server_ptr -> nx_dhcpv6_IP_address_declined_handler =  dhcpv6_address_declined_handler;
    dhcpv6_server_ptr -> nx_dhcpv6_server_option_request_handler = dhcpv6_option_request_handler;

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_server_option_request_handler_set       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX set dhcpv6 server */
/*    option request callback service.                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 server        */ 
/*    dhcpv6_option_request_handler_extended                              */ 
/*                                        Extended option request handler */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_option_request_handler_set                        */ 
/*                                        DHCPV6 server create service    */ 
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
UINT  _nxe_dhcpv6_server_option_request_handler_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr,
                                                    VOID (*dhcpv6_option_request_handler_extended)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, 
                                                                                                   UCHAR *buffer_ptr, UINT *index, UINT available_payload))
{

UINT    status;


    /* Check for invalid pointer input.  */
    if ((!dhcpv6_server_ptr) || (!dhcpv6_option_request_handler_extended))
    {
        return NX_PTR_ERROR;
    }

    /* Call actual DHCPV6 server option request handler set service.  */
    status = _nx_dhcpv6_server_option_request_handler_set(dhcpv6_server_ptr, dhcpv6_option_request_handler_extended);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_option_request_handler_set        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the DHCPv6 option request handler.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 server        */ 
/*    dhcpv6_option_request_handler_extended                              */ 
/*                                        Extended option request handler */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*    NX_SUCCESS                            Successful completion status  */
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
UINT  _nx_dhcpv6_server_option_request_handler_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr,
                                                   VOID (*dhcpv6_option_request_handler_extended)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, 
                                                                                                  UCHAR *buffer_ptr, UINT *index, UINT available_payload))
{


    /* Set the extended handler callback.  */
    dhcpv6_server_ptr -> nx_dhcpv6_server_option_request_handler_extended = dhcpv6_option_request_handler_extended;

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_add_client_record                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking for the copy client data       */
/*   service.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    table_index                        Table index to store client data */
/*    message_xid                        Client transaction ID            */
/*    client_address                     IP address leased to client      */
/*    client_state                       Client DHCPv6 state e.g. bound   */
/*    IP_lease_time_accrued              Time expired on current IP lease */
/*    valid_lifetime                     When address becomes invalid     */
/*    duid_type                          Type of Client DUID              */
/*    duid_hardware                      DUID hardware (usually ethernet) */
/*    physical_address_msw               MSB of client mac address        */
/*    physical_address_lsw               LSB of client mac address        */
/*    duid_time                          Time for link layer time DUID    */
/*    duid_vendor_number                 Enterprise ID for Vendor DUID    */
/*    duid_vendor_private                Pointer to private ID for DUID   */
/*    duid_private_length                Vendor Duid private ID length    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address copy       */
/*    NX_DHCPV6_PARAM_ERROR              Invalid non pointer input        */     
/*    NX_DHCPV6_TABLE_FULL               No empty slot for client data    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_client_record       Actual copy client data service  */ 
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
UINT _nxe_dhcpv6_add_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG message_xid, NXD_ADDRESS *client_address, UINT client_state, 
                                  ULONG IP_lease_time_accrued, ULONG valid_lifetime, UINT duid_type, UINT duid_hardware, ULONG physical_address_msw, 
                                  ULONG physical_address_lsw, ULONG duid_time, ULONG duid_vendor_number, UCHAR *duid_vendor_private, UINT duid_private_length)
{

UINT status;

    /* Check for invalid pointer input. */
    if ((dhcpv6_server_ptr == NX_NULL) || (client_address == NX_NULL))
    {

        return NX_PTR_ERROR;
    }

    /* Check for missing transaction ID or DUID type. */
    if ((message_xid == 0) || (duid_type == 0))
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check for missing time expired on the lease (if client in a bound state). */
    if ((IP_lease_time_accrued == 0) && (client_state == NX_DHCPV6_STATE_BOUND))
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check for invalid enterprise DUID data if type is enterprise. */
    if (duid_type == NX_DHCPV6_SERVER_DUID_TYPE_VENDOR_ASSIGNED)
    {

        if ((duid_private_length == 0) || (duid_vendor_private == NX_NULL))
        {

            return NX_INVALID_PARAMETERS;
        }
    }
    /* Check for invalid LINK LAYER duid data. */
    else if ((duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME) || (duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_ONLY))
    {

        if ((physical_address_msw == 0) && (physical_address_lsw == 0))
        {
            return NX_INVALID_PARAMETERS;
        }

        if ((duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME) && (duid_time == 0))
        {
            return NX_INVALID_PARAMETERS;
        }
    }

    /* Call the actual service and return completion status. */
    status =  _nx_dhcpv6_add_client_record(dhcpv6_server_ptr, table_index, message_xid, client_address, client_state, 
                                           IP_lease_time_accrued, valid_lifetime, duid_type, duid_hardware, physical_address_msw, physical_address_lsw, 
                                           duid_time, duid_vendor_number, duid_vendor_private, duid_private_length);
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_client_record                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function copies the input data into a DHCPv6 Client record and  */
/*   stored in the server's Client table.  It then uses the Client IP     */
/*   address to find the matching record in the server IP lease table to  */
/*   cross link the tables.                                               */
/*                                                                        */ 
/*   This is intended to satisfy the DHCPv6 protocol requirement that a   */
/*   DHPCv6 Server be able to store and retrieve lease data from          */
/*   nonvolatile memory e..g between reboots.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    table_index                        Table index to store client data */
/*    message_xid                        Client transaction ID            */
/*    client_address                     IP address leased to client      */
/*    client_state                       Client DHCPv6 state e.g. bound   */
/*    IP_lease_time_accrued              Time expired on current IP lease */
/*    valid_lifetime                     When address becomes invalid     */
/*    duid_type                          Type of Client DUID              */
/*    duid_hardware                      DUID hardware (usually ethernet) */
/*    physical_address_msw               MSB of client mac address        */
/*    physical_address_lsw               LSB of client mac address        */
/*    duid_time                          Time for link layer time DUID    */
/*    duid_vendor_number                 Enterprise ID for Vendor DUID    */
/*    duid_vendor_private                Pointer to private ID for DUID   */
/*    duid_private_length                Vendor Duid private ID length    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address copy       */
/*    NX_DHCPV6_PARAM_ERROR              Invalid non pointer input        */     
/*    NX_DHCPV6_TABLE_FULL               No empty slot for client data    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcpy                             Copy data to specified memory    */ 
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
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_add_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG message_xid, NXD_ADDRESS *client_address, UINT client_state, 
                                  ULONG IP_lease_time_accrued, ULONG valid_lifetime, UINT duid_type,  UINT duid_hardware, ULONG physical_address_msw, 
                                  ULONG physical_address_lsw, ULONG duid_time, ULONG duid_vendor_number, UCHAR *duid_vendor_private, UINT duid_private_length)
{

UINT i;
UINT found = NX_FALSE;
NX_DHCPV6_CLIENT *dhcpv6_client_ptr; 


    /* Check if a table index was specified. */
    if (table_index != 0xFFFFFFFF)
    {

        /* Check if it is a valid index. */
        if (table_index >= NX_DHCPV6_MAX_CLIENTS)
        {
        
            /* No, return a parameter error. */
            return NX_INVALID_PARAMETERS;
        }

        /* Check if it is actually empty. */
        if (dhcpv6_server_ptr -> nx_dhcpv6_clients[table_index].nx_dhcpv6_message_xid != 0)
        {

            /* Not empty. Indicate we'll have to find an empty slot. */
            table_index = 0xFFFFFFFF;
        }
    }

    /* Check if we still need to find a slot. */
    if (table_index == 0xFFFFFFFF)
    {

        /* We do, so loop through client records to find one. */
        for (i = 0; i < NX_DHCPV6_MAX_CLIENTS; i++)
        {

            /* Is this an empty slot? */
            if (dhcpv6_server_ptr -> nx_dhcpv6_clients[i].nx_dhcpv6_message_xid == 0)
            {

                /* This is available. */
                table_index = i;
                break;
            }
        }

        /* Do we have a slot?  */
        if (i == NX_DHCPV6_MAX_CLIENTS)
        {

            /* No, Indicate the table is full. */
            return NX_DHCPV6_TABLE_FULL;
        }
    }

    /* Create the table entry. */

    /* Fill in the generic client information. */
    dhcpv6_client_ptr =  &(dhcpv6_server_ptr -> nx_dhcpv6_clients[table_index]);
    dhcpv6_client_ptr -> nx_dhcpv6_id = NX_DHCPV6_CLIENT_ID;
    dhcpv6_client_ptr -> nx_dhcpv6_message_xid = message_xid;
    dhcpv6_client_ptr -> nx_dhcpv6_state = (UCHAR)client_state;
    dhcpv6_client_ptr -> nx_dhcpv6_IP_lease_time_accrued  = IP_lease_time_accrued;
    memcpy(&(dhcpv6_client_ptr -> nx_dhcpv6_server_duid), &(dhcpv6_server_ptr -> nx_dhcpv6_server_duid), sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */

    /*Fill in the Client DUID information. */
    dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_op_code = NX_DHCPV6_OP_DUID_CLIENT;
    dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_hardware_type = (USHORT)duid_hardware;
    dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_duid_type = (USHORT)duid_type;

    /* Determine DUID type to fill in the rest of the DUID fields. */
    if (duid_type == NX_DHCPV6_SERVER_DUID_TYPE_VENDOR_ASSIGNED)
    {

        /* Validate the DUID private length. */
        if (duid_private_length > NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH)
        {
            return NX_DHCPV6_INVALID_DUID;
        }

        dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_duid_enterprise_number = duid_vendor_number;
        memcpy(&(dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_duid_private_identifier[0]), duid_vendor_private, duid_private_length); /* Use case of memcpy is verified. */
        dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_option_length = (USHORT)(duid_private_length + 6);
    }
    else 
    {

        /* Either link layer or link layer plus time. */
        dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw = (USHORT)physical_address_msw;
        dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_lsw = physical_address_lsw;
        dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_option_length = 10;

        if (duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME)
        {
        
            dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_option_length = 14;
            dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_duid_time = duid_time;
        }
    }

    /* Fill in the necessary IA-NA and IA Address option fields. */
    dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_op_code = NX_DHCPV6_OP_IA_ADDRESS;
    dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_option_length = 24;
    dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = valid_lifetime;
    dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = 0; /* Don't care */
    COPY_NXD_ADDRESS(client_address, &(dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address));

    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_op_code = NX_DHCPV6_OP_OPTION_STATUS;
    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_option_length = 2; /* No message */
    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_SUCCESS;

    /* Now access the server lease table to find this client's lease. */
    for (i = 0; i < NX_DHCPV6_MAX_LEASES; i++)
    {

        /* Compare our client's assigned IP address with this lease table entry. */
        if(CHECK_IPV6_ADDRESSES_SAME(&(client_address -> nxd_ip_address.v6[0]), 
                                     &(dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_IP_address.nxd_ip_address.v6[0])))
        {

            /* They match! Cross link the tables. */
            dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_assigned_to = dhcpv6_client_ptr;
            found = NX_TRUE;
            break;
        }
    }

    if (!found)
    {

        /* This client apparently does not have a valid IP lease. Ok to keep in the tables,
           but they will need to request an IP address! */
        return NX_DHCPV6_ADDRESS_NOT_FOUND;
    }

    return NX_SUCCESS;

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_retrieve_client_record                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the retrieve DHCPv6 Client  */
/*   record service.                                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    table_index                        Table index to store client data */
/*    message_xid                        Client transaction ID            */
/*    client_address                     IP address leased to client      */
/*    client_state                       Client DHCPv6 state e.g. bound   */
/*    IP_lease_time_accrued              Time expired on current IP lease */
/*    valid_lifetime                     When address becomes invalid     */
/*    duid_type                          Type of Client DUID              */
/*    duid_hardware                      DUID hardware (usually ethernet) */
/*    physical_address_msw               MSB of client mac address        */
/*    physical_address_lsw               LSB of client mac address        */
/*    duid_time                          Time for link layer time DUID    */
/*    duid_vendor_number                 Enterprise ID for Vendor DUID    */
/*    duid_vendor_private                Pointer to private ID for DUID   */
/*    duid_private_length                Vendor Duid private ID length    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address copy       */
/*    NX_INVALID_PARAMETERS              Invalid non pointer input        */     
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_retrieve_client_record  Actual retrieve client data      */
/*                                         service                        */ 
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
UINT _nxe_dhcpv6_retrieve_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG *message_xid, NXD_ADDRESS *client_address, UINT *client_state, 
                                  ULONG *IP_lease_time_accrued, ULONG *valid_lifetime, UINT *duid_type,  UINT *duid_hardware, ULONG *physical_address_msw, 
                                  ULONG *physical_address_lsw, ULONG *duid_time, ULONG *duid_vendor_number, UCHAR *duid_vendor_private, UINT *duid_private_length)
{

UINT status;


    /* Check for invalid parameters. */
    if (table_index >= NX_DHCPV6_MAX_CLIENTS)
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check for invalid pointer input. */
    if ((dhcpv6_server_ptr == NX_NULL) || (message_xid == NX_NULL) || (client_address == NX_NULL) || (client_state == NX_NULL) ||
        (IP_lease_time_accrued == NX_NULL) || (valid_lifetime == NX_NULL) || (duid_type == NX_NULL))
    {

        return NX_PTR_ERROR;
    }


    /* Call the actual service. */
    status = _nx_dhcpv6_retrieve_client_record(dhcpv6_server_ptr, table_index, message_xid, client_address, client_state, 
                                      IP_lease_time_accrued, valid_lifetime, duid_type,  duid_hardware, physical_address_msw, 
                                      physical_address_lsw, duid_time, duid_vendor_number, duid_vendor_private, duid_private_length);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_retrieve_client_record                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function returns DHCPv6 Client record data at the specified     */
/*   index into the server table to the caller.  Note that if a slot in   */
/*   the server's client table is empty, evidenced by a zero transaction  */
/*   ID, the service returns null data in all parameters with a successful*/
/*   completion status.                                                   */
/*                                                                        */ 
/*   This is intended to satisfy the DHCPv6 protocol requirement that a   */
/*   DHPCv6 Server be able to store and retrieve IP lease data from       */
/*   nonvolatile memory e..g between reboots.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    table_index                        Table index to store client data */
/*    message_xid                        Client transaction ID            */
/*    client_address                     IP address leased to client      */
/*    client_state                       Client DHCPv6 state e.g. bound   */
/*    IP_lease_time_accrued              Time expired on current IP lease */
/*    valid_lifetime                     When address becomes invalid     */
/*    duid_type                          Type of Client DUID              */
/*    duid_hardware                      DUID hardware (usually ethernet) */
/*    physical_address_msw               MSB of client mac address        */
/*    physical_address_lsw               LSB of client mac address        */
/*    duid_time                          Time for link layer time DUID    */
/*    duid_vendor_number                 Enterprise ID for Vendor DUID    */
/*    duid_vendor_private                Pointer to private ID for DUID   */
/*    duid_private_length                Vendor Duid private ID length    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address copy       */
/*    NX_PTR_ERROR                       Invalid pointer input            */
/*    NX_INVALID_PARAMETERS              Invalid non pointer input        */     
/*    NX_DHCPV6_INVALID_DUID             Malformed DUID retrieved         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    COPY_NXD_ADDRESS                   Copy IPv6 address to specified   */
/*                                         memory (holding an IPv6 address*/ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), corrected*/
/*                                            the length of identifier,   */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_retrieve_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG *message_xid, NXD_ADDRESS *client_address, UINT *client_state, 
                                  ULONG *IP_lease_time_accrued, ULONG *valid_lifetime, UINT *duid_type,  UINT *duid_hardware, ULONG *physical_address_msw, 
                                  ULONG *physical_address_lsw, ULONG *duid_time, ULONG *duid_vendor_number, UCHAR *duid_vendor_private, UINT *duid_private_length)
{

NX_DHCPV6_CLIENT *client_ptr;
INT              length;


    /* Get a local pointer for convenience. */
    client_ptr = &(dhcpv6_server_ptr ->nx_dhcpv6_clients[table_index]);

    /* Load the generic client data. */
    *message_xid = client_ptr -> nx_dhcpv6_message_xid;

    /* Is this an empty record? */
    if (*message_xid == 0)
    {

        /* Yes so return null data. */
        memset(client_address, 0, sizeof(NXD_ADDRESS));
        *client_state = 0;
        *IP_lease_time_accrued = 0;
        *valid_lifetime = 0;
        *duid_type  = 0;
        if (duid_hardware)
            *duid_hardware = 0;
        if (physical_address_msw)
            *physical_address_msw = 0;
        if (physical_address_lsw)
            *physical_address_lsw = 0;
        if (duid_time)
            *duid_time = 0;
        if (duid_vendor_number)
            *duid_vendor_number = 0;
        if (duid_private_length)
            *duid_private_length = 0;

        return NX_SUCCESS;
    }

    /* Not an empty record. Extract the data! */
    COPY_NXD_ADDRESS(&(client_ptr -> nx_dhcpv6_ia.nx_global_address), client_address);
    *client_state = client_ptr -> nx_dhcpv6_state;
    *IP_lease_time_accrued = client_ptr -> nx_dhcpv6_IP_lease_time_accrued;
    *valid_lifetime = client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime;
    *duid_type  = client_ptr -> nx_dhcpv6_client_duid.nx_duid_type;

    /* If this is a LINK LAYER Duid type, load the appropriate data. */
    if ((*duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_ONLY) || (*duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME))
    {

        /* Check for invalid pointer input for DUID of the link layer type. */
        if ((duid_hardware == NX_NULL) || (physical_address_msw == NX_NULL) || (physical_address_lsw == NX_NULL))
        {
            return NX_PTR_ERROR;
        }

        *duid_hardware = client_ptr -> nx_dhcpv6_client_duid.nx_hardware_type;
        *physical_address_msw = client_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw;
        *physical_address_lsw = client_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_lsw;

        if (*duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME)
        {

            if (duid_time == NX_NULL)
            {
                return NX_PTR_ERROR;
            }

            *duid_time = client_ptr -> nx_dhcpv6_client_duid.nx_duid_time;
        }
    }
    else
    {

        /* Client DUID type is vendor assigned. */

        /* Check for valid pointer input for enterprise ID data */
        if ((duid_vendor_number == NX_NULL) || (duid_vendor_private == NX_NULL) || (duid_private_length == NX_NULL))
        {
            return NX_PTR_ERROR;
        }

       *duid_vendor_number = client_ptr -> nx_dhcpv6_client_duid.nx_duid_enterprise_number;
       length = client_ptr -> nx_dhcpv6_client_duid.nx_option_length - 6;
       if (length < 0)
       {
           return NX_DHCPV6_INVALID_DUID;
       }

       /* NOTE: The duid_vendor_private buffer is provided by user, and the buffer size must be not less than NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH.  */
       memcpy(duid_vendor_private, &(client_ptr -> nx_dhcpv6_client_duid.nx_duid_private_identifier[0]), (UINT)length); /* Use case of memcpy is verified. The buffer is provided by user.  */
       *duid_private_length = (UINT)length;
    }

   return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_add_ip_address_lease                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking for the copy lease service.    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    table_index                        Table index to store data        */
/*    lease_IP_address                   Lease IP address                 */
/*    T1                                 T1 lifetime (when to renew)      */
/*    T2                                 T2 lifetime (when to rebind)     */
/*    preferred_lifetime                 When address becomes deprecated  */
/*    valid_lifetime                     When address becomes invalid     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address copy       */
/*    NX_DHCPV6_PARAM_ERROR              Invalid non pointer input        */     
/*    NX_PTR_ERROR                       Invalid pointer input            */
/*    NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS                              */ 
/*                                       Address not onlink with Server   */
/*                                         DHCPv6 network interface       */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_ip_address_lease    Actual copy lease data service   */ 
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
UINT _nxe_dhcpv6_add_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, 
                                      ULONG T1, ULONG T2, ULONG valid_lifetime, ULONG preferred_lifetime)
{

UINT status;


    /* Check for invalid pointer input. */
    if ((dhcpv6_server_ptr == NX_NULL) || (lease_IP_address == NX_NULL))
    {

        return NX_PTR_ERROR;
    }

    /* Check for invalid non pointer input. */
    if (CHECK_UNSPECIFIED_ADDRESS(&(lease_IP_address -> nxd_ip_address.v6[0])))
    {

        return NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS;
    }

    /* Check for invalid lease time data. */
    if ((T1 == 0) || (T2 == 0) || (valid_lifetime == 0) || (preferred_lifetime == 0))
    {

        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Call the actual service and return completion status. */
    status = _nx_dhcpv6_add_ip_address_lease(dhcpv6_server_ptr, table_index, lease_IP_address,
                                              T1, T2, valid_lifetime, preferred_lifetime);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_ip_address_lease                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function copies lease data into the Server IP lease table.      */
/*   The caller can specify which slot with a index value up to the size  */
/*   of the lease table, (NX_DHCPV6_MAX_LEASES - 1), or the DHCPv6 server */
/*   will find an empty slot to store the lease data if the table index is*/
/*   set to 'infinity' e.g. 0xFFFFFFFF.                                   */
/*                                                                        */ 
/*   This is intended to satisfy the DHCPv6 protocol requirement that a   */
/*   DHPCv6 Server be able to store and retrieve IP lease data from       */
/*   nonvolatile memory e..g between reboots.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    table_index                        Table index to store data        */
/*    lease_IP_address                   Lease IP address                 */
/*    T1                                 T1 lifetime (when to renew)      */
/*    T2                                 T2 lifetime (when to rebind)     */
/*    preferred_lifetime                 When address becomes deprecated  */
/*    valid_lifetime                     When address becomes invalid     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address copy       */
/*    NX_DHCPV6_PARAM_ERROR              Invalid table index              */     
/*    NX_DHCPV6_TABLE_FULL               No empty entries for lease data  */
/*    NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS                              */ 
/*                                       Address not onlink with Server   */
/*                                         DHCPv6 network interface       */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
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
UINT _nx_dhcpv6_add_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, 
                                       ULONG T1, ULONG T2, ULONG valid_lifetime, ULONG preferred_lifetime)
{

UINT i;
NX_DHCPV6_ADDRESS_LEASE *lease_ptr;
UINT                    ga_address_index;
UINT                    prefix_length;


    /* Get the DHCPv6 server address interface index. */
    ga_address_index = dhcpv6_server_ptr -> nx_dhcpv6_server_ga_address_index; 

    prefix_length = dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address_prefix_length;

    /* Verify the input addresses match the server interface address prefix (in IPv4 terms, the
       network masked addresses should be equal). */

    if (!CHECK_IP_ADDRESSES_BY_PREFIX(&(dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address[0]),
                                       &lease_IP_address -> nxd_ip_address.v6[0], prefix_length))
    {

        return NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS;
    }

    /* Is there a table index specified? */
    if (table_index == 0xFFFFFFFF)
    {

        /* No, so lets search the server table and find an available slot. */
        for (i = 0; i < NX_DHCPV6_MAX_LEASES; i++)
        {

            /* Get a local pointer for convenience. */
            lease_ptr = &(dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i]);

            /* Does this slot have an address already? */
            if (CHECK_UNSPECIFIED_ADDRESS(&(lease_ptr -> nx_dhcpv6_lease_IP_address.nxd_ip_address.v6[0])))
            {
    
                /* This one looks empty. */
                table_index = i;
                break;
            }
        }

        if (table_index == 0xFFFFFFFF)
        {
            return NX_DHCPV6_TABLE_FULL;
        }
    }
    else
    {
        /* Check for valid table index. */
        if (table_index >= NX_DHCPV6_MAX_LEASES)
        {

            /* No good. Exceeds the size of the table. */
            return NX_DHCPV6_PARAM_ERROR;
        }
    }

    /* Copy the supplied IP data into the lease entry. */
    lease_ptr = &(dhcpv6_server_ptr -> nx_dhcpv6_lease_list[table_index]);

    COPY_NXD_ADDRESS(lease_IP_address, &(lease_ptr -> nx_dhcpv6_lease_IP_address));
    lease_ptr -> nx_dhcpv6_lease_T1_lifetime = T1;                     
    lease_ptr -> nx_dhcpv6_lease_T2_lifetime = T2;                     
    lease_ptr -> nx_dhcpv6_lease_valid_lifetime = valid_lifetime;              
    lease_ptr -> nx_dhcpv6_lease_preferred_lifetime =preferred_lifetime;   

    dhcpv6_server_ptr -> nx_dhcpv6_assignable_addresses++;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_retrieve_ip_address_lease               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking for the actual retrieve lease  */
/*   service.                                                             */
/*                                                                        */ 
/*   This is intended to satisfy the DHCPv6 protocol requirement that a   */
/*   DHPCv6 Server be able to store and retrieve lease data from          */
/*   nonvolatile memory e..g between reboots.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    table_index                        Table index to obtain data       */
/*    lease_IP_address                   Lease IP address                 */
/*    T1                                 T1 lifetime (when to renew)      */
/*    T2                                 T2 lifetime (when to rebind)     */
/*    preferred_lifetime                 When address becomes deprecated  */
/*    valid_lifetime                     When address becomes invalid     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address copy       */
/*    NX_PTR_ERROR                      Invalid pointer input             */     
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_retrieve_ip_address_lease                                */ 
/*                                       Actual retrieve data service     */
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
UINT _nxe_dhcpv6_retrieve_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, 
                                       ULONG *T1, ULONG *T2, ULONG *valid_lifetime, ULONG *preferred_lifetime)
{

UINT status;


    /* Check for invalid pointer input. */
    if ((dhcpv6_server_ptr == NX_NULL) || (lease_IP_address == NX_NULL) || (T1 == NX_NULL) ||
        (T2 == NX_NULL) || (valid_lifetime == NX_NULL) || (preferred_lifetime == NX_NULL))
    {

        return NX_PTR_ERROR;
    }

    status = _nx_dhcpv6_retrieve_ip_address_lease(dhcpv6_server_ptr, table_index, lease_IP_address, 
                                       T1, T2, valid_lifetime, preferred_lifetime);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_retrieve_ip_address_lease                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function retrieves lease data from the Server IP lease table.   */
/*   The caller must specify which slot with a index value up to the size */
/*   of the lease table, (NX_DHCPV6_MAX_LEASES - 1).                      */
/*                                                                        */ 
/*   This is intended to satisfy the DHCPv6 protocol requirement that a   */
/*   DHPCv6 Server be able to store and retrieve lease data from          */
/*   nonvolatile memory e..g between reboots.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    table_index                        Table index to obtain data       */
/*    lease_IP_address                   Lease IP address                 */
/*    T1                                 T1 lifetime (when to renew)      */
/*    T2                                 T2 lifetime (when to rebind)     */
/*    preferred_lifetime                 When address becomes deprecated  */
/*    valid_lifetime                     When address becomes invalid     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address copy       */
/*    NX_DHCPV6_PARAM_ERROR              Invalid table index              */     
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
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
UINT _nx_dhcpv6_retrieve_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, 
                                       ULONG *T1, ULONG *T2, ULONG *valid_lifetime, ULONG *preferred_lifetime)
{

NX_DHCPV6_ADDRESS_LEASE *lease_ptr;


    /* Check for valid table index. */
    if (table_index >= NX_DHCPV6_MAX_LEASES)
    {

        /* No good. Exceeds the size of the table. */
        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Get a local pointer for convenience. */
    lease_ptr = &(dhcpv6_server_ptr -> nx_dhcpv6_lease_list[table_index]);

    /* Copy the lease data into the input pointer buffers.  */
    COPY_NXD_ADDRESS(&(lease_ptr -> nx_dhcpv6_lease_IP_address), lease_IP_address);
    *T1 = lease_ptr -> nx_dhcpv6_lease_T1_lifetime;                     
    *T2 = lease_ptr -> nx_dhcpv6_lease_T2_lifetime;                     
    *valid_lifetime = lease_ptr -> nx_dhcpv6_lease_valid_lifetime;              
    *preferred_lifetime = lease_ptr -> nx_dhcpv6_lease_preferred_lifetime;   

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_server_delete                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX delete dhcpv6     */
/*    server service.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_delete            Actual DHCPV6 delete function   */ 
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
UINT  _nxe_dhcpv6_server_delete(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((dhcpv6_server_ptr == NX_NULL) || (dhcpv6_server_ptr -> nx_dhcpv6_id != NX_DHCPV6_SERVER_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 server delete service.  */
    status =  _nx_dhcpv6_server_delete(dhcpv6_server_ptr);

    /* Return commpletion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_delete                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the DHCPV6 Client instance and releases all of*/
/*    NetX and ThreadX resources.                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                    Pointer to Client instance     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful Completion status  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_unbind                  Release DHCPV6 UDP socket port*/ 
/*    nx_udp_socket_delete                  Delete the DHCPV6 UDP socket  */ 
/*    tx_thread_terminate                   Terminate DHCPV6 thread       */ 
/*    tx_thread_delete                      Delete DHCPV6 thread          */ 
/*    tx_timer_delete                       Delete DHCPV6 timers          */ 
/*    tx_mutex_delete                       Delete DHCPV6 mutexes         */
/*    tx_event_flags_delete                 Delete DHCPV6 flag queue      */ 
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
UINT  _nx_dhcpv6_server_delete(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{

    /* Terminate the DHCPV6 processing thread.  */
    tx_thread_terminate(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread));

    /* Delete the DHCPV6 processing thread.  */
    tx_thread_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread));

    /* Delete the flag event queue. */
    tx_event_flags_delete(&dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events);

    /* Delete the timers */
    tx_timer_delete(&(dhcpv6_server_ptr->nx_dhcpv6_lease_timer));
    tx_timer_delete(&(dhcpv6_server_ptr->nx_dhcpv6_session_timer));

    /* Delete the mutexes.  */
    tx_mutex_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex));

    /* Release the UDP socket port. */
    nx_udp_socket_unbind(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

    /* Delete the UDP socket.  */
    nx_udp_socket_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

    /* Clear the dhcpv6 structure ID. */
    dhcpv6_server_ptr -> nx_dhcpv6_id =  0;

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_create_ip_address_range                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the create address list     */
/*   service.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    start_ipv6_address                 Start of IP address in list.     */
/*    end_ipv6_address                   End of IP address in list.       */
/*    addresses_added                    Number of addresses added to list*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP list creation      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_dhcpv6_create_ip_address_range  Actual list create service       */ 
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
UINT  _nxe_dhcpv6_create_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr,  
                                         NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address,
                                         UINT *addresses_added)
{

UINT     status;


    /* Check for invalid input.  */
    if ((dhcpv6_server_ptr == NX_NULL) || 
        (start_ipv6_address == NX_NULL)|| (end_ipv6_address == NX_NULL)  ||
        (addresses_added == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid non pointer input. */
    if ((CHECK_UNSPECIFIED_ADDRESS(&start_ipv6_address -> nxd_ip_address.v6[0])) ||
        (CHECK_UNSPECIFIED_ADDRESS(&end_ipv6_address -> nxd_ip_address.v6[0])))
    {

        return(NX_DHCPV6_INVALID_IP_ADDRESS);                                           
    }

    /* Check for an invalid address range. */
    if (start_ipv6_address -> nxd_ip_address.v6[3] > end_ipv6_address -> nxd_ip_address.v6[3])
    {

        return(NX_DHCPV6_INVALID_IP_ADDRESS);                                           
    }

    status = _nx_dhcpv6_create_ip_address_range(dhcpv6_server_ptr, start_ipv6_address, end_ipv6_address, addresses_added);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_create_ip_address_range                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function creates a list of IPv6 addresses the DHCPv6 server     */
/*   may assign to clients, using a start and end address.  It returns the*/
/*   number of addresses actually added, which may be limited by the size */
/*   of the server address table.                                         */
/*                                                                        */ 
/*   Note: The NetX Duo DHCPv6 server accomodates up to 256 assignable    */
/*   address ranges in the current release.                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    start_ipv6_address                 Start of IP address in list.     */
/*    end_ipv6_address                   End of IP address in list.       */
/*    addresses_added                    Number of addresses added to list*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP list creation      */
/*    NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS                              */
/*                                       Supplied address not reachable   */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
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
UINT  _nx_dhcpv6_create_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr,  
                                         NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address,
                                         UINT *addresses_added)
{

UINT                         i;
NX_IP                        *ip_ptr;
UINT                         prefix_length;
NXD_ADDRESS                  next_ipv6_address;
UINT                         ga_address_index;


    *addresses_added = 0;

    COPY_NXD_ADDRESS(start_ipv6_address, &next_ipv6_address);

    /* Set up a local variable for convenience. */
    ip_ptr = dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr;
    ga_address_index = dhcpv6_server_ptr -> nx_dhcpv6_server_ga_address_index;      
    prefix_length = ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address_prefix_length;

    /* Verify the input addresses match the server interface address prefix (in IPv4 terms, the
       network masked addresses should be equal). */
    if (!CHECK_IP_ADDRESSES_BY_PREFIX(&(ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address[0]),
                                       &start_ipv6_address -> nxd_ip_address.v6[0], prefix_length))
    {

        return NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS;
    }

    if (!CHECK_IP_ADDRESSES_BY_PREFIX(&(ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address[0]),
                                       &end_ipv6_address -> nxd_ip_address.v6[0], prefix_length))
    {

        return NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS;
    }

    /* Set the server ip address based on the specified interface. */

    /* Clear out existing entries and start adding IP addresses at the beginning of the list. */
    i = 0;

    /* Fit as many IP addresses in the specified range as will fit in the table. */
    while (i < NX_DHCPV6_MAX_LEASES)
    {

        NX_DHCPV6_ADDRESS_LEASE *ip_lease_ptr; 

        /* Set local pointer to the next entry for convenience. */
        ip_lease_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i];

        /* Initialize each entry before adding data.  */
        memset(&dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i], 0, sizeof(NX_DHCPV6_ADDRESS_LEASE));

        /* Add the IP address into this slot. */
        COPY_NXD_ADDRESS(&next_ipv6_address, &ip_lease_ptr -> nx_dhcpv6_lease_IP_address);

        /* Update the number of addresses added. */
        (*addresses_added)++;

        /* Increase the address low word by one. */
        next_ipv6_address.nxd_ip_address.v6[3]++;

        /* Check if we are overflowing the last word. */
        if (next_ipv6_address.nxd_ip_address.v6[3] == 0xFFFFFFFF)
        {

            /* We are. Let's stop here. */
            break;
        }

        if (CHECK_IPV6_ADDRESSES_SAME(&next_ipv6_address.nxd_ip_address.v6[0], &end_ipv6_address -> nxd_ip_address.v6[0]))
        {
            break;
        }

        i++;
    }

    /* Indicate how many addresses were actually added to the server list. */
    dhcpv6_server_ptr -> nx_dhcpv6_assignable_addresses = *addresses_added;

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_reserve_ip_address_range                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function performs error checking on the reserve address list    */
/*   service.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    start_ipv6_address                 Start of IP address to reserve   */
/*    end_ipv6_address                   End of IP address to reserve     */
/*    addresses_reserved                 Number of addresses reserved     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP list creation      */
/*    NX_DHCPV6_INVALID_IP_ADDRESS       Invalid address supplied         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_dhcpv6_reserve_ip_address_range Actual list create service       */ 
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
UINT  _nxe_dhcpv6_reserve_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr,  
                                         NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address,
                                         UINT *addresses_reserved)
{

UINT                        status;


    /* Check for invalid input.  */
    if ((dhcpv6_server_ptr == NX_NULL) || 
        (start_ipv6_address == NX_NULL)|| (end_ipv6_address == NX_NULL)  ||
        (addresses_reserved == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid non pointer input. */
    if (start_ipv6_address -> nxd_ip_address.v6[3] > end_ipv6_address -> nxd_ip_address.v6[3])
    {

        return(NX_DHCPV6_INVALID_IP_ADDRESS);                                           
    }

    if ((CHECK_UNSPECIFIED_ADDRESS(&start_ipv6_address -> nxd_ip_address.v6[0])) ||
        (CHECK_UNSPECIFIED_ADDRESS(&end_ipv6_address -> nxd_ip_address.v6[0])))
    {
        return(NX_DHCPV6_INVALID_IP_ADDRESS);                                           
    }

    status = _nx_dhcpv6_reserve_ip_address_range(dhcpv6_server_ptr, start_ipv6_address, end_ipv6_address, addresses_reserved);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_reserve_ip_address_range                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function reserves a range of IPv6 addresses not to assign to    */
/*   Clients, using a start and end address.  It returns the number of    */
/*   addresses actually reserved, depending on the size of the server     */
/*   address table.  To reserve a single address, start and end address   */
/*   are the same.                                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    start_ipv6_address                 Start of IP address in list.     */
/*    end_ipv6_address                   End of IP address in list.       */
/*    addresses_reserved                 Number of addresses reserved     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful IP address reservation*/
/*    NX_DHCPV6_INVALID_IP_ADDRESS       Reserved addresses out of range  */
/*    NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS                              */
/*                                       Supplied address is unreachable  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    CHECK_IP_ADDRESSES_BY_PREFIX       Confirm addresses match the      */
/*                                        server domain by matching prefix*/ 
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
UINT  _nx_dhcpv6_reserve_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr,  
                                         NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address,
                                         UINT *addresses_reserved)
{

UINT                         i;
NX_IP                        *ip_ptr;
UINT                         prefix_length;
NXD_ADDRESS                  next_ipv6_address;
NX_DHCPV6_ADDRESS_LEASE      *ip_lease_ptr = NX_NULL; 
UINT                         ga_address_index;


    /* Initialize local and input variables. */
    *addresses_reserved = 0;

    COPY_NXD_ADDRESS(start_ipv6_address, &next_ipv6_address);

    /* Set up a local variable for convenience. */
    ip_ptr = dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr;    
    ga_address_index = dhcpv6_server_ptr -> nx_dhcpv6_server_ga_address_index;         
    prefix_length = ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address_prefix_length;

    /* Verify the input addresses match the server interface address prefix (in IPv4 terms, the
       network masked addresses should be equal). */
    if (!CHECK_IP_ADDRESSES_BY_PREFIX(&(ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address[0]),
                                       &start_ipv6_address -> nxd_ip_address.v6[0], prefix_length))
    {

        return NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS;
    }

    if (!CHECK_IP_ADDRESSES_BY_PREFIX(&(ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address[0]),
                                       &end_ipv6_address -> nxd_ip_address.v6[0], prefix_length))
    {

        return NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS;
    }

    /* Clear out existing entries and start adding IP addresses at the beginning of the list. */
    i = 0;

    /* Match on start-end addresses for which leases to reserve. */
    while (i < NX_DHCPV6_MAX_LEASES)
    {

        /* Set local pointer to the entry for convenience. */
        ip_lease_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i];

        /* Add the IP address into this slot. */
        COPY_NXD_ADDRESS(&next_ipv6_address, &ip_lease_ptr -> nx_dhcpv6_lease_IP_address);

        /* Check if we found the starting address.  */
        if (CHECK_IPV6_ADDRESSES_SAME(&ip_lease_ptr -> nx_dhcpv6_lease_IP_address.nxd_ip_address.v6[0], &start_ipv6_address -> nxd_ip_address.v6[0]))
        {
            break;
        }
    }

    if (i == NX_DHCPV6_MAX_LEASES)
    {

        /* No starting address found in the server IP address list. */
        return NX_DHCPV6_INVALID_IP_ADDRESS;
    }

    /* Set the lease times of this entry to infinity. */
    ip_lease_ptr -> nx_dhcpv6_lease_T1_lifetime = NX_DHCPV6_INFINTY_LEASE;
    ip_lease_ptr -> nx_dhcpv6_lease_T2_lifetime = NX_DHCPV6_INFINTY_LEASE;
    ip_lease_ptr -> nx_dhcpv6_lease_valid_lifetime = NX_DHCPV6_INFINTY_LEASE;
    ip_lease_ptr -> nx_dhcpv6_lease_preferred_lifetime = NX_DHCPV6_INFINTY_LEASE;
    ip_lease_ptr -> nx_dhcpv6_lease_assigned_to = (NX_DHCPV6_CLIENT *)(ALIGN_TYPE)0xFFFFFFFF;

    /* Update the number of addresses reserved. */
    (*addresses_reserved)++;

    /* Process the next entry. */

    /* We found the starting address, so start reserving the rest of the address(es). */
    while ((!CHECK_IPV6_ADDRESSES_SAME(&next_ipv6_address.nxd_ip_address.v6[0], &end_ipv6_address -> nxd_ip_address.v6[0])) &&
           (i < NX_DHCPV6_MAX_LEASES))
    {

        i++;

        /* Set local pointer to the next entry for convenience. */
        ip_lease_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i];

        /* Set the lease times of this entry to infinity. */
        ip_lease_ptr -> nx_dhcpv6_lease_T1_lifetime = NX_DHCPV6_INFINTY_LEASE;
        ip_lease_ptr -> nx_dhcpv6_lease_T2_lifetime = NX_DHCPV6_INFINTY_LEASE;
        ip_lease_ptr -> nx_dhcpv6_lease_valid_lifetime = NX_DHCPV6_INFINTY_LEASE;
        ip_lease_ptr -> nx_dhcpv6_lease_preferred_lifetime = NX_DHCPV6_INFINTY_LEASE;

        /* Set the number of addresses actually reserved. */
        (*addresses_reserved)++;

        /* Return a pointer to the beginning of non reserved addresses.*/
        COPY_NXD_ADDRESS(&ip_lease_ptr -> nx_dhcpv6_lease_IP_address, &next_ipv6_address);
    } 

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_create_dns_address                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function does error checking for the add dns address service.   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    dns_ipv6_address                   DNS server IP address            */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_DHCPV6_PARAM_ERROR              Invalid input                    */
/*    status                             Actual add dns address status    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_create_dns_address                                       */ 
/*                                       Add DNS server address service   */
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
UINT  _nxe_dhcpv6_create_dns_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr,  NXD_ADDRESS *dns_ipv6_address)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_server_ptr || !dns_ipv6_address)
    {
        return NX_PTR_ERROR;
    }

    /* Check for invalid parameters and null address. */
    if ((dhcpv6_server_ptr -> nx_dhcpv6_id != NX_DHCPV6_SERVER_ID) ||     
        (CHECK_UNSPECIFIED_ADDRESS(&dns_ipv6_address -> nxd_ip_address.v6[0])))

    {
        return NX_DHCPV6_PARAM_ERROR;
    }

    status = _nx_dhcpv6_create_dns_address(dhcpv6_server_ptr, dns_ipv6_address);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_create_dns_address                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function the DNS server address to the DHCPv6 server for when   */
/*   clients request the DNS server address.                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPv6 server         */ 
/*    dns_ipv6_address                   DNS server IP address            */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successfully added dns address   */
/*    NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS                              */ 
/*                                       DNS prefix does not match        */
/*                                       DHCPv6 server interface          */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
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
UINT  _nx_dhcpv6_create_dns_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *dns_ipv6_address)
{

NX_IP                        *ip_ptr;
UINT                         prefix_length;
UINT                         ga_address_index;


    /* Set up local variables for convenience. */
    ip_ptr = dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr;   
    ga_address_index = dhcpv6_server_ptr -> nx_dhcpv6_server_ga_address_index;      
    prefix_length = ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address_prefix_length;

    /* Verify the input addresses match the server interface address prefix (in IPv4 terms, the
       network masked addresses should be equal). */
    if (!CHECK_IP_ADDRESSES_BY_PREFIX(&(ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address[0]), 
                                      &dns_ipv6_address -> nxd_ip_address.v6[0], 
                                      prefix_length))
    {
    
        return NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS;
    }

    /* Set the dns server for this interface. */
    COPY_NXD_ADDRESS(dns_ipv6_address, &dhcpv6_server_ptr -> nx_dhcpv6_dns_ip_address);

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_set_server_duid                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the create server DUID    */
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    dhcpv6_server_ptr                 Pointer to DHCPv6 server instance */
/*    duid_type                         Type of DUID, e.g. LL or LLT      */
/*    hardware_type                     Network hardware type e.g IEEE 802*/
/*    mac_address_msw                   Mac address most significant bit  */
/*    mac_address_lsw                   Mac address least significant bit */
/*    time                              Time data for link layer DUIDs    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_set_server_duid        Actual create server duid service */
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
UINT    _nxe_dhcpv6_set_server_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT duid_type, UINT hardware_type, 
                                      ULONG mac_address_msw, ULONG mac_address_lsw, ULONG time)
{
UINT status;


    /* Check for invalid pointer input. */
    if (dhcpv6_server_ptr ==  NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    /* For link layer time DUIDs, the time input must be non zero. */
    if (duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME)
    {
        if (time == 0)
        {
            return NX_PTR_ERROR;

        }
    }

    /* Check for invalid non pointer input. */
    if ((duid_type == 0) || (hardware_type == 0) || (mac_address_msw == 0) || (mac_address_lsw == 0))
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Call the actual service. */
    status = _nx_dhcpv6_set_server_duid(dhcpv6_server_ptr, duid_type, hardware_type, mac_address_msw, mac_address_lsw, time);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_set_server_duid                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates the server DUID with the input data.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    dhcpv6_server_ptr                 Pointer to DHCPv6 server instance */
/*    duid_type                         Type of DUID, e.g. LL or LLT      */
/*    hardware_type                     Network hardware type e.g IEEE 802*/
/*    mac_address_msw                   Mac address most significant bit  */
/*    mac_address_lsw                   Mac address least significant bit */
/*    time                              Time data for link layer DUIDs    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */ 
/*    NX_SUCCESS                          Successful completion status    */
/*    NX_DHCPV6_BAD_SERVER_DUID           Unknown or invalid DUID type    */
/*    NX_DHCPV6_INVALID_VENDOR_DATA       Invalid Vendor ID supplied      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clear specified area of memory    */
/*    memcpy                            Copy data to specified location   */
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
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT    _nx_dhcpv6_set_server_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT duid_type, UINT hardware_type, 
                                      ULONG mac_address_msw, ULONG mac_address_lsw, ULONG time)
{

NX_DHCPV6_SVR_DUID  *duid_ptr;


    duid_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_server_duid;

    /* Initialize the DUID to null. */
    memset(duid_ptr, 0, sizeof(NX_DHCPV6_SVR_DUID));

    /* Assign the DUID op code. */
    duid_ptr -> nx_op_code = NX_DHCPV6_OP_DUID_SERVER;

    /* Set the DUID type. */
    duid_ptr -> nx_duid_type = (USHORT)duid_type;

    /* Set the DUID data for a non vendor DUID. */
    if (duid_ptr -> nx_duid_type != NX_DHCPV6_SERVER_DUID_TYPE_VENDOR_ASSIGNED)
    {

        /* Server has a valid link local address.  Add this to the server DUID fields
           if not a vendor assigned DUID type. */
        duid_ptr-> nx_link_layer_address_msw = (USHORT)mac_address_msw;
        duid_ptr -> nx_link_layer_address_lsw =  mac_address_lsw;
    
        /* Set the hardware type. */
        duid_ptr -> nx_hardware_type = (USHORT)hardware_type;
    }
    else
    {
        /* This is a private vendor DUID with private vendor ID. */
        duid_ptr -> nx_duid_enterprise_number = NX_DHCPV6_SERVER_DUID_VENDOR_PRIVATE_ID;

        /* Check that the server vendor ID size. */
        if (sizeof(NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_ID) > NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH)
        {
            return NX_DHCPV6_INVALID_VENDOR_DATA;
        }

        memcpy(&duid_ptr -> nx_duid_private_identifier[0], NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_ID, sizeof(NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_ID)); /* Use case of memcpy is verified. */
    }

    /* Now set the server DUID length. This will depend on DUID type. */
    if (duid_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_ONLY)
    {

        /* Set the length to include the duid type, hardware type, msw and lsw fields. */
        duid_ptr -> nx_option_length = 3 * sizeof(USHORT) + sizeof(ULONG);
    }
    else if (duid_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME)
    {

        /* Set the length to include the duid type, hardware type, time, and msw and lsw fields. */
        duid_ptr -> nx_option_length = 3 * sizeof(USHORT) + 2 * sizeof(ULONG);

        /* Set the DUID time. */
        duid_ptr -> nx_duid_time = time;

    }
    else if (duid_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_VENDOR_ASSIGNED)
    {

        /* Set the length to include the duid type, hardware type, enterprise number and 
           variable length private identifier. */
        duid_ptr -> nx_option_length = sizeof(USHORT) + sizeof(ULONG) + sizeof(NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_ID);
    }
    else
    {

        /* Undefined or unknown DUID type.  Return error status. */
        return NX_DHCPV6_BAD_SERVER_DUID;
    }

     return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_prepare_iana_status                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the IANA status option.  If the flag input is */
/*    set, the standard server status message is added to the             */
/*    message buffer, and the option length updated. If the flag is false,*/
/*    no message is sent with the status option.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_status_ptr                 Pointer to IANA status to add     */ 
/*    flag                              0 to clear; 1 to add message      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcpy                            Copy specified area of memory     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_response_to_client Send server response to client   */ 
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
UINT    _nx_dhcpv6_prepare_iana_status(NX_DHCPV6_SERVER_IANA_STATUS *dhcpv6_status_ptr, UINT flag)
{


    /* Make sure the basic header fields are filled in. No need to check the status code has been set; it is
       part of the decision to send a response. */
    dhcpv6_status_ptr -> nx_op_code = NX_DHCPV6_OP_OPTION_STATUS;
    dhcpv6_status_ptr -> nx_option_length = sizeof(USHORT);

    /* Check if we are adding or clearing a message. */
    if (flag == NX_FALSE)
    {

        /* Clear the message. */
        memset(&dhcpv6_status_ptr -> nx_status_message[0], 0, NX_DHCPV6_STATUS_MESSAGE_MAX);

        /*We're done! */
        return NX_SUCCESS;
    }

    /* Flag is set so we are adding a message into the packet. */

    /* Clear the message before adding anything into the message buffer. */
    memset(&dhcpv6_status_ptr -> nx_status_message[0], 0, NX_DHCPV6_STATUS_MESSAGE_MAX);

    /* Fill in message according to status code. */
     switch (dhcpv6_status_ptr -> nx_status_code)
     {

         case NX_DHCPV6_STATUS_SUCCESS:

             /* Copy the standard server reply to message buffer. */
             memcpy(&dhcpv6_status_ptr -> nx_status_message[0], NX_DHCPV6_STATUS_MESSAGE_SUCCESS, sizeof(NX_DHCPV6_STATUS_MESSAGE_SUCCESS)); /* Use case of memcpy is verified. */

             /* Update the option length. */
             dhcpv6_status_ptr -> nx_option_length = (USHORT)(dhcpv6_status_ptr -> nx_option_length + sizeof(NX_DHCPV6_STATUS_MESSAGE_SUCCESS));

             /* We're done! */
             break;

         case NX_DHCPV6_STATUS_UNSPECIFIED:

             /* Copy the standard server reply to message buffer. */
             memcpy(&dhcpv6_status_ptr -> nx_status_message[0], NX_DHCPV6_STATUS_MESSAGE_UNSPECIFIED, sizeof(NX_DHCPV6_STATUS_MESSAGE_UNSPECIFIED)); /* Use case of memcpy is verified. */

             /* Update the option length. */
             dhcpv6_status_ptr -> nx_option_length = (USHORT)(dhcpv6_status_ptr -> nx_option_length + sizeof(NX_DHCPV6_STATUS_MESSAGE_UNSPECIFIED));

             /* We're done! */
             break;

         case NX_DHCPV6_STATUS_NO_ADDRS_AVAILABLE:

             /* Copy the standard server reply to message buffer. */
             memcpy(&dhcpv6_status_ptr -> nx_status_message[0], NX_DHCPV6_STATUS_MESSAGE_NO_ADDRS_AVAILABLE, sizeof(NX_DHCPV6_STATUS_MESSAGE_NO_ADDRS_AVAILABLE)); /* Use case of memcpy is verified. */

             /* Update the option length. */
             dhcpv6_status_ptr -> nx_option_length = (USHORT)(dhcpv6_status_ptr -> nx_option_length + sizeof(NX_DHCPV6_STATUS_MESSAGE_NO_ADDRS_AVAILABLE));

             /* We're done! */
             break;

         case NX_DHCPV6_STATUS_NO_BINDING:
             
             /* Copy the standard server reply to message buffer. */
             memcpy(&dhcpv6_status_ptr -> nx_status_message[0], NX_DHCPV6_STATUS_MESSAGE_NO_BINDING, sizeof(NX_DHCPV6_STATUS_MESSAGE_NO_BINDING)); /* Use case of memcpy is verified. */

             /* Update the option length. */
             dhcpv6_status_ptr -> nx_option_length = (USHORT)(dhcpv6_status_ptr -> nx_option_length + sizeof(NX_DHCPV6_STATUS_MESSAGE_NO_BINDING));

             /* We're done! */
             break;

         case NX_DHCPV6_STATUS_NOT_ON_LINK:
             
             /* Copy the standard server reply to message buffer. */
             memcpy(&dhcpv6_status_ptr -> nx_status_message[0], NX_DHCPV6_STATUS_MESSAGE_NOT_ON_LINK, sizeof(NX_DHCPV6_STATUS_MESSAGE_NOT_ON_LINK)); /* Use case of memcpy is verified. */

             /* Update the option length. */
             dhcpv6_status_ptr -> nx_option_length = (USHORT)(dhcpv6_status_ptr -> nx_option_length + sizeof(NX_DHCPV6_STATUS_MESSAGE_NOT_ON_LINK));

             /* We're done! */
             break;

         case NX_DHCPV6_STATUS_USE_MULTICAST:
             
                 /* Copy the standard server reply to message buffer. */
                 memcpy(&dhcpv6_status_ptr -> nx_status_message[0], NX_DHCPV6_STATUS_MESSAGE_USE_MULTICAST, sizeof(NX_DHCPV6_STATUS_MESSAGE_USE_MULTICAST)); /* Use case of memcpy is verified. */

                 /* Update the option length. */
                 dhcpv6_status_ptr -> nx_option_length = (USHORT)(dhcpv6_status_ptr -> nx_option_length + sizeof(NX_DHCPV6_STATUS_MESSAGE_USE_MULTICAST));

                 /* We're done! */
                 break;


         default:
             break;
     }

     return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_preference                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds a DHCPv6 preference option to the server reply.  */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                 Pointer to DHCPv6 Server instance */ 
/*    dhcpv6_client_ptr                 Pointer to DHCPv6 Client instance */ 
/*    index                             Location where to write the data  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Data too large for packet payload */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcpy                            Copy specified area of memory     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_response_to_client Send server response to client   */ 
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
UINT    _nx_dhcpv6_add_preference(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UCHAR *buffer_ptr, UINT *index)
{

UINT    available_payload;
ULONG   message_word;


    /* Fill in option code and length. */
    dhcpv6_client_ptr -> nx_dhcpv6_preference.nx_op_code = NX_DHCPV6_OP_PREFERENCE;
    dhcpv6_client_ptr -> nx_dhcpv6_preference.nx_option_length = 1;
    dhcpv6_client_ptr -> nx_dhcpv6_preference.nx_pref_value = NX_DHCPV6_PREFERENCE_VALUE;

    /* Compute the available payload in the packet buffer. */
    available_payload = dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size - 
                        NX_IPv6_UDP_PACKET - *index;

    /* Check if the DUID will fit in the packet buffer. */
    if (available_payload < (UINT)(dhcpv6_client_ptr -> nx_dhcpv6_preference.nx_option_length + 4)) 
    {

        /* It won't!  */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Create the word to write to the buffer. */
    message_word = (ULONG)dhcpv6_client_ptr -> nx_dhcpv6_preference.nx_op_code << 16;
    message_word |= dhcpv6_client_ptr -> nx_dhcpv6_preference.nx_option_length;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy first half of the DUID option to packet buffer. */
    memcpy(buffer_ptr + *index, &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */

    *index += (ULONG)sizeof(ULONG);

    /* Set the preference value.  */
    *(buffer_ptr + *index) = (UCHAR)(dhcpv6_client_ptr -> nx_dhcpv6_preference.nx_pref_value);

    (*index)++;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_option_requests                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the 'option request' option to the server reply  */
/*    packet based on the options requested from the client, and what     */
/*    options the DHCPv6 server is configured to supply.                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                 Pointer to DHCPV6 server instance */ 
/*    dhcpv6_client_ptr                 Pointer to DHCPV6 client instance */
/*    buffer_ptr                        Pointer to packet buffer          */
/*    index                             Location into buffer to write data*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Not enough room for the option    */ 
/*                                       request in the packet payload    */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Compiles and sends the server     */
/*                                            DHCPv6 response             */ 
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
UINT _nx_dhcpv6_add_option_requests(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, 
                                   UCHAR *buffer_ptr, UINT *index) 
{

USHORT                  i, j;
ULONG                   message_word;
UINT                    available_payload;
NXD_ADDRESS             *dns_server_address;
NX_DHCPV6_SERVER_OPTIONREQUEST *dhcpv6_option_ptr;


    dhcpv6_option_ptr = &dhcpv6_client_ptr -> nx_dhcpv6_option_request;

    /* Compute the available payload in the packet buffer. */
    available_payload = dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size - 
                        NX_IPv6_UDP_PACKET - *index;

    /* Process up to the maximum allowed options.  */
    for(i = 0; i < NX_DHCPV6_MAX_OPTION_REQUEST_OPTIONS; i++)
    {

        /* Check for the end of the list. */
        if (dhcpv6_option_ptr -> nx_op_request[i] == 0)
        {

            /* No more options. */
            break;
        }

         /* Is this the DNS server option? */
        if (dhcpv6_option_ptr -> nx_op_request[i] == NX_DHCPV6_RFC_DNS_SERVER_OPTION)
        {

            /*  Create an option containing a single DNS server address. */
            dhcpv6_option_ptr -> nx_option_length =  4 * sizeof(ULONG);

            /* Check if the option request option will fit in the packet buffer. */
            if (available_payload < (UINT)(dhcpv6_option_ptr -> nx_option_length + 4)) 
            {

                /* It won't. */
                return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
            }

            message_word = (ULONG)dhcpv6_option_ptr -> nx_op_request[i] << 16;
            message_word |= dhcpv6_option_ptr -> nx_option_length;

            /* Adjust for endianness. */
            NX_CHANGE_ULONG_ENDIAN(message_word);

            /* Copy the option request option header to the packet. */
            memcpy(buffer_ptr + *index, &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */
            *index += (ULONG)sizeof(ULONG);

            dns_server_address = &dhcpv6_server_ptr -> nx_dhcpv6_dns_ip_address;
            for (j = 0; j < 4; j++)
            {

                message_word = dns_server_address -> nxd_ip_address.v6[j];

                NX_CHANGE_ULONG_ENDIAN(message_word);

                memcpy(buffer_ptr + *index, &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */

                *index += (ULONG)sizeof(ULONG); 
            }

            /* Update the available payload.  */
            available_payload = (UINT)(available_payload - (UINT)(dhcpv6_option_ptr -> nx_option_length + 4));
        }
        /* Does the DHCPv6 server have an extended option handler? */
        else if (dhcpv6_server_ptr -> nx_dhcpv6_server_option_request_handler_extended)
        {

            /* Yes, so call it.  This handler shall fill the buffer with the requested
               option data (if available).  

               Note: The handler must update the index pointing to the end of data in the buffer
               after it inserts the data. This is done for the DNS server option above.  */
            (dhcpv6_server_ptr -> nx_dhcpv6_server_option_request_handler_extended)(dhcpv6_server_ptr, dhcpv6_option_ptr -> nx_op_request[i], buffer_ptr, index, available_payload);

            /* Check if exceed the payload.  */
            if ((NX_IPv6_UDP_PACKET + *index) <= dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size)
            {

                /* Update the available payload.  */
                available_payload = dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size -
                                    NX_IPv6_UDP_PACKET - *index;
            }
            else
            {
                return (NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD);
            }
        }
        /* Does the DHCPv6 server have an option handler? */
        else if (dhcpv6_server_ptr -> nx_dhcpv6_server_option_request_handler)
        {

            /* Yes, so call it.  This handler shall fill the buffer with the requested
               option data (if available).  

               Note: The handler must update the index pointing to the end of data in the buffer
               after it inserts the data. This is done for the DNS server option above.  */
            (dhcpv6_server_ptr -> nx_dhcpv6_server_option_request_handler)(dhcpv6_server_ptr, dhcpv6_option_ptr -> nx_op_request[i], buffer_ptr, index);

            /* Check if exceed the payload.  */
            if ((NX_IPv6_UDP_PACKET + *index) <= dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size)
            {

                /* Update the available payload.  */
                available_payload = dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size -
                                    NX_IPv6_UDP_PACKET - *index;
            }
            else
            {
                return (NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD);
            }

        }
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_server_interface_set                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the set interface service. */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                     Pointer to the DHCPV6 Server  */ 
/*    iface_index                           Physical interface index for  */
/*                                            DHCP communication          */
/*    ga_address_index                      Global address index on DHCPv6*/
/*                                            interface                   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */ 
/*    NX_INVALID_INTERFACE                  Invalid interface index       */
/*    NX_NO_INTERFACE_ADDRESS               Invalid global address index  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     _nx_dhcpv6_server_interface_set      Actual set interfaces service */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*      Application Code                                                  */ 
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
UINT _nxe_dhcpv6_server_interface_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT iface_index, UINT ga_address_index)
{

UINT status;


    /* Check for invalid pointer input. */
    if (dhcpv6_server_ptr == NX_NULL)
    {

        return NX_PTR_ERROR;
    }

    /* Check for invalid parameters. */
    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return NX_INVALID_INTERFACE;
    }

    if (ga_address_index >= NX_MAX_IPV6_ADDRESSES)
    {
        return NX_NO_INTERFACE_ADDRESS;
    }

    /* Call the actual service. */
    status = _nx_dhcpv6_server_interface_set(dhcpv6_server_ptr, iface_index, ga_address_index);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_interface_set                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the physical network interface for DHCPv6        */
/*    communications and the address index of the IP instance for the     */
/*    primary global address on that interface.                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                     Pointer to the DHCPV6 Server  */ 
/*    iface_index                           Physical interface index for  */
/*                                            DHCP communication          */
/*    ga_address_index                      Global address index on DHCPv6*/
/*                                            interface                   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Indexes successfully set      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     None                                                               */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
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
UINT _nx_dhcpv6_server_interface_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT iface_index, UINT ga_address_index)
{

    dhcpv6_server_ptr -> nx_dhcpv6_server_interface_index = iface_index;
    dhcpv6_server_ptr -> nx_dhcpv6_server_ga_address_index = ga_address_index;
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_lease_timeout_entry               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is scheduled by the ThreadX scheduler on a user       */
/*    configurable time interval.  It sets a flag for the DHCPv6 server   */
/*    thread task to check the lease expiration on its leased IP address. */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr_value               Pointer to the DHCPV6 Server  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     tx_event_flags_set                   Adds a periodic event to the  */
/*                                                 ThreadX event queue    */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
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
VOID  _nx_dhcpv6_server_lease_timeout_entry(ULONG dhcpv6_server_ptr_value)
{

NX_DHCPV6_SERVER *dhcpv6_server_ptr;


    /* Setup DHCPv6 Server pointer.  */
    NX_TIMER_EXTENSION_PTR_GET(dhcpv6_server_ptr, NX_DHCPV6_SERVER, dhcpv6_server_ptr_value)

    /* Signal the DHCPv6 Server it is time to do check the lease time remaining on 
       clients it has leased IP addresses to.  */
    tx_event_flags_set(&(dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events), NX_DHCPV6_IP_LEASE_CHECK_PERIODIC_EVENT, TX_OR);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_server_resume                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the resume NetX DHCPv6     */
/*    server task service.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 Server       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_resume            Actual DHCPV6 resume function   */ 
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
UINT  _nxe_dhcpv6_server_resume(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcpv6_server_ptr == NX_NULL) || (dhcpv6_server_ptr -> nx_dhcpv6_id != NX_DHCPV6_SERVER_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 stop service.  */
    status =  _nx_dhcpv6_server_resume(dhcpv6_server_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_resume                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resumes DHCPV6 processing thread, activates the       */
/*    timers, and sets the running status of the DHCPv6 Server back to    */
/*    running.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                  Pointer to DHCPV6 server         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_DHCPV6_ALREADY_STARTED           DHPCv6 Server already running   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_resume                    Resumes DHCPV6 thread task      */ 
/*    tx_timer_activate                   Activates DHCPv6 timer          */ 
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
UINT  _nx_dhcpv6_server_resume(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{

UINT status;


    /* Is the DHCPv6 Client already running? */
    if (dhcpv6_server_ptr -> nx_dhcpv6_server_running)
    {

        /* Return a (benign) error status. */
        return NX_DHCPV6_ALREADY_STARTED;
    }

    /* Set the DHCPV6 running flag to indicate DHCPV6 is running.  */
    dhcpv6_server_ptr -> nx_dhcpv6_server_running =  NX_TRUE;

    /* Start the lease timer. */
    status = tx_timer_activate(&(dhcpv6_server_ptr -> nx_dhcpv6_lease_timer));

    /* Check for invalid activate status. */
    if (status != TX_SUCCESS)
    {

        /* Return error status. */
        return status;
    }
               
    /* Start the session timer. */
    status = tx_timer_activate(&(dhcpv6_server_ptr -> nx_dhcpv6_session_timer));

    /* Check for invalid activate status. */
    if (status != TX_SUCCESS)
    {

        /* Return error status. */
        return status;
    }

    /* Resume the Server thread as well.  */
    status = tx_thread_resume(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread));

    /* Check for invalid resume status. */
    if (status != TX_SUCCESS && status != TX_SUSPEND_LIFTED)
    {

        /* Return threadx error status. */
        return status;
    }
    /* Return completion status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_session_timeout_entry             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is scheduled by the ThreadX scheduler on a user       */
/*    configurable time interval.  It sets a flag which the DHCPv6 server */
/*    thread task updates all active client session times and checks if   */
/*    any have exceeded the session timeout without sending a response    */
/*    back to the server.                                                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr_value              Pointer to the DHCPV6 server   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     tx_event_flags_set                   Adds a periodic event to the  */
/*                                                 ThreadX event queue    */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
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
VOID  _nx_dhcpv6_server_session_timeout_entry(ULONG dhcpv6_server_ptr_value)
{

NX_DHCPV6_SERVER *dhcpv6_server_ptr;


    /* Setup DHCPv6 Server pointer.  */
    NX_TIMER_EXTENSION_PTR_GET(dhcpv6_server_ptr, NX_DHCPV6_SERVER, dhcpv6_server_ptr_value)

    /* Signal the DHCPv6 server it's time to do more session time keeping! */
    tx_event_flags_set(&(dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events), NX_DHCPV6_CHECK_SESSION_PERIODIC_EVENT, TX_OR);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_server_start                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX start the dhcpv6  */
/*    client service.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_start             Actual DHCPV6 start function    */ 
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
UINT  _nxe_dhcpv6_server_start(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcpv6_server_ptr == NX_NULL) || (dhcpv6_server_ptr -> nx_dhcpv6_id != NX_DHCPV6_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 start service.  */
    status =  _nx_dhcpv6_server_start(dhcpv6_server_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_start                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts the DHCPV6 processing thread and prepares the  */
/*    DHCPv6 Server to receive DHCPv6 client requests.                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                     Pointer to DHCPV6 Server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_DHCPV6_ALREADY_STARTED           DHCPv6 server already started   */ 
/*    NX_DHCPV6_MISSING_REQUIRED_OPTIONS  Server missing required options */
/*    NX_DHCPV6_NO_ASSIGNABLE_ADDRESSES   No addresses available to assign*/
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nxd_ipv6_linklocal_address_set        Obtain MAC address for DUID   */
/*    nx_udp_socket_create                  Create the Server UDP socket  */
/*    nx_udp_socket_bind                    Bind the Server socket        */ 
/*    nx_udp_socket_unbind                  Unbind the Server socket      */ 
/*    tx_timer_activate                     Activate Server lease timer   */
/*    tx_timer_deactivate                   Deactivate Server lease timer */
/*    tx_thread_resume                      Resume Server thread task     */ 
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
UINT  _nx_dhcpv6_server_start(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{

UINT        status;
#ifdef NETXDUO_MULTI_HOME
UINT        if_index;
#endif


    /* Determine if the DHCPV6 server has already started.  */
    if (dhcpv6_server_ptr -> nx_dhcpv6_server_running)
    {

        /* Yes, it has.  */
        return(NX_DHCPV6_ALREADY_STARTED);
    }

    /* Check if the server has assignable addresses. */
    if (dhcpv6_server_ptr -> nx_dhcpv6_assignable_addresses == 0)
    {
        return  NX_DHCPV6_NO_ASSIGNABLE_ADDRESSES;
    }

    /* Check that the global address is valid. */
    if (dhcpv6_server_ptr -> nx_dhcpv6_server_ga_address_index >= NX_MAX_IPV6_ADDRESSES)
    {
        return NX_DHCPV6_INVALID_GLOBAL_INDEX;
    }

    /* Create the Socket and check the status */
    status = nx_udp_socket_create(dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr, 
                                  &(dhcpv6_server_ptr -> nx_dhcpv6_server_socket), (CHAR *)"NetX DHCPV6 Server",
                                  NX_DHCPV6_TYPE_OF_SERVICE, NX_DHCPV6_FRAGMENT_OPTION, 
                                  NX_DHCPV6_TIME_TO_LIVE, NX_DHCPV6_QUEUE_DEPTH);

    /* Was the socket creation successful?  */
    if (status != NX_SUCCESS)
    {

        /* No, return error status.  */
        return status;
    }

    /* Save the DHCP instance pointer in the socket. */
    dhcpv6_server_ptr -> nx_dhcpv6_server_socket.nx_udp_socket_reserved_ptr =  (void *) dhcpv6_server_ptr;

    /* Set the DHCPv6 request handler for packets received on the DHCPv6 server socket. */
    status = nx_udp_socket_receive_notify(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket), _nx_dhcpv6_server_socket_receive_notify);

    /* Was the socket receive_notify set successful?  */
    if (status != NX_SUCCESS)
    {

        /* No, return error status.  */
        return status;
    }

    /* Check if the host application has created or retrieved a previously created server DUID. */
    if (dhcpv6_server_ptr -> nx_dhcpv6_server_duid.nx_duid_type == 0)
    {

        /* No valid server DUID detected. Server cannot run yet!*/
        return NX_DHCPV6_NO_SERVER_DUID;
    }

    /* Bind the UDP socket to the DHCPV6 Server port.  */
    status =  nx_udp_socket_bind(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket), NX_DHCPV6_SERVER_UDP_PORT, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

        return status;
    }

    /* Activate the DHCPv6 timers.  */
    status = tx_timer_activate(&dhcpv6_server_ptr -> nx_dhcpv6_lease_timer);

    /* Determine if the timer activate call was successful.  */
    if ((status != TX_SUCCESS) && (status != TX_ACTIVATE_ERROR))
    {

        /* No it was not; unbind the DHCPV6 socket.  */
        nx_udp_socket_unbind(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

        /* Return the error status. */
        return status;
    }

    /* Start the session timer. */
    status = tx_timer_activate(&(dhcpv6_server_ptr -> nx_dhcpv6_session_timer));

    /* Check for invalid activate status. */
    if ((status != TX_SUCCESS) && (status != TX_ACTIVATE_ERROR))
    {

        /* No it was not; unbind the DHCPV6 socket.  */
        nx_udp_socket_unbind(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

        /* Return error status. */
        return status;
    }

    /* Ready to resume the DHCPV6 server thread.  */
    status =  tx_thread_resume(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread));

    /* Determine if the resume was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Deactivate the timer. */
        tx_timer_deactivate(&dhcpv6_server_ptr -> nx_dhcpv6_lease_timer);

        /* Unbind the DHCPV6 socket.  */
        nx_udp_socket_unbind(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket));

        return status;
    }

    /* Set the DHCPV6 started flag to indicate the DHCPV6 server is now running.  */
    dhcpv6_server_ptr -> nx_dhcpv6_server_running =  NX_TRUE;

    /* Return completion status.  */
    return(status);  
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_socket_receive_notify             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is notified by NetX Duo when a packet arrives in the  */
/*    server socket. It sets a flag which the DHCPv6 server  thread       */
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
/*                                                                        */ 
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
VOID  _nx_dhcpv6_server_socket_receive_notify(NX_UDP_SOCKET *socket_ptr)
{

NX_DHCPV6_SERVER *dhcpv6_server_ptr;


    /* Get a pointer to the DHCPv6 server.  */
    dhcpv6_server_ptr =  (NX_DHCPV6_SERVER *) socket_ptr -> nx_udp_socket_reserved_ptr;

    /* Signal the DHCPv6 Server it has a UDP packet on its socket receive queue.  */
    tx_event_flags_set(&(dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events), NX_DHCPV6_SERVER_RECEIVE_EVENT, TX_OR);

    return;
}          


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_server_suspend                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the suspend NetX DHCPv6    */
/*    Server task service.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_suspend           Actual DHCPV6 suspend function  */ 
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
UINT  _nxe_dhcpv6_server_suspend(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcpv6_server_ptr == NX_NULL) || (dhcpv6_server_ptr -> nx_dhcpv6_id != NX_DHCPV6_SERVER_ID))
        return(NX_PTR_ERROR);
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 stop service.  */
    status =  _nx_dhcpv6_server_suspend(dhcpv6_server_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_suspend                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function suspends DHCPV6 processing thread, deactivates the    */
/*    timers, and sets the running status of the DHCPv6 Server to not     */
/*    running.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPV6 Server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_DHCPV6_NOT_STARTED               Task not running; can't be      */
/*                                               suspended                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_suspend                     Suspend DHCPV6 thread task    */ 
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
UINT  _nx_dhcpv6_server_suspend(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{


    /* Determine if the DHCPV6 server thread is running.  */
    if (!dhcpv6_server_ptr -> nx_dhcpv6_server_running)
    {

        /* DHCPV6 has not been started so we cannot 'suspend' it.  */
        return(NX_DHCPV6_NOT_STARTED);
    }

    /* Clear the DHCPV6 running flag to indicate DHCPV6 is not running.  */
    dhcpv6_server_ptr -> nx_dhcpv6_server_running =  NX_FALSE;
                    
    /* Stop the timer. */
    tx_timer_deactivate(&(dhcpv6_server_ptr -> nx_dhcpv6_lease_timer));

    /* Suspend the Server thread as well.  */
    tx_thread_suspend(&(dhcpv6_server_ptr -> nx_dhcpv6_server_thread));

    /* Return completion status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_thread_entry                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the background processing thread for the DHCPV6    */
/*    Client.  It waits to be notified by ThreadX event flags when to     */
/*    perform certain actions.  These include updating the time remaining */
/*    on the Client IP address lease time, and maintaining the duration of*/
/*    the current Client Server session (if Client has made a request).   */
/*    It will terminate if a host application handler is called and       */
/*    indicates the Client should abort.                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    info                               Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_get                Receive notice of event flags set */ 
/*    tx_mutex_get                      Obtain lock on Client resource    */ 
/*    tx_mutex_put                      Release lock on Client resource   */ 
/*    nx_dhcpv6_deprecated_IP_address_handler                             */
/*                                      Callback function for handling an */
/*                                         IP address at the end of its   */
/*                                         preferred life time            */
/*    nx_dhcpv6_expired_IP_address_handler                                */ 
/*                                      Callback function for handling an */
/*                                         IP address at the end of its   */
/*                                         valid life time                */
/*    _nx_dhcpv6_request_renew          Initiate the RENEW request        */
/*    _nx_dhcpv6_request_rebind         Initiate the REBIND request       */
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
VOID  _nx_dhcpv6_server_thread_entry(ULONG info)
{

UINT                    i;
NX_DHCPV6_SERVER        *dhcpv6_server_ptr;
NX_DHCPV6_CLIENT        *dhcpv6_client_ptr;
ULONG                   dhcpv6_events;
NXD_ADDRESS             client_address;


    /* Setup the DHCPV6 server pointer.  */
    NX_THREAD_EXTENSION_PTR_GET(dhcpv6_server_ptr, NX_DHCPV6_SERVER, info)

    /* Continue processing periodic tasks till this internal flag is cleared. */
    while (1)
    {


        /* Pick up IP event flags.  */
        tx_event_flags_get(&dhcpv6_server_ptr -> nx_dhcpv6_server_timer_events, NX_DHCPV6_ALL_EVENTS, 
                           TX_OR_CLEAR, &dhcpv6_events, TX_WAIT_FOREVER);

        if (dhcpv6_events & NX_DHCPV6_SERVER_RECEIVE_EVENT)
        {

            /* Listen for new Client DHCP service request e.g. DISCOVERY messages. */
            _nx_dhcpv6_listen_for_messages(dhcpv6_server_ptr);
        }

        if (dhcpv6_events & NX_DHCPV6_CHECK_SESSION_PERIODIC_EVENT)
        {
            /* Check the active clients whose sessions that have timed out. */
            for (i = 0; i < NX_DHCPV6_MAX_CLIENTS; i++)
            {

                /* Set a local pointer for convenience. */
                dhcpv6_client_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_clients[i];

                tx_mutex_get(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex, NX_WAIT_FOREVER);

                /* Is this client in session with the server? */
                if (dhcpv6_client_ptr -> nx_dhcpv6_client_session_time)
                {
                
                    /* Update the Client session time.  */
                    dhcpv6_client_ptr -> nx_dhcpv6_client_session_time += NX_DHCPV6_SESSION_TIMER_INTERVAL;
    
                    if (dhcpv6_client_ptr -> nx_dhcpv6_client_session_time >= NX_DHCPV6_SESSION_TIMEOUT)
                    {

                        /* The session has timed out without a response from the Client. Invalidate
                           the client entry. */

                        /* First get the assigned address. */
                        COPY_IPV6_ADDRESS(&dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0], 
                                          &client_address.nxd_ip_address.v6[0]);

                        /* Clear the client record. */
                        _nx_dhcpv6_clear_client_record(dhcpv6_server_ptr, dhcpv6_client_ptr);
                    }
                }

                tx_mutex_put(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex);
            }
        }

        /* Check for a periodic DHCP server task.  */
        if (dhcpv6_events & NX_DHCPV6_IP_LEASE_CHECK_PERIODIC_EVENT)
        {

            /* Check the active clients for the time remaining in their leased IP address. */
            for (i = 0; i < NX_DHCPV6_MAX_CLIENTS; i++)
            {

                /* Set a local pointer for convenience. */
                dhcpv6_client_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_clients[i];

                tx_mutex_get(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex, NX_WAIT_FOREVER);

                /* Skip those clients who do not have a lease assigned yet. */
                if (dhcpv6_client_ptr -> nx_dhcpv6_IP_lease_time_accrued == 0)
                {

                    tx_mutex_put(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex);

                    continue;
                }

                /* Update the time accrued on the client's current IP address lease. */
                dhcpv6_client_ptr -> nx_dhcpv6_IP_lease_time_accrued += NX_DHCPV6_IP_LEASE_TIMER_INTERVAL;

                /* Has this address become obsolete? */
                if (dhcpv6_client_ptr -> nx_dhcpv6_IP_lease_time_accrued >= dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime)
                {

                    tx_mutex_put(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex);

                    /* Clear the client record. */
                    _nx_dhcpv6_clear_client_record(dhcpv6_server_ptr, dhcpv6_client_ptr);
                }
                else if (dhcpv6_client_ptr -> nx_dhcpv6_IP_lease_time_accrued >= 
                         dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime)
                {

                    /* Address is deprecated! Client *should* not use it but no action taken by the server yet. */
                }

                tx_mutex_put(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex);
            }
        }

        /* Relinquish control before another cycle.*/
        tx_thread_sleep(1);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_extract_packet_information        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts data from DHCPv6 Client request packets. A   */
/*    request is checked for valid message type and required DHCPv6 data. */
/*    A client record is created if one does not exist for the client and */
/*    the data updated into it.                                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPv6 server        */
/*    dhcpv6_client_ptr                   Pointer to DHCPv6 client        */
/*    packet_ptr                          Pointer to received packet      */ 
/*    iface_index                         Interface packet received on    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_DHCPV6_ILLEGAL_MESSAGE_TYPE      Packet message type illegal     */
/*    NX_DHCPV6_BAD_TRANSACTION_ID        Message ID fails to match up    */
/*    NX_DHCPV6_PROCESSING_ERROR          Packet length is different than */
/*                                                   expected             */
/*    NX_DHCPV6_UNKNOWN_OPTION            Unknown option in message       */
/*    NX_DHCPV6_INVALID_HW_ADDRESS        Missing or invalid MAC address  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_get_data  Extract data from reply packet  */ 
/*    _nx_dhcpv6_server_utility_get_block_option_length                   */
/*                                        Extract option header data      */
/*    _nx_dhcpv6_process_duid             Extract DUID from Client message*/ 
/*    _nx_dhcpv6_process_DNS_server       Extract DNS server from message */
/*    memcpy                              Copies specified area of memory */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_listen_for_messages       Waits to receive client message*/
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_server_extract_packet_information(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT **dhcpv6_client_ptr, NX_PACKET *packet_ptr, 
                                                   UINT iface_index, NXD_ADDRESS source_address, NXD_ADDRESS destination_address)
{

UINT                status;
UINT                add_on;
UINT                record_index;
ULONG               option_code;
ULONG               option_length;
UCHAR               *buffer_ptr;
UINT                index;
ULONG               returned_xid;
NX_DHCPV6_SVR_DUID  client_duid, server_duid;
UINT                client_duid_received;
UINT                server_duid_received;
NX_DHCPV6_CLIENT    temp_client_rec;
ULONG               received_message_type;
UINT                matching;

    NX_PARAMETER_NOT_USED(iface_index);

    *dhcpv6_client_ptr = NX_NULL;

    /* Create a scratch record for this client so we can qualify it before adding it to the server table. */
    memset(&temp_client_rec, 0, sizeof(NX_DHCPV6_CLIENT));

    /* Initialize the record with the DHCPv6 Client ID. */
    temp_client_rec.nx_dhcpv6_id = NX_DHCPV6_CLIENT_ID;

    /* Assume the Client is not bound to an IP address yet.  */
    temp_client_rec.nx_dhcpv6_state =  NX_DHCPV6_STATE_UNBOUND;

    /* Check packet for message type and transaction ID. */
    if (packet_ptr -> nx_packet_length < 4)
    {

        /* Reported length of the packet may be incorrect.  */
        return NX_DHCPV6_PROCESSING_ERROR;
    }

    /* Set a pointer to the start of DHCPv6 data. */
    buffer_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* Initialize local variables. */
    index = 0;
    client_duid_received = NX_FALSE;
    server_duid_received = NX_FALSE;

    /* Extract the message type which should be the first byte.  */
    _nx_dhcpv6_server_utility_get_data(buffer_ptr, 1, &received_message_type);

    /* Check for an illegal message type. */
    if ((received_message_type == NX_DHCPV6_MESSAGE_TYPE_ADVERTISE) ||
        (received_message_type == NX_DHCPV6_MESSAGE_TYPE_REPLY) ||
        (received_message_type == NX_DHCPV6_MESSAGE_TYPE_RECONFIGURE) ||
        (received_message_type > NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST))
    {

        /* These should only be sent to DHCPv6 clients! The Inform Request is actually
           legal to receive but should NOT be used to add clients to the server table. */
        return NX_DHCPV6_ILLEGAL_MESSAGE_TYPE;
    }

    /* Copy the source address to the Client record. */
    COPY_IPV6_ADDRESS(&source_address.nxd_ip_address.v6[0], &temp_client_rec.nx_dhcp_source_ip_address.nxd_ip_address.v6[0]);
    temp_client_rec.nx_dhcp_source_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Copy the destination address to the Client record. */
    COPY_IPV6_ADDRESS(&destination_address.nxd_ip_address.v6[0], &temp_client_rec.nx_dhcp_destination_ip_address.nxd_ip_address.v6[0]);
    temp_client_rec.nx_dhcp_destination_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Advance to the data pointer. */
    buffer_ptr++;

    /* Parse the transaction ID. */
    _nx_dhcpv6_server_utility_get_data(buffer_ptr, 3, &returned_xid);

    temp_client_rec.nx_dhcpv6_message_type = received_message_type;

    /* Set the message ID. The server will need this to reply to the Client. */
    temp_client_rec.nx_dhcpv6_message_xid = returned_xid;

    buffer_ptr += 3;

    /* Update index for message type and transaction ID.  */
    index += 4;

    /* Now parse all the DHCPv6 option blocks in the packet buffer. */
    /* 4 bytes for option code and option length. */
    while (index + 4 <= packet_ptr -> nx_packet_length)
    {

        /* Get the option code and length of data of the current option block. */
        status = _nx_dhcpv6_server_utility_get_block_option_length(buffer_ptr, &option_code, &option_length);

        /* Check that the block data is valid. */
        if (status != NX_SUCCESS)
        {

            /* No, return the error status. */
            return status;
        }

        /* Keep track of how far into the packet we have parsed. */
        index += option_length + 4; 

        /* This is a double check to verify we haven't gone off the end of the packet buffer. */
        if (index > packet_ptr -> nx_packet_length)
        {

            /* Reported length of the packet may be incorrect. At any rate, we don't know
               exactly what the end of the message is, so abort. */
            return NX_DHCPV6_PROCESSING_ERROR;
        }

        /* Update buffer pointer to option data.  */
        buffer_ptr += 4;

        /* Process the option code with an option specific API. */
        switch (option_code)
        {

            /* Note - these 'process' functions will not move the buffer pointer. */

            case NX_DHCPV6_OP_DUID_CLIENT:

                client_duid.nx_op_code = (USHORT)option_code;
                client_duid.nx_option_length = (USHORT)option_length;

                status = _nx_dhcpv6_process_duid(&client_duid, option_code, option_length, buffer_ptr);

                if (status != NX_SUCCESS)
                {

                    return status;
                }

                /*  Copy to the temporary client record. */
                memcpy(&temp_client_rec.nx_dhcpv6_client_duid, &client_duid, sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */

                /* Indicate the message contained the required Client DUID. */
                client_duid_received = NX_TRUE;

            break;

            case NX_DHCPV6_OP_DUID_SERVER:

                server_duid.nx_op_code = (USHORT)option_code;
                server_duid.nx_option_length = (USHORT)option_length;

                status = _nx_dhcpv6_process_duid(&server_duid, option_code, option_length, buffer_ptr);

                if (status != NX_SUCCESS)
                {

                    return status;
                }

                /*  Copy to the temporary client record. */
                memcpy(&temp_client_rec.nx_dhcpv6_server_duid, &server_duid, sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */

                /* Indicate the message contained the required Server DUID. */
                server_duid_received = NX_TRUE;

            break;


            case NX_DHCPV6_OP_IA_NA:

                status = _nx_dhcpv6_server_process_iana(&temp_client_rec, option_code, option_length, buffer_ptr);

                if (status != NX_SUCCESS)
                {
                    return status;
                }

            break;

            /* This should not happen. The IA address option must be embedded in the IANA option. */
            case NX_DHCPV6_OP_IA_ADDRESS:

                /* Don't process an IA address option outside of an address association (IANA). */
                status = NX_DHCPV6_IANA_OPTION_MISSING;
                
            break;

            case NX_DHCPV6_OP_OPTION_REQUEST:

                /* Option request option contains request usually for network configuration parameters
                   such as the DNS server. */
                status = _nx_dhcpv6_process_option_request(&temp_client_rec, option_code, option_length, buffer_ptr);

                if (status != NX_SUCCESS)
                {

                    return status;
                }

            break;

            case NX_DHCPV6_OP_ELAPSED_TIME:

                /* The elapsed time is time elapsed since the client initiated the IP lease request. */
                status = _nx_dhcpv6_process_elapsed_time(&temp_client_rec, option_code, option_length, buffer_ptr); 

                if (status != NX_SUCCESS)
                {

                    return status;
                }

            break;


            default:
                break; 
        }

        /* Check for errors from option block processing. */
        if (status != NX_SUCCESS)
        {


            return status;
        }

        /* Move to the next top level option. */
         buffer_ptr += option_length;
    }

    /* Check for an improperly formatted packet. */
    if (index != packet_ptr -> nx_packet_length)
    {

        /* If we get here if we are not at the expected end of the buffer. Oops */
        return NX_DHCPV6_PROCESSING_ERROR;
    }

    /* Check if the message is missing the required DUID(s). */
    if ((temp_client_rec.nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT) ||
        (temp_client_rec.nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REBIND))
    {

        if (!client_duid_received)
        {

            return NX_DHCPV6_MESSAGE_DUID_MISSING;
        }
    }
    /* The following message types must include a client and server DUID. */
    else if ((temp_client_rec.nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REQUEST) ||
             (temp_client_rec.nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RENEW)   ||
             (temp_client_rec.nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE))
    {

        if (!server_duid_received || !client_duid_received)
        {

            return NX_DHCPV6_MESSAGE_DUID_MISSING;
        }
    }
    /* An inform request only requires a server DUID. */
    else if (temp_client_rec.nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST)
    {

        /* The Inform request message should NOT include an IA. */
        if (temp_client_rec.nx_dhcpv6_ia.nx_op_code != 0)
        {
            return NX_DHCPV6_INVALID_IA_DATA;
        }
    }

    /* Validate the client message, and determine how (if) the server should respond to it. */
    status = _nx_dhcpv6_validate_client_message(dhcpv6_server_ptr, &temp_client_rec);

    /* Check for errors. */
    if (status != NX_SUCCESS)
    {

        /* Return all other errors as completion status. Do not send a message to the client. */
        return(status);
    }

    if (temp_client_rec.nx_dhcpv6_response_back_from_server == NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT) 
    {

        /* Not responding to client. So do not assign an IP address or update 
           the client record with invalid message. */
        return  NX_SUCCESS;
    }

    /* Look up the client record in the server table. If not found, or this is 
       a new request e.g. a SOLICIT message, create a new client entry in the table and 
       fill in the extracted information. */
    add_on = NX_FALSE;

    if ((received_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT) ||
        (received_message_type == NX_DHCPV6_MESSAGE_TYPE_CONFIRM) ||
        (received_message_type == NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST))
    {

        /* Check first if the validation process indicates we do not reply to this message. */
        if (temp_client_rec.nx_dhcpv6_response_back_from_server != NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT)
        {

            /* Ok to update or create a new record. */
            add_on = NX_TRUE;
        }
    }

    /* Check if the client is already in the server table. */
    status = _nx_dhcpv6_find_client_record_by_duid(dhcpv6_server_ptr, &client_duid, &record_index, 
                                                   add_on, temp_client_rec.nx_dhcpv6_message_xid, &matching);

    /* Check for error during search. */
    if (status != NX_SUCCESS)
    {

        return(status);
    }

    /* Set a local pointer to the client record in the server table. */
    *dhcpv6_client_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_clients[record_index];

    /* Was a match was found? */
    if (!matching)
    {

        /* No, this is a new record. Create a new client record. */
        status = _nx_dhcpv6_update_client_record(dhcpv6_server_ptr, &temp_client_rec, *dhcpv6_client_ptr);
    }
    else
    {

        /* A match was found. Check if this is a different message request than the previous client request. */
        if ((temp_client_rec.nx_dhcpv6_message_type != (*dhcpv6_client_ptr) -> nx_dhcpv6_message_type) &&
            (received_message_type != NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST))
        {

            /*  This is a different message from the Client. Before updating the record check the client global address 
                to be the same as what the server assigned or is offering to assign. */
            if (!CHECK_IPV6_ADDRESSES_SAME(&temp_client_rec.nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0], 
                                          &(*dhcpv6_client_ptr )-> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0]))
            {

                /* This is a different address. Disregard this message. */
                (*dhcpv6_client_ptr) -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;

                return NX_SUCCESS;
            }

            /* This is a valid request. Update the client record with the new data. */
            status = _nx_dhcpv6_update_client_record(dhcpv6_server_ptr, &temp_client_rec, *dhcpv6_client_ptr);
        }
    }

    /* Yes we're done with no processing errors detected. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_validate_client_message                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function validates the client DHCPv6 request, determines if an */
/*    IP address need to be assigned, what state the DHCPv6 Client        */
/*    should be advanced to, and what the server response type should be. */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                   Pointer to DHCPv6 server        */
/*    dhcpv6_client_ptr                   Pointer to DHCPv6 client        */
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
/*    _nx_dhcpv6_listen_for_messages       Listen for, process and respond*/
/*                                            to DHCPV6 Client messages   */
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
UINT  _nx_dhcpv6_validate_client_message(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr)
{

UINT                status;
UINT                matching_clients;
UINT                matching_server_duids;
UINT                record_index;
UINT                dest_address_type;
UINT                ga_address_index;
UINT                if_index;
UINT                prefix_length;


    /* Clear the Server response to the Client. We will decide that below. */
    dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = 0;

    ga_address_index = dhcpv6_server_ptr -> nx_dhcpv6_server_ga_address_index;

    /* Initialize the response to the client. If the client request is invalid, ignore the request (no response 
       from server) or indicate the problem in the IANA status option. */

    /* Initialize the IANA status for the SOLICIT request as successfully granted.  */
    if (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT)
    {

        /* Respond to a valid Solicit request with an Advertise message. */
        dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_ADVERTISE;
        dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_SUCCESS;
    }
    else    
    {

        /* Respond to a valid Request request with a Reply message. */
        dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_REPLY;

        /* Set the expected IANA status. */
        if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REQUEST) ||
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RENEW) ||
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REBIND) ||
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE) ||
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE) ||
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST) || 
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_CONFIRM))
        {

            dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_SUCCESS;
        }
    }

    /* Never accept a packet without a DHCP ID. */
    if (!dhcpv6_client_ptr -> nx_dhcpv6_message_xid)
    {

         /* Don't respond, just discard the message. . */
         dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;

         return(NX_SUCCESS); 
    }

    /* Determine if destination address type (unicast or multicast). */
    dest_address_type = IPv6_Address_Type(&dhcpv6_client_ptr -> nx_dhcp_destination_ip_address.nxd_ip_address.v6[0]);

    /* Do validation specific for message type. This section will also
       advance the DHCPv6 Client state and set the server response message type. */
    switch (dhcpv6_client_ptr -> nx_dhcpv6_message_type)
    {

        case NX_DHCPV6_MESSAGE_TYPE_SOLICIT:
        {

            /* Check for missing required fields. */
            if ((dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_duid_type == 0) ||
                (dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_hardware_type == 0))
            {

                /* Don't respond, just discard the message. . */
                dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;

                return(NX_SUCCESS); 
            }

            /* Check for presence of a server DUID. */
            if (dhcpv6_client_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw != 0)
            {

                /* Don't respond, just discard the message. . */
                dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;

                /* Clear the success status. */
                dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_UNSPECIFIED;

                return(NX_SUCCESS); 
            }

            /* The Solicit message should never be unicast. Check if the destination is unicast. */
            if (IPV6_ADDRESS_UNICAST & dest_address_type)
            {

                /* Don't respond, just discard the message. . */
                dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;

                return NX_SUCCESS;
            }
            break;
        }

        case NX_DHCPV6_MESSAGE_TYPE_REQUEST:       
        case NX_DHCPV6_MESSAGE_TYPE_RENEW:       
        case NX_DHCPV6_MESSAGE_TYPE_REBIND:       
        case NX_DHCPV6_MESSAGE_TYPE_RELEASE:
        case NX_DHCPV6_MESSAGE_TYPE_DECLINE:       
        {

            /* Check for missing required fields. */
            if ((dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_duid_type == 0) ||
                (dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_hardware_type == 0))
            {

                /* Don't respond, just discard the message. . */
                dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;

                return(NX_SUCCESS);  
            }

            /* Check for an invalid unicast destination address type. */
            if (dhcpv6_client_ptr -> nx_dhcpv6_message_type != NX_DHCPV6_MESSAGE_TYPE_REBIND)
            {    
        
                /* The DHCPv6 server does not support unicast. Check for unicast client destination. */
                if (IPV6_ADDRESS_UNICAST & dest_address_type)
                {
        
                    /* It is unicast. Set the use multicast error status. */
                    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_USE_MULTICAST;
        
                    return NX_SUCCESS;
                }
            }

            /* This client message is not valid if an matching client record does not exist with the server. */
            status = _nx_dhcpv6_find_client_record_by_duid(dhcpv6_server_ptr, &dhcpv6_client_ptr -> nx_dhcpv6_client_duid, 
                                                           &record_index, NX_FALSE, 
                                                           dhcpv6_client_ptr -> nx_dhcpv6_message_xid, 
                                                           &matching_clients);
            /* Check for error. */
            if (status != NX_SUCCESS)
            {

                return status;
            }

            /* Check for matching client record. */
            if (!matching_clients)
            {

                /* No matching client record found. */

                /* For Release and Decline messages let the client know using the IANA status code that 
                   the server has no record of the assigned or requested address for this client.  */
                if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE) || 
                    (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE))
                {

                    /* Set the non-binding error status. */
                    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_NO_BINDING;

                    /* Set the lifetimes to zero. */
                    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = 0;
                    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = 0;
                    dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = 0;
                    dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = 0;
                }
                else
                {
                
                    /* Otherwise don't respond, just discard the message. */
                    dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;
    
                    /* Clear the success status. */
                    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_UNSPECIFIED;
    
                    return(NX_SUCCESS); 
                }
            }

            /* Handle Rebind a little differently. It should not specify a server DUID. */
            if (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REBIND)
            {
                /* Is there a server DUID in the client message? */
                if (dhcpv6_client_ptr -> nx_dhcpv6_server_duid.nx_option_length > 0)
                {

                    /* Yes; don't respond, just discard the message. . */
                    dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;

                    /* Clear the success status. */
                    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_UNSPECIFIED;
                }

                break;
            }
            else
            {

                /* With all other IA addresses the server DUID must match the server DUID. */
                status = _nx_dhcpv6_check_duids_same(&dhcpv6_client_ptr -> nx_dhcpv6_server_duid, 
                                                     &dhcpv6_server_ptr -> nx_dhcpv6_server_duid, &matching_server_duids);
    
                /* Check for error. */
                if (status != NX_SUCCESS)
                {
    
                    /* Clear the success status. */
                    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_UNSPECIFIED;
    
                    return status;
                }
    
                /* Do the two server DUIDs match? */
                if (!matching_server_duids)
                {
      
                    /* No; don't respond, just discard the message. */
                    dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;
    
                    /* Clear the success status. */
                    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_UNSPECIFIED;

                    return NX_SUCCESS;
                }
            }
            break;
        }
            /* The Client wished to confirm its IP address is on link. */
            case NX_DHCPV6_MESSAGE_TYPE_CONFIRM: 
            {
            
                /* Check for missing IA option. */
                if (dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_option_length == 0)
                {
    
                    /* No IA option, so don't respond, just discard the message. */
                    dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;
    
                    return NX_SUCCESS;
                }
    
                /* Verify the input addresses match the server interface address prefix (in IPv4 terms, the
                   network masked addresses should be equal). */
                prefix_length = dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address_prefix_length;                 
    
                if (!CHECK_IP_ADDRESSES_BY_PREFIX(&(dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr -> nx_ipv6_address[ga_address_index].nxd_ipv6_address[0]), 
                                                  &dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0], 
                                                  prefix_length))
                {

    
                    /* Inform the client their IA address is not on link. */
                    dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_REPLY;
                    
                    /* Set the IA-NA error status. */
                    dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_NOT_ON_LINK;
                    
                    return(NX_SUCCESS); 
                }
    
                /* More checks will be done when the client rebind address is searched in the server 
                   address table. For now we are ok. */
    
                break;
            }

            case NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST:
            {
        
                /* Verify Client's server duid matches the server DUID. Note that if the server DUID type is a
                   link layer time type (LLT), this comparison should ignore the time field because this is an 'asynchronous' request
                   from the client, so just match on the LL data. */
    
                /* Temporarily set the server DUID type to LL if it was LLT. */
                if ((dhcpv6_client_ptr -> nx_dhcpv6_server_duid.nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME) || 
                    (dhcpv6_client_ptr -> nx_dhcpv6_server_duid.nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_ONLY))
                {

                    /* Now verify the physical address matches what the server has for this client. */  
                    if_index = dhcpv6_server_ptr -> nx_dhcpv6_server_interface_index;         
                    if (dhcpv6_client_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw != 
                        dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr -> nx_ip_interface[if_index].nx_interface_physical_address_msw)
                    {
    
                        /* Inform the client their physical address is not on record. */
                        dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;
                    }
                    else if (dhcpv6_client_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_lsw != 
                             dhcpv6_server_ptr -> nx_dhcpv6_ip_ptr -> nx_ip_interface[if_index].nx_interface_physical_address_lsw)
                    {
                        /* Inform the client their physical address is not on record. */
                        dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;
                    }
                }
                else
                {
                
                    /* Verify the client inform request was intended for this server. If the Client did not
                       include a server DUID, which is acceptable in DHCPv6 protocol, it will 'pass' this check. */
                    status = _nx_dhcpv6_check_duids_same(&dhcpv6_client_ptr -> nx_dhcpv6_server_duid, 
                                                         &dhcpv6_client_ptr -> nx_dhcpv6_server_duid, &matching_server_duids);
        
                    /* Check for error. */
                    if (status != NX_SUCCESS)
                    {
        
                        /* Clear the success status. */
                        dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_UNSPECIFIED;
        
                        return status;
                    }
        
                    /* Do the two server DUIDs match? */
                    if (!matching_server_duids)
                    {
        
                        /* Don't respond, just discard the message. */
                        dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;
        
                        /* Clear the success status. */
                        dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_UNSPECIFIED;
        
                        return NX_SUCCESS;
                    }
                }
    
                /* Check for missing options request. */
                if (dhcpv6_client_ptr -> nx_dhcpv6_option_request.nx_option_length == 0)
                {
    
                    /* Don't respond, just discard the message. */
                    dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;
    
                    return NX_SUCCESS;
                }

                break;
            }

            default:
            {
    
                 dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server = NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT;
    
                 /* Clear the success status. */
                 dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_UNSPECIFIED;
    
                 return(NX_SUCCESS);
    
                /* No further processing to do. Leave the client state unchanged, whatever it was/is. */
            }
    } 

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_get_block_option_length   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function parses the input buffer (assuming to be an option     */
/*    block in a server reply) for option code and length. It assumes the */
/*    buffer pointer is pointed to the first byte of the option buffer.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_                             Pointer to DHCPv6 Client        */ 
/*    option                              Option code of requested data   */
/*    buffer_ptr                          Buffer to copy data to          */
/*    length                              Size of buffer                  */
/*                                              valid                     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_DHCPV6_OPTION_BLOCK_INCOMPLETE   Unknown option specified        */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_server_utility_get_data  Parses a specific data from the */ 
/*                                           DHCPv6 option                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_get_block_option_length                   */
/*                                     Parses option code and length      */
/*    _nx_dhcpv6_server_extract_packet_information                        */
/*                                     Parses each option from server     */
/*                                        reply and updates Client record */
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
UINT  _nx_dhcpv6_server_utility_get_block_option_length(UCHAR *buffer_ptr, ULONG *option, ULONG *length)
{
       
    /* Initialize to zero. */
    *option = 0;
    *length = 0;

    /* First byte should be the op code. */
    _nx_dhcpv6_server_utility_get_data(buffer_ptr, 2, option);

    buffer_ptr += 2;

    /* Next byte should be the option length. */
    _nx_dhcpv6_server_utility_get_data(buffer_ptr, 2, length);

    /* Buffer is now pointed at the data (past the length field). */
    buffer_ptr += 2;

    /* Check for null data. */
    if (*option == 0 || *length == 0)
    {

        return NX_DHCPV6_OPTION_BLOCK_INCOMPLETE; 
    }

    return(NX_SUCCESS);  
        
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_get_data                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function parses the input buffer and returns numeric data      */
/*    specified by the size argument, up to 4 bytes long.  Note that if   */ 
/*    caller is using this utility to extract bytes from a DHCPv6 packet  */
/*    there is no need for byte swapping, as compared to using memcpy in  */
/*    which case there is for little endian processors.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    buffer                            Pointer to data buffer            */ 
/*    value                             Pointer to data parsed from buffer*/
/*    size                              Size of buffer                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_DHCPV6_INVALID_DATA_SIZE       Requested data size too large     */
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*    _nx_dhcpv6_process_server_duid   Process server duid in server reply*/ 
/*    _nx_dhcpv6_process_client_duid   Process server duid in server reply*/ 
/*    _nx_dhcpv6_server_utility_get_block_option_length                   */
/*                                     Parses option code and length      */
/*    _nx_dhcpv6_server_extract_packet_information                        */
/*                                     Parses each option from server     */
/*                                        reply and updates Client record */
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
UINT  _nx_dhcpv6_server_utility_get_data(UCHAR *buffer, UINT size, ULONG *value)
{


    /* Check that the size of the data fits in a ULONG. */
    if (size > sizeof(ULONG))
    {
        return NX_DHCPV6_INVALID_DATA_SIZE;
    }

    *value = 0;
    
    /* Process the data retrieval request.  */
    while (size-- > 0)
    {

        /* Build return value.  */
        *value = (*value << 8) | *buffer++;
    }

    /* Return value.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_time_randomize            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function returns a value of between -1 seconds and 1 second    */ 
/*    in system ticks.  It is used to randomize timeouts as required by   */ 
/*    the RFC's so as to not overload a network server after power out.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    ticks                                 Number of ticks between 1 & -1*/ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_convert_delay_to_ticks     Convert seconds to ticks      */ 
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
INT  _nx_dhcpv6_server_utility_time_randomize(void)
{

INT temp;
UINT sign;


    temp = (INT)(NX_RAND() & 0x001F);
    sign = temp & 0x8;

    /* Try for a number between 0 and 0x1F. */
    if (sign)
    {
        temp = -temp;
    }
    
    return temp;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_listen_for_messages                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function listens for DHCPv6 client messages. It validates and  */
/*    extracts information for creating or updating the client record. It */
/*    also responds to the Client pending valid request is received, and  */
/*    updates its client records.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                           Pointer to DHCPv6 Server       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Message handled successfully   */ 
/*    NO_PACKET                            No packet received             */
/*    NX_DHCP_BAD_INTERFACE_INDEX          Packet interface not recognized*/
/*    status                               Actual completion outcome      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release DHCP packet           */ 
/*    nx_udp_socket_receive                 Receive DHCP packet           */ 
/*    _nx_dhcpv6_extract_information        Extract DHCP packet info      */ 
/*    _nx_dhcpv6_validate_client_message    Process the Client message    */
/*    _nx_dhcpv6_server_assign_ip_address   Assign IP address to Client   */
/*    _nx_dhcpv6_respond_to_client_message  Create and send response back */
/*    _nx_dhcpv6_clear_client_session       Clears client session data    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_thread_entry        DHCPv6 Server thread task     */ 
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
UINT  _nx_dhcpv6_listen_for_messages(NX_DHCPV6_SERVER *dhcpv6_server_ptr)
{

UINT                    status;
UINT                    iface_index;
NXD_ADDRESS             source_address;
NXD_ADDRESS             destination_address;
NX_IPV6_HEADER          *ipv6_header_ptr;
NX_PACKET               *packet_ptr;
NX_DHCPV6_CLIENT        *dhcpv6_client_ptr;
NX_DHCPV6_ADDRESS_LEASE *dhcpv6_interface_list_ptr;
NX_PACKET               *new_packet_ptr;
ULONG                   bytes_copied = 0;


    /* Check for an incoming DHCPv6 packet with non blocking option. */
    status = nx_udp_socket_receive(&dhcpv6_server_ptr -> nx_dhcpv6_server_socket, &packet_ptr, NX_DHCPV6_PACKET_WAIT_OPTION);

    /* Check for packet receive errors. */
    if (status != NX_SUCCESS)
    {

        /* Return the packet receive status. */
        return(status);
    }

    /* Check for valid packet length (message type and ID).  */
    if (packet_ptr -> nx_packet_length < 4)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        /* Return.  */
        return(NX_DHCPV6_INVALID_DATA_SIZE);
    }

    /* Get the interface index.  */
     nxd_udp_packet_info_extract(packet_ptr, NX_NULL, NX_NULL, NX_NULL, &iface_index);

    /* Does the DHCP server have a table for this packet interface? */
    if (iface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {

        /* No; Release the packet. */
        nx_packet_release(packet_ptr);

        /* No, return the error status. */
        return(NX_DHCPV6_INVALID_INTERFACE_INDEX);
    }

    /* Set a pointer to the IPv6 header to obtain source and destination address.  */
    ipv6_header_ptr = (NX_IPV6_HEADER*)(packet_ptr -> nx_packet_ip_header);

    /* Get the source address. */
    COPY_IPV6_ADDRESS(&ipv6_header_ptr -> nx_ip_header_source_ip[0], &source_address.nxd_ip_address.v6[0]);
    source_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Get the destination address. */
    COPY_IPV6_ADDRESS(&ipv6_header_ptr -> nx_ip_header_destination_ip[0], &destination_address.nxd_ip_address.v6[0]);
    destination_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* We will copy the received packet (datagram) over to a packet from the DHCP Server pool and release
       the packet from the receive packet pool as soon as possible. */
    status =  nx_packet_allocate(dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr, &new_packet_ptr, NX_IPv6_UDP_PACKET, NX_DHCPV6_PACKET_TIME_OUT);

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

        return(NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD);
    }          

    /* Use a packet from the DHCP Server as a buffer to store the received packet data.
       Then we can release the received packet back to its packet pool. */
    status = nx_packet_data_extract_offset(packet_ptr, 0, (VOID *)new_packet_ptr -> nx_packet_prepend_ptr, packet_ptr -> nx_packet_length, &bytes_copied);

    /* Check status.  */
    if ((status != NX_SUCCESS) || (bytes_copied == 0))
    {
                                    
        /* Release the original packet. */
        nx_packet_release(packet_ptr);

        /* Release the allocated packet we'll never send. */
        nx_packet_release(new_packet_ptr);

        /* Error extracting packet buffer, return error status.  */
        return(status);
    }                 

    /* Update the new packet with the bytes copied.  For chained packets, this will reflect the total
       'datagram' length. */
    new_packet_ptr -> nx_packet_length = bytes_copied;
    new_packet_ptr -> nx_packet_append_ptr = new_packet_ptr -> nx_packet_prepend_ptr + bytes_copied;
             
    /* Now we can release the original packet. */
    nx_packet_release(packet_ptr);

    /* Get the DHCPv6 Server Mutex.   */
    tx_mutex_get(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex, NX_WAIT_FOREVER);

    /* Extract the DHCPv6 specific information from the packet. This will create new Client record or update existing
       client record in the server table. */
    status = _nx_dhcpv6_server_extract_packet_information(dhcpv6_server_ptr, &dhcpv6_client_ptr, new_packet_ptr, iface_index, source_address, destination_address);

    /* We are done with this packet now regardless of the success of the above operations. */
    nx_packet_release(new_packet_ptr);

    /* Check if we have extracted a valid client record. */
    if (status != NX_SUCCESS) 
    {

        /* Not valid; is there a client record we need to clean up? */
        if (dhcpv6_client_ptr)
        {

           /* Yes; clear the client record completely. */
            _nx_dhcpv6_clear_client_record(dhcpv6_server_ptr, dhcpv6_client_ptr);
        }
        else
        {

            /* No record created; return the error status and abort. */
            tx_mutex_put(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex);

            /* Return the error from extracting packet data. */
            return(status);
        }
    }

    /* Determine if the server will respond to the message. */
    if ((dhcpv6_client_ptr == NX_NULL) ||
        (dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server == NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT))
    {

        /* Release the mutex.  */
        tx_mutex_put(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex);

        /* No, no further processing necessary. */
        return NX_SUCCESS;
    }

    /* Check if the server status indicates it will grant the client request. */
    if (dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code == NX_DHCPV6_STATUS_SUCCESS) 
    {

        /* Yes it will. Check if the client is requesting an IP address, or renewing or rebinding a 
           previously assigned one. */
        if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT) ||
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REQUEST) ||
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RENEW) ||
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REBIND))
        {

            /* Yes, it is.  This will (re)assign an IP address. */
            status = _nx_dhcpv6_assign_ip_address(dhcpv6_server_ptr, dhcpv6_client_ptr, &dhcpv6_interface_list_ptr);
    
            /* Check for errors or no address assigned. */
            if (status != NX_SUCCESS)  
            {

                /* Release the mutex.  */
                tx_mutex_put(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex);
    
                /* Return all other errors as completion status. Do not send a message to the client. 
                   The server will otherwise wait for the client to try again. Do not clear client record. */
                return(status);
            }
    
            /* Check if a valid IP address is available.  */
            if (dhcpv6_interface_list_ptr)
            {
    
                /* There is. DHCPv6 client has been assigned an IP address. */
        
                /* Now ensure we are sending valid lease time parameters with the IP address. */
                if (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REQUEST)
                {
                
                    /* Ignore invalid preferred or valid lifetimes from the client by setting to zero. */
                    if (dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime > dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime)
                    {
        
                        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = 0;
                        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = 0;
                    }
                    /* Check for invalid T1 or T2 values; if out of range, set to zero). */

                    if (dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime && 
                        (dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 > dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime))     
                    {
                        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = 0;
                        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = 0;
                    }
                    else if (dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime && 
                            (dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 > dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime))
                    {
                        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = 0; 
                        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = 0;
                    }
                    else if (dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 > dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2)
                    {
                        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = 0;
                        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = 0;
                    }
                }
                /* For a solicit message, ignore the preferred and valid lifetimes and check for
                   valid T1 and T2 times. */
                else if (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT)
                {

                    if (dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 > dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2)
                    {
                        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = 0;
                        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = 0;
                    }
                }
        
                /* If lifetime values are zero, or exceed the server default time, set to the default lease times. */
                if ((dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime == 0) || 
                    (dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime > NX_DHCPV6_DEFAULT_PREFERRED_TIME))
                {
    
                    dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = NX_DHCPV6_DEFAULT_PREFERRED_TIME; 
                }
                if ((dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime == 0) || 
                    (dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime > NX_DHCPV6_DEFAULT_VALID_TIME))
                {
    
                    dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = NX_DHCPV6_DEFAULT_VALID_TIME; 
                }
    
                if ((dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 == 0) || 
                    (dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 > NX_DHCPV6_DEFAULT_T1_TIME))
                {
    
                    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = NX_DHCPV6_DEFAULT_T1_TIME; 
                }
    
                if ((dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 == 0) || 
                    (dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 > NX_DHCPV6_DEFAULT_T2_TIME))
                {
    
                    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = NX_DHCPV6_DEFAULT_T2_TIME; 
                }
        
                /* Update the lease length in the server lease table. */
                dhcpv6_interface_list_ptr -> nx_dhcpv6_lease_preferred_lifetime = dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime;
                dhcpv6_interface_list_ptr -> nx_dhcpv6_lease_valid_lifetime = dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime;
                dhcpv6_interface_list_ptr -> nx_dhcpv6_lease_T1_lifetime = dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1;
                dhcpv6_interface_list_ptr -> nx_dhcpv6_lease_T2_lifetime = dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2;
        
                /* Ok to copy this address into the client record. */
                COPY_NXD_ADDRESS(&dhcpv6_interface_list_ptr -> nx_dhcpv6_lease_IP_address,
                                 &dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address);
            }
        }
    }

    /* Send the server reply back to the client. */
    status = _nx_dhcpv6_send_response_to_client(dhcpv6_server_ptr, dhcpv6_client_ptr); 

    /* Check if we need to remove the client record e.g. if the Client sent a Decline message. */
    if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE) ||
        (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE))
    {

        /* This will clear the client record and in the case of 
           a release message, return the IP address lease status to available. */
        status = _nx_dhcpv6_clear_client_record(dhcpv6_server_ptr, dhcpv6_client_ptr);

    }

    tx_mutex_put(&dhcpv6_server_ptr -> nx_dhcpv6_server_mutex);

    /* Return actual outcome status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_response_to_client                  PORTABLE C      */ 
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function prepares DHCPv6 messages to send back to the Client.  */
/*    Most of the information is obtained from the client session record. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                     Pointer to DHCPv6 Server      */ 
/*    dhcpv6_client_ptr                     Pointer to DHCPv6 Client      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a DHCPv6 packet      */ 
/*    nx_packet_release                     Release DHCPv6 packet         */ 
/*    nxd_udp_socket_send                   Send DHCPv6 packets           */ 
/*    memcpy                                Copy specified area of memory */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_dhcpv6_listen_for_messages         Process Client DHCPv6 messages*/
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  03-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_send_response_to_client(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr)
{

NX_PACKET     *packet_ptr;
UCHAR         *buffer;
UINT          status;
UINT          buffer_index;
ULONG         message_word;


    /* Allocate a UDP packet from the DHCPv6 server packet pool.  */
    status =  nx_packet_allocate(dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr, &packet_ptr, NX_IPv6_UDP_PACKET, NX_DHCPV6_PACKET_TIME_OUT);

    /* Was the packet allocation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Return status.  */
        return(status);
    }

    /* Verify packet payload. */
    if ((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 4)
    {
        nx_packet_release(packet_ptr);
        return(NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD);
    }

    /* Indicate this is an IPv6 packet. */
    packet_ptr -> nx_packet_ip_version = NX_IP_VERSION_V6;

    /* Setup the packet buffer pointer.  */
    buffer =  packet_ptr -> nx_packet_prepend_ptr;

    /* Clear the payload and packet interface field before preparing the packet for transmission. */
    memset(buffer, 0, (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr));

    /* Now reduce it to three bytes. */
    dhcpv6_client_ptr -> nx_dhcpv6_message_xid = dhcpv6_client_ptr -> nx_dhcpv6_message_xid & 0x0ffffff;
    
    /* Clear memory to make the message header. */
    memset(&message_word, 0, sizeof(ULONG));

    /* Add the server message type and matching client transaction ID as the DHCPv6 header fields. */
    message_word = (ULONG)((dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server << 24) | 
                            (0x0FFFFFF & dhcpv6_client_ptr -> nx_dhcpv6_message_xid));

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy the message header to the packet buffer. */
    memcpy(buffer, &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */

    /* Update the buffer 'pointer'. */
    buffer_index = sizeof(ULONG);

    /* Handle the special case of an Inform Request which does not include a Client identifier. */
    if (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST)
    {

        status = NX_SUCCESS;

        /* Only add the Client DUID if the client included one. */
        if (dhcpv6_client_ptr -> nx_dhcpv6_client_duid.nx_op_code != 0)
        {

            /* Add the Client DUID to the packet buffer. */
            status = _nx_dhcpv6_add_duid(dhcpv6_server_ptr, &dhcpv6_client_ptr -> nx_dhcpv6_client_duid, buffer, 
                                         &buffer_index, NX_DHCPV6_CLIENT_DUID_TYPE);
        }
    }
    else
    {
    
        /* Add the Client DUID to the packet buffer. */
        status = _nx_dhcpv6_add_duid(dhcpv6_server_ptr, &dhcpv6_client_ptr -> nx_dhcpv6_client_duid, buffer, 
                                     &buffer_index, NX_DHCPV6_CLIENT_DUID_TYPE);
    }

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
        /* Release the allocated back to the DHCPv6 Server packet pool. */
        nx_packet_release(packet_ptr);

        return status;
    }

    /* Now add to the server DUID to the message. */
    status = _nx_dhcpv6_add_duid(dhcpv6_server_ptr, &dhcpv6_server_ptr -> nx_dhcpv6_server_duid, buffer, 
                                 &buffer_index, NX_DHCPV6_SERVER_DUID_TYPE);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

        /* Release the allocated back to the DHCPv6 Server packet pool. */
        nx_packet_release(packet_ptr);

        return status;
    }

    /* Handle the special case of the Confirm request where we need only
       send a status option. */
    if (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_CONFIRM)
    {

        /* Update the IANA status message. */
        status = _nx_dhcpv6_prepare_iana_status(&dhcpv6_client_ptr -> nx_dhcpv6_iana_status, NX_TRUE);

        if (status != NX_SUCCESS)
        {
            /* Release the allocated back to the DHCPv6 Server packet pool. */
            nx_packet_release(packet_ptr);

            return status;
        }

        /* Ok to just send a status option without the IANA option. */
        status = _nx_dhcpv6_add_iana_status(dhcpv6_server_ptr, &dhcpv6_client_ptr -> nx_dhcpv6_iana_status, buffer, &buffer_index);

        /* Update the IANA status message. */
        status = _nx_dhcpv6_prepare_iana_status(&dhcpv6_client_ptr -> nx_dhcpv6_iana_status, NX_FALSE);

        if (status != NX_SUCCESS)
        {
            /* Release the allocated back to the DHCPv6 Server packet pool. */
            nx_packet_release(packet_ptr);

            return status;
        }
    }

    /* Exclude IANA options from Confirm and Inform Request messages. */
    if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type != NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST) &&
        (dhcpv6_client_ptr -> nx_dhcpv6_message_type != NX_DHCPV6_MESSAGE_TYPE_CONFIRM))
    {
    
        /* Add the client IANA to the packet buffer. */
        status = _nx_dhcpv6_server_add_iana(dhcpv6_server_ptr, dhcpv6_client_ptr, buffer, &buffer_index);
    
        /* Check for error. */
        if (status != NX_SUCCESS)
        {
            /* Release the allocated back to the DHCPv6 Server packet pool. */
            nx_packet_release(packet_ptr);
       
            return status;
        }
    }

    /* Only an Advertise response should include a preference. */
    if (dhcpv6_client_ptr -> nx_dhcpv6_response_back_from_server == NX_DHCPV6_MESSAGE_TYPE_ADVERTISE) 

    {

        status = _nx_dhcpv6_add_preference(dhcpv6_server_ptr, dhcpv6_client_ptr, buffer, &buffer_index);
    }

    /* Check if the message type requires the server to add option request data. */
    if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT) ||
        (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REQUEST) ||
        (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST))
    {
    
        /* It does. Did the Client ask for a requested option? Only provide information 
           if there are no status errors from the server. */
        if (dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code == NX_DHCPV6_STATUS_SUCCESS)
        {
    
            /* Add the option request. */
            status = _nx_dhcpv6_add_option_requests(dhcpv6_server_ptr, dhcpv6_client_ptr, buffer, &buffer_index); 
    
            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                /* Release the allocated back to the DHCPv6 Server packet pool. */
                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }
        }
    }

    /* Adjust the packet length for the DHCPv6 data. */
    packet_ptr -> nx_packet_length = buffer_index; 

    /* Update the packet pointer marking the end of the payload (DHCP) data. */
    packet_ptr -> nx_packet_append_ptr += buffer_index;

    /* Send the response packet to the client.  */    
    status = nxd_udp_socket_interface_send(&(dhcpv6_server_ptr -> nx_dhcpv6_server_socket), packet_ptr, 
                                           &dhcpv6_client_ptr -> nx_dhcp_source_ip_address, NX_DHCPV6_CLIENT_UDP_PORT, 
                                           dhcpv6_server_ptr -> nx_dhcpv6_server_interface_index);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
        /* Release the allocated back to the DHCPv6 Server packet pool. */
        nx_packet_release(packet_ptr);
    }
    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpc6_server_assign_ip_address                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This service finds an available IP address for the client if none is  */
/*  provided, and updates the client's record with the new IP address. It */ 
/*  updates the server IP address lease table with the assigned lease     */
/*  status                                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                       Pointer to DHCPV6 Server    */
/*    dhcpv6_client_ptr                       Pointer to DHCPV6client     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion         */ 
/*    NX_DHCPV6_NO_AVAILABLE_IP_ADDRESSES   No available address to assign*/ 
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
/*    _nx_dhcpv6_listen_for_messages        Process Client DHCPv6 messages*/
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
UINT  _nx_dhcpv6_assign_ip_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr,
                                   NX_DHCPV6_ADDRESS_LEASE **interface_address_ptr)
{
UINT                            status;
UINT                            i;
NX_DHCPV6_ADDRESS_LEASE         *temp_interface_address_ptr;
UINT                            matching;


    /* Initialize outcome to no address assigned. */
    *interface_address_ptr = NX_NULL;

    /* Does the Client have a IP address already assigned? */
    if (!CHECK_UNSPECIFIED_ADDRESS(&dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0]))
    {

        /* Yes; The client has an IP address. */

        /* Check if this address exists in the Server IP address list. */
        status = _nx_dhcpv6_find_ip_address(dhcpv6_server_ptr, dhcpv6_client_ptr, &temp_interface_address_ptr);

        /* Check for internal errors. */
        if (status != NX_SUCCESS)
        {

            return status;
        }

        /* Was the address found in the server table? */
        if (temp_interface_address_ptr)
        {

            matching = NX_FALSE;

            /* Verify the server has assigned this address. */
            if (temp_interface_address_ptr -> nx_dhcpv6_lease_assigned_to)
            {
                /* It has; now werify it is assigned to this client. */
                status = _nx_dhcpv6_check_duids_same(&dhcpv6_client_ptr -> nx_dhcpv6_client_duid, 
                                                     &(temp_interface_address_ptr -> nx_dhcpv6_lease_assigned_to -> nx_dhcpv6_client_duid), 
                                                     &matching);
    
                /* Check for internal errors. */
                if (status != NX_SUCCESS)
                {
       
                    return status;
                }
            }
            else
            {
                /* It has not (so it is available). Assign it to this client. */

                 /* Set the lease address entry in the server table. */
                *interface_address_ptr = temp_interface_address_ptr;

                /* Mark the owner of the lease as this client. */
                temp_interface_address_ptr -> nx_dhcpv6_lease_assigned_to = dhcpv6_client_ptr; 

                /* We are all done. */
                return NX_SUCCESS;
            }

            /* Was the address assigned to this client previously? */
            if (matching)
            {

                /* Yes, return a pointer to the IP lease in the server table. */
                *interface_address_ptr = temp_interface_address_ptr;

                /* We will (re)assign this IP address to the client. */
                return NX_SUCCESS;
            }
            else
            {
                /* No this address is not available. */

                /* Is it Ok to assign another IP address only if this is a Solicit request? */
                if (dhcpv6_client_ptr -> nx_dhcpv6_message_type != NX_DHCPV6_MESSAGE_TYPE_SOLICIT)
                {
                
                   /* No; Reply back the client request can not be granted. */
                   dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_NO_BINDING;

                   /* Because the server can not validate the IP address to assign, 
                     set the lifetimes to zero RFC 3315 Sect 18.2.3, 18.2.4. */
                   dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = 0;
                   dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = 0;
                   dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = 0;
                   dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = 0;

                   dhcpv6_client_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_UNBOUND;
                
                   return NX_SUCCESS;
                }
            }
        }
        else
        {

            /* Client address not found in table.  */

            /* Ok to assign another IP address only if this is a Solicit request? */
            if (dhcpv6_client_ptr -> nx_dhcpv6_message_type != NX_DHCPV6_MESSAGE_TYPE_SOLICIT)
            {

                /* No; reply backthe client request can not be granted. */
                dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_NO_BINDING;

                /* Because the server can not validate the IP address to assign, set the lifetimes to zero RFC 3315 Sect 18.2.3, 18.2.4. */
                dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = 0;
                dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = 0;
                dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = 0;
                dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = 0;
    
                dhcpv6_client_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_UNBOUND;
                            
                return NX_SUCCESS;
            }
        }
    }

    /* If we reach this point, we need to find an available address for the client. */
    for (i = 0; i < NX_DHCPV6_MAX_LEASES; i++)
    {

        if (!dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_assigned_to)
        {
            break;
        }
    }

    /* Check if we found anything. */
    if (i == NX_DHCPV6_MAX_LEASES)
    {

        /* No; reply back there are no addresses available. */
        dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code = NX_DHCPV6_STATUS_NO_ADDRS_AVAILABLE;

        /* Because the server can not assign the IP address, set the lifetimes to zero RFC 3315 Sect 18.2.3, 18.2.4. */
        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = 0;
        dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = 0;
        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = 0;
        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = 0;

        dhcpv6_client_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_UNBOUND;
    }
    else
    {

        /* Found an empty slot; assign it to this client. */

        /* Mark the owner of the lease as this client. */
        dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_assigned_to = dhcpv6_client_ptr;

        /* Return the lease address entry in the server table. */
        *interface_address_ptr = &(dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i]);
    }

    return NX_SUCCESS;
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
/*    dhcpv6_server_ptr                     Pointer to DHCPv6 Server      */ 
/*    dhcpv6_duid _ptr                      Pointer to client DUID        */
/*    record_index                          Index of client record found  */
/*                                             in server table            */
/*    add_on                                Option to add if not found    */
/*    message_xid                           Message transaction ID        */
/*    matching                              Indicates if client existed in*/
/*                                             server records already     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Message handled successfully   */ 
/*    NX_DHCPV6_TABLE_FULL                 Client record table is full    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*    _nx_dhcpv6_check_duids_same          Checks if DUIDs are the same   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_extract_packet_information                        */
/*                                         Extract DHCPv6 info from       */ 
/*                                             Client message             */
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
UINT  _nx_dhcpv6_find_client_record_by_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_SVR_DUID *duid_ptr, 
                                            UINT *record_index, UINT add_on, ULONG message_xid, UINT *matching)
{

UINT                i;
UINT                status;
UINT                available_index;
NX_DHCPV6_CLIENT    *client_record_ptr;


    /* Initialize result to not found. */
    *matching = NX_FALSE;

    /* Initialize an available slot in the table as outside the boundary (e.g. no slots available). */
    available_index = NX_DHCPV6_MAX_CLIENTS;

    /* Records are not necessarily added and deleted sequentially, 
       so search the whole table until a match is found. */
    i = 0;
    status = NX_SUCCESS;

    while (i < NX_DHCPV6_MAX_CLIENTS) 
    {

        /* Set local pointer for convenience. */
        client_record_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_clients[i];

        /* Skip empty records.  */
        if (!client_record_ptr -> nx_dhcpv6_id)
        {

            /* Flag the first empty record in case we need to add the current client to the table. */
            if (i < available_index)
                available_index = i;

            i++;

            continue;
        }

        status = _nx_dhcpv6_check_duids_same(&client_record_ptr -> nx_dhcpv6_client_duid, duid_ptr, matching);

        /* Check the DUID in the received client request against the server record. */
        if ((status == NX_SUCCESS) && *matching)
        {

            client_record_ptr -> nx_dhcpv6_message_xid = message_xid;

            
            /* Reset the client session time every time we hear from a client. */
            client_record_ptr -> nx_dhcpv6_client_session_time = 1;
            
            /* Got a match! Return the client record location. */
            *record_index = i;
            
            /* Set the result to client located. */
            *matching = NX_TRUE;
            
            return(NX_SUCCESS);
        }

        /* No match. Check the next client in the server table. */
        i++;
    }

    /* Not found. Create a record for this client? */
    if (!add_on)
    {

        /* Do not create a record and handle as an error. */
        return(NX_DHCPV6_CLIENT_RECORD_NOT_FOUND);
    }

    /* Check if there is available room in the table for a new client. */
    if (available_index >= NX_DHCPV6_MAX_CLIENTS)
    {

        /* No, we cannot add this client so the server's table. */
        return(NX_DHCPV6_TABLE_FULL);
    }

    /* Set local pointer to an available slot. */
    client_record_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_clients[available_index];

    /* Return the location of the newly created client record. */
    *record_index = available_index; 

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_find_ip_address                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function looks up a client DHCPv6 message IP address in the    */
/*    appropriate server IP address table, based on the client network    */
/*    interface index. For single homed hosts, this defaults to zero.     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                     Pointer to DHCPv6 Server      */ 
/*    dhcpv6_client_ptr                     Pointer to DHCPv6 client      */
/*    interface_address_ptr                 Pointer to matching entry     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Search completes successfully */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcp_clear_client_record          Removes client record from    */
/*                                              server table              */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    CHECK_IPV6_ADDRESSES_SAME             Does IPv6 address comparison  */ 
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
UINT _nx_dhcpv6_find_ip_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, 
                                NX_DHCPV6_ADDRESS_LEASE **interface_address_ptr)
{

UINT i;
NXD_ADDRESS server_table_address;


    /* Initialize to not found. */
    *interface_address_ptr = NX_NULL;

    /* Search through the interface specific address table. For single homed hosts, there is only one address table. */
    for (i = 0; i < NX_DHCPV6_MAX_LEASES; i++)
    {

        /* Get a pointer to the address, using the client's interface index to access the correct IP address table. */
        server_table_address = dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_IP_address;

        /* Check for a match. */
        if (CHECK_IPV6_ADDRESSES_SAME(&server_table_address.nxd_ip_address.v6[0], 
                                      &dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0]))
        {

            /* Set the location of the matching server entry, to indicate we found a match. */
            *interface_address_ptr = &dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i];
            break;
        }
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_clear_client_record                      PORTABLE C      */ 
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
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                           Pointer to DHCPV6 Server       */ 
/*    dhcpv6_client_ptr                    Pointer to DHCPV6 Client       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_SUCCESS                           Successful outcome             */
/*    NX_DHCP_BAD_INTERFACE_INDEX          Invalid client interface index */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                         Get server protection mutex    */ 
/*    tx_mutex_put                         Release server protection mutex*/ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_dhcpv6_listen_for_messages         Listen for client requests    */
/*   _nx_dhcpv6_find_client_record_by_chaddr                              */ 
/*                                          Find client in server table   */
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
UINT  _nx_dhcpv6_clear_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr)
{

UINT   i;


    if (!dhcpv6_client_ptr)
    {

        /* Benign error. Return success. */
        return(NX_SUCCESS);
    }

    i = 0;

    /* Does the client have an assigned IP address? */
    if (!CHECK_UNSPECIFIED_ADDRESS(&dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0]))
    {
    
        /* Yes, We need to free up the Client's assigned IP address in the server database. */
        while (i < NX_DHCPV6_MAX_LEASES)
        {

            tx_mutex_get(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex), NX_WAIT_FOREVER);

            /* Find the interface table entry by matching IP address. */
            if (CHECK_IPV6_ADDRESSES_SAME(&dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_IP_address.nxd_ip_address.v6[0], 
                                          &dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0]))
            {

                /* Clear the IP address list item properties. */
                if (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE)
                {

                    /* Apparently another client has this address so do not clear the assignment status. */
                    dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_assigned_to = (NX_DHCPV6_CLIENT *)(ALIGN_TYPE)0xFFFFFFFF;
                }
                else
                {

                    /* Restore availability by clearing the owner. */
                    dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_assigned_to = NX_NULL;
                }

                dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_T1_lifetime = 0;
                dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_T2_lifetime = 0;
                dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_valid_lifetime = 0;
                dhcpv6_server_ptr -> nx_dhcpv6_lease_list[i].nx_dhcpv6_lease_preferred_lifetime = 0;

                tx_mutex_put(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex));

                /* Address now available for DHCP client. */
                break;
            }
            i++;

            tx_mutex_put(&(dhcpv6_server_ptr -> nx_dhcpv6_server_mutex));

        }
    }

    /* Ok to clear the Client record. */
    memset(dhcpv6_client_ptr, 0, sizeof(NX_DHCPV6_CLIENT));

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_update_client_record                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the client record based on the most recent    */
/*    message received from the client. A typical example is after a      */
/*    Solicit message is received the server gets a Request message which */
/*    it will add to the client record it has in its table.               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                    Pointer to DHCPV6 Server       */ 
/*    to_server_ptr                        Pointer to client to update    */ 
/*    from_client_ptr                      Pointer to client updated from */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_SUCCESS                           Successful outcome             */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcpy                               Copy specified area of memory  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_extract_packet_information                        */
/*                                         Process DHCPv6 data in packet  */
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
UINT  _nx_dhcpv6_update_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, 
                                      NX_DHCPV6_CLIENT *from_extracted_client_data_ptr, 
                                      NX_DHCPV6_CLIENT *to_client_data_on_record_ptr)
{

    NX_PARAMETER_NOT_USED(dhcpv6_server_ptr);

    /* If this is a solicit message update the entire copy into the specified record. */
    if (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT)
    {

        /* Ok to copy the whole record over. */
        memcpy(to_client_data_on_record_ptr, from_extracted_client_data_ptr, sizeof(NX_DHCPV6_CLIENT)); /* Use case of memcpy is verified. */

        /* Update various fields now that the client is in a DHCPv6 session with the server. */
        to_client_data_on_record_ptr -> nx_dhcpv6_client_session_time = 1;
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_client_duid, &from_extracted_client_data_ptr -> nx_dhcpv6_client_duid, sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_server_duid, &from_extracted_client_data_ptr -> nx_dhcpv6_server_duid, sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */
    }
    else if (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST)
    {

        /* Update selected fields for an INFORM REQUEST. */
        to_client_data_on_record_ptr -> nx_dhcpv6_id = from_extracted_client_data_ptr -> nx_dhcpv6_id;                               
        to_client_data_on_record_ptr -> nx_dhcpv6_message_xid = from_extracted_client_data_ptr -> nx_dhcpv6_message_xid;             
        to_client_data_on_record_ptr -> nx_dhcpv6_message_type = from_extracted_client_data_ptr -> nx_dhcpv6_message_type;            
        to_client_data_on_record_ptr -> nx_dhcpv6_response_back_from_server = from_extracted_client_data_ptr -> nx_dhcpv6_response_back_from_server;   

        COPY_IPV6_ADDRESS(&from_extracted_client_data_ptr -> nx_dhcp_source_ip_address.nxd_ip_address.v6[0], 
                          &to_client_data_on_record_ptr -> nx_dhcp_source_ip_address.nxd_ip_address.v6[0]);
        to_client_data_on_record_ptr -> nx_dhcp_source_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

        COPY_IPV6_ADDRESS(&from_extracted_client_data_ptr -> nx_dhcp_destination_ip_address.nxd_ip_address.v6[0], 
                          &to_client_data_on_record_ptr -> nx_dhcp_destination_ip_address.nxd_ip_address.v6[0]);
        to_client_data_on_record_ptr -> nx_dhcp_destination_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_option_request, &from_extracted_client_data_ptr -> nx_dhcpv6_option_request, sizeof(NX_DHCPV6_SERVER_OPTIONREQUEST)); /* Use case of memcpy is verified. */
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_client_duid, &from_extracted_client_data_ptr -> nx_dhcpv6_client_duid, sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_server_duid, &from_extracted_client_data_ptr -> nx_dhcpv6_server_duid, sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */

    }
    /* If this is not a solicit message, update the selected fields into the specified record. */
    else if ((from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REQUEST) ||
             (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RENEW)   ||
             (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REBIND)  ||
             (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE) ||
             (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE) ||
             (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_CONFIRM))
    {


        /* Ok to update the existing client record. 

            Note: if this is a duplicate client request we update the client record just 
            the same. */
        to_client_data_on_record_ptr -> nx_dhcpv6_id = from_extracted_client_data_ptr -> nx_dhcpv6_id;                               
        to_client_data_on_record_ptr -> nx_dhcpv6_message_xid = from_extracted_client_data_ptr -> nx_dhcpv6_message_xid;             
        to_client_data_on_record_ptr -> nx_dhcpv6_message_type = from_extracted_client_data_ptr -> nx_dhcpv6_message_type;            
        to_client_data_on_record_ptr -> nx_dhcpv6_response_back_from_server = from_extracted_client_data_ptr -> nx_dhcpv6_response_back_from_server;        
     
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_client_duid, &from_extracted_client_data_ptr -> nx_dhcpv6_client_duid, sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_server_duid, &from_extracted_client_data_ptr -> nx_dhcpv6_server_duid, sizeof(NX_DHCPV6_SVR_DUID)); /* Use case of memcpy is verified. */
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_elapsed_time, &from_extracted_client_data_ptr -> nx_dhcpv6_elapsed_time, sizeof(NX_DHCPV6_SERVER_ELAPSED_TIME)); /* Use case of memcpy is verified. */
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_iana, &from_extracted_client_data_ptr -> nx_dhcpv6_iana, sizeof(NX_DHCPV6_SERVER_IA_NA)); /* Use case of memcpy is verified. */
        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_ia, &from_extracted_client_data_ptr -> nx_dhcpv6_ia, sizeof(NX_DHCPV6_SERVER_IA_ADDRESS)); /* Use case of memcpy is verified. */

        /* Update the IANA status option code, length and status message. The DHCPv6 server 
            will figure out the actual lenght with the status message text added in later. */
        to_client_data_on_record_ptr -> nx_dhcpv6_iana_status.nx_status_code = from_extracted_client_data_ptr -> nx_dhcpv6_iana_status.nx_status_code;
        to_client_data_on_record_ptr -> nx_dhcpv6_iana_status.nx_option_length = sizeof(USHORT);

        COPY_IPV6_ADDRESS(&from_extracted_client_data_ptr -> nx_dhcp_source_ip_address.nxd_ip_address.v6[0], 
                          &to_client_data_on_record_ptr -> nx_dhcp_source_ip_address.nxd_ip_address.v6[0]);

        to_client_data_on_record_ptr -> nx_dhcp_source_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

        COPY_IPV6_ADDRESS(&from_extracted_client_data_ptr -> nx_dhcp_destination_ip_address.nxd_ip_address.v6[0], 
                          &to_client_data_on_record_ptr -> nx_dhcp_destination_ip_address.nxd_ip_address.v6[0]);

        to_client_data_on_record_ptr -> nx_dhcp_destination_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

        memcpy(&to_client_data_on_record_ptr -> nx_dhcpv6_option_request, &from_extracted_client_data_ptr -> nx_dhcpv6_option_request, sizeof(NX_DHCPV6_SERVER_OPTIONREQUEST)); /* Use case of memcpy is verified. */

        /* Update fields based on the state of the client. At this point the client is bound to a 
           valid IP address to the session is 'over'.  Set timeout fields to stop the session timer
           and begin the lease life timer. */
        to_client_data_on_record_ptr -> nx_dhcpv6_client_session_time = 0;

        /* Start the lease timer if we have just (re)assigned an IP lease. */
        if ((from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REQUEST) ||
            (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RENEW)   ||
            (from_extracted_client_data_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_REBIND))
        {

            to_client_data_on_record_ptr -> nx_dhcpv6_IP_lease_time_accrued = 1; 
            to_client_data_on_record_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_BOUND;
        }

        /* No need to set the client state to unbound. We will remove the client record and restore the
           IP address status back to available after sending the server reply. */
    }
    /* else
         Confirm, Inform Request and other messages should not change the client record.  */

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_process_iana                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the IANA option from a client DHCPv6 message.*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_client_ptr                 Pointer to DHCPV6 Client instance */ 
/*    option_code                       Option code                       */
/*    option_length                     Size of option data               */
/*    option_data                       Pointer to option data            */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_DHCPV6_INVALID_IANA_DATA       IANA option is missing data or    */
/*                                       improperly formatted             */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_get_data                                  */
/*                                      Parses data from packet buffer    */
/*    _nx_dhcpv6_server_utility_get_block_option_length                   */ 
/*                                      Parses option header from buffer  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_extract_packet_information                        */
/*                                      Extracts DHCPV6 data from packet  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_server_process_iana(NX_DHCPV6_CLIENT *dhcpv6_client_ptr, ULONG option_code, UINT option_length, UCHAR *option_data)
{

UINT  status;
ULONG data;
UINT  index;
ULONG iana_option_code, iana_option_length;
UINT  process_ia_option; 


    /* Initialize local variables. */
    process_ia_option = NX_TRUE;
    index = 0;

    /* Set option code and length.  */
    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_op_code = (USHORT)option_code;
    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_option_length = (USHORT)option_length;

    /* Check option length for IANA ID.  */
    if (option_length < 4)
    {
        return(NX_DHCPV6_INVALID_IANA_DATA);
    }

    /* The first word should contain the IANA ID. */
    _nx_dhcpv6_server_utility_get_data(option_data + index, sizeof(ULONG), &data);

    /* Process the IANA ID. */
    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_IA_NA_id = data;

    /* Update the write location index. */
    index += 4; 

    /* Check option length for T1 and T2.  */
    if (index + 8 > option_length)
    {
        return(NX_DHCPV6_INVALID_IANA_DATA);
    }

    /* Copy T1 and T2 from the buffer into IANA. */
    _nx_dhcpv6_server_utility_get_data(option_data + index, sizeof(ULONG), &data);
    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 = data;
    index += (ULONG)sizeof(ULONG); 

    _nx_dhcpv6_server_utility_get_data(option_data + index, sizeof(ULONG), &data);
    dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2 = data;
    index += (ULONG)sizeof(ULONG);

    /* Check for invalid T1/T2 lifetimes. */
    if (dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T1 > dhcpv6_client_ptr -> nx_dhcpv6_iana.nx_T2)
    {

        /* Reject the server DHCPv6 response. */
        return NX_DHCPV6_INVALID_IANA_TIME; 
    }

    /* Check if we're at the end of option data yet. */
    if (index == option_length)
    {

        /* Yes, all done. */
        return NX_SUCCESS;
    }

    /* Now we recurse into the options embedded in the IANA option. We do it here
       because this is part of the IANA option data length. */
    while (index + 4 <= option_length)
    {

        /* Get the next option code and length. */
        status = _nx_dhcpv6_server_utility_get_block_option_length(option_data + index, &iana_option_code, &iana_option_length);
       
        /* Check that the block data is valid. */
        if (status != NX_SUCCESS)
        {

            /* No, return the error status. */
            return status;
        }

        /* Skip IA option code and length.  */
        index += 4;

        /* This is a double check to verify we haven't gone off the end of the packet buffer. */
        if (index + iana_option_length > option_length)
        {
            return (NX_DHCPV6_INVALID_IANA_DATA);
        }

        /* Check if this is an IA address option request, and if we have already
           processed an IA in this message (the DHCPv6 server is limited to 
           one IA per client message). */
        if (iana_option_code == NX_DHCPV6_OP_IA_ADDRESS)
        {

            /* Yes it is, so process it. */
            status = _nx_dhcpv6_server_process_ia(dhcpv6_client_ptr, iana_option_code, iana_option_length, (option_data + index), process_ia_option);

            /* Check for errors processing the DHCPv6 message. */
            if (status != NX_SUCCESS)
            {

                /* Abort further processing. Return error status. */
                return status;
            }

            /* Indicate we already have an IA option if the client request contains multiple IAs. */
            process_ia_option = NX_FALSE;
        }

        /* Keep track of how far into the packet we have parsed. */
        index += iana_option_length; 
    } 

    /* Check if we went past the reported size of IA-NA data. */
    if (index != option_length) 
    {

        /* Yes return an error status. */
        return NX_DHCPV6_INVALID_IANA_DATA;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_option_request                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the option request from the client packet.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_client_ptr                 Pointer to DHCPV6 client instance */ 
/*    option_code                       Option code                       */
/*    option_length                     Size of option data               */
/*    option_data                       Pointer to option data            */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_get_data                                  */
/*                                      Parses data from specified buffer */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_extract_packet_information                        */
/*                                      Extracts DHCPv6 data from packet  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_option_request(NX_DHCPV6_CLIENT *dhcpv6_client_ptr, ULONG option_code, UINT option_length, UCHAR *option_data)
{

ULONG           data;
UINT            i = 0;
UINT            index = 0;


    /* Set option code and length.  */
    dhcpv6_client_ptr -> nx_dhcpv6_option_request.nx_op_code = (USHORT)option_code;
    dhcpv6_client_ptr -> nx_dhcpv6_option_request.nx_option_length = (USHORT)option_length;

    /* Loop to process requested option.  */
    while ((index + 2 <= option_length) && (i < NX_DHCPV6_MAX_OPTION_REQUEST_OPTIONS))
    {

        /* Extract the option request which should be the next 2 bytes.  */
        _nx_dhcpv6_server_utility_get_data(option_data + index, 2, &data);

        dhcpv6_client_ptr -> nx_dhcpv6_option_request.nx_op_request[i] = (USHORT)data;

        /* Update all the length and index counters.*/
        index += 2;
        i++;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_elapsed_time                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the elapsed time option in the client packet.*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_client_ptr                 Pointer to DHCPV6 client instance */ 
/*    option_code                       Option code                       */
/*    option_length                     Size of option data               */
/*    option_data                       Pointer to option data            */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_OPTION_DATA     Elapsed time is missing data      */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_get_data                                  */
/*                                      Parses data from specified buffer */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_extract_packet_information                        */
/*                                      Extracts DHCPv6 data from packet  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_elapsed_time(NX_DHCPV6_CLIENT *dhcpv6_client_ptr, ULONG option_code, UINT option_length, UCHAR *option_data)
{

ULONG           data;


    /* Set option code and length.  */
    dhcpv6_client_ptr -> nx_dhcpv6_elapsed_time.nx_op_code = (USHORT)option_code;
    dhcpv6_client_ptr -> nx_dhcpv6_elapsed_time.nx_option_length = (USHORT)option_length;

    /* Check option length for elapsed-time.  */
    if (option_length != 2)
    {
        return(NX_DHCPV6_INVALID_OPTION_DATA);
    }

    /* Extract the elapsed session time which should be the next 2 bytes.  */
    _nx_dhcpv6_server_utility_get_data(option_data, 2, &data);
    dhcpv6_client_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = (USHORT)data;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_process_ia                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the IA address option from the client packet.*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_client_ptr                 Pointer to DHCPV6 Client instance */ 
/*    option_code                       Option code                       */
/*    option_length                     Size of option data               */
/*    option_data                       Pointer to option data            */
/*    process_ia                        If true, write IA to client record*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_IA_DATA         IA option is missing data or      */
/*                                          improperly formatted          */
/*    NX_DHCPV6_INVALID_IA_TIME         IA option lifetime data is invalid*/
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_server_utility_get_data                                  */
/*                                      Extract bytes from a DHCPv6 packet*/
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_process_iana    Processes IANA in client request  */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_server_process_ia(NX_DHCPV6_CLIENT *dhcpv6_client_ptr, ULONG option_code, UINT option_length, UCHAR *option_data, UINT process_ia)
{

UINT  index = 0, k;
ULONG data;


    /* Fill in the IA address code and length. Client might already have one on
       record, but use the server's data instead. */
    if (process_ia)
    {
        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_op_code = (USHORT)option_code;
        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_option_length = (USHORT)option_length;
    }

    /* Check option length for Ipv6 address (16 bytes), preferred-lifetime (4 bytes) and valid-lifetime (4 bytes).  */
    if (option_length < 24)
    {
        return(NX_DHCPV6_INVALID_IA_DATA);
    }

    /* Process IPv6 address.  */
    for (k = 0; k < 4; k++)
    {

        /* Copy each IPv6 address word into the IA address. */
        _nx_dhcpv6_server_utility_get_data(option_data + index, sizeof(ULONG), &data);

        if (process_ia)
        {
            dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[k] = data;
        }

        /* Move to the next IPv6 address word. */
        index += 4;
    }

    /* Copy the preferred lifetime data from the client buffer to IA.*/
    _nx_dhcpv6_server_utility_get_data(option_data + index, sizeof(ULONG), &data);
    if (process_ia)
    {
        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = data;
    }
    index += 4;
    
    /* Copy the valid lifetime data from the client buffer to IA.*/
    _nx_dhcpv6_server_utility_get_data(option_data + index, sizeof(ULONG), &data);
    if (process_ia)
    {
        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = data;
    }
    index += 4;

    /* Check if we went past the reported size of IA address data. */
    if (index != option_length)
    {

        /* Return an error status. Cannot accept this reply. */
        return NX_DHCPV6_INVALID_IA_DATA;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_duid                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the DUID from the client packet.             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    duid_ptr                          Pointer to DHCPV6 DUID instance   */ 
/*    option_code                       Option code                       */
/*    option_length                     Size of option data               */
/*    option_data                       Pointer to option data            */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_DUID            DUID is missing data or does not  */
/*                                       match the DUID on record         */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_utility_get_data                                  */
/*                                      Parses data from specified buffer */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_extract_packet_information                        */
/*                                      Extracts DHCPv6 data from packet  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            corrected the logic of      */
/*                                            processing DUID type,       */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_duid(NX_DHCPV6_SVR_DUID *duid_ptr, ULONG option_code, UINT option_length, UCHAR *option_data)
{

ULONG           data;
UINT            index = 0;

    NX_PARAMETER_NOT_USED(option_code);

    /* Check option length for DUID type.  */
    if (option_length < 2)
    {
        return(NX_DHCPV6_INVALID_OPTION_DATA);
    }

    /* Extract the DUID type which should be the next 2 bytes.  */
    _nx_dhcpv6_server_utility_get_data(option_data + index, 2, &data);
    duid_ptr -> nx_duid_type = (USHORT)data;
    index += 2;

    /* Check the duid type.  */
    if ((duid_ptr -> nx_duid_type < NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME) ||
        (duid_ptr -> nx_duid_type > NX_DHCPV6_SERVER_DUID_TYPE_LINK_ONLY))
    {
        return(NX_DHCPV6_INVALID_DUID);
    }

    /* Check if it is a link layer duid.  */
    if ((duid_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME) ||
        (duid_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_ONLY))
    {

        /* Check option length for hardware type.  */
        if (index + 2 > option_length)
        {
            return(NX_DHCPV6_INVALID_OPTION_DATA);
        }

        /* Extract the hardware type which should be the next 2 bytes.  */
        _nx_dhcpv6_server_utility_get_data(option_data + index, 2, &data);
        duid_ptr -> nx_hardware_type = (USHORT)data;
        index += 2;

        /* IS this a link layer plus time DUID type? */
        if (duid_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME)
        {

            /* Check option length for time.  */
            if (index + 4 > option_length)
            {
                return(NX_DHCPV6_INVALID_OPTION_DATA);
            }

            /* Yes; Extract the time which should be the next 4 bytes.  */
            _nx_dhcpv6_server_utility_get_data(option_data + index, 4, &data);    
            duid_ptr -> nx_duid_time = data;
            index += 4;
        }

        /* Check option length mac address.  */
        if (index + 6 > option_length)
        {
            return(NX_DHCPV6_INVALID_OPTION_DATA);
        }

        /* Extract the link local address msw which should be the next 2 bytes.  */
        _nx_dhcpv6_server_utility_get_data(option_data + index, 2, &data);
        duid_ptr -> nx_link_layer_address_msw = (USHORT)data;
        index += 2;

        /* Extract the link local address lsw which should be the next 4 bytes.  */
        _nx_dhcpv6_server_utility_get_data(option_data + index, 4, &data);
        duid_ptr -> nx_link_layer_address_lsw = data;
        index += 4;
    }
    else
    {

        /* DUID Assigned by Vendor Based on Enterprise Number.  */

        /* Check option length for enterprise-number.  */
        if (index + 4 > option_length)
        {
            return(NX_DHCPV6_INVALID_DUID);
        }

        /* Extract the enterprise-number which should be the next 4 bytes.  */
        _nx_dhcpv6_server_utility_get_data(option_data + index, 4, &data);
        duid_ptr -> nx_duid_enterprise_number = data;
        index += 4;

        /* Check the size of identifier.  */
        if (option_length - 6 <= NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH)
        {

            /* Set the identifier.  */
            memcpy(duid_ptr -> nx_duid_private_identifier, option_data + index, option_length - 6); /* Use case of memcpy is verified. */
            index = option_length;
        }
        else
        {
            return(NX_DHCPV6_INVALID_DUID);
        }
    }

    /* Are we past the end of the buffer, subtracting for the toplevel opcode and 
       length of the IANA option? */
    if (index != option_length)
    {

        /* Yes, return the error status to reject this packet. */
        return NX_DHCPV6_INVALID_OPTION_DATA;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_check_duids_same                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function compares the two input DUIDs to match field by field. */
/*    if they match, the match status is returned NX_TRUE.                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_duid1_ptr                     Pointer to first DHCPV6 DUID   */ 
/*    dhcpv6_duid1_ptr                     Pointer to 2nd DHCPV6 DUID     */ 
/*    matching                             Flag indicating if DUIDs match */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_SUCCESS                           No errors doing comparison     */
/*    NX_PTR_ERROR                         Invalid pointer input          */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                         Get server protection mutex    */ 
/*    tx_mutex_put                         Release server protection mutex*/ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_dhcpv6_listen_for_messages         Listen for client requests    */
/*   _nx_dhcpv6_find_client_record_by_chaddr                              */ 
/*                                          Find client in server table   */
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
UINT  _nx_dhcpv6_check_duids_same(NX_DHCPV6_SVR_DUID *dhcpv6_duid1_ptr, NX_DHCPV6_SVR_DUID *dhcpv6_duid2_ptr, UINT *matching)
{


    /* Check for invalid input. */
    if (!dhcpv6_duid1_ptr || !dhcpv6_duid2_ptr || !matching)
    {

        /* Return invalid input. */
        return(NX_PTR_ERROR);
    }

    /* Initialize result to no match. */
    *matching = NX_FALSE;
   
    /* CHeck field by field starting with the ones most likely to miss. */
    if (dhcpv6_duid1_ptr -> nx_link_layer_address_msw != dhcpv6_duid2_ptr -> nx_link_layer_address_msw)
    {
        return NX_SUCCESS;
    }
    else if (dhcpv6_duid1_ptr -> nx_link_layer_address_lsw != dhcpv6_duid2_ptr -> nx_link_layer_address_lsw)
    {
        return NX_SUCCESS;
    }
    else if (dhcpv6_duid1_ptr -> nx_duid_type != dhcpv6_duid2_ptr -> nx_duid_type)
    {
        return NX_SUCCESS;
    }
    else if (dhcpv6_duid1_ptr -> nx_hardware_type != dhcpv6_duid2_ptr -> nx_hardware_type)
    {
        return NX_SUCCESS;
    }
    else if (dhcpv6_duid1_ptr -> nx_op_code != dhcpv6_duid2_ptr -> nx_op_code)
    {
        return NX_SUCCESS;
    }
    else if (dhcpv6_duid1_ptr -> nx_option_length != dhcpv6_duid2_ptr -> nx_option_length)
    {
        return NX_SUCCESS;
    }
    else if (dhcpv6_duid1_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME)
    {

       if (dhcpv6_duid1_ptr -> nx_duid_time != dhcpv6_duid2_ptr -> nx_duid_time)
       {
           return NX_SUCCESS;
       }
    }
    else if (dhcpv6_duid1_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_VENDOR_ASSIGNED)
    {

        /* First compare the enterprise ID. */
        if (dhcpv6_duid1_ptr -> nx_duid_enterprise_number != dhcpv6_duid2_ptr -> nx_duid_enterprise_number) 
        {
            return NX_SUCCESS;
        }

        /* Then compare the private ID. */
        if (memcmp(dhcpv6_duid1_ptr -> nx_duid_private_identifier, dhcpv6_duid2_ptr -> nx_duid_private_identifier, 
                   dhcpv6_duid1_ptr -> nx_option_length - sizeof(ULONG)))
        {

            /* No match. */
            return NX_SUCCESS;
        }
    }

    /* It's match! */
    *matching = NX_TRUE;

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_duid                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds a DHCPv6 DUID to the server response  packet     */
/*    based on the input DUID and the DUID type (client or server).       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                 Pointer to DHCPV6 server instance */ 
/*    dhcpv6_duid_ptr                   Pointer to DHCPV6 DUID instance   */ 
/*    buffer                            Pointer to packet buffer          */
/*    index                             Location where to add DUID        */
/*    duid_type                         Indicate client or serve DUID     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Packet buffer too small for DUID  */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Compiles and sends the Server     */
/*                                            DHCPv6 response             */ 
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
UINT  _nx_dhcpv6_add_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_SVR_DUID *dhcpv6_duid_ptr, UCHAR *buffer_ptr, UINT *index, UINT duid_type)
{

ULONG message_word;
UINT  available_payload;

    NX_PARAMETER_NOT_USED(duid_type);

    /* Clear memory to make the message header. */
    memset(&message_word, 0, sizeof(ULONG));
    
    /* Compile the header from the DUID. */
    message_word = (ULONG)(dhcpv6_duid_ptr -> nx_op_code << 16);
    message_word |= (dhcpv6_duid_ptr -> nx_option_length);

    /* Compute the available payload in the packet buffer. */
    available_payload = dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size - 
                        NX_IPv6_UDP_PACKET - *index;

    /* Check if the DUID will fit in the packet buffer. */
    if (available_payload < (UINT)(dhcpv6_duid_ptr -> nx_option_length + 4)) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy first half of the DUID option to packet buffer. */
    memcpy(buffer_ptr + *index, &message_word, sizeof(UINT)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(UINT);

    memset(&message_word, 0, sizeof(ULONG));

    /* Set  up the rest of the Client DUID header. */
    message_word = (ULONG)(dhcpv6_duid_ptr -> nx_duid_type << 16);
    message_word |= dhcpv6_duid_ptr -> nx_hardware_type;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Set up the DUID option. */
    memcpy(buffer_ptr + *index, &message_word, sizeof(UINT)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(UINT);

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_duid_ptr -> nx_duid_time);
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_duid_ptr -> nx_link_layer_address_msw);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_duid_ptr -> nx_link_layer_address_lsw);

    /* Include the 'time' field if this is a Link layer time DUID type. */
    if (dhcpv6_duid_ptr -> nx_duid_type == NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME)
    {

        memcpy(buffer_ptr + *index, &(dhcpv6_duid_ptr -> nx_duid_time), sizeof(ULONG)); /* Use case of memcpy is verified. */
        *index += (ULONG)sizeof(ULONG);
    }
   
    /* Copy the rest of the DUID header. */
    memcpy(buffer_ptr + *index, &(dhcpv6_duid_ptr -> nx_link_layer_address_msw), sizeof(USHORT)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(USHORT);

    memcpy(buffer_ptr + *index, &(dhcpv6_duid_ptr -> nx_link_layer_address_lsw), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    /* Swap bytes back to original endianness. */
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_duid_ptr -> nx_link_layer_address_lsw);
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_duid_ptr -> nx_link_layer_address_msw);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_duid_ptr -> nx_duid_time);

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_add_iana                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the IANA option to the server response  packet   */
/*    using the Client IANA on record.  It will also add any 'nested'     */
/*    options such as the Client IA(s) and IANA status option.            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                 Pointer to DHCPV6 server instance */ 
/*    dhcpv6_client_ptr                 Pointer to DHCPV6 client instance */
/*    buffer_ptr                        Pointer to response packet buffer */
/*    index                             Location into buffer to write data*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Buffer too small for IANA option  */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_add_ia                 Adds the IA option to buffer      */
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Sends server DHCPv6 response      */ 
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
UINT _nx_dhcpv6_server_add_iana(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UCHAR *buffer_ptr, UINT *index)
{

UINT            status;
UINT            add_ia;
UINT            temp;
UINT            temp_index;
ULONG           message_word;
UINT            available_payload;
NX_DHCPV6_SERVER_IA_NA *iana_ptr;


    /* Initialize the completion status variable. */
    status = NX_SUCCESS;

    /* Compute the available payload in the packet buffer. */
    available_payload = dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size - 
                        NX_IPv6_UDP_PACKET - *index;

    iana_ptr = &dhcpv6_client_ptr -> nx_dhcpv6_iana;

    /* Check if the client IANA will fit in the packet buffer. */
    if (available_payload < (UINT)(iana_ptr -> nx_option_length + 4)) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* For release or decline messages, or a server status indicating no IP address can be 
       assigned yet, set the IANA IP lease times to expired. */
    if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE) ||
        (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE) ||
        (dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code != NX_DHCPV6_STATUS_SUCCESS))
    {

        iana_ptr -> nx_T1 = 0;
        iana_ptr -> nx_T2 = 0;

        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = 0;
        dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime = 0;
    }

    /* Clear memory to make the first word of the IA-NA header. */
    memset(&message_word, 0, sizeof(ULONG));

    /* Save this location in the buffer. */
    temp_index = *index;

    /* Write the IANA opcode and data length into one word. */
    message_word = (ULONG)((iana_ptr -> nx_op_code) << 16); 

    /* Reset the IANA length to just the IANA option fields, (not IA or status options). */
    iana_ptr -> nx_option_length = 3 * sizeof(ULONG);

    message_word |= iana_ptr -> nx_option_length;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy the word into the packet buffer going to the server. */
    memcpy(buffer_ptr + *index, &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */

    /* Update the buffer pointer. */
    *index += (ULONG)sizeof(ULONG);

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(iana_ptr -> nx_IA_NA_id);
    NX_CHANGE_ULONG_ENDIAN(iana_ptr -> nx_T1);
    NX_CHANGE_ULONG_ENDIAN(iana_ptr -> nx_T2);

    memcpy(buffer_ptr + *index, &(iana_ptr -> nx_IA_NA_id), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    memcpy(buffer_ptr + *index, &(iana_ptr -> nx_T1), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    memcpy(buffer_ptr + *index, &(iana_ptr -> nx_T2), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    /* Swap bytes back. */
    NX_CHANGE_ULONG_ENDIAN(iana_ptr -> nx_IA_NA_id);
    NX_CHANGE_ULONG_ENDIAN(iana_ptr -> nx_T1);
    NX_CHANGE_ULONG_ENDIAN(iana_ptr -> nx_T2);

    /* Determine if we include an IA address option. This requires a successful IANA status
       and applies only if the client is requesting or confirming an address including renew/rebinding one. */
    add_ia = NX_FALSE;

    /* We include an IA if we intend to assign or confirm an IP address. */
    if (dhcpv6_client_ptr -> nx_dhcpv6_iana_status.nx_status_code == NX_DHCPV6_STATUS_SUCCESS)
    {
        if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type >= NX_DHCPV6_MESSAGE_TYPE_SOLICIT) &&    
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type <= NX_DHCPV6_MESSAGE_TYPE_DECLINE))
        {

                add_ia = NX_TRUE;
        }
    }

    /* Or if we are confirming the IP address a client is releasing or declining, regardless
       if our records show this IP address is bound to this client. Sect 18.2.6-7 RFC 3315. */
    if ((dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE) ||    
            (dhcpv6_client_ptr -> nx_dhcpv6_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE))
    {
        add_ia = NX_TRUE;
    }

    if (add_ia)
    {

        temp = *index;

        /* Check for a missing IA option (as in the Solicit message did not have one). */
        if (dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_op_code == 0)
        {

            NX_DHCPV6_ADDRESS_LEASE *client_address_lease; 

            dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_op_code = NX_DHCPV6_OP_IA_ADDRESS;
    
            /* Set the length to include option code, lenght (one word), IPv6 address (4 words), and lifetime data (2 words). */
            dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_option_length = (2 * sizeof(ULONG))+ (4 * sizeof(ULONG));
    
            dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_preferred_lifetime = NX_DHCPV6_DEFAULT_PREFERRED_TIME;
    
            dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_valid_lifetime= NX_DHCPV6_DEFAULT_VALID_TIME;
    
            /* Find the Client's tentatively assigned address in the Server IP address list. */
            _nx_dhcpv6_find_ip_address(dhcpv6_server_ptr, dhcpv6_client_ptr, &client_address_lease);

            if ((status != NX_SUCCESS) || (client_address_lease == NX_NULL))
            {

                return status;
            }

            COPY_IPV6_ADDRESS(&client_address_lease -> nx_dhcpv6_lease_IP_address.nxd_ip_address.v6[0], 
                              &dhcpv6_client_ptr -> nx_dhcpv6_ia.nx_global_address.nxd_ip_address.v6[0]);
        }

        status = _nx_dhcpv6_add_ia(dhcpv6_server_ptr, &dhcpv6_client_ptr -> nx_dhcpv6_ia, buffer_ptr, &temp);

        if (status != NX_SUCCESS)
        {

            return status;
        }


        /* Add the size of the IA(s) to the client IANA. Note that we include the option code
           and length field of any nested options. */
        iana_ptr -> nx_option_length = (USHORT)(iana_ptr -> nx_option_length + dhcpv6_client_ptr-> nx_dhcpv6_ia.nx_option_length + 4);

        /* Update the index into packet buffer from our temporary variable. */
        *index = temp;
    }

    /* Update the IANA status message. */
    status = _nx_dhcpv6_prepare_iana_status(&dhcpv6_client_ptr -> nx_dhcpv6_iana_status, NX_TRUE);

    if (status != NX_SUCCESS)
    {

        return status;
    }

    temp = *index;
    
    /* Ok to add one to server reply.*/
    status = _nx_dhcpv6_add_iana_status(dhcpv6_server_ptr, &dhcpv6_client_ptr -> nx_dhcpv6_iana_status, buffer_ptr, &temp);

    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Update the index into packet buffer from our temporary variable. */
    *index = temp;

    /* Update the IANA option length with the iana status option size. */
    iana_ptr -> nx_option_length = (USHORT)(iana_ptr -> nx_option_length + dhcpv6_client_ptr->nx_dhcpv6_iana_status.nx_option_length + 4);

    /* Rewrite the IANA opcode and update the changed IANA data length with the additional IA address option. */
    message_word = (ULONG)((iana_ptr -> nx_op_code) << 16);
    message_word |= (ULONG)(iana_ptr -> nx_option_length);

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy the updated word into the packet buffer at the saved location. */
    memcpy(buffer_ptr + temp_index, &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */

    /* Erase the IANA status message. We will add another message on the next (if any) IANA option for the client. */
    status = _nx_dhcpv6_prepare_iana_status(&dhcpv6_client_ptr -> nx_dhcpv6_iana_status, NX_FALSE);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_ia                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the IA address option to the server response     */
/*    packet using the input IA option.                                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                 Pointer to DHCPV6 server instance */ 
/*    dhcpv6_ia_ptr                     Pointer to IA to add              */
/*    buffer_ptr                        Pointer to response packet buffer */
/*    index                             Location into buffer to write data*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Buffer to small for IA option     */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_add_iana         Adds the IANA to server response */
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
UINT _nx_dhcpv6_add_ia(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_SERVER_IA_ADDRESS *dhcpv6_ia_ptr, UCHAR *buffer_ptr, UINT *index)
{

ULONG   message_word;
UINT    available_payload;


    /* Compute the available payload in the packet buffer. */
    available_payload = dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size - 
                        NX_IPv6_UDP_PACKET - *index;

    /* Check if the client IA address option will fit in the packet buffer. */
    if (available_payload < (UINT)(dhcpv6_ia_ptr -> nx_option_length + 4)) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* If the client has created an address option, apply it to the message. */

    /* Clear memory to make the first word of the IA-Address header. */
    memset(&message_word, 0, sizeof(ULONG));

    /* Compile the header from the IA option on the client record. */
    message_word = (ULONG)(dhcpv6_ia_ptr -> nx_op_code << 16);
    message_word |= dhcpv6_ia_ptr -> nx_option_length;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy option header into the packet and update the index into the buffer. */
    memcpy(buffer_ptr + *index, &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[0]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[1]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[2]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[3]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_preferred_lifetime);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_valid_lifetime);

    /* Copy the IPv6 address to the packet. */
    memcpy(buffer_ptr + *index, &(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[0]), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    memcpy(buffer_ptr + *index, &(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[1]), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    memcpy(buffer_ptr + *index, &(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[2]), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    memcpy(buffer_ptr + *index, &(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[3]), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    /* Copy the Client's preference for lifetimes to packet. */
    memcpy(buffer_ptr + *index, &(dhcpv6_ia_ptr -> nx_preferred_lifetime), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    memcpy(buffer_ptr + *index, &(dhcpv6_ia_ptr -> nx_valid_lifetime), sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    /* Swap bytes back. */
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[0]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[1]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[2]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_global_address.nxd_ip_address.v6[3]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_preferred_lifetime);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ia_ptr -> nx_valid_lifetime);

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_iana_status                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the IANA status option to the server response    */
/*    packet using the input IANA status option.                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_server_ptr                 Pointer to DHCPV6 server instance */ 
/*    iana_status_ptr                   Pointer to IANA status option     */
/*    buffer_ptr                        Pointer to response packet buffer */
/*    index                             Location into buffer to write data*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Buffer to small for IA option     */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_server_add_iana        Add IANA status to server response*/
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
UINT _nx_dhcpv6_add_iana_status(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_SERVER_IANA_STATUS *iana_status_ptr, UCHAR *buffer_ptr, UINT *index)
{

ULONG   message_word;
USHORT  message_short;
UINT    available_payload;
UINT    message_length;


    /* Compute the available payload in the packet buffer. */
    available_payload = dhcpv6_server_ptr -> nx_dhcpv6_packet_pool_ptr -> nx_packet_pool_payload_size - 
                        NX_IPv6_UDP_PACKET - *index;

    /* Check if the status option will fit in the packet buffer. */
    if (available_payload < (UINT)(iana_status_ptr -> nx_option_length + 4)) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Clear memory to make the message header. */
    memset(&message_word, 0, sizeof(ULONG));

    /* Compile the header for the status option. */
    message_word = (ULONG)(iana_status_ptr -> nx_op_code << 16);
    message_word |= (iana_status_ptr -> nx_option_length);

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy the word to packet buffer. */
    memcpy(buffer_ptr + *index, &message_word, sizeof(UINT)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(UINT);

    memset(&message_word, 0, sizeof(ULONG));

    /* Set up the status option code. */
    message_short = iana_status_ptr -> nx_status_code;

    /* Adjust for endianness. */
    NX_CHANGE_USHORT_ENDIAN(message_short);

    /* Set up the DUID option. */
    memcpy(buffer_ptr + *index, &message_short, sizeof(USHORT)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(USHORT);

    message_length = iana_status_ptr -> nx_option_length - (ULONG)(sizeof(USHORT));
    memcpy(buffer_ptr + *index, &iana_status_ptr -> nx_status_message[0], message_length); /* Use case of memcpy is verified. */

    *index += message_length;

    /* No need to swap the last byte. */
    return NX_SUCCESS;
}
#endif /* FEATURE_NX_IPV6 */
