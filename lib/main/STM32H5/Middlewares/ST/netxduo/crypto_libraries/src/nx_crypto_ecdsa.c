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
/**   Elliptic Curve Digital Signature Algorithm (ECDSA)                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_ecdsa.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdsa_sign                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes a signature of the hash data using the       */
/*    private key.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Curve used in the ECDSA       */
/*    hash                                  Hash data to be signed        */
/*    hash_length                           Length of hash data           */
/*    private_key                           Pointer to EC private key     */
/*    signature                             Pointer to signature output   */
/*    scratch                               Pointer to scratch buffer.    */
/*                                            This scratch buffer can be  */
/*                                            reused after this function  */
/*                                            returns.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ec_key_pair_generation_extra                             */
/*                                          Generate EC Key Pair          */
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
/*                                            verified memmove use cases, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ecdsa_sign(NX_CRYPTO_EC *curve, UCHAR *hash, UINT hash_length,
                                          UCHAR *private_key, UINT private_key_length,
                                          UCHAR *signature, ULONG signature_length,
                                          ULONG *actual_signature_length, HN_UBASE *scratch)
{
UINT                  status;
NX_CRYPTO_HUGE_NUMBER privkey;
NX_CRYPTO_HUGE_NUMBER z;
NX_CRYPTO_HUGE_NUMBER k;
NX_CRYPTO_HUGE_NUMBER ik;
NX_CRYPTO_HUGE_NUMBER temp;
NX_CRYPTO_EC_POINT    pt;
UINT                  buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;
UINT                  i;
UINT                  curve_size;
UINT                  r_size;
UINT                  s_size;
UCHAR                 pad_zero_r;
UCHAR                 pad_zero_s;
UINT                  sequence_size;
UCHAR                *signature_r;
UCHAR                *signature_s;

    /* Signature format follows ASN1 DER encoding as per RFC 4492, section 5.8:
     * Size: 1   | 1 or 2 | 1   |   1   | 0 or 1 | N |  1  |  1   | 0 or 1 | M
     * Data: SEQ |  Size  | INT |  Size | 0x00   | r | INT | Size | 0x00   | s  */

    curve_size = curve -> nx_crypto_ec_bits >> 3;
    if (curve -> nx_crypto_ec_bits & 7)
    {
        curve_size++;
    }

    /* Check the signature_length for worst case. */
    if (signature_length < ((curve_size << 1) + 9))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    if (private_key_length > buffer_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&privkey, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&z, scratch, buffer_size);
    /* Initialize per-message secret k buffer with 64 bits more buffer size. */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&k, scratch, buffer_size + 8);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&ik, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch, buffer_size * 2);
    NX_CRYPTO_EC_POINT_INITIALIZE(&pt, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);

    /* Copy the private key from the caller's buffer. */
    status = _nx_crypto_huge_number_setup(&privkey, private_key, private_key_length);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Truncate the hash data to the size of group order. */
    if (hash_length > buffer_size)
    {
        hash_length = buffer_size;
    }

    status = _nx_crypto_huge_number_setup(&z, hash, hash_length);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* The bit length of the hash data used is not longer than the group order. */
    if (curve -> nx_crypto_ec_bits < hash_length << 3)
    {
        _nx_crypto_huge_number_shift_right(&z, (hash_length << 3) - curve -> nx_crypto_ec_bits);
    }

    do
    {
        do
        {
            /* Generate Key Pair. */
            _nx_crypto_ec_key_pair_generation_extra(curve, &curve -> nx_crypto_ec_g, &k, &pt,
                                                    scratch);

            /* Calculate r = pt.x mod n */
            _nx_crypto_huge_number_modulus(&pt.nx_crypto_ec_point_x, &curve -> nx_crypto_ec_n);

        } while (_nx_crypto_huge_number_is_zero(&pt.nx_crypto_ec_point_x));

        /* Calculate s = k^-1 * (z + r * private_key) */
        _nx_crypto_huge_number_inverse_modulus(&k, &curve -> nx_crypto_ec_n, &ik, scratch);
        _nx_crypto_huge_number_multiply(&pt.nx_crypto_ec_point_x, &privkey, &temp);
        _nx_crypto_huge_number_add_unsigned(&temp, &z);
        _nx_crypto_huge_number_modulus(&temp, &curve -> nx_crypto_ec_n);
        NX_CRYPTO_HUGE_NUMBER_COPY(&k, &temp);
        _nx_crypto_huge_number_multiply(&ik, &k, &temp);
        _nx_crypto_huge_number_modulus(&temp, &curve -> nx_crypto_ec_n);

    } while (_nx_crypto_huge_number_is_zero(&temp));

    /* Output r and s as two INTEGER in ASN.1 encoding */
    signature_r = signature + 3;
    status = _nx_crypto_huge_number_extract(&pt.nx_crypto_ec_point_x, signature_r, (curve_size + 3), &r_size);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    signature_s = signature + (curve_size + 6);
    status = _nx_crypto_huge_number_extract(&temp, signature_s, (curve_size + 3), &s_size);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Trim prefix zeros. */
    for (i = 0; i < r_size; i++)
    {
        if (signature_r[i])
        {

            /* Loop until none zero byte. */
            break;
        }
    }
    signature_r += i;
    r_size -= i;

    /* The most significant bit must be zero to indicate positive integer. */
    /* Pad zero at the front if necessary. */
    pad_zero_r = (signature_r[0] & 0x80) ? 1 : 0;

    for (i = 0; i < s_size; i++)
    {
        if (signature_s[i])
        {

            /* Loop until none zero byte. */
            break;
        }
    }
    signature_s += i;
    s_size -= i;

    /* The most significant bit must be zero to indicate positive integer. */
    /* Pad zero at the front if necessary. */
    pad_zero_s = (signature_s[0] & 0x80) ? 1 : 0;

    /* Size of sequence. */
    sequence_size = r_size + pad_zero_r + s_size + pad_zero_s + 4;

    signature[0] = 0x30;    /* SEQUENCE */
    if (sequence_size < 0x80)
    {
        signature[1] = (UCHAR)sequence_size;
        signature += 2;
        *actual_signature_length = sequence_size + 2;
    }
    else
    {
        signature[1] = 0x81;
        signature[2] = (UCHAR)sequence_size;
        signature += 3;
        *actual_signature_length = sequence_size + 3;
    }

    /* Setup r. */
    NX_CRYPTO_MEMMOVE(&signature[2 + pad_zero_r], signature_r, r_size); /* Use case of memmove is verified. */
    signature[0] = 0x02;    /* Integer */
    signature[1] = (UCHAR)(r_size + pad_zero_r);
    if (pad_zero_r)
    {
        signature[2] = 0;
    }
    signature += (2u + pad_zero_r + r_size);

    /* Setup s. */
    NX_CRYPTO_MEMMOVE(&signature[2 + pad_zero_s], signature_s, s_size); /* Use case of memmove is verified. */
    signature[0] = 0x02;    /* Integer */
    signature[1] = (UCHAR)(s_size + pad_zero_s);
    if (pad_zero_s)
    {
        signature[2] = 0;
    }

    return NX_CRYPTO_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdsa_verify                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the signature of the hash data using the     */
