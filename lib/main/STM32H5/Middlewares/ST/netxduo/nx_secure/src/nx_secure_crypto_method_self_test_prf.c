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

#include "nx_secure_crypto_table_self_test.h"

#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK

static UCHAR secret_sha1[] = { 0x86, 0xec, 0x88, 0xbb };
static UCHAR label_sha1[] = { 0xc8, 0x37, 0xaf, 0x7d };
static UCHAR seed_sha1[] = { 0x36, 0x54, 0xf1, 0x6f };
static UCHAR result_sha1[] = {
0xab, 0xe3, 0x77, 0xa6, 0x58, 0x4c, 0x97, 0x03, 0x98, 0xe9, 0xe4, 0x62, 0xe6, 0x44, 0xe4, 0x2d, 
0x21, 0x16, 0xdb, 0x4f, 0x0e, 0x70, 0xc9, 0x83, 0xe5, 0x31, 0x61, 0x95, 0x17, 0xcd, 0xc2, 0xd0, 
0x7e, 0x9a, 0xdf, 0xf6, 0xe2, 0x44, 0x01, 0x05, 0xa9, 0xb0, 0x7a, 0xbe, 0xc3, 0x9a, 0x47, 0x9b, 
0xd7, 0xd9, 0x2c, 0xba, 0xb7, 0x8e, 0x90, 0x1d, 0x4f, 0x21, 0xae, 0x4e, 0x0f, 0x60, 0xcf, 0x3b, 
0xdf, 0xe5, 0x77, 0x79, 0xff, 0x23, 0x23, 0x2d, 0x62, 0x48, 0xc6, 0x72, 0xb3, 0xf9, 0xce, 0x4f, 
0x46, 0x66, 0x2f, 0xc5, 0x0e, 0xbc, 0x2a, 0x34, 0xd0, 0xc5, 0x37, 0xa2, 0x2f, 0x69, 0x43, 0x74, 
0x6d, 0x11, 0x3c, 0x1c, 0x75, 0xa5, 0x12, 0x61, 0x1a, 0xc7, 0x8f, 0x41, 0xab, 0xa8, 0x45, 0xd5, 
0xf3, 0xb3, 0xb4, 0xbd, 0xe3, 0x7c, 0x8b, 0xbb, 0x0f, 0x0d, 0xcb, 0x57, 0xc8, 0x6c, 0x13, 0x32, 
0x8b, 0xe1, 0xd0, 0x2e, 0x2c, 0x2d, 0xb2, 0xd5, 0x67, 0x1c, 0xb0, 0x61, 0x3d, 0x77, 0x96, 0x68, 
0x1b, 0x47, 0xdf, 0x3a, 0x50, 0x62, 0x31, 0x4b, 0x30, 0x09, 0xe9, 0x3b, 0xdd, 0xfb, 0x34, 0x1d,
};

static UCHAR secret_sha256[] = { 0xbc, 0xd6, 0x2b, 0x3f };
static UCHAR label_sha256[] = { 0xad, 0x16, 0x21, 0xba };
static UCHAR seed_sha256[] = { 0x73, 0x21, 0xe1, 0x4c };
static UCHAR result_sha256[] = {
0x05, 0xab, 0x98, 0x15, 0xd8, 0x70, 0xd6, 0xdb, 0x27, 0xde, 0x5c, 0xd6, 0x8b, 0xd6, 0xbd, 0xfe, 
0x0c, 0xee, 0xde, 0x1b, 0xd8, 0x9e, 0x80, 0x4b, 0xf4, 0x71, 0xb2, 0x5f, 0x8a, 0xb6, 0x60, 0x83, 
0x95, 0xe3, 0x96, 0x36, 0x5d, 0xa6, 0xc0, 0x42, 0x57, 0x02, 0x57, 0x49, 0x5e, 0xff, 0x52, 0xf2, 
0xe2, 0x96, 0x9a, 0x26, 0x71, 0x87, 0x6d, 0xb7, 0x21, 0x90, 0x4e, 0x82, 0xbb, 0xd1, 0x77, 0xbf, 
0x8f, 0xd9, 0x89, 0x1f, 0x6d, 0xda, 0xbe, 0xb4, 0x35, 0xb3, 0x0d, 0x3e, 0xdf, 0xcb, 0x18, 0x5b,
};

/* Output. */
static UCHAR output[256];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_crypto_method_self_test_prf              PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for PRF crypto method. */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_crypto_method_self_test_prf(NX_CRYPTO_METHOD *crypto_method_prf,
                                            VOID *metadata, UINT metadata_size)
{
UCHAR  *secret;
UCHAR  *label;
UCHAR  *seed;
UCHAR  *result;
UINT    secret_length;
UINT    label_length;
UINT    seed_length;
UINT    result_length;
UINT    status;
VOID   *handler = NX_NULL;


    /* Validate the crypto method */
    if(crypto_method_prf == NX_NULL)
        return(NX_PTR_ERROR);

    /* Set the test data.  */
    switch (crypto_method_prf -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_PRF_HMAC_SHA1:
        secret = secret_sha1;
        secret_length = sizeof(secret_sha1);
        label = label_sha1;
        label_length = sizeof(label_sha1);
        seed = seed_sha1;
        seed_length = sizeof(seed_sha1);
        result = result_sha1;
        result_length = sizeof(result_sha1);
        break;
    case NX_CRYPTO_PRF_HMAC_SHA2_256:
        secret = secret_sha256;
        secret_length = sizeof(secret_sha256);
        label = label_sha256;
        label_length = sizeof(label_sha256);
        seed = seed_sha256;
        seed_length = sizeof(seed_sha256);
        result = result_sha256;
        result_length = sizeof(result_sha256);
        break;
    default:
        return(1);
    }

    /* Clear the output buffer.  */
    NX_SECURE_MEMSET(output, 0, sizeof(output));

    /* Call the crypto initialization function.  */
    if (crypto_method_prf -> nx_crypto_init)
    {
        status = crypto_method_prf -> nx_crypto_init(crypto_method_prf,
                                                     secret,
                                                     secret_length,
                                                     &handler,
                                                     metadata,
                                                     metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_prf -> nx_crypto_operation == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Call the crypto operation function.  */
    status = crypto_method_prf -> nx_crypto_operation(NX_CRYPTO_PRF,
                                                      handler,
                                                      crypto_method_prf,
                                                      label,
                                                      label_length,
                                                      seed,
                                                      seed_length,
                                                      NX_NULL,
                                                      (UCHAR *)output,
                                                      result_length,
                                                      metadata,
                                                      metadata_size,
                                                      NX_NULL, NX_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Validate the output.  */
    if(NX_SECURE_MEMCMP(output, result, result_length) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (crypto_method_prf -> nx_crypto_cleanup)
    {
        status = crypto_method_prf -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
