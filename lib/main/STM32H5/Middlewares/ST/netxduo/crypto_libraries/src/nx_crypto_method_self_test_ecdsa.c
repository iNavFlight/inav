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
#include "nx_crypto_ecdsa.h"

#ifdef NX_CRYPTO_SELF_TEST

extern NX_CRYPTO_METHOD crypto_method_ec_secp256;
extern NX_CRYPTO_METHOD crypto_method_sha256;

static UCHAR msg_p256_sha256[] = {
0xdb, 0x34, 0x4a, 0x78, 0x9b, 0x77, 0xde, 0x2e, 0x52, 0x75, 0xa9, 0xaf, 0x0e, 0xe6, 0x47, 0xff,
0x87, 0x86, 0x6f, 0x3b, 0x66, 0xdd, 0x89, 0x1e, 0x55, 0x80, 0x18, 0xed, 0xec, 0x70, 0x3b, 0xde,
0x9f, 0x7b, 0x4a, 0xc6, 0xc2, 0xc3, 0xe9, 0xdc, 0x14, 0x8e, 0x8b, 0xb8, 0x97, 0x4a, 0x21, 0x1f,
0x76, 0x8e, 0x43, 0xa9, 0x29, 0x86, 0xc1, 0x6f, 0x8f, 0xc3, 0x5a, 0xe9, 0xca, 0x9c, 0x14, 0x61,
0xb6, 0x8d, 0xab, 0x06, 0x1e, 0x97, 0x93, 0x55, 0xf8, 0x82, 0x8e, 0xeb, 0xfc, 0xd5, 0xa3, 0x62,
0xf7, 0x10, 0x87, 0xb4, 0x30, 0xd3, 0x5e, 0xd4, 0x70, 0xf8, 0xaa, 0x5c, 0x12, 0xbd, 0xde, 0xef,
0x83, 0xb9, 0xc3, 0x24, 0x48, 0x7c, 0x3b, 0x6d, 0x07, 0x51, 0xd9, 0xca, 0x7e, 0x9f, 0xb8, 0x1e,
0xda, 0x17, 0xea, 0xa6, 0xf9, 0x39, 0xeb, 0x42, 0x5f, 0xf3, 0x48, 0x96, 0xb6, 0xad, 0xa9, 0x75,
};

/* Output. */
static UCHAR output[400];
static UCHAR scratch_buffer[4000];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test_ecdsa                    PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for ECDSA crypto       */
/*    method.                                                             */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_self_test_ecdsa(NX_CRYPTO_METHOD *crypto_method_ecdsa,
                                                      VOID *metadata, UINT metadata_size)
{
UINT                  status;
VOID                 *handler = NX_CRYPTO_NULL;
NX_CRYPTO_HUGE_NUMBER private_key;
NX_CRYPTO_EC_POINT    public_key;
UCHAR                *privkey;
UCHAR                *pubkey;
UINT                  pubkey_length;
NX_CRYPTO_METHOD     *curve_method;
NX_CRYPTO_METHOD     *hash_method;
UINT                  buffer_size;
UINT                  curve_size;
HN_UBASE             *scratch;
NX_CRYPTO_EC         *curve = NX_CRYPTO_NULL;
UCHAR                *msg;
UINT                  msg_length;
ULONG                 sig_length;
NX_CRYPTO_EXTENDED_OUTPUT extended_output;


    /* Validate the crypto method */
    if(crypto_method_ecdsa == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    /* Set the test data.  */
    curve_method = &crypto_method_ec_secp256;
    hash_method = &crypto_method_sha256;
    msg = msg_p256_sha256;
    msg_length = sizeof(msg_p256_sha256);

    /* Clear the output buffer.  */
    NX_CRYPTO_MEMSET(output, 0, sizeof(output));

    /* Call the crypto initialization function.  */
    if (crypto_method_ecdsa -> nx_crypto_init)
    {
        status = crypto_method_ecdsa -> nx_crypto_init(crypto_method_ecdsa,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       &handler,
                                                       metadata,
                                                       metadata_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_ecdsa -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Set hash method.  */
    status = crypto_method_ecdsa -> nx_crypto_operation(NX_CRYPTO_HASH_METHOD_SET,
                                                        handler,
                                                        crypto_method_ecdsa,
                                                        NX_CRYPTO_NULL,
                                                        0,
                                                        (UCHAR *)hash_method,
                                                        sizeof(NX_CRYPTO_METHOD *),
                                                        NX_CRYPTO_NULL,
                                                        NX_CRYPTO_NULL,
                                                        0,
                                                        metadata,
                                                        metadata_size,
                                                        NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Set EC curve.  */
    status = crypto_method_ecdsa -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET,
                                                        handler,
                                                        crypto_method_ecdsa,
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

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Get EC curve.  */
    status = curve_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_GET,
                                                 NX_CRYPTO_NULL,
                                                 curve_method,
                                                 NX_CRYPTO_NULL,
                                                 0,
                                                 NX_CRYPTO_NULL,
                                                 0,
                                                 NX_CRYPTO_NULL,
                                                 (UCHAR *)&curve,
                                                 0,
                                                 NX_CRYPTO_NULL,
                                                 0,
                                                 NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;
    curve_size = curve -> nx_crypto_ec_bits >> 3;
    if (curve -> nx_crypto_ec_bits & 7)
    {
        curve_size++;
    }

    scratch = (HN_UBASE*)(&scratch_buffer[3 * buffer_size + 4]);
    privkey = scratch_buffer;
    pubkey = &scratch_buffer[buffer_size];
    NX_CRYPTO_EC_POINT_INITIALIZE(&public_key, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&private_key, scratch, buffer_size + 8);

    /* Generate the key pair. */
    status = _nx_crypto_ec_key_pair_generation_extra(curve, &curve -> nx_crypto_ec_g, &private_key,
                                                     &public_key, scratch);

    if (status)
    {
        return(status);
    }


    status = _nx_crypto_huge_number_extract_fixed_size(&private_key, privkey, buffer_size);
    if (status)
    {
        return(status);
    }

    pubkey_length = 0;
    _nx_crypto_ec_point_extract_uncompressed(curve, &public_key, pubkey, 4 + 2 * buffer_size, &pubkey_length);


    extended_output.nx_crypto_extended_output_data = output;
    extended_output.nx_crypto_extended_output_length_in_byte = sizeof(output);
    /* Sign the hash data using ECDSA. */
    status = crypto_method_ecdsa -> nx_crypto_operation(NX_CRYPTO_SIGNATURE_GENERATE,
                                                        handler,
                                                        crypto_method_ecdsa,
                                                        privkey,
                                                        buffer_size << 3,
                                                        msg,
                                                        msg_length,
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
    sig_length = extended_output.nx_crypto_extended_output_actual_size;

    /* Verify the signature. */
    status = crypto_method_ecdsa -> nx_crypto_operation(NX_CRYPTO_SIGNATURE_VERIFY,
                                                        handler,
                                                        crypto_method_ecdsa,
                                                        pubkey,
                                                        pubkey_length << 3,
                                                        msg,
                                                        msg_length,
                                                        NX_CRYPTO_NULL,
                                                        output,
                                                        sig_length,
                                                        metadata,
                                                        metadata_size,
                                                        NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (crypto_method_ecdsa -> nx_crypto_cleanup)
    {
        status = crypto_method_ecdsa -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
