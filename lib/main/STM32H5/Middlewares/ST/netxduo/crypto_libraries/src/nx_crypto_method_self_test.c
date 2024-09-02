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
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Crypto Self Test                                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_CRYPTO_SOURCE_CODE


/* Include necessary system files.  */
#include "nx_crypto_method_self_test.h"

#ifdef NX_CRYPTO_SELF_TEST
static UCHAR metadata[10240];

extern NX_CRYPTO_METHOD crypto_method_aes_cbc_128;
extern NX_CRYPTO_METHOD crypto_method_aes_cbc_192;
extern NX_CRYPTO_METHOD crypto_method_aes_cbc_256;
extern NX_CRYPTO_METHOD crypto_method_3des;
extern NX_CRYPTO_METHOD crypto_method_des;
extern NX_CRYPTO_METHOD crypto_method_rsa;
extern NX_CRYPTO_METHOD crypto_method_md5;
extern NX_CRYPTO_METHOD crypto_method_sha1;
extern NX_CRYPTO_METHOD crypto_method_sha224;
extern NX_CRYPTO_METHOD crypto_method_sha256;
extern NX_CRYPTO_METHOD crypto_method_sha384;
extern NX_CRYPTO_METHOD crypto_method_sha512;
extern NX_CRYPTO_METHOD crypto_method_sha512_224;
extern NX_CRYPTO_METHOD crypto_method_sha512_256;
extern NX_CRYPTO_METHOD crypto_method_hmac_md5;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha1;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha224;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha256;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha384;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha512;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha512_224;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha512_256;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_1;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_sha256;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_sha384;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_sha512;
extern NX_CRYPTO_METHOD crypto_method_drbg;
extern NX_CRYPTO_METHOD crypto_method_ecdsa;
extern NX_CRYPTO_METHOD crypto_method_pkcs1;
extern NX_CRYPTO_METHOD crypto_method_ecdh;
extern NX_CRYPTO_METHOD crypto_method_ecdhe;

const CHAR nx_crypto_hash_key[] = "EL_CRYPTO_VERSION_5.12   _FOR_FIPS";
const UINT nx_crypto_hash_key_size = sizeof(nx_crypto_hash_key) << 3;

#define NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)                                     \
    if(status)                                                                      \
    {                                                                               \
        _nx_crypto_library_state |= NX_CRYPTO_LIBRARY_STATE_POST_FAILED;            \
    }


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for crypto method.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method_ptr                            Pointer to the crypto method  */
/*                                            to be tested.               */
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/

NX_CRYPTO_KEEP INT _nx_crypto_method_self_test(INT arg)
{
UINT metadata_size = sizeof(metadata);
UINT status;

    /* Set the crypto state to POST_IN_PROGRESS */
    /* Also clear the UNINITIALIZED flag */
    _nx_crypto_library_state = _nx_crypto_library_state & (~NX_CRYPTO_LIBRARY_STATE_UNINITIALIZED);
    _nx_crypto_library_state = _nx_crypto_library_state | NX_CRYPTO_LIBRARY_STATE_POST_IN_PROGRESS;

    /* Initialize hardware random number generator.  */
    NX_CRYPTO_HARDWARE_RAND_INITIALIZE
        
    NX_CRYPTO_INTEGRITY_TEST

    status = _nx_crypto_method_self_test_des(&crypto_method_des, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_aes(&crypto_method_aes_cbc_256, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_3des(&crypto_method_3des, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_rsa(&crypto_method_rsa, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_md5(&crypto_method_md5, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_sha(&crypto_method_sha1, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_sha(&crypto_method_sha224, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_sha(&crypto_method_sha256, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_sha(&crypto_method_sha384, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_sha(&crypto_method_sha512, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_sha(&crypto_method_sha512_224, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_sha(&crypto_method_sha512_256, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_hmac_md5(&crypto_method_hmac_md5, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_hmac_sha(&crypto_method_hmac_sha1, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_hmac_sha(&crypto_method_hmac_sha224, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_hmac_sha(&crypto_method_hmac_sha256, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_hmac_sha(&crypto_method_hmac_sha384, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_hmac_sha(&crypto_method_hmac_sha512, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_hmac_sha(&crypto_method_hmac_sha512_224, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_hmac_sha(&crypto_method_hmac_sha512_256, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_prf(&crypto_method_tls_prf_1, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_prf(&crypto_method_tls_prf_sha256, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_prf(&crypto_method_tls_prf_sha384, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_prf(&crypto_method_tls_prf_sha512, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_drbg(&crypto_method_drbg, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_ecdsa(&crypto_method_ecdsa, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_pkcs1(&crypto_method_pkcs1, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_ecdh(&crypto_method_ecdh, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    status = _nx_crypto_method_self_test_ecdh(&crypto_method_ecdhe, metadata, metadata_size);
    NX_CRYPTO_FUNCTIONAL_TEST_CHECK(status)

    /* Clear the POST-inprogress flag */
    _nx_crypto_library_state = _nx_crypto_library_state & (~NX_CRYPTO_LIBRARY_STATE_POST_IN_PROGRESS);

    /* Set the library state to "operational" if POST is successful. */
    if((_nx_crypto_library_state & NX_CRYPTO_LIBRARY_STATE_POST_FAILED) == 0)
       _nx_crypto_library_state = NX_CRYPTO_LIBRARY_STATE_OPERATIONAL;

    /* All done. Return. */
    return(arg);
}
#endif
