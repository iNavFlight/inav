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

/* 39567E74085C7B4C94BCED431A18F109BBACEF73471C0E00B7FA0B1C4F979576B5F1025AD5ADAA1246A6974CEB6BEC30CF5CA14F65231F31F24338707AD35B7C */
static UCHAR key_1_96[] = {
0x39, 0x56, 0x7E, 0x74, 0x08, 0x5C, 0x7B, 0x4C, 0x94, 0xBC, 0xED, 0x43, 0x1A, 0x18, 0xF1, 0x09, 
0xBB, 0xAC, 0xEF, 0x73, 0x47, 0x1C, 0x0E, 0x00, 0xB7, 0xFA, 0x0B, 0x1C, 0x4F, 0x97, 0x95, 0x76, 
0xB5, 0xF1, 0x02, 0x5A, 0xD5, 0xAD, 0xAA, 0x12, 0x46, 0xA6, 0x97, 0x4C, 0xEB, 0x6B, 0xEC, 0x30, 
0xCF, 0x5C, 0xA1, 0x4F, 0x65, 0x23, 0x1F, 0x31, 0xF2, 0x43, 0x38, 0x70, 0x7A, 0xD3, 0x5B, 0x7C, 
};

/* F38B4C7FF694907F27C4BA665E872255855EEA0D2437916F0DEC9E07CD96B60EE7563B70FED5C25A51B33D338716F23D4BF44E2F11D57F2C60EC */
static UCHAR plain_1_96[] = {
0xF3, 0x8B, 0x4C, 0x7F, 0xF6, 0x94, 0x90, 0x7F, 0x27, 0xC4, 0xBA, 0x66, 0x5E, 0x87, 0x22, 0x55, 
0x85, 0x5E, 0xEA, 0x0D, 0x24, 0x37, 0x91, 0x6F, 0x0D, 0xEC, 0x9E, 0x07, 0xCD, 0x96, 0xB6, 0x0E, 
0xE7, 0x56, 0x3B, 0x70, 0xFE, 0xD5, 0xC2, 0x5A, 0x51, 0xB3, 0x3D, 0x33, 0x87, 0x16, 0xF2, 0x3D, 
0x4B, 0xF4, 0x4E, 0x2F, 0x11, 0xD5, 0x7F, 0x2C, 0x60, 0xEC, };

/* 589AD4A4CA1F4BCBDC35C48C7DDB53BA92DBBDB6 */
static UCHAR secret_1_96[] = {
0x58, 0x9A, 0xD4, 0xA4, 0xCA, 0x1F, 0x4B, 0xCB, 0xDC, 0x35, 0xC4, 0x8C, 
};

/* 02530F6A81BE0B595DB36D0D05718736B6CB4C1A19E6AD4896010153ECA6D172EE93D130CBA7B83F9C607A397BB9 */
static UCHAR key_1_160[] = {
0x02, 0x53, 0x0F, 0x6A, 0x81, 0xBE, 0x0B, 0x59, 0x5D, 0xB3, 0x6D, 0x0D, 0x05, 0x71, 0x87, 0x36, 
0xB6, 0xCB, 0x4C, 0x1A, 0x19, 0xE6, 0xAD, 0x48, 0x96, 0x01, 0x01, 0x53, 0xEC, 0xA6, 0xD1, 0x72, 
0xEE, 0x93, 0xD1, 0x30, 0xCB, 0xA7, 0xB8, 0x3F, 0x9C, 0x60, 0x7A, 0x39, 0x7B, 0xB9, };

/* B000062A8ECD6519687CCF78B92A8F025C03155C465D5B5BBF5681385064A80E578F6D4A8E6DB7405AF6603FDE08B221AD7D28351921741D19ED696D5BFDFD5749 */
static UCHAR plain_1_160[] = {
0xB0, 0x00, 0x06, 0x2A, 0x8E, 0xCD, 0x65, 0x19, 0x68, 0x7C, 0xCF, 0x78, 0xB9, 0x2A, 0x8F, 0x02, 
0x5C, 0x03, 0x15, 0x5C, 0x46, 0x5D, 0x5B, 0x5B, 0xBF, 0x56, 0x81, 0x38, 0x50, 0x64, 0xA8, 0x0E, 
0x57, 0x8F, 0x6D, 0x4A, 0x8E, 0x6D, 0xB7, 0x40, 0x5A, 0xF6, 0x60, 0x3F, 0xDE, 0x08, 0xB2, 0x21, 
0xAD, 0x7D, 0x28, 0x35, 0x19, 0x21, 0x74, 0x1D, 0x19, 0xED, 0x69, 0x6D, 0x5B, 0xFD, 0xFD, 0x57, 
0x49, };

/* F9992DFE624C6FFD0DE37D3C188A65B335F57D4B */
static UCHAR secret_1_160[] = {
0xF9, 0x99, 0x2D, 0xFE, 0x62, 0x4C, 0x6F, 0xFD, 0x0D, 0xE3, 0x7D, 0x3C, 0x18, 0x8A, 0x65, 0xB3, 
0x35, 0xF5, 0x7D, 0x4B, };

/* C4DA057B81EA740B697FFE1B6EB8591356BA6D5EA7F1B96E4F048030449ACD64E4BB271CB4DCF94937E6 */
static UCHAR key_256[] = {
0xC4, 0xDA, 0x05, 0x7B, 0x81, 0xEA, 0x74, 0x0B, 0x69, 0x7F, 0xFE, 0x1B, 0x6E, 0xB8, 0x59, 0x13, 
0x56, 0xBA, 0x6D, 0x5E, 0xA7, 0xF1, 0xB9, 0x6E, 0x4F, 0x04, 0x80, 0x30, 0x44, 0x9A, 0xCD, 0x64, 
0xE4, 0xBB, 0x27, 0x1C, 0xB4, 0xDC, 0xF9, 0x49, 0x37, 0xE6, };

/* BDACB6555D294D3AFFC245520116062D98F88D64276BDA593492AE71CFE16E46CABC287CB00DF21D96066D5856C2224EEF609D4896302540078F3A0EE325F5337E */
static UCHAR plain_256[] = {
0xBD, 0xAC, 0xB6, 0x55, 0x5D, 0x29, 0x4D, 0x3A, 0xFF, 0xC2, 0x45, 0x52, 0x01, 0x16, 0x06, 0x2D, 
0x98, 0xF8, 0x8D, 0x64, 0x27, 0x6B, 0xDA, 0x59, 0x34, 0x92, 0xAE, 0x71, 0xCF, 0xE1, 0x6E, 0x46, 
0xCA, 0xBC, 0x28, 0x7C, 0xB0, 0x0D, 0xF2, 0x1D, 0x96, 0x06, 0x6D, 0x58, 0x56, 0xC2, 0x22, 0x4E, 
0xEF, 0x60, 0x9D, 0x48, 0x96, 0x30, 0x25, 0x40, 0x07, 0x8F, 0x3A, 0x0E, 0xE3, 0x25, 0xF5, 0x33, 
0x7E, };

/* 940F986AC891C9000B72EF0CEC69AB66AF002E3A34EB8A3A5F94484E45C0396C */
static UCHAR secret_256[] = {
0x94, 0x0F, 0x98, 0x6A, 0xC8, 0x91, 0xC9, 0x00, 0x0B, 0x72, 0xEF, 0x0C, 0xEC, 0x69, 0xAB, 0x66, 
0xAF, 0x00, 0x2E, 0x3A, 0x34, 0xEB, 0x8A, 0x3A, 0x5F, 0x94, 0x48, 0x4E, 0x45, 0xC0, 0x39, 0x6C, 
};

/* 77B43506276D0B2F1850FD367D7FAF7A58E6FA520BEDBB51B6896C35E899185112D85D0799C83010BC3E */
static UCHAR key_384[] = {
0x77, 0xB4, 0x35, 0x06, 0x27, 0x6D, 0x0B, 0x2F, 0x18, 0x50, 0xFD, 0x36, 0x7D, 0x7F, 0xAF, 0x7A, 
0x58, 0xE6, 0xFA, 0x52, 0x0B, 0xED, 0xBB, 0x51, 0xB6, 0x89, 0x6C, 0x35, 0xE8, 0x99, 0x18, 0x51, 
0x12, 0xD8, 0x5D, 0x07, 0x99, 0xC8, 0x30, 0x10, 0xBC, 0x3E, };

/* 1B8B5F13C4899C23C86FD7443EA7C2172F581E470A44B66BDF91EA5070F45B34DF794F4BC45F383C2F20CB10A850C7733E968A4957D9FF60CEF7B60AFEB8744EA1 */
static UCHAR plain_384[] = {
0x1B, 0x8B, 0x5F, 0x13, 0xC4, 0x89, 0x9C, 0x23, 0xC8, 0x6F, 0xD7, 0x44, 0x3E, 0xA7, 0xC2, 0x17, 
0x2F, 0x58, 0x1E, 0x47, 0x0A, 0x44, 0xB6, 0x6B, 0xDF, 0x91, 0xEA, 0x50, 0x70, 0xF4, 0x5B, 0x34, 
0xDF, 0x79, 0x4F, 0x4B, 0xC4, 0x5F, 0x38, 0x3C, 0x2F, 0x20, 0xCB, 0x10, 0xA8, 0x50, 0xC7, 0x73, 
0x3E, 0x96, 0x8A, 0x49, 0x57, 0xD9, 0xFF, 0x60, 0xCE, 0xF7, 0xB6, 0x0A, 0xFE, 0xB8, 0x74, 0x4E, 
0xA1, };

/* 16289BDA360F58C4F2A0293ED9F500453E1AD288D5A920EF9BA9FBF69B30C08D33641654AEC02705052805432894F7E9 */
static UCHAR secret_384[] = {
0x16, 0x28, 0x9B, 0xDA, 0x36, 0x0F, 0x58, 0xC4, 0xF2, 0xA0, 0x29, 0x3E, 0xD9, 0xF5, 0x00, 0x45, 
0x3E, 0x1A, 0xD2, 0x88, 0xD5, 0xA9, 0x20, 0xEF, 0x9B, 0xA9, 0xFB, 0xF6, 0x9B, 0x30, 0xC0, 0x8D, 
0x33, 0x64, 0x16, 0x54, 0xAE, 0xC0, 0x27, 0x05, 0x05, 0x28, 0x05, 0x43, 0x28, 0x94, 0xF7, 0xE9, 
};

/* 9DB05D0C92A43507550E0840950B6D3ECF83452477BC3B5AD8F4102E92F6135E4602E16EB89620254117 */
static UCHAR key_512[] = {
0x9D, 0xB0, 0x5D, 0x0C, 0x92, 0xA4, 0x35, 0x07, 0x55, 0x0E, 0x08, 0x40, 0x95, 0x0B, 0x6D, 0x3E, 
0xCF, 0x83, 0x45, 0x24, 0x77, 0xBC, 0x3B, 0x5A, 0xD8, 0xF4, 0x10, 0x2E, 0x92, 0xF6, 0x13, 0x5E, 
0x46, 0x02, 0xE1, 0x6E, 0xB8, 0x96, 0x20, 0x25, 0x41, 0x17, };

/* B84AEC039A971A3CAB4882698F1B742A623FCB7122B1D34FC4BAE21E2D906C2D0958C038D1ABD50ED580DA481542A06AD0603F0088D8F31F7848577367BD685CFF */
static UCHAR plain_512[] = {
0xB8, 0x4A, 0xEC, 0x03, 0x9A, 0x97, 0x1A, 0x3C, 0xAB, 0x48, 0x82, 0x69, 0x8F, 0x1B, 0x74, 0x2A, 
0x62, 0x3F, 0xCB, 0x71, 0x22, 0xB1, 0xD3, 0x4F, 0xC4, 0xBA, 0xE2, 0x1E, 0x2D, 0x90, 0x6C, 0x2D, 
0x09, 0x58, 0xC0, 0x38, 0xD1, 0xAB, 0xD5, 0x0E, 0xD5, 0x80, 0xDA, 0x48, 0x15, 0x42, 0xA0, 0x6A, 
0xD0, 0x60, 0x3F, 0x00, 0x88, 0xD8, 0xF3, 0x1F, 0x78, 0x48, 0x57, 0x73, 0x67, 0xBD, 0x68, 0x5C, 
0xFF, };

/* 2105965B53FC1C1E2C806A72F0FC49067887FB598029815C373AD4ABCB506BDBA052549197021B459B6F7D15929DBC7DD2F1859A678E8C8C646053429F3CE0CB */
static UCHAR secret_512[] = {
0x21, 0x05, 0x96, 0x5B, 0x53, 0xFC, 0x1C, 0x1E, 0x2C, 0x80, 0x6A, 0x72, 0xF0, 0xFC, 0x49, 0x06, 
0x78, 0x87, 0xFB, 0x59, 0x80, 0x29, 0x81, 0x5C, 0x37, 0x3A, 0xD4, 0xAB, 0xCB, 0x50, 0x6B, 0xDB, 
0xA0, 0x52, 0x54, 0x91, 0x97, 0x02, 0x1B, 0x45, 0x9B, 0x6F, 0x7D, 0x15, 0x92, 0x9D, 0xBC, 0x7D, 
0xD2, 0xF1, 0x85, 0x9A, 0x67, 0x8E, 0x8C, 0x8C, 0x64, 0x60, 0x53, 0x42, 0x9F, 0x3C, 0xE0, 0xCB, 
};

/* Output. */
static ULONG output[16];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_crypto_method_self_test_hmac_sha          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for HMAC SHA crypto    */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_crypto_method_self_test_hmac_sha(NX_CRYPTO_METHOD *crypto_method_hmac_sha,
                                                 VOID *metadata, UINT metadata_size)
{
UCHAR  *input;
UCHAR  *secret;
UCHAR  *key;
UINT    input_length;
UINT    output_length;
UINT    key_length;
UINT    status;
VOID   *handler = NX_NULL;


    /* Validate the crypto method */
    if(crypto_method_hmac_sha == NX_NULL)
        return(NX_PTR_ERROR);

    /* Set the test data.  */
    switch (crypto_method_hmac_sha -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96:
        input = plain_1_96;
        secret = secret_1_96;
        key = key_1_96;
        input_length = sizeof(plain_1_96);
        output_length = sizeof(secret_1_96);
        key_length = (sizeof(key_1_96) << 3);
        break;
    case NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_160:
        input = plain_1_160;
        secret = secret_1_160;
        key = key_1_160;
        input_length = sizeof(plain_1_160);
        output_length = sizeof(secret_1_160);
        key_length = (sizeof(key_1_160) << 3);
        break;
    case NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256:
        input = plain_256;
        secret = secret_256;
        key = key_256;
        input_length = sizeof(plain_256);
        output_length = sizeof(secret_256);
        key_length = (sizeof(key_256) << 3);
        break;
    case NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_384:
        input = plain_384;
        secret = secret_384;
        key = key_384;
        input_length = sizeof(plain_384);
        output_length = sizeof(secret_384);
        key_length = (sizeof(key_384) << 3);
        break;
    case NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512:
        input = plain_512;
        secret = secret_512;
        key = key_512;
        input_length = sizeof(plain_512);
        output_length = sizeof(secret_512);
        key_length = (sizeof(key_512) <<3);
        break;
    default:
        return(1);
    }

    if (crypto_method_hmac_sha -> nx_crypto_init)
    {
        status = crypto_method_hmac_sha -> nx_crypto_init(crypto_method_hmac_sha,
                                                          key,
                                                          key_length,
                                                          &handler,
                                                          metadata,
                                                          metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_hmac_sha -> nx_crypto_operation == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Reset the output buffer.  */
    NX_SECURE_MEMSET(output, 0xFF, sizeof(output));

    /* Test HMAC SHA with Authentication operation.  */
    status = crypto_method_hmac_sha -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                                           handler,
                                                           crypto_method_hmac_sha,
                                                           key,
                                                           key_length,
                                                           input,
                                                           input_length,
                                                           NX_NULL,
                                                           (UCHAR *)output,
                                                           output_length,
                                                           metadata,
                                                           metadata_size,
                                                           NX_NULL, NX_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Validate the output.  */
    if(NX_SECURE_MEMCMP(output, secret, output_length) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Reset the output buffer.  */
    NX_SECURE_MEMSET(output, 0xFF, sizeof(output));

    /* Test HMAC SHA with Initialize, Update and Calculate operation.  */
    status = crypto_method_hmac_sha -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                           handler,
                                                           crypto_method_hmac_sha,
                                                           key,
                                                           key_length,
                                                           NX_NULL,
                                                           0,
                                                           NX_NULL,
                                                           NX_NULL,
                                                           0,
                                                           metadata,
                                                           metadata_size,
                                                           NX_NULL, NX_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    status = crypto_method_hmac_sha -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                           handler,
                                                           crypto_method_hmac_sha,
                                                           NX_NULL,
                                                           0,
                                                           input,
                                                           input_length,
                                                           NX_NULL,
                                                           NX_NULL,
                                                           0,
                                                           metadata,
                                                           metadata_size,
                                                           NX_NULL, NX_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    status = crypto_method_hmac_sha -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                           handler,
                                                           crypto_method_hmac_sha,
                                                           NX_NULL,
                                                           0,
                                                           NX_NULL,
                                                           0,
                                                           NX_NULL,
                                                           (UCHAR *)output,
                                                           output_length,
                                                           metadata,
                                                           metadata_size,
                                                           NX_NULL, NX_NULL);

    /* Check the status.  */
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Validate the output.  */
    if(NX_SECURE_MEMCMP(output, secret, output_length) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (crypto_method_hmac_sha -> nx_crypto_cleanup)
    {
        status = crypto_method_hmac_sha -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
