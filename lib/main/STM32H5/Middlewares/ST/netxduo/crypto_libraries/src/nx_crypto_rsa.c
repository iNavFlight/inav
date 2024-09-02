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
/**   RSA public-key encryption algorithm                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_rsa.h"
#include "nx_crypto_huge_number.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_rsa_operation                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     This function performs an RSA encryption/decryption operation -    */
/*     for RSA the operation is the same but with different values        */
/*     for the exponent.                                                  */
/*                                                                        */
/*     The output is always the same length as the modulus.               */
/*                                                                        */
/*     If NULL is passed for the scratch buffer pointer, an internal      */
/*     scratch buffer is used.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    exponent                              RSA exponent                  */
/*    exponent_length                       Length of exponent in bytes   */
/*    modulus                               RSA modulus                   */
/*    modulus_length                        Length of modulus in bytes    */
/*    p                                     RSA prime p                   */
/*    p_length                              Length of p in bytes          */
/*    q                                     RSA prime q                   */
/*    q_length                              Length of q in bytes          */
/*    input                                 Input data                    */
/*    input_length                          Length of input in bytes      */
/*    output                                Output buffer                 */
/*    scratch_buf_ptr                       Pointer to scratch buffer     */
/*    scratch_buf_length                    Length of scratch buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    _nx_crypto_huge_number_crt_power_modulus                            */
/*                                          Raise a huge number for CRT   */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_rsa_operation       Handle RSA operation          */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_rsa_operation(const UCHAR *exponent, UINT exponent_length, const UCHAR *modulus, UINT modulus_length,
                                              const UCHAR *p, UINT p_length, UCHAR *q, UINT q_length,
                                              const UCHAR *input, UINT input_length, UCHAR *output,
                                              USHORT *scratch_buf_ptr, UINT scratch_buf_length)
{
UCHAR                *scratch;
UINT                  mod_length;
NX_CRYPTO_HUGE_NUMBER modulus_hn, exponent_hn, input_hn, output_hn, p_hn, q_hn;

    NX_CRYPTO_PARAMETER_NOT_USED(scratch_buf_length);

    /* The RSA operation is reversible so both encryption and decryption can be done with the same operation. */
    /* Local pointer for pointer arithmetic. */
    scratch = (UCHAR *)scratch_buf_ptr;

    /* Set up each of the buffers - point into the scratch buffer at increments of the DH buffer size. */
    modulus_hn.nx_crypto_huge_number_data = (HN_UBASE *)scratch;
    scratch += modulus_length;
    modulus_hn.nx_crypto_huge_buffer_size = modulus_length;

    /* Input buffer(and scratch). */
    input_hn.nx_crypto_huge_number_data = (HN_UBASE *)scratch;
    scratch += modulus_length;
    input_hn.nx_crypto_huge_buffer_size = modulus_length;

    /* Exponent  buffer (and scratch). */
    exponent_hn.nx_crypto_huge_number_data = (HN_UBASE *)scratch;
    scratch += modulus_length;
    exponent_hn.nx_crypto_huge_buffer_size = modulus_length;

    /* Output buffer (and scratch). */
    output_hn.nx_crypto_huge_number_data = (HN_UBASE *)scratch;
    scratch += modulus_length * 2;
    output_hn.nx_crypto_huge_buffer_size = modulus_length * 2;

    /* Copy the exponent from the caller's buffer. */
    _nx_crypto_huge_number_setup(&exponent_hn, exponent, exponent_length);

    /* Copy the input from the caller's buffer. */
    _nx_crypto_huge_number_setup(&input_hn, input, input_length);

    /* Copy the modulus from the caller's buffer. */
    _nx_crypto_huge_number_setup(&modulus_hn, modulus, modulus_length);

    if (p && q)
    {

        p_hn.nx_crypto_huge_number_data = (HN_UBASE *)scratch;
        scratch += (modulus_length >> 1);
        p_hn.nx_crypto_huge_buffer_size = (modulus_length >> 1);

        q_hn.nx_crypto_huge_number_data = (HN_UBASE *)scratch;
        scratch += (modulus_length >> 1);
        q_hn.nx_crypto_huge_buffer_size = (modulus_length >> 1);

        /* Copy the prime p and q from the caller's buffer. */
        _nx_crypto_huge_number_setup(&p_hn, p, p_length);
        _nx_crypto_huge_number_setup(&q_hn, q, q_length);

        /* Finally, generate shared secret from the remote public key, our generated private key, and the modulus, modulus.
           The actual calculation is "shared_secret = (public_key**private_key) % modulus"
           where the "**" denotes exponentiation. */
        _nx_crypto_huge_number_crt_power_modulus(&input_hn, &exponent_hn, &p_hn, &q_hn,
                                                 &modulus_hn, &output_hn,
                                                 (HN_UBASE *)scratch);
    }
    else
    {

        /* Finally, generate shared secret from the remote public key, our generated private key, and the modulus, modulus.
           The actual calculation is "shared_secret = (public_key**private_key) % modulus"
           where the "**" denotes exponentiation. */
        _nx_crypto_huge_number_mont_power_modulus(&input_hn, &exponent_hn, &modulus_hn,
                                                  &output_hn, (HN_UBASE *)scratch);
    }

    /* Copy the shared secret into the return buffer. */
    _nx_crypto_huge_number_extract(&output_hn, output, modulus_length, &mod_length);

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_rsa_init                           PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the modulus for RSA context.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Pointer to RSA crypto method  */
/*    key                                   Pointer to modulus            */
/*    key_size_in_bits                      Length of modulus in bits     */
/*    handle                                Handle of method              */
/*    crypto_metadata                       Pointer to RSA context        */
/*    crypto_metadata_size                  Size of RSA context           */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_rsa_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                                UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                VOID **handle,
                                                VOID *crypto_metadata,
                                                ULONG crypto_metadata_size)
{
NX_CRYPTO_RSA *ctx;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    if ((method == NX_CRYPTO_NULL) || (key == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_RSA))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_RSA *)crypto_metadata;

    ctx -> nx_crypto_rsa_modulus = key;
    ctx -> nx_crypto_rsa_modulus_length = key_size_in_bits >> 3;
    ctx -> nx_crypto_rsa_prime_p = NX_CRYPTO_NULL;
    ctx -> nx_crypto_rsa_prime_p_length = 0;
    ctx -> nx_crypto_rsa_prime_q = NX_CRYPTO_NULL;
    ctx -> nx_crypto_rsa_prime_q_length = 0;

    /* Call _nx_crypto_crypto_rsa_set_prime() to set p and q for private key.
     * Chinese Remainder Theorem will be used when p and q are set. */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_rsa_cleanup                       PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_rsa_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_RSA));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_rsa_operation                      PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the RSA operation function for crypto method.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
