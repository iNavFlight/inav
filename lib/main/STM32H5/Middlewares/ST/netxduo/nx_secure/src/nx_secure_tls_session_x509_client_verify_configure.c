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
/*    _nx_secure_tls_session_x509_client_verify_configure PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables Client Certificate Verification for TLS       */
/*    Server instances and accepts buffer space to hold incoming          */
/*    certificates sent by the remote host. If enabled, the TLS Server    */
/*    will request and verify a remote TLS Client Certificate using all   */
/*    available crypto signature routines. The provided buffer must have  */
/*    enough space allocated for the maximum size of a certificate that   */
/*    may be provided by a client times the expected size of the          */
/*    certificate chain that may be provided. The size needed can be      */
/*    calculated using the following formula:                             */
/*                                                                        */
/*    size = (<# of certs>) * (sizeof(NX_SECURE_X509_CERT) +              */
/*                            <expected max cert size (~2KB)>)            */
/*                                                                        */
/*    The space will be divided equally amongst the number of certificates*/
/*    that can be carved from the provided buffer.                        */
/*                                                                        */
/*    The incoming certificate chain will be verified against the trusted */
/*    certificate store built using nx_secure_tls_trusted_certificate_add.*/
/*    Client X509 certificate verification in TLS Server proceeds in the  */
/*    same manner as the default TLS Client behavior in verifying server  */
/*    certificates.                                                       */
/*                                                                        */
/*    Note that this will only work for TLS Server sessions. Enabling     */
/*    Client Certificate Verification for TLS Client sessions will have   */
/*    no effect.                                                          */
/*                                                                        */
/*    As of 5.12, the certificate buffer may be set to NX_NULL to         */
/*    indicate that internal certificate buffering should be used. If the */
/*    certificate_buffer parameter is NX_NULL, the buffer_size parameter  */
/*    should be set to 0.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    certs_number                          Number of client certs        */
/*    certificate_buffer                    Buffer allocated for certs    */
/*    buffer_size                           Buffer size in bytes          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_buffer_allocate                   */
/*                                          Allocate certificate buffers  */
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
UINT  _nx_secure_tls_session_x509_client_verify_configure(NX_SECURE_TLS_SESSION *tls_session, UINT certs_number, VOID *certificate_buffer, ULONG buffer_size)
{
UINT status = NX_SUCCESS;

    /* Signal the TLS stack to request and verify remote Client certificates. */
    tls_session -> nx_secure_tls_verify_client_certificate = NX_TRUE;

    /* Allocate the certificate space. If buffer is NULL, then use internal in-place certificate buffering. */
    if(certificate_buffer != NX_NULL && buffer_size != 0)
    {
        status = _nx_secure_tls_remote_certificate_buffer_allocate(tls_session, certs_number, certificate_buffer, buffer_size);
    }

    return(status);
}

