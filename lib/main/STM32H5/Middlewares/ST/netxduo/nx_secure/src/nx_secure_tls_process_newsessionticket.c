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
/*    _nx_secure_tls_process_newsessionticket             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes a NewSessionTicket message from a TLS       */
/*    Server. The message contains data used to generate a PSK that can   */
/*    be used for session resumption should the same client attempt       */
/*    another connection within the lifespan of the ticket.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet used to send message   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    N/A                                                                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT _nx_secure_tls_process_newsessionticket(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                             UINT message_length)
{
UINT             status = NX_SUCCESS;
UINT             lifetime;
UINT             nonce;
UINT             nonce_len;
UINT             ticket_len;
UCHAR            *ticket;
UINT             psk_count;
NX_SECURE_TLS_PSK_STORE *ticket_psk;

    /* From RFC 8446:
       struct {
          uint32 ticket_lifetime;
          uint32 ticket_age_add;
          opaque ticket_nonce<0..255>;
          opaque ticket<1..2^16-1>;
          Extension extensions<0..2^16-2>;
       } NewSessionTicket;

       |    4     |    4    |     1     | <nonce_len> |     2      | <ticket_len> |       2        | <extensions_len> |
       | lifetime | age add | nonce_len |    nonce    | ticket_len |    ticket    | extensions_len |    extensions    |

       ticket_lifetime:  Indicates the lifetime in seconds as a 32-bit
          unsigned integer in network byte order from the time of ticket
          issuance.  Servers MUST NOT use any value greater than
          604800 seconds (7 days).  The value of zero indicates that the
          ticket should be discarded immediately.  Clients MUST NOT cache
          tickets for longer than 7 days, regardless of the ticket_lifetime,
          and MAY delete tickets earlier based on local policy.  A server
          MAY treat a ticket as valid for a shorter period of time than what
          is stated in the ticket_lifetime.

       ticket_age_add:  A securely generated, random 32-bit value that is
          used to obscure the age of the ticket that the client includes in
          the "pre_shared_key" extension.  The client-side ticket age is
          added to this value modulo 2^32 to obtain the value that is
          transmitted by the client.  The server MUST generate a fresh value
          for each ticket it sends.

       ticket_nonce:  A per-ticket value that is unique across all tickets
          issued on this connection.

       ticket:  The value of the ticket to be used as the PSK identity.  The
          ticket itself is an opaque label.  It MAY be either a database
          lookup key or a self-encrypted and self-authenticated value.

       extensions:  A set of extension values for the ticket.  The
          "Extension" format is defined in Section 4.2.  Clients MUST ignore
          unrecognized extensions.

       The sole extension currently defined for NewSessionTicket is
       "early_data", indicating that the ticket may be used to send 0-RTT
       data (Section 4.2.10).  It contains the following value:

       max_early_data_size:  The maximum amount of 0-RTT data that the
          client is allowed to send when using this ticket, in bytes.  Only
          Application Data payload (i.e., plaintext but not padding or the
          inner content type byte) is counted.  A server receiving more than
          max_early_data_size bytes of 0-RTT data SHOULD terminate the
          connection with an "unexpected_message" alert.  Note that servers
          that reject early data due to lack of cryptographic material will
          be unable to differentiate padding from content, so clients
          SHOULD NOT depend on being able to send large quantities of
          padding in early data records.

       The PSK associated with the ticket is computed as:

           HKDF-Expand-Label(resumption_master_secret,
                            "resumption", ticket_nonce, Hash.length)


    */

    /* Get a PSK entry into which we can copy the ticket data we received. */
    psk_count = tls_session->nx_secure_tls_credentials.nx_secure_tls_psk_count;
    psk_count++;
    if(psk_count >= NX_SECURE_TLS_MAX_PSK_KEYS)
    {
        psk_count = 0;
    }
    ticket_psk = &(tls_session->nx_secure_tls_credentials.nx_secure_tls_psk_store[psk_count]);


    /* First, the ticket lifetime in ms - 604800 is the maximum. */
    lifetime = (UINT)((packet_buffer[0] << 24) + (packet_buffer[1] << 16) + (packet_buffer[2] << 8) + packet_buffer[3]);
    packet_buffer = &packet_buffer[4];

    if(lifetime > NX_SECURE_TLS_MAX_SESSION_TICKET_AGE)
    {
        return(NX_SECURE_TLS_INVALID_SESSION_TICKET);
    }

    /* Save the lifetime of the ticket. */
    ticket_psk->nx_secure_tls_psk_ticket_lifetime = lifetime;

    /* Skip the age add value. */
    packet_buffer = &packet_buffer[4];

    /* Get the nonce length. */
    nonce_len = packet_buffer[0];
    packet_buffer = &packet_buffer[1];

    /* Get the nonce. */
    if(nonce_len > message_length || nonce_len > NX_SECURE_TLS_MAX_PSK_NONCE_SIZE)
    {
        return(NX_SECURE_TLS_INVALID_SESSION_TICKET);
    }
    nonce = (UINT)((packet_buffer[0] << 24) + (packet_buffer[1] << 16) + (packet_buffer[2] << 8) + packet_buffer[3]);;
    packet_buffer = &packet_buffer[nonce_len];

    /* Now for the ticket ID itself. 16-bit length with the ticket being a label
       used as the PSK identity. */
    ticket_len = (USHORT)((packet_buffer[0] << 8) + packet_buffer[1]);
    packet_buffer = &packet_buffer[2];
    ticket =  (UCHAR *)(&packet_buffer[0]);

    if(ticket_len > message_length || ticket_len > NX_SECURE_TLS_MAX_PSK_ID_SIZE)
    {
        return(NX_SECURE_TLS_INVALID_SESSION_TICKET);
    }
    packet_buffer = &packet_buffer[ticket_len];

    /* Copy ticket to PSK store - the ticket is the PSK ID used to identify the PSK in the future. */
    NX_SECURE_MEMCPY(ticket_psk->nx_secure_tls_psk_id, ticket, ticket_len); /* Use case of memcpy is verified. */
    ticket_psk->nx_secure_tls_psk_id_size = ticket_len;

    /* We can now generate the PSK for this session using our ticket nonce and the cryptographic
       secrets we created earlier in the handshake. */
    status = _nx_secure_tls_1_3_session_psk_generate(tls_session, ticket_psk, (UCHAR *)nonce, nonce_len);

    return(status);
}

#endif
