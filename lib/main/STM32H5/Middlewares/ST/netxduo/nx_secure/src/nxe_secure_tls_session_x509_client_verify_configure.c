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

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_tls_session_x509_client_verify_configure PORTABLE C     */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in setting up client X509           */
/*    certificate verification for a TLS server instance.                 */
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
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            removed redundant checking, */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_secure_tls_session_x509_client_verify_configure(NX_SECURE_TLS_SESSION *tls_session, UINT certs_number, VOID *certificate_buffer, ULONG buffer_size)
{
UINT status;
UINT metadata_size;
UINT cert_buffer_size;

    /* Check for NULL pointers. */
    if(tls_session == NX_NULL) 
    {
        return(NX_PTR_ERROR);
    }

    /* Make sure the session is initialized. */
    if(tls_session -> nx_secure_tls_id != NX_SECURE_TLS_ID)
    {
        return(NX_SECURE_TLS_SESSION_UNINITIALIZED);
    }

    /* If we have a non-null buffer but a size of 0, or a non-zero size but a NULL buffer
       return error - non-null buffer needs non-zero size, null buffer must have size 0. */
    if((certificate_buffer != NX_NULL && buffer_size == 0) ||
       (certificate_buffer == NX_NULL && buffer_size != 0))
    {
        return(NX_INVALID_PARAMETERS);        
    }

    /* Allow 0 remote certificates to be allocated - indicates that certs should be
       allocated from packet buffer instead. */
    if(certificate_buffer != NX_NULL)
    {
        /* Calculate the size of the X509 control blocks needed. */
        metadata_size = sizeof(NX_SECURE_X509_CERT) * certs_number;

        /* Check that buffer is large enough. */
        if(buffer_size < metadata_size || certs_number == 0)
        {
            return(NX_INVALID_PARAMETERS);
        }

        /* Calculate the per-certificate size allocated from the buffer. */
        cert_buffer_size = (buffer_size - metadata_size) / certs_number;

        /* Check that the certificate buffer size makes sense. */
        if(cert_buffer_size < NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE)
        {
            return(NX_INVALID_PARAMETERS);
        }
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual function. */
    status = _nx_secure_tls_session_x509_client_verify_configure(tls_session, certs_number, certificate_buffer, buffer_size);

    /* Return completion status.  */
    return(status);
}

