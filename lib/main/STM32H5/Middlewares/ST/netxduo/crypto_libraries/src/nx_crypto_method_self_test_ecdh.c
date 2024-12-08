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
/**   Crypto Self-test                                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_CRYPTO_SOURCE_CODE


/* Include necessary system files.  */
#include "nx_crypto_method_self_test.h"

#ifdef NX_CRYPTO_SELF_TEST

/* 7fc1c44f7f432927f404922107eae9d10b26ed35527d66d9858c58be388c98a1
   981a1b7c098a26747a723b171cae16670de0c320a82b1cfcbe77d2f807a217b5 */
static UCHAR test_public_key[] = {
0x04, 
0x7f, 0xc1, 0xc4, 0x4f, 0x7f, 0x43, 0x29, 0x27, 0xf4, 0x04, 0x92, 0x21, 0x07, 0xea, 0xe9, 0xd1, 
0x0b, 0x26, 0xed, 0x35, 0x52, 0x7d, 0x66, 0xd9, 0x85, 0x8c, 0x58, 0xbe, 0x38, 0x8c, 0x98, 0xa1,
0x98, 0x1a, 0x1b, 0x7c, 0x09, 0x8a, 0x26, 0x74, 0x7a, 0x72, 0x3b, 0x17, 0x1c, 0xae, 0x16, 0x67, 
0x0d, 0xe0, 0xc3, 0x20, 0xa8, 0x2b, 0x1c, 0xfc, 0xbe, 0x77, 0xd2, 0xf8, 0x07, 0xa2, 0x17, 0xb5,
};
static UCHAR test_public_key_len =sizeof(test_public_key);

/* 16439d147442b843cf849727211ba081e9f6275c0407d741652ffd9ec285ee2e */
static UCHAR test_private_key[] = {
0x16, 0x43, 0x9d, 0x14, 0x74, 0x42, 0xb8, 0x43, 0xcf, 0x84, 0x97, 0x27, 0x21, 0x1b, 0xa0, 0x81, 
0xe9, 0xf6, 0x27, 0x5c, 0x04, 0x07, 0xd7, 0x41, 0x65, 0x2f, 0xfd, 0x9e, 0xc2, 0x85, 0xee, 0x2e,
};
static UCHAR test_private_key_len =sizeof(test_private_key);

static UCHAR local_public_key[128];
static UINT local_public_key_len;

static UCHAR shared_secret[128];
static UINT shared_secret_len;

static UCHAR output[128];

extern NX_CRYPTO_METHOD crypto_method_ec_secp256;

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test_ecdh                     PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for ECDH crypto method.*/
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_self_test_ecdh(NX_CRYPTO_METHOD *crypto_method_ecdh,
                                                      VOID *metadata, UINT metadata_size)
{
NX_CRYPTO_METHOD *curve_method;
UINT   status;
NX_CRYPTO_EXTENDED_OUTPUT extended_output;


    /* Validate the crypto method */
    if(crypto_method_ecdh == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    /* Set the test data.  */
    curve_method = &crypto_method_ec_secp256;

    /* Clear the output buffer.  */
    NX_CRYPTO_MEMSET(output, 0, sizeof(output));

    /* Call the crypto initialization function.  */
    if (crypto_method_ecdh -> nx_crypto_init)
    {
        status = crypto_method_ecdh -> nx_crypto_init(crypto_method_ecdh,
                                                      NX_CRYPTO_NULL,
                                                      0,
                                                      NX_CRYPTO_NULL,
                                                      metadata,
                                                      metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_ecdh -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Set EC curve.  */
    status = crypto_method_ecdh -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET,
                                                       NX_CRYPTO_NULL,
                                                       crypto_method_ecdh,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       (UCHAR *)curve_method,
                                                       sizeof(NX_CRYPTO_METHOD *),
                                                       NX_CRYPTO_NULL,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Generate local public key.  */
    extended_output.nx_crypto_extended_output_data = local_public_key;
    extended_output.nx_crypto_extended_output_length_in_byte = sizeof(local_public_key);
    status = crypto_method_ecdh -> nx_crypto_operation(NX_CRYPTO_DH_SETUP,
                                                       NX_CRYPTO_NULL,
                                                       crypto_method_ecdh,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       NX_CRYPTO_NULL,
                                                       (UCHAR *)&extended_output,
                                                       sizeof(extended_output),
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }
    local_public_key_len = extended_output.nx_crypto_extended_output_actual_size;

    /* Calculate shared secret using the test public key.  */
    extended_output.nx_crypto_extended_output_data = shared_secret;
    extended_output.nx_crypto_extended_output_length_in_byte = sizeof(shared_secret);
    status = crypto_method_ecdh -> nx_crypto_operation(NX_CRYPTO_DH_CALCULATE,
                                                       NX_CRYPTO_NULL,
                                                       crypto_method_ecdh,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       test_public_key,
                                                       test_public_key_len,
                                                       NX_CRYPTO_NULL,
                                                       (UCHAR *)&extended_output,
                                                       sizeof(extended_output),
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }
    shared_secret_len = extended_output.nx_crypto_extended_output_actual_size;

    if (crypto_method_ecdh -> nx_crypto_cleanup)
    {
        status = crypto_method_ecdh -> nx_crypto_cleanup(metadata);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Verify.  */
    /* Call the crypto initialization function.  */
    if (crypto_method_ecdh -> nx_crypto_init)
    {
        status = crypto_method_ecdh -> nx_crypto_init(crypto_method_ecdh,
                                                      NX_CRYPTO_NULL,
                                                      0,
                                                      NX_CRYPTO_NULL,
                                                      metadata,
                                                      metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Set EC curve.  */
    status = crypto_method_ecdh -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET,
                                                       NX_CRYPTO_NULL,
                                                       crypto_method_ecdh,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       (UCHAR *)curve_method,
                                                       sizeof(NX_CRYPTO_METHOD *),
                                                       NX_CRYPTO_NULL,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Import the test private key.  */
    status = crypto_method_ecdh -> nx_crypto_operation(NX_CRYPTO_DH_KEY_PAIR_IMPORT,
                                                       NX_CRYPTO_NULL,
                                                       crypto_method_ecdh,
                                                       test_private_key,
                                                       (NX_CRYPTO_KEY_SIZE)(test_private_key_len << 3),
                                                       test_public_key,
                                                       test_public_key_len,
                                                       NX_CRYPTO_NULL,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Calculate the shared secret using the local public key.  */
    extended_output.nx_crypto_extended_output_data = output;
    extended_output.nx_crypto_extended_output_length_in_byte = sizeof(output);
    status = crypto_method_ecdh -> nx_crypto_operation(NX_CRYPTO_DH_CALCULATE,
                                                       NX_CRYPTO_NULL,
                                                       crypto_method_ecdh,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       local_public_key,
                                                       local_public_key_len,
                                                       NX_CRYPTO_NULL,
                                                       (UCHAR *)&extended_output,
                                                       sizeof(extended_output),
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Validate the output.  */
    if (extended_output.nx_crypto_extended_output_actual_size != shared_secret_len)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if(NX_CRYPTO_MEMCMP(output, shared_secret, extended_output.nx_crypto_extended_output_actual_size) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_ecdh -> nx_crypto_cleanup)
    {
        status = crypto_method_ecdh -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
