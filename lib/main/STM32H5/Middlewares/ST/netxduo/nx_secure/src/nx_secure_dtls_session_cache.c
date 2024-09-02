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

#ifdef NX_SECURE_ENABLE_DTLS
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_dtls_session_cache_delete                 PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes DTLS session with specific IP address and     */
/*    port.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_server                           DTLS server control block     */
/*    ip_address                            IP address to match           */
/*    remote_port                           Remote port to match          */
/*    local_port                            Local port to match           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_session_reset         Reset DTLS session            */
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
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler errors when  */
/*                                            IPv4 is disabled,           */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
VOID nx_secure_dtls_session_cache_delete(NX_SECURE_DTLS_SERVER *dtls_server, NXD_ADDRESS *ip_address, UINT remote_port, UINT local_port)
{
NX_SECURE_DTLS_SESSION *session_array;
UINT num_sessions;
UINT i;

    /* Get our session cache information from the DTLS server instance. */
    num_sessions = dtls_server->nx_dtls_server_sessions_count;
    session_array = dtls_server->nx_dtls_server_sessions;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Reset all entries with matching IP address and port. */
    for (i = 0; i < num_sessions; ++i)
    {
        /* If the IP address and port match, then reset the entry. */
        if (session_array[i].nx_secure_dtls_remote_port != remote_port ||
            session_array[i].nx_secure_dtls_local_port != local_port)
        {
            continue;
        }
        if (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_version !=
            ip_address -> nxd_ip_version)
        {
            continue;
        }
#ifndef NX_DISABLE_IPV4
        if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
        {
            if (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v4 !=
                ip_address -> nxd_ip_address.v4)
            {
                continue;
            }
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
        {
            if ((session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[0] != ip_address -> nxd_ip_address.v6[0]) ||
                (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[1] != ip_address -> nxd_ip_address.v6[1]) ||
                (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[2] != ip_address -> nxd_ip_address.v6[2]) ||
                (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[3] != ip_address -> nxd_ip_address.v6[3]))
            {
                continue;
            }
        }
#endif /* FEATURE_NX_IPV6 */

        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        _nx_secure_dtls_session_reset(&session_array[i]);

        /* Get the protection. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_dtls_session_cache_get_new                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allows the DTLS implementation to associate a DTLS    */
/*    session control block with a particular IP Address and Port,        */
/*    enabling multiple DTLS sessions on a single UDP socket.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_server                           DTLS server control block     */
/*    dtls_session                          Returned DTLS session         */
/*    ip_address                            IP address                    */
/*    remote_port                           Remote port                   */
/*    local_port                            Local port                    */
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
/*    _nx_secure_dtls_receive_callback      DTLS receive callback function*/
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
UINT nx_secure_dtls_session_cache_get_new(NX_SECURE_DTLS_SERVER *dtls_server, NX_SECURE_DTLS_SESSION **dtls_session, NXD_ADDRESS *ip_address, UINT remote_port, UINT local_port)
{
NX_SECURE_DTLS_SESSION *session_array;
UINT num_sessions;
UINT i;

    /* Get our session cache information from the DTLS server instance. */
    num_sessions = dtls_server->nx_dtls_server_sessions_count;
    session_array = dtls_server->nx_dtls_server_sessions;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* See if there are any free entries. */
    for (i = 0; i < num_sessions; ++i)
    {
        /* See if there is a session available. */
        if (session_array[i].nx_secure_dtls_session_in_use == NX_FALSE)
        {
            /* Set the IP and port to the passed-in values. */
            NX_SECURE_MEMCPY(&session_array[i].nx_secure_dtls_remote_ip_address, ip_address, sizeof(NXD_ADDRESS)); /* Use case of memcpy is verified. */
            session_array[i].nx_secure_dtls_local_port = local_port;
            session_array[i].nx_secure_dtls_remote_port = remote_port;
            session_array[i].nx_secure_dtls_session_in_use = NX_TRUE;

            /* Check if ptotocol version is overrided.  */
            if (dtls_server -> nx_dtls_server_protocol_version_override)
            {
                _nx_secure_tls_session_protocol_version_override(&(session_array[i].nx_secure_dtls_tls_session), dtls_server -> nx_dtls_server_protocol_version_override);
            }

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            /* Return the session. */
            *dtls_session = &session_array[i];
            return(NX_SUCCESS);
        }
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* No session found, return NULL and an error. */
    *dtls_session = NULL;
    return(NX_SECURE_TLS_NO_FREE_DTLS_SESSIONS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_dtls_session_cache_find                   PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allows the DTLS implementation to associate a DTLS    */
/*    session control block with a particular IP Address and Port,        */
/*    enabling multiple DTLS sessions on a single UDP socket.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_server                           DTLS server control block     */
/*    dtls_session                          Returned DTLS session         */
/*    ip_address                            IP address to match           */
/*    remote_port                           Remote port to match          */
/*    local_port                            Local port to match           */
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
/*    _nx_secure_dtls_receive_callback      DTLS receive callback function*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler errors when  */
/*                                            IPv4 is disabled,           */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  nx_secure_dtls_session_cache_find(NX_SECURE_DTLS_SERVER *dtls_server, NX_SECURE_DTLS_SESSION **dtls_session, NXD_ADDRESS *ip_address, UINT remote_port, UINT local_port)
{
NX_SECURE_DTLS_SESSION *session_array;
UINT num_sessions;
UINT i;

    /* Get our session cache information from the DTLS server instance. */
    num_sessions = dtls_server->nx_dtls_server_sessions_count;
    session_array = dtls_server->nx_dtls_server_sessions;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* See if there are any matches. */
    for (i = 0; i < num_sessions; ++i)
    {
        /* Check remote port. */
        if (session_array[i].nx_secure_dtls_remote_port != remote_port)
        {
            continue;
        }

        /* Check local port. */
        if (session_array[i].nx_secure_dtls_local_port != local_port)
        {
            continue;
        }

        /* Check remote IP address version. */
        if (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_version !=
            ip_address -> nxd_ip_version)
        {
            continue;
        }

        /* Check actual remote IP address value. */
#ifndef NX_DISABLE_IPV4
        if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
        {
            if (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v4 ==
                ip_address -> nxd_ip_address.v4)
            {

                /* Release the protection. */
                tx_mutex_put(&_nx_secure_tls_protection);

                /* Return the session. */
                *dtls_session = &session_array[i];
                return(NX_SUCCESS);
            }
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
        {
            if ((session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[0] == ip_address -> nxd_ip_address.v6[0]) &&
                (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[1] == ip_address -> nxd_ip_address.v6[1]) &&
                (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[2] == ip_address -> nxd_ip_address.v6[2]) &&
                (session_array[i].nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[3] == ip_address -> nxd_ip_address.v6[3]))
            {
        
                /* Release the protection. */
                tx_mutex_put(&_nx_secure_tls_protection);

                /* Return the session. */
                *dtls_session = &session_array[i];
                return(NX_SUCCESS);
            }
        }
#endif /* FEATURE_NX_IPV6 */
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* No session found, return NULL and an error. */
    *dtls_session = NULL;
    return(NX_SECURE_DTLS_SESSION_NOT_FOUND);
}
#endif /* NX_SECURE_ENABLE_DTLS */

