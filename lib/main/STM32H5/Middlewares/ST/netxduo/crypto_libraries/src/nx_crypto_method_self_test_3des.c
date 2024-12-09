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

#ifndef NX_CRYPTO_3DES_KEY_LEN_IN_BITS
#define NX_CRYPTO_3DES_KEY_LEN_IN_BITS    192
#endif


#ifdef NX_CRYPTO_SELF_TEST

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

/* 7acb4a6d4f37897029e5fefb8cfd2c4c499b4ca7f8d9df49 */
static UCHAR key_2[] = {
0x7a, 0xcb, 0x4a, 0x6d, 0x4f, 0x37, 0x89, 0x70, 0x29, 0xe5, 0xfe, 0xfb, 0x8c, 0xfd, 0x2c, 0x4c, 
0x49, 0x9b, 0x4c, 0xa7, 0xf8, 0xd9, 0xdf, 0x49,
};

/* c925db87872e384e */
static UCHAR iv_2[] = {
0xc9, 0x25, 0xdb, 0x87, 0x87, 0x2e, 0x38, 0x4e,
};
/* c482190e780ebe87136b60b489493fecacc84d62aae9d641d74266fbadf7e8dd11e50be8d820eadc8aa0eb956637e79aa6c68cf404b1003c7c90c7f85428e728 */
static UCHAR plain_2[] = {
0xc4, 0x82, 0x19, 0x0e, 0x78, 0x0e, 0xbe, 0x87, 0x13, 0x6b, 0x60, 0xb4, 0x89, 0x49, 0x3f, 0xec, 
0xac, 0xc8, 0x4d, 0x62, 0xaa, 0xe9, 0xd6, 0x41, 0xd7, 0x42, 0x66, 0xfb, 0xad, 0xf7, 0xe8, 0xdd, 
0x11, 0xe5, 0x0b, 0xe8, 0xd8, 0x20, 0xea, 0xdc, 0x8a, 0xa0, 0xeb, 0x95, 0x66, 0x37, 0xe7, 0x9a, 
0xa6, 0xc6, 0x8c, 0xf4, 0x04, 0xb1, 0x00, 0x3c, 0x7c, 0x90, 0xc7, 0xf8, 0x54, 0x28, 0xe7, 0x28,
};

/* 56304aa594944ede1dbd40f2e64d2da3057a52dc617c08094cf07a8a07a90eb4eba3d3c3790640ffd3b124213487e7c0031cf521251ae7d13f4e797aa4fbae81 */
static UCHAR secret_2[] = {
0x56, 0x30, 0x4a, 0xa5, 0x94, 0x94, 0x4e, 0xde, 0x1d, 0xbd, 0x40, 0xf2, 0xe6, 0x4d, 0x2d, 0xa3, 
0x05, 0x7a, 0x52, 0xdc, 0x61, 0x7c, 0x08, 0x09, 0x4c, 0xf0, 0x7a, 0x8a, 0x07, 0xa9, 0x0e, 0xb4, 
0xeb, 0xa3, 0xd3, 0xc3, 0x79, 0x06, 0x40, 0xff, 0xd3, 0xb1, 0x24, 0x21, 0x34, 0x87, 0xe7, 0xc0, 
0x03, 0x1c, 0xf5, 0x21, 0x25, 0x1a, 0xe7, 0xd1, 0x3f, 0x4e, 0x79, 0x7a, 0xa4, 0xfb, 0xae, 0x81,
};

static UCHAR output[72];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test_3des                     PORTABLE C      */
/*                                                           6.1.7        */
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_method_self_test_3des(NX_CRYPTO_METHOD *crypto_method_3des,
                                                     VOID *metadata, UINT metadata_size)
{
UINT status;
VOID *handler = NX_CRYPTO_NULL;
UINT input_output_length;


    /* Validate the crypto method */
    if(crypto_method_3des == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);
    
    /* Encryption. */
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

    if (crypto_method_3des -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    input_output_length = sizeof(plain_1);

    status = crypto_method_3des -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                       handler,
                                                       crypto_method_3des,
                                                       key_1,
                                                       NX_CRYPTO_3DES_KEY_LEN_IN_BITS,
                                                       plain_1,
                                                       input_output_length,
                                                       iv_1,
                                                       (UCHAR *)output,
                                                       input_output_length,
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_CRYPTO_MEMCMP(output, secret_1, input_output_length) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_3des -> nx_crypto_cleanup)
    {
        status = crypto_method_3des -> nx_crypto_cleanup(metadata);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Decryption. */
    if (crypto_method_3des -> nx_crypto_init)
    {
        status = crypto_method_3des -> nx_crypto_init(crypto_method_3des,
                                                     key_2,
                                                     NX_CRYPTO_3DES_KEY_LEN_IN_BITS,
                                                     &handler,
                                                     metadata,
                                                     metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    input_output_length = sizeof(secret_2);

    status = crypto_method_3des -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                       handler,
                                                       crypto_method_3des,
                                                       key_2,
                                                       NX_CRYPTO_3DES_KEY_LEN_IN_BITS,
                                                       secret_2,
                                                       input_output_length,
                                                       iv_2,
                                                       output,
                                                       input_output_length,
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_CRYPTO_MEMCMP(output, plain_2, input_output_length) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_3des -> nx_crypto_cleanup)
    {
        status = crypto_method_3des -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
