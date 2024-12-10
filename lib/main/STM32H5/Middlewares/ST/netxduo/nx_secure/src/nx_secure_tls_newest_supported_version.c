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

/* We need to access the supported versions table located in nx_secure_tls_check_protocol_version.c. */
extern const NX_SECURE_VERSIONS_LIST nx_secure_supported_versions_list[];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_newest_supported_version             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Return the newest currently supported and enabled TLS/DTLS protocol */
/*    version. This enables the backward-compatible TLS/DTLS handshake    */
/*    and forces the upgrade to the latest supported TLS/DTLS version.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS session                   */
/*    protocol_version                      Pointer to version variable   */
/*    id                                    TLS or DTLS                   */
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
/*    _nx_secure_dtls_send_clienthello      Send ClientHello              */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_send_clienthello       Send ClientHello              */
/*    _nx_secure_tls_protocol_version_get   Get current TLS version to use*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID _nx_secure_tls_newest_supported_version(NX_SECURE_TLS_SESSION *session_ptr,
                                             USHORT *protocol_version, UINT id)
{
INT i;

    NX_PARAMETER_NOT_USED(session_ptr); 

#ifndef NX_SECURE_TLS_SERVER_DISABLED    
    if(session_ptr -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER &&
       session_ptr -> nx_secure_tls_protocol_version_override != 0)
    {
        /* If this is a server and the user has overriden the protocol version,
           THAT version is now the higest supported. */
        (*protocol_version) = session_ptr -> nx_secure_tls_protocol_version_override;
        return;
    }
#endif    
    /* Table is in order of oldest to newest, so walk backward to get
     * the newest version we support. */
    for (i = (INT)(nx_secure_supported_versions_list[id].nx_secure_versions_list_count - 1); i >= 0; --i)
    {
        /* If the version is supported, return it. */
        if (nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_is_supported)
        {
            (*protocol_version) = nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version;
            return;
        }
    }

    /* If we get here, no versions of TLS have been enabled. Set protocol to 0 to indicate failure. */
    (*protocol_version) = 0x0;
    return;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_highest_supported_version_negotiate     PORTABLE C   */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Negotiate the highested supported and enabled TLS/DTLS protocol     */
/*    version, if the protocol version in clientHello is not supported    */
/*    by the server.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS session                   */
/*    protocol_version                      Pointer to version variable   */
/*    id                                    TLS or DTLS                   */
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
/*    _nx_secure_dtls_process_clienthello   Send ClientHello              */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Timothy Stapko           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
void _nx_secure_tls_highest_supported_version_negotiate(NX_SECURE_TLS_SESSION *session_ptr,
                                             USHORT *protocol_version, UINT id)
{
INT i;
USHORT highest_version = 0;
USHORT lowest_version = 0xFF;
USHORT highest_version_not_greater_than_client_version = 0;

    NX_PARAMETER_NOT_USED(session_ptr);

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    if(session_ptr -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER &&
       session_ptr -> nx_secure_tls_negotiated_highest_protocol_version != 0)
    {
        /* If this is a server and the user has the negotiated highest version,
           return it directly. */
        (*protocol_version) = session_ptr -> nx_secure_tls_negotiated_highest_protocol_version;
        return;
    }
#endif

    /* Find the highest and the lowest supported server version. */

    for (i = (INT)(nx_secure_supported_versions_list[id].nx_secure_versions_list_count - 1); i >= 0; --i)
    {
        /* If the version is supported, return it. */
        if (nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_is_supported)
        {
            if (highest_version < nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version)
            {
                highest_version = nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version;
            }

            if (lowest_version > nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version)
            {
                lowest_version = nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version;
            }

            if (!highest_version_not_greater_than_client_version &&
                (*protocol_version) >= nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version)
            {
                highest_version_not_greater_than_client_version = nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version;
            }

        }
    }

    /* No versions of TLS have been enabled. Set protocol to 0 to indicate failure. */
    if (highest_version == 0)
    {
        (*protocol_version) = 0;
        session_ptr -> nx_secure_tls_negotiated_highest_protocol_version = 0;
        return;
    }

    /* According to RFC 5246 E.1,
       if protocol_version > highest_version, set protocol_version to highest_version to indicate to send "server_hello" with highest_version;
       if lowest_version < protocol_version < highest_version, set protocol_version to the highest server version not greater than protocol_version;
       if protocol_version < lowest_version, set protocol_version to 0 to indicate to send a "protocol_version" alert message.
    */

    if ((*protocol_version) > highest_version)
    {
        (*protocol_version) = highest_version;
    }
    else
    {
        if ((*protocol_version) < highest_version && (*protocol_version) > lowest_version)
        {
            (*protocol_version) = highest_version_not_greater_than_client_version;
        }
        else
        {
            (*protocol_version) = 0;
        }
    }

    session_ptr -> nx_secure_tls_negotiated_highest_protocol_version = (*protocol_version);
    return;
}



