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
/**   Deterministic Random Bit Generator (DRBG)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_CRYPTO_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_crypto_drbg.h"

#ifdef NX_CRYPTO_DRBG_CTR_METADATA_SIZE
#include "nx_crypto_aes.h"
static UCHAR _nx_crypto_ctr_metadata[NX_CRYPTO_DRBG_CTR_METADATA_SIZE];
#endif

static NX_CRYPTO_DRBG _nx_crypto_drbg_ctx;

static const UCHAR zeroiv[16] = { 0 };
static const UCHAR _nx_crypto_drbg_df_key[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static UINT _nx_crypto_drbg_update(NX_CRYPTO_DRBG *drbg_ptr, UCHAR *provided_data);
static UINT _nx_crypto_drbg_block_cipher_df(NX_CRYPTO_DRBG *drbg_ptr, UINT input_len, UCHAR *output, UINT output_len);


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_drbg_ctr_add_one                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This this a utility function incrementing a large integer by one.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ctr                                   Number to add one to.         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_drbg_generate                                            */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_drbg_ctr_add_one(UCHAR *ctr)
{
USHORT result;
INT i;

    /* Add one for last byte. */
    result = (USHORT)(ctr[15] + 1);
    ctr[15] = (UCHAR)(result & 0xFF);

    /* Handle carry. */
    for (i = 14; i >= 0; i--)
    {
        result = (USHORT)((result >> 8) + ctr[i]);
        ctr[i] = (UCHAR)(result & 0xFF);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_drbg_update                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This isan internal function updates the DRBG with new data.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    drbg_ptr                              DRBG context                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), verified */
/*                                            memcpy use cases, disabled  */
/*                                            unaligned access by default,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static UINT _nx_crypto_drbg_update(NX_CRYPTO_DRBG *drbg_ptr, UCHAR *provided_data)
{
NX_CRYPTO_METHOD *crypto_method;
UINT key_len;
UCHAR temp[NX_CRYPTO_DRBG_MAX_SEEDLEN];
UINT temp_len = 0;
UINT status = NX_CRYPTO_SUCCESS;
VOID *handler = NX_CRYPTO_NULL;

    crypto_method = drbg_ptr -> nx_crypto_drbg_crypto_method;

    /* Initialize crypto method with DRBG key. */
    if (crypto_method -> nx_crypto_init)
    {
        status = crypto_method -> nx_crypto_init(crypto_method,
                                                 drbg_ptr -> nx_crypto_drbg_key,
                                                 crypto_method -> nx_crypto_key_size_in_bits,
                                                 &handler,
                                                 drbg_ptr -> nx_crypto_drbg_crypto_metadata,
                                                 crypto_method -> nx_crypto_metadata_area_size);
        if (status)
        {
            return(status);
        }
    }

    while (temp_len < drbg_ptr -> nx_crypto_drbg_seedlen)
    {
        /* V = (V+1) mod 2^blocklen */
        _nx_crypto_drbg_ctr_add_one(drbg_ptr -> nx_crypto_drbg_v);

        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                      handler,
                                                      crypto_method,
                                                      NX_CRYPTO_NULL,
                                                      crypto_method -> nx_crypto_key_size_in_bits,
                                                      (UCHAR *)drbg_ptr -> nx_crypto_drbg_v,
                                                      NX_CRYPTO_DRBG_BLOCK_LENGTH_AES,
                                                      (UCHAR *)zeroiv,
                                                      &temp[temp_len],
                                                      NX_CRYPTO_DRBG_BLOCK_LENGTH_AES,
                                                      drbg_ptr -> nx_crypto_drbg_crypto_metadata,
                                                      crypto_method -> nx_crypto_metadata_area_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        temp_len += NX_CRYPTO_DRBG_BLOCK_LENGTH_AES;

    }

    if (crypto_method -> nx_crypto_cleanup)
    {
        status = crypto_method -> nx_crypto_cleanup(drbg_ptr -> nx_crypto_drbg_crypto_metadata);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (provided_data != NX_CRYPTO_NULL)
    {
        /* temp = temp xor provided_data. */
#ifdef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
        for (temp_len = 0; temp_len < (drbg_ptr -> nx_crypto_drbg_seedlen >> 2); temp_len++)
        {
            ((UINT*)temp)[temp_len] ^= ((UINT*)provided_data)[temp_len];
        }
#else
        for (temp_len = 0; temp_len < drbg_ptr -> nx_crypto_drbg_seedlen; temp_len++)
        {
            temp[temp_len] ^= provided_data[temp_len];
        }
#endif
    }

    key_len = crypto_method -> nx_crypto_key_size_in_bits >> 3;

    NX_CRYPTO_MEMCPY(drbg_ptr -> nx_crypto_drbg_key, temp, key_len); /* Use case of memcpy is verified. */
    NX_CRYPTO_MEMCPY(drbg_ptr -> nx_crypto_drbg_v, &temp[key_len], NX_CRYPTO_DRBG_BLOCK_LENGTH_AES); /* Use case of memcpy is verified. */

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(temp, 0, sizeof(temp));
#endif /* NX_SECURE_KEY_CLEAR */

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_drbg_instantiate                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function instantiate a DRBG context.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    drbg_ptr                              DRBG context                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), verified */
/*                                            memcpy use cases, disabled  */
/*                                            unaligned access by default,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_drbg_instantiate(NX_CRYPTO_DRBG *drbg_ptr,
                                                UCHAR *nonce,
                                                UINT nonce_len,
                                                UCHAR *personalization_string,
                                                UINT personalization_string_len)
{
UINT i;
UCHAR seed_material[NX_CRYPTO_DRBG_MAX_SEEDLEN];
UCHAR *df_input;
UINT status;
UINT entropy_len;

    if (drbg_ptr -> nx_crypto_drbg_crypto_method == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    drbg_ptr -> nx_crypto_drbg_seedlen = (UINT)(NX_CRYPTO_DRBG_BLOCK_LENGTH + (drbg_ptr -> nx_crypto_drbg_crypto_method -> nx_crypto_key_size_in_bits >> 3));

    if (drbg_ptr -> nx_crypto_drbg_use_df)
    {
        entropy_len = drbg_ptr -> nx_crypto_drbg_security_strength;

        if (entropy_len + nonce_len + personalization_string_len + NX_CRYPTO_DRBG_DF_INPUT_OFFSET >= NX_CRYPTO_DRBG_SEED_BUFFER_LEN)
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        /* seed_material = entropy_input || nonce || personalization_string. */
        df_input = drbg_ptr -> nx_crypto_drbg_buffer + NX_CRYPTO_DRBG_DF_INPUT_OFFSET;

        status = drbg_ptr -> nx_crypto_drbg_get_entropy(df_input, &entropy_len, NX_CRYPTO_DRBG_MAX_ENTROPY_LEN);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        df_input += entropy_len;
        NX_CRYPTO_MEMCPY(df_input, nonce, nonce_len); /* Use case of memcpy is verified. */
        df_input += nonce_len;
        NX_CRYPTO_MEMCPY(df_input, personalization_string, personalization_string_len); /* Use case of memcpy is verified. */

        /* seed_material = Block_Cipher_df (seed_material, seedlen). */
        _nx_crypto_drbg_block_cipher_df(drbg_ptr, entropy_len + nonce_len + personalization_string_len, seed_material, drbg_ptr -> nx_crypto_drbg_seedlen);
    }
    else
    {
        if (personalization_string_len > drbg_ptr -> nx_crypto_drbg_seedlen)
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        if (personalization_string != NX_CRYPTO_NULL && personalization_string_len > 0)
        {
            NX_CRYPTO_MEMCPY(seed_material, personalization_string, personalization_string_len); /* Use case of memcpy is verified. */
        }

        /* Ensure that the length of the personalization_string is exactly seedlen bits. */
        if (personalization_string_len < drbg_ptr -> nx_crypto_drbg_seedlen)
        {
            NX_CRYPTO_MEMSET(&seed_material[personalization_string_len], 0,
                   drbg_ptr -> nx_crypto_drbg_seedlen - personalization_string_len);
        }

        entropy_len = drbg_ptr -> nx_crypto_drbg_seedlen;
        status = drbg_ptr -> nx_crypto_drbg_get_entropy(drbg_ptr -> nx_crypto_drbg_buffer, &entropy_len, drbg_ptr -> nx_crypto_drbg_seedlen);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* seed_material = entropy_input xor personalization_string. */
#ifdef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
        for (i = 0; i < (drbg_ptr -> nx_crypto_drbg_seedlen >> 2); i++)
        {
            ((UINT*)seed_material)[i] ^= ((UINT*)drbg_ptr -> nx_crypto_drbg_buffer)[i];
        }
#else
        for (i = 0; i < drbg_ptr -> nx_crypto_drbg_seedlen; i++)
        {
            seed_material[i] ^= drbg_ptr -> nx_crypto_drbg_buffer[i];
        }
#endif
    }

    NX_CRYPTO_MEMSET(drbg_ptr -> nx_crypto_drbg_key, 0, sizeof(drbg_ptr -> nx_crypto_drbg_key));
    NX_CRYPTO_MEMSET(drbg_ptr -> nx_crypto_drbg_v, 0, sizeof(drbg_ptr -> nx_crypto_drbg_v));

    _nx_crypto_drbg_update(drbg_ptr, seed_material);

    drbg_ptr -> nx_crypto_drgb_reseed_counter = 1;

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(seed_material, 0, sizeof(seed_material));
#endif /* NX_SECURE_KEY_CLEAR */

    drbg_ptr -> nx_crypto_drbg_instantiated = 1;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_drbg_reseed                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reseed a DRBG context.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    drbg_ptr                              DRBG context                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), verified */
