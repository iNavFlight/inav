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
/** NetX Duo Component                                                    */ 
/**   Trivial File Transfer Protocol (TFTP) Server                        */
/**                                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_TFTP_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nxd_tftp_server.h" 
#ifdef FEATURE_NX_IPV6
#include    "nx_ipv6.h"
#endif

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */  
/*    _nxde_tftp_server_create                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP server create call.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server_ptr                       Pointer to TFTP server        */ 
/*    tftp_server_name                      Name of TFTP server           */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    media_ptr                             Pointer to media structure    */ 
/*    stack_ptr                             Server thread's stack pointer */ 
/*    stack_size                            Server thread's stack size    */ 
/*    pool_ptr                              Pointer to packet pool        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_TFTP_POOL_ERROR                    Invalid packet size for TFTP  */
/*                                              server packet pool        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nxd_tftp_server_create               Actual server create call     */ 
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
UINT  _nxde_tftp_server_create(NX_TFTP_SERVER *tftp_server_ptr, CHAR *tftp_server_name, NX_IP *ip_ptr, 
                                FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr)
{

UINT        status;
NX_PACKET   *packet_ptr;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||        
        (tftp_server_ptr == NX_NULL) || (tftp_server_ptr -> nx_tftp_server_id == NXD_TFTP_SERVER_ID) || 
        (stack_ptr == NX_NULL) || (pool_ptr == NX_NULL))
    {

        return(NX_PTR_ERROR);
    }

    /* Pickup a packet from the supplied packet pool.  */
    packet_ptr =  pool_ptr -> nx_packet_pool_available_list;

    /* Determine if the packet payload is large enough (must include 512 bytes plus packet headers).  */
    if ((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_data_start) < NX_TFTP_PACKET_SIZE)
    {

        return(NX_TFTP_POOL_ERROR);
    }
                                         
    /* Call actual server create function.  */
    status =  _nxd_tftp_server_create(tftp_server_ptr, tftp_server_name, ip_ptr, media_ptr, stack_ptr, stack_size, pool_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */             
/*    _nxd_tftp_server_create                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a TFTP server on the specified IP. In doing   */ 
/*    so this function creates an UDP socket for subsequent TFTP          */ 
/*    transfers and a thread for the TFTP server.                         */ 
/*                                                                        */ 
/*    If  NX_TFTP_SERVER_RETRANSMIT_ENABLE is enabled, it also creates    */
/*    a timer for retransmitting TFTP server packets up to a maximum      */
/*    number of retries before terminating the connection.                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server_ptr                       Pointer to TFTP server        */ 
/*    tftpvserver_name                      Name of TFTP server           */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    media_ptr                             Pointer to media structure    */ 
/*    stack_ptr                             Server thread's stack pointer */ 
/*    stack_size                            Server thread's stack size    */ 
/*    pool_ptr                              Pointer to packet pool        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual cmpletion status       */ 
/*    NX_SUCCESS                            Successful completion         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_bind                    Bind the TFTP server socket   */ 
/*    nx_udp_socket_create                  Create TFTP server socket     */ 
/*    nx_udp_socket_delete                  Delete TFTP server socket     */ 
/*    nx_udp_socket_unbind                  Unbind TFTP server socket     */ 
/*    tx_thread_create                      Create TFTP server thread     */ 
/*    tx_timer_create                       Create retransmit timer       */
/*    nx_udp_socket_receive_notify          Set a receive callback        */
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
UINT  _nxd_tftp_server_create(NX_TFTP_SERVER *tftp_server_ptr, CHAR *tftp_server_name, NX_IP *ip_ptr, 
                               FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr)
{

UINT        status;


    /* Clear the TFTP server structure.  */
    memset((void *) tftp_server_ptr, 0, sizeof(NX_TFTP_SERVER));

    /* Create the server's UDP socket.  */
    status =  nx_udp_socket_create(ip_ptr, &(tftp_server_ptr -> nx_tftp_server_socket), tftp_server_name, 
                          NX_TFTP_TYPE_OF_SERVICE,  NX_TFTP_FRAGMENT_OPTION, NX_TFTP_TIME_TO_LIVE, NX_TFTP_QUEUE_DEPTH);

    /* Determine if an error occurred.   */
    if (status != NX_SUCCESS)
    {

        /* Yes, return error code.  */
        return(status);
    }

    /* Register the receive function.  */
    nx_udp_socket_receive_notify(&(tftp_server_ptr -> nx_tftp_server_socket), _nx_tftp_server_data_present);

    /* Make sure the socket points to the TFTP server.  */
    tftp_server_ptr -> nx_tftp_server_socket.nx_udp_socket_reserved_ptr =  tftp_server_ptr;

    /* Now, bind the socket to a the well known TFTP UDP port number.  */
    status =  nx_udp_socket_bind(&(tftp_server_ptr -> nx_tftp_server_socket), NX_TFTP_SERVER_PORT, NX_NO_WAIT);

    /* Determine if an error occurred.  */
    if (status != NX_SUCCESS)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(tftp_server_ptr -> nx_tftp_server_socket));

        /* Yes, return error code.  */
        return(status);
    }

    /* Now create the TFTP Server thread.  */
    status =  tx_thread_create(&(tftp_server_ptr -> nx_tftp_server_thread), "TFTP Server Thread", _nx_tftp_server_thread_entry, 
                (ULONG) tftp_server_ptr, stack_ptr, stack_size, NX_TFTP_SERVER_PRIORITY, NX_TFTP_SERVER_PRIORITY, 
                NX_TFTP_SERVER_TIME_SLICE, TX_DONT_START);

    /* Determine if an error occurred creating the thread.  */
    if (status != NX_SUCCESS)
    {

        /* Unbind the UDP socket.  */
        nx_udp_socket_unbind(&(tftp_server_ptr -> nx_tftp_server_socket));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(tftp_server_ptr -> nx_tftp_server_socket));

        /* Yes, return error code.  */
        return(status);
    }

    /* Create the ThreadX event flags.  These will be used to drive the TFTP server thread.  */
    status =  tx_event_flags_create(&(tftp_server_ptr -> nx_tftp_server_event_flags), "TFTP Server Thread Events");

    /* Determine if an error occurred creating the event flags.  */
    if (status != TX_SUCCESS)
    {

        /* Unbind the UDP socket.  */
        nx_udp_socket_unbind(&(tftp_server_ptr -> nx_tftp_server_socket));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(tftp_server_ptr -> nx_tftp_server_socket));

        /* Delete the server thread.  */
        tx_thread_delete(&(tftp_server_ptr -> nx_tftp_server_thread));

        /* Error creating the server event flags.  */
        return(status);
    }

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE

    /* Create the ThreadX retransmit timeout timer.  This will be used to retransmit TFTP server
       packets if the Client has not responded or sends a duplicate (old) ACK or data packet.  */
    status =  tx_timer_create(&(tftp_server_ptr -> nx_tftp_server_timer), "TFTP Server Timer", 
                              _nx_tftp_server_timer_entry, (ULONG) tftp_server_ptr, 
                              (NX_TFTP_SERVER_TIMEOUT_PERIOD), 
                              (NX_TFTP_SERVER_TIMEOUT_PERIOD), TX_NO_ACTIVATE);

    if (status != NX_SUCCESS)
    {

        /* Unbind the UDP socket.  */
        nx_udp_socket_unbind(&(tftp_server_ptr -> nx_tftp_server_socket));

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(tftp_server_ptr -> nx_tftp_server_socket));

        /* Delete the event flags.  */
        tx_event_flags_delete(&(tftp_server_ptr -> nx_tftp_server_event_flags));

        /* Delete the server thread.  */
        tx_thread_delete(&(tftp_server_ptr -> nx_tftp_server_thread));

        /* Yes, return error code.  */
        return(status);
    }
