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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_end                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function ends an active TLS session by sending the TLS         */
/*    CloseNotify alert to the remote host, then waiting for the response */
/*    CloseNotify before returning.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    wait_option                           Indicates how long the caller */
/*                                          should wait for the response  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_packet_allocate        Allocate internal TLS packet  */
/*    _nx_secure_tls_send_alert             Generate the CloseNotify      */
/*    _nx_secure_tls_send_record            Send the CloseNotify          */
/*    _nx_secure_tls_session_reset          Clear out the session         */
/*    nx_secure_tls_packet_release          Release packet                */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            supported chained packet,   */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_session_end(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option)
{
UINT       status;
UINT       error_return;
UINT       send_close_notify;
NX_PACKET *send_packet;
NX_PACKET *tmp_ptr;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Release packets in queue. */
    while (tls_session -> nx_secure_record_queue_header)
    {
        tmp_ptr = tls_session -> nx_secure_record_queue_header;
        tls_session -> nx_secure_record_queue_header = tmp_ptr -> nx_packet_queue_next;
        tmp_ptr -> nx_packet_queue_next = NX_NULL;
        nx_secure_tls_packet_release(tmp_ptr);
    }
    if (tls_session -> nx_secure_record_decrypted_packet)
    {
        nx_secure_tls_packet_release(tls_session -> nx_secure_record_decrypted_packet);
        tls_session -> nx_secure_record_decrypted_packet = NX_NULL;
    }

    /* See if we want to send a CloseNotify alert, or if there was an error, don't send
       a CloseNotify, just reset the TLS session. */
    send_close_notify = 0;

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    /* Only send a CloseNotify if the handshake was finished. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        send_close_notify = tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED;
    }
#endif

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
    {
        send_close_notify = tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED;
    }
#endif

    if (send_close_notify)
    {
        /* Release the protection before suspending on nx_packet_allocate. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Allocate a packet for our close-notify alert. */
        status = _nx_secure_tls_packet_allocate(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

        /* Check for errors in allocating packet. */
        if (status != NX_SUCCESS)
        {
            /* Save the return status before resetting the TLS session. */
            error_return = status;

            /* Reset the TLS state so this socket can be reused. */
            status = _nx_secure_tls_session_reset(tls_session);


            if(status != NX_SUCCESS)
            {
                return(status);
            }

            return(error_return);
        }

        /* Get the protection after nx_packet_allocate. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        /* A close-notify alert shuts down the TLS session cleanly. */
        _nx_secure_tls_send_alert(tls_session, send_packet, NX_SECURE_TLS_ALERT_CLOSE_NOTIFY, NX_SECURE_TLS_ALERT_LEVEL_WARNING);

        /* Finally, send the alert record to the remote host. */
        status = _nx_secure_tls_send_record(tls_session, send_packet, NX_SECURE_TLS_ALERT, wait_option);

        if (status)
        {
            /* Release the packet on send errors. */
            nx_secure_tls_packet_release(send_packet);

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            /* Save the return status before resetting the TLS session. */
            error_return = status;

            /* Reset the TLS state so this socket can be reused. */
            status = _nx_secure_tls_session_reset(tls_session);


            if(status != NX_SUCCESS)
            {
                return(status);
            }

            return(error_return);
        }
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* Reset the TLS state so this socket can be reused. */
    status = _nx_secure_tls_session_reset(tls_session);

    return(status);
}