/*    handle                                Handle to method              */
/*    method                                Pointer to RSA crypto method  */
/*    key                                   Exponent of RSA operation     */
/*    key_size_in_bits                      Size of exponent in bits      */
/*    input                                 Input stream                  */
/*    input_length_in_byte                  Length of input in byte       */
/*    iv_ptr                                Initial Vector (not used)     */
/*    output                                Output stream                 */
/*    output_length_in_byte                 Length of output in byte      */
/*    crypto_metadata                       Pointer to RSA context        */
/*    crypto_metadata_size                  Size of RSA context           */
/*    packet_ptr                            Pointer to packet (not used)  */
/*    nx_crypto_hw_process_callback         Pointer to callback function  */
/*                                            (not used)                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_rsa_operation              Perform RSA operation         */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_rsa_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                                     VOID *handle, /* Crypto handler */
                                                     struct NX_CRYPTO_METHOD_STRUCT *method,
                                                     UCHAR *key,
                                                     NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                     UCHAR *input,
                                                     ULONG input_length_in_byte,
                                                     UCHAR *iv_ptr,
                                                     UCHAR *output,
                                                     ULONG output_length_in_byte,
                                                     VOID *crypto_metadata,
                                                     ULONG crypto_metadata_size,
                                                     VOID *packet_ptr,
                                                     VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status))
{
NX_CRYPTO_RSA *ctx;
UINT           return_value = NX_CRYPTO_SUCCESS;


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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_RSA))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_RSA *)crypto_metadata;


    if (op == NX_CRYPTO_SET_PRIME_P)
    {
        ctx -> nx_crypto_rsa_prime_p = input;
        ctx -> nx_crypto_rsa_prime_p_length = input_length_in_byte;
    }
    else if (op == NX_CRYPTO_SET_PRIME_Q)
    {
        ctx -> nx_crypto_rsa_prime_q = input;
        ctx -> nx_crypto_rsa_prime_q_length = input_length_in_byte;
    }
    else
    {

        if (key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        if(output_length_in_byte < (key_size_in_bits >> 3))
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);

        if (input_length_in_byte > (ctx -> nx_crypto_rsa_modulus_length))
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        return_value = _nx_crypto_rsa_operation(key,
                                                key_size_in_bits >> 3,
                                                ctx -> nx_crypto_rsa_modulus,
                                                ctx -> nx_crypto_rsa_modulus_length,
                                                ctx -> nx_crypto_rsa_prime_p,
                                                ctx -> nx_crypto_rsa_prime_p_length,
                                                ctx -> nx_crypto_rsa_prime_q,
                                                ctx -> nx_crypto_rsa_prime_q_length,
                                                input, input_length_in_byte,
                                                output,
                                                ctx -> nx_crypto_rsa_scratch_buffer,
                                                NX_CRYPTO_RSA_SCRATCH_BUFFER_SIZE);
    }

    return(return_value);
}

