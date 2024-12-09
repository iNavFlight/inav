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
/*    _nx_secure_dtls_session_send                        PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends data using an active DTLS session, handling     */
/*    all encryption and hashing before sending data over the UDP socket. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    packet_ptr                            Pointer to packet data        */
/*    ip_address                            Remote IP address             */
/*    port                                  Remote port                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_send_record           Send DTLS encrypted record    */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    nx_secure_dtls_server_session_send    Server session send packet    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler errors when  */
/*                                            IPv4 is disabled,           */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_session_send(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr,
                                  NXD_ADDRESS *ip_address, UINT port)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Check that the passed-in DTLS session matches the ip_address and port passed in by the caller. */
    if (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_version == 0)
    {

        /* If the IP Address and port are uninitialized, set them now (possibly an error?). */
        NX_SECURE_MEMCPY(&dtls_session -> nx_secure_dtls_remote_ip_address, ip_address, sizeof(NXD_ADDRESS)); /* Use case of memcpy is verified. */
        dtls_session -> nx_secure_dtls_local_port = port;
    }
    else if ((dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_version != ip_address -> nxd_ip_version) ||
             (dtls_session -> nx_secure_dtls_remote_port != port))
    {

        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* IP address and port don't match - probably caller error. */
        return(NX_SECURE_TLS_SEND_ADDRESS_MISMATCH);
    }
    else
    {
#ifndef NX_DISABLE_IPV4
        if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
        {
            if (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v4 != ip_address -> nxd_ip_address.v4)
            {

                /* Release the protection. */
                tx_mutex_put(&_nx_secure_tls_protection);

                /* IP address and port don't match - probably caller error. */
                return(NX_SECURE_TLS_SEND_ADDRESS_MISMATCH);
            }
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
        {
            if ((dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[0] != ip_address -> nxd_ip_address.v6[0]) ||
                (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[1] != ip_address -> nxd_ip_address.v6[1]) ||
                (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[2] != ip_address -> nxd_ip_address.v6[2]) ||
                (dtls_session -> nx_secure_dtls_remote_ip_address.nxd_ip_address.v6[3] != ip_address -> nxd_ip_address.v6[3]))
            {

                /* Release the protection. */
                tx_mutex_put(&_nx_secure_tls_protection);

                /* IP address and port don't match - probably caller error. */
                return(NX_SECURE_TLS_SEND_ADDRESS_MISMATCH);
            }
        }
#endif /* FEATURE_NX_IPV6 */
    }

    status = _nx_secure_dtls_send_record(dtls_session, packet_ptr, NX_SECURE_TLS_APPLICATION_DATA, NX_WAIT_FOREVER);

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(packet_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(port);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