/*    public key.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Curve used in the ECDSA       */
/*    hash                                  Hash data to be verified      */
/*    hash_length                           Length of hash data           */
/*    public_key                            Pointer to EC public key      */
/*    signature                             Signature to be verified      */
/*    scratch                               Pointer to scratch buffer.    */
/*                                            This scratch buffer can be  */
/*                                            reused after this function  */
/*                                            returns.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_setup          Generate private key          */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), and      */
/*                                            fixed input validation,     */
/*                                            added public key validation,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ecdsa_verify(NX_CRYPTO_EC *curve, UCHAR *hash, UINT hash_length,
                                            UCHAR *public_key, UINT public_key_length,
                                            UCHAR *signature, UINT signature_length, HN_UBASE *scratch)
{
UINT                  status;
NX_CRYPTO_HUGE_NUMBER r;
NX_CRYPTO_HUGE_NUMBER s;
NX_CRYPTO_HUGE_NUMBER z;
NX_CRYPTO_HUGE_NUMBER w;
NX_CRYPTO_HUGE_NUMBER u1;
NX_CRYPTO_HUGE_NUMBER u2;
NX_CRYPTO_EC_POINT    pubkey;
NX_CRYPTO_EC_POINT    pt;
NX_CRYPTO_EC_POINT    pt2;
UINT                  buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;

    /* Signature format follows ASN1 DER encoding as per RFC 4492, section 5.8:
     * Size: 1   | 1 or 2 | 1   |   1   | 0 or 1 | N |  1  |  1   | 0 or 1 | M
     * Data: SEQ |  Size  | INT |  Size | 0x00   | r | INT | Size | 0x00   | s  */


    if (public_key_length > 1 + (buffer_size << 1))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Buffer should contain the signature data sequence. */
    if (signature[0] != 0x30)
    {
        return(NX_CRYPTO_AUTHENTICATION_FAILED);
    }

    /* Check the size in SEQUENCE.  */
    if (signature[1] & 0x80)
    {
        if (signature_length < (signature[2] + 3u))
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }
        signature_length = signature[2];
        signature += 3;
    }
    else
    {
        if (signature_length < (signature[1] + 2u))
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }
        signature_length = signature[1];
        signature += 2;
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&r, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&s, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&z, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&w, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&u1, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&u2, scratch, buffer_size << 1);
    NX_CRYPTO_EC_POINT_INITIALIZE(&pubkey, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&pt, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&pt2, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);

    /* Copy the public key from the caller's buffer. */
    status = _nx_crypto_ec_point_setup(&pubkey, public_key, public_key_length);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

