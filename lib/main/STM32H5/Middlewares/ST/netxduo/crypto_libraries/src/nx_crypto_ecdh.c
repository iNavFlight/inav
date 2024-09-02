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
/**   Elliptic-curve Diffie-Hellman (ECDH)                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_CRYPTO_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_crypto_ecdh.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_key_pair_import                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up a Elliptic-curve Diffie-Hellman context by    */
/*    importing a local key pair.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    curve                                 Elliptic Curve                */
/*    local_private_key_ptr                 Pointer to local private key  */
/*    local_private_key_len                 Local private key length      */
/*    local_public_key_ptr                  Pointer to local public key   */
/*    local_public_key_len                  Remote public key length      */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecdh_key_pair_import(NX_CRYPTO_ECDH  *ecdh_ptr,
                                                    NX_CRYPTO_EC *curve,
                                                    UCHAR  *local_private_key_ptr,
                                                    ULONG   local_private_key_len,
                                                    UCHAR  *local_public_key_ptr,
                                                    ULONG   local_public_key_len)
{
UINT public_key_len;
NX_CRYPTO_HUGE_NUMBER private_key;

    NX_CRYPTO_PARAMETER_NOT_USED(local_public_key_ptr);

    if (local_private_key_len > sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    public_key_len = 1 + (((curve -> nx_crypto_ec_bits + 7) >> 3) << 1);
    if (local_public_key_len > public_key_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Assign the desired key size based on the chosen elliptic curve. */
    ecdh_ptr -> nx_crypto_ecdh_key_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;

    /* Clear the private key buffer. */
    NX_CRYPTO_MEMSET(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer, 0,
                     sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer));

    /* Setup the private key. */
    private_key.nx_crypto_huge_number_data = ecdh_ptr -> nx_crypto_ecdh_private_key_buffer;
    private_key.nx_crypto_huge_buffer_size = sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer);
    _nx_crypto_huge_number_setup(&private_key, local_private_key_ptr, local_private_key_len);

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_private_key_export                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function exports the local private key in Elliptic-curve       */
/*    Diffie-Hellman context.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    local_private_key_ptr                 Pointer to local private key  */
/*    local_private_key_len                 Local private key length      */
/*    actual_local_private_key_len          Pointer to private key length */
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
UINT _nx_crypto_ecdh_private_key_export(NX_CRYPTO_ECDH  *ecdh_ptr,
                                        UCHAR  *local_private_key_ptr,
                                        ULONG   local_private_key_len,
                                        ULONG  *actual_local_private_key_len)
{
UINT                  status;
UINT                  key_size;
UINT                  clen;
NX_CRYPTO_EC         *curve;
NX_CRYPTO_HUGE_NUMBER private_key;

    /* Make sure the key size was assigned before we do anything else. Generally, this means
       _nx_crypto_ecdh_setup was not called to set up the NX_CRYPTO_ECDH structure prior to this call.  */
    if (0 == ecdh_ptr -> nx_crypto_ecdh_key_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    curve = ecdh_ptr -> nx_crypto_ecdh_curve;

    /* Figure out the sizes of our keys and buffers. */
    key_size = ecdh_ptr -> nx_crypto_ecdh_key_size;

    /* Check to make sure the buffer is large enough to hold the private key. */
    clen = (curve -> nx_crypto_ec_bits + 7) >> 3;
    if (local_private_key_len < clen)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Private key buffer - note that no scratch is required for the private key, but we set it in case
       it is needed in the future. */
    private_key.nx_crypto_huge_number_data = (HN_UBASE *)ecdh_ptr -> nx_crypto_ecdh_private_key_buffer;
    private_key.nx_crypto_huge_number_size = key_size >> HN_SIZE_SHIFT;
    private_key.nx_crypto_huge_buffer_size = key_size;
    private_key.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

    /* Copy the private key into the return buffer. */
    status = _nx_crypto_huge_number_extract_fixed_size(&private_key,
                                                       local_private_key_ptr, clen);
    *actual_local_private_key_len = clen;

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_setup                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up a Elliptic-curve Diffie-Hellman context by    */
/*    generating a local key pair.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    share_secret_key_ptr                  Shared secret buffer pointer  */
/*    share_secret_key_len_ptr              Length of shared secret       */
/*    local_public_key_ptr                  Pointer to local public key   */
/*    local_public_key_len                  Remote public key length      */
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
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ecdh_setup(NX_CRYPTO_ECDH  *ecdh_ptr,
                                          UCHAR  *local_public_key_ptr,
                                          ULONG   local_public_key_len,
                                          ULONG  *actual_local_public_key_len,
                                          NX_CRYPTO_EC *curve,
                                          HN_UBASE *scratch_buf_ptr)
{
UINT public_key_len;
/* Actual huge numbers used in calculations */
NX_CRYPTO_HUGE_NUMBER private_key;
NX_CRYPTO_EC_POINT    public_key;

    public_key_len = 1 + (((curve -> nx_crypto_ec_bits + 7) >> 3) << 1);
    if (local_public_key_len < public_key_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    ecdh_ptr -> nx_crypto_ecdh_curve = curve;

    /* Assign the desired key size based on the chosen elliptic curve. */
    ecdh_ptr -> nx_crypto_ecdh_key_size = curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;

    /* Public key buffer (and scratch). */
    NX_CRYPTO_EC_POINT_INITIALIZE(&public_key, NX_CRYPTO_EC_POINT_AFFINE, scratch_buf_ptr,
                                  ecdh_ptr -> nx_crypto_ecdh_key_size);

    /* Private key buffer - note that no scratch is required for the private key. */
    private_key.nx_crypto_huge_number_data = ecdh_ptr -> nx_crypto_ecdh_private_key_buffer;
    private_key.nx_crypto_huge_number_size = ecdh_ptr -> nx_crypto_ecdh_key_size >> HN_SIZE_SHIFT;
    private_key.nx_crypto_huge_buffer_size = sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer);
    private_key.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

    /* Clear the private key buffer. */
    NX_CRYPTO_MEMSET(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer, 0,
                     sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer));

    /* Generate Key Pair. */
    _nx_crypto_ec_key_pair_generation_extra(curve, &curve -> nx_crypto_ec_g, &private_key,
                                            &public_key, scratch_buf_ptr);

    /* Copy the public key into the return buffer. */
    _nx_crypto_ec_point_extract_uncompressed(curve, &public_key, local_public_key_ptr,
                                             local_public_key_len, &public_key_len);

    if (public_key_len == 0)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }
    *actual_local_public_key_len = public_key_len;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_compute_secret                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes the Elliptic-curve Diffie-Hellman shared     */
/*    secret using an existing Elliptic-curve Diffie-Hellman context      */
/*    and a public key received from a remote entity.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
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
/*                                            added public key validation,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ecdh_compute_secret(NX_CRYPTO_ECDH  *ecdh_ptr,
                                                   UCHAR  *share_secret_key_ptr,
                                                   ULONG   share_secret_key_len_ptr,
                                                   ULONG  *actual_share_secret_key_len,
                                                   UCHAR  *remote_public_key,
                                                   ULONG   remote_public_key_len,
                                                   HN_UBASE *scratch_buf_ptr)
{
UINT                  status;
UINT                  key_size;
UINT                  clen;
NX_CRYPTO_EC         *curve;
/* Actual huge numbers used in calculations */
NX_CRYPTO_HUGE_NUMBER private_key;
NX_CRYPTO_EC_POINT    public_key, shared_secret;

    /* Make sure the key size was assigned before we do anything else. Generally, this means
       _nx_crypto_ecdh_setup was not called to set up the NX_CRYPTO_ECDH structure prior to this call.  */
    if (0 == ecdh_ptr -> nx_crypto_ecdh_key_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    curve = ecdh_ptr -> nx_crypto_ecdh_curve;

    /* Figure out the sizes of our keys and buffers. We need 4X the key size for our buffer space. */
    key_size = ecdh_ptr -> nx_crypto_ecdh_key_size;

    /* Make sure the remote public key is small enough to fit into the huge number buffer. */
    if (remote_public_key_len > 1 + 2 * key_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Check to make sure the buffer is large enough to hold the shared secret key. */
    clen = (curve -> nx_crypto_ec_bits + 7) >> 3;
    if (share_secret_key_len_ptr < clen)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    NX_CRYPTO_EC_POINT_INITIALIZE(&public_key, NX_CRYPTO_EC_POINT_AFFINE, scratch_buf_ptr, key_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&shared_secret, NX_CRYPTO_EC_POINT_AFFINE, scratch_buf_ptr, key_size);


    /* Copy the remote public key from the caller's buffer. */
    status = _nx_crypto_ec_point_setup(&public_key, remote_public_key, remote_public_key_len);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

#ifndef NX_CRYPTO_ECC_DISABLE_KEY_VALIDATION
    status = _nx_crypto_ec_validate_public_key(&public_key, curve, NX_CRYPTO_TRUE, scratch_buf_ptr);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }
#endif /* NX_CRYPTO_ECC_DISABLE_KEY_VALIDATION */

    /* Private key buffer - note that no scratch is required for the private key, but we set it in case
       it is needed in the future. */
    private_key.nx_crypto_huge_number_data = (HN_UBASE *)ecdh_ptr -> nx_crypto_ecdh_private_key_buffer;
    private_key.nx_crypto_huge_number_size = key_size >> HN_SIZE_SHIFT;
    private_key.nx_crypto_huge_buffer_size = key_size;
    private_key.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

    /* Finally, generate shared secret from the remote public key, our generated private key, and the curve.
       The actual calculation is "shared_secret = private_key * public_key". */
    curve -> nx_crypto_ec_multiple(curve, &public_key, &private_key, &shared_secret, scratch_buf_ptr);

    /* The public key size is simply the key size for this group. */

    /* Copy the shared secret into the return buffer. */
    status = _nx_crypto_huge_number_extract_fixed_size(&shared_secret.nx_crypto_ec_point_x,
                                                       share_secret_key_ptr, clen);
    *actual_share_secret_key_len = clen;

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdh_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported ECDH cryptographic algorithm.                   */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_ecdh_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                 VOID  **handle,
                                                 VOID  *crypto_metadata,
                                                 ULONG crypto_metadata_size)
{

    NX_CRYPTO_STATE_CHECK

    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    if ((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_ECDH))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdh_cleanup                      PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_ecdh_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_ECDH));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdh_operation                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs an ECDH operation.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    ECDH operation                */
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
/*    _nx_crypto_ecdh_setup                 Setup local key pair          */
/*    _nx_crypto_ecdh_compute_secret        Compute shared secret         */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ecdh_operation(UINT op,
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
NX_CRYPTO_ECDH *ecdh;
UINT            status = NX_CRYPTO_SUCCESS;
NX_CRYPTO_EXTENDED_OUTPUT
               *extended_output;

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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_ECDH))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ecdh = (NX_CRYPTO_ECDH *)crypto_metadata;

    if (op == NX_CRYPTO_EC_CURVE_SET)
    {
        /* Set curve to be used in the ECDH. */
        status = ((NX_CRYPTO_METHOD *)input) -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_GET,
                                                                    NX_CRYPTO_NULL,
                                                                    (NX_CRYPTO_METHOD *)input,
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL,
                                                                    (UCHAR *)&ecdh -> nx_crypto_ecdh_curve,
                                                                    sizeof(NX_CRYPTO_EC *),
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    }
    else if (op == NX_CRYPTO_EC_KEY_PAIR_GENERATE)
    {
        if (ecdh -> nx_crypto_ecdh_curve == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;
        status = _nx_crypto_ec_key_pair_stream_generate(ecdh -> nx_crypto_ecdh_curve,
                                                        extended_output -> nx_crypto_extended_output_data,
                                                        extended_output -> nx_crypto_extended_output_length_in_byte,
                                                        &extended_output -> nx_crypto_extended_output_actual_size,
                                                        ecdh -> nx_crypto_ecdh_scratch_buffer);
    }
    else if (op == NX_CRYPTO_DH_SETUP)
    {
        /* Setup local key pair. */
        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;
#ifdef NX_CRYPTO_ENABLE_CURVE25519_448
        if (ecdh -> nx_crypto_ecdh_curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X25519 ||
            ecdh -> nx_crypto_ecdh_curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X448)
        {
            status = _nx_crypto_ecdh_setup_x25519_448(ecdh,
                                                      extended_output -> nx_crypto_extended_output_data,
                                                      extended_output -> nx_crypto_extended_output_length_in_byte,
                                                      &extended_output -> nx_crypto_extended_output_actual_size,
                                                      ecdh -> nx_crypto_ecdh_curve,
                                                      ecdh -> nx_crypto_ecdh_scratch_buffer);
        }
        else
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
        {
            status = _nx_crypto_ecdh_setup(ecdh,
                                           extended_output -> nx_crypto_extended_output_data,
                                           extended_output -> nx_crypto_extended_output_length_in_byte,
                                           &extended_output -> nx_crypto_extended_output_actual_size,
                                           ecdh -> nx_crypto_ecdh_curve,
                                           ecdh -> nx_crypto_ecdh_scratch_buffer);
        }
    }
    else if (op == NX_CRYPTO_DH_KEY_PAIR_IMPORT)
    {
        if (key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

#ifdef NX_CRYPTO_ENABLE_CURVE25519_448
        if (ecdh -> nx_crypto_ecdh_curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X25519 ||
            ecdh -> nx_crypto_ecdh_curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X448)
        {

            /* Import local key pair. */
            status = _nx_crypto_ecdh_key_pair_import_x25519_448(ecdh, ecdh -> nx_crypto_ecdh_curve,
                                                                key, (key_size_in_bits >> 3),
                                                                input, input_length_in_byte);
        }
        else
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
        {

            /* Import local key pair. */
            status = _nx_crypto_ecdh_key_pair_import(ecdh, ecdh -> nx_crypto_ecdh_curve,
                                                     key, (key_size_in_bits >> 3),
                                                     input, input_length_in_byte);
        }
    }
    else if (op == NX_CRYPTO_DH_PRIVATE_KEY_EXPORT)
    {
        /* Export local private key. */
        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;

#ifdef NX_CRYPTO_ENABLE_CURVE25519_448
        if (ecdh -> nx_crypto_ecdh_curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X25519 ||
            ecdh -> nx_crypto_ecdh_curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X448)
        {
            status = _nx_crypto_ecdh_private_key_export_x25519_448(ecdh,
                                                                   extended_output -> nx_crypto_extended_output_data,
                                                                   extended_output -> nx_crypto_extended_output_length_in_byte,
                                                                   &extended_output -> nx_crypto_extended_output_actual_size);
        }
        else
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
        {

            status = _nx_crypto_ecdh_private_key_export(ecdh,
                                                        extended_output -> nx_crypto_extended_output_data,
                                                        extended_output -> nx_crypto_extended_output_length_in_byte,
                                                        &extended_output -> nx_crypto_extended_output_actual_size);
        }
    }
    else if (op == NX_CRYPTO_DH_CALCULATE)
    {
        /* Compute shared secret. */
        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;

#ifdef NX_CRYPTO_ENABLE_CURVE25519_448
        if (ecdh -> nx_crypto_ecdh_curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X25519 ||
            ecdh -> nx_crypto_ecdh_curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X448)
        {

            status = _nx_crypto_ecdh_compute_secret_x25519_448(ecdh,
                                                               extended_output -> nx_crypto_extended_output_data,
                                                               extended_output -> nx_crypto_extended_output_length_in_byte,
                                                               &extended_output -> nx_crypto_extended_output_actual_size,
                                                               input,
                                                               input_length_in_byte,
                                                               ecdh -> nx_crypto_ecdh_scratch_buffer);
        }
        else
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
        {

            status = _nx_crypto_ecdh_compute_secret(ecdh,
                                                    extended_output -> nx_crypto_extended_output_data,
                                                    extended_output -> nx_crypto_extended_output_length_in_byte,
                                                    &extended_output -> nx_crypto_extended_output_actual_size,
                                                    input,
                                                    input_length_in_byte,
                                                    ecdh -> nx_crypto_ecdh_scratch_buffer);
        }
    }
    else
    {
        status = NX_CRYPTO_NOT_SUCCESSFUL;
    }

    return(status);
}

#ifdef NX_CRYPTO_ENABLE_CURVE25519_448

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_key_pair_import_x25519_448          PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up a Elliptic-curve Diffie-Hellman context by    */
/*    importing a local key pair.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    curve                                 Elliptic Curve                */
/*    local_private_key_ptr                 Pointer to local private key  */
/*    local_private_key_len                 Local private key length      */
/*    local_public_key_ptr                  Pointer to local public key   */
/*    local_public_key_len                  Remote public key length      */
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
/*    _nx_crypto_method_ecdh_operation      Perform ECDH operation        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ecdh_key_pair_import_x25519_448(NX_CRYPTO_ECDH *ecdh_ptr,
                                                               NX_CRYPTO_EC *curve,
                                                               UCHAR *local_private_key_ptr,
                                                               ULONG  local_private_key_len,
                                                               UCHAR *local_public_key_ptr,
                                                               ULONG  local_public_key_len)
{
UINT key_len;


    NX_CRYPTO_PARAMETER_NOT_USED(local_public_key_ptr);

    key_len = (curve -> nx_crypto_ec_bits + 7) >> 3;

    if (local_private_key_len != key_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    if (local_public_key_len > key_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Assign the desired key size based on the chosen elliptic curve. */
    ecdh_ptr -> nx_crypto_ecdh_key_size = key_len;

    /* Copy the private key buffer. */
    NX_CRYPTO_MEMCPY(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer, local_private_key_ptr, local_private_key_len); /* Use case of memcpy is verified. */

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_private_key_export_x25519_448       PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function exports the local private key in Elliptic-curve       */
/*    Diffie-Hellman context.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    local_private_key_ptr                 Pointer to local private key  */
/*    local_private_key_len                 Local private key length      */
/*    actual_local_private_key_len          Pointer to private key length */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMCPY                      Copy the key                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_ecdh_operation      Perform ECDH operation        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ecdh_private_key_export_x25519_448(NX_CRYPTO_ECDH *ecdh_ptr,
                                                                  UCHAR *local_private_key_ptr,
                                                                  ULONG  local_private_key_len,
                                                                  ULONG *actual_local_private_key_len)
{
UINT          clen;
NX_CRYPTO_EC *curve;

    /* Make sure the key size was assigned before we do anything else. */
    if (0 == ecdh_ptr -> nx_crypto_ecdh_key_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    curve = ecdh_ptr -> nx_crypto_ecdh_curve;

    /* Check to make sure the buffer is large enough to hold the private key. */
    clen = (curve -> nx_crypto_ec_bits + 7) >> 3;
    if (local_private_key_len < clen)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Copy the private key buffer. */
    NX_CRYPTO_MEMCPY(local_private_key_ptr, ecdh_ptr -> nx_crypto_ecdh_private_key_buffer, clen); /* Use case of memcpy is verified. */

    *actual_local_private_key_len = clen;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_setup_x25519_448                    PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up a Elliptic-curve Diffie-Hellman context by    */
/*    generating a local key pair.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    share_secret_key_ptr                  Shared secret buffer pointer  */
/*    share_secret_key_len_ptr              Length of shared secret       */
/*    local_public_key_ptr                  Pointer to local public key   */
/*    local_public_key_len                  Remote public key length      */
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
/*    _nx_crypto_ec_key_pair_generation_x25519                            */
/*                                          Generate EC Key Pair          */
/*    _nx_crypto_ec_extract_fixed_size_le   Extract huge number           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_ecdh_operation      Perform ECDH operation        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            added x448 curve,           */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ecdh_setup_x25519_448(NX_CRYPTO_ECDH *ecdh_ptr,
                                                     UCHAR *local_public_key_ptr,
                                                     ULONG  local_public_key_len,
                                                     ULONG *actual_local_public_key_len,
                                                     NX_CRYPTO_EC *curve,
                                                     HN_UBASE *scratch_buf_ptr)
{
UINT key_len;
UINT status;
NX_CRYPTO_HUGE_NUMBER private_key;
NX_CRYPTO_EC_POINT    public_key;

    key_len = (curve -> nx_crypto_ec_bits + 7) >> 3;
    if (local_public_key_len < key_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    ecdh_ptr -> nx_crypto_ecdh_curve = curve;

    /* Assign the desired key size based on the chosen elliptic curve. */
    ecdh_ptr -> nx_crypto_ecdh_key_size = key_len;

    /* Public key buffer (and scratch). */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&public_key.nx_crypto_ec_point_x, scratch_buf_ptr, key_len);

    /* Private key buffer - note that no scratch is required for the private key. */
    private_key.nx_crypto_huge_number_data = ecdh_ptr -> nx_crypto_ecdh_private_key_buffer;
    private_key.nx_crypto_huge_number_size = ecdh_ptr -> nx_crypto_ecdh_key_size >> HN_SIZE_SHIFT;
    private_key.nx_crypto_huge_buffer_size = sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer);
    private_key.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

    /* Clear the private key buffer. */
    NX_CRYPTO_MEMSET(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer, 0,
                     sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key_buffer));

    /* Generate Key Pair. */
    status = _nx_crypto_ec_key_pair_generation_x25519_448(curve, &curve -> nx_crypto_ec_g, &private_key,
                                                          &public_key, scratch_buf_ptr);
    if (status)
    {
        return(status);
    }

    /* Copy the public key into the return buffer. */
    status = _nx_crypto_ec_extract_fixed_size_le(&public_key.nx_crypto_ec_point_x,
                                                 local_public_key_ptr, key_len);
    if (status)
    {
        return(status);
    }

    *actual_local_public_key_len = key_len;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_compute_secret_x25519_448           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes the Elliptic-curve Diffie-Hellman shared     */
/*    secret using an existing Elliptic-curve Diffie-Hellman context      */
/*    and a public key received from a remote entity.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    share_secret_key_ptr                  Shared secret buffer pointer  */
/*    share_secret_key_len_ptr              Length of shared secret       */
/*    remote_public_key                     Pointer to remote public key  */
/*    remote_public_key_len                 Remote public key length      */
/*    scratch_buf_ptr                       Pointer to scratch buffer,    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ec_extract_fixed_size_le   Extract huge number           */
/*    [nx_crypto_ec_multiple]               Perform multiplication for EC */
/*    _nx_crypto_huge_number_is_zero        Check for all-zero value      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_ecdh_operation      Perform ECDH operation        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ecdh_compute_secret_x25519_448(NX_CRYPTO_ECDH *ecdh_ptr,
                                                              UCHAR *share_secret_key_ptr,
                                                              ULONG  share_secret_key_len_ptr,
                                                              ULONG *actual_share_secret_key_len,
                                                              UCHAR *remote_public_key,
                                                              ULONG  remote_public_key_len,
                                                              HN_UBASE *scratch_buf_ptr)
{
UINT                  status;
UINT                  key_size;
UINT                  clen;
NX_CRYPTO_EC         *curve;
/* Actual huge numbers used in calculations */
NX_CRYPTO_HUGE_NUMBER private_key;
NX_CRYPTO_EC_POINT    public_key, shared_secret;

    /* Make sure the key size was assigned before we do anything else. */
    if (0 == ecdh_ptr -> nx_crypto_ecdh_key_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    curve = ecdh_ptr -> nx_crypto_ecdh_curve;

    /* Figure out the sizes of our keys and buffers. */
    key_size = ecdh_ptr -> nx_crypto_ecdh_key_size;

    /* Make sure the remote public key size is correct. */
    if (remote_public_key_len != key_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Check to make sure the buffer is large enough to hold the shared secret key. */
    clen = (curve -> nx_crypto_ec_bits + 7) >> 3;
    if (share_secret_key_len_ptr < clen)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&public_key.nx_crypto_ec_point_x, scratch_buf_ptr, key_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&shared_secret.nx_crypto_ec_point_x, scratch_buf_ptr, key_size);

    /* Copy the remote public key from the caller's buffer. */
    NX_CRYPTO_MEMCPY(public_key.nx_crypto_ec_point_x.nx_crypto_huge_number_data, remote_public_key, remote_public_key_len); /* Use case of memcpy is verified. */
    public_key.nx_crypto_ec_point_x.nx_crypto_huge_number_size = key_size >> HN_SIZE_SHIFT;

    /* Private key buffer  */
    private_key.nx_crypto_huge_number_data = (HN_UBASE*)ecdh_ptr -> nx_crypto_ecdh_private_key_buffer;
    private_key.nx_crypto_huge_number_size = key_size >> HN_SIZE_SHIFT;
    private_key.nx_crypto_huge_buffer_size = key_size;
    private_key.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

    /* Finally, generate shared secret from the remote public key, our generated private key, and the curve.
       The actual calculation is "shared_secret = private_key * public_key". */
    curve -> nx_crypto_ec_multiple(curve, &public_key, &private_key, &shared_secret, scratch_buf_ptr);

    /* Check for the all-zero value results. */
    if (_nx_crypto_huge_number_is_zero(&shared_secret.nx_crypto_ec_point_x))
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Copy the shared secret into the return buffer. */
    status = _nx_crypto_ec_extract_fixed_size_le(&shared_secret.nx_crypto_ec_point_x,
                                                 share_secret_key_ptr, clen);

    /* The public key size is simply the key size for this group. */
    *actual_share_secret_key_len = clen;

    return(status);
}

#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