#endif /* NX_TFTP_SERVER_RETRANSMIT_ENABLE */

    /* Save the Server name.  */
    tftp_server_ptr -> nx_tftp_server_name =  tftp_server_name;

    /* Save the IP pointer address.  */
    tftp_server_ptr -> nx_tftp_server_ip_ptr =  ip_ptr;

    /* Save the packet pool pointer.  */
    tftp_server_ptr -> nx_tftp_server_packet_pool_ptr =  pool_ptr;

    /* Save the media pointer address.  */
    tftp_server_ptr -> nx_tftp_server_media_ptr =  media_ptr;

    /* Clear the error code and error string.  */
    tftp_server_ptr -> nx_tftp_server_error_code =       0;
    tftp_server_ptr -> nx_tftp_server_error_string[0] =  NX_NULL;
                                                                       
    /* Set the server ID to indicate the TFTP server thread is ready.  */
    tftp_server_ptr -> nx_tftp_server_id =  NXD_TFTP_SERVER_ID;

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */       
/*    _nxde_tftp_server_delete                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP server delete call.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server_ptr                       Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_tftp_server_delete               Actual server delete call     */ 
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
UINT  _nxde_tftp_server_delete(NX_TFTP_SERVER *tftp_server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */    
    if ((tftp_server_ptr == NX_NULL) || (tftp_server_ptr -> nx_tftp_server_id != NXD_TFTP_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                      
    /* Call actual server delete function.  */
    status =  _nxd_tftp_server_delete(tftp_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */  
/*    _nxd_tftp_server_delete                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a previously created TFTP server on the       */ 
/*    specified IP.                                                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server_ptr                       Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion         */ 
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    fx_file_close                         File close                    */ 
/*    nx_udp_socket_delete                  Delete TFTP server socket     */ 
/*    nx_udp_socket_unbind                  Unbind TFTP server socket     */ 
/*    tx_thread_delete                      Delete TFTP server thread     */ 
/*    tx_thread_suspend                     Suspend TFTP server thread    */ 
/*    tx_thread_terminate                   Terminate TFTP server         */ 
/*                                            thread                      */ 
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
UINT  _nxd_tftp_server_delete(NX_TFTP_SERVER *tftp_server_ptr)
{

UINT                        i;
NX_TFTP_CLIENT_REQUEST      *client_request_ptr;


    /* Clear the server ID to indicate the TFTP server is no longer ready.  */
    tftp_server_ptr -> nx_tftp_server_id =  0;

    /* Unbind the UDP socket.  */
    nx_udp_socket_unbind(&(tftp_server_ptr -> nx_tftp_server_socket));

    /* Delete the UDP socket.  */
    nx_udp_socket_delete(&(tftp_server_ptr -> nx_tftp_server_socket));

    /* Suspend the TFTP server thread.  */
    tx_thread_suspend(&(tftp_server_ptr -> nx_tftp_server_thread));

    /* Terminate server thread. */
    tx_thread_terminate(&(tftp_server_ptr -> nx_tftp_server_thread));

    /* Delete the server flag group. */
    tx_event_flags_delete(&(tftp_server_ptr -> nx_tftp_server_event_flags));

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
    tx_timer_delete(&(tftp_server_ptr -> nx_tftp_server_timer));
#endif


    /* Delete server thread.  */
    tx_thread_delete(&(tftp_server_ptr -> nx_tftp_server_thread));

    /* Walk through the server structure to close any remaining open files.  */
    i =  0;

    client_request_ptr =  &(tftp_server_ptr -> nx_tftp_server_client_list[0]);

    while (i < NX_TFTP_MAX_CLIENTS)
    {

        /* Is this entry in use?  */

        /* First determine which IP address type. */
        if (client_request_ptr -> nx_tftp_client_request_ip_address.nxd_ip_version == NX_IP_VERSION_V4)
        {

#ifndef NX_DISABLE_IPV4
            /* This is an IPv4 client. Is this slot empty? */
            if ((client_request_ptr -> nx_tftp_client_request_port != 0) && 
                (client_request_ptr -> nx_tftp_client_request_ip_address.nxd_ip_address.v4) != 0)
            {

                /* No, need to close the file on this client!  */
                fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));
            }
#endif /* NX_DISABLE_IPV4 */
        }
        else if (client_request_ptr -> nx_tftp_client_request_ip_address.nxd_ip_version == NX_IP_VERSION_V6)
        {
            
#ifdef FEATURE_NX_IPV6
              /* This is an IPv6 Client. Is this slot empty? */
              if ((client_request_ptr -> nx_tftp_client_request_port != 0) && 
                  (!CHECK_UNSPECIFIED_ADDRESS(&client_request_ptr -> nx_tftp_client_request_ip_address.nxd_ip_address.v6[0])))
              {
      
                  /* No, need to close the file on this client!*/
                  fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));
              }
#endif  /* FEATURE_NX_IPV6  */            
        }

        /* Increment the pointer into the client request list.  */
        client_request_ptr++;
        i++;
    }

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */  
/*    _nxde_tftp_server_start                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP server start call.      */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server_ptr                       Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nxd_tftp_server_start                Actual server start call      */ 
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
UINT  _nxde_tftp_server_start(NX_TFTP_SERVER *tftp_server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */      
    if ((tftp_server_ptr == NX_NULL) || (tftp_server_ptr -> nx_tftp_server_id != NXD_TFTP_SERVER_ID))
        return(NX_PTR_ERROR);
                            
    /* Call actual server start function.  */
    status =  _nxd_tftp_server_start(tftp_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */  
/*    _nxd_tftp_server_start                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts a previously created TFTP server on the        */ 
/*    specified IP.                                                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server_ptr                       Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_resume                      Resume TFTP server thread     */ 
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
UINT  _nxd_tftp_server_start(NX_TFTP_SERVER *tftp_server_ptr)
{

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
    /* Activate TFTP server timer.  */
    tx_timer_activate(&(tftp_server_ptr -> nx_tftp_server_timer));
#endif

    /* Start the TFTP server thread.  */
    tx_thread_resume(&(tftp_server_ptr -> nx_tftp_server_thread));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_tftp_server_stop                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP server stop call.       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server_ptr                       Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nxd_tftp_server_stop                 Actual server start call      */ 
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
UINT  _nxde_tftp_server_stop(NX_TFTP_SERVER *tftp_server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */        
    if ((tftp_server_ptr == NX_NULL) || (tftp_server_ptr -> nx_tftp_server_id != NXD_TFTP_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                
    /* Call actual server delete function.  */
    status =  _nxd_tftp_server_stop(tftp_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_tftp_server_stop                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function stops a previously started TFTP server on the         */ 
/*    specified IP.                                                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server_ptr                       Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_suspend                     Suspend TFTP server thread    */ 
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
UINT  _nxd_tftp_server_stop(NX_TFTP_SERVER *tftp_server_ptr)
{

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
    /* Deactivate TFTP server timer.  */
    tx_timer_deactivate(&(tftp_server_ptr -> nx_tftp_server_timer));
#endif

    /* Suspend the TFTP server thread.  */
    tx_thread_suspend(&(tftp_server_ptr -> nx_tftp_server_thread));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_data_present                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is notified by NetX of receive events generated by    */
/*    TFTP Clients connecting or sending data.                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Socket receiving data         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set events for server thread  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX                                  NetX TCP socket callback      */ 
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
VOID  _nx_tftp_server_data_present(NX_UDP_SOCKET *socket_ptr)
{

NX_TFTP_SERVER   *server_ptr;

    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr = (NX_TFTP_SERVER *)(socket_ptr -> nx_udp_socket_reserved_ptr);

    /* Set the data event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_tftp_server_event_flags), NX_TFTP_SERVER_RECEIVE_EVENT, TX_OR);
}


#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_timer_entry                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function captures timer events used to determine if the Client */
/*    retransmit timer has expired.                                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ULONG                                 Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set events for server thread  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX                                  NetX connect callback         */ 
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
VOID  _nx_tftp_server_timer_entry(ULONG tftp_server_address)
{

NX_TFTP_SERVER   *server_ptr;

    /* Pickup server pointer.  */
    server_ptr =  (NX_TFTP_SERVER *) tftp_server_address;

    /* Set the data event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_tftp_server_event_flags), NX_TFTP_SERVER_TIMER_EVENT, TX_OR);

    return;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_timer_process                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks all the active TFTP client connections         */ 
/*    for their retransmission timeout. If expired, the Server retransmits*/
/*    the data or ACK up to a maximum number of retries                   */
/*    (NX_TFTP_SERVER_MAX_RETRIES) and then terminates the connection and */
/*    closes any open files. The client request is cleared and available  */
/*    for the next client request                                         */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    fx_file_close                         Close session file.           */ 
/*    fx_file_delete                        Delete the file               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_thread_entry          TFTP server task function     */ 
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
VOID  _nx_tftp_server_timer_process(NX_TFTP_SERVER *server_ptr)
{

UINT                     i;
NX_TFTP_CLIENT_REQUEST   *client_request_ptr;


    /* Now look through all the sockets.  */
    for (i = 0; i < NX_TFTP_MAX_CLIENTS; i++)
    {

        /* Set a pointer to client request structure.  */
        client_request_ptr =  &(server_ptr -> nx_tftp_server_client_list[i]);

        /* Check if this request has a retransmit timeout pending. */
        if (client_request_ptr -> nx_tftp_client_retransmit_timeout )
        {

            /* Update reatransmit timeout. */
            if (client_request_ptr -> nx_tftp_client_retransmit_timeout >= NX_TFTP_SERVER_TIMEOUT_PERIOD)
            {

                client_request_ptr -> nx_tftp_client_retransmit_timeout -= NX_TFTP_SERVER_TIMEOUT_PERIOD;
            }
            else
            {
            
                client_request_ptr -> nx_tftp_client_retransmit_timeout = 0;
            }

            /* Has the retransmit timeout expired? */
            if (client_request_ptr -> nx_tftp_client_retransmit_timeout == 0) 
            {            

                /* Yes, retransmit unless we have hit the max retry limit. */
                if (client_request_ptr -> nx_tftp_client_retransmit_retries < NX_TFTP_SERVER_MAX_RETRIES)
                {


                    /* Update the Client request retransmit timeout and number of retries. */
                    client_request_ptr -> nx_tftp_client_retransmit_timeout = NX_TFTP_SERVER_RETRANSMIT_TIMEOUT;
                    client_request_ptr -> nx_tftp_client_retransmit_retries++;

                    /* Determine which type of request this is. */
                    if (client_request_ptr -> nx_tftp_client_request_open_type == NX_TFTP_STATE_WRITE_OPEN)
                    {

                        /* Retransmit the ACK. */
                        _nx_tftp_server_send_ack(server_ptr, client_request_ptr, NX_TRUE);
                    }
                    else
                    {

                        /* Retransmit the file data. */
                        _nx_tftp_server_send_data(server_ptr, client_request_ptr, NX_TRUE);
                    }
                }
                else
                {

                    /* The session has timed out. Send error and close. */
                    _nx_tftp_server_send_error(server_ptr, &client_request_ptr -> nx_tftp_client_request_ip_address, 
                                               client_request_ptr -> nx_tftp_client_request_port, 
                                               NX_TFTP_SESSION_TIMED_OUT, "NetX TFTP Server: Session timed out");

                    /* Error, close the file and delete the client request.  */
                    fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

                    memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));
                }
            }
        }
    }
}
#endif /* NX_TFTP_SERVER_RETRANSMIT_ENABLE */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_thread_entry                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the entry of the TFTP server.  All basic           */
/*    processing is initiated by this function.                           */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_server                           Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_get                    Get the TFTP events           */ 
/*    _nx_tftp_server_process_received_data Process received packet       */ 
/*    _nx_tftp_server_timer_process         Process TFTP timer            */  
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
VOID  _nx_tftp_server_thread_entry(ULONG tftp_server)
{

NX_TFTP_SERVER          *server_ptr;
UINT                    status;
ULONG                   events;


    /* Setup the server pointer.  */
    server_ptr =  (NX_TFTP_SERVER *) tftp_server; 

    /* Loop to process TFTP Server requests.  */
    while(1)
    {

        /* Wait for TFTP events.  */
        status =  tx_event_flags_get(&(server_ptr -> nx_tftp_server_event_flags), NX_SERVER_TFTP_ANY_EVENT, 
                                     TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

        /* Check the return status.  */
        if (status)
        {

            /* If an error occurs, simply continue the loop.  */
            continue;
        }

        /* Otherwise, an event is present.  Process according to the event.  */

        /* Check for a client receive event.  */
        if (events & NX_TFTP_SERVER_RECEIVE_EVENT)
        {

            /* Call the data received handler.  */
            _nx_tftp_server_process_received_data(server_ptr);
        }

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
        /* Check for a  timer event. */
        if (events & NX_TFTP_SERVER_TIMER_EVENT)
        {
            /* Call the timer timeout handler.  */
            _nx_tftp_server_timer_process(server_ptr);
        }
#endif /* NX_TFTP_SERVER_RETRANSMIT_ENABLE*/
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_process_received_data               PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is called when the TFTP server is notified of a       */ 
/*    receive event. It parses the TFTP Client message and calls the      */
/*    appropriate handler for read and write requests, client errors, or  */
/*    ACKing data sent by the server.                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_tftp_server_ack_process           Process ACK from previous read*/ 
/*    _nx_tftp_server_data_process          Write data packet to file     */ 
/*    _nx_tftp_server_error_process         Process error packet          */ 
/*    _nx_tftp_server_open_for_read_process Open file for reading         */ 
/*    _nx_tftp_server_open_for_write_process Open for writing             */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_receive                 Receive next TFTP packet      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    nx_tftp_server_thread_entry           TFTP thread task function     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), improved */
/*                                            the logic of processing     */
/*                                            chained packet,             */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID _nx_tftp_server_process_received_data(NX_TFTP_SERVER *server_ptr)
{

UINT         status;
NX_PACKET    *packet_ptr;
UCHAR        request_code[2];
ULONG        bytes_copyed;


    /* Wait for a request on the TFTP UDP well known port 69.  */
    status =  nx_udp_socket_receive(&(server_ptr -> nx_tftp_server_socket), &packet_ptr, NX_NO_WAIT);

    /* Check the return status. */
    if (status)
    {

        /* If an error occurs, simply return to caller.  */
        return;
    }

    /* Check for valid packet length (The minimum TFTP header size is ACK packet, four bytes).  */
    if (packet_ptr -> nx_packet_length < 4)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        /* Return.  */
        return;
    }

    /* Otherwise, we have received a packet successfully.  */

    /* Pickup up the request code. First one must be zero.  */
    status = nx_packet_data_extract_offset(packet_ptr, 0, request_code, sizeof(request_code), &bytes_copyed);

    /* Check the return status. */
    if (status || (request_code[0] != 0))
    {

        nx_packet_release(packet_ptr);
        return; 
    }

    /* Process relative to the TFTP request code.  */
    switch (request_code[1])
    {

    case NX_TFTP_CODE_READ:

        /* Process an open for read request.  */
        _nx_tftp_server_open_for_read_process(server_ptr, packet_ptr);

        /* Increment the number of open for read requests.  */
        server_ptr -> nx_tftp_server_open_for_read_requests++;
        break;

    case NX_TFTP_CODE_WRITE:

        /* Process an open for write request.  */
        _nx_tftp_server_open_for_write_process(server_ptr, packet_ptr);

        /* Increment the number of open for write requests.  */
        server_ptr -> nx_tftp_server_open_for_write_requests++;
        break;

    case NX_TFTP_CODE_DATA:

        /* Process a data request.  */
        _nx_tftp_server_data_process(server_ptr, packet_ptr);

        /* Increment the number of data block write requests.  */
        server_ptr -> nx_tftp_server_data_blocks_received++;
        break;

    case NX_TFTP_CODE_ACK:

        /* Process an ack response.  */
        _nx_tftp_server_ack_process(server_ptr, packet_ptr);

        /* Increment the number of acks for previous data blocks sent.  */
        server_ptr -> nx_tftp_server_acks_received++;
        break;

    case NX_TFTP_CODE_ERROR:

        /* Process an error request.  */
        _nx_tftp_server_error_process(server_ptr, packet_ptr);

        /* Increment the number of errors received.  */
        server_ptr -> nx_tftp_server_errors_received++;
        break;

    default:

        /* Increment the number of unknown codes received.  */
        server_ptr -> nx_tftp_server_unknown_commands++;

        /* Just release the packet.  */
        nx_packet_release(packet_ptr);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_open_for_read_process               PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function opens the specified file for reading, and returns     */ 
/*    the first block of the file.                                        */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    packet_ptr                            Pointer to TFTP request packet*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    fx_directory_information_get          Get information about file    */ 
/*    fx_file_close                         Close file on EOF or error    */ 
/*    fx_file_open                          Open file for reading         */ 
/*    fx_file_read                          Read block from file          */ 
/*    _nx_tftp_server_find_client_request   Find client entry             */ 
/*    _nx_tftp_server_send_error            Send error message            */ 
/*    nx_packet_allocate                    Allocate a new packet         */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_send                    Send TFTP data packet         */ 
/*    nx_udp_source_extract                 Extract IP and port           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_thread_entry          TFTP Server thread            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            checked the format of the   */
/*                                            received packet, improved   */
/*                                            the logic of processing     */
/*                                            chained packet, fixed the   */
/*                                            issue of cleaning up the    */
/*                                            client request entry,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
void  _nx_tftp_server_open_for_read_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
NXD_ADDRESS              ip_address;
ULONG                    file_size;
ULONG                    actual_size;
UINT                     port;
UCHAR                    *buffer_ptr;
NX_TFTP_CLIENT_REQUEST   *client_request_ptr;
NX_PACKET                *new_packet = NX_NULL;
UINT                     status;
UINT                     count = 0;


    /* Extract the source IP and port numbers.  */
    nxd_udp_source_extract(packet_ptr, &ip_address, &port);

    /* First, try to find a matching exiting entry in the client request structure.  */
    client_request_ptr =  _nx_tftp_server_find_client_request(server_ptr, port, &ip_address);

    /* See if we need to find a new entry.  */
    if (client_request_ptr == NX_NULL)
    {

        /* Yes, find a free entry in the client request structure.  */
        client_request_ptr =  _nx_tftp_server_find_client_request(server_ptr, 0, NX_NULL);
    }
    else
    {

        /* This is a dupe request. Ignore it. */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Determine if there was a free entry.  */
    if (client_request_ptr == NX_NULL)
    {

        /* Increment the maximum clients errors.  */
        server_ptr -> nx_tftp_server_clients_exceeded_errors++;

        /* Send an error to the client.  */
        _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_NOT_DEFINED, "NetX TFTP Server: Too Many Clients");

        /* No more clients can be serviced, release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Packet chain isn't supported.  */
#ifndef NX_DISABLE_PACKET_CHAIN
    if (packet_ptr -> nx_packet_next)
    {
        nx_packet_release(packet_ptr);
        return;
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Setup a pointer to the file name.  */
    buffer_ptr =  (UCHAR *) (packet_ptr -> nx_packet_prepend_ptr + 2);

    /* The format of RRQ/WRQ packet should be:
        2 bytes     string    1 byte     string   1 byte
        ------------------------------------------------
        | Opcode |  Filename  |   0  |    Mode    |   0  |
        ------------------------------------------------
    */
    while (buffer_ptr < (packet_ptr -> nx_packet_append_ptr - 1))
    {
        if (*buffer_ptr == 0)
        {
            count++;
            if (count == 2)
                break;
        }

        buffer_ptr++;
    }

    /* Check if the format of the termination is correct.  */
    if ((count != 1) || *(UCHAR *)(packet_ptr -> nx_packet_append_ptr - 1))
    {
        nx_packet_release(packet_ptr);
        return;
    }

    /* Reset the pointer to the file name.  */
    buffer_ptr =  (UCHAR *) (packet_ptr -> nx_packet_prepend_ptr + 2);

    /* Pickup the file size.  */
    status =  fx_directory_information_get(server_ptr -> nx_tftp_server_media_ptr, (CHAR *) buffer_ptr, NX_NULL, &file_size, NX_NULL, NX_NULL, NX_NULL, NX_NULL, NX_NULL, NX_NULL);

    /* Check the return status.  */
    if (status != FX_SUCCESS)
    {

        /* Send an error to the client.  */
        _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_FILE_NOT_FOUND, "NetX TFTP Server: File Not Found");

        /* Unable to find the file size, release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Attempt to open the file.  */
    status =  fx_file_open(server_ptr -> nx_tftp_server_media_ptr, &(client_request_ptr ->nx_tftp_client_request_file), (CHAR *) buffer_ptr, FX_OPEN_FOR_READ);

    /* Check the return status.  */
    if (status != FX_SUCCESS)
    {

        /* Send an error to the client.  */
        _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_FILE_NOT_FOUND, "NetX TFTP Server: File Open Failed");

        /* Unable to open the file, release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* The file has been opened successfully, now try to read up to 512 bytes.  */

   /* Allocate packet for the read packet. Determine whether we are sending IP packets.   */
    if (ip_address.nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv4_UDP_PACKET, NX_WAIT_FOREVER);
    }
    else if (ip_address.nxd_ip_version == NX_IP_VERSION_V6)
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv6_UDP_PACKET, NX_WAIT_FOREVER);
    }
    else
        status = NX_TFTP_INVALID_ADDRESS_TYPE;
    /* Check for successful packet allocation.  */
    if (status != NX_SUCCESS)
    {

        /* Increment the number of server allocation errors.  */
        server_ptr -> nx_tftp_server_allocation_errors++;

        /* Unable to allocate net packet, release the original.  */
        nx_packet_release(packet_ptr);
        return;
    }

    if (4u + NX_TFTP_FILE_TRANSFER_MAX > ((ULONG)(new_packet -> nx_packet_data_end) - (ULONG)(new_packet -> nx_packet_append_ptr)))
    {
        /* Release the original packet.  */
        nx_packet_release(packet_ptr);

        nx_packet_release(new_packet);
        return;
    }

    /* Initialize the client request structure.  */
#ifndef NX_DISABLE_IPV4
    if (ip_address.nxd_ip_version == NX_IP_VERSION_V4)
    {

        client_request_ptr ->nx_tftp_client_request_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
        client_request_ptr ->nx_tftp_client_request_ip_address.nxd_ip_address.v4 = ip_address.nxd_ip_address.v4;
    }
    else
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
    if (ip_address.nxd_ip_version == NX_IP_VERSION_V6)
    {
        COPY_NXD_ADDRESS(&ip_address, &(client_request_ptr ->nx_tftp_client_request_ip_address));
    }
    else
#endif
    {
        /* Release the original packet.  */
        nx_packet_release(packet_ptr);

        return;
    }

    client_request_ptr -> nx_tftp_client_request_port =             port;
    client_request_ptr -> nx_tftp_client_request_block_number =     1;
    client_request_ptr -> nx_tftp_client_request_open_type =        NX_TFTP_STATE_OPEN;
    client_request_ptr -> nx_tftp_client_request_exact_fit =        NX_FALSE;

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE

    /* Reset the retransmission timeout and retries for the client request. */
    client_request_ptr -> nx_tftp_client_retransmit_timeout =  NX_TFTP_SERVER_RETRANSMIT_TIMEOUT;
    client_request_ptr -> nx_tftp_client_retransmit_retries =  0;
#else

    /* Clear the count of ACK retransmits from the other side. */
    client_request_ptr -> nx_tftp_client_request_retransmits = 0;
    
#endif

    client_request_ptr -> nx_tftp_client_file_size = file_size;  

    /* Read data when length of file is larger then 0. */
    if(file_size)
    {

        /* Attempt to read the requested file.  */
        status =  fx_file_read(&(client_request_ptr ->nx_tftp_client_request_file), new_packet -> nx_packet_prepend_ptr+4, NX_TFTP_FILE_TRANSFER_MAX, &actual_size);

        /* Check for successful file read.  */
        if ((status != NX_SUCCESS) || 
            ((file_size > NX_TFTP_FILE_TRANSFER_MAX) && (actual_size != NX_TFTP_FILE_TRANSFER_MAX)) || 
            ((file_size < NX_TFTP_FILE_TRANSFER_MAX) && (actual_size != file_size)))
        {

            /* Send an error to the client.  */
            _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_NOT_DEFINED, "NetX TFTP Server: File Read Error");

            /* Unable to read the file, close it and release the packet.  */
            fx_file_close(&(client_request_ptr ->nx_tftp_client_request_file));

            memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));

            nx_packet_release(packet_ptr);
            return;
        }
    }
    else
    {
    
        actual_size = 0;
    }

    /* Increment the number of total bytes sent.  */
    server_ptr -> nx_tftp_server_total_bytes_sent +=  actual_size;

    /* Setup the client request structure.  */
    client_request_ptr -> nx_tftp_client_request_remaining_bytes =  file_size - actual_size;

    /* Determine if the file size is evenly divisible by our TFTP transfer size.  */
    if (file_size % NX_TFTP_FILE_TRANSFER_MAX)
    {

        /* Not an exact fit, ensure the exact fit flag is clear.  */
        client_request_ptr -> nx_tftp_client_request_exact_fit =  NX_FALSE;
    }
    else if (file_size > 0)
    {

        /* Yes, the file size happens to be evenly divisible by the TFTP transfer size.  In this
           case, we need to send a zero-length data packet at the end of file to let the other side
           know we are at the end.  */
        client_request_ptr -> nx_tftp_client_request_exact_fit =  NX_TRUE;
    }
    
    /* Move the TFTP data code and block number into the payload before sending it to the client.  */
    buffer_ptr =  new_packet -> nx_packet_prepend_ptr;
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  NX_TFTP_CODE_DATA;
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  1;                                     /* First block number of file.  */

    /* Setup the packet pointers appropriately.  */
    new_packet -> nx_packet_length =  actual_size + 4;
    new_packet -> nx_packet_append_ptr =  new_packet -> nx_packet_prepend_ptr + new_packet -> nx_packet_length;

    /* Send the data packet out.  */
    status = nxd_udp_socket_send(&(server_ptr -> nx_tftp_server_socket), new_packet, &ip_address, port);

    /* Release packet if send fails. */
    if (status)
    {
        nx_packet_release(new_packet);
    }

    /* Release the original packet.  */
    nx_packet_release(packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_open_for_write_process              PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function opens the specified file for writing, and returns     */ 
/*    an ACK for block 0 to let the client know everything is good.       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    packet_ptr                            Pointer to TFTP request packet*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    fx_file_create                        Create file, if necessary     */ 
/*    fx_file_close                         Close file on EOF or error    */ 
/*    fx_file_open                          Open file for reading         */ 
/*    fx_file_write                         Write block to file           */ 
/*    _nx_tftp_server_find_client_request   Find client entry             */ 
/*    _nx_tftp_server_send_error            Send error message            */ 
/*    nx_packet_allocate                    Allocate a new packet         */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_send                    Send TFTP data packet         */ 
/*    nx_udp_source_extract                 Extract IP and port           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_thread_entry          TFTP Server thread task       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            checked the format of the   */
/*                                            received packet, improved   */
/*                                            the logic of processing     */
/*                                            chained packet,             */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tftp_server_open_for_write_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
NXD_ADDRESS             ip_address;
UINT                    port;
UCHAR                   *buffer_ptr;
NX_TFTP_CLIENT_REQUEST  *client_request_ptr;
NX_PACKET               *new_packet;
UINT                    status;
UINT                    count = 0;


    /* Extract the source IP and port numbers.  */
    nxd_udp_source_extract(packet_ptr, &ip_address, &port);

    /* First, try to find a matching exiting entry in the client request structure.  */
    client_request_ptr =  _nx_tftp_server_find_client_request(server_ptr, port, &ip_address);

    /* See if we need to find a new entry.  */
    if (client_request_ptr == NX_NULL)
    {

        /* Find a free entry in the client request structure.  */
        client_request_ptr =  _nx_tftp_server_find_client_request(server_ptr, 0, NX_NULL);
    }
    else
    {

        /* This is a dupe request. Ignore it. */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Determine if there was a free entry.  */
    if (client_request_ptr == NX_NULL)
    {

        /* Increment the maximum clients errors.  */
        server_ptr -> nx_tftp_server_clients_exceeded_errors++;

        /* Send an error to the client.  */
        _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_NOT_DEFINED, "NetX TFTP Server: Too Many Clients");

        /* No more clients can be serviced, release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Packet chain isn't supported.  */
#ifndef NX_DISABLE_PACKET_CHAIN
    if (packet_ptr -> nx_packet_next)
    {
        nx_packet_release(packet_ptr);
        return;
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Setup a pointer to the file name.  */
    buffer_ptr =  (UCHAR *) (packet_ptr -> nx_packet_prepend_ptr + 2);

    /* The format of RRQ/WRQ packet should be:
        2 bytes     string    1 byte     string   1 byte
        ------------------------------------------------
        | Opcode |  Filename  |   0  |    Mode    |   0  |
        ------------------------------------------------
    */
    while (buffer_ptr < (packet_ptr -> nx_packet_append_ptr - 1))
    {
        if (*buffer_ptr == 0)
        {
            count++;
            if (count == 2)
                break;
        }

        buffer_ptr++;
    }

    /* Check if the format of the termination is correct.  */
    if ((count != 1) || *(UCHAR *)(packet_ptr -> nx_packet_append_ptr - 1))
    {
        nx_packet_release(packet_ptr);
        return;
    }

    /* Reset the pointer to the file name.  */
    buffer_ptr =  (UCHAR *) (packet_ptr -> nx_packet_prepend_ptr + 2);

    /* Perform a file create. This will fail if the file is already present, which we don't care about at this point.  */
    fx_file_delete(server_ptr -> nx_tftp_server_media_ptr, (CHAR *) buffer_ptr);
    fx_file_create(server_ptr -> nx_tftp_server_media_ptr, (CHAR *) buffer_ptr);

    /* Attempt to open the file.  */
    status =  fx_file_open(server_ptr -> nx_tftp_server_media_ptr, &(client_request_ptr -> nx_tftp_client_request_file), (CHAR *) buffer_ptr, FX_OPEN_FOR_WRITE);

    /* Check for file is open errors. */
    if (status != NX_SUCCESS)
    {

        /*  This is an actual file open error. Send an error to the client.  */
        if (status == FX_ACCESS_ERROR)
            _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_ACCESS_VIOLATION, "NetX TFTP Server: File Access Error");
        else
            _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_FILE_NOT_FOUND, "NetX TFTP Server: File Open Failed");

        /* Unable to open the file, release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Now, attempt to build an ACK response to let the client know it can start writing.  */

    /* Allocate packet for the ACK packet.  Determine whether we are sending IP packets.   */
    if (ip_address.nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv4_UDP_PACKET, NX_WAIT_FOREVER);
    }
    else
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv6_UDP_PACKET, NX_WAIT_FOREVER);
    }

    /* Check for successful packet allocation.  */
    if (status != NX_SUCCESS)
    {

        /* Increment the number of server allocation errors.  */
        server_ptr -> nx_tftp_server_allocation_errors++;

        /* Close the file.  */
        fx_file_close(&(client_request_ptr ->nx_tftp_client_request_file));

        /* Unable to allocate net packet, release the original.  */
        nx_packet_release(packet_ptr);
        return;
    }

    if (4u > ((ULONG)(new_packet -> nx_packet_data_end) - (ULONG)(new_packet -> nx_packet_append_ptr)))
    {
        nx_packet_release(new_packet);

        /* Release the original packet.  */
        nx_packet_release(packet_ptr);

        return;
    }

    new_packet -> nx_packet_append_ptr = new_packet -> nx_packet_prepend_ptr;

    /* Setup the client request structure.  */
#ifndef NX_DISABLE_IPV4
    if (ip_address.nxd_ip_version == NX_IP_VERSION_V4)
    {

        client_request_ptr ->nx_tftp_client_request_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
        client_request_ptr ->nx_tftp_client_request_ip_address.nxd_ip_address.v4 = ip_address.nxd_ip_address.v4;
    }
    else
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
    if (ip_address.nxd_ip_version == NX_IP_VERSION_V6)
    {
        COPY_NXD_ADDRESS(&ip_address, &(client_request_ptr ->nx_tftp_client_request_ip_address));
    }
    else
#endif        
    {    

        /* Close the file.  */
        fx_file_close(&(client_request_ptr ->nx_tftp_client_request_file));

        /* Release the original packet.  */
        nx_packet_release(packet_ptr);

        return;
    }

    
    client_request_ptr -> nx_tftp_client_request_port =             port;
    client_request_ptr -> nx_tftp_client_request_block_number =     1;
    client_request_ptr -> nx_tftp_client_request_open_type =        NX_TFTP_STATE_WRITE_OPEN;
    client_request_ptr -> nx_tftp_client_request_remaining_bytes =  0;

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
    /* Reset retransmission timeouts and retries on current client request.  */
    client_request_ptr -> nx_tftp_client_retransmit_timeout =  NX_TFTP_SERVER_RETRANSMIT_TIMEOUT;
    client_request_ptr -> nx_tftp_client_retransmit_retries =  0;

#else

    /* Clear the count of data retransmits from the other side. */
    client_request_ptr -> nx_tftp_client_request_retransmits = 0;
#endif

    /* Create the ACK packet.  */
    buffer_ptr =  new_packet -> nx_packet_prepend_ptr;
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  NX_TFTP_CODE_ACK;
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  0;                                     /* 0, just to signal server is ready.  */

    /* Setup the packet pointers appropriately.  */
    new_packet -> nx_packet_length =  4;
    new_packet -> nx_packet_append_ptr =  new_packet -> nx_packet_prepend_ptr + 4;

    /* Send the data packet out.  */
    status = nxd_udp_socket_send(&(server_ptr -> nx_tftp_server_socket), new_packet, &ip_address, port);

    /* Release packet if send fails. */
    if (status)
    {
        nx_packet_release(new_packet);
    }

    /* Release the original packet.  */
    nx_packet_release(packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_data_process                        PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function takes the supplied data packet and writes it to the   */ 
/*    previously opened file.                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    packet_ptr                            Pointer to TFTP request packet*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    fx_file_close                         Close file on EOF or error    */ 
/*    fx_file_write                         Write block to file           */ 
/*    _nx_tftp_server_find_client_request   Find client entry             */ 
/*    _nx_tftp_server_send_error            Send error message            */ 
/*    nx_packet_allocate                    Allocate a new packet         */ 
/*    nx_packet_copy                        Copy packet                   */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_send                    Send TFTP ACK packet          */ 
/*    nx_udp_source_extract                 Extract IP and port           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_thread_entry          TFTP Server thread loop       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), improved */
/*                                            the logic of processing     */
/*                                            chained packet,             */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tftp_server_data_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{

NXD_ADDRESS             ip_address;
UINT                    port;
USHORT                  block_number;
ULONG                   bytes_copyed;
NX_TFTP_CLIENT_REQUEST  *client_request_ptr;
UINT                    status;

    /* Extract the source IP and port numbers.  */
    nxd_udp_source_extract(packet_ptr, &ip_address, &port);

    /* Find the matching entry in the client request structure.  */
    client_request_ptr =  _nx_tftp_server_find_client_request(server_ptr, port, &ip_address);


    /* Determine if there was a matching entry.  */
    if (client_request_ptr == NX_NULL)
    {

        /* Increment the unknown clients errors.  */
        server_ptr -> nx_tftp_server_unknown_clients_errors++;

        /* Send an error to the client.  */
        _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_NO_SUCH_USER, "NetX TFTP Server: Unknown connection");

        /* No more clients can be serviced, release the packet.  */
        nx_packet_release(packet_ptr);

        return;
    }

    /* Pickup the block number.  */
    status = nx_packet_data_extract_offset(packet_ptr, 2, (UCHAR *)&block_number, sizeof(block_number), &bytes_copyed);

    /* Check return status.  */
    if (status || (bytes_copyed != 2))
    {
        nx_packet_release(packet_ptr);
        return;
    }

    /* Adjust the endianness.  */
    NX_CHANGE_USHORT_ENDIAN(block_number);

    /* Determine if this block number matches the current client block number.  */
    if (client_request_ptr -> nx_tftp_client_request_block_number != block_number)
    {

        /* No, it does not. */

        /* Check if it matches our previous ACK e.g. it could be a retransmit from the other side. */
        if (client_request_ptr -> nx_tftp_client_request_block_number == (USHORT)(block_number + 1))
        {

#ifndef NX_TFTP_SERVER_RETRANSMIT_ENABLE

            /* It does. Update how many we have received. */
            client_request_ptr -> nx_tftp_client_request_retransmits++;

            /* Decide if we should close the client request. */
            if (client_request_ptr -> nx_tftp_client_request_retransmits <= NX_TFTP_MAX_CLIENT_RETRANSMITS)
            {

                /* Not yet. Just drop the packet for now. */                     
                nx_packet_release(packet_ptr);

                return;
            }
            /* Else handle as an error. */
#else
            nx_packet_release(packet_ptr);

            /* (Let the retransmit timeout handler retransmit our ACK to the client.) */
            return;
#endif
        }
         
         /* Send an error to the client.  */
         _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_ILLEGAL_OPERATION, "NetX TFTP Server: Bad block number");

        /* Error, close the file, release the packet and delete the client request.  */
        fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));

        return;
    }

    /* At this point we have a valid packet. */

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE

    /* Reset the retransmit retry counter and retransmit timeout. */
    client_request_ptr -> nx_tftp_client_retransmit_retries =  0;
    client_request_ptr -> nx_tftp_client_retransmit_timeout =  NX_TFTP_SERVER_RETRANSMIT_TIMEOUT;
#else

    /* Clear the count of retransmits from the other side. */
    client_request_ptr -> nx_tftp_client_request_retransmits = 0;
#endif

    /* Determine if there is anything to write.  */
    if (packet_ptr -> nx_packet_length > 4)
    {

        /* At this point, we need to write the next block of the file.  */

#ifndef NX_DISABLE_PACKET_CHAIN
        /* Determine if the current packet is chained.  */
        if (packet_ptr -> nx_packet_next)
        {

        NX_PACKET   *temp_ptr;


            /* Yes, the packet is chained.  We have to copy the receive packet into a packet with the 
               a payload of at least 560 bytes so the write request can be supplied with just the 
               payload pointer.  */
            status =  nx_packet_copy(packet_ptr, &temp_ptr, server_ptr -> nx_tftp_server_packet_pool_ptr, NX_WAIT_FOREVER);

            /* Check for successful packet copy.  */
            if (status != NX_SUCCESS)
            {

                /* Increment the number of server allocation errors.  */
                server_ptr -> nx_tftp_server_allocation_errors++;

                /* Unable to allocate net packet, release the original.  */
                fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

                nx_packet_release(packet_ptr);

                memset(client_request_ptr,0, sizeof(NX_TFTP_CLIENT_REQUEST));

                return;
            }

            /* Successful packet copy.  Release the original packet and reassign the packet pointer variable.  */
            nx_packet_release(packet_ptr);
            packet_ptr =  temp_ptr;
        }
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* Attempt to write the block to the file.  */
        status =  fx_file_write(&(client_request_ptr -> nx_tftp_client_request_file), packet_ptr -> nx_packet_prepend_ptr+4, 
                               packet_ptr -> nx_packet_length - 4);

        /* Check for successful file write.  */
        if (status != NX_SUCCESS)
        {

            /* Send an error to the client.  */
            _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_NOT_DEFINED, "NetX TFTP Server: File Write Error");

            /* Unable to write the file, close it and release the packet.  */
            fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

            nx_packet_release(packet_ptr);

            return;
        }
    }

    /* Check the last packet. */
    if (packet_ptr -> nx_packet_length - 4 < NX_TFTP_FILE_TRANSFER_MAX)
    {
        fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));
    }

    status = _nx_tftp_server_send_ack(server_ptr, client_request_ptr, NX_FALSE);

    if (status == NX_SUCCESS)
    {

        /* Increment the number of total bytes received.  */
        server_ptr -> nx_tftp_server_total_bytes_received +=  (packet_ptr -> nx_packet_length - 4);

        /* Determine if this was the last write.  */
        if ((packet_ptr -> nx_packet_length - 4) < NX_TFTP_FILE_TRANSFER_MAX)
        {

            /* No, nothing left to write.  Close the file, release the packet and delete 
               the client request.  */
            fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));
            memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));
        }
    }

    /* Release the original packet.  */
    nx_packet_release(packet_ptr);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_ack_process                         PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes an ACK to the previous file read            */ 
