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
#ifndef NX_CRYPTO_3DES_KEY_LEN_IN_BITS
#define NX_CRYPTO_3DES_KEY_LEN_IN_BITS    192
#endif


#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK

#define INPUT_OUTPUT_LENGTH 72

/* 8f4f7aab25043720f4fbae01aedf071c68a283689b08ad20 */
static UCHAR key_1[] = {
0x8f, 0x4f, 0x7a, 0xab, 0x25, 0x04, 0x37, 0x20, 0xf4, 0xfb, 0xae, 0x01, 0xae, 0xdf, 0x07, 0x1c, 
0x68, 0xa2, 0x83, 0x68, 0x9b, 0x08, 0xad, 0x20,
};

/* 17fdf67a5290baff */
static UCHAR iv_1[] = {
0x17, 0xfd, 0xf6, 0x7a, 0x52, 0x90, 0xba, 0xff,
};

/* 67c3aaf34a1dce2ee6a65b4f6c9c272384d343cae3fd2d1520284733c388888da07772ee63ba44e76b067072dcc24fd5ef0f14c98d06ffdc1d40d3149a9c89d5e83c460468d18b6d */
static UCHAR plain_1[] = {
0x67, 0xc3, 0xaa, 0xf3, 0x4a, 0x1d, 0xce, 0x2e, 0xe6, 0xa6, 0x5b, 0x4f, 0x6c, 0x9c, 0x27, 0x23, 
0x84, 0xd3, 0x43, 0xca, 0xe3, 0xfd, 0x2d, 0x15, 0x20, 0x28, 0x47, 0x33, 0xc3, 0x88, 0x88, 0x8d, 
0xa0, 0x77, 0x72, 0xee, 0x63, 0xba, 0x44, 0xe7, 0x6b, 0x06, 0x70, 0x72, 0xdc, 0xc2, 0x4f, 0xd5, 
0xef, 0x0f, 0x14, 0xc9, 0x8d, 0x06, 0xff, 0xdc, 0x1d, 0x40, 0xd3, 0x14, 0x9a, 0x9c, 0x89, 0xd5, 
0xe8, 0x3c, 0x46, 0x04, 0x68, 0xd1, 0x8b, 0x6d,
};

/* b2a6679b02081e22aaa950f81a414fa3e7023ca6b1ba0e8e599ecfa80797392a70081c68b73aec962384d70835a80f8739d6d5f1aba404f6d16eab6f6115ccedc4da93a27ef36bff */
static UCHAR secret_1[] = {
0xb2, 0xa6, 0x67, 0x9b, 0x02, 0x08, 0x1e, 0x22, 0xaa, 0xa9, 0x50, 0xf8, 0x1a, 0x41, 0x4f, 0xa3, 
0xe7, 0x02, 0x3c, 0xa6, 0xb1, 0xba, 0x0e, 0x8e, 0x59, 0x9e, 0xcf, 0xa8, 0x07, 0x97, 0x39, 0x2a, 
0x70, 0x08, 0x1c, 0x68, 0xb7, 0x3a, 0xec, 0x96, 0x23, 0x84, 0xd7, 0x08, 0x35, 0xa8, 0x0f, 0x87, 
0x39, 0xd6, 0xd5, 0xf1, 0xab, 0xa4, 0x04, 0xf6, 0xd1, 0x6e, 0xab, 0x6f, 0x61, 0x15, 0xcc, 0xed, 
0xc4, 0xda, 0x93, 0xa2, 0x7e, 0xf3, 0x6b, 0xff,
};

static UCHAR output[INPUT_OUTPUT_LENGTH];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_crypto_method_self_test_3des              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for 3DES crypto method.*/
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
UINT _nx_secure_crypto_method_self_test_3des(NX_CRYPTO_METHOD *crypto_method_3des,
                                             VOID *metadata, UINT metadata_size)
{
UINT status;
VOID *handler = NX_NULL;


    /* Validate the crypto method */
    if(crypto_method_3des == NX_NULL)
        return(NX_PTR_ERROR);

    if (crypto_method_3des -> nx_crypto_init)
    {
        status = crypto_method_3des -> nx_crypto_init(crypto_method_3des,
                                                      key_1,
                                                      NX_CRYPTO_3DES_KEY_LEN_IN_BITS,
                                                      &handler,
                                                      metadata,
                                                      metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_3des -> nx_crypto_operation == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Encryption. */
    status = crypto_method_3des -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                       handler,
                                                       crypto_method_3des,
                                                       key_1,
                                                       NX_CRYPTO_3DES_KEY_LEN_IN_BITS,
                                                       plain_1,
                                                       INPUT_OUTPUT_LENGTH,
                                                       iv_1,
                                                       (UCHAR *)output,
                                                       INPUT_OUTPUT_LENGTH,
                                                       metadata,
                                                       metadata_size,
                                                       NX_NULL, NX_NULL);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_SECURE_MEMCMP(output, secret_1, INPUT_OUTPUT_LENGTH) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Decryption. */
    status = crypto_method_3des -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                       handler,
                                                       crypto_method_3des,
                                                       key_1,
                                                       NX_CRYPTO_3DES_KEY_LEN_IN_BITS,
                                                       secret_1,
                                                       INPUT_OUTPUT_LENGTH,
                                                       iv_1,
                                                       output,
                                                       INPUT_OUTPUT_LENGTH,
                                                       metadata,
                                                       metadata_size,
                                                       NX_NULL, NX_NULL);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_SECURE_MEMCMP(output, plain_1, INPUT_OUTPUT_LENGTH) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (crypto_method_3des -> nx_crypto_cleanup)
    {
        status = crypto_method_3des -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
