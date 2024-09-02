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

static UCHAR secret_sha256[] = { 
0x6e, 0x97, 0x83, 0xfe, 0x56, 0x7e, 0xda, 0x66, 0xfc, 0x0e, 0xf5, 0x17, 0xd9, 0x52, 0xc4, 0x7c, 
0x2f, 0x17, 0x06, 0x3c, 0xc5, 0x44, 0xd2, 0x6f, 0x98, 0x49, 0x2c, 0x50, 0x21, 0xe1, 0x27, 0x74, 
0xa5, 0x98, 0x60, 0x87, 0x3e, 0x7a, 0x31, 0xaa, 0x2d, 0xc6, 0x90, 0x9e, 0x93, 0xb1, 0xb5, 0x72 
};
static UCHAR label_sha256[] = { 
0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74 
};
static UCHAR seed_sha256[] = { 
0x66, 0xc1, 0x7c, 0xf0, 0x53, 0x40, 0x84, 0x42, 0x7c, 0xaa, 0x2b, 0xf7, 0xbd, 0x58, 0x53, 0x9d, 
0xb9, 0x3a, 0xc0, 0xeb, 0xcc, 0x05, 0xd7, 0x3c, 0x46, 0x43, 0x63, 0x6b, 0x09, 0x3a, 0x70, 0xe8, 
0xa4, 0xe5, 0x80, 0x45, 0x5b, 0x08, 0xda, 0x0f, 0x33, 0x0f, 0x98, 0x82, 0xaf, 0xcd, 0x7f, 0xe7, 
0x9c, 0x64, 0x9c, 0x35, 0x86, 0x24, 0x43, 0xb9, 0xb7, 0x99, 0x1b, 0xa0, 0xd1, 0xb1, 0x28, 0x9c 
};
static UCHAR result_sha256[] = {
0x8f, 0x80, 0x2d, 0x64, 0x6f, 0x18, 0x04, 0xdd, 0xda, 0x9d, 0xc9, 0x57, 0x79, 0x85, 0x4f, 0x86, 
0xc2, 0x9e, 0x2b, 0x9c, 0x17, 0xc3, 0x91, 0x68, 0xac, 0xea, 0x05, 0xc5, 0x3b, 0x4a, 0x67, 0xd0, 
0x4d, 0x78, 0x49, 0x87, 0xdf, 0x04, 0xd2, 0x5c, 0xd3, 0xe8, 0xb4, 0x6d, 0x11, 0x70, 0x3a, 0xe5,
};

static UCHAR secret_sha384[] = { 
0x06, 0x40, 0xec, 0xd9, 0x8b, 0xee, 0xba, 0x99, 0x79, 0xae, 0x75, 0x1d, 0xfa, 0x51, 0x0f, 0xc2, 
0xc7, 0x4a, 0x83, 0x54, 0xec, 0x9a, 0x12, 0x1e, 0x99, 0xe1, 0xde, 0xcb, 0xcb, 0xe2, 0x33, 0x7c, 
0x35, 0xb6, 0xc8, 0xd5, 0x85, 0x9a, 0xd2, 0x57, 0xbd, 0x9d, 0x04, 0x59, 0x40, 0x44, 0x1f, 0x10 
};
static UCHAR label_sha384[] = {
0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74
};
static UCHAR seed_sha384[] = { 
0xdd, 0xfe, 0x04, 0x5c, 0x59, 0xd7, 0x87, 0xf8, 0x5d, 0xaf, 0x1c, 0x49, 0x48, 0xd9, 0xa8, 0x16, 
0x62, 0x07, 0x16, 0x7b, 0x18, 0x20, 0xa2, 0x24, 0x0b, 0x5b, 0x52, 0x8b, 0x51, 0xfb, 0x3c, 0xf6, 
0x70, 0xd2, 0xfe, 0xd4, 0x8a, 0xc7, 0xe5, 0x37, 0xac, 0xbb, 0xf6, 0x5b, 0x8b, 0x4d, 0xd6, 0x99, 
0x4a, 0xb7, 0x9c, 0x1f, 0x26, 0x8e, 0x2f, 0xfb, 0xf2, 0x0e, 0x76, 0x4a, 0x2b, 0xcf, 0x09, 0xa1 
};
static UCHAR result_sha384[] = {
0xd2, 0xfc, 0x23, 0x9e, 0x32, 0x0c, 0xbe, 0x2b, 0x4d, 0xa4, 0xc3, 0x79, 0x76, 0xff, 0xd5, 0x56, 
0x56, 0x45, 0xc9, 0xff, 0x7c, 0x23, 0x9c, 0xfe, 0xa1, 0x0f, 0x6a, 0xf4, 0x47, 0xbe, 0xfe, 0x67, 
0xa2, 0xc5, 0x45, 0x78, 0x2b, 0x6d, 0x1b, 0xcb, 0xda, 0xc7, 0x24, 0x8c, 0xe6, 0xb6, 0x25, 0xda
};

