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


/* Include necessary system files.  */

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_protocol_version_override    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function overrides the TLS protocol version to use for the TLS */
/*    session. This allows for a different version of TLS to be utilized  */
/*    even if a newer version is enabled. For example, to use TLSv1.0 for */
/*    a specific host but use TLSv1.2 for all other hosts.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    protocol_version                      Version of TLS to use         */
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
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_session_protocol_version_override(NX_SECURE_TLS_SESSION *tls_session,
                                                       USHORT protocol_version)
{
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_3)
    {
        if (tls_session -> nx_secure_tls_1_3_supported)
        {

            /* Set legacy version to TLS 1.2. */
            tls_session -> nx_secure_tls_protocol_version_override = NX_SECURE_TLS_VERSION_TLS_1_2;
        }
        else
        {
            return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
        }
    }
    else
#endif
    {
        tls_session -> nx_secure_tls_protocol_version_override = protocol_version;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        tls_session -> nx_secure_tls_1_3 = NX_FALSE;
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
        tls_session -> nx_secure_tls_renegotation_enabled = NX_TRUE;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
#endif
    }

    /* Return completion status.  */
    return(NX_SUCCESS);
}

