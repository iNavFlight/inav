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

#ifndef NX_CRYPTO_DES_KEY_LEN_IN_BITS
#define NX_CRYPTO_DES_KEY_LEN_IN_BITS 64
#endif


#ifdef NX_CRYPTO_SELF_TEST

#define INPUT_OUTPUT_LENGTH 64

/* 7a52e4d342aa0725 */
static UCHAR key_1[] = {
0x7a, 0x52, 0xe4, 0xd3, 0x42, 0xaa, 0x07, 0x25,
};

/* 5a7e7c34266cf730 */
static UCHAR iv_1[] = {
0x5a, 0x7e, 0x7c, 0x34, 0x26, 0x6c, 0xf7, 0x30,
};

/* 7e771c6ee4b26db89050e982ba7e9803c8da34606434dd85d2910e538076d00168c9885ba2be03181f65f1e04e83d6ba6880467550bcf099be26dc9d9c0af15a */
static UCHAR plain_1[] = {
0x7e, 0x77, 0x1c, 0x6e, 0xe4, 0xb2, 0x6d, 0xb8, 0x90, 0x50, 0xe9, 0x82, 0xba, 0x7e, 0x98, 0x03, 
0xc8, 0xda, 0x34, 0x60, 0x64, 0x34, 0xdd, 0x85, 0xd2, 0x91, 0x0e, 0x53, 0x80, 0x76, 0xd0, 0x01, 
0x68, 0xc9, 0x88, 0x5b, 0xa2, 0xbe, 0x03, 0x18, 0x1f, 0x65, 0xf1, 0xe0, 0x4e, 0x83, 0xd6, 0xba, 
0x68, 0x80, 0x46, 0x75, 0x50, 0xbc, 0xf0, 0x99, 0xbe, 0x26, 0xdc, 0x9d, 0x9c, 0x0a, 0xf1, 0x5a, 
};

/* 13570ef15c1e8a71ae9de5552d9c7f7ca12eb830d85e6eb6d0402f627e23b967ba6e15cd97e9c9b64b16fe910903d9cb3757b3078a90c2ba4645fd638fa0725c */
static UCHAR secret_1[] = {
0x13, 0x57, 0x0e, 0xf1, 0x5c, 0x1e, 0x8a, 0x71, 0xae, 0x9d, 0xe5, 0x55, 0x2d, 0x9c, 0x7f, 0x7c, 
0xa1, 0x2e, 0xb8, 0x30, 0xd8, 0x5e, 0x6e, 0xb6, 0xd0, 0x40, 0x2f, 0x62, 0x7e, 0x23, 0xb9, 0x67, 
0xba, 0x6e, 0x15, 0xcd, 0x97, 0xe9, 0xc9, 0xb6, 0x4b, 0x16, 0xfe, 0x91, 0x09, 0x03, 0xd9, 0xcb, 
0x37, 0x57, 0xb3, 0x07, 0x8a, 0x90, 0xc2, 0xba, 0x46, 0x45, 0xfd, 0x63, 0x8f, 0xa0, 0x72, 0x5c, 
};

static UCHAR output[INPUT_OUTPUT_LENGTH];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test_des                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for DES crypto method. */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_self_test_des(NX_CRYPTO_METHOD *crypto_method_des,
                                                    VOID *metadata, UINT metadata_size)
{
UINT status;
VOID *handler = NX_CRYPTO_NULL;


    /* Validate the crypto method */
    if(crypto_method_des == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    if (crypto_method_des -> nx_crypto_init)
    {
        status = crypto_method_des -> nx_crypto_init(crypto_method_des,
                                                     key_1,
                                                     NX_CRYPTO_DES_KEY_LEN_IN_BITS,
                                                     &handler,
                                                     metadata,
                                                     metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_des -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Encryption. */
    status = crypto_method_des -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                      handler,
                                                      crypto_method_des,
                                                      key_1,
                                                      NX_CRYPTO_DES_KEY_LEN_IN_BITS,
                                                      plain_1,
                                                      INPUT_OUTPUT_LENGTH,
                                                      iv_1,
                                                      (UCHAR *)output,
                                                      INPUT_OUTPUT_LENGTH,
                                                      metadata,
                                                      metadata_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_CRYPTO_MEMCMP(output, secret_1, INPUT_OUTPUT_LENGTH) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Decryption. */
    status = crypto_method_des -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                      handler,
                                                      crypto_method_des,
                                                      key_1,
                                                      NX_CRYPTO_DES_KEY_LEN_IN_BITS,
                                                      secret_1,
                                                      INPUT_OUTPUT_LENGTH,
                                                      iv_1,
                                                      output,
                                                      INPUT_OUTPUT_LENGTH,
                                                      metadata,
                                                      metadata_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_CRYPTO_MEMCMP(output, plain_1, INPUT_OUTPUT_LENGTH) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_des -> nx_crypto_cleanup)
    {
        status = crypto_method_des -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
