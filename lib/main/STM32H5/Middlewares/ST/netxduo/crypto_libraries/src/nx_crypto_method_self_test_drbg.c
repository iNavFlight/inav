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
#include "nx_crypto_drbg.h"
#include "nx_crypto_aes.h"

#ifdef NX_CRYPTO_SELF_TEST

/*
[AES-128 use df]
[PredictionResistance = True]
[EntropyInputLen = 128]
[NonceLen = 64]
[PersonalizationStringLen = 128]
[AdditionalInputLen = 128]
[ReturnedBitsLen = 512]

COUNT = 0
EntropyInput = c87bcfa53ded926698bb792254eba853
Nonce = 1165ebdb6fe78232
PersonalizationString = 7ee8c40ebd3551fe103c8349377ea7cc
AdditionalInput = db2422dba1157857785ce57b43bc938e
EntropyInputPR = da60722bd8d6d4313d47ce07c4f63c29
AdditionalInput = 06b93f6b069e4859a8c76247a4825a64
EntropyInputPR = 933623ef7527bc0449ca8d629d84a91f
ReturnedBits = fd76ace1287a800d01a969fc99d6db8b3f309f6cf5b2b07eec12e3f0169cbbcdebf9a1485a57ed0f5ecc14de122c5eee9c39f48ee8e98e5bb58141e9dc33121e

[AES-128 use df]
[PredictionResistance = False]
[EntropyInputLen = 128]
[NonceLen = 64]
[PersonalizationStringLen = 128]
[AdditionalInputLen = 128]
[ReturnedBitsLen = 512]

COUNT = 0
EntropyInput = 63f226b4a964681df99ac1965cecac3a
Nonce = d888574290239f93
PersonalizationString = d0ef2ed4a740a78ee06b461758a67912
EntropyInputReseed = 5d093a54a1201f46ab562e8abf77fa8f
AdditionalInputReseed = 39232ea4871483ae596a8fa56fec03d6
AdditionalInput = b8f5e41effe52e62f2a258c9e29ad5b3
AdditionalInput = bb8fd9404da2c601b0209e0f4b1a5094
ReturnedBits = 22e3066c23bbaae85b118dffc21c3572dcbede7a7c7a7c73bcdee8a060500ef67af6c13cee6f4da309f8b3e3fa8dc4c37f94d99b591a1cede6aec30b200c81f7
*/

static UCHAR entropy_input_aes128[] = {
0xc8, 0x7b, 0xcf, 0xa5, 0x3d, 0xed, 0x92, 0x66, 0x98, 0xbb, 0x79, 0x22, 0x54, 0xeb, 0xa8, 0x53 
};
static UINT entropy_input_len_aes128 = 16;
static UCHAR nonce_aes128[] = {
0x11, 0x65, 0xeb, 0xdb, 0x6f, 0xe7, 0x82, 0x32 };
static UCHAR personalization_string_aes128[] = {
0x7e, 0xe8, 0xc4, 0x0e, 0xbd, 0x35, 0x51, 0xfe, 0x10, 0x3c, 0x83, 0x49, 0x37, 0x7e, 0xa7, 0xcc 
};
static UCHAR additional_input_0_aes128[] = {
0xdb, 0x24, 0x22, 0xdb, 0xa1, 0x15, 0x78, 0x57, 0x78, 0x5c, 0xe5, 0x7b, 0x43, 0xbc, 0x93, 0x8e 
};
static UCHAR entropy_input_pr_0_aes128[] = {
0xda, 0x60, 0x72, 0x2b, 0xd8, 0xd6, 0xd4, 0x31, 0x3d, 0x47, 0xce, 0x07, 0xc4, 0xf6, 0x3c, 0x29 
};
static UCHAR additional_input_1_aes128[] = {
0x06, 0xb9, 0x3f, 0x6b, 0x06, 0x9e, 0x48, 0x59, 0xa8, 0xc7, 0x62, 0x47, 0xa4, 0x82, 0x5a, 0x64 
};
static UCHAR entropy_input_pr_1_aes128[] = {
0x93, 0x36, 0x23, 0xef, 0x75, 0x27, 0xbc, 0x04, 0x49, 0xca, 0x8d, 0x62, 0x9d, 0x84, 0xa9, 0x1f 
};
static UCHAR returned_bits_aes128[] = {
0xfd, 0x76, 0xac, 0xe1, 0x28, 0x7a, 0x80, 0x0d, 0x01, 0xa9, 0x69, 0xfc, 0x99, 0xd6, 0xdb, 0x8b, 
0x3f, 0x30, 0x9f, 0x6c, 0xf5, 0xb2, 0xb0, 0x7e, 0xec, 0x12, 0xe3, 0xf0, 0x16, 0x9c, 0xbb, 0xcd, 
0xeb, 0xf9, 0xa1, 0x48, 0x5a, 0x57, 0xed, 0x0f, 0x5e, 0xcc, 0x14, 0xde, 0x12, 0x2c, 0x5e, 0xee, 
0x9c, 0x39, 0xf4, 0x8e, 0xe8, 0xe9, 0x8e, 0x5b, 0xb5, 0x81, 0x41, 0xe9, 0xdc, 0x33, 0x12, 0x1e
};

