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
/**   ECJPAKE                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_ecjpake.h"
#include "nx_crypto_huge_number.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_init                             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the ECJPAKE context.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecjpake                               Pointer to ECJPAKE context    */
/*    curve                                 Pointer to curve              */
/*    hash_method                           Hash method used by ECJPAKE   */
/*    hash_metadata                         Metadata of hash method       */
/*    hash_metadata_size                    Size of metadata              */
/*    scratch_pptr                          Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_ecjpake_operation   Initialize ECJPAKE crypto     */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ecjpake_init(NX_CRYPTO_ECJPAKE *ecjpake,
                                            NX_CRYPTO_EC *curve,
                                            NX_CRYPTO_METHOD *hash_method,
                                            VOID *hash_metadata,
                                            ULONG hash_metadata_size,
                                            HN_UBASE **scratch_pptr)
{
HN_UBASE *scratch_ptr = *scratch_pptr;
UINT      buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;

    ecjpake -> nx_crypto_ecjpake_curve = curve;
    ecjpake -> nx_crypto_ecjpake_hash_method = hash_method;
    ecjpake -> nx_crypto_ecjpake_hash_metadata_size = hash_metadata_size;
    if (hash_metadata == NX_CRYPTO_NULL)
    {
        ecjpake -> nx_crypto_ecjpake_hash_metadata = scratch_ptr;
        scratch_ptr += hash_metadata_size >> HN_SIZE_SHIFT;
    }
    else
    {
        ecjpake -> nx_crypto_ecjpake_hash_metadata = hash_metadata;
    }

    NX_CRYPTO_EC_POINT_INITIALIZE(&ecjpake -> nx_crypto_ecjpake_public_x1,
                                  NX_CRYPTO_EC_POINT_AFFINE, scratch_ptr, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&ecjpake -> nx_crypto_ecjpake_public_x2,
                                  NX_CRYPTO_EC_POINT_AFFINE, scratch_ptr, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&ecjpake -> nx_crypto_ecjpake_public_x3,
                                  NX_CRYPTO_EC_POINT_AFFINE, scratch_ptr, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&ecjpake -> nx_crypto_ecjpake_public_x4,
                                  NX_CRYPTO_EC_POINT_AFFINE, scratch_ptr, buffer_size);

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&ecjpake -> nx_crypto_ecjpake_private_x2, scratch_ptr, buffer_size);

    *scratch_pptr = scratch_ptr;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_hello_generate                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the message for TLS hello.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecjpake                               Pointer to ECJPAKE context    */
