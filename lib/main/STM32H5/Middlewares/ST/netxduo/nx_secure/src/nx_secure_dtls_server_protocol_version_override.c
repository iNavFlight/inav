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


/* Include necessary system files.  */

#include "nx_secure_dtls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_server_protocol_version_override    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function overrides the DTLS protocol version to use for the    */
/*    DTLS server. This allows for a different version of DTLS to be      */
/*    utilized even if a newer version is enabled. For example, to use    */
/*    DTLSv1.0 for a specific host but use DTLSv1.2 for all other hosts.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_server                           Pointer to DTLS server        */
/*    protocol_version                      Version of DTLS to use        */
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
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_server_protocol_version_override(NX_SECURE_DTLS_SERVER *dtls_server,
                                                      USHORT protocol_version)
{
#ifdef NX_SECURE_ENABLE_DTLS

    dtls_server -> nx_dtls_server_protocol_version_override = protocol_version;

    /* Return completion status.  */
    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(dtls_server);
    NX_PARAMETER_NOT_USED(protocol_version);

    return(NX_NOT_SUPPORTED);
#endif
}

