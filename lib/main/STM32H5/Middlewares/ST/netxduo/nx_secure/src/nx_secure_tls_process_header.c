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
/*    _nx_secure_tls_process_header                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an NX_PACKET data structure, extracting     */
/*    and parsing a TLS header received from a remote host.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS control block  */
/*    packet_ptr                            Pointer to incoming packet    */
/*    record_offset                         Offset of current record      */
/*    message_type                          Return message type value     */
/*    length                                Return message length value   */
/*    header_data                           Pointer to header to parse    */
/*    header_length                         Length of header data (bytes) */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_extract_offset         Extract data from NX_PACKET   */
/*    _nx_secure_tls_check_protocol_version Check incoming TLS version    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_record         Process TLS record            */
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
UINT _nx_secure_tls_process_header(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *packet_ptr,
                                   ULONG record_offset, USHORT *message_type, UINT *length,
                                   UCHAR *header_data, USHORT *header_length)
{
ULONG  bytes_copied;
UINT   status;
USHORT protocol_version;


    /* Check the packet. */
    if (packet_ptr == NX_NULL)
    {

        /* There was an error in extracting the header from the supplied packet. */
        return(NX_SECURE_TLS_INVALID_PACKET);
    }

    /* Process the TLS record header, which will set the state. */
    status = nx_packet_data_extract_offset(packet_ptr, record_offset, header_data,
                                           NX_SECURE_TLS_RECORD_HEADER_SIZE, &bytes_copied);

    /* Make sure we actually got a header. */
    if (status != NX_SUCCESS)
    {

        /* There was an error in extracting the header from the supplied packet. */
        return(NX_SECURE_TLS_INVALID_PACKET);
    }

    if (bytes_copied != NX_SECURE_TLS_RECORD_HEADER_SIZE)
    {

        /* Wait more TCP packets for this one record. */
        return(NX_CONTINUE);
    }

    /* Extract message type from packet/record. */
    *message_type = header_data[0];

    /* Extract the protocol version. */
    protocol_version = (USHORT)(((USHORT)header_data[1] << 8) | header_data[2]);

    /* Get the length of the TLS data. */
    *length = (UINT)(((UINT)header_data[3] << 8) + header_data[4]);

    /* Set header length. */
    *header_length = NX_SECURE_TLS_RECORD_HEADER_SIZE;

    /* Check the protocol version, except when we haven't established a version yet */
    if (tls_session -> nx_secure_tls_protocol_version != 0)
    {
        /* Check the record's protocol version against the current session. */
        status = _nx_secure_tls_check_protocol_version(tls_session, protocol_version, NX_SECURE_TLS);
        return(status);
    }

    return(NX_SUCCESS);
}