/*    id                                    Client or Server              */
/*    id_len                                Length of ID                  */
/*    output                                Output buffer                 */
/*    output_length                         Length of output buffer       */
/*    actual_size                           Actual size of output         */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_ec_point_extract_uncompressed                            */
/*                                          Extract point to byte stream  */
/*                                            in uncompressed format      */
/*    _nx_crypto_ecjpake_schnorr_zkp_generate                             */
/*                                          Perform Schnorr ZKP generation*/
/*    _nx_crypto_huge_number_extract_fixed_size                           */
/*                                          Extract huge number           */
/*    _nx_crypto_ec_key_pair_generation_extra                             */
/*                                          Generate EC Key Pair          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_ecjpake_operation   Initialize ECJPAKE crypto     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecjpake_hello_generate(NX_CRYPTO_ECJPAKE *ecjpake,
                                                      CHAR *id, UINT id_len,
                                                      UCHAR *output, ULONG output_length,
                                                      ULONG *actual_size,
                                                      HN_UBASE *scratch)
{
NX_CRYPTO_EC         *curve = ecjpake -> nx_crypto_ecjpake_curve;
UINT                  curve_size = (curve -> nx_crypto_ec_bits + 7) >> 3;
UINT                  buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;
NX_CRYPTO_HUGE_NUMBER private_key, r;
NX_CRYPTO_EC_POINT    v;
UINT                  total_length = 0;
UINT                  length;
UINT                  status;

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&private_key, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&r, scratch, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&v, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);

    status = _nx_crypto_ec_key_pair_generation_extra(curve, &curve -> nx_crypto_ec_g, &private_key,
                                            &ecjpake -> nx_crypto_ecjpake_public_x1, scratch);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    status = _nx_crypto_ecjpake_schnorr_zkp_generate(ecjpake -> nx_crypto_ecjpake_hash_method,
                                            ecjpake -> nx_crypto_ecjpake_hash_metadata,
                                            curve,
                                            &curve -> nx_crypto_ec_g,
                                            &v,
                                            &ecjpake -> nx_crypto_ecjpake_public_x1,
                                            id, id_len,
                                            &private_key,
                                            &r,
                                            scratch);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    _nx_crypto_ec_point_extract_uncompressed(curve,
                                             &ecjpake -> nx_crypto_ecjpake_public_x1,
                                             &output[total_length + 1],
                                             output_length - (total_length + 1),
                                             &length);
    output[total_length] = (UCHAR)length;
    total_length += (length + 1);
    _nx_crypto_ec_point_extract_uncompressed(curve,
                                             &v,
                                             &output[total_length + 1],
                                             output_length - (total_length + 1),
                                             &length);
    output[total_length] = (UCHAR)length;
    total_length += (length + 1);
    length = curve_size;
    status = _nx_crypto_huge_number_extract_fixed_size(&r,
                                              &output[total_length + 1],
                                              length);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    output[total_length] = (UCHAR)length;
    total_length += (length + 1);

    status = _nx_crypto_ec_key_pair_generation_extra(curve, &curve -> nx_crypto_ec_g,
                                            &ecjpake -> nx_crypto_ecjpake_private_x2,
                                            &ecjpake -> nx_crypto_ecjpake_public_x2, scratch);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    status = _nx_crypto_ecjpake_schnorr_zkp_generate(ecjpake -> nx_crypto_ecjpake_hash_method,
                                            ecjpake -> nx_crypto_ecjpake_hash_metadata,
                                            curve,
                                            &curve -> nx_crypto_ec_g,
                                            &v,
                                            &ecjpake -> nx_crypto_ecjpake_public_x2,
                                            id, id_len,
                                            &ecjpake -> nx_crypto_ecjpake_private_x2,
                                            &r,
                                            scratch);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    _nx_crypto_ec_point_extract_uncompressed(curve,
                                             &ecjpake -> nx_crypto_ecjpake_public_x2,
                                             &output[total_length + 1],
                                             output_length - (total_length + 1),
                                             &length);
    output[total_length] = (UCHAR)length;
    total_length += (length + 1);
    _nx_crypto_ec_point_extract_uncompressed(curve,
                                             &v,
                                             &output[total_length + 1],
                                             output_length - (total_length + 1),
                                             &length);
    output[total_length] = (UCHAR)length;
    total_length += (length + 1);
    length = curve_size;
    status = _nx_crypto_huge_number_extract_fixed_size(&r,
                                              &output[total_length + 1],
                                              length);
    output[total_length] = (UCHAR)length;

    *actual_size = total_length + length + 1;

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_hello_process                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the message for TLS hello.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecjpake                               Pointer to ECJPAKE context    */
/*    id                                    Client or Server              */
/*    id_len                                Length of ID                  */
/*    input                                 Input buffer                  */
/*    input_length                          Length of input buffer        */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_ec_point_setup             Set up point from byte steam  */
/*    _nx_crypto_ecjpake_schnorr_zkp_verify Perform Schnorr ZKP           */
/*                                            verification                */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_ecjpake_operation   Initialize ECJPAKE crypto     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecjpake_hello_process(NX_CRYPTO_ECJPAKE *ecjpake,
                                                     CHAR *id, UINT id_len,
                                                     UCHAR *input, UINT input_length,
                                                     HN_UBASE *scratch)
{
NX_CRYPTO_EC         *curve = ecjpake -> nx_crypto_ecjpake_curve;
UINT                  buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;
NX_CRYPTO_HUGE_NUMBER r;
NX_CRYPTO_EC_POINT    v;
UINT                  status;
UINT                  total_length = 0;

    NX_CRYPTO_PARAMETER_NOT_USED(input_length);

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&r, scratch, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&v, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);

    /* Setup X3. */
    status = _nx_crypto_ec_point_setup(&ecjpake -> nx_crypto_ecjpake_public_x3,
                                       &input[total_length + 1],
                                       (UINT)input[total_length]);
    if (status)
    {
        return(status);
    }

    total_length += (UINT)(input[total_length] + 1);

    /* Setup r and v. */
    status = _nx_crypto_ec_point_setup(&v, &input[total_length + 1], (UINT)input[total_length]);
    if (status)
    {
        return(status);
    }

    total_length += (UINT)(input[total_length] + 1);
    status = _nx_crypto_huge_number_setup(&r, &input[total_length + 1], (UINT)input[total_length]);
    if (status)
    {
        return(status);
    }

    total_length += (UINT)(input[total_length] + 1);

    status = _nx_crypto_ecjpake_schnorr_zkp_verify(ecjpake -> nx_crypto_ecjpake_hash_method,
                                                   ecjpake -> nx_crypto_ecjpake_hash_metadata,
                                                   curve,
                                                   &curve -> nx_crypto_ec_g,
                                                   &v,
                                                   &ecjpake -> nx_crypto_ecjpake_public_x3,
                                                   id, id_len,
                                                   &r,
                                                   scratch);
    if (status)
    {
        return(status);
    }

    /* Setup X4. */
    status = _nx_crypto_ec_point_setup(&ecjpake -> nx_crypto_ecjpake_public_x4,
                                       &input[total_length + 1],
                                       (UINT)input[total_length]);
    if (status)
    {
        return(status);
    }

    total_length += (UINT)(input[total_length] + 1);

    /* Setup r and v. */
    status = _nx_crypto_ec_point_setup(&v, &input[total_length + 1], (UINT)input[total_length]);
    if (status)
    {
        return(status);
    }

    total_length += (UINT)(input[total_length] + 1);
    status = _nx_crypto_huge_number_setup(&r, &input[total_length + 1], (UINT)input[total_length]);
    if (status)
    {
        return(status);
    }

    status = _nx_crypto_ecjpake_schnorr_zkp_verify(ecjpake -> nx_crypto_ecjpake_hash_method,
                                                   ecjpake -> nx_crypto_ecjpake_hash_metadata,
                                                   curve,
                                                   &curve -> nx_crypto_ec_g,
                                                   &v,
                                                   &ecjpake -> nx_crypto_ecjpake_public_x4,
                                                   id, id_len,
                                                   &r,
                                                   scratch);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_key_exchange_generate            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the message for TLS key exchange.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecjpake                               Pointer to ECJPAKE context    */
/*    shared_secret                         Pointer to shared secret      */
/*    shared_secret_len                     Length of shared secret       */
/*    id                                    Client or Server              */
/*    id_len                                Length of ID                  */
/*    output                                Output buffer                 */
/*    output_length                         Length of output buffer       */
/*    actual_size                           Actual size of output         */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_ec_point_extract_uncompressed                            */
/*                                          Extract point to byte stream  */
/*                                            in uncompressed format      */
/*    _nx_crypto_ecjpake_public_key_generate                              */
/*                                          Perform public key generation */
/*    _nx_crypto_ecjpake_schnorr_zkp_generate                             */
/*                                          Perform Schnorr ZKP generation*/
/*    _nx_crypto_huge_number_extract_fixed_size                           */
/*                                          Extract huge number           */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_ecjpake_operation   Initialize ECJPAKE crypto     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecjpake_key_exchange_generate(NX_CRYPTO_ECJPAKE *ecjpake,
                                                             UCHAR *shared_secret,
                                                             UINT shared_secret_len,
                                                             CHAR *id, UINT id_len,
                                                             UCHAR *output, ULONG output_length,
                                                             ULONG *actual_size,
                                                             HN_UBASE *scratch)
{
NX_CRYPTO_EC         *curve = ecjpake -> nx_crypto_ecjpake_curve;
UINT                  curve_size = (curve -> nx_crypto_ec_bits + 7) >> 3;
UINT                  buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;
NX_CRYPTO_HUGE_NUMBER private_key, r, s;
NX_CRYPTO_EC_POINT    ga;
NX_CRYPTO_EC_POINT    public_key, v;
UINT                  total_length = 0;
UINT                  length;
UINT                  status;

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&private_key, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&r, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&s, scratch,
                                     (shared_secret_len + 3) & (ULONG) ~3);
    NX_CRYPTO_EC_POINT_INITIALIZE(&ga, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&public_key, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&v, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);

    _nx_crypto_huge_number_setup(&s, shared_secret, shared_secret_len);
    _nx_crypto_huge_number_modulus(&s, &curve -> nx_crypto_ec_n);

    _nx_crypto_ecjpake_public_key_generate(curve,
                                           &ecjpake -> nx_crypto_ecjpake_public_x1,
                                           &ecjpake -> nx_crypto_ecjpake_public_x3,
                                           &ecjpake -> nx_crypto_ecjpake_public_x4,
                                           &ecjpake -> nx_crypto_ecjpake_private_x2,
                                           &s,
                                           &ga,
                                           &public_key,
                                           &private_key,
                                           scratch);

    status = _nx_crypto_ecjpake_schnorr_zkp_generate(ecjpake -> nx_crypto_ecjpake_hash_method,
                                            ecjpake -> nx_crypto_ecjpake_hash_metadata,
                                            curve,
                                            &ga,
                                            &v,
                                            &public_key,
                                            id, id_len,
                                            &private_key,
                                            &r,
                                            scratch);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    _nx_crypto_ec_point_extract_uncompressed(curve,
                                             &public_key,
                                             &output[total_length + 1],
                                             output_length - (total_length + 1),
                                             &length);
    output[total_length] = (UCHAR)length;
    total_length += (length + 1);
    _nx_crypto_ec_point_extract_uncompressed(curve,
                                             &v,
                                             &output[total_length + 1],
                                             output_length - (total_length + 1),
                                             &length);
    output[total_length] = (UCHAR)length;
    total_length += (length + 1);
    length = curve_size;
    status = _nx_crypto_huge_number_extract_fixed_size(&r,
                                                       &output[total_length + 1],
                                                       length);
    output[total_length] = (UCHAR)length;

    *actual_size = total_length + length + 1;

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_key_exchange_process             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the message for TLS key exchange.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecjpake                               Pointer to ECJPAKE context    */
/*    shared_secret                         Pointer to shared secret      */
/*    shared_secret_len                     Length of shared secret       */
/*    id                                    Client or Server              */
/*    id_len                                Length of ID                  */
/*    input                                 Input buffer                  */
/*    input_length                          Length of input buffer        */
/*    pms                                   Input buffer                  */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_ec_point_setup             Set up point from byte steam  */
/*    _nx_crypto_ecjpake_pre_master_secret_generate                       */
/*    _nx_crypto_ecjpake_schnorr_zkp_verify Perform Schnorr ZKP           */
/*                                            verification                */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    [nx_crypto_ec_add]                    Perform addtion for EC        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_ecjpake_operation   Initialize ECJPAKE crypto     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecjpake_key_exchange_process(NX_CRYPTO_ECJPAKE *ecjpake,
                                                            UCHAR *shared_secret,
                                                            UINT shared_secret_len,
                                                            CHAR *id, UINT id_len,
                                                            UCHAR *input, UINT input_length,
                                                            UCHAR *pms,
                                                            HN_UBASE *scratch)
{
NX_CRYPTO_EC         *curve = ecjpake -> nx_crypto_ecjpake_curve;
UINT                  buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;
NX_CRYPTO_HUGE_NUMBER r, s;
NX_CRYPTO_EC_POINT    ga;
NX_CRYPTO_EC_POINT    public_key, v;
UINT                  status;
UINT                  total_length = 0;

    NX_CRYPTO_PARAMETER_NOT_USED(input_length);

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&r, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&s, scratch,
                                     (shared_secret_len + 3) & (ULONG) ~3);
    NX_CRYPTO_EC_POINT_INITIALIZE(&ga, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&public_key, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&v, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);

    status = _nx_crypto_huge_number_setup(&s, shared_secret, shared_secret_len);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    _nx_crypto_huge_number_modulus(&s, &curve -> nx_crypto_ec_n);

    /* G = X1 + X2 + X3 */
    NX_CRYPTO_HUGE_NUMBER_COPY(&ga.nx_crypto_ec_point_x,
                               &ecjpake -> nx_crypto_ecjpake_public_x1.nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&ga.nx_crypto_ec_point_y,
                               &ecjpake -> nx_crypto_ecjpake_public_x1.nx_crypto_ec_point_y);
    curve -> nx_crypto_ec_add(curve, &ga, &ecjpake -> nx_crypto_ecjpake_public_x2, scratch);
    curve -> nx_crypto_ec_add(curve, &ga, &ecjpake -> nx_crypto_ecjpake_public_x3, scratch);

    /* Setup public_key. */
    status = _nx_crypto_ec_point_setup(&public_key,
                              &input[total_length + 1],
                              (UINT)input[total_length]);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    total_length += (UINT)(input[total_length] + 1);

    /* Setup r and v. */
    _nx_crypto_ec_point_setup(&v, &input[total_length + 1], (UINT)input[total_length]);
    total_length += (UINT)(input[total_length] + 1);
    status = _nx_crypto_huge_number_setup(&r, &input[total_length + 1], (UINT)input[total_length]);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    

    status = _nx_crypto_ecjpake_schnorr_zkp_verify(ecjpake -> nx_crypto_ecjpake_hash_method,
                                                   ecjpake -> nx_crypto_ecjpake_hash_metadata,
                                                   curve,
                                                   &ga,
                                                   &v,
                                                   &public_key,
                                                   id, id_len,
                                                   &r,
                                                   scratch);
    if (status)
    {
        return(status);
    }

    status = _nx_crypto_ecjpake_pre_master_secret_generate(ecjpake -> nx_crypto_ecjpake_hash_method,
                                                           ecjpake -> nx_crypto_ecjpake_hash_metadata,
                                                           curve,
                                                           &ecjpake -> nx_crypto_ecjpake_public_x4,
                                                           &s,
                                                           &public_key,
                                                           &ecjpake -> nx_crypto_ecjpake_private_x2,
                                                           pms,
                                                           scratch);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_schnorr_zkp_hash                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs Schnorr ZKP hash calculation.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hash_method                           Hash method used by ECJPAKE   */
/*    hash_metadata                         Metadata of hash method       */
/*    curve                                 Pointer to curve              */
/*    g                                     Generator                     */
/*    v                                     ZKP ephemeral public key      */
/*    x                                     Public key to be verified     */
/*    h                                     Hash for output               */
/*    id                                    Client or Server              */
/*    id_len                                Length of ID                  */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ec_point_extract_uncompressed                            */
/*                                          Extract point to byte stream  */
/*                                            in uncompressed format      */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    [nx_crypto_operation]                 Crypto opeartion              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecjpake_schnorr_zkp_generate                             */
/*                                          Perform Schnorr ZKP generation*/
/*    _nx_crypto_ecjpake_schnorr_zkp_verify Perform Schnorr ZKP           */
/*                                            verification                */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecjpake_schnorr_zkp_hash(NX_CRYPTO_METHOD *hash_method,
                                                        VOID *hash_metadata,
                                                        NX_CRYPTO_EC *curve,
                                                        NX_CRYPTO_EC_POINT *g,
                                                        NX_CRYPTO_EC_POINT *v,
                                                        NX_CRYPTO_EC_POINT *x,
                                                        NX_CRYPTO_HUGE_NUMBER *h,
                                                        CHAR *id,
                                                        UINT id_len,
                                                        HN_UBASE *scratch)
{
UINT size;
UINT status;
VOID *handler;

    if (hash_method -> nx_crypto_init)
    {
        status = hash_method -> nx_crypto_init(hash_method,
                                      NX_CRYPTO_NULL,
                                      0,
                                      &handler,
                                      hash_metadata,
                                      hash_method -> nx_crypto_metadata_area_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     
    }

    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       NX_CRYPTO_NULL,
                                       0,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     

    size = g -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size +
           g -> nx_crypto_ec_point_y.nx_crypto_huge_buffer_size + 1;
    _nx_crypto_ec_point_extract_uncompressed(curve, g, (UCHAR *)scratch, size, &size);
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(size);
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       (UCHAR *)&size,
                                       sizeof(ULONG),
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     

    NX_CRYPTO_CHANGE_ULONG_ENDIAN(size);
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       (UCHAR *)scratch,
                                       size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    

    size = v -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size +
           v -> nx_crypto_ec_point_y.nx_crypto_huge_buffer_size + 1;
    _nx_crypto_ec_point_extract_uncompressed(curve, v, (UCHAR *)scratch, size, &size);
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(size);
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       (UCHAR *)&size,
                                       sizeof(ULONG),
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(size);
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       (UCHAR *)scratch,
                                       size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    

    size = x -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size +
           x -> nx_crypto_ec_point_y.nx_crypto_huge_buffer_size + 1;
    _nx_crypto_ec_point_extract_uncompressed(curve, x, (UCHAR *)scratch, size, &size);
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(size);
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       (UCHAR *)&size,
                                       sizeof(ULONG),
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(size);
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       (UCHAR *)scratch,
                                       size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(id_len);
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       (UCHAR *)&id_len,
                                       sizeof(ULONG),
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(id_len);
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       (UCHAR *)id,
                                       id_len,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       NX_CRYPTO_NULL,
                                       0,
                                       NX_CRYPTO_NULL,
                                       (UCHAR *)scratch,
                                       hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    

    status = _nx_crypto_huge_number_setup(h, (UCHAR *)scratch,
                                          hash_method -> nx_crypto_ICV_size_in_bits >> 3);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    _nx_crypto_huge_number_modulus(h, &curve -> nx_crypto_ec_n);

    if (hash_method -> nx_crypto_cleanup)
    {
        status = hash_method -> nx_crypto_cleanup(hash_metadata);
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_schnorr_zkp_generate             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs Schnorr ZKP generation.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hash_method                           Hash method used by ECJPAKE   */
/*    hash_metadata                         Metadata of hash method       */
/*    curve                                 Pointer to curve              */
/*    g                                     Generator                     */
/*    v                                     ZKP ephemeral public key      */
/*    public_key                            Public key generated          */
/*    id                                    Client or Server              */
/*    id_len                                Length of ID                  */
/*    private_key                           Private key generated         */
/*    r                                     Schnorr signature for output  */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_multiply       Multiply two huge numbers     */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                             huge numbers               */
/*    _nx_crypto_ecjpake_schnorr_zkp_hash   Perform Schnorr ZKP hash      */
/*                                            calculation                 */
/*    _nx_crypto_ec_key_pair_generation_extra                             */
/*                                          Generate EC Key Pair          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecjpake_hello_generate     Generate hello message        */
/*    _nx_crypto_ecjpake_key_exchange_generate                            */
/*                                          Generate key exchange message */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecjpake_schnorr_zkp_generate(NX_CRYPTO_METHOD *hash_method,
                                                            VOID *hash_metadata,
                                                            NX_CRYPTO_EC *curve,
                                                            NX_CRYPTO_EC_POINT *g,
                                                            NX_CRYPTO_EC_POINT *v,
                                                            NX_CRYPTO_EC_POINT *public_key,
                                                            CHAR *id,
                                                            UINT id_len,
                                                            NX_CRYPTO_HUGE_NUMBER *private_key,
                                                            NX_CRYPTO_HUGE_NUMBER *r,
                                                            HN_UBASE *scratch)
{
UINT                  buffer_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;
NX_CRYPTO_HUGE_NUMBER h, temp1, temp2;
UINT status;

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&h, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp1, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp2, scratch, buffer_size << 1);

    _nx_crypto_ec_key_pair_generation_extra(curve, g, &temp1, v, scratch);

    status = _nx_crypto_ecjpake_schnorr_zkp_hash(hash_method,
                                        hash_metadata,
                                        curve,
                                        g,
                                        v,
                                        public_key,
                                        &h,
                                        id,
                                        id_len,
                                        scratch);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }
    
    _nx_crypto_huge_number_multiply(&h, private_key, &temp2);
    _nx_crypto_huge_number_subtract(&temp1, &temp2);
    _nx_crypto_huge_number_modulus(&temp1, &curve -> nx_crypto_ec_n);
    NX_CRYPTO_HUGE_NUMBER_COPY(r, &temp1);
    
    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_schnorr_zkp_verify               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs Schnorr ZKP verification.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hash_method                           Hash method used by ECJPAKE   */
/*    hash_metadata                         Metadata of hash method       */
/*    curve                                 Pointer to curve              */
/*    g                                     Generator                     */
/*    v                                     ZKP ephemeral public key      */
/*    public_key                            Public key generated          */
/*    id                                    Client or Server              */
/*    id_len                                Length of ID                  */
/*    r                                     Schnorr signature             */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_ecjpake_schnorr_zkp_hash   Perform Schnorr ZKP hash      */
/*                                            calculation                 */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    [nx_crypto_ec_add]                    Perform addtion for EC        */
/*    [nx_crypto_ec_multiple]               Perform multiplication for EC */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecjpake_hello_process      Process hello message         */
/*    _nx_crypto_ecjpake_key_exchange_process                             */
/*                                          Process key exchange message  */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecjpake_schnorr_zkp_verify(NX_CRYPTO_METHOD *hash_method,
                                                          VOID *hash_metadata,
                                                          NX_CRYPTO_EC *curve,
                                                          NX_CRYPTO_EC_POINT *g,
                                                          NX_CRYPTO_EC_POINT *v,
                                                          NX_CRYPTO_EC_POINT *public_key,
                                                          CHAR *id,
                                                          UINT id_len,
                                                          NX_CRYPTO_HUGE_NUMBER *r,
                                                          HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER h;
NX_CRYPTO_EC_POINT    temp1, temp2;

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&h, scratch,
                                     curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&temp1,
                                  NX_CRYPTO_EC_POINT_AFFINE, scratch,
                                  public_key -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&temp2,
                                  NX_CRYPTO_EC_POINT_AFFINE, scratch,
                                  g -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size);

    _nx_crypto_ecjpake_schnorr_zkp_hash(hash_method,
                                        hash_metadata,
                                        curve,
                                        g,
                                        v,
                                        public_key,
                                        &h,
                                        id,
                                        id_len,
                                        scratch);

    curve -> nx_crypto_ec_multiple(curve, public_key, &h, &temp1, scratch);
    curve -> nx_crypto_ec_multiple(curve, g, r, &temp2, scratch);
    curve -> nx_crypto_ec_add(curve, &temp1, &temp2, scratch);

    if ((_nx_crypto_huge_number_compare(&temp1.nx_crypto_ec_point_x,
                                        &v -> nx_crypto_ec_point_x) == NX_CRYPTO_HUGE_NUMBER_EQUAL) &&
        (_nx_crypto_huge_number_compare(&temp1.nx_crypto_ec_point_y,
                                        &v -> nx_crypto_ec_point_y) == NX_CRYPTO_HUGE_NUMBER_EQUAL))
    {
        return(NX_CRYPTO_SUCCESS);
    }
    else
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_public_key_generate              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs public key generation.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    x1                                    Public key x1                 */
/*    x3                                    Public key x3                 */
/*    x4                                    Public key x4                 */
/*    x2                                    Private key x2                */
/*    s                                     Shared secret                 */
/*    g                                     Base point                    */
/*    public_key                            Public key for output         */
/*    private_key                           Private key for output        */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_multiply       Multiply two huge numbers     */
/*    [nx_crypto_ec_add]                    Perform addtion for EC        */
/*    [nx_crypto_ec_multiple]               Perform multiplication for EC */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecjpake_key_exchange_generate                            */
/*                                          Generate key exchange message */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ecjpake_public_key_generate(NX_CRYPTO_EC *curve,
                                                           NX_CRYPTO_EC_POINT *x1,
                                                           NX_CRYPTO_EC_POINT *x3,
                                                           NX_CRYPTO_EC_POINT *x4,
                                                           NX_CRYPTO_HUGE_NUMBER *x2,
                                                           NX_CRYPTO_HUGE_NUMBER *s,
                                                           NX_CRYPTO_EC_POINT *g,
                                                           NX_CRYPTO_EC_POINT *public_key,
                                                           NX_CRYPTO_HUGE_NUMBER *private_key,
                                                           HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp1;

    /* G = X1 + X3 + X4 */
    NX_CRYPTO_HUGE_NUMBER_COPY(&g -> nx_crypto_ec_point_x, &x1 -> nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&g -> nx_crypto_ec_point_y, &x1 -> nx_crypto_ec_point_y);
    curve -> nx_crypto_ec_add(curve, g, x3, scratch);
    curve -> nx_crypto_ec_add(curve, g, x4, scratch);

    /* private_key = x2 * s mod n */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp1, scratch,
                                     curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size << 1);
    _nx_crypto_huge_number_multiply(x2, s, &temp1);
    _nx_crypto_huge_number_modulus(&temp1, &curve -> nx_crypto_ec_n);
    NX_CRYPTO_HUGE_NUMBER_COPY(private_key, &temp1);

    /* public_key = G * xs */
    curve -> nx_crypto_ec_multiple(curve, g, private_key, public_key, scratch);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_pre_master_secret_generate       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs pre-master secret generation.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hash_method                           Hash method used by ECJPAKE   */
/*    hash_metadata                         Metadata of hash method       */
/*    curve                                 Pointer to curve              */
/*    x4                                    Public key x4                 */
/*    s                                     Shared secret                 */
/*    public_key                            Public key                    */
/*    x2                                    Private key x2                */
/*    pms                                   Pre-master key                */
/*    scratch                               Pointer to scratch            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_extract_fixed_size                           */
/*                                          Extract huge number           */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    [nx_crypto_ec_multiple]               Perform multiplication for EC */
/*    [nx_crypto_ec_subtract]               Perform subtraction for EC    */
/*    [nx_crypto_operation]                 Crypto opeartion              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecjpake_key_exchange_process                             */
/*                                          Process key exchange message  */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecjpake_pre_master_secret_generate(NX_CRYPTO_METHOD *hash_method,
                                                                  VOID *hash_metadata,
                                                                  NX_CRYPTO_EC *curve,
                                                                  NX_CRYPTO_EC_POINT *x4,
                                                                  NX_CRYPTO_HUGE_NUMBER *s,
                                                                  NX_CRYPTO_EC_POINT *public_key,
                                                                  NX_CRYPTO_HUGE_NUMBER *x2,
                                                                  UCHAR *pms,
                                                                  HN_UBASE *scratch)
{
NX_CRYPTO_EC_POINT temp1, temp2;
UCHAR             *input;
VOID              *handler;
UINT               input_size = (curve -> nx_crypto_ec_bits + 7) >> 3;
UINT               status;

    NX_CRYPTO_EC_POINT_INITIALIZE(&temp1,
                                  NX_CRYPTO_EC_POINT_AFFINE, scratch,
                                  public_key -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&temp2,
                                  NX_CRYPTO_EC_POINT_AFFINE, scratch,
                                  public_key -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size);
    input = (UCHAR *)scratch;
    scratch += input_size;

    /* PMSK = (public_key - X4 * x2 * s) * x2 */
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp1.nx_crypto_ec_point_x, &x4 -> nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp1.nx_crypto_ec_point_y, &x4 -> nx_crypto_ec_point_y);
    curve -> nx_crypto_ec_multiple(curve, &temp1, x2, &temp2, scratch);
    curve -> nx_crypto_ec_multiple(curve, &temp2, s, &temp1, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp2.nx_crypto_ec_point_x, &public_key -> nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp2.nx_crypto_ec_point_y, &public_key -> nx_crypto_ec_point_y);
    curve -> nx_crypto_ec_subtract(curve, &temp2, &temp1, scratch);
    curve -> nx_crypto_ec_multiple(curve, &temp2, x2, &temp1, scratch);

    /* PMS = SHA-256(str(32, X coordinate of PMSK)) */
    status = _nx_crypto_huge_number_extract_fixed_size(&temp1.nx_crypto_ec_point_x, input, input_size);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    
    if (hash_method -> nx_crypto_init)
    {
        status = hash_method -> nx_crypto_init(hash_method,
                                      NX_CRYPTO_NULL,
                                      0,
                                      &handler,
                                      hash_metadata,
                                      hash_method -> nx_crypto_metadata_area_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     
    }

    status = hash_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       input,
                                       input_size,
                                       NX_CRYPTO_NULL,
                                       pms,
                                       hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    

    if (hash_method -> nx_crypto_cleanup)
    {
        status = hash_method -> nx_crypto_cleanup(hash_metadata);
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecjpake_key_encryption_key_generate      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs key encryption key generation.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hash_method                           Hash method used by ECJPAKE   */
/*    hash_metadata                         Metadata of hash method       */
/*    key_expansion                         Pointer to key expansion      */
/*    key_expansion_len                     Length of key expansion       */
/*    key_encryption_key                    Key encryption key for output */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto opeartion              */
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
#ifndef NX_CRYPTO_SELF_TEST
UINT _nx_crypto_ecjpake_key_encryption_key_generate(NX_CRYPTO_METHOD *hash_method,
                                                    VOID *hash_metadata,
                                                    UCHAR *key_expansion,
                                                    UINT key_expansion_len,
                                                    UCHAR *key_encryption_key)
{
UCHAR buffer[32];
VOID *handler;
UINT status;

    if (hash_method -> nx_crypto_init)
    {
        status = hash_method -> nx_crypto_init(hash_method,
                                      NX_CRYPTO_NULL,
                                      0,
                                      &handler,
                                      hash_metadata,
                                      hash_method -> nx_crypto_metadata_area_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                    
    }

    /* KEK = SHA-256(key expansion)[0..15] */
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       key_expansion,
                                       key_expansion_len,
                                       NX_CRYPTO_NULL,
                                       buffer,
                                       hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                       hash_metadata,
                                       hash_method -> nx_crypto_metadata_area_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     
    

    NX_CRYPTO_MEMCPY(key_encryption_key, buffer, 16); /* Use case of memcpy is verified. */

    if (hash_method -> nx_crypto_cleanup)
    {
        status = hash_method -> nx_crypto_cleanup(hash_metadata);
    }

    return(status);
}
#endif
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecjpake_init                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the ECJPAKE crypto method.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Crypto Method Object          */
/*    key                                   Key                           */
/*    key_size_in_bits                      Size of the key, in bits      */
/*    handle                                Handle, specified by user     */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_ecjpake_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                                    UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                    VOID **handle,
                                                    VOID *crypto_metadata,
                                                    ULONG crypto_metadata_size)
{
NX_CRYPTO_ECJPAKE *ecjpake;
UINT               i;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    if (key_size_in_bits == 0)
    {
        /* PSK length must not be 0. */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if ((method == NX_CRYPTO_NULL) || (key == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_ECJPAKE))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    for (i = 0; i < (key_size_in_bits >> 3); i++)
    {
        if (key[i] == 0)
        {
            /* No NULL character in the PSK. */
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }

    NX_CRYPTO_MEMSET(crypto_metadata, 0, crypto_metadata_size);

    ecjpake = (NX_CRYPTO_ECJPAKE *)crypto_metadata;
    ecjpake -> nx_crypto_ecjpake_psk = key;
    ecjpake -> nx_crypto_ecjpake_psk_length = key_size_in_bits >> 3;
    ecjpake -> nx_crypto_ecjpake_scratch_ptr = (HN_UBASE *)ecjpake -> nx_crypto_ecjpake_scratch_buffer;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecjpake_cleanup                   PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_ecjpake_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_ECJPAKE));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecjpake_operation                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the ECJPAKE crypto method.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    ECJPAKE operation             */
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
/*    _nx_crypto_ecjpake_hello_generate     Generate hello message        */
/*    _nx_crypto_ecjpake_hello_process      Process hello message         */
/*    _nx_crypto_ecjpake_init               Initialize the ECJPAKE context*/
/*    _nx_crypto_ecjpake_key_exchange_process                             */
/*                                          Process key exchange message  */
/*    _nx_crypto_ecjpake_key_exchange_generate                            */
/*                                          Generate key exchange message */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ecjpake_operation(UINT op,
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
NX_CRYPTO_ECJPAKE          *ecjpake;
NX_CRYPTO_EXTENDED_OUTPUT  *extended_output;
CHAR                       *id = NX_CRYPTO_NULL;
UINT                        id_size = 0;
UINT                        status = NX_CRYPTO_SUCCESS;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_ECJPAKE))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if ((op == NX_CRYPTO_ECJPAKE_CLIENT_HELLO_GENERATE) ||
        (op == NX_CRYPTO_ECJPAKE_CLIENT_HELLO_PROCESS) ||
        (op == NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_GENERATE) ||
        (op == NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_PROCESS))
    {
        id = NX_CRYPTO_ECJPAKE_CLIENT_ID;
        id_size = sizeof(NX_CRYPTO_ECJPAKE_CLIENT_ID) - 1;
    }
    else if((op == NX_CRYPTO_ECJPAKE_SERVER_HELLO_GENERATE) ||
            (op == NX_CRYPTO_ECJPAKE_SERVER_HELLO_PROCESS) ||
            (op == NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_GENERATE) ||
            (op == NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_PROCESS))
    {
        id = NX_CRYPTO_ECJPAKE_SERVER_ID;
        id_size = sizeof(NX_CRYPTO_ECJPAKE_SERVER_ID) - 1;
    }

    ecjpake = (NX_CRYPTO_ECJPAKE *)crypto_metadata;

    if (op == NX_CRYPTO_ECJPAKE_HASH_METHOD_SET)
    {

        if (ecjpake -> nx_crypto_ecjpake_curve != NX_CRYPTO_NULL)
        {
            _nx_crypto_ecjpake_init(ecjpake, ecjpake -> nx_crypto_ecjpake_curve,
                                    (NX_CRYPTO_METHOD *)input, (VOID *)key,
                                    key_size_in_bits >> 3,
                                    &ecjpake -> nx_crypto_ecjpake_scratch_ptr);
        }
        else
        {
            ecjpake -> nx_crypto_ecjpake_hash_method = (NX_CRYPTO_METHOD *)input;
            ecjpake -> nx_crypto_ecjpake_hash_metadata = (VOID *)key;
            ecjpake -> nx_crypto_ecjpake_hash_metadata_size = key_size_in_bits >> 3;
        }
    }
    else if (op == NX_CRYPTO_ECJPAKE_CURVE_SET)
    {
        status = ((NX_CRYPTO_METHOD *)input) -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_GET,
                                                                    NX_CRYPTO_NULL,
                                                                    (NX_CRYPTO_METHOD *)input,
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL,
                                                                    (UCHAR *)&ecjpake -> nx_crypto_ecjpake_curve,
                                                                    sizeof(NX_CRYPTO_METHOD *),
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL, NX_CRYPTO_NULL);

        if (status)
        {
            return(status);
        }

        if (ecjpake -> nx_crypto_ecjpake_hash_method != NX_CRYPTO_NULL)
        {
            _nx_crypto_ecjpake_init(ecjpake, ecjpake -> nx_crypto_ecjpake_curve,
                                    ecjpake -> nx_crypto_ecjpake_hash_method,
                                    ecjpake -> nx_crypto_ecjpake_hash_metadata,
                                    ecjpake -> nx_crypto_ecjpake_hash_metadata_size,
                                    &ecjpake -> nx_crypto_ecjpake_scratch_ptr);
        }
    }
    else if ((op == NX_CRYPTO_ECJPAKE_CLIENT_HELLO_GENERATE) ||
             (op == NX_CRYPTO_ECJPAKE_SERVER_HELLO_GENERATE))
    {
        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;
        status = _nx_crypto_ecjpake_hello_generate(ecjpake,
                                          id, id_size,
                                          extended_output -> nx_crypto_extended_output_data,
                                          extended_output -> nx_crypto_extended_output_length_in_byte,
                                          &extended_output -> nx_crypto_extended_output_actual_size,
                                          ecjpake -> nx_crypto_ecjpake_scratch_ptr);
    }
    else if ((op == NX_CRYPTO_ECJPAKE_CLIENT_HELLO_PROCESS) ||
             (op == NX_CRYPTO_ECJPAKE_SERVER_HELLO_PROCESS))
    {
        status = _nx_crypto_ecjpake_hello_process(ecjpake,
                                                  id, id_size,
                                                  input, input_length_in_byte,
                                                  ecjpake -> nx_crypto_ecjpake_scratch_ptr);
    }
    else if ((op == NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_GENERATE) ||
             (op == NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_GENERATE))
    {
        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;
        status = _nx_crypto_ecjpake_key_exchange_generate(ecjpake,
                                                 ecjpake -> nx_crypto_ecjpake_psk,
                                                 ecjpake -> nx_crypto_ecjpake_psk_length,
                                                 id, id_size,
                                                 extended_output -> nx_crypto_extended_output_data,
                                                 extended_output -> nx_crypto_extended_output_length_in_byte,
                                                 &extended_output -> nx_crypto_extended_output_actual_size,
                                                 ecjpake -> nx_crypto_ecjpake_scratch_ptr);
    }
    else if ((op == NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_PROCESS) ||
             (op == NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_PROCESS))
    {
        status = _nx_crypto_ecjpake_key_exchange_process(ecjpake,
                                                         ecjpake -> nx_crypto_ecjpake_psk,
                                                         ecjpake -> nx_crypto_ecjpake_psk_length,
                                                         id, id_size,
                                                         input, input_length_in_byte,
                                                         output,
                                                         ecjpake -> nx_crypto_ecjpake_scratch_ptr);
    }
    else
    {
        status = NX_CRYPTO_NOT_SUCCESSFUL;
    }

    return(status);
}