static UCHAR entropy_input_npr_aes128[] = {
0x63, 0xf2, 0x26, 0xb4, 0xa9, 0x64, 0x68, 0x1d, 0xf9, 0x9a, 0xc1, 0x96, 0x5c, 0xec, 0xac, 0x3a
};
static UINT entropy_input_len_npr_aes128 = 16;
static UCHAR nonce_npr_aes128[] = {
0xd8, 0x88, 0x57, 0x42, 0x90, 0x23, 0x9f, 0x93
};
static UCHAR personalization_string_npr_aes128[] = {
0xd0, 0xef, 0x2e, 0xd4, 0xa7, 0x40, 0xa7, 0x8e, 0xe0, 0x6b, 0x46, 0x17, 0x58, 0xa6, 0x79, 0x12
};
static UCHAR entropy_input_reseed_npr_aes128[] = {
0x5d, 0x09, 0x3a, 0x54, 0xa1, 0x20, 0x1f, 0x46, 0xab, 0x56, 0x2e, 0x8a, 0xbf, 0x77, 0xfa, 0x8f
};
static UCHAR additional_input_reseed_npr_aes128[] = {
0x39, 0x23, 0x2e, 0xa4, 0x87, 0x14, 0x83, 0xae, 0x59, 0x6a, 0x8f, 0xa5, 0x6f, 0xec, 0x03, 0xd6
};
static UCHAR additional_input_0_npr_aes128[] = {
0xb8, 0xf5, 0xe4, 0x1e, 0xff, 0xe5, 0x2e, 0x62, 0xf2, 0xa2, 0x58, 0xc9, 0xe2, 0x9a, 0xd5, 0xb3
};
static UCHAR additional_input_1_npr_aes128[] = {
0xbb, 0x8f, 0xd9, 0x40, 0x4d, 0xa2, 0xc6, 0x01, 0xb0, 0x20, 0x9e, 0x0f, 0x4b, 0x1a, 0x50, 0x94
};
static UCHAR returned_bits_npr_aes128[] = {
0x22, 0xe3, 0x06, 0x6c, 0x23, 0xbb, 0xaa, 0xe8, 0x5b, 0x11, 0x8d, 0xff, 0xc2, 0x1c, 0x35, 0x72,
0xdc, 0xbe, 0xde, 0x7a, 0x7c, 0x7a, 0x7c, 0x73, 0xbc, 0xde, 0xe8, 0xa0, 0x60, 0x50, 0x0e, 0xf6,
0x7a, 0xf6, 0xc1, 0x3c, 0xee, 0x6f, 0x4d, 0xa3, 0x09, 0xf8, 0xb3, 0xe3, 0xfa, 0x8d, 0xc4, 0xc3,
0x7f, 0x94, 0xd9, 0x9b, 0x59, 0x1a, 0x1c, 0xed, 0xe6, 0xae, 0xc3, 0x0b, 0x20, 0x0c, 0x81, 0xf7
};

/* Output.  */
static UCHAR output[64];
static NX_CRYPTO_AES aes_metadata;

