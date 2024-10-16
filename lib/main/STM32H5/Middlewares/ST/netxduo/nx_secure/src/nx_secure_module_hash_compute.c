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

#define NX_SECURE_SOURCE_CODE
#include "nx_secure_tls_api.h"
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_module_hash_compute                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function uses user-supplied HMAC-SHA256 function (in the       */
/*    proper NX_CRYPTO_METHOD structure) to compute the hash value of     */
/*    the program memory marked by the symbols EL_SECURE_PROGRAM_BEGIN    */
/*    and EL_SECURE_PROGRAM_END.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_sha2_ptr                         Pointer to NX_CRYPTO_METHOD   */
/*                                            structure that contains     */
/*                                            HMAC-SHA256                 */
/*    key                                   User-specified key for        */
/*                                            computing the hash          */
/*    key_length                            Size of the key, in bytes     */
/*    output_buffer                         Output buffer space for       */
/*                                            storing the computed HMAC   */
/*    output_buffer_size                    Size of the output buffer     */
/*    actual_size                           Size of the HMAC message, in  */
/*                                            bytes                       */
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
UINT nx_secure_module_hash_compute(NX_CRYPTO_METHOD *hmac_ptr,
                                   UINT start_address,
                                   UINT end_address,
                                   UCHAR *key, UINT key_length,
                                   VOID *metadata, UINT metadata_size,
                                   UCHAR *output_buffer, UINT output_buffer_size, UINT *actual_size)
{
#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK
VOID *handler = NX_NULL;
UINT status;

    if(output_buffer_size < 32)
        return(1);

    /* Validate the crypto table. */
    if(hmac_ptr == NX_NULL)
        return(1);

    if(hmac_ptr -> nx_crypto_algorithm != NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256)
        return(1);

    if (hmac_ptr -> nx_crypto_init)
    {
        status = hmac_ptr -> nx_crypto_init(hmac_ptr,
                                               key,
                                               (key_length << 3),
                                               &handler,
                                               metadata,
                                               metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(1);
        }
    }

    if (hmac_ptr -> nx_crypto_operation == NX_NULL)
    {
        return(1);
    }

    /* Now compute the hash */
    status = hmac_ptr -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                                handler,     /* handle, not used */
                                                hmac_ptr, /* Method, not used */
                                                key,
                                                (key_length << 3),
                                                (UCHAR*)start_address, /* Data start */
                                                end_address - start_address, /* Data Length */
                                                NX_NULL, /* iv_ptr, not used */
                                                output_buffer,
                                                output_buffer_size,
                                                metadata,
                                                metadata_size,
                                                NX_NULL, /* packet_ptr, not used. */
                                                NX_NULL);/* HW process callback, not used. */

    if (status)
    {
        return(1);
    }

    if (hmac_ptr -> nx_crypto_cleanup)
    {
        status = hmac_ptr -> nx_crypto_cleanup(metadata);

        if (status)
        {
            return(1);
        }
    }

    *actual_size = (hmac_ptr -> nx_crypto_ICV_size_in_bits >> 3);

    return(0);
#else
    NX_PARAMETER_NOT_USED(hmac_ptr);
    NX_PARAMETER_NOT_USED(start_address);
    NX_PARAMETER_NOT_USED(end_address);
    NX_PARAMETER_NOT_USED(key);
    NX_PARAMETER_NOT_USED(key_length);
    NX_PARAMETER_NOT_USED(metadata);
    NX_PARAMETER_NOT_USED(metadata_size);
    NX_PARAMETER_NOT_USED(output_buffer);
    NX_PARAMETER_NOT_USED(output_buffer_size);
    NX_PARAMETER_NOT_USED(actual_size);
    return(0);
#endif
}

