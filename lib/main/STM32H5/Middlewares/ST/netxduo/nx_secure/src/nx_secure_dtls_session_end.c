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
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_session_end                         PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function ends an active DTLS session by sending the DTLS       */
/*    CloseNotify alert to the remote host, then waiting for the response */
/*    CloseNotify before returning.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS session control block    */
/*    wait_option                           Indicates how long the caller */
/*                                          should wait for the response  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_packet_allocate       Allocate internal DTLS packet */
/*    _nx_secure_tls_send_alert             Generate the CloseNotify      */
/*    _nx_secure_dtls_send_record           Send the CloseNotify          */
/*    _nx_secure_dtls_session_reset         Clear out the session         */
/*    _nx_secure_dtls_session_receive       Receive DTLS data             */
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
/*                                            released packet securely,   */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            fixed out-of-order handling,*/
/*                                            resulting in version 6.1.10 */   
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_dtls_session_end(NX_SECURE_DTLS_SESSION *dtls_session, UINT wait_option)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT                    status;
UINT                    error_status;
NX_PACKET              *send_packet;
NX_PACKET              *incoming_packet;
NX_PACKET              *tmp_ptr;
NX_SECURE_TLS_SESSION  *tls_session;

    /* Get reference to internal TLS state. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

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

    /* If the remote session is already finished, don't try to send. */
    if(!dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_remote_session_active)
    {
        /* Reset the TLS state so this socket can be reused. */        
        tx_mutex_put(&_nx_secure_tls_protection);
        status = _nx_secure_dtls_session_reset(dtls_session);
        return(status);
    }


    /* Release the protection before suspending on nx_packet_allocate. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* Allocate a packet for our close-notify alert. */
    status = _nx_secure_dtls_packet_allocate(dtls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

    /* Check for errors in allocating packet. */
    if (status != NX_SUCCESS)
    {
        _nx_secure_dtls_session_reset(dtls_session);
        return(status);
    }

    /* Get the protection after nx_packet_allocate. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* A close-notify alert shuts down the TLS session cleanly. */
    _nx_secure_tls_send_alert(tls_session, send_packet, NX_SECURE_TLS_ALERT_CLOSE_NOTIFY, NX_SECURE_TLS_ALERT_LEVEL_WARNING);

    /* Finally, send the alert record to the remote host. */
    status = _nx_secure_dtls_send_record(dtls_session, send_packet, NX_SECURE_TLS_ALERT, wait_option);

    if (status)
    {
        /* Failed to send, release the packet. */
        nx_secure_tls_packet_release(send_packet);
        _nx_secure_dtls_session_reset(dtls_session);
        tx_mutex_put(&_nx_secure_tls_protection);
        return(status);
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* See if we recevied the CloseNotify, or if we need to wait. */
    if(tls_session -> nx_secure_tls_received_alert_level != NX_SECURE_TLS_ALERT_LEVEL_WARNING &&
       tls_session -> nx_secure_tls_received_alert_value != NX_SECURE_TLS_ALERT_CLOSE_NOTIFY)
    {
        while (status != NX_SECURE_TLS_ALERT_RECEIVED)
        {
            /* Wait for the CloseNotify response. */
            /* Get the protection after nx_packet_allocate. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            status = _nx_secure_dtls_session_receive(dtls_session, &incoming_packet, wait_option);

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            if (status == NX_SECURE_TLS_CLOSE_NOTIFY_RECEIVED)
            {
                status = NX_SUCCESS;
                break;
            }
            /* Release the alert packet. */
            nx_secure_tls_packet_release(incoming_packet);
        }
    }

    /* Save error status for return below. */
    error_status = status;

    /* Reset the TLS state so this socket can be reused. */
    status = _nx_secure_dtls_session_reset(dtls_session);

    if(error_status != NX_SECURE_TLS_ALERT_RECEIVED && error_status != NX_SECURE_TLS_CLOSE_NOTIFY_RECEIVED)
    {
        status = error_status;
    }

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