/* Global variable.  */
static UCHAR drbg_test_entropy_count_pr;
static UCHAR drbg_test_entropy_count_npr;

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    drbg_test_get_entropy_pr                            PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function serves an entrpoy source for the DRBG module, for the */
/*    purpose of the self test.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    entropy                               Pointer to the buffer for     */
/*                                            the value to be returned    */
/*    entropy_len                           Number of valid bytes written */
/*    entrypy_max_len                       Size of the input buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMCPY                                                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    DRBG internal logic                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static UINT drbg_test_get_entropy_pr(UCHAR *entropy, UINT *entropy_len, UINT entropy_max_len)
{
    if (entropy_input_len_aes128 < *entropy_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }
    
    if (entropy_input_len_aes128 > entropy_max_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    if (drbg_test_entropy_count_pr == 0)
    {
        NX_CRYPTO_MEMCPY(entropy, entropy_input_aes128, entropy_input_len_aes128); /* Use case of memcpy is verified. */
        *entropy_len = entropy_input_len_aes128;
    }
    else if (drbg_test_entropy_count_pr == 1)
    {
        NX_CRYPTO_MEMCPY(entropy, entropy_input_pr_0_aes128, entropy_input_len_aes128); /* Use case of memcpy is verified. */
        *entropy_len = entropy_input_len_aes128;
    }
    else if (drbg_test_entropy_count_pr == 2)
    {
        NX_CRYPTO_MEMCPY(entropy, entropy_input_pr_1_aes128, entropy_input_len_aes128); /* Use case of memcpy is verified. */
        *entropy_len = entropy_input_len_aes128;
    }
    else
    {
        return(1);
    }

    drbg_test_entropy_count_pr++;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    drbg_test_get_entropy_npr                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function serves an entrpoy source for the DRBG module, for the */