/*    operation and prepares the next data packet to send if necessary.   */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    packet_ptr                            Pointer to TFTP request packet*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    fx_file_close                         Close file on EOF or error    */ 
/*    fx_file_read                          Read block from file          */ 
/*    _nx_tftp_server_find_client_request   Find client entry             */ 
/*    _nx_tftp_server_send_error            Send error message            */ 
/*    nx_packet_allocate                    Allocate a new packet         */ 
/*    nx_packet_copy                        Copy packet                   */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_send                    Send TFTP data packet         */ 
/*    nx_udp_source_extract                 Extract IP and port           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_thread_entry          TFTP Server thread            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), improved */
/*                                            the logic of processing     */
/*                                            chained packet,             */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tftp_server_ack_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{

NXD_ADDRESS             ip_address;
UINT                    port;
USHORT                  block_number;
ULONG                   bytes_copyed;
NX_TFTP_CLIENT_REQUEST  *client_request_ptr;
UINT                    status;


    /* Extract the source IP and port numbers.  */
    nxd_udp_source_extract(packet_ptr, &ip_address, &port);

    
    /* Find a matching entry in the client request structure.  */
    client_request_ptr =  _nx_tftp_server_find_client_request(server_ptr, port, &ip_address);

    /* Determine if there was a matching entry.  */
    if (client_request_ptr == NX_NULL)
    {

        /* Increment the unknown clients errors.  */
        server_ptr -> nx_tftp_server_unknown_clients_errors++;

        /* Send an error to the client.  */
        _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_NO_SUCH_USER, "NetX TFTP Server: Unknown connection");

        /* No more clients can be serviced, release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Pickup the block number.  */
    status = nx_packet_data_extract_offset(packet_ptr, 2, (UCHAR *)&block_number, sizeof(block_number), &bytes_copyed);

    /* Check return status.  */
    if (status || (bytes_copyed != 2))
    {
        nx_packet_release(packet_ptr);
        return;
    }

    /* Adjust the endianness.  */
    NX_CHANGE_USHORT_ENDIAN(block_number);

    /* Determine if this block number matches the request.  */
    if (client_request_ptr -> nx_tftp_client_request_block_number != block_number)
    {

        /* Check if this is a retransmitted ACK e.g. our previous data packet was dropped our
           delayed. */
        if (client_request_ptr -> nx_tftp_client_request_block_number == (USHORT)(block_number + 1))
        {

#ifndef NX_TFTP_SERVER_RETRANSMIT_ENABLE

            /* It does. Update how many we have received. */
            client_request_ptr -> nx_tftp_client_request_retransmits++;

            /* Decide if we should close the client request. */
            if (client_request_ptr -> nx_tftp_client_request_retransmits <= NX_TFTP_MAX_CLIENT_RETRANSMITS)
            {

                /* Not yet. Just drop the packet for now. */                     
                nx_packet_release(packet_ptr);

                return;
            }

            /* Else handle as an error. */
#else
            nx_packet_release(packet_ptr);

            /* (Let the retransmit timeout handler retransmit our data to the client.) */
            return;
#endif
        }
        
        /* Send an error to the client.  */
        _nx_tftp_server_send_error(server_ptr, &ip_address, port, NX_TFTP_ERROR_ILLEGAL_OPERATION, "NetX TFTP Server: Bad block number");

        /* Error, close the file, release the packet and delete the client request.  */
        fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

        nx_packet_release(packet_ptr);

        memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));

        return;
    }

    /* The block number matches, see if there is anything left to send.  */
    if ((client_request_ptr -> nx_tftp_client_request_remaining_bytes == 0) && 
        (client_request_ptr -> nx_tftp_client_request_exact_fit == NX_FALSE))
    {

        /* No, nothing left to send.  Close the file, release the packet and delete 
           the client request.  */
        fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

        nx_packet_release(packet_ptr);

        memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));

        return;
    }

#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
    /* We have a valid ACK. Reset the retransmit retry counter and retransmit timeout. */
    client_request_ptr -> nx_tftp_client_retransmit_retries =  0;
    client_request_ptr -> nx_tftp_client_retransmit_timeout =  NX_TFTP_SERVER_RETRANSMIT_TIMEOUT;
#endif

    /* At this point, we need to send the next block of the file.  */
    _nx_tftp_server_send_data(server_ptr, client_request_ptr, NX_FALSE);

    /* Release the original packet.  */
    nx_packet_release(packet_ptr);

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_send_data                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a data packet based on the last ACK  received */
/*    and sends it out. This will also retransmit a data packet if        */
/*    specified. On error it will close the file and the client request.  */ 
/*    It does not update the Client request e.g. block number.            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    client_request_ptr                    Pointer to Client request     */ 
/*    retransmit                            Indicate if retransmiting a   */
/*                                            previously sent ACK         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_tftp_server_close_client_request  Terminate a client request    */ 
/*    _nx_tftp_server_send_error            Send error status to Client   */
/*    nx_packet_allocate                    Allocate a new packet         */ 
/*    nx_udp_socket_send                    Send TFTP data packet         */
/*    fx_file_seek                          Set location in file          */
/*    fx_file_read                          Read from set location in file*/
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_timer_process         TFTP timeout event            */ 
/*    _nx_tftp_server_ack_process           Process a received ACK        */
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
UINT _nx_tftp_server_send_data(NX_TFTP_SERVER *server_ptr, NX_TFTP_CLIENT_REQUEST *client_request_ptr, UINT retransmit)
{

UINT        status;
ULONG       actual_size = 0;
NX_PACKET   *new_packet;
UCHAR       *buffer_ptr;


   /* Allocate packet for the read packet. Determine whether we are sending IP packets.   */
    if (client_request_ptr -> nx_tftp_client_request_ip_address.nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv4_UDP_PACKET, NX_WAIT_FOREVER);
    }
    else
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv6_UDP_PACKET, NX_WAIT_FOREVER);
    }


    /* Check for successful packet allocation.  */
    if (status != NX_SUCCESS)
    {

        /* Increment the number of server allocation errors.  */
        server_ptr -> nx_tftp_server_allocation_errors++;

        /* Unable to allocate net packet, release the original.  */
        fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

        memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));

        return status;
    }

    if (4u + NX_TFTP_FILE_TRANSFER_MAX > ((ULONG)(new_packet -> nx_packet_data_end) - (ULONG)(new_packet -> nx_packet_append_ptr)))
    {
        nx_packet_release(new_packet);
        return(NX_SIZE_ERROR);
    }

    new_packet -> nx_packet_append_ptr = new_packet -> nx_packet_prepend_ptr;

    /* Determine if there are more bytes to read.  */
    if (client_request_ptr -> nx_tftp_client_request_remaining_bytes)
    {

        status = NX_SUCCESS;

        /* Are we retransmitting? */
        if (retransmit)
        {

            /* Yes, figure out where to reset the file pointer to the previous
               file read so we can retrieve the previous data we sent. */
            UINT index =  client_request_ptr -> nx_tftp_client_file_size - 
                                   client_request_ptr -> nx_tftp_client_request_remaining_bytes - 
                                   client_request_ptr -> nx_tftp_client_previous_write_size;

            status = fx_file_seek(&(client_request_ptr -> nx_tftp_client_request_file), index);
        }

        /* Are we still ok to do a file read? */
        if (status == NX_SUCCESS)
        {
        
            /* Yes. Attempt to read the requested file.  */
            UINT status_read =  fx_file_read(&(client_request_ptr -> nx_tftp_client_request_file), new_packet -> nx_packet_prepend_ptr+4, 
                                             NX_TFTP_FILE_TRANSFER_MAX, &actual_size);

            /* Check for successful file read.  */
            if ((status_read != NX_SUCCESS) || 
               ((client_request_ptr -> nx_tftp_client_request_remaining_bytes > NX_TFTP_FILE_TRANSFER_MAX) && (actual_size < NX_TFTP_FILE_TRANSFER_MAX)) ||
               ((client_request_ptr -> nx_tftp_client_request_remaining_bytes < NX_TFTP_FILE_TRANSFER_MAX) && (actual_size != client_request_ptr -> nx_tftp_client_request_remaining_bytes)))
            {

                /* Update our 'status' variable with the result from file read. */
                status = status_read;
            }       
        }

        /* Are we ok to transmit more data? */
        if (status != NX_SUCCESS)
        {

            /* No, send an error back to the client.  */
            _nx_tftp_server_send_error(server_ptr, &client_request_ptr -> nx_tftp_client_request_ip_address, 
                                       client_request_ptr -> nx_tftp_client_request_port, 
                                       NX_TFTP_ERROR_NOT_DEFINED, "NetX TFTP Server: File Read Error");

            memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));

            /* Unable to read the file, close it and release the packet.  */
            fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

            nx_packet_release(new_packet);

            return status;
        }
    }
    else
    {

        /* Clear the exact fit flag since the only way we can be here is if the TFTP transfer size
           evenly divided into the file size and we need to send a zero length data buffer to signal
           the end of the file.  */
        client_request_ptr -> nx_tftp_client_request_exact_fit =  NX_FALSE;

        /* Set the actual size to zero for exact fit case.  */
        actual_size =  0;
    }

    /* Increment the number of total bytes sent.  */
    server_ptr -> nx_tftp_server_total_bytes_sent +=  actual_size;

    /* Is this new data being sent (not a retransmit)?   */
    if (retransmit == NX_FALSE)
    {

        /* Yes, so advance the block number and number of bytes of the file sent. */
        client_request_ptr -> nx_tftp_client_request_block_number++;

        client_request_ptr -> nx_tftp_client_request_remaining_bytes -=  actual_size;
    }

    client_request_ptr -> nx_tftp_client_previous_write_size = actual_size;

    /* Move the TFTP data code and block number into the payload before sending it to the client.  */
    buffer_ptr =  new_packet -> nx_packet_prepend_ptr;
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  NX_TFTP_CODE_DATA;
    *buffer_ptr++ =  (UCHAR) (client_request_ptr -> nx_tftp_client_request_block_number >> 8);
    *buffer_ptr++ =  (UCHAR) (client_request_ptr -> nx_tftp_client_request_block_number & 0xFF);

    /* Setup the packet pointers appropriately.  */
    new_packet -> nx_packet_length =  actual_size + 4;
    new_packet -> nx_packet_append_ptr =  new_packet -> nx_packet_prepend_ptr + new_packet -> nx_packet_length;

    /* Send the data packet out.  */
    status = nxd_udp_socket_send(&(server_ptr -> nx_tftp_server_socket), new_packet, &client_request_ptr -> nx_tftp_client_request_ip_address, 
                                 client_request_ptr -> nx_tftp_client_request_port);

    /* Release packet if send fails. */
    if (status)
    {
        nx_packet_release(new_packet);
    }

   return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_send_ack                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates an ACK packet based on the last block of data */
/*    received, and sends it out. This will also retransmit an ACK if     */
/*    specified. On error it will close the file and the client request.  */ 
/*    It does not update the Client request e.g. block number.            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    client_request_ptr                    Pointer to Client request     */ 
/*    retransmit                            Indicate if retransmiting a   */
/*                                            previously sent ACK         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_tftp_server_close_client_request  Terminate a client request    */ 
/*    nx_packet_allocate                    Allocate a new packet         */ 
/*    nx_udp_socket_send                    Send TFTP data packet         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_timer_process         TFTP timeout event            */ 
/*    _nx_tftp_server_data_process          Process a received packet     */
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
UINT _nx_tftp_server_send_ack(NX_TFTP_SERVER *server_ptr, NX_TFTP_CLIENT_REQUEST *client_request_ptr, UINT retransmit)
{

UINT        status;
UCHAR       *buffer_ptr;
NX_PACKET   *new_packet;


   /* Allocate packet for the ACK.  Determine whether we are sending IP packets.   */
    if (client_request_ptr -> nx_tftp_client_request_ip_address.nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv4_UDP_PACKET, NX_WAIT_FOREVER);
    }
    else
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv6_UDP_PACKET, NX_WAIT_FOREVER);
    }
    
    /* Check for successful packet allocation.  */
    if (status != NX_SUCCESS)
    {

        /* Increment the number of server allocation errors.  */
        server_ptr -> nx_tftp_server_allocation_errors++;

        /* Unable to allocate net packet, release the original.  */
        fx_file_close(&(client_request_ptr -> nx_tftp_client_request_file));

        memset(client_request_ptr, 0, sizeof(NX_TFTP_CLIENT_REQUEST));

        return status;
    }

    if (4u > ((ULONG)(new_packet -> nx_packet_data_end) - (ULONG)(new_packet -> nx_packet_append_ptr)))
    {
        nx_packet_release(new_packet);
        return(NX_SIZE_ERROR);
    }

    new_packet -> nx_packet_append_ptr = new_packet -> nx_packet_prepend_ptr;

    /* Now build the ACK message to the client.  */
    buffer_ptr =  new_packet -> nx_packet_prepend_ptr;
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  NX_TFTP_CODE_ACK;

    /* If we are retransmitting, send the block number of the previous ACK. */
    if (retransmit)
    {

        *buffer_ptr++ =  (UCHAR) ((client_request_ptr -> nx_tftp_client_request_block_number - 1) >> 8);
        *buffer_ptr++ =  (UCHAR) ((client_request_ptr -> nx_tftp_client_request_block_number - 1) & 0xFF);
    }
    else
    {

        *buffer_ptr++ =  (UCHAR) ((client_request_ptr -> nx_tftp_client_request_block_number) >> 8);
        *buffer_ptr++ =  (UCHAR) (client_request_ptr -> nx_tftp_client_request_block_number & 0xFF);
    }

    /* Setup the packet pointers appropriately.  */
    new_packet -> nx_packet_length =  4;
    new_packet -> nx_packet_append_ptr =  new_packet -> nx_packet_prepend_ptr + 4;

    /* Send the ACK packet out.  */
    status = nxd_udp_socket_send(&(server_ptr -> nx_tftp_server_socket), new_packet, &client_request_ptr -> nx_tftp_client_request_ip_address, 
                                 client_request_ptr -> nx_tftp_client_request_port);

    /* Release packet if send fails. */
    if (status)
    {
        nx_packet_release(new_packet);
    }

    /* Is this ACK for new data received (e.g. not a retransmit)? */
    if (retransmit == NX_FALSE)
    {

        /* Yes, so increase the block number.  */
        client_request_ptr -> nx_tftp_client_request_block_number++;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_error_process                      PORTABLE C       */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes an error sent by a client.                  */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    packet_ptr                            Pointer to TFTP request packet*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_thread_entry          TFTP Server thread task       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), improved */
