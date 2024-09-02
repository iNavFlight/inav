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
/*    _nx_secure_tls_send_newsessionticket                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a NewSessionTicket message for a TLS Server.*/
/*    The message contains data used to generate a PSK that can be used   */
/*    for session resumption should the same client attempt another       */
/*    connection within the lifespan of the ticket.                       */
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
/*    NX_RAND                               Generate ticket data          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT _nx_secure_tls_send_newsessionticket(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet)
{
UINT             status = NX_SUCCESS;
UINT             length;
UINT             lifetime;
UINT             age_add;
UINT             nonce;
UINT             nonce_len;
UINT             ticket_len;
UCHAR            *ticket;
UINT             extensions_len;
UCHAR            *packet_buffer;

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

     NX_PARAMETER_NOT_USED(tls_session);

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < 13u)
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    length = 0;
    packet_buffer = send_packet -> nx_packet_append_ptr;

    /* First, the ticket lifetime in ms - 604800 is the maximum. */
    lifetime = 604800;
    packet_buffer[length]     = (UCHAR)((lifetime & 0xFF000000) >> 24);
    packet_buffer[length + 1] = (UCHAR)((lifetime & 0x00FF0000) >> 16);
    packet_buffer[length + 2] = (UCHAR)((lifetime & 0x0000FF00) >> 8);
    packet_buffer[length + 3] = (UCHAR) (lifetime & 0x000000FF);
    length += 4;

    /* The age_add is a cryptographically secure random 32-bit number. */
    age_add = (UINT)NX_RAND();
    packet_buffer[length]     = (UCHAR)((age_add & 0xFF000000) >> 24);
    packet_buffer[length + 1] = (UCHAR)((age_add & 0x00FF0000) >> 16);
    packet_buffer[length + 2] = (UCHAR)((age_add & 0x0000FF00) >> 8);
    packet_buffer[length + 3] = (UCHAR) (age_add & 0x000000FF);
    length += 4;


    /* Set up the nonce - 4 bytes of random data. */
    nonce = (UINT)NX_RAND();
    nonce_len = 4;

    /* Add in the nonce length. */
    packet_buffer[length] = (UCHAR)(nonce_len & 0xFF);
    length += 1;

    /*  Nonces are currently max 32 bits */
    packet_buffer[length]     = (UCHAR)((nonce & 0xFF000000) >> 24);
    packet_buffer[length + 1] = (UCHAR)((nonce & 0x00FF0000) >> 16);
    packet_buffer[length + 2] = (UCHAR)((nonce & 0x0000FF00) >> 8);
    packet_buffer[length + 3] = (UCHAR) (nonce & 0x000000FF);
    length += nonce_len;


    /* Now for the ticket ID itself. 16-bit length with the ticket being a label
       used as the PSK identity. */
    ticket =  (UCHAR *)("NewSessionTicket");
    ticket_len = sizeof("NewSessionTicket");

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < (17u + ticket_len))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Insert ticket length. */
    packet_buffer[length]     = (UCHAR)((ticket_len & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(ticket_len & 0x00FF);
    length += 2;

    /* Copy in ticket. */
    NX_SECURE_MEMCPY(&packet_buffer[length], ticket, ticket_len); /* Use case of memcpy is verified. */
    length += ticket_len;

    /* Add in extensions if available. */
    extensions_len = 0;
    packet_buffer[length]     = (UCHAR)((extensions_len & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR) (extensions_len & 0x00FF);
    length += 2;

    /* Adjust the packet into which we just wrote the finished hash. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    return(status);
}

#endif