/*    purpose of the self test.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    entropy                               Pointer to the buffer for     */
/*                                            the value to be returned    */
/*    entropy_len                           Number of valid bytes written */
/*    entrypy_max_len                       Size of the input buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMCPY                                                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    DRBG internal logic                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static UINT drbg_test_get_entropy_npr(UCHAR *entropy, UINT *entropy_len, UINT entropy_max_len)
{
    if (entropy_input_len_npr_aes128 < *entropy_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    if (entropy_input_len_npr_aes128 > entropy_max_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    if (drbg_test_entropy_count_npr == 0)
    {
        NX_CRYPTO_MEMCPY(entropy, entropy_input_npr_aes128, entropy_input_len_npr_aes128); /* Use case of memcpy is verified. */
        *entropy_len = entropy_input_len_npr_aes128;
    }
    else if (drbg_test_entropy_count_npr == 1)
    {
        NX_CRYPTO_MEMCPY(entropy, entropy_input_reseed_npr_aes128, entropy_input_len_npr_aes128); /* Use case of memcpy is verified. */
        *entropy_len = entropy_input_len_npr_aes128;
    }
    else
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    drbg_test_entropy_count_npr++;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test_drbg                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for DRBG crypto method.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method_ptr                            Pointer to the crypto method  */
/*                                            to be tested.               */
/*    metadata                              Metadata area required by     */
/*                                            the DRBG module             */
/*    metadata_size                         Size of the metadata area     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMCPY                                                    */
/*    NX_CRYPTO_MEMCMP                                                    */
/*                                                                        */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_self_test_drbg(NX_CRYPTO_METHOD *crypto_method_drbg,
                                                     VOID *metadata, UINT metadata_size)
{
NX_CRYPTO_DRBG_OPTIONS   drbg_opt;
UCHAR                   *personalization_string;
UINT                     personalization_string_len;
UCHAR                   *additional_input[2];
UINT                     additional_input_len;
UCHAR                   *nonce;
UINT                     nonce_len;
UCHAR                   *returned_bits;
UINT                     returned_bits_len;
UINT                     status;
VOID                    *handler = NX_CRYPTO_NULL;


    /* Validate the crypto method */
    if(crypto_method_drbg == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    /* Set the test data.  */
    drbg_opt.crypto_method = &crypto_method_aes_cbc_128;
    drbg_opt.crypto_metadata = (UCHAR *)&aes_metadata;
    drbg_opt.entropy_input = drbg_test_get_entropy_pr;
    drbg_opt.use_df = 1;
    drbg_opt.prediction_resistance = 1;
    drbg_opt.security_strength = entropy_input_len_aes128;

    personalization_string = personalization_string_aes128;
    personalization_string_len = sizeof(personalization_string_aes128);
    nonce = nonce_aes128;
    nonce_len = sizeof(nonce_aes128);
    additional_input[0] = additional_input_0_aes128;
    additional_input[1] = additional_input_1_aes128;
    additional_input_len = sizeof(additional_input_0_aes128);
    returned_bits = returned_bits_aes128;
    returned_bits_len = sizeof(returned_bits_aes128);

    /* Clear the output buffer.  */
    NX_CRYPTO_MEMSET(output, 0, sizeof(output));
    drbg_test_entropy_count_pr = 0;

    /* Call the crypto initialization function.  */
    if (crypto_method_drbg -> nx_crypto_init)
    {
        status = crypto_method_drbg -> nx_crypto_init(crypto_method_drbg,
                                                      NX_CRYPTO_NULL,
                                                      0,
                                                      &handler,
                                                      metadata,
                                                      metadata_size);

        /* Check the status.  */
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_drbg -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_OPTIONS_SET,
                                                       handler,
                                                       crypto_method_drbg,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       (UCHAR *)&drbg_opt,
                                                       sizeof(drbg_opt),
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

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_INSTANTIATE,
                                                       handler,
                                                       crypto_method_drbg,
                                                       nonce,
                                                       nonce_len << 3,
                                                       personalization_string,
                                                       personalization_string_len,
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

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_GENERATE,
                                                       handler,
                                                       crypto_method_drbg,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       additional_input[0],
                                                       additional_input_len,
                                                       NX_CRYPTO_NULL,
                                                       output,
                                                       returned_bits_len,
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_GENERATE,
                                                       handler,
                                                       crypto_method_drbg,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       additional_input[1],
                                                       additional_input_len,
                                                       NX_CRYPTO_NULL,
                                                       output,
                                                       returned_bits_len,
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Validate the output.  */
    if(NX_CRYPTO_MEMCMP(output, returned_bits, returned_bits_len) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_drbg -> nx_crypto_cleanup)
    {
        status = crypto_method_drbg -> nx_crypto_cleanup(metadata);
    }

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }


    /* Set the test data for reseed.  */
    drbg_opt.crypto_method = &crypto_method_aes_cbc_128;
    drbg_opt.crypto_metadata = (UCHAR *)&aes_metadata;
    drbg_opt.entropy_input = drbg_test_get_entropy_npr;
    drbg_opt.use_df = 1;
    drbg_opt.prediction_resistance = 0;
    drbg_opt.security_strength = entropy_input_len_npr_aes128;

    /* Clear the output buffer.  */
    NX_CRYPTO_MEMSET(output, 0, sizeof(output));
    drbg_test_entropy_count_npr = 0;

    /* Call the crypto initialization function.  */
    if (crypto_method_drbg -> nx_crypto_init)
    {
        status = crypto_method_drbg -> nx_crypto_init(crypto_method_drbg,
                                                      NX_CRYPTO_NULL,
                                                      0,
                                                      &handler,
                                                      metadata,
                                                      metadata_size);

        /* Check the status.  */
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_drbg -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_OPTIONS_SET,
                                                       handler,
                                                       crypto_method_drbg,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       (UCHAR *)&drbg_opt,
                                                       sizeof(drbg_opt),
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

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_INSTANTIATE,
                                                       handler,
                                                       crypto_method_drbg,
                                                       nonce_npr_aes128,
                                                       sizeof(nonce_npr_aes128) << 3,
                                                       personalization_string_npr_aes128,
                                                       sizeof(personalization_string_npr_aes128),
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

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_RESEED,
                                                       handler,
                                                       crypto_method_drbg,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       additional_input_reseed_npr_aes128,
                                                       sizeof(additional_input_reseed_npr_aes128),
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

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_GENERATE,
                                                       handler,
                                                       crypto_method_drbg,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       additional_input_0_npr_aes128,
                                                       sizeof(additional_input_0_npr_aes128),
                                                       NX_CRYPTO_NULL,
                                                       output,
                                                       sizeof(returned_bits_npr_aes128),
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Call the crypto operation function.  */
    status = crypto_method_drbg -> nx_crypto_operation(NX_CRYPTO_DRBG_GENERATE,
                                                       handler,
                                                       crypto_method_drbg,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       additional_input_1_npr_aes128,
                                                       sizeof(additional_input_1_npr_aes128),
                                                       NX_CRYPTO_NULL,
                                                       output,
                                                       sizeof(returned_bits_npr_aes128),
                                                       metadata,
                                                       metadata_size,
                                                       NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Validate the output.  */
    if(NX_CRYPTO_MEMCMP(output, returned_bits_npr_aes128, sizeof(returned_bits_npr_aes128)) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_drbg -> nx_crypto_cleanup)
    {
        status = crypto_method_drbg -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