#ifndef NX_CRYPTO_ECC_DISABLE_KEY_VALIDATION
    status = _nx_crypto_ec_validate_public_key(&pubkey, curve, NX_CRYPTO_FALSE, scratch);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }
#endif /* NX_CRYPTO_ECC_DISABLE_KEY_VALIDATION */

    if (signature_length < (signature[1] + 2u))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Read r value from input signature. */
    status = _nx_crypto_huge_number_setup(&r, &signature[2], signature[1]);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }
    signature_length -= (signature[1] + 2u);
    signature += signature[1] + 2;

    if (signature_length < (signature[1] + 2u))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Read s value from input signature. */
    status = _nx_crypto_huge_number_setup(&s, &signature[2], signature[1]);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* r and s must be in the range [1..n-1] */
    if(_nx_crypto_huge_number_is_zero(&r) || _nx_crypto_huge_number_is_zero(&s))
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if(NX_CRYPTO_HUGE_NUMBER_LESS != _nx_crypto_huge_number_compare_unsigned(&r, &curve -> nx_crypto_ec_n))
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if(NX_CRYPTO_HUGE_NUMBER_LESS != _nx_crypto_huge_number_compare_unsigned(&s, &curve -> nx_crypto_ec_n))
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Truncate the hash data to the size of group order. */
    if (hash_length > buffer_size)
    {
        hash_length = buffer_size;
    }

    status = _nx_crypto_huge_number_setup(&z, hash, hash_length);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (curve -> nx_crypto_ec_bits < hash_length << 3)
    {
        _nx_crypto_huge_number_shift_right(&z, (hash_length << 3) - curve -> nx_crypto_ec_bits);
    }

    /* Calculate w = s^-1 mod n */
    _nx_crypto_huge_number_inverse_modulus(&s, &curve -> nx_crypto_ec_n, &w, scratch);

    /* Calculate u1 = zw mod n */
    _nx_crypto_huge_number_multiply(&z, &w, &u1);
    _nx_crypto_huge_number_modulus(&u1, &curve -> nx_crypto_ec_n);

    /* Calculate u2 = rw mod n */
    _nx_crypto_huge_number_multiply(&r, &w, &u2);
    _nx_crypto_huge_number_modulus(&u2, &curve -> nx_crypto_ec_n);

    /* Calculate (x1,y1) = u1*G + u2*public_key */
    curve -> nx_crypto_ec_multiple(curve, &curve -> nx_crypto_ec_g, &u1, &pt, scratch);
    curve -> nx_crypto_ec_multiple(curve, &pubkey, &u2, &pt2, scratch);

    curve -> nx_crypto_ec_add(curve, &pt, &pt2, scratch);

    _nx_crypto_huge_number_modulus(&pt.nx_crypto_ec_point_x, &curve -> nx_crypto_ec_n);

    /* Check r == x1 mod n */
    if (NX_CRYPTO_HUGE_NUMBER_EQUAL != _nx_crypto_huge_number_compare_unsigned(&pt.nx_crypto_ec_point_x, &r))
    {
        return NX_CRYPTO_NOT_SUCCESSFUL;
    }
    return NX_CRYPTO_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdsa_init                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported ECDSA cryptographic algorithm.                  */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_ecdsa_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                  UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                  VOID  **handle,
                                                  VOID  *crypto_metadata,
                                                  ULONG crypto_metadata_size)
{
NX_CRYPTO_ECDSA *ecdsa;

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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_ECDSA))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ecdsa = (NX_CRYPTO_ECDSA *)crypto_metadata;

    /* Reset ECDSA metadata. */
    ecdsa -> nx_crypto_ecdsa_curve = NX_CRYPTO_NULL;
    ecdsa -> nx_crypto_ecdsa_hash_method = NX_CRYPTO_NULL;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdsa_cleanup                     PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_ecdsa_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_ECDSA));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdsa_operation                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs an ECDSA operation.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    ECDSA operation               */
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
/*    _nx_crypto_ecdsa_sign                 Sign using ECDSA              */
/*    _nx_crypto_ecdsa_verify               Verify ECDSA signature        */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ecdsa_operation(UINT op,
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
NX_CRYPTO_ECDSA *ecdsa;
UINT             status = NX_CRYPTO_SUCCESS;
NX_CRYPTO_EXTENDED_OUTPUT
                *extended_output;
NX_CRYPTO_METHOD *hash_method;
VOID            *hash_handler = NX_CRYPTO_NULL;
UCHAR           *hash_output = NX_CRYPTO_NULL;

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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_ECDSA))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ecdsa = (NX_CRYPTO_ECDSA *)crypto_metadata;

    if (op == NX_CRYPTO_EC_CURVE_SET)
    {
        status = ((NX_CRYPTO_METHOD *)input) -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_GET,
                                                                    NX_CRYPTO_NULL,
                                                                    (NX_CRYPTO_METHOD *)input,
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL,
                                                                    (UCHAR *)&ecdsa -> nx_crypto_ecdsa_curve,
                                                                    sizeof(NX_CRYPTO_METHOD *),
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL, NX_CRYPTO_NULL);

        if (status)
        {
            return(status);
        }
    }
    else if (op == NX_CRYPTO_AUTHENTICATE)
    {
        if ((key == NX_CRYPTO_NULL) || (ecdsa -> nx_crypto_ecdsa_curve == NX_CRYPTO_NULL))
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;

        status = _nx_crypto_ecdsa_sign(ecdsa -> nx_crypto_ecdsa_curve,
                                       input,
                                       input_length_in_byte,
                                       key,
                                       key_size_in_bits >> 3,
                                       extended_output -> nx_crypto_extended_output_data,
                                       extended_output -> nx_crypto_extended_output_length_in_byte,
                                       &extended_output -> nx_crypto_extended_output_actual_size,
                                       ecdsa -> nx_crypto_ecdsa_scratch_buffer);
    }
    else if (op == NX_CRYPTO_VERIFY)
    {
        if (key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        status = _nx_crypto_ecdsa_verify(ecdsa->nx_crypto_ecdsa_curve,
                                         input,
                                         input_length_in_byte,
                                         key,
                                         key_size_in_bits >> 3,
                                         output, output_length_in_byte,
                                         ecdsa -> nx_crypto_ecdsa_scratch_buffer);
    }
    else if (op == NX_CRYPTO_EC_KEY_PAIR_GENERATE)
    {
        if (ecdsa->nx_crypto_ecdsa_curve == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;
        status = _nx_crypto_ec_key_pair_stream_generate(ecdsa->nx_crypto_ecdsa_curve,
                                                        extended_output -> nx_crypto_extended_output_data,
                                                        extended_output -> nx_crypto_extended_output_length_in_byte,
                                                        &extended_output -> nx_crypto_extended_output_actual_size,
                                                        ecdsa -> nx_crypto_ecdsa_scratch_buffer);
    }
    else if (op == NX_CRYPTO_HASH_METHOD_SET)
    {

        /* Setup hash method used by ECDSA. */
        hash_method = (NX_CRYPTO_METHOD *)input;

        /* ECDSA scratch buffer shares with metadata of hash method.
         * Check the size required by metadata of hash method. */
        if ((hash_method -> nx_crypto_metadata_area_size +
             (hash_method -> nx_crypto_ICV_size_in_bits >> 3)) >
            sizeof(ecdsa -> nx_crypto_ecdsa_scratch_buffer))
        {
            status = NX_CRYPTO_SIZE_ERROR;
        }
        else
        {
            ecdsa -> nx_crypto_ecdsa_hash_method = hash_method;
        }
    }
    else if ((op == NX_CRYPTO_SIGNATURE_GENERATE) || (op == NX_CRYPTO_SIGNATURE_VERIFY))
    {
        hash_method = ecdsa -> nx_crypto_ecdsa_hash_method;
        if (hash_method == NX_CRYPTO_NULL)
        {

            /* Hash method is not set successfully. */
            status = NX_CRYPTO_PTR_ERROR;
        }
        else
        {

            /* Put the hash at the end of scratch buffer. */
            hash_output = (UCHAR *)(ecdsa -> nx_crypto_ecdsa_scratch_buffer) +
                (sizeof(ecdsa -> nx_crypto_ecdsa_scratch_buffer) - (hash_method -> nx_crypto_ICV_size_in_bits >> 3));

            /* First, calculate hash value of input message. */
            if (hash_method -> nx_crypto_init)
            {
                status = hash_method -> nx_crypto_init(hash_method,
                                                       NX_CRYPTO_NULL,
                                                       0,
                                                       &hash_handler,
                                                       ecdsa -> nx_crypto_ecdsa_scratch_buffer,
                                                       hash_method -> nx_crypto_metadata_area_size);
            }

            if (status == NX_CRYPTO_SUCCESS)
            {
                status = hash_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                                            NX_CRYPTO_NULL,
                                                            hash_method,
                                                            NX_CRYPTO_NULL,
                                                            0,
                                                            input,
                                                            input_length_in_byte,
                                                            NX_CRYPTO_NULL,
                                                            hash_output,
                                                            (hash_method -> nx_crypto_ICV_size_in_bits >> 3),
                                                            ecdsa -> nx_crypto_ecdsa_scratch_buffer,
                                                            hash_method -> nx_crypto_metadata_area_size,
                                                            NX_CRYPTO_NULL, NX_CRYPTO_NULL);

                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            if (hash_method -> nx_crypto_cleanup)
            {
                status = hash_method -> nx_crypto_cleanup(ecdsa -> nx_crypto_ecdsa_scratch_buffer);
            }
        }

        if (status == NX_CRYPTO_SUCCESS)
        {

            /* Second, generate/verify signature. */
            if ((key == NX_CRYPTO_NULL) || (ecdsa -> nx_crypto_ecdsa_curve == NX_CRYPTO_NULL))
            {
                status = NX_CRYPTO_PTR_ERROR;
            }
            else if (op == NX_CRYPTO_SIGNATURE_GENERATE)
            {

                /* Signature generation. */
                extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;

                status = _nx_crypto_ecdsa_sign(ecdsa -> nx_crypto_ecdsa_curve,
                                               hash_output,
                                               (hash_method -> nx_crypto_ICV_size_in_bits >> 3),
                                               key,
                                               key_size_in_bits >> 3,
                                               extended_output -> nx_crypto_extended_output_data,
                                               extended_output -> nx_crypto_extended_output_length_in_byte,
                                               &extended_output -> nx_crypto_extended_output_actual_size,
                                               ecdsa -> nx_crypto_ecdsa_scratch_buffer);
            }
            else
            {

                /* Signature verification. */
                status = _nx_crypto_ecdsa_verify(ecdsa->nx_crypto_ecdsa_curve,
                                                 hash_output,
                                                 (hash_method -> nx_crypto_ICV_size_in_bits >> 3),
                                                 key,
                                                 key_size_in_bits >> 3,
                                                 output, output_length_in_byte,
                                                 ecdsa -> nx_crypto_ecdsa_scratch_buffer);
            }
        }
    }
    else
    {
        status = NX_CRYPTO_NOT_SUCCESSFUL;
    }

    return(status);
}
