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
/*    _nxe_secure_dtls_server_x509_client_verify_configure PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors when configuring X.509 certificate  */
/*    authentication on a DTLS server.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*    certs_per_session                     # of certs allocated          */
/*    certs_buffer                          Buffer to use for remote certs*/
/*    buffer_size                           Size of buffer                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_server_x509_client_verify_configure                 */
/*                                          Actual function call          */
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
UINT _nxe_secure_dtls_server_x509_client_verify_configure(NX_SECURE_DTLS_SERVER *server_ptr, UINT certs_per_session,
                                                          UCHAR *certs_buffer, ULONG buffer_size)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;

    if(server_ptr == NX_NULL || certs_buffer == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if(certs_per_session == 0 || buffer_size == 0)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Call actual function. */
    status = _nx_secure_dtls_server_x509_client_verify_configure(server_ptr, certs_per_session, certs_buffer, buffer_size);

    return(status);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(certs_per_session);
    NX_PARAMETER_NOT_USED(certs_buffer);
    NX_PARAMETER_NOT_USED(buffer_size);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