/*                                            the logic of processing     */
/*                                            chained packet,             */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tftp_server_error_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{

UINT    i;
UCHAR   *buffer_ptr;
UINT    port;
NXD_ADDRESS  ip_address;
NX_TFTP_CLIENT_REQUEST *client_request_ptr;

    /* Packet chain isn't supported.  */
#ifndef NX_DISABLE_PACKET_CHAIN
    if (packet_ptr -> nx_packet_next)
    {
        nx_packet_release(packet_ptr);
        return;
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Pickup a pointer to the error code in the buffer.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr + 2;

    /* Set the error code in the server control block.  */
    server_ptr -> nx_tftp_server_error_code =  (((UINT) (*buffer_ptr)) << 8);
    buffer_ptr++;
    server_ptr -> nx_tftp_server_error_code |= ((UINT) (*buffer_ptr) & 0xFF);
    buffer_ptr++;

    /* Loop to save error message.  */
    server_ptr -> nx_tftp_server_error_string[sizeof(server_ptr -> nx_tftp_server_error_string) - 1] =  NX_NULL;
    for (i = 0; i < NX_TFTP_ERROR_STRING_MAX; i++)
    {

        /* Store desired file name.  */
        server_ptr -> nx_tftp_server_error_string[i] =  (CHAR) *buffer_ptr++;

        /* Check for NULL character.  */
        if (server_ptr -> nx_tftp_server_error_string[i] == NX_NULL)
            break;

        /* Check for packet buffer boundary. */
        if (buffer_ptr >= packet_ptr -> nx_packet_append_ptr)
            break;
    }

    /* Extract the source IP and port numbers.  */

    nxd_udp_source_extract(packet_ptr, &ip_address, &port);

    /* First, try to find a matching existing entry in the client request structure.  */
    client_request_ptr =  _nx_tftp_server_find_client_request(server_ptr, port, &ip_address);

    /* Reset the retransmit timeout on the client request. */
    if (client_request_ptr)
    {
#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE

        client_request_ptr -> nx_tftp_client_retransmit_timeout =  NX_TFTP_SERVER_RETRANSMIT_TIMEOUT;
        client_request_ptr -> nx_tftp_client_retransmit_retries =  0;
#else

        client_request_ptr -> nx_tftp_client_request_retransmits = 0;

#endif /* NX_TFTP_SERVER_RETRANSMIT_ENABLE */
    }

    /* Release the packet.  */
    nx_packet_release(packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_find_client_request                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to find the specified IP and port number in  */ 
/*    the client request array of this TFTP server instance.              */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    port                                  Client port number            */ 
/*    ip_address                            Client IP address             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_TFTP_CLIENT_REQUEST *              NULL if not found             */ 
/*                                          Non null if match found       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_ack_process             ACK processing              */ 
/*    _nx_tftp_server_data_process            Data packet processing      */ 
/*    _nx_tftp_server_open_for_write_process  Open for write processing   */ 
/*    _nx_tftp_server_open_for_read_process   Open for read processing    */ 
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
NX_TFTP_CLIENT_REQUEST * _nx_tftp_server_find_client_request(NX_TFTP_SERVER *server_ptr, UINT port, NXD_ADDRESS *ip_address)
{

UINT                        i;
NX_TFTP_CLIENT_REQUEST    *client_request_ptr;

    /* First, find a free entry in the client request structure.  */
    i =  0;
    for (i = 0; i < NX_TFTP_MAX_CLIENTS; i++)
    {
        client_request_ptr =  &(server_ptr -> nx_tftp_server_client_list[i]);

        /* First check if we are adding an address.  If the caller sends in a blank
           address and port, we are. */
        if (port == 0)
        {

            /* We are adding an address. Now check if this slot is empty. */

            if (client_request_ptr -> nx_tftp_client_request_port == 0) 
            {

                /* If there is no port number, this is an empty slot. But let's clear the slot anyway. */
                memset(&(server_ptr -> nx_tftp_server_client_list[i]), 0, sizeof(NX_TFTP_CLIENT_REQUEST));

                break;
            }
        }
        /* This is a valid address. Does it match the current entry?  */
        else 
        {
            /* Check if it is an IPv4 or IPv6 packet. */
            if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
            {

#ifndef NX_DISABLE_IPV4
                /* This is an IPv4 request packet. */
                if ((client_request_ptr -> nx_tftp_client_request_port == port) &&
                    (client_request_ptr -> nx_tftp_client_request_ip_address.nxd_ip_address.v4 == ip_address -> nxd_ip_address.v4))
                {
        
                    /* Yes, they match! */
                    break;
                }
#else
                return NX_NULL;
#endif /* NX_DISABLE_IPV4 */
            }
            else if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
            {
              
#ifndef FEATURE_NX_IPV6
                return NX_NULL;
#else                
                /* This is an IPv6 request packet. */
                if ((client_request_ptr -> nx_tftp_client_request_port == port) &&
                    (CHECK_IPV6_ADDRESSES_SAME(&client_request_ptr -> nx_tftp_client_request_ip_address.nxd_ip_address.v6[0], &ip_address -> nxd_ip_address.v6[0])))
                {
        
                    /* Yes, they match! */
                    break;
                }
#endif                
            }
        }
    }

    /* Determine if a match was found.    */
    if (i < NX_TFTP_MAX_CLIENTS)
    {
        /* Return a pointer to the matching client request. */
        return(&(server_ptr -> nx_tftp_server_client_list[i]));
    }
    else
    {
        /* Return a NULL pointer indicating no match found. */
        return((NX_TFTP_CLIENT_REQUEST *)NX_NULL);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_server_send_error                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function send a TFTP error message to the client specified     */ 
/*    by the port and IP address.                                         */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TFTP server        */ 
/*    ip_address                            Client IP address             */ 
/*    port                                  Client port number            */ 
/*    error                                 TFTP error code               */ 
/*    error_message                         Error string                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                      Allocate error packet       */ 
/*    nx_udp_socket_send                      Send TFPT error packet      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_tftp_server_ack_process             ACK processing              */ 
/*    _nx_tftp_server_data_process            Data packet processing      */ 
/*    _nx_tftp_server_open_for_write_process  Open for write processing   */ 
/*    _nx_tftp_server_open_for_read_process   Open for read processing    */ 
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
VOID  _nx_tftp_server_send_error(NX_TFTP_SERVER *server_ptr, NXD_ADDRESS *ip_address, UINT port, UINT error, CHAR *error_message)
{

UINT        status;
UCHAR       *buffer_ptr;
NX_PACKET   *new_packet;


   /* Allocate packet for the read packet. Determine whether we are sending IP packets.   */
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv4_UDP_PACKET, NX_WAIT_FOREVER);
    }
    else
    {
        status =  nx_packet_allocate(server_ptr -> nx_tftp_server_packet_pool_ptr, &new_packet, NX_IPv6_UDP_PACKET, NX_WAIT_FOREVER);
    }

    /* Check for successful packet allocation.  */
    if (status != NX_SUCCESS)
    {

        /* Increment the number of server allocation errors.  */
        server_ptr -> nx_tftp_server_allocation_errors++;

        /* Unable to allocate packet for error message, just return!  */
        return;
    }

    if (6u > ((ULONG)(new_packet -> nx_packet_data_end) - (ULONG)(new_packet -> nx_packet_append_ptr)))
    {
        nx_packet_release(new_packet);
        return;
    }

    new_packet -> nx_packet_append_ptr = new_packet -> nx_packet_prepend_ptr;

    /* Move the TFTP error code and message into the payload before sending it to the client.  */
    buffer_ptr =  new_packet -> nx_packet_prepend_ptr;
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  NX_TFTP_CODE_ERROR;
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  (UCHAR) (error & 0xFF);

    /* Loop to copy the error message into the buffer.  */
    do
    {
    
        /* Copy a byte of the error message into the buffer.  */
        *buffer_ptr =  (UCHAR) *error_message;

        /* Determine if a NULL is present.  */
        if (*buffer_ptr == NX_NULL)
        {

            /* Yes, we are at the end of the string end the loop.  */
            break;
        }

        /* Move both pointers to the next character.  */
        buffer_ptr++;
        error_message++;
    } while (buffer_ptr < (new_packet -> nx_packet_data_end - 1));

    /* Ensure a NULL is in the last position!  */
    *buffer_ptr++ =  NX_NULL;

    /* Setup the packet pointers appropriately.  */
    new_packet -> nx_packet_length =  (ULONG)(buffer_ptr - new_packet -> nx_packet_prepend_ptr);
    new_packet -> nx_packet_append_ptr =  buffer_ptr;

    /* Send the data packet out.  */
    status = nxd_udp_socket_send(&(server_ptr -> nx_tftp_server_socket), new_packet, ip_address, port);

    /* Release packet if send fails. */
    if (status)
    {
        nx_packet_release(new_packet);
    }
}