static UCHAR secret_sha512[] = { 
0xcb, 0x1b, 0xc6, 0x9f, 0xea, 0xb9, 0x64, 0xb7, 0x66, 0x29, 0x7c, 0x7e, 0xb8, 0xf2, 0x9c, 0x89, 
0x2a, 0xb3, 0x80, 0x27, 0x08, 0x98, 0x7d, 0xf5, 0xb6, 0x1a, 0x80, 0x66, 0x6b, 0x9e, 0x12, 0xb2, 
0x32, 0xcb, 0x5e, 0x0e, 0x40, 0xf0, 0x5f, 0xb3, 0x92, 0x35, 0xb4, 0x88, 0x94, 0x5a, 0x70, 0x61
};
static UCHAR label_sha512[] = {
0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74
};
static UCHAR seed_sha512[] = { 
0x13, 0x20, 0xa8, 0xa1, 0x17, 0xbb, 0x6c, 0xa0, 0xf8, 0x8e, 0x19, 0x23, 0x64, 0x11, 0x4b, 0x7f, 
0x73, 0x21, 0x6b, 0x7a, 0xcc, 0x4f, 0xb9, 0x1b, 0xef, 0x49, 0xfa, 0x9b, 0xb1, 0x4d, 0x32, 0xc2, 
0xac, 0xd7, 0x02, 0xca, 0xf7, 0xd3, 0x76, 0xf3, 0x5e, 0x65, 0x66, 0xb8, 0x5d, 0xdb, 0x3d, 0xa9, 
0x3a, 0xe4, 0xeb, 0x63, 0x6d, 0x2a, 0x7a, 0x7a, 0xf7, 0x2f, 0xf3, 0x2a, 0x43, 0xe1, 0x64, 0x95
};
static UCHAR result_sha512[] = {
0x91, 0xf2, 0xc6, 0x51, 0x77, 0xe4, 0x8f, 0xde, 0x4d, 0xd1, 0x27, 0xc3, 0xf5, 0xb5, 0x9c, 0x2a, 
0x7d, 0x38, 0xa4, 0x78, 0x29, 0x86, 0x19, 0x7c, 0x11, 0x0c, 0xca, 0x2f, 0x0a, 0x18, 0x56, 0x8f, 
0x31, 0x7a, 0x74, 0xa6, 0xea, 0xd3, 0x3c, 0x0f, 0x84, 0xa5, 0xd4, 0x1f, 0x8d, 0x36, 0x8f, 0x3a
};

/* Output. */
static UCHAR output[256];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test_prf                      PORTABLE C      */
/*                                                           6.1.7        */
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_method_self_test_prf(NX_CRYPTO_METHOD *crypto_method_prf,
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
VOID   *handler = NX_CRYPTO_NULL;


    /* Validate the crypto method */
    if(crypto_method_prf == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

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
    case NX_CRYPTO_PRF_HMAC_SHA2_384:
        secret = secret_sha384;
        secret_length = sizeof(secret_sha384);
        label = label_sha384;
        label_length = sizeof(label_sha384);
        seed = seed_sha384;
        seed_length = sizeof(seed_sha384);
        result = result_sha384;
        result_length = sizeof(result_sha384);
        break;
    case NX_CRYPTO_PRF_HMAC_SHA2_512:
        secret = secret_sha512;
        secret_length = sizeof(secret_sha512);
        label = label_sha512;
        label_length = sizeof(label_sha512);
        seed = seed_sha512;
        seed_length = sizeof(seed_sha512);
        result = result_sha512;
        result_length = sizeof(result_sha512);
        break;
    default:
        return(1);
    }

    /* Clear the output buffer.  */
    NX_CRYPTO_MEMSET(output, 0, sizeof(output));

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

    if (crypto_method_prf -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Call the crypto operation function.  */
    status = crypto_method_prf -> nx_crypto_operation(NX_CRYPTO_PRF,
                                                      handler,
                                                      crypto_method_prf,
                                                      label,
                                                      label_length,
                                                      seed,
                                                      seed_length,
                                                      NX_CRYPTO_NULL,
                                                      (UCHAR *)output,
                                                      result_length,
                                                      metadata,
                                                      metadata_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Validate the output.  */
    if(NX_CRYPTO_MEMCMP(output, result, result_length) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_prf -> nx_crypto_cleanup)
    {
        status = crypto_method_prf -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