/*                                            memcpy use cases, disabled  */
/*                                            unaligned access by default,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_drbg_reseed(NX_CRYPTO_DRBG *drbg_ptr,
                                 UCHAR *additional_input,
                                 UINT additional_input_len)
{
UINT i;
UCHAR seed_material[NX_CRYPTO_DRBG_MAX_SEEDLEN];
UCHAR *df_input;
UINT entropy_len;
UINT status;

    if (!drbg_ptr -> nx_crypto_drbg_instantiated)
    {
        return(NX_CRYPTO_NO_INSTANCE);
    }

    if (drbg_ptr -> nx_crypto_drbg_use_df)
    {
        entropy_len = drbg_ptr -> nx_crypto_drbg_security_strength;

        if (entropy_len + additional_input_len + NX_CRYPTO_DRBG_DF_INPUT_OFFSET >= NX_CRYPTO_DRBG_SEED_BUFFER_LEN)
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        /* seed_material = entropy_input || additional_input. */
        df_input = drbg_ptr -> nx_crypto_drbg_buffer + NX_CRYPTO_DRBG_DF_INPUT_OFFSET;

        status = drbg_ptr -> nx_crypto_drbg_get_entropy(df_input, &entropy_len, NX_CRYPTO_DRBG_MAX_ENTROPY_LEN);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        df_input += entropy_len;
        NX_CRYPTO_MEMCPY(df_input, additional_input, additional_input_len); /* Use case of memcpy is verified. */

        /* seed_material = Block_Cipher_df (seed_material, seedlen). */
        _nx_crypto_drbg_block_cipher_df(drbg_ptr, entropy_len + additional_input_len, seed_material, drbg_ptr -> nx_crypto_drbg_seedlen);
    }
    else
    {
        if (additional_input_len > drbg_ptr -> nx_crypto_drbg_seedlen)
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        /* Ensure that the length of the additional_input is exactly seedlen bits. */
        NX_CRYPTO_MEMCPY(seed_material, additional_input, additional_input_len); /* Use case of memcpy is verified. */
        if (additional_input_len < drbg_ptr -> nx_crypto_drbg_seedlen)
        {
            NX_CRYPTO_MEMSET(&seed_material[additional_input_len], 0,
                   drbg_ptr -> nx_crypto_drbg_seedlen - additional_input_len);
        }

        entropy_len = drbg_ptr -> nx_crypto_drbg_seedlen;
        status = drbg_ptr -> nx_crypto_drbg_get_entropy(drbg_ptr -> nx_crypto_drbg_buffer, &entropy_len, NX_CRYPTO_DRBG_MAX_ENTROPY_LEN);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* seed_material = entropy_input xor additional_input. */
#ifdef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
        for (i = 0; i < (drbg_ptr -> nx_crypto_drbg_seedlen >> 2); i++)
        {
            ((UINT*)seed_material)[i] ^= ((UINT*)drbg_ptr -> nx_crypto_drbg_buffer)[i];
        }
#else
        for (i = 0; i < drbg_ptr -> nx_crypto_drbg_seedlen; i++)
        {
            seed_material[i] ^= drbg_ptr -> nx_crypto_drbg_buffer[i];
        }
#endif
    }

    _nx_crypto_drbg_update(drbg_ptr, seed_material);

    drbg_ptr -> nx_crypto_drgb_reseed_counter = 1;

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(seed_material, 0, sizeof(seed_material));
#endif /* NX_SECURE_KEY_CLEAR */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_drbg_generate                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates requested number of bits from DRBG.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    drbg_ptr                              DRBG context                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
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
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_drbg_generate(NX_CRYPTO_DRBG *drbg_ptr,
                                             UCHAR *output, UINT output_length_in_byte,
                                             UCHAR *additional_input,
                                             UINT additional_input_len)
{
NX_CRYPTO_METHOD *crypto_method;
UCHAR addition[NX_CRYPTO_DRBG_MAX_SEEDLEN];
UCHAR temp[NX_CRYPTO_DRBG_BLOCK_LENGTH_AES];
UCHAR *out_ptr;
UINT temp_len = 0;
UINT status = NX_CRYPTO_SUCCESS;
VOID *handler = NX_CRYPTO_NULL;

    if (!drbg_ptr -> nx_crypto_drbg_instantiated)
    {
        return(NX_CRYPTO_NO_INSTANCE);
    }

    if (drbg_ptr -> nx_crypto_drgb_reseed_counter > NX_CRYPTO_DRBG_MAX_SEED_LIFE || drbg_ptr -> nx_crypto_drbg_prediction_resistance)
    {
        status = _nx_crypto_drbg_reseed(drbg_ptr, additional_input, additional_input_len);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        additional_input_len = 0;
    }

    if (drbg_ptr -> nx_crypto_drbg_use_df)
    {
        if (additional_input_len + NX_CRYPTO_DRBG_DF_INPUT_OFFSET >= NX_CRYPTO_DRBG_SEED_BUFFER_LEN)
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        if (additional_input_len > 0 && additional_input != NX_CRYPTO_NULL)
        {
            /* additional_input = Block_Cipher_df (additional_input, 256). */
            NX_CRYPTO_MEMCPY(drbg_ptr -> nx_crypto_drbg_buffer + NX_CRYPTO_DRBG_DF_INPUT_OFFSET, additional_input, additional_input_len); /* Use case of memcpy is verified. */

            _nx_crypto_drbg_block_cipher_df(drbg_ptr, additional_input_len, addition, drbg_ptr -> nx_crypto_drbg_seedlen);

            _nx_crypto_drbg_update(drbg_ptr, addition);
        }
    }
    else
    {
        if (additional_input_len > drbg_ptr -> nx_crypto_drbg_seedlen)
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        if (additional_input_len == drbg_ptr -> nx_crypto_drbg_seedlen && additional_input != NX_CRYPTO_NULL)
        {
            _nx_crypto_drbg_update(drbg_ptr, additional_input);
        }
        else if (additional_input_len > 0 && additional_input != NX_CRYPTO_NULL)
        {
            /* Ensure that the length of the additional_input is exactly seedlen bits. */
            NX_CRYPTO_MEMCPY(addition, additional_input, additional_input_len); /* Use case of memcpy is verified. */
            if (additional_input_len < drbg_ptr -> nx_crypto_drbg_seedlen)
            {
                NX_CRYPTO_MEMSET(&addition[additional_input_len], 0,
                       drbg_ptr -> nx_crypto_drbg_seedlen - additional_input_len);
            }
            _nx_crypto_drbg_update(drbg_ptr, addition);
        }
    }

    if (additional_input_len == 0)
    {
        additional_input = NX_CRYPTO_NULL;
    }

    crypto_method = drbg_ptr -> nx_crypto_drbg_crypto_method;

    /* Initialize crypto method with DRBG key. */
    if (crypto_method -> nx_crypto_init)
    {
        status = crypto_method -> nx_crypto_init(crypto_method,
                                                 drbg_ptr -> nx_crypto_drbg_key,
                                                 crypto_method -> nx_crypto_key_size_in_bits,
                                                 &handler,
                                                 drbg_ptr -> nx_crypto_drbg_crypto_metadata,
                                                 crypto_method -> nx_crypto_metadata_area_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    while (temp_len < output_length_in_byte)
    {
        /* V = (V+1) mod 2^blocklen */
        _nx_crypto_drbg_ctr_add_one(drbg_ptr -> nx_crypto_drbg_v);

        if (output_length_in_byte - temp_len < NX_CRYPTO_DRBG_BLOCK_LENGTH_AES)
        {
            out_ptr = temp;
        }
        else
        {
            out_ptr = &output[temp_len];
        }

        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                      handler,
                                                      crypto_method,
                                                      NX_CRYPTO_NULL,
                                                      crypto_method -> nx_crypto_key_size_in_bits,
                                                      (UCHAR *)drbg_ptr -> nx_crypto_drbg_v,
                                                      NX_CRYPTO_DRBG_BLOCK_LENGTH_AES,
                                                      (UCHAR *)zeroiv,
                                                      out_ptr,
                                                      NX_CRYPTO_DRBG_BLOCK_LENGTH_AES,
                                                      drbg_ptr -> nx_crypto_drbg_crypto_metadata,
                                                      crypto_method -> nx_crypto_metadata_area_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        if (output_length_in_byte - temp_len < NX_CRYPTO_DRBG_BLOCK_LENGTH_AES)
        {
            NX_CRYPTO_MEMCPY(&output[temp_len], temp, output_length_in_byte - temp_len); /* Use case of memcpy is verified. */
        }

        temp_len += NX_CRYPTO_DRBG_BLOCK_LENGTH_AES;
    }

    if (crypto_method -> nx_crypto_cleanup)
    {
        status = crypto_method -> nx_crypto_cleanup(drbg_ptr -> nx_crypto_drbg_crypto_metadata);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (additional_input_len > 0 && (drbg_ptr -> nx_crypto_drbg_use_df || additional_input_len != drbg_ptr -> nx_crypto_drbg_seedlen))
    {
        _nx_crypto_drbg_update(drbg_ptr, addition);
    }
    else
    {
        _nx_crypto_drbg_update(drbg_ptr, additional_input);
    }

    drbg_ptr -> nx_crypto_drgb_reseed_counter++;

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(addition, 0, sizeof(addition));
    NX_CRYPTO_MEMSET(temp, 0, sizeof(temp));
#endif /* NX_SECURE_KEY_CLEAR */

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_drbg_block_cipher_df                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates requested number of bits from DRBG.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    drbg_ptr                              DRBG context                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
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
/*                                            verified memcpy use cases,  */
/*                                            and updated constants,      */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static UINT _nx_crypto_drbg_block_cipher_df(NX_CRYPTO_DRBG *drbg_ptr, UINT input_len,
                                                           UCHAR *output, UINT output_len)
{
NX_CRYPTO_METHOD *crypto_method;
UINT status = 0;
UCHAR temp[NX_CRYPTO_DRBG_MAX_SEEDLEN];
UINT temp_len;
UINT s_len;
UINT i, j;
UCHAR bcc_chain[NX_CRYPTO_DRBG_BLOCK_LENGTH];
UCHAR *iv;
UCHAR *s;
UCHAR *out_ptr;
VOID *handler = NX_CRYPTO_NULL;


    crypto_method = drbg_ptr -> nx_crypto_drbg_crypto_method;

    /* Initialize crypto method with DRBG key. */
    if (crypto_method -> nx_crypto_init)
    {
        status = crypto_method -> nx_crypto_init(crypto_method,
                                                 (UCHAR *)_nx_crypto_drbg_df_key,
                                                 crypto_method -> nx_crypto_key_size_in_bits,
                                                 &handler,
                                                 drbg_ptr -> nx_crypto_drbg_crypto_metadata,
                                                 crypto_method -> nx_crypto_metadata_area_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    iv = drbg_ptr -> nx_crypto_drbg_buffer;
    s = &iv[NX_CRYPTO_DRBG_BLOCK_LENGTH];
    NX_CRYPTO_MEMSET(iv, 0, NX_CRYPTO_DRBG_BLOCK_LENGTH);
    s[0] = (UCHAR)((input_len >> 24) & 0xff);
    s[1] = (UCHAR)((input_len >> 16) & 0xff);
    s[2] = (UCHAR)((input_len >> 8) & 0xff);
    s[3] = (UCHAR)(input_len & 0xff);
    s[4] = (UCHAR)((output_len >> 24) & 0xff);
    s[5] = (UCHAR)((output_len >> 16) & 0xff);
    s[6] = (UCHAR)((output_len >> 8) & 0xff);
    s[7] = (UCHAR)(output_len & 0xff);
    s[input_len + 8] = 0x80;
    s_len = NX_CRYPTO_DRBG_BLOCK_LENGTH + input_len + 8 + 1;
    if (s_len % NX_CRYPTO_DRBG_BLOCK_LENGTH != 0)
    {
        NX_CRYPTO_MEMSET(&iv[s_len], 0, NX_CRYPTO_DRBG_BLOCK_LENGTH - (s_len % NX_CRYPTO_DRBG_BLOCK_LENGTH));
        s_len += NX_CRYPTO_DRBG_BLOCK_LENGTH - (s_len % NX_CRYPTO_DRBG_BLOCK_LENGTH);
    }

    temp_len = 0;
    while (temp_len < drbg_ptr -> nx_crypto_drbg_seedlen)
    {
        NX_CRYPTO_MEMSET(bcc_chain, 0, sizeof(bcc_chain));

        for (i = 0; i < s_len; i += NX_CRYPTO_DRBG_BLOCK_LENGTH)
        {
            for (j = 0; j < NX_CRYPTO_DRBG_BLOCK_LENGTH; j++)
            {
                bcc_chain[j] ^= iv[i + j];
            }

            status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                          handler,
                                                          crypto_method,
                                                          NX_CRYPTO_NULL,
                                                          crypto_method -> nx_crypto_key_size_in_bits,
                                                          bcc_chain,
                                                          NX_CRYPTO_DRBG_BLOCK_LENGTH,
                                                          (UCHAR *)zeroiv,
                                                          bcc_chain,
                                                          NX_CRYPTO_DRBG_BLOCK_LENGTH,
                                                          drbg_ptr -> nx_crypto_drbg_crypto_metadata,
                                                          crypto_method -> nx_crypto_metadata_area_size,
                                                          NX_CRYPTO_NULL, NX_CRYPTO_NULL);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

        }

        NX_CRYPTO_MEMCPY(&temp[temp_len], bcc_chain, NX_CRYPTO_DRBG_BLOCK_LENGTH); /* Use case of memcpy is verified. */
        temp_len += NX_CRYPTO_DRBG_BLOCK_LENGTH;

        iv[3]++;
    }

    if (crypto_method -> nx_crypto_cleanup)
    {
        status = crypto_method -> nx_crypto_cleanup(drbg_ptr -> nx_crypto_drbg_crypto_metadata);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    if (crypto_method -> nx_crypto_init)
    {
        status = crypto_method -> nx_crypto_init(crypto_method,
                                                 temp,
                                                 crypto_method -> nx_crypto_key_size_in_bits,
                                                 &handler,
                                                 drbg_ptr -> nx_crypto_drbg_crypto_metadata,
                                                 crypto_method -> nx_crypto_metadata_area_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    s = &temp[crypto_method -> nx_crypto_key_size_in_bits >> 3];

    temp_len = 0;
    while (temp_len < output_len)
    {
        if (output_len - temp_len < NX_CRYPTO_DRBG_BLOCK_LENGTH)
        {
            out_ptr = bcc_chain;
        }
        else
        {
            out_ptr = &output[temp_len];
        }

        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                      handler,
                                                      crypto_method,
                                                      NX_CRYPTO_NULL,
                                                      crypto_method -> nx_crypto_key_size_in_bits,
                                                      s,
                                                      NX_CRYPTO_DRBG_BLOCK_LENGTH_AES,
                                                      (UCHAR *)zeroiv,
                                                      out_ptr,
                                                      NX_CRYPTO_DRBG_BLOCK_LENGTH_AES,
                                                      drbg_ptr -> nx_crypto_drbg_crypto_metadata,
                                                      crypto_method -> nx_crypto_metadata_area_size,
                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        s = out_ptr;

        if (output_len - temp_len < NX_CRYPTO_DRBG_BLOCK_LENGTH_AES)
        {
            NX_CRYPTO_MEMCPY(&output[temp_len], bcc_chain, output_len - temp_len); /* Use case of memcpy is verified. */
        }

        temp_len += NX_CRYPTO_DRBG_BLOCK_LENGTH;
    }

    if (crypto_method -> nx_crypto_cleanup)
    {
        status = crypto_method -> nx_crypto_cleanup(drbg_ptr -> nx_crypto_drbg_crypto_metadata);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(temp, 0, sizeof(temp));
    NX_CRYPTO_MEMSET(bcc_chain, 0, sizeof(bcc_chain));
#endif /* NX_SECURE_KEY_CLEAR */

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_drbg_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported DRBG cryptographic algorithm.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Pointer to crypto method      */
/*    key                                   Pointer to key                */
/*    key_size_in_bits                      Length of key size in bits    */
/*    handler                               Returned crypto handler       */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Size of the metadata area     */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_drbg_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                 VOID  **handle,
                                                 VOID  *crypto_metadata,
                                                 ULONG crypto_metadata_size)
{

    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    if ((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_DRBG))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_drbg_cleanup                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up the crypto metadata.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Crypto metadata               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMSET                      Set the memory                */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_drbg_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_DRBG));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_drbg_operation                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs a DRBG operation.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    DRBG operation                */
/*    handle                                Crypto handle                 */
/*    method                                Cryption Method Object        */
/*    key                                   Encryption Key                */
/*    key_size_in_bits                      Key size in bits              */
/*    input                                 Input data                    */
/*    input_length_in_byte                  Input data size               */
/*    iv_ptr                                Initial vector                */
/*    output                                Output buffer                 */
/*    output_length_in_byte                 Output buffer size            */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Metadata area size            */
/*    packet_ptr                            Pointer to packet             */
/*    nx_crypto_hw_process_callback         Callback function pointer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_drbg_operation(UINT op,
                                                     VOID *handle,
                                                     struct NX_CRYPTO_METHOD_STRUCT *method,
                                                     UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                     UCHAR *input, ULONG input_length_in_byte,
                                                     UCHAR *iv_ptr,
                                                     UCHAR *output, ULONG output_length_in_byte,
                                                     VOID *crypto_metadata, ULONG crypto_metadata_size,
                                                     VOID *packet_ptr,
                                                     VOID (*nx_crypto_hw_process_callback)(VOID *, UINT))
{
NX_CRYPTO_DRBG *drbg;
UINT            status = NX_CRYPTO_SUCCESS;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_DRBG))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    drbg = (NX_CRYPTO_DRBG *)crypto_metadata;

    if (op == NX_CRYPTO_DRBG_OPTIONS_SET)
    {

        drbg -> nx_crypto_drbg_security_strength = ((NX_CRYPTO_DRBG_OPTIONS *)input) -> security_strength;
        drbg -> nx_crypto_drbg_get_entropy = ((NX_CRYPTO_DRBG_OPTIONS *)input) -> entropy_input;
        drbg -> nx_crypto_drbg_use_df = ((NX_CRYPTO_DRBG_OPTIONS *)input) -> use_df;
        drbg -> nx_crypto_drbg_prediction_resistance = ((NX_CRYPTO_DRBG_OPTIONS *)input) -> prediction_resistance;
        drbg -> nx_crypto_drbg_crypto_method = ((NX_CRYPTO_DRBG_OPTIONS *)input) -> crypto_method;
        drbg -> nx_crypto_drbg_crypto_metadata = ((NX_CRYPTO_DRBG_OPTIONS *)input) -> crypto_metadata;
        drbg -> nx_crypto_drbg_instantiated = 0;
    }
    else if (op == NX_CRYPTO_DRBG_INSTANTIATE)
    {
        if (key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        status = _nx_crypto_drbg_instantiate(drbg,
                                             key,
                                             key_size_in_bits >> 3,
                                             input,
                                             input_length_in_byte);
    }
    else if (op == NX_CRYPTO_DRBG_RESEED)
    {
        status = _nx_crypto_drbg_reseed(drbg, input, input_length_in_byte);
    }
    else if (op == NX_CRYPTO_DRBG_GENERATE)
    {
        status = _nx_crypto_drbg_generate(drbg,
                                          output,
                                          output_length_in_byte,
                                          input,
                                          input_length_in_byte);
    }
    else
    {
        status = NX_CRYPTO_NOT_SUCCESSFUL;
    }

    return(status);
}

NX_CRYPTO_KEEP static UINT _nx_crypto_drbg_rnd_entropy_input(UCHAR *entropy, UINT *entropy_len,
                                                             UINT entropy_max_len)
{
UINT bytes;
UINT random_number;

    NX_CRYPTO_PARAMETER_NOT_USED(entropy_max_len);

    bytes = *entropy_len;

    while (bytes > 3)
    {
        random_number = (UINT)NX_CRYPTO_RAND();
        entropy[0] = (UCHAR)(random_number & 0xFF);
        entropy[1] = (UCHAR)((random_number >> 8) & 0xFF);
        entropy[2] = (UCHAR)((random_number >> 16) & 0xFF);
        entropy[3] = (UCHAR)((random_number >> 24) & 0xFF);
        entropy += 4;
        bytes -= 4;
    }

    if (bytes == 0)
    {
        return(NX_CRYPTO_SUCCESS);
    }

    random_number = (UINT)NX_CRYPTO_RAND();
    while (bytes > 0)
    {
        entropy[0] = (UCHAR)(random_number & 0xFF);
        random_number = random_number >> 8;
        entropy++;
        bytes--;
    }

    return(NX_CRYPTO_SUCCESS);
}

NX_CRYPTO_KEEP static UINT _nx_crypto_drbg_initialize()
{
UINT status;

    _nx_crypto_drbg_ctx.nx_crypto_drbg_get_entropy = NX_CRYPTO_DRBG_ENTROPY_INPUT_FUNC;
    _nx_crypto_drbg_ctx.nx_crypto_drbg_use_df = NX_CRYPTO_DRBG_USE_DF;
    _nx_crypto_drbg_ctx.nx_crypto_drbg_prediction_resistance = NX_CRYPTO_DRBG_PREDICTION_RESISTANCE;
    _nx_crypto_drbg_ctx.nx_crypto_drbg_crypto_method = NX_CRYPTO_DRBG_CTR_CRYPTO_METHOD;
    _nx_crypto_drbg_ctx.nx_crypto_drbg_crypto_metadata = NX_CRYPTO_DRBG_CTR_CRYPTO_METADATA;
    _nx_crypto_drbg_ctx.nx_crypto_drbg_security_strength = _nx_crypto_drbg_ctx.nx_crypto_drbg_crypto_method -> nx_crypto_key_size_in_bits >> 3;


    status = _nx_crypto_drbg_instantiate(&_nx_crypto_drbg_ctx,
                                         NX_CRYPTO_NULL,
                                         0,
                                         NX_CRYPTO_NULL,
                                         0);
    return(status);
}

NX_CRYPTO_KEEP UINT _nx_crypto_drbg(UINT bits, UCHAR *result)
{
UINT bytes;
UINT mask;
UINT status;

    bytes = (bits + 7) >> 3;

    NX_CRYPTO_DRBG_MUTEX_GET;

    if (!_nx_crypto_drbg_ctx.nx_crypto_drbg_instantiated)
    {
        _nx_crypto_drbg_initialize();
    }

    status = _nx_crypto_drbg_generate(&_nx_crypto_drbg_ctx, result, bytes, NX_CRYPTO_NULL, 0);
    if (status)
    {
        return(status);
    }

    NX_CRYPTO_DRBG_MUTEX_PUT;

    /* Zero out extra bits generated. */
    bits = bits & 7;
    if (bits)
    {
        mask = (UINT)((1 << bits) - 1);
        result[0] = (UCHAR)(result[0] & mask);
    }

    return(status);
}
