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
/**   AES Encryption                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#include "nx_crypto_aes.h"
#include "nx_crypto_xcbc_mac.h"

#if !defined(NX_CRYPTO_LITTLE_ENDIAN)
/*
    Encryption table for BIG ENDIAN architecture.

    Lookup table for computing the MixColumns() Transformation with SubBytes applied to input value S.
    Each entry of this table represents the result of {02} dot S[x,c] | {01} dot S[x,c] | {01} dot S[x,c] | {03} dot S[x,c]. See Equation
    5.6 on page 18 AES specification(Pub 197)

    The most significant byte is the result of {02} dot S, and the least significant byte is the result
    of {03} dot S[x,c].
 */
NX_CRYPTO_AES_TABLE ULONG aes_encryption_table[256] =
{
    0xc66363a5, 0xf87c7c84, 0xee777799, 0xf67b7b8d, 0xfff2f20d, 0xd66b6bbd, 0xde6f6fb1, 0x91c5c554,
    0x60303050, 0x02010103, 0xce6767a9, 0x562b2b7d, 0xe7fefe19, 0xb5d7d762, 0x4dababe6, 0xec76769a,
    0x8fcaca45, 0x1f82829d, 0x89c9c940, 0xfa7d7d87, 0xeffafa15, 0xb25959eb, 0x8e4747c9, 0xfbf0f00b,
    0x41adadec, 0xb3d4d467, 0x5fa2a2fd, 0x45afafea, 0x239c9cbf, 0x53a4a4f7, 0xe4727296, 0x9bc0c05b,
    0x75b7b7c2, 0xe1fdfd1c, 0x3d9393ae, 0x4c26266a, 0x6c36365a, 0x7e3f3f41, 0xf5f7f702, 0x83cccc4f,
    0x6834345c, 0x51a5a5f4, 0xd1e5e534, 0xf9f1f108, 0xe2717193, 0xabd8d873, 0x62313153, 0x2a15153f,
    0x0804040c, 0x95c7c752, 0x46232365, 0x9dc3c35e, 0x30181828, 0x379696a1, 0x0a05050f, 0x2f9a9ab5,
    0x0e070709, 0x24121236, 0x1b80809b, 0xdfe2e23d, 0xcdebeb26, 0x4e272769, 0x7fb2b2cd, 0xea75759f,
    0x1209091b, 0x1d83839e, 0x582c2c74, 0x341a1a2e, 0x361b1b2d, 0xdc6e6eb2, 0xb45a5aee, 0x5ba0a0fb,
    0xa45252f6, 0x763b3b4d, 0xb7d6d661, 0x7db3b3ce, 0x5229297b, 0xdde3e33e, 0x5e2f2f71, 0x13848497,
    0xa65353f5, 0xb9d1d168, 0x00000000, 0xc1eded2c, 0x40202060, 0xe3fcfc1f, 0x79b1b1c8, 0xb65b5bed,
    0xd46a6abe, 0x8dcbcb46, 0x67bebed9, 0x7239394b, 0x944a4ade, 0x984c4cd4, 0xb05858e8, 0x85cfcf4a,
    0xbbd0d06b, 0xc5efef2a, 0x4faaaae5, 0xedfbfb16, 0x864343c5, 0x9a4d4dd7, 0x66333355, 0x11858594,
    0x8a4545cf, 0xe9f9f910, 0x04020206, 0xfe7f7f81, 0xa05050f0, 0x783c3c44, 0x259f9fba, 0x4ba8a8e3,
    0xa25151f3, 0x5da3a3fe, 0x804040c0, 0x058f8f8a, 0x3f9292ad, 0x219d9dbc, 0x70383848, 0xf1f5f504,
    0x63bcbcdf, 0x77b6b6c1, 0xafdada75, 0x42212163, 0x20101030, 0xe5ffff1a, 0xfdf3f30e, 0xbfd2d26d,
    0x81cdcd4c, 0x180c0c14, 0x26131335, 0xc3ecec2f, 0xbe5f5fe1, 0x359797a2, 0x884444cc, 0x2e171739,
    0x93c4c457, 0x55a7a7f2, 0xfc7e7e82, 0x7a3d3d47, 0xc86464ac, 0xba5d5de7, 0x3219192b, 0xe6737395,
    0xc06060a0, 0x19818198, 0x9e4f4fd1, 0xa3dcdc7f, 0x44222266, 0x542a2a7e, 0x3b9090ab, 0x0b888883,
    0x8c4646ca, 0xc7eeee29, 0x6bb8b8d3, 0x2814143c, 0xa7dede79, 0xbc5e5ee2, 0x160b0b1d, 0xaddbdb76,
    0xdbe0e03b, 0x64323256, 0x743a3a4e, 0x140a0a1e, 0x924949db, 0x0c06060a, 0x4824246c, 0xb85c5ce4,
    0x9fc2c25d, 0xbdd3d36e, 0x43acacef, 0xc46262a6, 0x399191a8, 0x319595a4, 0xd3e4e437, 0xf279798b,
    0xd5e7e732, 0x8bc8c843, 0x6e373759, 0xda6d6db7, 0x018d8d8c, 0xb1d5d564, 0x9c4e4ed2, 0x49a9a9e0,
    0xd86c6cb4, 0xac5656fa, 0xf3f4f407, 0xcfeaea25, 0xca6565af, 0xf47a7a8e, 0x47aeaee9, 0x10080818,
    0x6fbabad5, 0xf0787888, 0x4a25256f, 0x5c2e2e72, 0x381c1c24, 0x57a6a6f1, 0x73b4b4c7, 0x97c6c651,
    0xcbe8e823, 0xa1dddd7c, 0xe874749c, 0x3e1f1f21, 0x964b4bdd, 0x61bdbddc, 0x0d8b8b86, 0x0f8a8a85,
    0xe0707090, 0x7c3e3e42, 0x71b5b5c4, 0xcc6666aa, 0x904848d8, 0x06030305, 0xf7f6f601, 0x1c0e0e12,
    0xc26161a3, 0x6a35355f, 0xae5757f9, 0x69b9b9d0, 0x17868691, 0x99c1c158, 0x3a1d1d27, 0x279e9eb9,
    0xd9e1e138, 0xebf8f813, 0x2b9898b3, 0x22111133, 0xd26969bb, 0xa9d9d970, 0x078e8e89, 0x339494a7,
    0x2d9b9bb6, 0x3c1e1e22, 0x15878792, 0xc9e9e920, 0x87cece49, 0xaa5555ff, 0x50282878, 0xa5dfdf7a,
    0x038c8c8f, 0x59a1a1f8, 0x09898980, 0x1a0d0d17, 0x65bfbfda, 0xd7e6e631, 0x844242c6, 0xd06868b8,
    0x824141c3, 0x299999b0, 0x5a2d2d77, 0x1e0f0f11, 0x7bb0b0cb, 0xa85454fc, 0x6dbbbbd6, 0x2c16163a
};


/*
   Decryption table for LITTLE ENDIAN architecture.

   Lookup table for computing the InvMixColumns() Transformation with InvSubBytes applied to input
   value S.  Each entry of this table represents
   the result of {0e} dot S[x,c] | {09} dot S[x,c] | {0d} dot S[x,c] | {0b} dot S[x,c]. See Equation
   5.10 on page 23 AES specification(Pub 197)

   The most significant byte is the result of {0e} dot S, and the least significant byte is the result
   of {0b} dot S[x,c].
 */
NX_CRYPTO_AES_TABLE ULONG aes_decryption_table[256] =
{
    0x51f4a750, 0x7e416553, 0x1a17a4c3, 0x3a275e96, 0x3bab6bcb, 0x1f9d45f1, 0xacfa58ab, 0x4be30393,
    0x2030fa55, 0xad766df6, 0x88cc7691, 0xf5024c25, 0x4fe5d7fc, 0xc52acbd7, 0x26354480, 0xb562a38f,
    0xdeb15a49, 0x25ba1b67, 0x45ea0e98, 0x5dfec0e1, 0xc32f7502, 0x814cf012, 0x8d4697a3, 0x6bd3f9c6,
    0x038f5fe7, 0x15929c95, 0xbf6d7aeb, 0x955259da, 0xd4be832d, 0x587421d3, 0x49e06929, 0x8ec9c844,
    0x75c2896a, 0xf48e7978, 0x99583e6b, 0x27b971dd, 0xbee14fb6, 0xf088ad17, 0xc920ac66, 0x7dce3ab4,
    0x63df4a18, 0xe51a3182, 0x97513360, 0x62537f45, 0xb16477e0, 0xbb6bae84, 0xfe81a01c, 0xf9082b94,
    0x70486858, 0x8f45fd19, 0x94de6c87, 0x527bf8b7, 0xab73d323, 0x724b02e2, 0xe31f8f57, 0x6655ab2a,
    0xb2eb2807, 0x2fb5c203, 0x86c57b9a, 0xd33708a5, 0x302887f2, 0x23bfa5b2, 0x02036aba, 0xed16825c,
    0x8acf1c2b, 0xa779b492, 0xf307f2f0, 0x4e69e2a1, 0x65daf4cd, 0x0605bed5, 0xd134621f, 0xc4a6fe8a,
    0x342e539d, 0xa2f355a0, 0x058ae132, 0xa4f6eb75, 0x0b83ec39, 0x4060efaa, 0x5e719f06, 0xbd6e1051,
    0x3e218af9, 0x96dd063d, 0xdd3e05ae, 0x4de6bd46, 0x91548db5, 0x71c45d05, 0x0406d46f, 0x605015ff,
    0x1998fb24, 0xd6bde997, 0x894043cc, 0x67d99e77, 0xb0e842bd, 0x07898b88, 0xe7195b38, 0x79c8eedb,
    0xa17c0a47, 0x7c420fe9, 0xf8841ec9, 0x00000000, 0x09808683, 0x322bed48, 0x1e1170ac, 0x6c5a724e,
    0xfd0efffb, 0x0f853856, 0x3daed51e, 0x362d3927, 0x0a0fd964, 0x685ca621, 0x9b5b54d1, 0x24362e3a,
    0x0c0a67b1, 0x9357e70f, 0xb4ee96d2, 0x1b9b919e, 0x80c0c54f, 0x61dc20a2, 0x5a774b69, 0x1c121a16,
    0xe293ba0a, 0xc0a02ae5, 0x3c22e043, 0x121b171d, 0x0e090d0b, 0xf28bc7ad, 0x2db6a8b9, 0x141ea9c8,
    0x57f11985, 0xaf75074c, 0xee99ddbb, 0xa37f60fd, 0xf701269f, 0x5c72f5bc, 0x44663bc5, 0x5bfb7e34,
    0x8b432976, 0xcb23c6dc, 0xb6edfc68, 0xb8e4f163, 0xd731dcca, 0x42638510, 0x13972240, 0x84c61120,
    0x854a247d, 0xd2bb3df8, 0xaef93211, 0xc729a16d, 0x1d9e2f4b, 0xdcb230f3, 0x0d8652ec, 0x77c1e3d0,
    0x2bb3166c, 0xa970b999, 0x119448fa, 0x47e96422, 0xa8fc8cc4, 0xa0f03f1a, 0x567d2cd8, 0x223390ef,
    0x87494ec7, 0xd938d1c1, 0x8ccaa2fe, 0x98d40b36, 0xa6f581cf, 0xa57ade28, 0xdab78e26, 0x3fadbfa4,
    0x2c3a9de4, 0x5078920d, 0x6a5fcc9b, 0x547e4662, 0xf68d13c2, 0x90d8b8e8, 0x2e39f75e, 0x82c3aff5,
    0x9f5d80be, 0x69d0937c, 0x6fd52da9, 0xcf2512b3, 0xc8ac993b, 0x10187da7, 0xe89c636e, 0xdb3bbb7b,
    0xcd267809, 0x6e5918f4, 0xec9ab701, 0x834f9aa8, 0xe6956e65, 0xaaffe67e, 0x21bccf08, 0xef15e8e6,
    0xbae79bd9, 0x4a6f36ce, 0xea9f09d4, 0x29b07cd6, 0x31a4b2af, 0x2a3f2331, 0xc6a59430, 0x35a266c0,
    0x744ebc37, 0xfc82caa6, 0xe090d0b0, 0x33a7d815, 0xf104984a, 0x41ecdaf7, 0x7fcd500e, 0x1791f62f,
    0x764dd68d, 0x43efb04d, 0xccaa4d54, 0xe49604df, 0x9ed1b5e3, 0x4c6a881b, 0xc12c1fb8, 0x4665517f,
    0x9d5eea04, 0x018c355d, 0xfa877473, 0xfb0b412e, 0xb3671d5a, 0x92dbd252, 0xe9105633, 0x6dd64713,
    0x9ad7618c, 0x37a10c7a, 0x59f8148e, 0xeb133c89, 0xcea927ee, 0xb761c935, 0xe11ce5ed, 0x7a47b13c,
    0x9cd2df59, 0x55f2733f, 0x1814ce79, 0x73c737bf, 0x53f7cdea, 0x5ffdaa5b, 0xdf3d6f14, 0x7844db86,
    0xcaaff381, 0xb968c43e, 0x3824342c, 0xc2a3405f, 0x161dc372, 0xbce2250c, 0x283c498b, 0xff0d9541,
    0x39a80171, 0x080cb3de, 0xd8b4e49c, 0x6456c190, 0x7bcb8461, 0xd532b670, 0x486c5c74, 0xd0b85742
};

/*
    Inverse Mix Table for LITTLE ENDIAN architecture.
    Lookup table for computing the InvMixColumns() Transformation.  Each entry of this table represents
    the result of {0e} dot S[x,c] | {09} dot S[x,c] | {0d} dot S[x,c] | {0b} dot S[x,c]. See Equation
    5.10 on page 23 AES specification(Pub 197)

    The most significant byte is the result of {0e} dot S, and the least significant byte is the result
    of {0b} dot S[x,c].
 */
NX_CRYPTO_AES_TABLE ULONG aes_inv_mix_table[256] =
{
    0x00000000, 0x0e090d0b, 0x1c121a16, 0x121b171d, 0x3824342c, 0x362d3927, 0x24362e3a, 0x2a3f2331,
    0x70486858, 0x7e416553, 0x6c5a724e, 0x62537f45, 0x486c5c74, 0x4665517f, 0x547e4662, 0x5a774b69,
    0xe090d0b0, 0xee99ddbb, 0xfc82caa6, 0xf28bc7ad, 0xd8b4e49c, 0xd6bde997, 0xc4a6fe8a, 0xcaaff381,
    0x90d8b8e8, 0x9ed1b5e3, 0x8ccaa2fe, 0x82c3aff5, 0xa8fc8cc4, 0xa6f581cf, 0xb4ee96d2, 0xbae79bd9,
    0xdb3bbb7b, 0xd532b670, 0xc729a16d, 0xc920ac66, 0xe31f8f57, 0xed16825c, 0xff0d9541, 0xf104984a,
    0xab73d323, 0xa57ade28, 0xb761c935, 0xb968c43e, 0x9357e70f, 0x9d5eea04, 0x8f45fd19, 0x814cf012,
    0x3bab6bcb, 0x35a266c0, 0x27b971dd, 0x29b07cd6, 0x038f5fe7, 0x0d8652ec, 0x1f9d45f1, 0x119448fa,
    0x4be30393, 0x45ea0e98, 0x57f11985, 0x59f8148e, 0x73c737bf, 0x7dce3ab4, 0x6fd52da9, 0x61dc20a2,
    0xad766df6, 0xa37f60fd, 0xb16477e0, 0xbf6d7aeb, 0x955259da, 0x9b5b54d1, 0x894043cc, 0x87494ec7,
    0xdd3e05ae, 0xd33708a5, 0xc12c1fb8, 0xcf2512b3, 0xe51a3182, 0xeb133c89, 0xf9082b94, 0xf701269f,
    0x4de6bd46, 0x43efb04d, 0x51f4a750, 0x5ffdaa5b, 0x75c2896a, 0x7bcb8461, 0x69d0937c, 0x67d99e77,
    0x3daed51e, 0x33a7d815, 0x21bccf08, 0x2fb5c203, 0x058ae132, 0x0b83ec39, 0x1998fb24, 0x1791f62f,
    0x764dd68d, 0x7844db86, 0x6a5fcc9b, 0x6456c190, 0x4e69e2a1, 0x4060efaa, 0x527bf8b7, 0x5c72f5bc,
    0x0605bed5, 0x080cb3de, 0x1a17a4c3, 0x141ea9c8, 0x3e218af9, 0x302887f2, 0x223390ef, 0x2c3a9de4,
    0x96dd063d, 0x98d40b36, 0x8acf1c2b, 0x84c61120, 0xaef93211, 0xa0f03f1a, 0xb2eb2807, 0xbce2250c,
    0xe6956e65, 0xe89c636e, 0xfa877473, 0xf48e7978, 0xdeb15a49, 0xd0b85742, 0xc2a3405f, 0xccaa4d54,
    0x41ecdaf7, 0x4fe5d7fc, 0x5dfec0e1, 0x53f7cdea, 0x79c8eedb, 0x77c1e3d0, 0x65daf4cd, 0x6bd3f9c6,
    0x31a4b2af, 0x3fadbfa4, 0x2db6a8b9, 0x23bfa5b2, 0x09808683, 0x07898b88, 0x15929c95, 0x1b9b919e,
    0xa17c0a47, 0xaf75074c, 0xbd6e1051, 0xb3671d5a, 0x99583e6b, 0x97513360, 0x854a247d, 0x8b432976,
    0xd134621f, 0xdf3d6f14, 0xcd267809, 0xc32f7502, 0xe9105633, 0xe7195b38, 0xf5024c25, 0xfb0b412e,
    0x9ad7618c, 0x94de6c87, 0x86c57b9a, 0x88cc7691, 0xa2f355a0, 0xacfa58ab, 0xbee14fb6, 0xb0e842bd,
    0xea9f09d4, 0xe49604df, 0xf68d13c2, 0xf8841ec9, 0xd2bb3df8, 0xdcb230f3, 0xcea927ee, 0xc0a02ae5,
    0x7a47b13c, 0x744ebc37, 0x6655ab2a, 0x685ca621, 0x42638510, 0x4c6a881b, 0x5e719f06, 0x5078920d,
    0x0a0fd964, 0x0406d46f, 0x161dc372, 0x1814ce79, 0x322bed48, 0x3c22e043, 0x2e39f75e, 0x2030fa55,
    0xec9ab701, 0xe293ba0a, 0xf088ad17, 0xfe81a01c, 0xd4be832d, 0xdab78e26, 0xc8ac993b, 0xc6a59430,
    0x9cd2df59, 0x92dbd252, 0x80c0c54f, 0x8ec9c844, 0xa4f6eb75, 0xaaffe67e, 0xb8e4f163, 0xb6edfc68,
    0x0c0a67b1, 0x02036aba, 0x10187da7, 0x1e1170ac, 0x342e539d, 0x3a275e96, 0x283c498b, 0x26354480,
    0x7c420fe9, 0x724b02e2, 0x605015ff, 0x6e5918f4, 0x44663bc5, 0x4a6f36ce, 0x587421d3, 0x567d2cd8,
    0x37a10c7a, 0x39a80171, 0x2bb3166c, 0x25ba1b67, 0x0f853856, 0x018c355d, 0x13972240, 0x1d9e2f4b,
    0x47e96422, 0x49e06929, 0x5bfb7e34, 0x55f2733f, 0x7fcd500e, 0x71c45d05, 0x63df4a18, 0x6dd64713,
    0xd731dcca, 0xd938d1c1, 0xcb23c6dc, 0xc52acbd7, 0xef15e8e6, 0xe11ce5ed, 0xf307f2f0, 0xfd0efffb,
    0xa779b492, 0xa970b999, 0xbb6bae84, 0xb562a38f, 0x9f5d80be, 0x91548db5, 0x834f9aa8, 0x8d4697a3
};

#else

/*
    Encryption table for LITTLE ENDIAN architecture.

    Lookup table for computing the MixColumns() Transformation with SubBytes applied to input value S.
    Each entry of this table represents the result of {02} dot S[x,c] | {01} dot S[x,c] | {01} dot S[x,c] | {03} dot S[x,c]. See Equation
    5.6 on page 18 AES specification(Pub 197)

    The most significant byte is the result of {02} dot S, and the least significant byte is the result
    of {03} dot S[x,c].
 */

NX_CRYPTO_AES_TABLE ULONG aes_encryption_table[256] =
{
    0xa56363c6, 0x847c7cf8, 0x997777ee, 0x8d7b7bf6, 0x0df2f2ff, 0xbd6b6bd6, 0xb16f6fde, 0x54c5c591,
    0x50303060, 0x03010102, 0xa96767ce, 0x7d2b2b56, 0x19fefee7, 0x62d7d7b5, 0xe6abab4d, 0x9a7676ec,
    0x45caca8f, 0x9d82821f, 0x40c9c989, 0x877d7dfa, 0x15fafaef, 0xeb5959b2, 0xc947478e, 0x0bf0f0fb,
    0xecadad41, 0x67d4d4b3, 0xfda2a25f, 0xeaafaf45, 0xbf9c9c23, 0xf7a4a453, 0x967272e4, 0x5bc0c09b,
    0xc2b7b775, 0x1cfdfde1, 0xae93933d, 0x6a26264c, 0x5a36366c, 0x413f3f7e, 0x02f7f7f5, 0x4fcccc83,
    0x5c343468, 0xf4a5a551, 0x34e5e5d1, 0x08f1f1f9, 0x937171e2, 0x73d8d8ab, 0x53313162, 0x3f15152a,
    0x0c040408, 0x52c7c795, 0x65232346, 0x5ec3c39d, 0x28181830, 0xa1969637, 0x0f05050a, 0xb59a9a2f,
    0x0907070e, 0x36121224, 0x9b80801b, 0x3de2e2df, 0x26ebebcd, 0x6927274e, 0xcdb2b27f, 0x9f7575ea,
    0x1b090912, 0x9e83831d, 0x742c2c58, 0x2e1a1a34, 0x2d1b1b36, 0xb26e6edc, 0xee5a5ab4, 0xfba0a05b,
    0xf65252a4, 0x4d3b3b76, 0x61d6d6b7, 0xceb3b37d, 0x7b292952, 0x3ee3e3dd, 0x712f2f5e, 0x97848413,
    0xf55353a6, 0x68d1d1b9, 0x00000000, 0x2cededc1, 0x60202040, 0x1ffcfce3, 0xc8b1b179, 0xed5b5bb6,
    0xbe6a6ad4, 0x46cbcb8d, 0xd9bebe67, 0x4b393972, 0xde4a4a94, 0xd44c4c98, 0xe85858b0, 0x4acfcf85,
    0x6bd0d0bb, 0x2aefefc5, 0xe5aaaa4f, 0x16fbfbed, 0xc5434386, 0xd74d4d9a, 0x55333366, 0x94858511,
    0xcf45458a, 0x10f9f9e9, 0x06020204, 0x817f7ffe, 0xf05050a0, 0x443c3c78, 0xba9f9f25, 0xe3a8a84b,
    0xf35151a2, 0xfea3a35d, 0xc0404080, 0x8a8f8f05, 0xad92923f, 0xbc9d9d21, 0x48383870, 0x04f5f5f1,
    0xdfbcbc63, 0xc1b6b677, 0x75dadaaf, 0x63212142, 0x30101020, 0x1affffe5, 0x0ef3f3fd, 0x6dd2d2bf,
    0x4ccdcd81, 0x140c0c18, 0x35131326, 0x2fececc3, 0xe15f5fbe, 0xa2979735, 0xcc444488, 0x3917172e,
    0x57c4c493, 0xf2a7a755, 0x827e7efc, 0x473d3d7a, 0xac6464c8, 0xe75d5dba, 0x2b191932, 0x957373e6,
    0xa06060c0, 0x98818119, 0xd14f4f9e, 0x7fdcdca3, 0x66222244, 0x7e2a2a54, 0xab90903b, 0x8388880b,
    0xca46468c, 0x29eeeec7, 0xd3b8b86b, 0x3c141428, 0x79dedea7, 0xe25e5ebc, 0x1d0b0b16, 0x76dbdbad,
    0x3be0e0db, 0x56323264, 0x4e3a3a74, 0x1e0a0a14, 0xdb494992, 0x0a06060c, 0x6c242448, 0xe45c5cb8,
    0x5dc2c29f, 0x6ed3d3bd, 0xefacac43, 0xa66262c4, 0xa8919139, 0xa4959531, 0x37e4e4d3, 0x8b7979f2,
    0x32e7e7d5, 0x43c8c88b, 0x5937376e, 0xb76d6dda, 0x8c8d8d01, 0x64d5d5b1, 0xd24e4e9c, 0xe0a9a949,
    0xb46c6cd8, 0xfa5656ac, 0x07f4f4f3, 0x25eaeacf, 0xaf6565ca, 0x8e7a7af4, 0xe9aeae47, 0x18080810,
    0xd5baba6f, 0x887878f0, 0x6f25254a, 0x722e2e5c, 0x241c1c38, 0xf1a6a657, 0xc7b4b473, 0x51c6c697,
    0x23e8e8cb, 0x7cdddda1, 0x9c7474e8, 0x211f1f3e, 0xdd4b4b96, 0xdcbdbd61, 0x868b8b0d, 0x858a8a0f,
    0x907070e0, 0x423e3e7c, 0xc4b5b571, 0xaa6666cc, 0xd8484890, 0x05030306, 0x01f6f6f7, 0x120e0e1c,
    0xa36161c2, 0x5f35356a, 0xf95757ae, 0xd0b9b969, 0x91868617, 0x58c1c199, 0x271d1d3a, 0xb99e9e27,
    0x38e1e1d9, 0x13f8f8eb, 0xb398982b, 0x33111122, 0xbb6969d2, 0x70d9d9a9, 0x898e8e07, 0xa7949433,
    0xb69b9b2d, 0x221e1e3c, 0x92878715, 0x20e9e9c9, 0x49cece87, 0xff5555aa, 0x78282850, 0x7adfdfa5,
    0x8f8c8c03, 0xf8a1a159, 0x80898909, 0x170d0d1a, 0xdabfbf65, 0x31e6e6d7, 0xc6424284, 0xb86868d0,
    0xc3414182, 0xb0999929, 0x772d2d5a, 0x110f0f1e, 0xcbb0b07b, 0xfc5454a8, 0xd6bbbb6d, 0x3a16162c
};

/*
   Decryption table for LITTLE ENDIAN architecture.

   Lookup table for computing the InvMixColumns() Transformation with InvSubBytes applied to input
   value S.  Each entry of this table represents
   the result of {0e} dot S[x,c] | {09} dot S[x,c] | {0d} dot S[x,c] | {0b} dot S[x,c]. See Equation
   5.10 on page 23 AES specification(Pub 197)

   The most significant byte is the result of {0e} dot S, and the least significant byte is the result
   of {0b} dot S[x,c].
 */
NX_CRYPTO_AES_TABLE ULONG aes_decryption_table[256] =
{
    0x50a7f451, 0x5365417e, 0xc3a4171a, 0x965e273a, 0xcb6bab3b, 0xf1459d1f, 0xab58faac, 0x9303e34b,
    0x55fa3020, 0xf66d76ad, 0x9176cc88, 0x254c02f5, 0xfcd7e54f, 0xd7cb2ac5, 0x80443526, 0x8fa362b5,
    0x495ab1de, 0x671bba25, 0x980eea45, 0xe1c0fe5d, 0x02752fc3, 0x12f04c81, 0xa397468d, 0xc6f9d36b,
    0xe75f8f03, 0x959c9215, 0xeb7a6dbf, 0xda595295, 0x2d83bed4, 0xd3217458, 0x2969e049, 0x44c8c98e,
    0x6a89c275, 0x78798ef4, 0x6b3e5899, 0xdd71b927, 0xb64fe1be, 0x17ad88f0, 0x66ac20c9, 0xb43ace7d,
    0x184adf63, 0x82311ae5, 0x60335197, 0x457f5362, 0xe07764b1, 0x84ae6bbb, 0x1ca081fe, 0x942b08f9,
    0x58684870, 0x19fd458f, 0x876cde94, 0xb7f87b52, 0x23d373ab, 0xe2024b72, 0x578f1fe3, 0x2aab5566,
    0x0728ebb2, 0x03c2b52f, 0x9a7bc586, 0xa50837d3, 0xf2872830, 0xb2a5bf23, 0xba6a0302, 0x5c8216ed,
    0x2b1ccf8a, 0x92b479a7, 0xf0f207f3, 0xa1e2694e, 0xcdf4da65, 0xd5be0506, 0x1f6234d1, 0x8afea6c4,
    0x9d532e34, 0xa055f3a2, 0x32e18a05, 0x75ebf6a4, 0x39ec830b, 0xaaef6040, 0x069f715e, 0x51106ebd,
    0xf98a213e, 0x3d06dd96, 0xae053edd, 0x46bde64d, 0xb58d5491, 0x055dc471, 0x6fd40604, 0xff155060,
    0x24fb9819, 0x97e9bdd6, 0xcc434089, 0x779ed967, 0xbd42e8b0, 0x888b8907, 0x385b19e7, 0xdbeec879,
    0x470a7ca1, 0xe90f427c, 0xc91e84f8, 0x00000000, 0x83868009, 0x48ed2b32, 0xac70111e, 0x4e725a6c,
    0xfbff0efd, 0x5638850f, 0x1ed5ae3d, 0x27392d36, 0x64d90f0a, 0x21a65c68, 0xd1545b9b, 0x3a2e3624,
    0xb1670a0c, 0x0fe75793, 0xd296eeb4, 0x9e919b1b, 0x4fc5c080, 0xa220dc61, 0x694b775a, 0x161a121c,
    0x0aba93e2, 0xe52aa0c0, 0x43e0223c, 0x1d171b12, 0x0b0d090e, 0xadc78bf2, 0xb9a8b62d, 0xc8a91e14,
    0x8519f157, 0x4c0775af, 0xbbdd99ee, 0xfd607fa3, 0x9f2601f7, 0xbcf5725c, 0xc53b6644, 0x347efb5b,
    0x7629438b, 0xdcc623cb, 0x68fcedb6, 0x63f1e4b8, 0xcadc31d7, 0x10856342, 0x40229713, 0x2011c684,
    0x7d244a85, 0xf83dbbd2, 0x1132f9ae, 0x6da129c7, 0x4b2f9e1d, 0xf330b2dc, 0xec52860d, 0xd0e3c177,
    0x6c16b32b, 0x99b970a9, 0xfa489411, 0x2264e947, 0xc48cfca8, 0x1a3ff0a0, 0xd82c7d56, 0xef903322,
    0xc74e4987, 0xc1d138d9, 0xfea2ca8c, 0x360bd498, 0xcf81f5a6, 0x28de7aa5, 0x268eb7da, 0xa4bfad3f,
    0xe49d3a2c, 0x0d927850, 0x9bcc5f6a, 0x62467e54, 0xc2138df6, 0xe8b8d890, 0x5ef7392e, 0xf5afc382,
    0xbe805d9f, 0x7c93d069, 0xa92dd56f, 0xb31225cf, 0x3b99acc8, 0xa77d1810, 0x6e639ce8, 0x7bbb3bdb,
    0x097826cd, 0xf418596e, 0x01b79aec, 0xa89a4f83, 0x656e95e6, 0x7ee6ffaa, 0x08cfbc21, 0xe6e815ef,
    0xd99be7ba, 0xce366f4a, 0xd4099fea, 0xd67cb029, 0xafb2a431, 0x31233f2a, 0x3094a5c6, 0xc066a235,
    0x37bc4e74, 0xa6ca82fc, 0xb0d090e0, 0x15d8a733, 0x4a9804f1, 0xf7daec41, 0x0e50cd7f, 0x2ff69117,
    0x8dd64d76, 0x4db0ef43, 0x544daacc, 0xdf0496e4, 0xe3b5d19e, 0x1b886a4c, 0xb81f2cc1, 0x7f516546,
    0x04ea5e9d, 0x5d358c01, 0x737487fa, 0x2e410bfb, 0x5a1d67b3, 0x52d2db92, 0x335610e9, 0x1347d66d,
    0x8c61d79a, 0x7a0ca137, 0x8e14f859, 0x893c13eb, 0xee27a9ce, 0x35c961b7, 0xede51ce1, 0x3cb1477a,
    0x59dfd29c, 0x3f73f255, 0x79ce1418, 0xbf37c773, 0xeacdf753, 0x5baafd5f, 0x146f3ddf, 0x86db4478,
    0x81f3afca, 0x3ec468b9, 0x2c342438, 0x5f40a3c2, 0x72c31d16, 0x0c25e2bc, 0x8b493c28, 0x41950dff,
    0x7101a839, 0xdeb30c08, 0x9ce4b4d8, 0x90c15664, 0x6184cb7b, 0x70b632d5, 0x745c6c48, 0x4257b8d0
};

/*
    Inverse Mix Table for LITTLE ENDIAN architecture.
    Lookup table for computing the InvMixColumns() Transformation.  Each entry of this table represents
    the result of {0e} dot S[x,c] | {09} dot S[x,c] | {0d} dot S[x,c] | {0b} dot S[x,c]. See Equation
    5.10 on page 23 AES specification(Pub 197)

    The most significant byte is the result of {0e} dot S, and the least significant byte is the result
    of {0b} dot S[x,c].
 */

NX_CRYPTO_AES_TABLE ULONG aes_inv_mix_table[256] =
{
    0x00000000, 0x0b0d090e, 0x161a121c, 0x1d171b12, 0x2c342438, 0x27392d36, 0x3a2e3624, 0x31233f2a,
    0x58684870, 0x5365417e, 0x4e725a6c, 0x457f5362, 0x745c6c48, 0x7f516546, 0x62467e54, 0x694b775a,
    0xb0d090e0, 0xbbdd99ee, 0xa6ca82fc, 0xadc78bf2, 0x9ce4b4d8, 0x97e9bdd6, 0x8afea6c4, 0x81f3afca,
    0xe8b8d890, 0xe3b5d19e, 0xfea2ca8c, 0xf5afc382, 0xc48cfca8, 0xcf81f5a6, 0xd296eeb4, 0xd99be7ba,
    0x7bbb3bdb, 0x70b632d5, 0x6da129c7, 0x66ac20c9, 0x578f1fe3, 0x5c8216ed, 0x41950dff, 0x4a9804f1,
    0x23d373ab, 0x28de7aa5, 0x35c961b7, 0x3ec468b9, 0x0fe75793, 0x04ea5e9d, 0x19fd458f, 0x12f04c81,
    0xcb6bab3b, 0xc066a235, 0xdd71b927, 0xd67cb029, 0xe75f8f03, 0xec52860d, 0xf1459d1f, 0xfa489411,
    0x9303e34b, 0x980eea45, 0x8519f157, 0x8e14f859, 0xbf37c773, 0xb43ace7d, 0xa92dd56f, 0xa220dc61,
    0xf66d76ad, 0xfd607fa3, 0xe07764b1, 0xeb7a6dbf, 0xda595295, 0xd1545b9b, 0xcc434089, 0xc74e4987,
    0xae053edd, 0xa50837d3, 0xb81f2cc1, 0xb31225cf, 0x82311ae5, 0x893c13eb, 0x942b08f9, 0x9f2601f7,
    0x46bde64d, 0x4db0ef43, 0x50a7f451, 0x5baafd5f, 0x6a89c275, 0x6184cb7b, 0x7c93d069, 0x779ed967,
    0x1ed5ae3d, 0x15d8a733, 0x08cfbc21, 0x03c2b52f, 0x32e18a05, 0x39ec830b, 0x24fb9819, 0x2ff69117,
    0x8dd64d76, 0x86db4478, 0x9bcc5f6a, 0x90c15664, 0xa1e2694e, 0xaaef6040, 0xb7f87b52, 0xbcf5725c,
    0xd5be0506, 0xdeb30c08, 0xc3a4171a, 0xc8a91e14, 0xf98a213e, 0xf2872830, 0xef903322, 0xe49d3a2c,
    0x3d06dd96, 0x360bd498, 0x2b1ccf8a, 0x2011c684, 0x1132f9ae, 0x1a3ff0a0, 0x0728ebb2, 0x0c25e2bc,
    0x656e95e6, 0x6e639ce8, 0x737487fa, 0x78798ef4, 0x495ab1de, 0x4257b8d0, 0x5f40a3c2, 0x544daacc,
    0xf7daec41, 0xfcd7e54f, 0xe1c0fe5d, 0xeacdf753, 0xdbeec879, 0xd0e3c177, 0xcdf4da65, 0xc6f9d36b,
    0xafb2a431, 0xa4bfad3f, 0xb9a8b62d, 0xb2a5bf23, 0x83868009, 0x888b8907, 0x959c9215, 0x9e919b1b,
    0x470a7ca1, 0x4c0775af, 0x51106ebd, 0x5a1d67b3, 0x6b3e5899, 0x60335197, 0x7d244a85, 0x7629438b,
    0x1f6234d1, 0x146f3ddf, 0x097826cd, 0x02752fc3, 0x335610e9, 0x385b19e7, 0x254c02f5, 0x2e410bfb,
    0x8c61d79a, 0x876cde94, 0x9a7bc586, 0x9176cc88, 0xa055f3a2, 0xab58faac, 0xb64fe1be, 0xbd42e8b0,
    0xd4099fea, 0xdf0496e4, 0xc2138df6, 0xc91e84f8, 0xf83dbbd2, 0xf330b2dc, 0xee27a9ce, 0xe52aa0c0,
    0x3cb1477a, 0x37bc4e74, 0x2aab5566, 0x21a65c68, 0x10856342, 0x1b886a4c, 0x069f715e, 0x0d927850,
    0x64d90f0a, 0x6fd40604, 0x72c31d16, 0x79ce1418, 0x48ed2b32, 0x43e0223c, 0x5ef7392e, 0x55fa3020,
    0x01b79aec, 0x0aba93e2, 0x17ad88f0, 0x1ca081fe, 0x2d83bed4, 0x268eb7da, 0x3b99acc8, 0x3094a5c6,
    0x59dfd29c, 0x52d2db92, 0x4fc5c080, 0x44c8c98e, 0x75ebf6a4, 0x7ee6ffaa, 0x63f1e4b8, 0x68fcedb6,
    0xb1670a0c, 0xba6a0302, 0xa77d1810, 0xac70111e, 0x9d532e34, 0x965e273a, 0x8b493c28, 0x80443526,
    0xe90f427c, 0xe2024b72, 0xff155060, 0xf418596e, 0xc53b6644, 0xce366f4a, 0xd3217458, 0xd82c7d56,
    0x7a0ca137, 0x7101a839, 0x6c16b32b, 0x671bba25, 0x5638850f, 0x5d358c01, 0x40229713, 0x4b2f9e1d,
    0x2264e947, 0x2969e049, 0x347efb5b, 0x3f73f255, 0x0e50cd7f, 0x055dc471, 0x184adf63, 0x1347d66d,
    0xcadc31d7, 0xc1d138d9, 0xdcc623cb, 0xd7cb2ac5, 0xe6e815ef, 0xede51ce1, 0xf0f207f3, 0xfbff0efd,
    0x92b479a7, 0x99b970a9, 0x84ae6bbb, 0x8fa362b5, 0xbe805d9f, 0xb58d5491, 0xa89a4f83, 0xa397468d
};


#endif

/* S-Box.  Refer to figure 7 on page 16,  AES specification(Pub 197) */
NX_CRYPTO_AES_TABLE UCHAR sub_bytes_sbox[] =
{
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

/* Inverse S-Box.  Refer to figure 14 on page 22,  AES specification(Pub 197) */
NX_CRYPTO_AES_TABLE UCHAR inverse_sub_bytes_sbox[] =
{
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};


/* Rcon array, used for key expansion.  Refer to Appendix A on page 27,  AES specification(Pub 197) */
NX_CRYPTO_AES_TABLE UCHAR aes_rcon_array[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

/* This array is generated using the nx_crypto_aes_multiply function, as powers of x in the
   AES polynomial field. The array is shifted such that the indexes line up so that x**1 is
   at index 1. The powers of x are cyclical, so the element at index 0 is actually x**255. */
/* For Little endian */
#ifdef NX_CRYPTO_LITTLE_ENDIAN
#define LEFT_ROTATE8(val)     (((val) >> 8) | ((val) << 24))
#define LEFT_ROTATE16(val)    (((val) >> 16) | ((val) << 16))
#define LEFT_ROTATE24(val)    (((val) >> 24) | ((val) << 8))
#define SET_MSB_BYTE(val)     ((UINT)(val))
#define SET_2ND_BYTE(val)     ((UINT)(val) << 8)
#define SET_3RD_BYTE(val)     ((UINT)(val) << 16)
#define SET_LSB_BYTE(val)     ((UINT)(val) << 24)
#define EXTRACT_MSB_BYTE(val) ((val) & 0xFF)
#define EXTRACT_2ND_BYTE(val) (((val) >> 8) & 0xFF)
#define EXTRACT_3RD_BYTE(val) (((val) >> 16) & 0xFF)
#define EXTRACT_LSB_BYTE(val) ((val) >> 24)

#define SWAP_ENDIAN(val)      ((((val) & 0xFF000000) >> 24) | (((val) & 0x00FF0000) >> 8) | (((val) & 0x0000FF00) << 8) | (((val) & 0x000000FF) << 24))



/* End of for little endian */
/* For Big Endian */
#else
#define LEFT_ROTATE8(val)     (((val) << 8) | ((val) >> 24))
#define LEFT_ROTATE16(val)    (((val) >> 16) | ((val) << 16))
#define LEFT_ROTATE24(val)    (((val) << 24) | ((val) >> 8))

#define SET_MSB_BYTE(val)     ((val) << 24)
#define SET_2ND_BYTE(val)     ((val) << 16)
#define SET_3RD_BYTE(val)     ((val) << 8)
#define SET_LSB_BYTE(val)     ((UINT)(val))
#define EXTRACT_MSB_BYTE(val) ((val) >> 24)
#define EXTRACT_2ND_BYTE(val) (((val) >> 16) & 0xFF)
#define EXTRACT_3RD_BYTE(val) (((val) >> 8) & 0xFF)
#define EXTRACT_LSB_BYTE(val) ((val) & 0xFF)

#endif

#ifdef NX_CRYPTO_SELF_TEST
extern UINT _nx_crypto_library_state;
#endif /* NX_CRYPTO_SELF_TEST */

/**************************************************************************/
/* Utility routines                                                       */
/**************************************************************************/



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_add_round_key                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function implements AddRoundKey() function described in        */
/*    section 5.1.4 of the AES specification(Pub 197)                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    round_key                             Pointer to key schedule       */
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
/*    _nx_crypto_aes_encrypt                Perform AES mode encryption   */
/*    _nx_crypto_aes_decrypt                Perform AES mode decryption   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), updated  */
/*                                            constants and conditionals, */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static VOID _nx_crypto_aes_add_round_key(NX_CRYPTO_AES *aes_ptr, UINT *round_key)
{
UINT i;

    for (i = 0; i < 4; ++i)
    {
        aes_ptr -> nx_crypto_aes_state[i] ^= round_key[i];
    }
}


/**************************************************************************/
/* Encryption routines                                                    */
/**************************************************************************/
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_sub_shift_roundkey                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function combines the following operations:                    */
/*       SubBytes(state)                                                  */
/*       Shiftrows(state)                                                 */
/*       AddRoundKey(state, w[Nr*Nb, (Nr+1)*Nb-1])                        */
/*    Refer to Figure 5 on page 15 of the AES Specification (Pub 197)     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    round_key                             Pointer to round key          */
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
/*    _nx_crypto_aes_encrypt                Perform AES mode encryption   */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_aes_sub_shift_roundkey(NX_CRYPTO_AES *aes_ptr, UINT *round_key)
{
/*
   S00    S04    S08    S12          S00    S04    S08    S12

   S01    S05    S09    S13          S05    S09    S13    S01
                              --->
   S02    S06    S10    S14          S10    S14    S02    S06

   S03    S07    S11    S15          S15    S03    S07    S11
 */


UINT   S0, S1, S2, S3;
UCHAR *S;

    S = (UCHAR *)aes_ptr -> nx_crypto_aes_state;

    S0 = ((SET_MSB_BYTE(sub_bytes_sbox[S[0]])) |
          (SET_2ND_BYTE(sub_bytes_sbox[S[5]])) |
          (SET_3RD_BYTE(sub_bytes_sbox[S[10]])) |
          (SET_LSB_BYTE(sub_bytes_sbox[S[15]]))) ^ round_key[0];

    S1 = ((SET_MSB_BYTE(sub_bytes_sbox[S[4]])) |
          (SET_2ND_BYTE(sub_bytes_sbox[S[9]])) |
          (SET_3RD_BYTE(sub_bytes_sbox[S[14]])) |
          (SET_LSB_BYTE(sub_bytes_sbox[S[03]]))) ^ round_key[1];

    S2 = ((SET_MSB_BYTE(sub_bytes_sbox[S[8]])) |
          (SET_2ND_BYTE(sub_bytes_sbox[S[13]])) |
          (SET_3RD_BYTE(sub_bytes_sbox[S[02]])) |
          (SET_LSB_BYTE(sub_bytes_sbox[S[07]]))) ^ round_key[2];

    S3 = ((SET_MSB_BYTE(sub_bytes_sbox[S[12]])) |
          (SET_2ND_BYTE(sub_bytes_sbox[S[1]])) |
          (SET_3RD_BYTE(sub_bytes_sbox[S[06]])) |
          (SET_LSB_BYTE(sub_bytes_sbox[S[11]]))) ^ round_key[3];

    aes_ptr -> nx_crypto_aes_state[0] = S0;
    aes_ptr -> nx_crypto_aes_state[1] = S1;
    aes_ptr -> nx_crypto_aes_state[2] = S2;
    aes_ptr -> nx_crypto_aes_state[3] = S3;

#ifdef NX_SECURE_KEY_CLEAR
    S0 = 0; S1 = 0; S2 = 0; S3 = 0;
#endif /* NX_SECURE_KEY_CLEAR  */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_inv_sub_shift_roundkey               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function combines the following operations in EqInvCipher:     */
/*       InvSubBytes(state)                                               */
/*       InvShiftrows(state)                                              */
/*       AddRoundKey(state, dw[0, Nb-1])                                  */
/*    Refer to Figure 15 on page 25 of the AES Specification (Pub 197)    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    round_key                             Pointer to round key          */
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
/*    _nx_crypto_aes_decrypt                Perform AES mode decryption   */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_aes_inv_sub_shift_roundkey(NX_CRYPTO_AES *aes_ptr, UINT *round_key)
{
/*
   S00    S04    S08    S12          S00    S04    S08    S12

   S01    S05    S09    S13          S13    S01    S05    S09
                              --->
   S02    S06    S10    S14          S10    S14    S02    S06

   S03    S07    S11    S15          S07    S11    S15    S03
 */


UINT   S0, S1, S2, S3;
UCHAR *S;

    S = (UCHAR *)aes_ptr -> nx_crypto_aes_state;

    S0 = ((SET_MSB_BYTE(inverse_sub_bytes_sbox[S[0]])) |
          (SET_2ND_BYTE(inverse_sub_bytes_sbox[S[13]])) |
          (SET_3RD_BYTE(inverse_sub_bytes_sbox[S[10]])) |
          (SET_LSB_BYTE(inverse_sub_bytes_sbox[S[7]]))) ^ round_key[0];

    S1 = ((SET_MSB_BYTE(inverse_sub_bytes_sbox[S[4]])) |
          (SET_2ND_BYTE(inverse_sub_bytes_sbox[S[1]])) |
          (SET_3RD_BYTE(inverse_sub_bytes_sbox[S[14]])) |
          (SET_LSB_BYTE(inverse_sub_bytes_sbox[S[11]]))) ^ round_key[1];

    S2 = ((SET_MSB_BYTE(inverse_sub_bytes_sbox[S[8]])) |
          (SET_2ND_BYTE(inverse_sub_bytes_sbox[S[5]])) |
          (SET_3RD_BYTE(inverse_sub_bytes_sbox[S[2]])) |
          (SET_LSB_BYTE(inverse_sub_bytes_sbox[S[15]]))) ^ round_key[2];

    S3 = ((SET_MSB_BYTE(inverse_sub_bytes_sbox[S[12]])) |
          (SET_2ND_BYTE(inverse_sub_bytes_sbox[S[9]])) |
          (SET_3RD_BYTE(inverse_sub_bytes_sbox[S[6]])) |
          (SET_LSB_BYTE(inverse_sub_bytes_sbox[S[3]]))) ^ round_key[3];


    aes_ptr -> nx_crypto_aes_state[0] = S0;
    aes_ptr -> nx_crypto_aes_state[1] = S1;
    aes_ptr -> nx_crypto_aes_state[2] = S2;
    aes_ptr -> nx_crypto_aes_state[3] = S3;

#ifdef NX_SECURE_KEY_CLEAR
    S0= 0; S1 = 0; S2 = 0; S3 = 0;
#endif /* NX_SECURE_KEY_CLEAR  */
}


/*

    __  __   __             __  __   __
 | S0'|   | 02  03  01  01|  | S0  |
 | S1'|   | 01  02  03  01|  | S1  |
 | S2'| = | 01  01  02  03|  | S2  |
 | S3'|   | 03  01  01  02|  | S3  |
    --  --   --             --  --   --

    S' =    V0   ^    V1   ^     V2  ^    V3
   S0' = {02}*S0 ^ {03}*S1 ^ {01}*S2 ^ {01}*S3
   S1' = {01}*S0 ^ {02}*S1 ^ {03}*S2 ^ {01}*S3
   S2' = {01}*S0 ^ {01}*S1 ^ {02}*S2 ^ {03}*S3
   S3' = {03}*S0 ^ {01}*S1 ^ {01}*S2 ^ {02}*S3


 */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_encryption_round                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function combines the following operations:                    */
/*       SubBytes(state)                                                  */
/*       Shiftrows(state)                                                 */
/*       MixColumns(state)                                                */
/*       AddRoundKey(state, w[round*Nb, (round+1)*Nb-1])                  */
/*    This step is the body of the loop in the middle of Cipher.          */
/*    Refer to Figure 5 on page 15 of the AES Specification (Pub 197)     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    num_rounds                            Indicates total number of     */
/*                                            rounds for this operation.  */
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
/*    _nx_crypto_aes_encrypt                Perform AES mode encryption   */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_aes_encryption_round(NX_CRYPTO_AES *aes_ptr, int num_rounds)
{
UINT state_data[4];
UINT saved_state[4];

int  round;


    state_data[0] = aes_ptr -> nx_crypto_aes_state[0];
    state_data[1] = aes_ptr -> nx_crypto_aes_state[1];
    state_data[2] = aes_ptr -> nx_crypto_aes_state[2];
    state_data[3] = aes_ptr -> nx_crypto_aes_state[3];

    for (round = 1; round < num_rounds; round++)
    {
        /* make a copy of the state data. */

        saved_state[0] = state_data[0];
        saved_state[1] = state_data[1];
        saved_state[2] = state_data[2];
        saved_state[3] = state_data[3];

        /*
           The MixColumns operates on the state after rotation.  The table below
           illustrates the bytes before and after the ShiftRows operation.
           This routine picks up bytes from its position BEFORE the rotation,
           effectively combining SubBytes, ShiftRows, MixColumns, AddRoundKey
           into one routine.

           S00    S04    S08    S12          S00    S04    S08    S12

           S01    S05    S09    S13          S05    S09    S13    S01
                                      --->
           S02    S06    S10    S14          S10    S14    S02    S06

           S03    S07    S11    S15          S15    S03    S07    S11

           The following code segment fully spells out the AES encryption, step by step.



           S = bytes[ 0];
           V0 = aes_encryption_table[S];
           S = bytes[ 5];
           val = aes_encryption_table[S];
           V1 = LEFT_ROTATE24(val);
           S = bytes[10];
           val = aes_encryption_table[S];
           V2 = LEFT_ROTATE16(val);
           S = bytes[15];
           val = aes_encryption_table[S];
           V3 = LEFT_ROTATE8(val);
           temp_state[0] = V0 ^ V1 ^ V2 ^ V3 ^ (aes_ptr -> nx_crypto_aes_key_schedule[round * 4]);

           S = bytes[ 4];
           V0 = aes_encryption_table[S];
           S = bytes[ 9];
           val = aes_encryption_table[S];
           V1 = LEFT_ROTATE24(val);
           S = bytes[14];
           val = aes_encryption_table[S];
           V2 = LEFT_ROTATE16(val);
           S = bytes[ 3];
           val = aes_encryption_table[S];
           V3 = LEFT_ROTATE8(val);
           temp_state[1] = V0 ^ V1 ^ V2 ^ V3 ^ (aes_ptr -> nx_crypto_aes_key_schedule[round * 4 + 1]);


           S = bytes[ 8];
           V0 = aes_encryption_table[S];
           S = bytes[13];
           val = aes_encryption_table[S];
           V1 = LEFT_ROTATE24(val);
           S = bytes[ 2];
           val = aes_encryption_table[S];
           V2 = LEFT_ROTATE16(val);
           S = bytes[ 7];
           val = aes_encryption_table[S];
           V3 = LEFT_ROTATE8(val);
           temp_state[2] = V0 ^ V1 ^ V2 ^ V3 ^ (aes_ptr -> nx_crypto_aes_key_schedule[round * 4 + 2]);

           S = bytes[12];
           V0 = aes_encryption_table[S];
           S = bytes[ 1];
           val = aes_encryption_table[S];
           V1 = LEFT_ROTATE24(val);
           S = bytes[ 6];
           val = aes_encryption_table[S];
           V2 = LEFT_ROTATE16(val);
           S = bytes[11];
           val = aes_encryption_table[S];
           V3 = LEFT_ROTATE8(val);
           temp_state[3] = V0 ^ V1 ^ V2 ^ V3 ^ (aes_ptr -> nx_crypto_aes_key_schedule[round * 4 + 3]);

         */

        /* The following logic implements the steps above but could allow compiler to produce more
           efficient code. */

        state_data[0] = aes_encryption_table[EXTRACT_MSB_BYTE(saved_state[0])] ^
            (LEFT_ROTATE24(aes_encryption_table[EXTRACT_2ND_BYTE(saved_state[1])])) ^
            (LEFT_ROTATE16(aes_encryption_table[EXTRACT_3RD_BYTE(saved_state[2])])) ^
            (LEFT_ROTATE8(aes_encryption_table[EXTRACT_LSB_BYTE(saved_state[3])])) ^
            (aes_ptr -> nx_crypto_aes_key_schedule[round * 4]);

        state_data[1] = aes_encryption_table[EXTRACT_MSB_BYTE(saved_state[1])] ^
            (LEFT_ROTATE24(aes_encryption_table[EXTRACT_2ND_BYTE(saved_state[2])])) ^
            (LEFT_ROTATE16(aes_encryption_table[EXTRACT_3RD_BYTE(saved_state[3])])) ^
            (LEFT_ROTATE8(aes_encryption_table[EXTRACT_LSB_BYTE(saved_state[0])])) ^
            (aes_ptr -> nx_crypto_aes_key_schedule[round * 4 + 1]);

        state_data[2] = aes_encryption_table[EXTRACT_MSB_BYTE(saved_state[2])] ^
            (LEFT_ROTATE24(aes_encryption_table[EXTRACT_2ND_BYTE(saved_state[3])])) ^
            (LEFT_ROTATE16(aes_encryption_table[EXTRACT_3RD_BYTE(saved_state[0])])) ^
            (LEFT_ROTATE8(aes_encryption_table[EXTRACT_LSB_BYTE(saved_state[1])])) ^
            (aes_ptr -> nx_crypto_aes_key_schedule[round * 4 + 2]);

        state_data[3] = aes_encryption_table[EXTRACT_MSB_BYTE(saved_state[3])] ^
            (LEFT_ROTATE24(aes_encryption_table[EXTRACT_2ND_BYTE(saved_state[0])])) ^
            (LEFT_ROTATE16(aes_encryption_table[EXTRACT_3RD_BYTE(saved_state[1])])) ^
            (LEFT_ROTATE8(aes_encryption_table[EXTRACT_LSB_BYTE(saved_state[2])])) ^
            (aes_ptr -> nx_crypto_aes_key_schedule[round * 4 + 3]);
    }

    /* Write the data back to state structure. */
    aes_ptr -> nx_crypto_aes_state[0] = state_data[0];
    aes_ptr -> nx_crypto_aes_state[1] = state_data[1];
    aes_ptr -> nx_crypto_aes_state[2] = state_data[2];
    aes_ptr -> nx_crypto_aes_state[3] = state_data[3];

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(state_data, 0, sizeof(state_data));
    NX_CRYPTO_MEMSET(saved_state, 0, sizeof(saved_state));
#endif /* NX_SECURE_KEY_CLEAR  */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_decryption_round                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function combines the following operations:                    */
/*       InvSubBytes(state)                                               */
/*       InvShiftrows(state)                                              */
/*       InvMixColumns(state)                                             */
/*       AddRoundKey(state, dw[round*Nb, (round+1)*Nb-1])                 */
/*    This step is the body of the loop in the middle of EqInvCipher.     */
/*    Refer to Figure 15 on page 25 of the AES Specification (Pub 197)    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    round                                 Indicates the number of       */
/*                                            rounds of this iteration.   */
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
/*    _nx_crypto_aes_decrypt                Perform AES mode decryption   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), updated  */
/*                                            constants, resulting        */
/*                                            in version 6.1              */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static VOID _nx_crypto_aes_decryption_round(NX_CRYPTO_AES *aes_ptr, int round)
{
UINT   temp_state[4];
UCHAR *bytes = (UCHAR *)aes_ptr -> nx_crypto_aes_state;
UINT  *w = aes_ptr -> nx_crypto_aes_decrypt_key_schedule;
UINT   V0, V1, V2, V3;

UCHAR  S;
ULONG  val;


/*
   The InvMixColumns operates on the state after rotation.  The table below
   illustrates the bytes before and after the InvShiftRows operation.
   This routine picks up bytes from its position BEFORE the rotation,
   effectively combining InvSubBytes, InvShiftRows, InvMixColumns, AddRoundKey
   into one routine.

   S00    S04    S08    S12          S00    S04    S08    S12

   S01    S05    S09    S13          S13    S01    S05    S09
                              --->
   S02    S06    S10    S14          S10    S14    S02    S06

   S03    S07    S11    S15          S07    S11    S15    S03
 */
    S = bytes[0];
    V0 = aes_decryption_table[S];
    S = bytes[13];
    val = aes_decryption_table[S];
    V1 = LEFT_ROTATE24(val);
    S = bytes[10];
    val = aes_decryption_table[S];
    V2 = LEFT_ROTATE16(val);
    S = bytes[7];
    val = aes_decryption_table[S];
    V3 = LEFT_ROTATE8(val);
    temp_state[0] = V0 ^ V1 ^ V2 ^ V3 ^ (w[round * 4]);

    S = bytes[4];
    V0 = aes_decryption_table[S];
    S = bytes[1];
    val = aes_decryption_table[S];
    V1 = LEFT_ROTATE24(val);
    S = bytes[14];
    val = aes_decryption_table[S];
    V2 = LEFT_ROTATE16(val);
    S = bytes[11];
    val = aes_decryption_table[S];
    V3 = LEFT_ROTATE8(val);
    temp_state[1] = V0 ^ V1 ^ V2 ^ V3 ^ (w[round * 4 + 1]);

    S = bytes[8];
    V0 = aes_decryption_table[S];
    S = bytes[5];
    val = aes_decryption_table[S];
    V1 = LEFT_ROTATE24(val);
    S = bytes[2];
    val = aes_decryption_table[S];
    V2 = LEFT_ROTATE16(val);
    S = bytes[15];
    val = aes_decryption_table[S];
    V3 = LEFT_ROTATE8(val);
    temp_state[2] = V0 ^ V1 ^ V2 ^ V3 ^ (w[round * 4 + 2]);

    S = bytes[12];
    V0 = aes_decryption_table[S];
    S = bytes[9];
    val = aes_decryption_table[S];
    V1 = LEFT_ROTATE24(val);
    S = bytes[6];
    val = aes_decryption_table[S];
    V2 = LEFT_ROTATE16(val);
    S = bytes[3];
    val = aes_decryption_table[S];
    V3 = LEFT_ROTATE8(val);
    temp_state[3] = V0 ^ V1 ^ V2 ^ V3 ^ (w[round * 4 + 3]);

    aes_ptr -> nx_crypto_aes_state[0] = temp_state[0];
    aes_ptr -> nx_crypto_aes_state[1] = temp_state[1];
    aes_ptr -> nx_crypto_aes_state[2] = temp_state[2];
    aes_ptr -> nx_crypto_aes_state[3] = temp_state[3];

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(temp_state, 0, sizeof(temp_state));
#endif /* NX_SECURE_KEY_CLEAR  */
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_encrypt                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs AES encryption on a block of 16 byte message */
/*    pointed to by "input", and the encrypted text is stored in buffer   */
/*    pointed to by "output".  The size of the output buffer must be at   */
/*    least 16 bytes.  The output buffer may point to the same input      */
/*    buffer, in which case the encrypted text overwrites the input       */
/*    message.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    input                                 Pointer to an input message   */
/*                                            of 16 bytes.                */
/*    output                                Pointer to an output buffer   */
/*                                            for storing the encrypted   */
/*                                            message.  The output buffer */
/*                                            must be at least 16 bytes.  */
/*    length                                The length of output buffer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_seucre_aes_add_round_key          Perform AddRoundKey operation */
/*    _nx_crypto_aes_encryption_round       The main body of AES          */
/*                                            encryption                  */
/*    _nx_crypto_aes_sub_shift_roundkey     Perform the last step in AES  */
/*                                            encryption operation        */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), disabled */
/*                                            unaligned access by default,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_aes_encrypt(NX_CRYPTO_AES *aes_ptr, UCHAR *input, UCHAR *output, UINT length)
{
UINT  num_rounds;
UINT *w;
#ifndef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
UCHAR *aes_state;
#else
UINT *buf;
#endif


    NX_CRYPTO_PARAMETER_NOT_USED(length);

    w = aes_ptr -> nx_crypto_aes_key_schedule;

    num_rounds = aes_ptr -> nx_crypto_aes_rounds;

    if (num_rounds < 10 || num_rounds > 14)
    {
        return(NX_CRYPTO_INVALID_PARAMETER);
    }

#ifndef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
    aes_state = (UCHAR *)aes_ptr -> nx_crypto_aes_state;
    aes_state[0] = input[0];
    aes_state[1] = input[1];
    aes_state[2] = input[2];
    aes_state[3] = input[3];
    aes_state[4] = input[4];
    aes_state[5] = input[5];
    aes_state[6] = input[6];
    aes_state[7] = input[7];
    aes_state[8] = input[8];
    aes_state[9] = input[9];
    aes_state[10] = input[10];
    aes_state[11] = input[11];
    aes_state[12] = input[12];
    aes_state[13] = input[13];
    aes_state[14] = input[14];
    aes_state[15] = input[15];
#else
    buf = (UINT *)input;
    aes_ptr -> nx_crypto_aes_state[0] = buf[0];
    aes_ptr -> nx_crypto_aes_state[1] = buf[1];
    aes_ptr -> nx_crypto_aes_state[2] = buf[2];
    aes_ptr -> nx_crypto_aes_state[3] = buf[3];
#endif

    _nx_crypto_aes_add_round_key(aes_ptr, &w[0]);

    _nx_crypto_aes_encryption_round(aes_ptr, (INT)num_rounds);

    _nx_crypto_aes_sub_shift_roundkey(aes_ptr, &w[num_rounds * 4]);


#ifndef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
    output[0] = aes_state[0];
    output[1] = aes_state[1];
    output[2] = aes_state[2];
    output[3] = aes_state[3];
    output[4] = aes_state[4];
    output[5] = aes_state[5];
    output[6] = aes_state[6];
    output[7] = aes_state[7];
    output[8] = aes_state[8];
    output[9] = aes_state[9];
    output[10] = aes_state[10];
    output[11] = aes_state[11];
    output[12] = aes_state[12];
    output[13] = aes_state[13];
    output[14] = aes_state[14];
    output[15] = aes_state[15];
#else
    buf = (UINT *)output;
    buf[0] = aes_ptr -> nx_crypto_aes_state[0];
    buf[1] = aes_ptr -> nx_crypto_aes_state[1];
    buf[2] = aes_ptr -> nx_crypto_aes_state[2];
    buf[3] = aes_ptr -> nx_crypto_aes_state[3];
#endif

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/* Key expansion routines                                                 */
/**************************************************************************/
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_subword                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs SubBytes(state) operation according to       */
/*    section 5.1.1 on page 15 of the AES specification (Pub 197)         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*   word                                   The input 4-byte word         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*   UINT word                              The value after being         */
/*                                            substituted.                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_aes_key_expansion          AES key expansion             */
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
NX_CRYPTO_KEEP static UINT _nx_crypto_aes_subword(UINT word)
{
UINT result;

    result = sub_bytes_sbox[word & 0xFF];
    result |= (UINT)((sub_bytes_sbox[(word & 0x0000FF00) >>  8]) <<  8);
    result |= (UINT)((sub_bytes_sbox[(word & 0x00FF0000) >> 16]) << 16);
    result |= (UINT)((sub_bytes_sbox[(word & 0xFF000000) >> 24]) << 24);
    return result;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_key_expansion                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine performs AES Key Expansion as outlined in seciton 5.2  */
/*    on page 195 of the AES specification (Pub 197)                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_aes_subword                Apply sbox substitution       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_aes_key_set                Set AES crypto key            */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_aes_key_expansion(NX_CRYPTO_AES *aes_ptr)
{
UINT  temp;
UINT  i;
UINT  iterations = 0;
UINT  loop_count;
UINT *expanded_key;
UINT  key_size;

    expanded_key = aes_ptr -> nx_crypto_aes_key_schedule;

    key_size = aes_ptr -> nx_crypto_aes_key_size;

    switch (key_size)
    {
    case NX_CRYPTO_AES_KEY_SIZE_128_BITS:
        iterations = 9;
        break;
    case NX_CRYPTO_AES_KEY_SIZE_192_BITS:
        iterations = 7;
        break;

    /* case NX_CRYPTO_AES_KEY_SIZE_256_BITS: */
    default:
        iterations = 6;
        break;
    }

    temp = expanded_key[key_size - 1];
    /* Expand the key schedule from the initial key. */
    i = key_size;
    for (loop_count = 0; loop_count < iterations; loop_count++)
    {

        temp = LEFT_ROTATE8(temp);                              /* (temp << 8) | (temp >> 24); */
        temp = _nx_crypto_aes_subword(temp);
        temp ^= SET_MSB_BYTE((UINT)aes_rcon_array[loop_count]); /* (((UINT)aes_rcon_array[loop_count]) << 24); */
        temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
        switch (key_size)
        {
        case NX_CRYPTO_AES_KEY_SIZE_256_BITS:
            temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
            temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
        /* AES KEY 256 bits: fall through the rest of the statement to complete
           one iteration of key expansion. */ /* fallthrough */
        case NX_CRYPTO_AES_KEY_SIZE_192_BITS:
            temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;

            /* Special case for AES 256: Need an extra subword operation on 4th step. */
            if (key_size == NX_CRYPTO_AES_KEY_SIZE_256_BITS)
            {
                temp = _nx_crypto_aes_subword(temp);
            }
            temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
        /* AES KEY 256 and 192 bits: fall through the rest of the statement to complete
           one iteration of key expansion. */ /* fallthrough */
        /* case NX_CRYPTO_AES_KEY_SIZE_128_BITS: */
        default:
            temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
            temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
            temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
            break;
        }
    }
    /* Last round. */
    temp = LEFT_ROTATE8(temp);                              /* (temp << 8) | (temp >> 24); */
    temp = _nx_crypto_aes_subword(temp);
    temp ^= SET_MSB_BYTE((UINT)aes_rcon_array[loop_count]); /* (((UINT)aes_rcon_array[loop_count]) << 24); */
    temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
    temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
    temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;
    temp = expanded_key[i - key_size] ^ temp; expanded_key[i] = temp; i++;

#ifdef NX_SECURE_KEY_CLEAR
    temp = 0;
#endif /* NX_SECURE_KEY_CLEAR  */

    return;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_key_expansion_inverse                PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This routine performs AES Key Expansion as outlined in seciton 5.2  */
/*    on page 195 of the AES specification (Pub 197)                      */
/*                                                                        */
/*    Note on the decryption side, NetX Crypto AES uses Equivalent        */
/*    Inverse Cipher (section 5.3.5 in Pub 197).  Therefore the inverse   */
/*    key expansion starts with the regular key expansion then apply      */
/*    InvMixClumns(state). (Page 24 Pub 197).                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
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
/*    _nx_crypto_aes_key_set                Set AES crypto key            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            moved inverse key expansion,*/
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static VOID _nx_crypto_aes_key_expansion_inverse(NX_CRYPTO_AES *aes_ptr)
{
UINT  i;
UINT  num_rounds;
UINT *expanded_key;
UINT  V0, V1, V2, V3;

UCHAR S;
ULONG val;
ULONG key;

    expanded_key = aes_ptr -> nx_crypto_aes_decrypt_key_schedule;

    num_rounds = aes_ptr -> nx_crypto_aes_rounds;

    /* Copy the 1st set of keys */
    for (i = 0; i < 4; i++)
    {
        aes_ptr -> nx_crypto_aes_decrypt_key_schedule[i] = aes_ptr -> nx_crypto_aes_key_schedule[i];
    }

    for (; i < 4 * (num_rounds); i++)
    {

        /* Pick up the key value from key_schedule[]. */
        key = aes_ptr -> nx_crypto_aes_key_schedule[i];
        S = EXTRACT_MSB_BYTE(key); /* (key >> 24) & 0xFF; */

        /* Performs InvMixColumns() operation by using the look up table. */
        V0 = aes_inv_mix_table[S];

        S = EXTRACT_2ND_BYTE(key);        /* (key >> 16) & 0xFF; */
        val = aes_inv_mix_table[S];
        V1 = LEFT_ROTATE24(val);          /* (val >> 8) | (val << 24); */

        S = EXTRACT_3RD_BYTE(key);        /* (key >>  8) & 0xFF;*/
        val = aes_inv_mix_table[S];
        V2 = LEFT_ROTATE16(val);          /* (val >> 16) | (val << 16); */

        S = (UCHAR)EXTRACT_LSB_BYTE(key); /* key & 0xFF; */
        val = aes_inv_mix_table[S];
        V3 = LEFT_ROTATE8(val);           /* (val >> 24) | (val << 8);*/

        /* Put values together */
        key =  V0 ^ V1 ^ V2 ^ V3;

        /* Stores the expanded key (after applying InvMixColumns()) */
        expanded_key[i] = key;
    }

    /* No need to perform InvMixColums() on the last 4 words. */
    expanded_key[i]     = aes_ptr -> nx_crypto_aes_key_schedule[i];
    expanded_key[i + 1] = aes_ptr -> nx_crypto_aes_key_schedule[i + 1];
    expanded_key[i + 2] = aes_ptr -> nx_crypto_aes_key_schedule[i + 2];
    expanded_key[i + 3] = aes_ptr -> nx_crypto_aes_key_schedule[i + 3];

    /* Set the inverse key expansion flag. */
    aes_ptr -> nx_crypto_aes_inverse_key_expanded = 1;

#ifdef NX_SECURE_KEY_CLEAR
    key = 0;
#endif /* NX_SECURE_KEY_CLEAR  */

    return;
}


/**************************************************************************/
/* Decryption routines                                                    */
/**************************************************************************/




/*

    __  __   __             __  __   __
 | S0'|   | 0e  0b  0d  09|  | S0  |
 | S1'|   | 09  0e  0b  0d|  | S1  |
 | S2'| = | 0d  09  0e  0b|  | S2  |
 | S3'|   | 0b  0d  09  0e|  | S3  |
    --  --   --             --  --   --

    S' =    V0   ^    V1   ^     V2  ^    V3
   S0' = {0e}*S0 ^ {0b}*S1 ^ {0d}*S2 ^ {09}*S3
   S1' = {09}*S0 ^ {0e}*S1 ^ {0b}*S2 ^ {0d}*S3
   S2' = {0d}*S0 ^ {09}*S1 ^ {0e}*S2 ^ {0b}*S3
   S3' = {0b}*S0 ^ {0d}*S1 ^ {09}*S2 ^ {0e}*S3


 */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_decrypt                              PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs AES decryption on a block of 16 byte message */
/*    pointed to by "input", and the decrypted text is stored in buffer   */
/*    pointed to by "output".  The size of the output buffer must be at   */
/*    least 16 bytes.  The output buffer may point to the same input      */
/*    buffer, in which case the decrypted text overwrites the input       */
/*    message.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    input                                 Pointer to an input message   */
/*                                            of 16 bytes.                */
/*    output                                Pointer to an output buffer   */
/*                                            for storing the decrypted   */
/*                                            message.  The output buffer */
/*                                            must be at least 16 bytes.  */
/*    length                                The length of output buffer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_seucre_aes_add_round_key          Perform AddRoundKey operation */
/*    _nx_crypto_aes_decryption_round       The main body of AES          */
/*                                            decryption                  */
/*    _nx_crypto_aes_inv_sub_shift_roundkey Perform the last step in AES  */
/*                                            decryption operation        */
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
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            moved inverse key expansion,*/
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_aes_decrypt(NX_CRYPTO_AES *aes_ptr, UCHAR *input, UCHAR *output, UINT length)
{
UINT  num_rounds;
UINT  round;
UINT *w;
UINT *v;
#ifndef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
UCHAR *aes_state;
#else
UINT *buf;
#endif


    NX_CRYPTO_PARAMETER_NOT_USED(length);

    /* If the flag is not set, we assume the inverse key expansion 
       table is not created yet. Call the routine to create one. */
    if(aes_ptr -> nx_crypto_aes_inverse_key_expanded == 0)
    {
        _nx_crypto_aes_key_expansion_inverse(aes_ptr);
    }


    w = aes_ptr -> nx_crypto_aes_decrypt_key_schedule;
    v = aes_ptr -> nx_crypto_aes_key_schedule;

#ifndef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
    aes_state = (UCHAR *)aes_ptr -> nx_crypto_aes_state;
    aes_state[0] = input[0];
    aes_state[1] = input[1];
    aes_state[2] = input[2];
    aes_state[3] = input[3];
    aes_state[4] = input[4];
    aes_state[5] = input[5];
    aes_state[6] = input[6];
    aes_state[7] = input[7];
    aes_state[8] = input[8];
    aes_state[9] = input[9];
    aes_state[10] = input[10];
    aes_state[11] = input[11];
    aes_state[12] = input[12];
    aes_state[13] = input[13];
    aes_state[14] = input[14];
    aes_state[15] = input[15];
#else
    buf = (UINT *)input;
    aes_ptr -> nx_crypto_aes_state[0] = buf[0];
    aes_ptr -> nx_crypto_aes_state[1] = buf[1];
    aes_ptr -> nx_crypto_aes_state[2] = buf[2];
    aes_ptr -> nx_crypto_aes_state[3] = buf[3];
#endif


    num_rounds = aes_ptr -> nx_crypto_aes_rounds;

    if (num_rounds < 10 || num_rounds > 14)
    {
        return(NX_CRYPTO_INVALID_PARAMETER);
    }

    _nx_crypto_aes_add_round_key(aes_ptr, &v[num_rounds * 4]);

    for (round = num_rounds - 1; round >= 1; --round)
    {

        _nx_crypto_aes_decryption_round(aes_ptr, (INT)round);
    }

    _nx_crypto_aes_inv_sub_shift_roundkey(aes_ptr, &w[0]);

    /* Extract the output encrypted block. */
#ifndef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
    output[0] = aes_state[0];
    output[1] = aes_state[1];
    output[2] = aes_state[2];
    output[3] = aes_state[3];
    output[4] = aes_state[4];
    output[5] = aes_state[5];
    output[6] = aes_state[6];
    output[7] = aes_state[7];
    output[8] = aes_state[8];
    output[9] = aes_state[9];
    output[10] = aes_state[10];
    output[11] = aes_state[11];
    output[12] = aes_state[12];
    output[13] = aes_state[13];
    output[14] = aes_state[14];
    output[15] = aes_state[15];
#else
    buf = (UINT *)output;
    buf[0] = aes_ptr -> nx_crypto_aes_state[0];
    buf[1] = aes_ptr -> nx_crypto_aes_state[1];
    buf[2] = aes_ptr -> nx_crypto_aes_state[2];
    buf[3] = aes_ptr -> nx_crypto_aes_state[3];
#endif

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_key_set                              PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function configures key for AES encryption and decryption.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    key                                   Pointer to key string         */
/*    key_size                              Size of the key, valid values */
/*                                            are:                        */
/*                                        NX_CRYPTO_AES_KEY_SIZE_128_BITS */
/*                                        NX_CRYPTO_AES_KEY_SIZE_192_BITS */
/*                                        NX_CRYPTO_AES_KEY_SIZE_256_BITS */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_aes_key_expansion          Key expansion for encryption  */
/*    _nx_crypto_aes_key_expansion_inverse  Key expansion for decryption  */
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
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            moved inverse key expansion,*/
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_aes_key_set(NX_CRYPTO_AES *aes_ptr, UCHAR *key, UINT key_size)
{
UCHAR *expanded_key;
UINT   i;

    /* Set the AES key size (should be in 32-bit *words*). */
    aes_ptr -> nx_crypto_aes_key_size = (USHORT)key_size;

    expanded_key = (UCHAR *)aes_ptr -> nx_crypto_aes_key_schedule;

    /* Copy the key into the beginning of the expanded key buffer. */
    for (i = 0; i < key_size * 4; ++i)
    {
        expanded_key[i]  = key[i];
    }

    /* Based on the key size, determine the number of rounds. */
    switch (key_size)
    {
    case NX_CRYPTO_AES_KEY_SIZE_128_BITS:
        aes_ptr -> nx_crypto_aes_rounds = 10;
        break;
    case NX_CRYPTO_AES_KEY_SIZE_192_BITS:
        aes_ptr -> nx_crypto_aes_rounds = 12;
        break;
    /* case NX_CRYPTO_AES_KEY_SIZE_256_BITS: */
    default:
        aes_ptr -> nx_crypto_aes_rounds = 14;
        break;
    }


    _nx_crypto_aes_key_expansion(aes_ptr);

    /* Move key_expansion_inverse into the decrypt logic. 
       No reason to build the inverse table if the application doesn't do decryption. */
    /* Clear the inverse key expansion flag; */
    aes_ptr -> nx_crypto_aes_inverse_key_expanded = 0;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_init                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the AES crypto module.                    */
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
/*    _nx_crypto_aes_key_set                Set the key for AES           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_operation       Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_aes_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                                UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                VOID **handle,
                                                VOID *crypto_metadata,
                                                ULONG crypto_metadata_size)
{

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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_AES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* We only support 128-bit or 256-bit key size for the time-being. */
    if ((key_size_in_bits != NX_CRYPTO_AES_128_KEY_LEN_IN_BITS) && (key_size_in_bits != NX_CRYPTO_AES_192_KEY_LEN_IN_BITS) && (key_size_in_bits != NX_CRYPTO_AES_256_KEY_LEN_IN_BITS))
    {
        return(NX_CRYPTO_UNSUPPORTED_KEY_SIZE);
    }

    NX_CRYPTO_MEMSET(&((NX_CRYPTO_AES *)crypto_metadata) -> nx_crypto_aes_mode_context, 0, sizeof(((NX_CRYPTO_AES *)crypto_metadata) -> nx_crypto_aes_mode_context));

    _nx_crypto_aes_key_set((NX_CRYPTO_AES *)crypto_metadata, key, key_size_in_bits >> 5);

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_cleanup                       PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_aes_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_AES));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_operation                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES algorithm.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
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
/*    _nx_crypto_method_aes_cbc_operation   Handle AES CBC operation      */
/*    _nx_crypto_method_aes_ccm_operation   Handle AES CCM operation      */
/*    _nx_crypto_method_aes_gcm_operation   Handle AES GCM operation      */
/*    _nx_crypto_method_aes_ctr_operation   Handle AES CTR operation      */
/*    _nx_crypto_method_aes_xcbc_operation  Handle AES XCBC operation     */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_aes_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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
UINT    status;

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_AES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Check if the algorithm is cbc or ctr. */
    if (method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
    {

        /* AES CBC */
        status = _nx_crypto_method_aes_cbc_operation(op, handle, method, key, key_size_in_bits,
                                                     input, input_length_in_byte, iv_ptr,
                                                     output, output_length_in_byte,
                                                     crypto_metadata, crypto_metadata_size,
                                                     packet_ptr, nx_crypto_hw_process_callback);
    }
    else if ((method -> nx_crypto_algorithm >= NX_CRYPTO_ENCRYPTION_AES_CCM_8) &&
             (method -> nx_crypto_algorithm <= NX_CRYPTO_ENCRYPTION_AES_CCM))
    {

        /* AES CCM */
        status = _nx_crypto_method_aes_ccm_operation(op, handle, method, key, key_size_in_bits,
                                                     input, input_length_in_byte, iv_ptr,
                                                     output, output_length_in_byte,
                                                     crypto_metadata, crypto_metadata_size,
                                                     packet_ptr, nx_crypto_hw_process_callback);

    }
    else if ((method -> nx_crypto_algorithm >= NX_CRYPTO_ENCRYPTION_AES_GCM_8) &&
             (method -> nx_crypto_algorithm <= NX_CRYPTO_ENCRYPTION_AES_GCM_16))
    {

        /* AES GCM */
        status = _nx_crypto_method_aes_gcm_operation(op, handle, method, key, key_size_in_bits,
                                                     input, input_length_in_byte, iv_ptr,
                                                     output, output_length_in_byte,
                                                     crypto_metadata, crypto_metadata_size,
                                                     packet_ptr, nx_crypto_hw_process_callback);

    }
    else if (method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CTR)
    {

        /* AES_CTR */
        status = _nx_crypto_method_aes_ctr_operation(op, handle, method, key, key_size_in_bits,
                                                     input, input_length_in_byte, iv_ptr,
                                                     output, output_length_in_byte,
                                                     crypto_metadata, crypto_metadata_size,
                                                     packet_ptr, nx_crypto_hw_process_callback);
    }
    else if (method -> nx_crypto_algorithm == NX_CRYPTO_AUTHENTICATION_AES_XCBC_MAC_96)
    {
        status = _nx_crypto_method_aes_xcbc_operation(op, handle, method, key, key_size_in_bits,
                                                      input, input_length_in_byte, iv_ptr,
                                                      output, output_length_in_byte,
                                                      crypto_metadata, crypto_metadata_size,
                                                      packet_ptr, nx_crypto_hw_process_callback);
    }
    else
    {
        status = NX_CRYPTO_INVALID_ALGORITHM;
    }

    return status;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_cbc_operation                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES CBC algorithm.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
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
/*    _nx_crypto_cbc_encrypt                Perform CBC mode encryption   */
/*    _nx_crypto_cbc_decrypt                Perform CBC mode decryption   */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_aes_cbc_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

NX_CRYPTO_AES *ctx;
UINT    status;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);
    
    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_AES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_AES *)crypto_metadata;

    switch (op)
    {
        case NX_CRYPTO_DECRYPT:
        {
            status = _nx_crypto_cbc_decrypt_init(&(ctx -> nx_crypto_aes_mode_context.cbc),
                                                 iv_ptr, method -> nx_crypto_IV_size_in_bits >> 3);
            if (status)
            {
                break;
            }

            status = _nx_crypto_cbc_decrypt(ctx, &(ctx -> nx_crypto_aes_mode_context.cbc),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_decrypt,
                                            input, output, input_length_in_byte,
                                            NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT:
        {
            status = _nx_crypto_cbc_encrypt_init(&(ctx -> nx_crypto_aes_mode_context.cbc),
                                                 iv_ptr, method -> nx_crypto_IV_size_in_bits >> 3);
            if (status)
            {
                break;
            }

            status = _nx_crypto_cbc_encrypt(ctx, &(ctx -> nx_crypto_aes_mode_context.cbc),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                            input, output, input_length_in_byte,
                                            NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_DECRYPT_INITIALIZE:
        {
            status = _nx_crypto_cbc_decrypt_init(&(ctx -> nx_crypto_aes_mode_context.cbc),
                                                 iv_ptr, method -> nx_crypto_IV_size_in_bits >> 3);
        } break;

        case NX_CRYPTO_ENCRYPT_INITIALIZE:
        {
            status = _nx_crypto_cbc_encrypt_init(&(ctx -> nx_crypto_aes_mode_context.cbc),
                                                 iv_ptr, method -> nx_crypto_IV_size_in_bits >> 3);
        } break;

        case NX_CRYPTO_DECRYPT_UPDATE:
        {
            status = _nx_crypto_cbc_decrypt(ctx, &(ctx -> nx_crypto_aes_mode_context.cbc),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_decrypt,
                                            input, output, input_length_in_byte,
                                            NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_UPDATE:
        {
            status = _nx_crypto_cbc_encrypt(ctx, &(ctx -> nx_crypto_aes_mode_context.cbc),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                            input, output, input_length_in_byte,
                                            NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_CALCULATE:
        /* fallthrough */
        case NX_CRYPTO_DECRYPT_CALCULATE:
        {

            /* Nothing to do. */
            status = NX_CRYPTO_SUCCESS;
        } break;

        default:
        {
            status = NX_CRYPTO_INVALID_ALGORITHM;
        } break;
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_ccm_operation                 PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES CCM algorithm.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
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
/*    _nx_crypto_cbc_encrypt                Perform CBC mode encryption   */
/*    _nx_crypto_cbc_decrypt                Perform CBC mode decryption   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_operation       Handle AES encrypt or decrypt */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  xx-xx-xxxx     Tiejun Zhou              Modified comment(s), and      */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.x    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_aes_ccm_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

NX_CRYPTO_AES *ctx;
UINT    status;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);
    
    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_AES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_AES *)crypto_metadata;

    if ((method -> nx_crypto_algorithm < NX_CRYPTO_ENCRYPTION_AES_CCM_8) ||
        (method -> nx_crypto_algorithm > NX_CRYPTO_ENCRYPTION_AES_CCM))
    {
        return(NX_CRYPTO_INVALID_ALGORITHM);
    }

    /* IV : Nonce length(1 byte) + Nonce
       nx_crypto_ICV_size_in_bits: authentication tag length in bits */
    switch (op)
    {
        case NX_CRYPTO_DECRYPT:
        {
            if (iv_ptr == NX_CRYPTO_NULL ||
                (ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data_len > 0 &&
                 ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data == NX_CRYPTO_NULL))
            {
                status = NX_CRYPTO_PTR_ERROR;
                break;
            }

            if (input_length_in_byte < (ULONG)(method -> nx_crypto_ICV_size_in_bits >> 3) ||
                output_length_in_byte < input_length_in_byte - (method -> nx_crypto_ICV_size_in_bits >> 3))
            {
                status = NX_CRYPTO_INVALID_BUFFER_SIZE;
                break;
            }

            status = _nx_crypto_ccm_decrypt_init(ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                 ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data,
                                                 ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data_len,
                                                 input_length_in_byte - (method -> nx_crypto_ICV_size_in_bits >> 3), iv_ptr, 
                                                 (UCHAR)(method -> nx_crypto_ICV_size_in_bits >> 3),
                                                 NX_CRYPTO_AES_BLOCK_SIZE);

            if (status)
            {
                break;
            }

            status = _nx_crypto_ccm_decrypt_update(NX_CRYPTO_DECRYPT_UPDATE,
                                                   ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                   (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                   input, output, input_length_in_byte - (method -> nx_crypto_ICV_size_in_bits >> 3),
                                                   NX_CRYPTO_AES_BLOCK_SIZE);
            if (status)
            {
                break;
            }

            status = _nx_crypto_ccm_decrypt_calculate(ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                      (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                      input + input_length_in_byte - (method -> nx_crypto_ICV_size_in_bits >> 3),
                                                      NX_CRYPTO_AES_BLOCK_SIZE);
            if (status)
            {
                break;
            }
        } break;

        case NX_CRYPTO_ENCRYPT:
        {
            if (iv_ptr == NX_CRYPTO_NULL ||
                (ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data_len > 0 &&
                 ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data == NX_CRYPTO_NULL))
            {
                status = NX_CRYPTO_PTR_ERROR;
                break;
            }

            if (output_length_in_byte < input_length_in_byte + (method -> nx_crypto_ICV_size_in_bits >> 3))
            {
                status = NX_CRYPTO_INVALID_BUFFER_SIZE;
                break;
            }

            status = _nx_crypto_ccm_encrypt_init(ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                 ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data,
                                                 ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data_len,
                                                 input_length_in_byte, iv_ptr, 
                                                 (UCHAR)(method -> nx_crypto_ICV_size_in_bits >> 3),
                                                 NX_CRYPTO_AES_BLOCK_SIZE);

            if (status)
            {
                break;
            }

            status = _nx_crypto_ccm_encrypt_update(NX_CRYPTO_ENCRYPT_UPDATE,
                                                   ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                   (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                   input, output, input_length_in_byte,
                                                   NX_CRYPTO_AES_BLOCK_SIZE);
            if (status)
            {
                break;
            }

            status = _nx_crypto_ccm_encrypt_calculate(ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                      (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                      output + input_length_in_byte,
                                                      NX_CRYPTO_AES_BLOCK_SIZE);
            if (status)
            {
                break;
            }
        } break;

        case NX_CRYPTO_SET_ADDITIONAL_DATA:
        {

            /* Set additonal data pointer.  */
            ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data = (VOID *)input;

            /* Set additional data length.  */
            ctx -> nx_crypto_aes_mode_context.ccm.nx_crypto_ccm_additional_data_len = input_length_in_byte;

            status = NX_CRYPTO_SUCCESS;
        } break;

        case NX_CRYPTO_DECRYPT_INITIALIZE:
        {
            if (iv_ptr == NX_CRYPTO_NULL)
            {
                status = NX_CRYPTO_PTR_ERROR;
                break;
            }

            status = _nx_crypto_ccm_decrypt_init(ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                 input, /* pointers to AAD */
                                                 input_length_in_byte, /* length of AAD */
                                                 output_length_in_byte, /* total length of message */
                                                 iv_ptr, 
                                                 (UCHAR)(method -> nx_crypto_ICV_size_in_bits >> 3),
                                                 NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_DECRYPT_UPDATE:
        {
            status = _nx_crypto_ccm_decrypt_update(NX_CRYPTO_DECRYPT_UPDATE,
                                                   ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                   (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                   input, output, input_length_in_byte,
                                                   NX_CRYPTO_AES_BLOCK_SIZE);

        } break;

        case NX_CRYPTO_DECRYPT_CALCULATE:
        {
            status = _nx_crypto_ccm_decrypt_calculate(ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                      (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                      input, NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_INITIALIZE:
        {
            if (iv_ptr == NX_CRYPTO_NULL)
            {
                status = NX_CRYPTO_PTR_ERROR;
                break;
            }

            status = _nx_crypto_ccm_encrypt_init(ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                 input, /* pointers to AAD */
                                                 input_length_in_byte, /* length of AAD */
                                                 output_length_in_byte, /* total length of message */
                                                 iv_ptr, 
                                                 (UCHAR)(method -> nx_crypto_ICV_size_in_bits >> 3),
                                                 NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_UPDATE:
        {

            status = _nx_crypto_ccm_encrypt_update(NX_CRYPTO_ENCRYPT_UPDATE,
                                                   ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                   (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                   input, output, input_length_in_byte,
                                                   NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_CALCULATE:
        {

            status = _nx_crypto_ccm_encrypt_calculate(ctx, &(ctx -> nx_crypto_aes_mode_context.ccm),
                                                      (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                      output, NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        default:
        {
            status = NX_CRYPTO_INVALID_ALGORITHM;
        } break;
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_gcm_operation                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES GCM algorithm.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
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
/*    _nx_crypto_gcm_operation              Perform GCM encryption or     */
/*                                            decryption                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_operation       Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_aes_gcm_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

NX_CRYPTO_AES *ctx;
UINT icv_len;
UINT message_len;
UINT    status;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_AES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_AES *)crypto_metadata;

    if ((method -> nx_crypto_algorithm < NX_CRYPTO_ENCRYPTION_AES_GCM_8) ||
        (method -> nx_crypto_algorithm > NX_CRYPTO_ENCRYPTION_AES_GCM_16))
    {
        return(NX_CRYPTO_INVALID_ALGORITHM);
    }

    /* IV : Nonce length(1 byte) + Nonce
       nx_crypto_ICV_size_in_bits: authentication tag length in bits */
    switch (op)
    {
        case NX_CRYPTO_DECRYPT:
        {
            if (iv_ptr == NX_CRYPTO_NULL ||
                (ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data_len > 0 &&
                 ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data == NX_CRYPTO_NULL))
            {
                status = NX_CRYPTO_PTR_ERROR;
                break;
            }

            icv_len = (method -> nx_crypto_ICV_size_in_bits >> 3);

            if (input_length_in_byte < icv_len || output_length_in_byte < input_length_in_byte - icv_len)
            {
                status = NX_CRYPTO_INVALID_BUFFER_SIZE;
                break;
            }

            message_len = input_length_in_byte - icv_len;
            status = _nx_crypto_gcm_decrypt_init(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                 ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data,
                                                 ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data_len,
                                                 iv_ptr, NX_CRYPTO_AES_BLOCK_SIZE);

            if (status)
            {
                break;
            }

            status = _nx_crypto_gcm_decrypt_update(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                   (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                   input, output, message_len,
                                                   NX_CRYPTO_AES_BLOCK_SIZE);

            if (status)
            {
                break;
            }

            status = _nx_crypto_gcm_decrypt_calculate(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                      (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                      input + message_len, icv_len,
                                                      NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT:
        {
            if (iv_ptr == NX_CRYPTO_NULL ||
                (ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data_len > 0 &&
                 ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data == NX_CRYPTO_NULL))
            {
                status = NX_CRYPTO_PTR_ERROR;
                break;
            }

            icv_len = (method -> nx_crypto_ICV_size_in_bits >> 3);

            if (output_length_in_byte < input_length_in_byte + icv_len)
            {
                status = NX_CRYPTO_INVALID_BUFFER_SIZE;
                break;
            }

            status = _nx_crypto_gcm_encrypt_init(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                 ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data,
                                                 ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data_len,
                                                 iv_ptr, NX_CRYPTO_AES_BLOCK_SIZE);

            if (status)
            {
                break;
            }

            status = _nx_crypto_gcm_encrypt_update(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                   (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                   input, output, input_length_in_byte,
                                                   NX_CRYPTO_AES_BLOCK_SIZE);

            if (status)
            {
                break;
            }

            status = _nx_crypto_gcm_encrypt_calculate(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                      (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                      output + input_length_in_byte, icv_len,
                                                      NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_DECRYPT_INITIALIZE:
        {
            if (iv_ptr == NX_CRYPTO_NULL)
            {
                status = NX_CRYPTO_PTR_ERROR;
                break;
            }

            status = _nx_crypto_gcm_decrypt_init(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                 input, /* pointers to AAD */
                                                 input_length_in_byte, /* length of AAD */
                                                 iv_ptr, NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_DECRYPT_UPDATE:
        {
            status = _nx_crypto_gcm_decrypt_update(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                   (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                   input, output, input_length_in_byte,
                                                   NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_DECRYPT_CALCULATE:
        {
            icv_len = (method -> nx_crypto_ICV_size_in_bits >> 3);
            if (input_length_in_byte < icv_len)
            {
                status = NX_CRYPTO_INVALID_BUFFER_SIZE;
                break;
            }

            status = _nx_crypto_gcm_decrypt_calculate(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                      (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                      input, icv_len,
                                                      NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_INITIALIZE:
        {
            if (iv_ptr == NX_CRYPTO_NULL)
            {
                status = NX_CRYPTO_PTR_ERROR;
                break;
            }

            status = _nx_crypto_gcm_encrypt_init(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                 input, /* pointers to AAD */
                                                 input_length_in_byte, /* length of AAD */
                                                 iv_ptr, NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_UPDATE:
        {
            status = _nx_crypto_gcm_encrypt_update(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                   (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                   input, output, input_length_in_byte,
                                                   NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_CALCULATE:
        {
            icv_len = (method -> nx_crypto_ICV_size_in_bits >> 3);
            if (output_length_in_byte < icv_len)
            {
                status = NX_CRYPTO_INVALID_BUFFER_SIZE;
                break;
            }

            status = _nx_crypto_gcm_encrypt_calculate(ctx, &(ctx -> nx_crypto_aes_mode_context.gcm),
                                                      (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                                      output, icv_len,
                                                      NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_SET_ADDITIONAL_DATA:
        {

            /* Set additonal data pointer.  */
            ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data = (VOID *)input;

            /* Set additional data length.  */
            ctx -> nx_crypto_aes_mode_context.gcm.nx_crypto_gcm_additional_data_len = input_length_in_byte;

            status = NX_CRYPTO_SUCCESS;
        } break;

        default:
        {
            status = NX_CRYPTO_INVALID_ALGORITHM;
        } break;
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_ctr_operation                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES CTR algorithm.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
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
/*    _nx_crypto_ctr_encrypt                Perform CTR mode encryption   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_operation       Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_aes_ctr_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

NX_CRYPTO_AES      *ctx;
UINT                status;

    NX_CRYPTO_PARAMETER_NOT_USED(op);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);
    
    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_AES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_AES *)crypto_metadata;

    switch (op)
    {
        case NX_CRYPTO_ENCRYPT:
        /* fallthrough */
        case NX_CRYPTO_DECRYPT:
        {
            status = _nx_crypto_ctr_encrypt_init(&(ctx -> nx_crypto_aes_mode_context.ctr),
                                                 iv_ptr, 8, &key[key_size_in_bits >> 3], 4);
            if (status)
            {
                break;
            }

            status = _nx_crypto_ctr_encrypt(ctx, &(ctx -> nx_crypto_aes_mode_context.ctr),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                            input, output, input_length_in_byte,
                                            NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_INITIALIZE:
        /* fallthrough */
        case NX_CRYPTO_DECRYPT_INITIALIZE:
        {
            status = _nx_crypto_ctr_encrypt_init(&(ctx -> nx_crypto_aes_mode_context.ctr),
                                                 iv_ptr, 8, input, /* input pointers to nonce */
                                                 input_length_in_byte);
        } break;

        case NX_CRYPTO_ENCRYPT_UPDATE:
        /* fallthrough */
        case NX_CRYPTO_DECRYPT_UPDATE:
        {
            status = _nx_crypto_ctr_encrypt(ctx, &(ctx -> nx_crypto_aes_mode_context.ctr),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                            input, output, input_length_in_byte,
                                            NX_CRYPTO_AES_BLOCK_SIZE);
        } break;

        case NX_CRYPTO_ENCRYPT_CALCULATE:
        /* fallthrough */
        case NX_CRYPTO_DECRYPT_CALCULATE:
        {

            /* Nothing to do. */
            status = NX_CRYPTO_SUCCESS;
        } break;

        default:
        {
            status = NX_CRYPTO_INVALID_ALGORITHM;
        } break;
    }

    return status;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_xcbc_operation                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES XCBC algorithm.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
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
/*    _nx_crypto_xcbc_mac                   Perform XCBC mode             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_operation       Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_aes_xcbc_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

NX_CRYPTO_AES  *ctx;
UINT            status;

    NX_CRYPTO_PARAMETER_NOT_USED(op);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (key == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_AES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_AES *)crypto_metadata;

    status = _nx_crypto_xcbc_mac(ctx,
                                 (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_aes_encrypt,
                                 (UINT (*)(VOID *, UCHAR *, UINT))_nx_crypto_aes_key_set,
                                 key, key_size_in_bits,
                                 input, output, input_length_in_byte,
                                 NX_CRYPTO_NULL, NX_CRYPTO_AUTHENTICATION_ICV_TRUNC_BITS >> 3,
                                 NX_CRYPTO_AES_BLOCK_SIZE);

    return status;
}
