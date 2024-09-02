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
/**  Transport Layer Security (TLS) - PKCS#1 v1.5 functions               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_pkcs1_v1.5.h"

/* DER encodings (with OIDs for common algorithms) from RFC 8017.
 * NOTE: This is the equivalent DER-encoding for the value "T" described in RFC 8017 section 9.2. */
static const UCHAR _NX_CRYPTO_DER_OID_SHA_1[]       =  {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_224[]     =  {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_256[]     =  {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_384[]     =  {0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_512[]     =  {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_512_224[] =  {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x05, 0x05, 0x00, 0x04, 0x1c};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_512_256[] =  {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x06, 0x05, 0x00, 0x04, 0x20};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_pkcs1_v1_5_sign                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates an encoded signature using PKCS#1v1.5       */
/*    formatting.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    input                                 Data to sign                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operations             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
NX_CRYPTO_KEEP UINT _nx_crypto_pkcs1_v1_5_sign(UCHAR *input, UINT input_length,
                                               UCHAR *private_key, UINT private_key_size,
                                               UCHAR *metadata_area,
                                               UCHAR *output, UINT output_size)
{
NX_CRYPTO_PKCS1 *ctx = (NX_CRYPTO_PKCS1 *)metadata_area;
UINT status;

/*  From RFC 8017 Section 9.2,  EMSA-PKCS1-v1_5-ENCODE (M, emLen):
 *
 *       1.  Apply the hash function to the message M to produce a hash
 *        value H:
 *
 *           H = Hash(M).
 *
 *        If the hash function outputs "message too long", output
 *        "message too long" and stop.
 *
 *    2.  Encode the algorithm ID for the hash function and the hash
 *        value into an ASN.1 value of type DigestInfo (see
 *        Appendix A.2.4) with the DER, where the type DigestInfo has
 *        the syntax
 *
 *             DigestInfo ::= SEQUENCE {
 *                 digestAlgorithm AlgorithmIdentifier,
 *                 digest OCTET STRING
 *             }
 *
 *        The first field identifies the hash function and the second
 *        contains the hash value.  Let T be the DER encoding of the
 *        DigestInfo value (see the notes below), and let tLen be the
 *        length in octets of T.
 *
 *    3.  If emLen < tLen + 11, output "intended encoded message length
 *        too short" and stop.
 *
 *    4.  Generate an octet string PS consisting of emLen - tLen - 3
 *        octets with hexadecimal value 0xff.  The length of PS will be
 *        at least 8 octets.
 *
 *    5.  Concatenate PS, the DER encoding T, and other padding to form
 *        the encoded message EM as
 *
 *           EM = 0x00 || 0x01 || PS || 0x00 || T.
 *
 */

    /* Make sure we found a supported version (essentially an assertion check). */
    if (ctx -> hash_method == NX_CRYPTO_NULL
        || ctx -> public_cipher_method == NX_CRYPTO_NULL
        || (ctx -> public_cipher_method) -> nx_crypto_init == NX_CRYPTO_NULL
        || (ctx -> public_cipher_method) -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if (output_size < ctx -> modulus_size)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    status = _nx_crypto_pkcs1_v1_5_encode(input, input_length, ctx -> hash_method, ctx -> hash_metadata, ctx -> hash_metadata_size, output, private_key_size);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }                                                     

    /* Initialize the crypto method with the modulus. */
    status = (ctx -> public_cipher_method) -> nx_crypto_init(ctx -> public_cipher_method,
                                                             ctx -> modulus,
                                                             (NX_CRYPTO_KEY_SIZE)(ctx -> modulus_size << 3),
                                                             NX_CRYPTO_NULL,
                                                             ctx -> public_cipher_metadata,
                                                             ctx -> public_cipher_metadata_size);
    if (status)
    {
        return(status);
    }

    /* Sign the hash we just generated using our local RSA private key (associated with our local cert). */
    status = (ctx -> public_cipher_method) -> nx_crypto_operation(NX_CRYPTO_ENCRYPT,
                                                                  NX_CRYPTO_NULL,
                                                                  ctx -> public_cipher_method,
                                                                  private_key,
                                                                  (NX_CRYPTO_KEY_SIZE)(private_key_size << 3),
                                                                  output,
                                                                  (NX_CRYPTO_KEY_SIZE)(private_key_size),
                                                                  NX_CRYPTO_NULL,
                                                                  output,
                                                                  output_size,
                                                                  ctx -> public_cipher_metadata,
                                                                  ctx -> public_cipher_metadata_size,
                                                                  NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if ((ctx -> public_cipher_method) -> nx_crypto_cleanup)
    {
        status = (ctx -> public_cipher_method) -> nx_crypto_cleanup(ctx -> public_cipher_metadata);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_pkcs1_v1_5_verify                        PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies an encoded signature using PKCS#1v1.5        */
/*    formatting.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    input                                 Data to sign                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operations             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            corrected the operation,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_pkcs1_v1_5_verify(UCHAR *message, UINT message_length,
                                                 UCHAR *signature, UINT signature_length,
                                                 UCHAR *public_key, UINT public_key_size,
                                                 UCHAR *metadata_area)
{
UCHAR *EM1, *EM2;
NX_CRYPTO_PKCS1 *ctx = (NX_CRYPTO_PKCS1 *)metadata_area;
UINT status;

    /* Make sure we found a supported version (essentially an assertion check). */
    if (ctx -> hash_method == NX_CRYPTO_NULL
        || ctx -> public_cipher_method == NX_CRYPTO_NULL
        || (ctx -> public_cipher_method) -> nx_crypto_init == NX_CRYPTO_NULL
        || (ctx -> public_cipher_method) -> nx_crypto_operation == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if (sizeof(ctx -> scratch_buffer) < (2 * (ctx -> modulus_size)))
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Allocate space for encoded messages need to be compared. */
    EM1 = ctx -> scratch_buffer;
    EM2 = ctx -> scratch_buffer + ctx -> modulus_size;

    /* Initialize the crypto method with the modulus. */
    status = (ctx -> public_cipher_method) -> nx_crypto_init(ctx -> public_cipher_method,
                                                             ctx -> modulus,
                                                             (NX_CRYPTO_KEY_SIZE)(ctx -> modulus_size << 3),
                                                             NX_CRYPTO_NULL,
                                                             ctx -> public_cipher_metadata,
                                                             ctx -> public_cipher_metadata_size);
    if (status)
    {
        return status;
    }

    /* Decrypt the signature by the public key to get EM1 */
    status = (ctx -> public_cipher_method) -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                                  NX_CRYPTO_NULL,
                                                                  ctx -> public_cipher_method,
                                                                  public_key,
                                                                  (NX_CRYPTO_KEY_SIZE)(public_key_size << 3),
                                                                  signature,
                                                                  signature_length,
                                                                  NX_CRYPTO_NULL,
                                                                  EM1,
                                                                  ctx -> modulus_size,
                                                                  ctx -> public_cipher_metadata,
                                                                  ctx -> public_cipher_metadata_size,
                                                                  NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    if (status)
    {
        return status;
    }

    if ((ctx -> public_cipher_method) -> nx_crypto_cleanup)
    {
        status = (ctx -> public_cipher_method) -> nx_crypto_cleanup(ctx -> public_cipher_metadata);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Encoded the original message to get EM2. */
    status = _nx_crypto_pkcs1_v1_5_encode(message, message_length, ctx -> hash_method, ctx -> hash_metadata, ctx -> hash_metadata_size, EM2, ctx -> modulus_size);
    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (NX_CRYPTO_MEMCMP(EM1, EM2, ctx -> modulus_size))
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_pkcs1_v1_5_encode                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encodes a message with PKCS#1v1.5.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    input                                 Data to sign                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operations             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_pkcs1_v1_5_encode(UCHAR *input, UINT input_length,
                                                 NX_CRYPTO_METHOD *hash_method,
                                                 UCHAR *metadata_area, UINT metadata_size,
                                                 UCHAR *output, UINT expected_output_length)
{
const UCHAR *der_encoding;
UINT der_encoding_length;
UINT signature_length = 0;
UINT hash_length;
UCHAR *working_ptr;
UINT status;

    /* PKCS-1 Signature padding. The scheme is to start with the block type (0x00, 0x01 for signing)
       then pad with 0xFF bytes (for signing) followed with a single 0 byte right before the payload,
       which comes at the end of the RSA block. */

    /* Start with a clear buffer. */
    NX_CRYPTO_MEMSET(output, 0xff, expected_output_length);
    output[0] = 0x0;
    output[1] = 0x1;

    /* Get the size of the hash output. */
    hash_length = (hash_method -> nx_crypto_ICV_size_in_bits) >> 3;

    /* Figure out which hash method we are using to get our DER-encoding. */
    switch(hash_method -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_HASH_SHA1:
        der_encoding = _NX_CRYPTO_DER_OID_SHA_1;
        der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_1);
        break;
    case NX_CRYPTO_HASH_SHA224:
        der_encoding = _NX_CRYPTO_DER_OID_SHA_224;
        der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_224);
        break;
    case NX_CRYPTO_HASH_SHA256:
        der_encoding = _NX_CRYPTO_DER_OID_SHA_256;
        der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_256);
        break;
    case NX_CRYPTO_HASH_SHA384:
        der_encoding = _NX_CRYPTO_DER_OID_SHA_384;
        der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_384);
        break;
    case NX_CRYPTO_HASH_SHA512:
        der_encoding = _NX_CRYPTO_DER_OID_SHA_512;
        der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_512);
        break;
    case NX_CRYPTO_HASH_SHA512_224:
        der_encoding = _NX_CRYPTO_DER_OID_SHA_512_224;
        der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_512_224);
        break;
    case NX_CRYPTO_HASH_SHA512_256:
        der_encoding = _NX_CRYPTO_DER_OID_SHA_512_256;
        der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_512_256);
        break;
    default:
        return(NX_CRYPTO_AUTHENTICATION_FAILED);
    }

    /* Encoded signature in RSA buffer (plaintext):
     * signature_length = der_encoding_length + hash_length
     * x = data_size - (der_encoding_length + hash_length)
     * Length: |     x       |  der_encoding_length  | hash_length   |
     * Field:  | Padding     |  DER encoding header  | Hash value    |
     * Value:  | 0x0,0x1,... | <ASN.1 sequence, OID> | <hash output> |
     */

    /* Calculate our final signature length for later offset calculations. */
    signature_length = der_encoding_length + hash_length; /* DER encoding size + hash size = plaintext encoded signature length */
    if (signature_length + 1 > expected_output_length)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }
    output[expected_output_length - signature_length - 1] = 0;

    /* Get a working pointer into the padded signature buffer. All PKCS-1 encoded data
       comes at the end of the RSA encrypted block. */
    working_ptr = &output[expected_output_length - signature_length];

    /* Copy in the DER encoding. */
    NX_CRYPTO_MEMCPY(working_ptr, der_encoding, der_encoding_length); /* Use case of memcpy is verified. */

    /* Move the working pointer to the end of the DER encoding. */
    working_ptr += der_encoding_length;

    /* Generate hash of input data using supplied hash method, placing it in the output buffer at the end
     * of the RSA block (data_size).  */
    status = hash_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                       NX_CRYPTO_NULL,
                                       hash_method,
                                       NX_CRYPTO_NULL,
                                       0,
                                       input,
                                       input_length,
                                       NX_CRYPTO_NULL,
                                       working_ptr,
                                       hash_length,
                                       metadata_area,
                                       metadata_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    return (status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_pkcs1_v1_5_init                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported PKCS#1v1.5 cryptographic algorithm.             */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_pkcs1_v1_5_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_PKCS1))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_pkcs1_v1_5_cleanup                PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_pkcs1_v1_5_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_PKCS1));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_pkcs1_operation                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs an PKCS#1v1.5 operation.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    PKCS#1v1.5 operation          */
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
/*    _nx_crypto_pkcs1_v1_5_sign            Sign using PKCS#1v1.5         */
/*    _nx_crypto_pkcs1_v1_5_verify          Verify PKCS#1v1.5 signature   */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_pkcs1_v1_5_operation(UINT op,
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
NX_CRYPTO_PKCS1 *ctx;
NX_CRYPTO_PKCS1_OPTIONS *options;
UINT             status = NX_CRYPTO_SUCCESS;

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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_PKCS1))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_PKCS1 *)(crypto_metadata);

    if (op == NX_CRYPTO_SET_ADDITIONAL_DATA)
    {
        options = (NX_CRYPTO_PKCS1_OPTIONS *)input;

        if ((options -> public_cipher_metadata_size < (options -> public_cipher_method) -> nx_crypto_metadata_area_size)
            || (options -> hash_metadata_size < (options -> hash_method) -> nx_crypto_metadata_area_size))
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }

        ctx -> public_cipher_method = options -> public_cipher_method;
        ctx -> public_cipher_metadata = options -> public_cipher_metadata;
        ctx -> public_cipher_metadata_size = options -> public_cipher_metadata_size;
        ctx -> hash_method = options -> hash_method;
        ctx -> hash_metadata = options -> hash_metadata;
        ctx -> hash_metadata_size = options -> hash_metadata_size;
        ctx -> modulus = key;
        ctx -> modulus_size = (key_size_in_bits >> 3);
    }
    else if (op == NX_CRYPTO_AUTHENTICATE)
    {
        status = _nx_crypto_pkcs1_v1_5_sign(input,
                                            input_length_in_byte,
                                            key,
                                            (key_size_in_bits >> 3),
                                            crypto_metadata,
                                            output,
                                            output_length_in_byte);

    }
    else if (op == NX_CRYPTO_VERIFY)
    {
        status = _nx_crypto_pkcs1_v1_5_verify(input,
                                              input_length_in_byte,
                                              output,
                                              output_length_in_byte,
                                              key,
                                              (key_size_in_bits >> 3),
                                              crypto_metadata);
    }
    else
    {
        status = NX_CRYPTO_NOT_SUCCESSFUL;
    }

    return(status);
}
