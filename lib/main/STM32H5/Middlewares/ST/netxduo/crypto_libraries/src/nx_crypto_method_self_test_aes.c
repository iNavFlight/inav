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

#define INPUT_OUTPUT_LENGTH (16)
/* 02CF1E1D9796AD3395D8126757B6FC7C705F082FB85270427760AC173759CE07 */
static const UCHAR key_cbc_256_encrypt[] = {
0x02, 0xCF, 0x1E, 0x1D, 0x97, 0x96, 0xAD, 0x33, 0x95, 0xD8, 0x12, 0x67, 0x57, 0xB6, 0xFC, 0x7C, 
0x70, 0x5F, 0x08, 0x2F, 0xB8, 0x52, 0x70, 0x42, 0x77, 0x60, 0xAC, 0x17, 0x37, 0x59, 0xCE, 0x07, 
};

/* BB35190002242F28FC79C01B89022604 */
static const UCHAR iv_cbc_256_encrypt[] = {
0xBB, 0x35, 0x19, 0x00, 0x02, 0x24, 0x2F, 0x28, 0xFC, 0x79, 0xC0, 0x1B, 0x89, 0x02, 0x26, 0x04, 
};

/* 05D4EE7191E69A0AC7CBC1625258085D */
static const UCHAR plain_cbc_256_encrypt[] = {
0x05, 0xD4, 0xEE, 0x71, 0x91, 0xE6, 0x9A, 0x0A, 0xC7, 0xCB, 0xC1, 0x62, 0x52, 0x58, 0x08, 0x5D, 
};

/* F65F0278E9F5D525F66DC5AFAD99F205 */
static const UCHAR secret_cbc_256_encrypt[] = {
0xF6, 0x5F, 0x02, 0x78, 0xE9, 0xF5, 0xD5, 0x25, 0xF6, 0x6D, 0xC5, 0xAF, 0xAD, 0x99, 0xF2, 0x05, 
};

/* 6CB931425732CB1F813B2F3B648A13326942421EA6BAB74E6F774764AE85541D */
static UCHAR key_cbc_256_decrypt[] = {
0x6C, 0xB9, 0x31, 0x42, 0x57, 0x32, 0xCB, 0x1F, 0x81, 0x3B, 0x2F, 0x3B, 0x64, 0x8A, 0x13, 0x32, 
0x69, 0x42, 0x42, 0x1E, 0xA6, 0xBA, 0xB7, 0x4E, 0x6F, 0x77, 0x47, 0x64, 0xAE, 0x85, 0x54, 0x1D, 
};

/* CDA29622B5C38C785784EC74E6F8BA31 */
static UCHAR iv_cbc_256_decrypt[] = {
0xCD, 0xA2, 0x96, 0x22, 0xB5, 0xC3, 0x8C, 0x78, 0x57, 0x84, 0xEC, 0x74, 0xE6, 0xF8, 0xBA, 0x31, 
};

/* F19B9D43CE897F2577D10C60C82E1E17 */
static UCHAR plain_cbc_256_decrypt[] = {
0xF1, 0x9B, 0x9D, 0x43, 0xCE, 0x89, 0x7F, 0x25, 0x77, 0xD1, 0x0C, 0x60, 0xC8, 0x2E, 0x1E, 0x17, 
};

/* 3CBB285FB0244F551DEEF7C0DDBE5BFE */
static UCHAR secret_cbc_256_decrypt[] = {
0x3C, 0xBB, 0x28, 0x5F, 0xB0, 0x24, 0x4F, 0x55, 0x1D, 0xEE, 0xF7, 0xC0, 0xDD, 0xBE, 0x5B, 0xFE, 
};

/*Note: For CTR, the key_ctr is the conjunction of key and nonce.  */
/* D0E78C4D0B30D33F5BF4A132B2F94A4A38963511A3904B117E35A37B5AAC8A193BF0D158 */
static const UCHAR key_ctr_256_encrypt[] = {
0xD0, 0xE7, 0x8C, 0x4D, 0x0B, 0x30, 0xD3, 0x3F, 0x5B, 0xF4, 0xA1, 0x32, 0xB2, 0xF9, 0x4A, 0x4A, 
0x38, 0x96, 0x35, 0x11, 0xA3, 0x90, 0x4B, 0x11, 0x7E, 0x35, 0xA3, 0x7B, 0x5A, 0xAC, 0x8A, 0x19, 
0x3B, 0xF0, 0xD1, 0x58, 
};

/* A1A31704C8B7E16C */
static const UCHAR iv_ctr_256_encrypt[] = {
0xA1, 0xA3, 0x17, 0x04, 0xC8, 0xB7, 0xE1, 0x6C,
};

/* 981FA33222C5451017530155A4BF7F29 */
static const UCHAR plain_ctr_256_encrypt[] = {
0x98, 0x1F, 0xA3, 0x32, 0x22, 0xC5, 0x45, 0x10, 0x17, 0x53, 0x01, 0x55, 0xA4, 0xBF, 0x7F, 0x29, 
};

/* 643B91B4E541B20AAAEAB77F2D328566 */
static const UCHAR secret_ctr_256_encrypt[] = {
0x64, 0x3B, 0x91, 0xB4, 0xE5, 0x41, 0xB2, 0x0A, 0xAA, 0xEA, 0xB7, 0x7F, 0x2D, 0x32, 0x85, 0x66, 
};

/* D1C30E262F0D457A4B1F9B0B5753084793C5A627776D41430B152E6D195CA276 */
static UCHAR key_ctr_256_decrypt[] = {
0xD1, 0xC3, 0x0E, 0x26, 0x2F, 0x0D, 0x45, 0x7A, 0x4B, 0x1F, 0x9B, 0x0B, 0x57, 0x53, 0x08, 0x47, 
0x93, 0xC5, 0xA6, 0x27, 0x77, 0x6D, 0x41, 0x43, 0x0B, 0x15, 0x2E, 0x6D, 0x19, 0x5C, 0xA2, 0x76, 
};

/* 65F42E5B2D9EEC75DEF7017500000001 */
static UCHAR iv_ctr_256_decrypt[] = {
0x65, 0xF4, 0x2E, 0x5B, 0x2D, 0x9E, 0xEC, 0x75, 0xDE, 0xF7, 0x01, 0x75, 0x00, 0x00, 0x00, 0x01, 
};

/* F0D0D405C53C6B67200B485B6219BB22 */
static UCHAR plain_ctr_256_decrypt[] = {
0xF0, 0xD0, 0xD4, 0x05, 0xC5, 0x3C, 0x6B, 0x67, 0x20, 0x0B, 0x48, 0x5B, 0x62, 0x19, 0xBB, 0x22, 
};

/* 8EF64BF7EAF4390BB42D3B3B5D3611D3 */
static UCHAR secret_ctr_256_decrypt[] = {
0x8E, 0xF6, 0x4B, 0xF7, 0xEA, 0xF4, 0x39, 0x0B, 0xB4, 0x2D, 0x3B, 0x3B, 0x5D, 0x36, 0x11, 0xD3, 
};

static UCHAR output[INPUT_OUTPUT_LENGTH];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_method_self_test_aes                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for AES crypto method. */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_self_test_aes(NX_CRYPTO_METHOD *crypto_method_aes,
                                                    VOID *metadata, UINT metadata_size)
{
UCHAR *key_encrypt, *key_decrypt;
UCHAR *iv_encrypt, *iv_decrypt;
UCHAR *plain_encrypt, *plain_decrypt;
UCHAR *secret_encrypt, *secret_decrypt;
UINT key_size;
UINT status;
VOID *handler = NX_CRYPTO_NULL;


    /* Validate the crypto method */
    if(crypto_method_aes == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    key_size = crypto_method_aes -> nx_crypto_key_size_in_bits;

    switch (crypto_method_aes -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_ENCRYPTION_AES_CBC:
        switch (key_size)
        {
        case 256:
            key_encrypt = (UCHAR *)key_cbc_256_encrypt;
            iv_encrypt = (UCHAR *)iv_cbc_256_encrypt;
            plain_encrypt = (UCHAR *)plain_cbc_256_encrypt;
            secret_encrypt = (UCHAR *)secret_cbc_256_encrypt;
            key_decrypt = (UCHAR *)key_cbc_256_decrypt;
            iv_decrypt = (UCHAR *)iv_cbc_256_decrypt;
            plain_decrypt = (UCHAR *)plain_cbc_256_decrypt;
            secret_decrypt = (UCHAR *)secret_cbc_256_decrypt;
            break;
        default:
            return(1);
        }
        break;

    case NX_CRYPTO_ENCRYPTION_AES_CTR:
        switch (key_size)
        {
        case 256:
            key_encrypt = (UCHAR *)key_ctr_256_encrypt;
            iv_encrypt = (UCHAR *)iv_ctr_256_encrypt;
            plain_encrypt = (UCHAR *)plain_ctr_256_encrypt;
            secret_encrypt = (UCHAR *)secret_ctr_256_encrypt;
            key_decrypt = (UCHAR *)key_ctr_256_decrypt;
            iv_decrypt = (UCHAR *)iv_ctr_256_decrypt;
            plain_decrypt = (UCHAR *)plain_ctr_256_decrypt;
            secret_decrypt = (UCHAR *)secret_ctr_256_decrypt;
            break;
        default:
            return(1);
        }
        break;

    default:
        return(1);
    }

    if (crypto_method_aes -> nx_crypto_init)
    {
        status = crypto_method_aes -> nx_crypto_init(crypto_method_aes,
                                                     key_encrypt,
                                                     key_size,
                                                     &handler,
                                                     metadata,
                                                     metadata_size);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_aes -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Encryption. */
    status = crypto_method_aes -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                      handler,
                                                      crypto_method_aes,
                                                      key_encrypt,
                                                      key_size,
                                                      plain_encrypt,
                                                      INPUT_OUTPUT_LENGTH,
                                                      iv_encrypt,
                                                      output,
                                                      INPUT_OUTPUT_LENGTH,
                                                      metadata,
                                                      metadata_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_CRYPTO_MEMCMP(output, secret_encrypt, INPUT_OUTPUT_LENGTH) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_aes -> nx_crypto_cleanup)
    {
        status = crypto_method_aes -> nx_crypto_cleanup(metadata);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Decryption. */
    if (crypto_method_aes -> nx_crypto_init)
    {
        status = crypto_method_aes -> nx_crypto_init(crypto_method_aes,
                                                     key_decrypt,
                                                     key_size,
                                                     &handler,
                                                     metadata,
                                                     metadata_size);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    status = crypto_method_aes -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                      handler,
                                                      crypto_method_aes,
                                                      key_decrypt,
                                                      key_size,
                                                      secret_decrypt,
                                                      INPUT_OUTPUT_LENGTH,
                                                      iv_decrypt,
                                                      output,
                                                      INPUT_OUTPUT_LENGTH,
                                                      metadata,
                                                      metadata_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_CRYPTO_MEMCMP(output, plain_decrypt, INPUT_OUTPUT_LENGTH) != 0)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (crypto_method_aes -> nx_crypto_cleanup)
    {
        status = crypto_method_aes -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif


