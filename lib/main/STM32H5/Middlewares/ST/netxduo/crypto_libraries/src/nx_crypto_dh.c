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
/**   Diffie-Hellman (DH)                                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_CRYPTO_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_crypto_dh.h"
#ifndef NX_CRYPTO_SELF_TEST

/* The Diffie-Hellman group 2 modulus. */
/* Modulus, in byte stream, be */
/* ffffffffffffffffc90fdaa22168c234c4c6628b80dc1cd129024e088a67cc74020bbea63b139b22514a08798e3404ddef9519b3cd3a431b302b0a6df25f14374fe1356d6d51c245e485b576625e7ec6f44c42e9a637ed6b0bff5cb6f406b7edee386bfb5a899fa5ae9f24117c4b1fe649286651ece65381ffffffffffffffff */
static const HN_UBASE _nx_dh_group_2_modulus[] =
{
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xECE65381), HN_ULONG_TO_UBASE(0x49286651),
    HN_ULONG_TO_UBASE(0x7C4B1FE6), HN_ULONG_TO_UBASE(0xAE9F2411),
    HN_ULONG_TO_UBASE(0x5A899FA5), HN_ULONG_TO_UBASE(0xEE386BFB),
    HN_ULONG_TO_UBASE(0xF406B7ED), HN_ULONG_TO_UBASE(0x0BFF5CB6),
    HN_ULONG_TO_UBASE(0xA637ED6B), HN_ULONG_TO_UBASE(0xF44C42E9),
    HN_ULONG_TO_UBASE(0x625E7EC6), HN_ULONG_TO_UBASE(0xE485B576),
    HN_ULONG_TO_UBASE(0x6D51C245), HN_ULONG_TO_UBASE(0x4FE1356D),
    HN_ULONG_TO_UBASE(0xF25F1437), HN_ULONG_TO_UBASE(0x302B0A6D),
    HN_ULONG_TO_UBASE(0xCD3A431B), HN_ULONG_TO_UBASE(0xEF9519B3),
    HN_ULONG_TO_UBASE(0x8E3404DD), HN_ULONG_TO_UBASE(0x514A0879),
    HN_ULONG_TO_UBASE(0x3B139B22), HN_ULONG_TO_UBASE(0x020BBEA6),
    HN_ULONG_TO_UBASE(0x8A67CC74), HN_ULONG_TO_UBASE(0x29024E08),
    HN_ULONG_TO_UBASE(0x80DC1CD1), HN_ULONG_TO_UBASE(0xC4C6628B),
    HN_ULONG_TO_UBASE(0x2168C234), HN_ULONG_TO_UBASE(0xC90FDAA2),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_dh_setup                                 PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up a Diffie-Hellman context by generating a      */
/*    local.                                                              */
/*                                                                        */
/*    Note: The scratch buffer must be able to hold a number of *bytes*   */
/*          at least equal to NX_CRYPTO_DIFFIE_HELLMAN_SCRATCH_SIZE.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dh_ptr                                Diffie-Hellman context        */
/*    local_public_key                      Pointer to local public key   */
/*    local_public_key_len                  Local public key length       */
/*    dh_group_num                          Chosen DH group number        */
/*    scratch_buf_ptr                       Pointer to scratch buffer,    */
/*                                            which cannot be smaller     */
/*                                            than 6 times of the key     */
/*                                            size (in bytes). This       */
/*                                            scratch buffer can be       */
/*                                            reused after this function  */
/*                                            returns.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                          number to remove leading      */
/*                                          zeroes                        */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
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
NX_CRYPTO_KEEP UINT _nx_crypto_dh_setup(NX_CRYPTO_DH  *dh_ptr,
                                        UCHAR  *local_public_key_ptr,
                                        UINT   *local_public_key_len_ptr,
                                        ULONG   dh_group_num,
                                        HN_UBASE *scratch_buf_ptr)
{

UINT i;


/* Actual huge numbers used in calculations */
NX_CRYPTO_HUGE_NUMBER modulus, public_key, private_key, generator;
HN_UBASE              generator_value;

    NX_CRYPTO_STATE_CHECK

    /* Assign the desired key size based on the chosen Diffie-Hellman group. */
    switch (dh_group_num)
    {
    case NX_CRYPTO_DH_GROUP_2:
        dh_ptr -> nx_crypto_dh_key_size = NX_CRYPTO_DIFFIE_HELLMAN_GROUP_2_KEY_SIZE;
        dh_ptr -> nx_crypto_dh_modulus = (HN_UBASE *)_nx_dh_group_2_modulus;

        NX_CRYPTO_HUGE_NUMBER_INITIALIZE_DIGIT(&generator, &generator_value,
                                               NX_CRYPTO_DH_GROUP_2_GENERATOR)

        /* Setup the modulus value. */
        modulus.nx_crypto_huge_number_data = dh_ptr -> nx_crypto_dh_modulus;
        modulus.nx_crypto_huge_number_size =  dh_ptr -> nx_crypto_dh_key_size >> HN_SIZE_SHIFT;
        modulus.nx_crypto_huge_buffer_size =  dh_ptr -> nx_crypto_dh_key_size;
        modulus.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;


        break;

    default:
        /* No supported group specified - error! */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Local pointer for pointer arithmetic.  The power-modulus operation does not require the scratch area
       to be "clean". Therefore no need to zero out the buffer.

       Public key buffer (and scratch). */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&public_key, scratch_buf_ptr, dh_ptr -> nx_crypto_dh_key_size << 1);

    /* Private key buffer - note that no scratch is required for the private key. */
    private_key.nx_crypto_huge_number_data = dh_ptr -> nx_crypto_dh_private_key_buffer;
    private_key.nx_crypto_huge_number_size = dh_ptr -> nx_crypto_dh_key_size >> HN_SIZE_SHIFT;
    private_key.nx_crypto_huge_buffer_size = sizeof(dh_ptr -> nx_crypto_dh_private_key_buffer);
    private_key.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;


    /* Generate the private key. */
    for (i = 0; i < private_key.nx_crypto_huge_number_size; i++)
    {
        /* Grab a random value - this may be more than one byte and we want to use
           all the bytes in the value, so we do not simply copy the random_value
           into the buffers. */
        dh_ptr -> nx_crypto_dh_private_key_buffer[i] = (HN_UBASE)((HN_UBASE)(NX_CRYPTO_RAND()) & HN_MASK);
    }

    /* Finally, generate the public key from the private key, modulus, and the generator.
       The actual calculation is "public_key = (generator**private_key) % modulus"
       where the "**" denotes exponentiation. */
    _nx_crypto_huge_number_mont_power_modulus(&generator, &private_key,
                                              &modulus, &public_key, scratch_buf_ptr);

    /* Copy the public key into the return buffer. */
    _nx_crypto_huge_number_extract(&public_key, local_public_key_ptr,
                                   dh_ptr -> nx_crypto_dh_key_size, local_public_key_len_ptr);

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_dh_compute_secret                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes the Diffie-Hellman shared secret using an    */
/*    existing Diffie-Hellman context and a public key received from a    */
/*    remote entity, usually as part of an IPSEC or SSL key exchange.     */
/*                                                                        */
/*    Note: The scratch buffer must be able to hold a number of *bytes*   */
/*          at least equal to NX_CRYPTO_DIFFIE_HELLMAN_SCRATCH_SIZE.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dh_ptr                                Diffie-Hellman context        */
/*    share_secret_key_ptr                  Shared secret buffer pointer  */
/*    share_secret_key_len_ptr              Length of shared secret       */
/*    remote_public_key                     Pointer to remote public key  */
/*    remote_public_key_len                 Remote public key length      */
/*    scratch_buf_ptr                       Pointer to scratch buffer,    */
/*                                            which cannot be smaller     */
/*                                            than 8 times of the key     */
/*                                            size (in bytes). This       */
/*                                            scratch buffer can be       */
/*                                            reused after this function  */
/*                                            returns.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
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
NX_CRYPTO_KEEP UINT _nx_crypto_dh_compute_secret(NX_CRYPTO_DH  *dh_ptr,
                                                 UCHAR  *share_secret_key_ptr,
                                                 ULONG  *share_secret_key_len_ptr,
                                                 UCHAR  *remote_public_key,
                                                 ULONG   remote_public_key_len,
                                                 HN_UBASE *scratch_buf_ptr)
{

UINT key_size;

/* Actual huge numbers used in calculations */
NX_CRYPTO_HUGE_NUMBER modulus, public_key, private_key, shared_secret;

    NX_CRYPTO_STATE_CHECK

    /* Make sure the key size was assigned before we do anything else. Generally, this means
       _nx_crypto_dh_setup was not called to set up the NX_DH structure prior to this call.  */
    if (0 == dh_ptr -> nx_crypto_dh_key_size)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Make sure the remote public key is small enough to fit into the huge number buffer. */
    if (remote_public_key_len > dh_ptr -> nx_crypto_dh_key_size)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Figure out the sizes of our keys and buffers. We need 2X the key size for our buffer space. */
    key_size = dh_ptr -> nx_crypto_dh_key_size;

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&public_key, scratch_buf_ptr, key_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&shared_secret, scratch_buf_ptr, key_size << 1);

    /* Copy the remote public key from the caller's buffer. */
    _nx_crypto_huge_number_setup(&public_key, remote_public_key, remote_public_key_len);



    /* Set up each of the buffers - point into the scratch buffer at increments of the DH buffer size. */
    modulus.nx_crypto_huge_number_data = (HN_UBASE *)dh_ptr -> nx_crypto_dh_modulus;
    modulus.nx_crypto_huge_number_size = key_size >> HN_SIZE_SHIFT;
    modulus.nx_crypto_huge_buffer_size = key_size;
    modulus.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;


    /* Private key buffer - note that no scratch is required for the private key, but we set it in case
       it is needed in the future. */
    private_key.nx_crypto_huge_number_data = (HN_UBASE *)dh_ptr -> nx_crypto_dh_private_key_buffer;
    private_key.nx_crypto_huge_number_size = key_size >> HN_SIZE_SHIFT;
    private_key.nx_crypto_huge_buffer_size = key_size;
    private_key.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

    /* Finally, generate shared secret from the remote public key, our generated private key, and the modulus, modulus.
       The actual calculation is "shared_secret = (public_key**private_key) % modulus"
       where the "**" denotes exponentiation. */
    _nx_crypto_huge_number_mont_power_modulus(&public_key, &private_key, &modulus,
                                              &shared_secret, scratch_buf_ptr);

    /* Now we have a shared secret to return to the caller. Check to make sure the buffer is large enough to hold the public key. */
    if (*share_secret_key_len_ptr < key_size)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* The public key size is simply the key size for this group. */

    /* Copy the shared secret into the return buffer. */
    _nx_crypto_huge_number_extract(&shared_secret, share_secret_key_ptr,
                                   key_size, (UINT *)share_secret_key_len_ptr);

    return(NX_CRYPTO_SUCCESS);
}

#endif
