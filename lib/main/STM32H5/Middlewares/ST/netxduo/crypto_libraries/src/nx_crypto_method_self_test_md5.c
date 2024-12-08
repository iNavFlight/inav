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

#define NX_CRYPTO_SOURCE_CODE


/* Include necessary system files.  */
#include "nx_crypto_method_self_test.h"

#ifdef NX_CRYPTO_SELF_TEST

/* f149e41d848f59276cfddd743bafa9a90e1ee4a263a118142b33e3702176ef0a59f8237a1cb51b42f3ded6b202d9af0997898fdd03cf60bda951c514547a0850cec25444ae2f24cb711bfbafcc3956c941d3de69f155e3f8b10f06db5f37359b772ddd43e1035a0a0d3db33242d5843033833b0dd43b870c6bf60e8deab55f317cc3273f5e3ba747f0cb65050cb7228796210d9254873643008d45f29cfd6c5b060c9a */
static UCHAR plain_1[] = {
0xf1, 0x49, 0xe4, 0x1d, 0x84, 0x8f, 0x59, 0x27, 0x6c, 0xfd, 0xdd, 0x74, 0x3b, 0xaf, 0xa9, 0xa9,
0x0e, 0x1e, 0xe4, 0xa2, 0x63, 0xa1, 0x18, 0x14, 0x2b, 0x33, 0xe3, 0x70, 0x21, 0x76, 0xef, 0x0a,
0x59, 0xf8, 0x23, 0x7a, 0x1c, 0xb5, 0x1b, 0x42, 0xf3, 0xde, 0xd6, 0xb2, 0x02, 0xd9, 0xaf, 0x09,
0x97, 0x89, 0x8f, 0xdd, 0x03, 0xcf, 0x60, 0xbd, 0xa9, 0x51, 0xc5, 0x14, 0x54, 0x7a, 0x08, 0x50,
0xce, 0xc2, 0x54, 0x44, 0xae, 0x2f, 0x24, 0xcb, 0x71, 0x1b, 0xfb, 0xaf, 0xcc, 0x39, 0x56, 0xc9,
0x41, 0xd3, 0xde, 0x69, 0xf1, 0x55, 0xe3, 0xf8, 0xb1, 0x0f, 0x06, 0xdb, 0x5f, 0x37, 0x35, 0x9b,
0x77, 0x2d, 0xdd, 0x43, 0xe1, 0x03, 0x5a, 0x0a, 0x0d, 0x3d, 0xb3, 0x32, 0x42, 0xd5, 0x84, 0x30,
0x33, 0x83, 0x3b, 0x0d, 0xd4, 0x3b, 0x87, 0x0c, 0x6b, 0xf6, 0x0e, 0x8d, 0xea, 0xb5, 0x5f, 0x31,
0x7c, 0xc3, 0x27, 0x3f, 0x5e, 0x3b, 0xa7, 0x47, 0xf0, 0xcb, 0x65, 0x05, 0x0c, 0xb7, 0x22, 0x87,
0x96, 0x21, 0x0d, 0x92, 0x54, 0x87, 0x36, 0x43, 0x00, 0x8d, 0x45, 0xf2, 0x9c, 0xfd, 0x6c, 0x5b,
0x06, 0x0c, 0x9a,
};

/* c108aa74e7a4dfd52806e67e72d76f54 */
static UCHAR secret_1[] = {
0xc1, 0x08, 0xaa, 0x74, 0xe7, 0xa4, 0xdf, 0xd5, 0x28, 0x06, 0xe6, 0x7e, 0x72, 0xd7, 0x6f, 0x54,
};

/* Output. */
static ULONG output[4];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test_md5                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for MD5 crypto method. */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_self_test_md5(NX_CRYPTO_METHOD *crypto_method_md5,
                                                    VOID *metadata, UINT metadata_size)
{
UINT    status;
VOID   *handler = NX_CRYPTO_NULL;


    /* Validate the crypto method */
    if(crypto_method_md5 == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    if (crypto_method_md5 -> nx_crypto_init)
    {
        status = crypto_method_md5 -> nx_crypto_init(crypto_method_md5,
                                                     NX_CRYPTO_NULL,
                                                     0,
                                                     &handler,
                                                     metadata,
                                                     metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_md5 -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Clear the output buffer.  */
    NX_CRYPTO_MEMSET(output, 0, sizeof(output));

    /* Call the crypto operation function.  */
    status = crypto_method_md5 -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                                      handler,
                                                      crypto_method_md5,
                                                      NX_CRYPTO_NULL,
                                                      0,
                                                      plain_1,
                                                      sizeof(plain_1),
                                                      NX_CRYPTO_NULL,
                                                      (UCHAR *)output,
                                                      sizeof(secret_1),
                                                      metadata,
                                                      metadata_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Validate the output.  */
    if(NX_CRYPTO_MEMCMP(output, secret_1, sizeof(secret_1)) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_md5 -> nx_crypto_cleanup)
    {
        status = crypto_method_md5 -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
