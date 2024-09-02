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
/**    X.509 Digital Certificates                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_secure_x509.h"

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_x509_certificate_initialize             PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the X509 Initialize call.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Certificate structure         */
/*    certificate_data                      Pointer to certificate data   */
/*    length                                Length of certificate data    */
/*    raw_data_buffer                       Buffer to hold raw cert data  */
/*    buffer_size                           Size of raw data buffer       */
/*    private_key                           Pointer to private key data   */
/*    priv_len                              Length of private key data    */
/*    private_key_type                      Type of private key data      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_initialize                              */
/*                                          Actual X509 certificate       */
/*                                            initialize call             */
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
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            removed dependency on TLS,  */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nxe_secure_x509_certificate_initialize(NX_SECURE_X509_CERT *certificate, UCHAR *certificate_data,
                                             USHORT length, UCHAR *raw_data_buffer,
                                             USHORT buffer_size, const UCHAR *private_key,
                                             USHORT priv_len, UINT private_key_type)
{
UINT status;

    if ((certificate == NX_CRYPTO_NULL) || (certificate_data == NX_CRYPTO_NULL) || (length == 0))
    {
#ifdef NX_CRYPTO_STANDALONE_ENABLE
        return(NX_CRYPTO_PTR_ERROR);
#else
        return(NX_PTR_ERROR);
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status = _nx_secure_x509_certificate_initialize(certificate, certificate_data, length, raw_data_buffer, buffer_size, private_key, priv_len, private_key_type);

    /* Return completion status.  */
    return(status);
}

