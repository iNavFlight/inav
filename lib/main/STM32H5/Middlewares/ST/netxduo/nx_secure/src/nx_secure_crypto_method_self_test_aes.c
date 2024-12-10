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

#define INPUT_OUTPUT_LENGTH (16)

/* A42E843B22E6CC7C09AD783FACE7F251 */
static const UCHAR key_cbc_128[] = {
0xA4, 0x2E, 0x84, 0x3B, 0x22, 0xE6, 0xCC, 0x7C, 0x09, 0xAD, 0x78, 0x3F, 0xAC, 0xE7, 0xF2, 0x51, 
};

/* 87BFAE15B37B90751B426240F5BC0C16 */
static const UCHAR iv_cbc_128[] = {
0x87, 0xBF, 0xAE, 0x15, 0xB3, 0x7B, 0x90, 0x75, 0x1B, 0x42, 0x62, 0x40, 0xF5, 0xBC, 0x0C, 0x16, 
};

/* 997F8E2A478FF2479A83B90BD44EDE5C */
static const UCHAR plain_cbc_128[] = {
0x99, 0x7F, 0x8E, 0x2A, 0x47, 0x8F, 0xF2, 0x47, 0x9A, 0x83, 0xB9, 0x0B, 0xD4, 0x4E, 0xDE, 0x5C, 
};

/* 24E38770FF9397AFA5A5F7F0689610EF */
static const UCHAR secret_cbc_128[] = {
0x24, 0xE3, 0x87, 0x70, 0xFF, 0x93, 0x97, 0xAF, 0xA5, 0xA5, 0xF7, 0xF0, 0x68, 0x96, 0x10, 0xEF, 
};

/* DCF688403AB1C0079D3FCC1B046398675FAF971E50A92525 */
static const UCHAR key_cbc_192[] = {
0xDC, 0xF6, 0x88, 0x40, 0x3A, 0xB1, 0xC0, 0x07, 0x9D, 0x3F, 0xCC, 0x1B, 0x04, 0x63, 0x98, 0x67, 
0x5F, 0xAF, 0x97, 0x1E, 0x50, 0xA9, 0x25, 0x25, };

/* 78E45F7E3868534D38B45B7E8F549243 */
static const UCHAR iv_cbc_192[] = {
0x78, 0xE4, 0x5F, 0x7E, 0x38, 0x68, 0x53, 0x4D, 0x38, 0xB4, 0x5B, 0x7E, 0x8F, 0x54, 0x92, 0x43, 
};

/* 178BB520EBE2D330539EB315A5EDE75B */
static const UCHAR plain_cbc_192[] = {
0x17, 0x8B, 0xB5, 0x20, 0xEB, 0xE2, 0xD3, 0x30, 0x53, 0x9E, 0xB3, 0x15, 0xA5, 0xED, 0xE7, 0x5B, 
};

/* 58266726FD6E7E9FA64501681B73FE88 */
static const UCHAR secret_cbc_192[] = {
0x58, 0x26, 0x67, 0x26, 0xFD, 0x6E, 0x7E, 0x9F, 0xA6, 0x45, 0x01, 0x68, 0x1B, 0x73, 0xFE, 0x88, 
};

static const UCHAR key_cbc_256[] = {
0x02, 0xCF, 0x1E, 0x1D, 0x97, 0x96, 0xAD, 0x33, 0x95, 0xD8, 0x12, 0x67, 0x57, 0xB6, 0xFC, 0x7C, 
0x70, 0x5F, 0x08, 0x2F, 0xB8, 0x52, 0x70, 0x42, 0x77, 0x60, 0xAC, 0x17, 0x37, 0x59, 0xCE, 0x07, 
};

/* BB35190002242F28FC79C01B89022604 */
static const UCHAR iv_cbc_256[] = {
0xBB, 0x35, 0x19, 0x00, 0x02, 0x24, 0x2F, 0x28, 0xFC, 0x79, 0xC0, 0x1B, 0x89, 0x02, 0x26, 0x04, 
};

/* 05D4EE7191E69A0AC7CBC1625258085D */
static const UCHAR plain_cbc_256[] = {
0x05, 0xD4, 0xEE, 0x71, 0x91, 0xE6, 0x9A, 0x0A, 0xC7, 0xCB, 0xC1, 0x62, 0x52, 0x58, 0x08, 0x5D, 
};

/* F65F0278E9F5D525F66DC5AFAD99F205 */
static const UCHAR secret_cbc_256[] = {
0xF6, 0x5F, 0x02, 0x78, 0xE9, 0xF5, 0xD5, 0x25, 0xF6, 0x6D, 0xC5, 0xAF, 0xAD, 0x99, 0xF2, 0x05, 
};


/*Note: For CTR, the key_ctr_128, key_ctr_192 and key_ctr_256 is the conjunction of key and nonce.  */
/* 3982F50DA5D5D9587EE8FD7078F2BB309C19BD6C */
static const UCHAR key_ctr_128[] = {
0x39, 0x82, 0xF5, 0x0D, 0xA5, 0xD5, 0xD9, 0x58, 0x7E, 0xE8, 0xFD, 0x70, 0x78, 0xF2, 0xBB, 0x30, 
0x9C, 0x19, 0xBD, 0x6C, 
};

/* C3834608D6198033 */
static const UCHAR iv_ctr_128[] = {
0xC3, 0x83, 0x46, 0x08, 0xD6, 0x19, 0x80, 0x33,
};

/* 8854117807D40A6837520939CA38FA2A */
static const UCHAR plain_ctr_128[] = {
0x88, 0x54, 0x11, 0x78, 0x07, 0xD4, 0x0A, 0x68, 0x37, 0x52, 0x09, 0x39, 0xCA, 0x38, 0xFA, 0x2A, 
};

/* 33B8C6FF1ED6A85B4E29AB6585869FEE */
static const UCHAR secret_ctr_128[] = {
0x33, 0xB8, 0xC6, 0xFF, 0x1E, 0xD6, 0xA8, 0x5B, 0x4E, 0x29, 0xAB, 0x65, 0x85, 0x86, 0x9F, 0xEE, 
};

/* AE5DB35787511A77E554FF2258E1A15D62CB7F416E8BDA223E25646D */
static const UCHAR key_ctr_192[] = {
0xAE, 0x5D, 0xB3, 0x57, 0x87, 0x51, 0x1A, 0x77, 0xE5, 0x54, 0xFF, 0x22, 0x58, 0xE1, 0xA1, 0x5D, 
0x62, 0xCB, 0x7F, 0x41, 0x6E, 0x8B, 0xDA, 0x22, 0x3E, 0x25, 0x64, 0x6D, };

/* 0C3605005954FF2A */
static const UCHAR iv_ctr_192[] = {
0x0C, 0x36, 0x05, 0x00, 0x59, 0x54, 0xFF, 0x2A,
};

/* BB924D2DDE863F2C01F2A549C9D06444 */
static const UCHAR plain_ctr_192[] = {
0xBB, 0x92, 0x4D, 0x2D, 0xDE, 0x86, 0x3F, 0x2C, 0x01, 0xF2, 0xA5, 0x49, 0xC9, 0xD0, 0x64, 0x44, 
};

/* ED988DC30B5C6394D21A3F2477836FBB */
static const UCHAR secret_ctr_192[] = {
0xED, 0x98, 0x8D, 0xC3, 0x0B, 0x5C, 0x63, 0x94, 0xD2, 0x1A, 0x3F, 0x24, 0x77, 0x83, 0x6F, 0xBB, 
};

/* D0E78C4D0B30D33F5BF4A132B2F94A4A38963511A3904B117E35A37B5AAC8A193BF0D158 */
static const UCHAR key_ctr_256[] = {
0xD0, 0xE7, 0x8C, 0x4D, 0x0B, 0x30, 0xD3, 0x3F, 0x5B, 0xF4, 0xA1, 0x32, 0xB2, 0xF9, 0x4A, 0x4A, 
0x38, 0x96, 0x35, 0x11, 0xA3, 0x90, 0x4B, 0x11, 0x7E, 0x35, 0xA3, 0x7B, 0x5A, 0xAC, 0x8A, 0x19, 
0x3B, 0xF0, 0xD1, 0x58, 
};

/* A1A31704C8B7E16C */
static const UCHAR iv_ctr_256[] = {
0xA1, 0xA3, 0x17, 0x04, 0xC8, 0xB7, 0xE1, 0x6C,
};

/* 981FA33222C5451017530155A4BF7F29 */
static const UCHAR plain_ctr_256[] = {
0x98, 0x1F, 0xA3, 0x32, 0x22, 0xC5, 0x45, 0x10, 0x17, 0x53, 0x01, 0x55, 0xA4, 0xBF, 0x7F, 0x29, 
};

/* 643B91B4E541B20AAAEAB77F2D328566 */
static const UCHAR secret_ctr_256[] = {
0x64, 0x3B, 0x91, 0xB4, 0xE5, 0x41, 0xB2, 0x0A, 0xAA, 0xEA, 0xB7, 0x7F, 0x2D, 0x32, 0x85, 0x66, 
};

/* 83F9D97D4AB759FDDCC3EF54A0E2A8EC */
static const UCHAR key_gcm_128[] = {
0x83, 0xF9, 0xD9, 0x7D, 0x4A, 0xB7, 0x59, 0xFD, 0xDC, 0xC3, 0xEF, 0x54, 0xA0, 0xE2, 0xA8, 0xEC,
};

/* 01CF */
static const UCHAR iv_gcm_128[] = {
0x01, 0xCF,
};

/* 77E6329CF9424F71C808DF9170BFD298 */
static const UCHAR plain_gcm_128[] = {
0x77, 0xE6, 0x32, 0x9C, 0xF9, 0x42, 0x4F, 0x71, 0xC8, 0x08, 0xDF, 0x91, 0x70, 0xBF, 0xD2, 0x98,
};

/* 6DD49EAEB4103DAC8F97E3234946DD2D */
static const UCHAR aad_gcm_128[] = {
0x6D, 0xD4, 0x9E, 0xAE, 0xB4, 0x10, 0x3D, 0xAC, 0x8F, 0x97, 0xE3, 0x23, 0x49, 0x46, 0xDD, 0x2D,
};

/* 50DE86A7A92A8A5EA33DB5696B96CD77AA181E84BC8B4BF5A68927C409D422CB */
static const UCHAR secret_gcm_128[] = {
0x50, 0xDE, 0x86, 0xA7, 0xA9, 0x2A, 0x8A, 0x5E, 0xA3, 0x3D, 0xB5, 0x69, 0x6B, 0x96, 0xCD, 0x77,
0xAA, 0x18, 0x1E, 0x84, 0xBC, 0x8B, 0x4B, 0xF5, 0xA6, 0x89, 0x27, 0xC4, 0x09, 0xD4, 0x22, 0xCB
};

static UCHAR output[INPUT_OUTPUT_LENGTH * 2];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_crypto_method_self_test_aes               PORTABLE C      */
/*                                                           6.1          */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_crypto_method_self_test_aes(NX_CRYPTO_METHOD *crypto_method_aes,
                                            VOID *metadata, UINT metadata_size)
{
UCHAR *key;
UCHAR *iv;
UCHAR *plain;
UCHAR *secret;
UCHAR *aad = NX_NULL;
UINT key_size;
UINT secret_size = INPUT_OUTPUT_LENGTH;
UINT status;
VOID *handler = NX_NULL;


    /* Validate the crypto method */
    if(crypto_method_aes == NX_NULL)
        return(NX_PTR_ERROR);

    key_size = crypto_method_aes -> nx_crypto_key_size_in_bits;

    switch (crypto_method_aes -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_ENCRYPTION_AES_CBC:
        switch (key_size)
        {
        case 128:
            key = (UCHAR *)key_cbc_128;
            iv = (UCHAR *)iv_cbc_128;
            plain = (UCHAR *)plain_cbc_128;
            secret = (UCHAR *)secret_cbc_128;
            break;
        case 192:
            key = (UCHAR *)key_cbc_192;
            iv = (UCHAR *)iv_cbc_192;
            plain = (UCHAR *)plain_cbc_192;
            secret = (UCHAR *)secret_cbc_192;
            break;
        case 256:
            key = (UCHAR *)key_cbc_256;
            iv = (UCHAR *)iv_cbc_256;
            plain = (UCHAR *)plain_cbc_256;
            secret = (UCHAR *)secret_cbc_256;
            break;
        default:
            return(1);
        }
        break;

    case NX_CRYPTO_ENCRYPTION_AES_CTR:
        switch (key_size)
        {
        case 128:
            key = (UCHAR *)key_ctr_128;
            iv = (UCHAR *)iv_ctr_128;
            plain = (UCHAR *)plain_ctr_128;
            secret = (UCHAR *)secret_ctr_128;
            break;
        case 192:
            key = (UCHAR *)key_ctr_192;
            iv = (UCHAR *)iv_ctr_192;
            plain = (UCHAR *)plain_ctr_192;
            secret = (UCHAR *)secret_ctr_192;
            break;
        case 256:
            key = (UCHAR *)key_ctr_256;
            iv = (UCHAR *)iv_ctr_256;
            plain = (UCHAR *)plain_ctr_256;
            secret = (UCHAR *)secret_ctr_256;
            break;
        default:
            return(1);
        }
        break;

    case NX_CRYPTO_ENCRYPTION_AES_GCM_16:
        switch (key_size)
        {
        case 128:
            key = (UCHAR *)key_gcm_128;
            iv = (UCHAR *)iv_gcm_128;
            plain = (UCHAR *)plain_gcm_128;
            secret = (UCHAR *)secret_gcm_128;
            aad = (UCHAR *)aad_gcm_128;
            secret_size = sizeof(secret_gcm_128);
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
                                                     key,
                                                     key_size,
                                                     &handler,
                                                     metadata,
                                                     metadata_size);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_aes -> nx_crypto_operation == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (aad != NX_NULL)
    {
        /* Set additional data pointer and length. */
        status = crypto_method_aes -> nx_crypto_operation(NX_CRYPTO_SET_ADDITIONAL_DATA,
                                                          NX_NULL,
                                                          crypto_method_aes,
                                                          key,
                                                          key_size,
                                                          aad,
                                                          INPUT_OUTPUT_LENGTH,
                                                          (UCHAR *)iv,
                                                          (UCHAR *)NX_NULL,
                                                          0,
                                                          metadata,
                                                          metadata_size,
                                                          NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Encryption. */
    status = crypto_method_aes -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                      handler,
                                                      crypto_method_aes,
                                                      key,
                                                      key_size,
                                                      plain,
                                                      INPUT_OUTPUT_LENGTH,
                                                      iv,
                                                      output,
                                                      secret_size,
                                                      metadata,
                                                      metadata_size,
                                                      NX_NULL, NX_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_SECURE_MEMCMP(output, secret, secret_size) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Decryption. */
    status = crypto_method_aes -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                      handler,
                                                      crypto_method_aes,
                                                      key,
                                                      key_size,
                                                      secret,
                                                      secret_size,
                                                      iv,
                                                      output,
                                                      INPUT_OUTPUT_LENGTH,
                                                      metadata,
                                                      metadata_size,
                                                      NX_NULL, NX_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_SECURE_MEMCMP(output, plain, INPUT_OUTPUT_LENGTH) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (crypto_method_aes -> nx_crypto_cleanup)
    {
        status = crypto_method_aes -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif


