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

/* BF8D54470B5F365F14F84C0E046DE4053DE7A05DB847CD2C9844632082F588602EA3885DEF9CA21B8B46AA7CB9588938 */
static UCHAR key_1[] = {
0xBF, 0x8D, 0x54, 0x47, 0x0B, 0x5F, 0x36, 0x5F, 0x14, 0xF8, 0x4C, 0x0E, 0x04, 0x6D, 0xE4, 0x05, 
0x3D, 0xE7, 0xA0, 0x5D, 0xB8, 0x47, 0xCD, 0x2C, 0x98, 0x44, 0x63, 0x20, 0x82, 0xF5, 0x88, 0x60, 
0x2E, 0xA3, 0x88, 0x5D, 0xEF, 0x9C, 0xA2, 0x1B, 0x8B, 0x46, 0xAA, 0x7C, 0xB9, 0x58, 0x89, 0x38, 
};

/* B9CE4947AFA6EB6FB36FC514B7C6A33704F47007E9846663B4564C455202D568FCD43044C156DC0154C5772F99D0093194864A5CEDD5640F202C3874222AB061AF */
static UCHAR plain_1[] = {
0xB9, 0xCE, 0x49, 0x47, 0xAF, 0xA6, 0xEB, 0x6F, 0xB3, 0x6F, 0xC5, 0x14, 0xB7, 0xC6, 0xA3, 0x37, 
0x04, 0xF4, 0x70, 0x07, 0xE9, 0x84, 0x66, 0x63, 0xB4, 0x56, 0x4C, 0x45, 0x52, 0x02, 0xD5, 0x68, 
0xFC, 0xD4, 0x30, 0x44, 0xC1, 0x56, 0xDC, 0x01, 0x54, 0xC5, 0x77, 0x2F, 0x99, 0xD0, 0x09, 0x31, 
0x94, 0x86, 0x4A, 0x5C, 0xED, 0xD5, 0x64, 0x0F, 0x20, 0x2C, 0x38, 0x74, 0x22, 0x2A, 0xB0, 0x61, 
0xAF, };

/* 806EB34DD557E8C076832A27D2E5FC51 */
static UCHAR secret_1[] = {
0x80, 0x6E, 0xB3, 0x4D, 0xD5, 0x57, 0xE8, 0xC0, 0x76, 0x83, 0x2A, 0x27, 0xD2, 0xE5, 0xFC, 0x51, 
};

/* Output. */
static ULONG output[4];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_secure_crypto_method_self_test_hmac_md5          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the Known Answer Test for HMAC MD5 crypto    */
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
UINT _nx_secure_crypto_method_self_test_hmac_md5(NX_CRYPTO_METHOD *crypto_method_hmac_md5,
                                                 VOID *metadata, UINT metadata_size)
{
UINT    status;
UINT    output_length;
VOID   *handler = NX_NULL;


    /* Validate the crypto method */
    if(crypto_method_hmac_md5 == NX_NULL)
        return(NX_PTR_ERROR);

    if (crypto_method_hmac_md5 -> nx_crypto_algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96)
    {
        output_length = 12;
    }
    else if (crypto_method_hmac_md5 -> nx_crypto_algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_MD5_128)
    {
        output_length = 16;
    }
    else
    {
        return(1);
    }

    if (crypto_method_hmac_md5 -> nx_crypto_init)
    {
        status = crypto_method_hmac_md5 -> nx_crypto_init(crypto_method_hmac_md5,
                                                          key_1,
                                                          (sizeof(key_1) << 3),
                                                          &handler,
                                                          metadata,
                                                          metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method_hmac_md5 -> nx_crypto_operation == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Reset the output buffer.  */
    NX_SECURE_MEMSET(output, 0xFF, sizeof(output));

    /* Test HMAC MD5 with Authenticate operation.  */
    status = crypto_method_hmac_md5 -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                                           handler,
                                                           crypto_method_hmac_md5,
                                                           key_1,
                                                           (sizeof(key_1) << 3),
                                                           plain_1,
                                                           sizeof(plain_1),
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
    if(NX_SECURE_MEMCMP(output, secret_1, output_length) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Reset the output buffer.  */
    NX_SECURE_MEMSET(output, 0xFF, sizeof(output));

    /* Test HMAC MD5 with Initialize, Update and Calculate operation.  */
    status = crypto_method_hmac_md5 -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                           handler,
                                                           crypto_method_hmac_md5,
                                                           key_1,
                                                           (sizeof(key_1) << 3),
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

    status = crypto_method_hmac_md5 -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                           handler,
                                                           crypto_method_hmac_md5,
                                                           NX_NULL,
                                                           0,
                                                           plain_1,
                                                           sizeof(plain_1),
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

    status = crypto_method_hmac_md5 -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                           handler,
                                                           crypto_method_hmac_md5,
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
    if(NX_SECURE_MEMCMP(output, secret_1, output_length) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (crypto_method_hmac_md5 -> nx_crypto_cleanup)
    {
        status = crypto_method_hmac_md5 -> nx_crypto_cleanup(metadata);
    }

    return(status);
}
#endif
