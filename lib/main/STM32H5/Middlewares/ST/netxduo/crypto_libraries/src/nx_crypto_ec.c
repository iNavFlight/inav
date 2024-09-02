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
/**   Elliptic Curve                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_ec.h"

/* secp192r1 */
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp192r1_p[] =
{

    /* p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF FFFFFFFF */
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFE), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp192r1_a[] =
{

    /* a = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF FFFFFFFC */
    HN_ULONG_TO_UBASE(0xFFFFFFFC), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFE), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp192r1_b[] =
{

    /* b = 64210519 E59C80E7 0FA7E9AB 72243049 FEB8DEEC C146B9B1 */
    HN_ULONG_TO_UBASE(0xC146B9B1), HN_ULONG_TO_UBASE(0xFEB8DEEC),
    HN_ULONG_TO_UBASE(0x72243049), HN_ULONG_TO_UBASE(0x0FA7E9AB),
    HN_ULONG_TO_UBASE(0xE59C80E7), HN_ULONG_TO_UBASE(0x64210519)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp192r1_gx[] =
{

    /* G.x = 188DA80E B03090F6 7CBF20EB 43A18800 F4FF0AFD 82FF1012 */
    HN_ULONG_TO_UBASE(0x82FF1012), HN_ULONG_TO_UBASE(0xF4FF0AFD),
    HN_ULONG_TO_UBASE(0x43A18800), HN_ULONG_TO_UBASE(0x7CBF20EB),
    HN_ULONG_TO_UBASE(0xB03090F6), HN_ULONG_TO_UBASE(0x188DA80E)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp192r1_gy[] =
{

    /* G.y = 07192B95 FFC8DA78 631011ED 6B24CDD5 73F977A1 1E794811*/
    HN_ULONG_TO_UBASE(0x1E794811), HN_ULONG_TO_UBASE(0x73F977A1),
    HN_ULONG_TO_UBASE(0x6B24CDD5), HN_ULONG_TO_UBASE(0x631011ED),
    HN_ULONG_TO_UBASE(0xFFC8DA78), HN_ULONG_TO_UBASE(0x07192B95)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp192r1_n[] =
{

    /* n = FFFFFFFF FFFFFFFF FFFFFFFF 99DEF836 146BC9B1 B4D22831 */
    HN_ULONG_TO_UBASE(0xB4D22831), HN_ULONG_TO_UBASE(0x146BC9B1),
    HN_ULONG_TO_UBASE(0x99DEF836), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp192r1_h[] =
{

    /* h = 01 */
    HN_ULONG_TO_UBASE(0x00000001)
};

/* secp224r1 */
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp224r1_p[] =
{

    /* p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF 00000000 00000000 00000001 */
    HN_ULONG_TO_UBASE(0x00000001), HN_ULONG_TO_UBASE(0x00000000),
    HN_ULONG_TO_UBASE(0x00000000), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp224r1_a[] =
{

    /* a = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF FFFFFFFF FFFFFFFE */
    HN_ULONG_TO_UBASE(0xFFFFFFFE), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFE),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp224r1_b[] =
{

    /* b = B4050A85 0C04B3AB F5413256 5044B0B7 D7BFD8BA 270B3943 2355FFB4 */
    HN_ULONG_TO_UBASE(0x2355FFB4), HN_ULONG_TO_UBASE(0x270B3943),
    HN_ULONG_TO_UBASE(0xD7BFD8BA), HN_ULONG_TO_UBASE(0x5044B0B7),
    HN_ULONG_TO_UBASE(0xF5413256), HN_ULONG_TO_UBASE(0x0C04B3AB),
    HN_ULONG_TO_UBASE(0xB4050A85)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp224r1_gx[] =
{

    /* G.x = B70E0CBD 6BB4BF7F 321390B9 4A03C1D3 56C21122 343280D6 115C1D21 */
    HN_ULONG_TO_UBASE(0x115C1D21), HN_ULONG_TO_UBASE(0x343280D6),
    HN_ULONG_TO_UBASE(0x56C21122), HN_ULONG_TO_UBASE(0x4A03C1D3),
    HN_ULONG_TO_UBASE(0x321390B9), HN_ULONG_TO_UBASE(0x6BB4BF7F),
    HN_ULONG_TO_UBASE(0xB70E0CBD)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp224r1_gy[] =
{

    /* G.y = BD376388 B5F723FB 4C22DFE6 CD4375A0 5A074764 44D58199 85007E34 */
    HN_ULONG_TO_UBASE(0x85007E34), HN_ULONG_TO_UBASE(0x44D58199),
    HN_ULONG_TO_UBASE(0x5A074764), HN_ULONG_TO_UBASE(0xCD4375A0),
    HN_ULONG_TO_UBASE(0x4C22DFE6), HN_ULONG_TO_UBASE(0xB5F723FB),
    HN_ULONG_TO_UBASE(0xBD376388)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp224r1_n[] =
{

    /* n = FFFFFFFF FFFFFFFF FFFFFFFF FFFF16A2 E0B8F03E 13DD2945 5C5C2A3D */
    HN_ULONG_TO_UBASE(0x5C5C2A3D), HN_ULONG_TO_UBASE(0x13DD2945),
    HN_ULONG_TO_UBASE(0xE0B8F03E), HN_ULONG_TO_UBASE(0xFFFF16A2),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp224r1_h[] =
{

    /* h = 01 */
    HN_ULONG_TO_UBASE(0x00000001)
};

/* secp256r1 */
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp256r1_p[] =
{

    /* p = FFFFFFFF 00000001 00000000 00000000 00000000 FFFFFFFF FFFFFFFF FFFFFFFF */
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0x00000000),
    HN_ULONG_TO_UBASE(0x00000000), HN_ULONG_TO_UBASE(0x00000000),
    HN_ULONG_TO_UBASE(0x00000001), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp256r1_a[] =
{

    /* a = FFFFFFFF 00000001 00000000 00000000 00000000 FFFFFFFF FFFFFFFF FFFFFFFC */
    HN_ULONG_TO_UBASE(0xFFFFFFFC), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0x00000000),
    HN_ULONG_TO_UBASE(0x00000000), HN_ULONG_TO_UBASE(0x00000000),
    HN_ULONG_TO_UBASE(0x00000001), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp256r1_b[] =
{

    /* b = 5AC635D8 AA3A93E7 B3EBBD55 769886BC 651D06B0 CC53B0F6 3BCE3C3E 27D2604B */
    HN_ULONG_TO_UBASE(0x27D2604B), HN_ULONG_TO_UBASE(0x3BCE3C3E),
    HN_ULONG_TO_UBASE(0xCC53B0F6), HN_ULONG_TO_UBASE(0x651D06B0),
    HN_ULONG_TO_UBASE(0x769886BC), HN_ULONG_TO_UBASE(0xB3EBBD55),
    HN_ULONG_TO_UBASE(0xAA3A93E7), HN_ULONG_TO_UBASE(0x5AC635D8)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp256r1_gx[] =
{

    /* G.x = 6B17D1F2 E12C4247 F8BCE6E5 63A440F2 77037D81 2DEB33A0 F4A13945 D898C296 */
    HN_ULONG_TO_UBASE(0xD898C296), HN_ULONG_TO_UBASE(0xF4A13945),
    HN_ULONG_TO_UBASE(0x2DEB33A0), HN_ULONG_TO_UBASE(0x77037D81),
    HN_ULONG_TO_UBASE(0x63A440F2), HN_ULONG_TO_UBASE(0xF8BCE6E5),
    HN_ULONG_TO_UBASE(0xE12C4247), HN_ULONG_TO_UBASE(0x6B17D1F2)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp256r1_gy[] =
{

    /* G.y = 4FE342E2 FE1A7F9B 8EE7EB4A 7C0F9E16 2BCE3357 6B315ECE CBB64068 37BF51F5 */
    HN_ULONG_TO_UBASE(0x37BF51F5), HN_ULONG_TO_UBASE(0xCBB64068),
    HN_ULONG_TO_UBASE(0x6B315ECE), HN_ULONG_TO_UBASE(0x2BCE3357),
    HN_ULONG_TO_UBASE(0x7C0F9E16), HN_ULONG_TO_UBASE(0x8EE7EB4A),
    HN_ULONG_TO_UBASE(0xFE1A7F9B), HN_ULONG_TO_UBASE(0x4FE342E2)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp256r1_n[] =
{

    /* n = FFFFFFFF 00000000 FFFFFFFF FFFFFFFF BCE6FAAD A7179E84 F3B9CAC2 FC632551 */
    HN_ULONG_TO_UBASE(0xFC632551), HN_ULONG_TO_UBASE(0xF3B9CAC2),
    HN_ULONG_TO_UBASE(0xA7179E84), HN_ULONG_TO_UBASE(0xBCE6FAAD),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0x00000000), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp256r1_h[] =
{

    /* h = 01 */
    HN_ULONG_TO_UBASE(0x00000001)
};

/* secp384r1 */
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp384r1_p[] =
{

    /* p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF 00000000 00000000 FFFFFFFF */
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0x00000000),
    HN_ULONG_TO_UBASE(0x00000000), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFE), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp384r1_a[] =
{

    /* a = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF 00000000 00000000 FFFFFFFC */
    HN_ULONG_TO_UBASE(0xFFFFFFFC), HN_ULONG_TO_UBASE(0x00000000),
    HN_ULONG_TO_UBASE(0x00000000), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFE), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp384r1_b[] =
{

    /* b = B3312FA7 E23EE7E4 988E056B E3F82D19 181D9C6E FE814112 0314088F 5013875A C656398D 8A2ED19D 2A85C8ED D3EC2AEF */
    HN_ULONG_TO_UBASE(0xD3EC2AEF), HN_ULONG_TO_UBASE(0x2A85C8ED),
    HN_ULONG_TO_UBASE(0x8A2ED19D), HN_ULONG_TO_UBASE(0xC656398D),
    HN_ULONG_TO_UBASE(0x5013875A), HN_ULONG_TO_UBASE(0x0314088F),
    HN_ULONG_TO_UBASE(0xFE814112), HN_ULONG_TO_UBASE(0x181D9C6E),
    HN_ULONG_TO_UBASE(0xE3F82D19), HN_ULONG_TO_UBASE(0x988E056B),
    HN_ULONG_TO_UBASE(0xE23EE7E4), HN_ULONG_TO_UBASE(0xB3312FA7)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp384r1_gx[] =
{

    /* G.x = AA87CA22 BE8B0537 8EB1C71E F320AD74 6E1D3B62 8BA79B98
             59F741E0 82542A38 5502F25D BF55296C 3A545E38 72760AB7 */
    HN_ULONG_TO_UBASE(0x72760AB7), HN_ULONG_TO_UBASE(0x3A545E38),
    HN_ULONG_TO_UBASE(0xBF55296C), HN_ULONG_TO_UBASE(0x5502F25D),
    HN_ULONG_TO_UBASE(0x82542A38), HN_ULONG_TO_UBASE(0x59F741E0),
    HN_ULONG_TO_UBASE(0x8BA79B98), HN_ULONG_TO_UBASE(0x6E1D3B62),
    HN_ULONG_TO_UBASE(0xF320AD74), HN_ULONG_TO_UBASE(0x8EB1C71E),
    HN_ULONG_TO_UBASE(0xBE8B0537), HN_ULONG_TO_UBASE(0xAA87CA22)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp384r1_gy[] =
{

    /* G.y = 3617DE4A 96262C6F 5D9E98BF 9292DC29 F8F41DBD 289A147C
             E9DA3113 B5F0B8C0 0A60B1CE 1D7E819D 7A431D7C 90EA0E5F */
    HN_ULONG_TO_UBASE(0x90EA0E5F), HN_ULONG_TO_UBASE(0x7A431D7C),
    HN_ULONG_TO_UBASE(0x1D7E819D), HN_ULONG_TO_UBASE(0x0A60B1CE),
    HN_ULONG_TO_UBASE(0xB5F0B8C0), HN_ULONG_TO_UBASE(0xE9DA3113),
    HN_ULONG_TO_UBASE(0x289A147C), HN_ULONG_TO_UBASE(0xF8F41DBD),
    HN_ULONG_TO_UBASE(0x9292DC29), HN_ULONG_TO_UBASE(0x5D9E98BF),
    HN_ULONG_TO_UBASE(0x96262C6F), HN_ULONG_TO_UBASE(0x3617DE4A)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp384r1_n[] =
{

    /* n = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF C7634D81 F4372DDF 581A0DB2 48B0A77A ECEC196A CCC52973 */
    HN_ULONG_TO_UBASE(0xCCC52973), HN_ULONG_TO_UBASE(0xECEC196A),
    HN_ULONG_TO_UBASE(0x48B0A77A), HN_ULONG_TO_UBASE(0x581A0DB2),
    HN_ULONG_TO_UBASE(0xF4372DDF), HN_ULONG_TO_UBASE(0xC7634D81),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp384r1_h[] =
{

    /* h = 01 */
    HN_ULONG_TO_UBASE(0x00000001)
};

/* secp521r1 */
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp521r1_p[] =
{

    /* p = 01FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF */
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0x000001FF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp521r1_a[] =
{

    /* a = 01FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFC */
    HN_ULONG_TO_UBASE(0xFFFFFFFC), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0x000001FF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp521r1_b[] =
{

    /* b = 0051 953EB961 8E1C9A1F 929A21A0 B68540EE A2DA725B 99B315F3 B8B48991 8EF109E1 56193951 EC7E937B 1652C0BD 3BB1BF07 3573DF88 3D2C34F1 EF451FD4 6B503F00 */
    HN_ULONG_TO_UBASE(0x6B503F00), HN_ULONG_TO_UBASE(0xEF451FD4),
    HN_ULONG_TO_UBASE(0x3D2C34F1), HN_ULONG_TO_UBASE(0x3573DF88),
    HN_ULONG_TO_UBASE(0x3BB1BF07), HN_ULONG_TO_UBASE(0x1652C0BD),
    HN_ULONG_TO_UBASE(0xEC7E937B), HN_ULONG_TO_UBASE(0x56193951),
    HN_ULONG_TO_UBASE(0x8EF109E1), HN_ULONG_TO_UBASE(0xB8B48991),
    HN_ULONG_TO_UBASE(0x99B315F3), HN_ULONG_TO_UBASE(0xA2DA725B),
    HN_ULONG_TO_UBASE(0xB68540EE), HN_ULONG_TO_UBASE(0x929A21A0),
    HN_ULONG_TO_UBASE(0x8E1C9A1F), HN_ULONG_TO_UBASE(0x953EB961),
    HN_ULONG_TO_UBASE(0x00000051)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp521r1_gx[] =
{

    /* G.x = 00C6858E 06B70404 E9CD9E3E CB662395 B4429C64 8139053F B521F828 AF606B4D 3DBAA14B 5E77EFE7 5928FE1D C127A2FF A8DE3348 B3C1856A 429BF97E 7E31C2E5 BD66 */
    HN_ULONG_TO_UBASE(0xC2E5BD66), HN_ULONG_TO_UBASE(0xF97E7E31),
    HN_ULONG_TO_UBASE(0x856A429B), HN_ULONG_TO_UBASE(0x3348B3C1),
    HN_ULONG_TO_UBASE(0xA2FFA8DE), HN_ULONG_TO_UBASE(0xFE1DC127),
    HN_ULONG_TO_UBASE(0xEFE75928), HN_ULONG_TO_UBASE(0xA14B5E77),
    HN_ULONG_TO_UBASE(0x6B4D3DBA), HN_ULONG_TO_UBASE(0xF828AF60),
    HN_ULONG_TO_UBASE(0x053FB521), HN_ULONG_TO_UBASE(0x9C648139),
    HN_ULONG_TO_UBASE(0x2395B442), HN_ULONG_TO_UBASE(0x9E3ECB66),
    HN_ULONG_TO_UBASE(0x0404E9CD), HN_ULONG_TO_UBASE(0x858E06B7),
    HN_ULONG_TO_UBASE(0x000000C6)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp521r1_gy[] =
{

    /* G.y = 0118 39296A78 9A3BC004 5C8A5FB4 2C7D1BD9 98F54449 579B4468 17AFBD17 273E662C 97EE7299 5EF42640 C550B901 3FAD0761 353C7086 A272C240 88BE9476 9FD16650*/
    HN_ULONG_TO_UBASE(0x9FD16650), HN_ULONG_TO_UBASE(0x88BE9476),
    HN_ULONG_TO_UBASE(0xA272C240), HN_ULONG_TO_UBASE(0x353C7086),
    HN_ULONG_TO_UBASE(0x3FAD0761), HN_ULONG_TO_UBASE(0xC550B901),
    HN_ULONG_TO_UBASE(0x5EF42640), HN_ULONG_TO_UBASE(0x97EE7299),
    HN_ULONG_TO_UBASE(0x273E662C), HN_ULONG_TO_UBASE(0x17AFBD17),
    HN_ULONG_TO_UBASE(0x579B4468), HN_ULONG_TO_UBASE(0x98F54449),
    HN_ULONG_TO_UBASE(0x2C7D1BD9), HN_ULONG_TO_UBASE(0x5C8A5FB4),
    HN_ULONG_TO_UBASE(0x9A3BC004), HN_ULONG_TO_UBASE(0x39296A78),
    HN_ULONG_TO_UBASE(0x00000118)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp521r1_n[] =
{

    /* n = 01FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFA 51868783 BF2F966B 7FCC0148 F709A5D0 3BB5C9B8 899C47AE BB6FB71E 91386409 */
    HN_ULONG_TO_UBASE(0x91386409), HN_ULONG_TO_UBASE(0xBB6FB71E),
    HN_ULONG_TO_UBASE(0x899C47AE), HN_ULONG_TO_UBASE(0x3BB5C9B8),
    HN_ULONG_TO_UBASE(0xF709A5D0), HN_ULONG_TO_UBASE(0x7FCC0148),
    HN_ULONG_TO_UBASE(0xBF2F966B), HN_ULONG_TO_UBASE(0x51868783),
    HN_ULONG_TO_UBASE(0xFFFFFFFA), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0x000001FF)
};
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_secp521r1_h[] =
{

    /* h = 01 */
    HN_ULONG_TO_UBASE(0x00000001)
};

extern NX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_secp192r1_fixed_points;
extern NX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_secp224r1_fixed_points;
extern NX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_secp256r1_fixed_points;
extern NX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_secp384r1_fixed_points;
extern NX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_secp521r1_fixed_points;

NX_CRYPTO_CONST NX_CRYPTO_EC                     _nx_crypto_ec_secp192r1 =
{
    "secp192r1",
    NX_CRYPTO_EC_SECP192R1,
    4,
    192,
    {
        .fp =
        {
            (HN_UBASE *)_nx_crypto_ec_secp192r1_p,
            sizeof(_nx_crypto_ec_secp192r1_p) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp192r1_p),
            (UINT)NX_CRYPTO_FALSE
        }
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp192r1_a,
        sizeof(_nx_crypto_ec_secp192r1_a) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp192r1_a),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp192r1_b,
        sizeof(_nx_crypto_ec_secp192r1_b) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp192r1_b),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)_nx_crypto_ec_secp192r1_gx,
            sizeof(_nx_crypto_ec_secp192r1_gx) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp192r1_gx),
            (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)_nx_crypto_ec_secp192r1_gy,
            sizeof(_nx_crypto_ec_secp192r1_gy) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp192r1_gy),
            (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp192r1_n,
        sizeof(_nx_crypto_ec_secp192r1_n) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp192r1_n),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp192r1_h,
        sizeof(_nx_crypto_ec_secp192r1_h) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp192r1_h),
        (UINT)NX_CRYPTO_FALSE
    },
    (NX_CRYPTO_EC_FIXED_POINTS *)&_nx_crypto_ec_secp192r1_fixed_points,
    _nx_crypto_ec_fp_affine_add,
    _nx_crypto_ec_fp_affine_subtract,
    _nx_crypto_ec_fp_projective_multiple,
    _nx_crypto_ec_secp192r1_reduce
};

NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp224r1 =
{
    "secp224r1",
    NX_CRYPTO_EC_SECP224R1,
    4,
    224,
    {
        .fp =
        {
            (HN_UBASE *)_nx_crypto_ec_secp224r1_p,
            sizeof(_nx_crypto_ec_secp224r1_p) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp224r1_p),
            (UINT)NX_CRYPTO_FALSE
        }
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp224r1_a,
        sizeof(_nx_crypto_ec_secp224r1_a) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp224r1_a),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp224r1_b,
        sizeof(_nx_crypto_ec_secp224r1_b) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp224r1_b),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)_nx_crypto_ec_secp224r1_gx,
            sizeof(_nx_crypto_ec_secp224r1_gx) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp224r1_gx),
            (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)_nx_crypto_ec_secp224r1_gy,
            sizeof(_nx_crypto_ec_secp224r1_gy) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp224r1_gy),
            (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp224r1_n,
        sizeof(_nx_crypto_ec_secp224r1_n) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp224r1_n),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp224r1_h,
        sizeof(_nx_crypto_ec_secp224r1_h) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp224r1_h),
        (UINT)NX_CRYPTO_FALSE
    },
    (NX_CRYPTO_EC_FIXED_POINTS *)&_nx_crypto_ec_secp224r1_fixed_points,
    _nx_crypto_ec_fp_affine_add,
    _nx_crypto_ec_fp_affine_subtract,
    _nx_crypto_ec_fp_projective_multiple,
    _nx_crypto_ec_secp224r1_reduce
};

NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp256r1 =
{
    "secp256r1",
    NX_CRYPTO_EC_SECP256R1,
    4,
    256,
    {
        .fp =
        {
            (HN_UBASE *)_nx_crypto_ec_secp256r1_p,
            sizeof(_nx_crypto_ec_secp256r1_p) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp256r1_p),
            (UINT)NX_CRYPTO_FALSE
        }
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp256r1_a,
        sizeof(_nx_crypto_ec_secp256r1_a) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp256r1_a),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp256r1_b,
        sizeof(_nx_crypto_ec_secp256r1_b) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp256r1_b),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)_nx_crypto_ec_secp256r1_gx,
            sizeof(_nx_crypto_ec_secp256r1_gx) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp256r1_gx),
            (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)_nx_crypto_ec_secp256r1_gy,
            sizeof(_nx_crypto_ec_secp256r1_gy) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp256r1_gy),
            (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp256r1_n,
        sizeof(_nx_crypto_ec_secp256r1_n) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp256r1_n),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp256r1_h,
        sizeof(_nx_crypto_ec_secp256r1_h) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp256r1_h),
        (UINT)NX_CRYPTO_FALSE
    },
    (NX_CRYPTO_EC_FIXED_POINTS *)&_nx_crypto_ec_secp256r1_fixed_points,
    _nx_crypto_ec_fp_affine_add,
    _nx_crypto_ec_fp_affine_subtract,
    _nx_crypto_ec_fp_projective_multiple,
    _nx_crypto_ec_secp256r1_reduce
};

NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp384r1 =
{
    "secp384r1",
    NX_CRYPTO_EC_SECP384R1,
    5,
    384,
    {
        .fp =
        {
            (HN_UBASE *)_nx_crypto_ec_secp384r1_p,
            sizeof(_nx_crypto_ec_secp384r1_p) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp384r1_p),
            (UINT)NX_CRYPTO_FALSE
        }
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp384r1_a,
        sizeof(_nx_crypto_ec_secp384r1_a) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp384r1_a),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp384r1_b,
        sizeof(_nx_crypto_ec_secp384r1_b) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp384r1_b),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)_nx_crypto_ec_secp384r1_gx,
            sizeof(_nx_crypto_ec_secp384r1_gx) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp384r1_gx),
            (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)_nx_crypto_ec_secp384r1_gy,
            sizeof(_nx_crypto_ec_secp384r1_gy) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp384r1_gy),
            (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp384r1_n,
        sizeof(_nx_crypto_ec_secp384r1_n) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp384r1_n),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp384r1_h,
        sizeof(_nx_crypto_ec_secp384r1_h) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp384r1_h),
        (UINT)NX_CRYPTO_FALSE
    },
    (NX_CRYPTO_EC_FIXED_POINTS *)&_nx_crypto_ec_secp384r1_fixed_points,
    _nx_crypto_ec_fp_affine_add,
    _nx_crypto_ec_fp_affine_subtract,
    _nx_crypto_ec_fp_projective_multiple,
    _nx_crypto_ec_secp384r1_reduce
};

NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp521r1 =
{
    "secp521r1",
    NX_CRYPTO_EC_SECP521R1,
    5,
    521,
    {
        .fp =
        {
            (HN_UBASE *)_nx_crypto_ec_secp521r1_p,
            sizeof(_nx_crypto_ec_secp521r1_p) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp521r1_p),
            (UINT)NX_CRYPTO_FALSE
        }
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp521r1_a,
        sizeof(_nx_crypto_ec_secp521r1_a) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp521r1_a),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp521r1_b,
        sizeof(_nx_crypto_ec_secp521r1_b) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp521r1_b),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)_nx_crypto_ec_secp521r1_gx,
            sizeof(_nx_crypto_ec_secp521r1_gx) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp521r1_gx),
            (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)_nx_crypto_ec_secp521r1_gy,
            sizeof(_nx_crypto_ec_secp521r1_gy) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_secp521r1_gy),
            (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp521r1_n,
        sizeof(_nx_crypto_ec_secp521r1_n) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp521r1_n),
        (UINT)NX_CRYPTO_FALSE
    },
    {
        (HN_UBASE *)_nx_crypto_ec_secp521r1_h,
        sizeof(_nx_crypto_ec_secp521r1_h) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_secp521r1_h),
        (UINT)NX_CRYPTO_FALSE
    },
    (NX_CRYPTO_EC_FIXED_POINTS *)&_nx_crypto_ec_secp521r1_fixed_points,
    _nx_crypto_ec_fp_affine_add,
    _nx_crypto_ec_fp_affine_subtract,
    _nx_crypto_ec_fp_projective_multiple,
    _nx_crypto_ec_secp521r1_reduce
};
#ifndef NX_CRYPTO_SELF_TEST
static NX_CRYPTO_CONST NX_CRYPTO_EC *_nx_crypto_ec_named_curves[] =
{
    &_nx_crypto_ec_secp192r1,
    &_nx_crypto_ec_secp224r1,
    &_nx_crypto_ec_secp256r1,
    &_nx_crypto_ec_secp384r1,
    &_nx_crypto_ec_secp521r1
};
#endif
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_point_is_infinite                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether the point is infinite.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    point                                 Pointer to point              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_CRYPTO_TRUE                        Point is infinite             */
/*    NX_CRYPTO_FALSE                       Point is not infinite         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_is_zero        Check if number is zero or not*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_point_fp_projective_to_affine                         */
/*                                          Convert point from projective */
/*                                            to affine                   */
/*    _nx_crypto_ec_fp_projective_add       Perform addition for points of*/
/*                                            projective and affine       */
/*    _nx_crypto_ec_fp_projective_double    Perform doubling for points of*/
/*                                            projective                  */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ec_point_is_infinite(NX_CRYPTO_EC_POINT *point)
{
    if (point -> nx_crypto_ec_point_type == NX_CRYPTO_EC_POINT_AFFINE)
    {
        if (_nx_crypto_huge_number_is_zero(&point -> nx_crypto_ec_point_x) &&
            _nx_crypto_huge_number_is_zero(&point -> nx_crypto_ec_point_y))
        {
            return(NX_CRYPTO_TRUE);
        }
        else
        {
            return(NX_CRYPTO_FALSE);
        }
    }

    return(_nx_crypto_huge_number_is_zero(&point -> nx_crypto_ec_point_z));
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_point_set_infinite                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the point to infinite.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    point                                 Pointer to point              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_fixed_multiple       Calculate the fixed           */
/*                                            multiplication              */
/*    _nx_crypto_ec_fp_projective_multiple  Calculate the projective      */
/*                                            multiplication              */
/*    _nx_crypto_ec_point_fp_projective_to_affine                         */
/*                                          Convert point from projective */
/*                                            to affine                   */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_point_set_infinite(NX_CRYPTO_EC_POINT *point)
{
    if (point -> nx_crypto_ec_point_type == NX_CRYPTO_EC_POINT_AFFINE)
    {
        NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&point -> nx_crypto_ec_point_x, 0);
        NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&point -> nx_crypto_ec_point_y, 0);
    }
    else
    {
        NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&point -> nx_crypto_ec_point_z, 0);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_point_setup                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up point from byte steam.                        */
/*                                                                        */
/*    Note: only uncompressed format is supported now.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    point                                 Pointer to point              */
/*    byte_stream                           Byte stream                   */
/*    byte_stream_size                      Size of byte stream           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), updated  */
/*                                            constants, resulting        */
/*                                            in version 6.1              */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ec_point_setup(NX_CRYPTO_EC_POINT *point, UCHAR *byte_stream, UINT byte_stream_size)
{
UINT len;
UINT status;

    if (*byte_stream != 0x04)
    {

        /* Only uncompressed format is supported. */
        return(NX_CRYPTO_FORMAT_NOT_SUPPORTED);
    }

    byte_stream++;
    byte_stream_size--;
    len = (byte_stream_size >> 1);
    status = _nx_crypto_huge_number_setup(&point -> nx_crypto_ec_point_x, byte_stream, len);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    byte_stream += len;
    status = _nx_crypto_huge_number_setup(&point -> nx_crypto_ec_point_y, byte_stream, len);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_point_extract_uncompressed            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts point to byte stream in uncompressed format. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    point                                 Pointer to point              */
/*    byte_stream                           Byte stream for output        */
/*    byte_stream_size                      Size of byte stream buffer    */
/*    huge_number_size                      Actual size of byte stream    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecjpake_hello_generate     Generate hello message        */
/*    _nx_crypto_ecjpake_key_exchange_generate                            */
/*                                          Generate key exchange message */
/*    _nx_crypto_ecjpake_schnorr_zkp_hash   Perform Schnorr ZKP hash      */
/*                                            calculation                 */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_point_extract_uncompressed(NX_CRYPTO_EC *curve, NX_CRYPTO_EC_POINT *point,
                                                             UCHAR *byte_stream, UINT byte_stream_size,
                                                             UINT *huge_number_size)
{
UINT status;
UINT clen;

    clen = (curve -> nx_crypto_ec_bits + 7) >> 3;

    if (byte_stream_size < 1 + (clen << 1))
    {
        *huge_number_size = 0;
        return;
    }

    *byte_stream++ = 0x04;
    *huge_number_size = 1;
    byte_stream_size--;

    status = _nx_crypto_huge_number_extract_fixed_size(&point -> nx_crypto_ec_point_x,
                                                       byte_stream, clen);
    if (status)
    {
        *huge_number_size = 0;
        return;
    }

    byte_stream += clen;
    *huge_number_size = *huge_number_size + clen;
    byte_stream_size -= clen;

    status = _nx_crypto_huge_number_extract_fixed_size(&point -> nx_crypto_ec_point_y,
                                                       byte_stream, clen);
    if (status)
    {
        *huge_number_size = 0;
        return;
    }

    *huge_number_size = *huge_number_size + clen;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_point_fp_affine_to_projective         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts point from affine coordinate to projective   */
/*    coordinate in prime field.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    point                                 Pointer to point              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_projective_add       Perform addition for points of*/
/*                                            projective and affine       */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_point_fp_affine_to_projective(NX_CRYPTO_EC_POINT *point)
{
    point -> nx_crypto_ec_point_type = NX_CRYPTO_EC_POINT_PROJECTIVE;
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&point -> nx_crypto_ec_point_z, 1);
}

/* FP: (X, Y, Z) -> (X/Z^2, Y/Z^3) */
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_point_fp_projective_to_affine         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts point from projective coordinate to affine   */
/*    coordinate in prime field.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    point                                 Pointer to point              */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ec_point_is_infinite       Check if the point is infinite*/
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*    NX_CRYPTO_EC_MULTIPLE_REDUCE          Multiply two huge numbers     */
/*    NX_CRYPTO_EC_SQUARE_REDUCE            Computes the square of a value*/
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_fixed_multiple       Calculate the fixed           */
/*                                            multiplication              */
/*    _nx_crypto_ec_fp_projective_multiple  Calculate the projective      */
/*                                            multiplication              */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_point_fp_projective_to_affine(NX_CRYPTO_EC *curve,
                                                                NX_CRYPTO_EC_POINT *point,
                                                                HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER  temp1, temp2, zi;
NX_CRYPTO_HUGE_NUMBER *p;
UINT                   buffer_size;

    p = &curve -> nx_crypto_ec_field.fp;

    if (_nx_crypto_ec_point_is_infinite(point))
    {
        point -> nx_crypto_ec_point_type = NX_CRYPTO_EC_POINT_AFFINE;
        _nx_crypto_ec_point_set_infinite(point);
        return;
    }

    buffer_size = point -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size;
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp1, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp2, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&zi, scratch, buffer_size);

    /* zi = Z ^ -1 mod p */
    _nx_crypto_huge_number_inverse_modulus_prime(&point -> nx_crypto_ec_point_z,
                                                 p, &zi, scratch);

    /* X = X * Z ^ -2 mod p */
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &zi, &temp1, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &point -> nx_crypto_ec_point_x,
                                 &temp1, &temp2, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&point -> nx_crypto_ec_point_x, &temp2);

    /* Y = Y * Z ^ -3 mod p */
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &zi, &temp1, &temp2, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &point -> nx_crypto_ec_point_y,
                                 &temp2, &temp1, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&point -> nx_crypto_ec_point_y, &temp1);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_secp192r1_reduce                      PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reduces the value of curve secp192r1.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    point                                 Pointer to point              */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_SECP192R1_DATA_SETUP     Setup EC SECP192R1 data       */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
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
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_ec_secp192r1_reduce(NX_CRYPTO_EC *curve,
                                                   NX_CRYPTO_HUGE_NUMBER *value,
                                                   HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp;
UINT                 *data;
HN_UBASE              rev1;
UINT                  size = 0;
UINT                  compare_value;

    compare_value = _nx_crypto_huge_number_compare(value, &curve -> nx_crypto_ec_field.fp);
    if (compare_value == NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        return;
    }
    else if (compare_value == NX_CRYPTO_HUGE_NUMBER_EQUAL)
    {
        NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(value, 0);
        return;
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch, 36);

    data = (UINT *)(((ULONG)scratch + 3) & (ULONG) ~3);

    /* c= (c5,...,c2,c1,c0), ci is a 64-bit word */
    _nx_crypto_huge_number_extract(value, (UCHAR *)data, 48, &size);
    if (size < 48)
    {
        NX_CRYPTO_MEMMOVE((UCHAR *)data + (48 - size), data, size); /* Use case of memmove is verified. */
        NX_CRYPTO_MEMSET(data, 0, (48 - size));
    }

    /* r = s1 + s2 + s3 + s4 mod p */

    /* s1 = (c2,c1,c0) */
    _nx_crypto_huge_number_setup(value, (UCHAR *)data + 24, 24);

    /* s2 = (0,c3,c3) */
    NX_CRYPTO_EC_SECP192R1_DATA_SETUP(&temp, rev1,
                                      0, 0, data[4], data[5],
                                      data[4], data[5]);
    _nx_crypto_huge_number_add(value, &temp);

    /* s3 = (c4,c4,0) */
    NX_CRYPTO_EC_SECP192R1_DATA_SETUP(&temp, rev1,
                                      data[2], data[3], data[2],
                                      data[3], 0, 0);
    _nx_crypto_huge_number_add(value, &temp);

    /* s4 = (c5,c5,c5) */
    NX_CRYPTO_EC_SECP192R1_DATA_SETUP(&temp, rev1,
                                      data[0], data[1], data[0], data[1],
                                      data[0], data[1]);
    _nx_crypto_huge_number_add(value, &temp);

    compare_value = _nx_crypto_huge_number_compare_unsigned(value, &curve -> nx_crypto_ec_field.fp);
    while (compare_value != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        _nx_crypto_huge_number_subtract_unsigned(value, &curve -> nx_crypto_ec_field.fp, value);
        compare_value = _nx_crypto_huge_number_compare_unsigned(value,
                                                                &curve -> nx_crypto_ec_field.fp);
    }
    if (_nx_crypto_huge_number_is_zero(value))
    {
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
    else if (value -> nx_crypto_huge_number_is_negative)
    {
        _nx_crypto_huge_number_subtract_unsigned(&curve -> nx_crypto_ec_field.fp, value, value);
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_secp224r1_reduce                      PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reduces the value of curve secp224r1.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    point                                 Pointer to point              */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_SECP224R1_DATA_SETUP     Setup EC SECP224R1 data       */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_is_zero        Check if number is zero or not*/
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
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
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_ec_secp224r1_reduce(NX_CRYPTO_EC *curve,
                                                   NX_CRYPTO_HUGE_NUMBER *value,
                                                   HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp;
UINT                 *data;
HN_UBASE              rev1;
UINT                  size = 0;
UINT                  compare_value;

    compare_value = _nx_crypto_huge_number_compare(value, &curve -> nx_crypto_ec_field.fp);
    if (compare_value == NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        return;
    }
    else if (compare_value == NX_CRYPTO_HUGE_NUMBER_EQUAL)
    {
        NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(value, 0);
        return;
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch, 36);

    data = (UINT *)(((ULONG)scratch + 3) & (ULONG) ~3);

    /* c= (c13,...,c2,c1,c0), ci is a 32-bit word */
    _nx_crypto_huge_number_extract(value, (UCHAR *)data, 56, &size);
    if (size < 56)
    {
        NX_CRYPTO_MEMMOVE((UCHAR *)data + (56 - size), data, size); /* Use case of memmove is verified. */
        NX_CRYPTO_MEMSET(data, 0, (56 - size));
    }

    /* r = s1 + s2 + s3 - s4 - s5 mod p */

    /* s1 = (c6,c5,c4,c3,c2,c1,c0) */
    _nx_crypto_huge_number_setup(value, (UCHAR *)data + 28, 28);

    /* s2 = (c10,c9,c8,c7,0,0,0) */
    NX_CRYPTO_EC_SECP224R1_DATA_SETUP(&temp, rev1,
                                      data[3], data[4], data[5],
                                      data[6], 0, 0, 0);
    _nx_crypto_huge_number_add(value, &temp);

    /* s3 = (0,c13,c12,c11,0,0,0) */
    NX_CRYPTO_EC_SECP224R1_DATA_SETUP(&temp, rev1,
                                      0, data[0], data[1],
                                      data[2], 0, 0, 0);
    _nx_crypto_huge_number_add(value, &temp);

    /* s4 = (c13,c12,c11,c10,c9,c8,c7) */
    NX_CRYPTO_EC_SECP224R1_DATA_SETUP(&temp, rev1,
                                      data[0], data[1], data[2], data[3],
                                      data[4], data[5], data[6]);
    _nx_crypto_huge_number_subtract(value, &temp);

    /* s5 = (0,0,0,0,c13,c12,c11) */
    NX_CRYPTO_EC_SECP224R1_DATA_SETUP(&temp, rev1,
                                      0, 0, 0, 0, data[0],
                                      data[1], data[2]);
    _nx_crypto_huge_number_subtract(value, &temp);

    compare_value = _nx_crypto_huge_number_compare_unsigned(value, &curve -> nx_crypto_ec_field.fp);
    while (compare_value != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        _nx_crypto_huge_number_subtract_unsigned(value, &curve -> nx_crypto_ec_field.fp, value);
        compare_value = _nx_crypto_huge_number_compare_unsigned(value,
                                                                &curve -> nx_crypto_ec_field.fp);
    }
    if (_nx_crypto_huge_number_is_zero(value))
    {
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
    else if (value -> nx_crypto_huge_number_is_negative)
    {
        _nx_crypto_huge_number_subtract_unsigned(&curve -> nx_crypto_ec_field.fp, value, value);
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_secp256r1_reduce                      PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reduces the value of curve secp256r1.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    point                                 Pointer to point              */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_SECP256R1_DATA_SETUP     Setup EC SECP256R1 data       */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*    _nx_crypto_huge_number_shift_left     Shift left for huge number    */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
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
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_ec_secp256r1_reduce(NX_CRYPTO_EC *curve,
                                                   NX_CRYPTO_HUGE_NUMBER *value,
                                                   HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp;
UINT                 *data;
HN_UBASE              rev1;
HN_UBASE              rev2;
UINT                  size = 0;
UINT                  compare_value;

    compare_value = _nx_crypto_huge_number_compare(value, &curve -> nx_crypto_ec_field.fp);
    if (compare_value == NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        return;
    }
    else if (compare_value == NX_CRYPTO_HUGE_NUMBER_EQUAL)
    {
        NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(value, 0);
        return;
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch, 36);

    data = (UINT *)(((ULONG)scratch + 3) & (ULONG) ~3);

    /* c= (c15,...,c2,c1,c0), ci is a 32-bit word */
    _nx_crypto_huge_number_extract(value, (UCHAR *)data, 64, &size);
    if (size < 64)
    {
        NX_CRYPTO_MEMMOVE((UCHAR *)data + (64 - size), data, size); /* Use case of memmove is verified. */
        NX_CRYPTO_MEMSET(data, 0, (64 - size));
    }

    /* r = s1 + 2 * s2 + 2 * s3 + s4 + s5 - s6 - s7 - s8 - s9 mod p */

    /* s1 = (c7,c6,c5,c4,c3,c2,c1,c0) */
    _nx_crypto_huge_number_setup(value, (UCHAR *)data + 32, 32);

    /* s2 = (c15,c14,c13,c12,c11,0,0,0) */
    NX_CRYPTO_EC_SECP256R1_DATA_SETUP_LS1(&temp, rev1, rev2,
                                          data[0], data[1], data[2], data[3],
                                          data[4], 0, 0, 0);
    _nx_crypto_huge_number_add(value, &temp);

    /* s3 = (0,c15,c14,c13,c12,0,0,0) */
    NX_CRYPTO_EC_SECP256R1_DATA_SETUP_LS1(&temp, rev1, rev2,
                                          0, data[0], data[1], data[2],
                                          data[3], 0, 0, 0);
    _nx_crypto_huge_number_add(value, &temp);

    /* s4 = (c15,c14,0,0,0,c10,c9,c8) */
    NX_CRYPTO_EC_SECP256R1_DATA_SETUP(&temp, rev1,
                                      data[0], data[1], 0, 0,
                                      0, data[5], data[6], data[7]);
    _nx_crypto_huge_number_add(value, &temp);

    /* s5 = (c8,c13,c15,c14,c13,c11,c10,c9) */
    NX_CRYPTO_EC_SECP256R1_DATA_SETUP(&temp, rev1,
                                      data[7], data[2], data[0], data[1],
                                      data[2], data[4], data[5], data[6]);
    _nx_crypto_huge_number_add(value, &temp);

    /* s6 = (c10,c8,0,0,0,c13,c12,c11) */
    NX_CRYPTO_EC_SECP256R1_DATA_SETUP(&temp, rev1,
                                      data[5], data[7], 0, 0,
                                      0, data[2], data[3], data[4]);
    _nx_crypto_huge_number_subtract(value, &temp);

    /* s7 = (c11,c9,0,0,c15,c14,c13,c12) */
    NX_CRYPTO_EC_SECP256R1_DATA_SETUP(&temp, rev1,
                                      data[4], data[6], 0, 0,
                                      data[0], data[1], data[2], data[3]);
    _nx_crypto_huge_number_subtract(value, &temp);

    /* s8 = (c12,0,c10,c9,c8,c15,c14,c13) */
    NX_CRYPTO_EC_SECP256R1_DATA_SETUP(&temp, rev1,
                                      data[3], 0, data[5], data[6],
                                      data[7], data[0], data[1], data[2]);
    _nx_crypto_huge_number_subtract(value, &temp);

    /* s9 = (c13,0,c11,c10,c9,0,c15,c14) */
    NX_CRYPTO_EC_SECP256R1_DATA_SETUP(&temp, rev1,
                                      data[2], 0, data[4], data[5],
                                      data[6], 0, data[0], data[1]);
    _nx_crypto_huge_number_subtract(value, &temp);

    compare_value = _nx_crypto_huge_number_compare_unsigned(value, &curve -> nx_crypto_ec_field.fp);
    while (compare_value != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        _nx_crypto_huge_number_subtract_unsigned(value, &curve -> nx_crypto_ec_field.fp, value);
        compare_value = _nx_crypto_huge_number_compare_unsigned(value,
                                                                &curve -> nx_crypto_ec_field.fp);
    }
    if (_nx_crypto_huge_number_is_zero(value))
    {
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
    else if (value -> nx_crypto_huge_number_is_negative)
    {
        _nx_crypto_huge_number_subtract_unsigned(&curve -> nx_crypto_ec_field.fp, value, value);
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_secp384r1_reduce                      PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reduces the value of curve secp384r1.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    point                                 Pointer to point              */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_SECP384R1_DATA_SETUP     Setup EC SECP384R1 data       */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
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
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_ec_secp384r1_reduce(NX_CRYPTO_EC *curve,
                                                   NX_CRYPTO_HUGE_NUMBER *value,
                                                   HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp;
UINT                 *data;
HN_UBASE              rev1;
HN_UBASE              rev2;
UINT                  size = 0;
UINT                  compare_value;

    compare_value = _nx_crypto_huge_number_compare(value, &curve -> nx_crypto_ec_field.fp);
    if (compare_value == NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        return;
    }
    else if (compare_value == NX_CRYPTO_HUGE_NUMBER_EQUAL)
    {
        NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(value, 0);
        return;
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch, 52);

    data = (UINT *)(((ULONG)scratch + 3) & (ULONG) ~3);

    /* c= (c23,...,c2,c1,c0), ci is a 32-bit word */
    _nx_crypto_huge_number_extract(value, (UCHAR *)data, 96, &size);
    if (size < 96)
    {
        NX_CRYPTO_MEMMOVE((UCHAR *)data + (96 - size), data, size); /* Use case of memmove is verified. */
        NX_CRYPTO_MEMSET(data, 0, (96 - size));
    }

    /* r = s1 + 2 * s2 + s3 + s4 + s5 + s6 + s7 - s8 - s9 -s10 mod p */

    /* s1 = (c11,c10,c9,c8,c7,c6,c5,c4,c3,c2,c1,c0) */
    _nx_crypto_huge_number_setup(value, (UCHAR *)data + 48, 48);

    /* s2 = (0,0,0,0,0,c23,c22,c21,0,0,0,0) */
    NX_CRYPTO_EC_SECP384R1_DATA_SETUP_LS1(&temp, rev1, rev2,
                                          0, 0, 0, 0, 0, data[0],
                                          data[1], data[2], 0, 0, 0, 0);
    _nx_crypto_huge_number_add(value, &temp);

    /* s3 = (c23,c22,c21,c20,c19,c18,c17,c16,c15,c14,c13,c12) */
    _nx_crypto_huge_number_setup(&temp, (UCHAR *)data, 48);
    _nx_crypto_huge_number_add(value, &temp);

    /* s4 = (c20,c19,c18,c17,c16,c15,c14,c13,c12,c23,c22,c21) */
    NX_CRYPTO_EC_SECP384R1_DATA_SETUP(&temp, rev1,
                                      data[3], data[4], data[5], data[6],
                                      data[7], data[8], data[9], data[10],
                                      data[11], data[0], data[1], data[2]);
    _nx_crypto_huge_number_add(value, &temp);

    /* s5 = (c19,c18,c17,c16,c15,c14,c13,c12,c20,0,c23,0) */
    NX_CRYPTO_EC_SECP384R1_DATA_SETUP(&temp, rev1,
                                      data[4], data[5], data[6],
                                      data[7], data[8], data[9], data[10],
                                      data[11], data[3], 0, data[0], 0);
    _nx_crypto_huge_number_add(value, &temp);

    /* s6 = (0,0,0,0,c23,c22,c21,c20,0,0,0,0) */
    NX_CRYPTO_EC_SECP384R1_DATA_SETUP(&temp, rev1,
                                      0, 0, 0, 0,
                                      data[0], data[1], data[2], data[3],
                                      0, 0, 0, 0);
    _nx_crypto_huge_number_add(value, &temp);

    /* s7 = (0,0,0,0,0,0,c23,c22,c21,0,0,c20) */
    NX_CRYPTO_EC_SECP384R1_DATA_SETUP(&temp, rev1,
                                      0, 0, 0, 0, 0, 0,
                                      data[0], data[1], data[2],
                                      0, 0, data[3]);
    _nx_crypto_huge_number_add(value, &temp);

    /* s8 = (c22,c21,c20,c19,c18,c17,c16,c15,c14,c13,c12,c23) */
    NX_CRYPTO_EC_SECP384R1_DATA_SETUP(&temp, rev1,
                                      data[1], data[2], data[3], data[4],
                                      data[5], data[6], data[7], data[8],
                                      data[9], data[10], data[11], data[0]);
    _nx_crypto_huge_number_subtract(value, &temp);

    /* s9 = (0,0,0,0,0,0,0,c23,c22,c21,c20,0) */
    NX_CRYPTO_EC_SECP384R1_DATA_SETUP(&temp, rev1,
                                      0, 0, 0, 0, 0, 0, 0, data[0],
                                      data[1], data[2], data[3], 0);
    _nx_crypto_huge_number_subtract(value, &temp);

    /* s10 = (0,0,0,0,0,0,0,c23,c23,0,0,0) */
    NX_CRYPTO_EC_SECP384R1_DATA_SETUP(&temp, rev1,
                                      0, 0, 0, 0, 0, 0, 0, data[0],
                                      data[0], 0, 0, 0);
    _nx_crypto_huge_number_subtract(value, &temp);

    compare_value = _nx_crypto_huge_number_compare_unsigned(value, &curve -> nx_crypto_ec_field.fp);
    while (compare_value != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        _nx_crypto_huge_number_subtract_unsigned(value, &curve -> nx_crypto_ec_field.fp, value);
        compare_value = _nx_crypto_huge_number_compare_unsigned(value,
                                                                &curve -> nx_crypto_ec_field.fp);
    }
    if (_nx_crypto_huge_number_is_zero(value))
    {
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
    else if (value -> nx_crypto_huge_number_is_negative)
    {
        _nx_crypto_huge_number_subtract_unsigned(&curve -> nx_crypto_ec_field.fp, value, value);
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_secp521r1_reduce                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reduces the value of curve secp521r1.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    point                                 Pointer to point              */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_SECP521R1_DATA_SETUP     Setup EC SECP521R1 data       */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_is_zero        Check if number is zero or not*/
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*    _nx_crypto_huge_number_shift_right    Shift right for huge number   */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_secp521r1_reduce(NX_CRYPTO_EC *curve,
                                                   NX_CRYPTO_HUGE_NUMBER *value,
                                                   HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp;
UCHAR                *data;
UINT                  size = 0;
UINT                  compare_value;

    compare_value = _nx_crypto_huge_number_compare(value, &curve -> nx_crypto_ec_field.fp);
    if (compare_value == NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        return;
    }
    else if (compare_value == NX_CRYPTO_HUGE_NUMBER_EQUAL)
    {
        NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(value, 0);
        return;
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch, 66);

    data = (UCHAR *)(((ULONG)scratch + 3) & (ULONG) ~3);


    /* c= (c1041,...,c2,c1,c0) */
    _nx_crypto_huge_number_extract(value, (UCHAR *)data, 132, &size);
    if (size < 132)
    {
        NX_CRYPTO_MEMMOVE(data + (132 - size), data, size); /* Use case of memmove is verified. */
        NX_CRYPTO_MEMSET(data, 0, (132 - size));
    }

    /* r = s1 + s2 mod p */

    /* s1 = (c1041,...,c523,c522,c521) */
    _nx_crypto_huge_number_setup(value, data, 67);
    _nx_crypto_huge_number_shift_right(value, 1);

    /* s2 = (c520,...,c2,c1,c0) */
    data[66] &= 1;
    _nx_crypto_huge_number_setup(&temp, data + 66, 66);
    _nx_crypto_huge_number_add(value, &temp);

    compare_value = _nx_crypto_huge_number_compare_unsigned(value, &curve -> nx_crypto_ec_field.fp);
    while (compare_value != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        _nx_crypto_huge_number_subtract_unsigned(value, &curve -> nx_crypto_ec_field.fp, value);
        compare_value = _nx_crypto_huge_number_compare_unsigned(value,
                                                                &curve -> nx_crypto_ec_field.fp);
    }
    if (_nx_crypto_huge_number_is_zero(value))
    {
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
    else if (value -> nx_crypto_huge_number_is_negative)
    {
        _nx_crypto_huge_number_subtract_unsigned(&curve -> nx_crypto_ec_field.fp, value, value);
        value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_add_digit_reduce                      PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs addition between huge number and digit       */
/*    number. The result is reduced after addition.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    value                                 Huge number                   */
/*    digit                                 Digit value                   */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_add_digit_unsigned                           */
/*                                          Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application                                                          */
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
#ifndef NX_CRYPTO_SELF_TEST
NX_CRYPTO_KEEP VOID _nx_crypto_ec_add_digit_reduce(NX_CRYPTO_EC *curve,
                                                   NX_CRYPTO_HUGE_NUMBER *value,
                                                   HN_UBASE digit,
                                                   HN_UBASE *scratch)
{
    NX_CRYPTO_PARAMETER_NOT_USED(scratch);

    _nx_crypto_huge_number_add_digit_unsigned(value, digit);
    if (_nx_crypto_huge_number_compare_unsigned(value,
                                                &curve -> nx_crypto_ec_field.fp) != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        _nx_crypto_huge_number_subtract_unsigned(value, &curve -> nx_crypto_ec_field.fp, value);
    }
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_subtract_digit_reduce                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs subtraction between huge number and digit    */
/*    number. The result is reduced after subtraction.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    value                                 Huge number                   */
/*    digit                                 Digit value                   */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_add_unsigned   Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_subtract_digit_unsigned                      */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_affine_add           Perform addition for points of*/
/*                                            affine                      */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_subtract_digit_reduce(NX_CRYPTO_EC *curve,
                                                        NX_CRYPTO_HUGE_NUMBER *value,
                                                        HN_UBASE digit,
                                                        HN_UBASE *scratch)
{
    NX_CRYPTO_PARAMETER_NOT_USED(scratch);

    if (((value -> nx_crypto_huge_number_size == 1) ||
         (value -> nx_crypto_huge_number_data[0] < digit)))
    {
        _nx_crypto_huge_number_add_unsigned(value, &curve -> nx_crypto_ec_field.fp);
    }
    _nx_crypto_huge_number_subtract_digit_unsigned(value, digit);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_fp_reduce                             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reduces huge number for common prime field.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    value                                 Huge number                   */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
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
#ifndef NX_CRYPTO_SELF_TEST
NX_CRYPTO_KEEP VOID _nx_crypto_ec_fp_reduce(NX_CRYPTO_EC *curve,
                                            NX_CRYPTO_HUGE_NUMBER *value,
                                            HN_UBASE *scratch)
{
    NX_CRYPTO_PARAMETER_NOT_USED(scratch);

    _nx_crypto_huge_number_modulus(value, &curve -> nx_crypto_ec_field.fp);
}
#endif
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_add_reduce                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs addition between two huge numbers. The result*/
/*    is reduced after addition.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    left                                  Left huge number              */
/*    right                                 Right huge number             */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_add_unsigned   Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_projective_add       Perform addition for points of*/
/*                                            projective and affine       */
/*    _nx_crypto_ec_fp_projective_double    Perform doubling for points of*/
/*                                            projective                  */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_add_reduce(NX_CRYPTO_EC *curve,
                                             NX_CRYPTO_HUGE_NUMBER *left,
                                             NX_CRYPTO_HUGE_NUMBER *right,
                                             HN_UBASE *scratch)
{
    NX_CRYPTO_PARAMETER_NOT_USED(scratch);

    _nx_crypto_huge_number_add_unsigned(left, right);
    if (_nx_crypto_huge_number_compare_unsigned(left,
                                                &curve -> nx_crypto_ec_field.fp) != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        _nx_crypto_huge_number_subtract_unsigned(left, &curve -> nx_crypto_ec_field.fp, left);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_subtract_reduce                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs subtraction between two huge numbers. The    */
/*    result is reduced after subtraction.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    left                                  Left huge number              */
/*    right                                 Right huge number             */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_add_unsigned   Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_affine_add           Perform addition for points of*/
/*                                            affine                      */
/*    _nx_crypto_ec_fp_affine_subtract      Perform subtraction for points*/
/*                                            of affine                   */
/*    _nx_crypto_ec_fp_projective_add       Perform addition for points of*/
/*                                            projective and affine       */
/*    _nx_crypto_ec_fp_projective_double    Perform doubling for points of*/
/*                                            projective                  */
/*    _nx_crypto_ec_fp_projective_multiple  Calculate the projective      */
/*                                            multiplication              */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_subtract_reduce(NX_CRYPTO_EC *curve,
                                                  NX_CRYPTO_HUGE_NUMBER *left,
                                                  NX_CRYPTO_HUGE_NUMBER *right,
                                                  HN_UBASE *scratch)
{
    NX_CRYPTO_PARAMETER_NOT_USED(scratch);

    if (_nx_crypto_huge_number_compare_unsigned(left, right) == NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        _nx_crypto_huge_number_add_unsigned(left, &curve -> nx_crypto_ec_field.fp);
    }
    _nx_crypto_huge_number_subtract_unsigned(left, right, left);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_fp_projective_add                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs addition for points of projective and affine */
/*    coordinates in prime filed.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    projective_point                      Projective point              */
/*    affine_point                          Affine point                  */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_MULTIPLE_REDUCE          Multiply two huge numbers     */
/*    NX_CRYPTO_EC_SHIFT_LEFT_REDUCE        Shift left N bits             */
/*    NX_CRYPTO_EC_SQUARE_REDUCE            Computes the square of a value*/
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_ec_add_reduce              Perform addition between      */
/*                                            two huge numbers            */
/*    _nx_crypto_ec_point_is_infinite       Check if the point is infinite*/
/*    _nx_crypto_ec_point_fp_affine_to_projective                         */
/*                                          Convert point from affine to  */
/*                                            projective                  */
/*    _nx_crypto_ec_subtract_reduce         Perform subtraction between   */
/*                                            two huge numbers            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_fixed_multiple       Calculate the fixed           */
/*                                            multiplication              */
/*    _nx_crypto_ec_fp_projective_multiple  Calculate the projective      */
/*                                            multiplication              */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_fp_projective_add(NX_CRYPTO_EC *curve,
                                                    NX_CRYPTO_EC_POINT *projective_point,
                                                    NX_CRYPTO_EC_POINT *affine_point,
                                                    HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp1, temp2, temp3, temp4, temp5;
UINT                  buffer_size;

    /* Page 12, Software Implementation of the NIST Elliptic Curves Over Prime Fields. */
    /*
        A = X2 * Z1 ^ 2
        B = Y2 * Z1 ^ 3
        C = A - X1
        D = B - Y1
        X3 = D ^ 2 - (C ^ 3 + 2 * X1 * C ^ 2)
        Y3 = D * (X1 * C ^ 2 - X3) - Y1 * C ^ 3
        Z3 = Z1 * C
     */

    if (_nx_crypto_ec_point_is_infinite(projective_point))
    {
        NX_CRYPTO_HUGE_NUMBER_COPY(&projective_point -> nx_crypto_ec_point_x,
                                   &affine_point -> nx_crypto_ec_point_x);
        NX_CRYPTO_HUGE_NUMBER_COPY(&projective_point -> nx_crypto_ec_point_y,
                                   &affine_point -> nx_crypto_ec_point_y);
        _nx_crypto_ec_point_fp_affine_to_projective(projective_point);
        return;
    }

    if (_nx_crypto_ec_point_is_infinite(affine_point))
    {
        return;
    }

    buffer_size = projective_point -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size;
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp1, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp2, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp3, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp4, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp5, scratch, buffer_size << 1);

    /* A = X2 * Z1 ^ 2 */
    /* C = A - X1 */
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_z,
                               &temp1, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &affine_point -> nx_crypto_ec_point_x,
                                 &temp1, &temp2, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp2, &projective_point -> nx_crypto_ec_point_x,
                                  scratch);

    /* temp1 = Z1 ^ 2 */
    /* B = Y2 * Z1 ^ 3 */
    /* D = B - Y1 */
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_z,
                                 &temp1, &temp3, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &affine_point -> nx_crypto_ec_point_y,
                                 &temp3, &temp1, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp1, &projective_point -> nx_crypto_ec_point_y,
                                  scratch);

    /* temp2 = C */
    /* Z3 = Z1 * C */
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_z,
                                 &temp2, &temp3, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&projective_point -> nx_crypto_ec_point_z, &temp3);

    /* temp1 = D
     * temp2 = C */
    /* X3 = D ^ 2 - (C ^ 3 + 2 * X1 * C ^ 2) */
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &temp2, &temp3, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &temp2, &temp3, &temp4, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_x,
                                 &temp3, &temp2, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp3, &temp2);
    NX_CRYPTO_EC_SHIFT_LEFT_REDUCE(curve, &temp3, 1, scratch);
    _nx_crypto_ec_add_reduce(curve, &temp3, &temp4, scratch);
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &temp1, &temp5, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp5, &temp3, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&projective_point -> nx_crypto_ec_point_x, &temp5);

    /* temp1 = D
     * temp2 = X1 * C ^ 2
     * temp4 = C ^ 3
     * temp5 = X3 */
    /* Y3 = D * (X1 * C ^ 2 - X3) - Y1 * C ^ 3 */
    _nx_crypto_ec_subtract_reduce(curve, &temp2, &temp5, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &temp1, &temp2, &temp3, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_y,
                                 &temp4, &temp1, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp3, &temp1, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&projective_point -> nx_crypto_ec_point_y, &temp3);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_fp_projective_double                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs doubling for point of projective coordinate  */
/*    in prime filed.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    projective_point                      Point in projective coordinate*/
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_MULTIPLE_DIGIT_REDUCE    Multiply two huge numbers     */
/*                                            with digit                  */
/*    NX_CRYPTO_EC_MULTIPLE_REDUCE          Multiply two huge numbers     */
/*    NX_CRYPTO_EC_SHIFT_LEFT_REDUCE        Shift left N bits             */
/*    NX_CRYPTO_EC_SQUARE_REDUCE            Computes the square of a value*/
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_ec_add_reduce              Perform addition between      */
/*                                            two huge numbers            */
/*    _nx_crypto_ec_subtract_reduce         Perform subtraction between   */
/*                                            two huge numbers            */
/*    _nx_crypto_ec_point_is_infinite       Check if the point is infinite*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_fixed_multiple       Calculate the fixed           */
/*                                            multiplication              */
/*    _nx_crypto_ec_fp_projective_multiple  Calculate the projective      */
/*                                            multiplication              */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_fp_projective_double(NX_CRYPTO_EC *curve,
                                                       NX_CRYPTO_EC_POINT *projective_point,
                                                       HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp1, temp2, temp3, temp4, temp5;
UINT                  buffer_size;

    /* Page 12, Software Implementation of the NIST Elliptic Curves Over Prime Fields. */
    /*
        A = 4 * X1 * Y1 ^ 2
        B = 8 * Y1 ^ 4
        C = 3 * (X1 - Z1 ^ 2) * (X1 + Z1 ^ 2)
        D = C ^ 2 - 2 * A
        X3 = D
        Y3 = C * (A - D) - B
        Z3 = 2 * Y1 * Z1
     */

    if (_nx_crypto_ec_point_is_infinite(projective_point))
    {
        return;
    }

    buffer_size = projective_point -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size;
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp1, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp2, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp3, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp4, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp5, scratch, buffer_size << 1);

    /* A = 4 * X1 * Y1 ^ 2 */
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_y,
                               &temp3, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_x,
                                 &temp3, &temp1, scratch);
    NX_CRYPTO_EC_SHIFT_LEFT_REDUCE(curve, &temp1, 2, scratch);

    /* temp3 = Y1 ^ 2 */
    /* B = 8 * Y1 ^ 4 */
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &temp3, &temp2, scratch);
    NX_CRYPTO_EC_SHIFT_LEFT_REDUCE(curve, &temp2, 3, scratch);

    /* C = 3 * (X1 - Z1 ^ 2) * (X1 + Z1 ^ 2) */
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_z,
                               &temp3, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp4, &projective_point -> nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp5, &projective_point -> nx_crypto_ec_point_x);
    _nx_crypto_ec_subtract_reduce(curve, &temp4, &temp3, scratch);
    _nx_crypto_ec_add_reduce(curve, &temp5, &temp3, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &temp4, &temp5, &temp3, scratch);
    NX_CRYPTO_EC_MULTIPLE_DIGIT_REDUCE(curve, &temp3, 3, &temp4, scratch);

    /* Z3 = 2 * Y1 * Z1 */
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &projective_point -> nx_crypto_ec_point_y,
                                 &projective_point -> nx_crypto_ec_point_z, &temp5, scratch);
    NX_CRYPTO_EC_SHIFT_LEFT_REDUCE(curve, &temp5, 1, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&projective_point -> nx_crypto_ec_point_z, &temp5);

    /* temp1 = A
     * temp4 = C */
    /* D = C ^ 2 - 2 * A */
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &temp4, &temp3, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp5, &temp1);
    NX_CRYPTO_EC_SHIFT_LEFT_REDUCE(curve, &temp5, 1, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp3, &temp5, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&projective_point -> nx_crypto_ec_point_x, &temp3);

    /* temp1 = A
     * temp2 = B
     * temp3 = D
     * temp4 = C */
    /* Y3 = C * (A - D) - B */
    _nx_crypto_ec_subtract_reduce(curve, &temp1, &temp3, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &temp1, &temp4, &temp3, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp3, &temp2, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&projective_point -> nx_crypto_ec_point_y, &temp3);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_fp_affine_add                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs addition for two points of affine coordinate */
/*    in prime field.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    left                                  Left huge number              */
/*    right                                 Right huge number             */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_MULTIPLE_DIGIT_REDUCE    Multiply two huge numbers     */
/*                                            with digit                  */
/*    NX_CRYPTO_EC_MULTIPLE_REDUCE          Multiply two huge numbers     */
/*    NX_CRYPTO_EC_SHIFT_LEFT_REDUCE        Shift left N bits             */
/*    NX_CRYPTO_EC_SQUARE_REDUCE            Computes the square of a value*/
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    _nx_crypto_ec_subtract_digit_reduce   Perform subtraction between   */
/*                                            huge number and digit number*/
/*    _nx_crypto_ec_subtract_reduce         Perform subtraction between   */
/*                                            two huge numbers            */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_affine_subtract      Perform subtraction for points*/
/*                                            of affine                   */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_fp_affine_add(NX_CRYPTO_EC *curve,
                                                NX_CRYPTO_EC_POINT *left,
                                                NX_CRYPTO_EC_POINT *right,
                                                HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER  temp1, temp2, temp3;
NX_CRYPTO_HUGE_NUMBER *p;
UINT                   buffer_size;

    if (_nx_crypto_ec_point_is_infinite(left))
    {
        NX_CRYPTO_HUGE_NUMBER_COPY(&left -> nx_crypto_ec_point_x,
                                   &right -> nx_crypto_ec_point_x);
        NX_CRYPTO_HUGE_NUMBER_COPY(&left -> nx_crypto_ec_point_y,
                                   &right -> nx_crypto_ec_point_y);
        return;
    }

    if (_nx_crypto_ec_point_is_infinite(right))
    {
        return;
    }

    p = &curve -> nx_crypto_ec_field.fp;

    buffer_size = left -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size;
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp1, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp2, scratch, buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp3, scratch, buffer_size << 1);

    if ((_nx_crypto_huge_number_compare(&left -> nx_crypto_ec_point_x,
                                        &right -> nx_crypto_ec_point_x) == NX_CRYPTO_HUGE_NUMBER_EQUAL) &&
        (_nx_crypto_huge_number_compare(&left -> nx_crypto_ec_point_y,
                                        &right -> nx_crypto_ec_point_y) == NX_CRYPTO_HUGE_NUMBER_EQUAL))
    {

        /* Double */
        /* r = (3 * x1 ^ 2 - 3) * (2 * y1) ^ -1 mod p */
        NX_CRYPTO_HUGE_NUMBER_COPY(&temp3, &left -> nx_crypto_ec_point_y);
        NX_CRYPTO_EC_SHIFT_LEFT_REDUCE(curve, &temp3, 1, scratch);
        _nx_crypto_huge_number_inverse_modulus_prime(&temp3, p, &temp1, scratch);
        NX_CRYPTO_EC_SQUARE_REDUCE(curve, &left -> nx_crypto_ec_point_x,
                                   &temp3, scratch);
        NX_CRYPTO_EC_MULTIPLE_DIGIT_REDUCE(curve, &temp3, 3, &temp2, scratch);
        _nx_crypto_ec_subtract_digit_reduce(curve, &temp2, 3, scratch);
        NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &temp2, &temp1, &temp3, scratch);
    }
    else
    {

        /* Add */
        /* r = (y2 - y1) * (x2 - x1) ^ -1 mod p */
        NX_CRYPTO_HUGE_NUMBER_COPY(&temp1, &right -> nx_crypto_ec_point_x);
        _nx_crypto_ec_subtract_reduce(curve, &temp1, &left -> nx_crypto_ec_point_x, scratch);
        _nx_crypto_huge_number_inverse_modulus_prime(&temp1, p, &temp2, scratch);
        NX_CRYPTO_HUGE_NUMBER_COPY(&temp1, &right -> nx_crypto_ec_point_y);
        _nx_crypto_ec_subtract_reduce(curve, &temp1, &left -> nx_crypto_ec_point_y, scratch);
        NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &temp1, &temp2, &temp3, scratch);
    }

    /* x3 = r ^ 2 - x1 - x2 */
    NX_CRYPTO_EC_SQUARE_REDUCE(curve, &temp3, &temp1, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp1, &left -> nx_crypto_ec_point_x, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp1, &right -> nx_crypto_ec_point_x, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp2, &left -> nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&left -> nx_crypto_ec_point_x, &temp1);

    /* y3 = r * (x1 - x3) - y1 */
    _nx_crypto_ec_subtract_reduce(curve, &temp2, &temp1, scratch);
    NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, &temp2, &temp3, &temp1, scratch);
    _nx_crypto_ec_subtract_reduce(curve, &temp1, &left -> nx_crypto_ec_point_y, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&left -> nx_crypto_ec_point_y, &temp1);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_fp_affine_subtract                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs subtraction for two points of affine         */
/*    coordinate in prime field.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    left                                  Left huge number              */
/*    right                                 Right huge number             */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    _nx_crypto_ec_fp_affine_add           Perform addition for points of*/
/*                                            affine                      */
/*    _nx_crypto_ec_subtract_reduce         Perform subtraction between   */
/*                                            two huge numbers            */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_fp_affine_subtract(NX_CRYPTO_EC *curve,
                                                     NX_CRYPTO_EC_POINT *left,
                                                     NX_CRYPTO_EC_POINT *right,
                                                     HN_UBASE *scratch)
{
NX_CRYPTO_EC_POINT point;

    NX_CRYPTO_EC_POINT_INITIALIZE(&point, NX_CRYPTO_EC_POINT_AFFINE, scratch,
                                  right -> nx_crypto_ec_point_y.nx_crypto_huge_buffer_size);
    NX_CRYPTO_HUGE_NUMBER_COPY(&point.nx_crypto_ec_point_x, &right -> nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&point.nx_crypto_ec_point_y, &curve -> nx_crypto_ec_field.fp);

    _nx_crypto_ec_subtract_reduce(curve, &point.nx_crypto_ec_point_y,
                                  &right -> nx_crypto_ec_point_y, scratch);
    _nx_crypto_ec_fp_affine_add(curve, left, &point, scratch);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_naf_compute                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes the non-adjacent form(NAF) of huge number.   */
/*    It has the property that no two consecutive coeffcients are nonzero.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    d                                     Pointer to huge number        */
/*    naf_data                              Buffer of NAF for output      */
/*    naf_size                              Size of NAF                   */
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
/*    _nx_crypto_ec_fp_projective_multiple  Calculate the projective      */
/*                                            multiplication              */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_naf_compute(NX_CRYPTO_HUGE_NUMBER *d, HN_UBASE *naf_data, UINT *naf_size)
{
HN_UBASE2 digit;
HN_UBASE  value;
HN_UBASE *ptr;
UINT      shift;
UINT      i, j;

    shift = 0;
    ptr = naf_data;
    *ptr = 0;
    digit = 0;
    for (i = 0; i < d -> nx_crypto_huge_number_size - 1; i++)
    {
        digit = digit + d -> nx_crypto_huge_number_data[i];
        for (j = 0; j < (HN_SHIFT - 1); j++)
        {
            value = digit & 3;

            *ptr = *ptr | (value << shift);
            shift += 2;
            if (shift == HN_SHIFT)
            {
                shift = 0;
                ptr++;
                *ptr = 0;
            }

            digit >>= 1;

            if (value == 3)
            {
                digit++;
            }
        }

        value = (((d -> nx_crypto_huge_number_data[i + 1] & 1) << 1) + digit) & 3;

        *ptr = *ptr | (value << shift);
        shift += 2;
        if (shift == HN_SHIFT)
        {
            shift = 0;
            ptr++;
            *ptr = 0;
        }

        digit >>= 1;

        if (value == 3)
        {
            digit++;
        }
    }

    digit = digit + d -> nx_crypto_huge_number_data[i];
    while (digit >= 1)
    {
        value = digit & 3;

        *ptr = *ptr | (value << shift);
        shift += 2;
        if (shift == HN_SHIFT)
        {
            shift = 0;
            ptr++;
            *ptr = 0;
        }

        digit >>= 1;

        if (value == 3)
        {
            digit++;
        }
    }

    *naf_size = ((ULONG)ptr - (ULONG)naf_data) >> HN_SIZE_SHIFT;
    if (shift != 0)
    {
        *naf_size = *naf_size + 1;
    }
}

/* r and g are allowed to be the same pointer. */
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_fp_projective_multiple                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates the multiplication in prime field. The     */
/*    point g is unknown. r = g * d.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    g                                     Base point g                  */
/*    d                                     Factor d                      */
/*    r                                     Result r                      */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    _nx_crypto_ec_fp_fixed_multiple       Calculate the fixed           */
/*                                            multiplication              */
/*    _nx_crypto_ec_fp_projective_add       Perform addition for points of*/
/*                                            projective and affine       */
/*    _nx_crypto_ec_fp_projective_double    Perform doubling for points of*/
/*                                            projective                  */
/*    _nx_crypto_ec_naf_compute             Compute the non-adjacent form */
/*                                            of huge number              */
/*    _nx_crypto_ec_point_fp_projective_to_affine                         */
/*                                          Convert point from projective */
/*                                            to affine                   */
/*    _nx_crypto_ec_point_set_infinite      Set the point to infinite     */
/*    _nx_crypto_ec_subtract_reduce         Perform subtraction between   */
/*                                            two huge numbers            */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_fp_projective_multiple(NX_CRYPTO_EC *curve,
                                                         NX_CRYPTO_EC_POINT *g,
                                                         NX_CRYPTO_HUGE_NUMBER *d,
                                                         NX_CRYPTO_EC_POINT *r,
                                                         HN_UBASE *scratch)
{
NX_CRYPTO_EC_POINT projective_point;
NX_CRYPTO_EC_POINT negative_g;
HN_UBASE          *naf_data;
UINT               naf_size;
HN_UBASE           digit;
HN_UBASE           value;
HN_UBASE          *ptr;
UINT               bit;

    if ((curve -> nx_crypto_ec_fixed_points) && (&curve -> nx_crypto_ec_g == g))
    {
        _nx_crypto_ec_fp_fixed_multiple(curve, d, r, scratch);
        return;
    }

    NX_CRYPTO_EC_POINT_INITIALIZE(&projective_point, NX_CRYPTO_EC_POINT_PROJECTIVE, scratch,
                                  g -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size);
    NX_CRYPTO_EC_POINT_INITIALIZE(&negative_g, NX_CRYPTO_EC_POINT_AFFINE, scratch,
                                  g -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size);

    _nx_crypto_ec_point_set_infinite(&projective_point);
    NX_CRYPTO_HUGE_NUMBER_COPY(&negative_g.nx_crypto_ec_point_x, &g -> nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&negative_g.nx_crypto_ec_point_y, &curve -> nx_crypto_ec_field.fp);
    _nx_crypto_ec_subtract_reduce(curve, &negative_g.nx_crypto_ec_point_y,
                                  &g -> nx_crypto_ec_point_y, scratch);

    naf_data = scratch;
    _nx_crypto_ec_naf_compute(d, naf_data, &naf_size);
    scratch += naf_size;

    /* Trim zero bit from the highest significant bit. */
    ptr = naf_data + naf_size - 1;
    digit = *ptr;
    for (bit = 0; bit < HN_SHIFT - 1; bit += 2)
    {
        value = (digit >> (HN_SHIFT - bit - 2)) & 3;
        if (value != 0)
        {
            break;
        }
    }

    for (; (ULONG)ptr >= (ULONG)naf_data; ptr--)
    {
        digit = *ptr;

        for (; bit < HN_SHIFT - 1; bit += 2)
        {
            _nx_crypto_ec_fp_projective_double(curve, &projective_point, scratch);

            value = (digit >> (HN_SHIFT - bit - 2)) & 3;

            if (value == 1)
            {
                _nx_crypto_ec_fp_projective_add(curve, &projective_point, g, scratch);
            }
            else if (value == 3)
            {
                _nx_crypto_ec_fp_projective_add(curve, &projective_point, &negative_g, scratch);
            }
        }
        bit = 0;
    }

    _nx_crypto_ec_point_fp_projective_to_affine(curve, &projective_point, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&r -> nx_crypto_ec_point_x,
                               &projective_point.nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&r -> nx_crypto_ec_point_y,
                               &projective_point.nx_crypto_ec_point_y);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_fp_fixed_multiple                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates the multiplication in prime field. The     */
/*    point g is selected from the curve. And the value of 2 ^ N * g      */
/*    has been precomputed. r = g * d.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    d                                     Factor d                      */
/*    r                                     Result r                      */
/*    scratch                               Pointer to scratch buffer     */
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
/*    _nx_crypto_ec_fp_projective_add       Perform addition for points of*/
/*                                            projective and affine       */
/*    _nx_crypto_ec_fp_projective_double    Perform doubling for points of*/
/*                                            projective                  */
/*    _nx_crypto_ec_point_fp_projective_to_affine                         */
/*                                          Convert point from projective */
/*                                            to affine                   */
/*    _nx_crypto_ec_point_set_infinite      Set the point to infinite     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_projective_multiple  Calculate the projective      */
/*                                            multiplication              */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_fp_fixed_multiple(NX_CRYPTO_EC *curve,
                                                    NX_CRYPTO_HUGE_NUMBER *d,
                                                    NX_CRYPTO_EC_POINT *r,
                                                    HN_UBASE *scratch)
{
NX_CRYPTO_EC_POINT         projective_point;
NX_CRYPTO_EC_POINT        *g;
NX_CRYPTO_EC_FIXED_POINTS *fixed_points;
NX_CRYPTO_HUGE_NUMBER      expanded_d;
UINT                       expanded_size;
ULONG                      transpose_d;
HN_UBASE                   value;
UINT                       bit_index;
INT                        i;
UINT                       j;

    fixed_points = curve -> nx_crypto_ec_fixed_points;
    expanded_size = fixed_points -> nx_crypto_ec_fixed_points_window_width *
        (fixed_points -> nx_crypto_ec_fixed_points_e << 1);
    expanded_size = (expanded_size + 7) >> 3;
    expanded_size = (expanded_size + 3) & (ULONG) ~3;

    g = &curve -> nx_crypto_ec_g;

    NX_CRYPTO_EC_POINT_INITIALIZE(&projective_point, NX_CRYPTO_EC_POINT_PROJECTIVE, scratch,
                                  g -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size << 1);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&expanded_d, scratch, expanded_size);
    _nx_crypto_ec_point_set_infinite(&projective_point);

    NX_CRYPTO_HUGE_NUMBER_COPY(&expanded_d, d);
    NX_CRYPTO_MEMSET(&expanded_d.nx_crypto_huge_number_data[expanded_d.nx_crypto_huge_number_size], 0,
           expanded_size - (d -> nx_crypto_huge_number_size << HN_SIZE_SHIFT));
    expanded_d.nx_crypto_huge_number_size = expanded_size >> HN_SIZE_SHIFT;

    for (i = (INT)(fixed_points -> nx_crypto_ec_fixed_points_e - 1); i >= 0; i--)
    {
        _nx_crypto_ec_fp_projective_double(curve, &projective_point, scratch);

        transpose_d = 0;
        bit_index = (UINT)i;

        for (j = 0; j < fixed_points -> nx_crypto_ec_fixed_points_window_width; j++)
        {
            value = expanded_d.nx_crypto_huge_number_data[bit_index >> (HN_SIZE_SHIFT + 3)];
            transpose_d |= (((value >> (bit_index & (NX_CRYPTO_HUGE_NUMBER_BITS - 1))) & 1) << j);
            bit_index += fixed_points -> nx_crypto_ec_fixed_points_d;
        }

        if (transpose_d == 1)
        {
            _nx_crypto_ec_fp_projective_add(curve, &projective_point, g, scratch);
        }
        else if (transpose_d > 0)
        {
            _nx_crypto_ec_fp_projective_add(curve, &projective_point,
                                            &fixed_points -> nx_crypto_ec_fixed_points_array[transpose_d - 2],
                                            scratch);
        }
        if ((fixed_points -> nx_crypto_ec_fixed_points_d & 1) &&
            (i == (INT)(fixed_points -> nx_crypto_ec_fixed_points_e - 1)))
        {
            continue;
        }

        transpose_d = 0;
        bit_index = (UINT)(i + (INT)fixed_points -> nx_crypto_ec_fixed_points_e);
        for (j = 0; j < fixed_points -> nx_crypto_ec_fixed_points_window_width; j++)
        {
            value = expanded_d.nx_crypto_huge_number_data[bit_index >> (HN_SIZE_SHIFT + 3)];
            transpose_d |= (((value >> (bit_index & (NX_CRYPTO_HUGE_NUMBER_BITS - 1))) & 1) << j);
            bit_index += fixed_points -> nx_crypto_ec_fixed_points_d;
        }

        if (transpose_d > 0)
        {
            _nx_crypto_ec_fp_projective_add(curve, &projective_point,
                                            &fixed_points -> nx_crypto_ec_fixed_points_array_2e[transpose_d - 1],
                                            scratch);
        }
    }

    _nx_crypto_ec_point_fp_projective_to_affine(curve, &projective_point, scratch);
    NX_CRYPTO_HUGE_NUMBER_COPY(&r -> nx_crypto_ec_point_x,
                               &projective_point.nx_crypto_ec_point_x);
    NX_CRYPTO_HUGE_NUMBER_COPY(&r -> nx_crypto_ec_point_y,
                               &projective_point.nx_crypto_ec_point_y);
}

/* nist.fips.186-4 APPENDIX B.4.1 */
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_key_pair_generation_extra             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates Elliptic Curve Key Pair using extra random  */
/*    bits per nist.fips.186-4 APPENDIX B.4.1.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    g                                     Base point g                  */
/*    private_key                           Private key generated         */
/*    public_key                            Public key generated          */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    [nx_crypto_ec_multiple]               Perform multiplication for EC */
/*    _nx_crypto_huge_number_subtract_digit_unsigned                      */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_add_digit_unsigned                           */
/*                                          Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                            number to remove leading    */
/*                                            zeroes                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_ec_key_pair_stream_generate                              */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ec_key_pair_generation_extra(NX_CRYPTO_EC *curve,
                                                            NX_CRYPTO_EC_POINT *g,
                                                            NX_CRYPTO_HUGE_NUMBER *private_key,
                                                            NX_CRYPTO_EC_POINT *public_key,
                                                            HN_UBASE *scratch)
{
UINT status;
UINT bits = curve -> nx_crypto_ec_bits + 64;
UINT buffer_size = (bits + 7) >> 3;
NX_CRYPTO_HUGE_NUMBER random_number;
NX_CRYPTO_HUGE_NUMBER modulus;

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&random_number, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&modulus, scratch, buffer_size);

    /* Get random number with specified length. */
    status = NX_CRYPTO_RBG(bits, (UCHAR *)scratch);

    if (status)
    {
        return(status);
    }

    status = _nx_crypto_huge_number_setup(&random_number, (const UCHAR *)scratch, buffer_size);

    if (status)
    {
        return(status);
    }

    /* d = (c mod (n-1))+1 */
    NX_CRYPTO_HUGE_NUMBER_COPY(&modulus, &curve -> nx_crypto_ec_n);
    _nx_crypto_huge_number_subtract_digit_unsigned(&modulus, 1u);

    _nx_crypto_huge_number_modulus(&random_number, &modulus);
    _nx_crypto_huge_number_add_digit_unsigned(&random_number, 1u);
    _nx_crypto_huge_number_adjust_size(&random_number);
    NX_CRYPTO_HUGE_NUMBER_COPY(private_key, &random_number);

    /* Q = dG */
    curve -> nx_crypto_ec_multiple(curve, g, private_key, public_key, scratch);

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_key_pair_stream_generate              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates Elliptic Curve Key Pair using extra random  */
/*    bits per nist.fips.186-4 APPENDIX B.4.1.                            */
/*                                                                        */
/*    The output is: private_key || public_key. The public_key is in      */
/*    uncompressed format. The output length is (3 * key_size + 1).       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    output                                Pointer to output buffer      */
/*    output_length_in_byte                 Length of output in byte      */
/*    actual_output_length                  Actual length of output buffer*/
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ec_key_pair_generation_extra                             */
/*                                          Generate key pair             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ec_key_pair_stream_generate(NX_CRYPTO_EC *curve,
                                                           UCHAR *output,
                                                           ULONG output_length_in_byte,
                                                           ULONG *actual_output_length,
                                                           HN_UBASE *scratch)
{
UINT status;
UINT private_key_len;
UINT public_key_len;
/* Actual huge numbers used in calculations */
NX_CRYPTO_HUGE_NUMBER private_key;
NX_CRYPTO_EC_POINT    public_key;

    /* Get key length. */
    private_key_len = (curve -> nx_crypto_ec_bits + 7) >> 3;
    public_key_len = 1 + (private_key_len << 1);

    /* Check output buffer size. */
    if ((private_key_len + public_key_len) > output_length_in_byte)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Public key buffer (and scratch). */
    NX_CRYPTO_EC_POINT_INITIALIZE(&public_key, NX_CRYPTO_EC_POINT_AFFINE, scratch,
                                  private_key_len);

    /* Private key buffer - note that no scratch is required for the private key. */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&private_key, scratch, private_key_len);

    /* Generate Key Pair. */
    status = _nx_crypto_ec_key_pair_generation_extra(curve, &curve -> nx_crypto_ec_g, &private_key,
                                                     &public_key, scratch);
    if (status)
    {
        return(status);
    }

    /* Copy the private key and public key into the return buffer. */
    status = _nx_crypto_huge_number_extract_fixed_size(&private_key, output, private_key_len);
    if (status)
    {
        return(status);
    }

    _nx_crypto_ec_point_extract_uncompressed(curve, &public_key, &output[private_key_len],
                                             public_key_len, &public_key_len);

    if (public_key_len == 0)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }
    *actual_output_length = (private_key_len + public_key_len);

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_precomputation                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function precomputes the fixed points of the curve. With fixed */
/*    points, the multiplication of the curve is faster than the one      */
/*    without fixed points.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    window_width                          Width of window               */
/*    bits                                  Bits of fixed points          */
/*    scratch                               Pointer to scratch buffer     */
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
/*    [nx_crypto_ec_add]                    Perform addtion for EC        */
/*    [nx_crypto_ec_multiple]               Perform multiplication for EC */
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
#ifndef NX_CRYPTO_SELF_TEST
NX_CRYPTO_KEEP VOID _nx_crypto_ec_precomputation(NX_CRYPTO_EC *curve,
                                                 UINT window_width,
                                                 UINT bits,
                                                 HN_UBASE **scratch_pptr)
{
NX_CRYPTO_EC_FIXED_POINTS *fixed_points;
NX_CRYPTO_EC_POINT        *g;
NX_CRYPTO_EC_POINT        *array;
NX_CRYPTO_HUGE_NUMBER      d;
HN_UBASE                  *scratch_ptr;
UINT                       offset;
UINT                       i, j;

    if (curve -> nx_crypto_ec_fixed_points)
    {

        /* Fixed points are already computed. */
        return;
    }

    if (window_width == 0)
    {
        return;
    }

    g = &curve -> nx_crypto_ec_g;

    scratch_ptr = *scratch_pptr;
    fixed_points = (NX_CRYPTO_EC_FIXED_POINTS *)scratch_ptr;
    scratch_ptr += sizeof(NX_CRYPTO_EC_FIXED_POINTS) >> HN_SIZE_SHIFT;

    fixed_points -> nx_crypto_ec_fixed_points_window_width = window_width;
    fixed_points -> nx_crypto_ec_fixed_points_bits = bits;
    fixed_points -> nx_crypto_ec_fixed_points_d = (bits + window_width - 1) / window_width;
    fixed_points -> nx_crypto_ec_fixed_points_e = (fixed_points -> nx_crypto_ec_fixed_points_d +
                                                   1) >> 1;

    fixed_points -> nx_crypto_ec_fixed_points_array = (NX_CRYPTO_EC_POINT *)scratch_ptr;
    scratch_ptr += (sizeof(NX_CRYPTO_EC_POINT) * (UINT)((1 << window_width) - 2)) >> HN_SIZE_SHIFT;
    fixed_points -> nx_crypto_ec_fixed_points_array_2e = (NX_CRYPTO_EC_POINT *)scratch_ptr;
    scratch_ptr += (sizeof(NX_CRYPTO_EC_POINT) * (UINT)((1 << window_width) - 1)) >> HN_SIZE_SHIFT;

    /* Allocate buffers for fixed points. */
    for (i = 0; i < (1u << window_width) - 2; i++)
    {
        NX_CRYPTO_EC_POINT_INITIALIZE(&fixed_points -> nx_crypto_ec_fixed_points_array[i],
                                      NX_CRYPTO_EC_POINT_AFFINE, scratch_ptr,
                                      g -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size);
    }
    for (i = 0; i < (1u << window_width) - 1; i++)
    {
        NX_CRYPTO_EC_POINT_INITIALIZE(&fixed_points -> nx_crypto_ec_fixed_points_array_2e[i],
                                      NX_CRYPTO_EC_POINT_AFFINE, scratch_ptr,
                                      g -> nx_crypto_ec_point_x.nx_crypto_huge_buffer_size);
    }

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&d, scratch_ptr,
                                     (((fixed_points -> nx_crypto_ec_fixed_points_d >> 3) + 4) & (ULONG) ~3));

    /* Calculate [a(w-1),...a(0)]G */
    /* First calculate 2 ^ d * G */
    array = fixed_points -> nx_crypto_ec_fixed_points_array;
    d.nx_crypto_huge_number_size = (fixed_points -> nx_crypto_ec_fixed_points_d >>
                                    (HN_SIZE_SHIFT + 3)) + 1;
    NX_CRYPTO_MEMSET(d.nx_crypto_huge_number_data, 0, d.nx_crypto_huge_number_size << HN_SIZE_SHIFT);
    d.nx_crypto_huge_number_data[d.nx_crypto_huge_number_size - 1] =
        (HN_UBASE)(1 << (fixed_points -> nx_crypto_ec_fixed_points_d & (NX_CRYPTO_HUGE_NUMBER_BITS - 1)));
    curve -> nx_crypto_ec_multiple(curve, g, &d, &array[0], scratch_ptr);

    /* Calculate rest of [a(w-1),...a(0)]G */
    for (i = 1; i < window_width; i++)
    {
        offset = (UINT)(1 << i);
        if (i > 1)
        {
            curve -> nx_crypto_ec_multiple(curve, &array[(1 << (i - 1)) - 2], &d,
                                           &array[offset - 2], scratch_ptr);
        }
        NX_CRYPTO_HUGE_NUMBER_COPY(&array[offset - 1].nx_crypto_ec_point_x,
                                   &array[offset - 2].nx_crypto_ec_point_x);
        NX_CRYPTO_HUGE_NUMBER_COPY(&array[offset - 1].nx_crypto_ec_point_y,
                                   &array[offset - 2].nx_crypto_ec_point_y);
        curve -> nx_crypto_ec_add(curve, &array[offset - 1], g, scratch_ptr);

        for (j = 1; j < offset - 1; j++)
        {
            NX_CRYPTO_HUGE_NUMBER_COPY(&array[offset + j - 1].nx_crypto_ec_point_x,
                                       &array[offset - 2].nx_crypto_ec_point_x);
            NX_CRYPTO_HUGE_NUMBER_COPY(&array[offset + j - 1].nx_crypto_ec_point_y,
                                       &array[offset - 2].nx_crypto_ec_point_y);
            curve -> nx_crypto_ec_add(curve, &array[offset + j - 1], &array[j - 1], scratch_ptr);
        }
    }

    /* 2^e[a(w-1),...a(0)]G */
    /* First calculate 2 ^ e * G and 2 ^ e * 2 ^ d * G */
    array = fixed_points -> nx_crypto_ec_fixed_points_array_2e;
    d.nx_crypto_huge_number_size = (fixed_points -> nx_crypto_ec_fixed_points_e >>
                                    (HN_SIZE_SHIFT + 3)) + 1;
    NX_CRYPTO_MEMSET(d.nx_crypto_huge_number_data, 0, d.nx_crypto_huge_number_size << HN_SIZE_SHIFT);
    d.nx_crypto_huge_number_data[d.nx_crypto_huge_number_size - 1] =
        (HN_UBASE)(1 << (fixed_points -> nx_crypto_ec_fixed_points_e & (NX_CRYPTO_HUGE_NUMBER_BITS - 1)));
    curve -> nx_crypto_ec_multiple(curve, g, &d, &array[0], scratch_ptr);
    d.nx_crypto_huge_number_size = (fixed_points -> nx_crypto_ec_fixed_points_d >>
                                    (HN_SIZE_SHIFT + 3)) + 1;
    NX_CRYPTO_MEMSET(d.nx_crypto_huge_number_data, 0, d.nx_crypto_huge_number_size << HN_SIZE_SHIFT);
    d.nx_crypto_huge_number_data[d.nx_crypto_huge_number_size - 1] =
        (HN_UBASE)(1 << (fixed_points -> nx_crypto_ec_fixed_points_d & (NX_CRYPTO_HUGE_NUMBER_BITS - 1)));
    curve -> nx_crypto_ec_multiple(curve, &array[0], &d, &array[1], scratch_ptr);

    /* Calculate rest of 2^e[a(w-1),...a(0)]G */
    for (i = 1; i < window_width; i++)
    {
        offset = (UINT)(1 << i);
        if (i > 1)
        {
            curve -> nx_crypto_ec_multiple(curve, &array[(1 << (i - 1)) - 1], &d,
                                           &array[offset - 1], scratch_ptr);
        }

        for (j = 0; j < offset - 1; j++)
        {
            NX_CRYPTO_HUGE_NUMBER_COPY(&array[offset + j].nx_crypto_ec_point_x,
                                       &array[offset - 1].nx_crypto_ec_point_x);
            NX_CRYPTO_HUGE_NUMBER_COPY(&array[offset + j].nx_crypto_ec_point_y,
                                       &array[offset - 1].nx_crypto_ec_point_y);
            curve -> nx_crypto_ec_add(curve, &array[offset + j], &array[j], scratch_ptr);
        }
    }

    curve -> nx_crypto_ec_fixed_points = fixed_points;

    *scratch_pptr = scratch_ptr;
}
#endif
/* NOTE: This function should be run when NX_CRYPTO_HUGE_NUMBER_BITS is 32. */
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_fixed_output                          PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function outputs the fixed points. So the fixed points can be  */
/*    stored outside the RAM or avoid computation every time.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    output                                Callback function for output  */
/*    tab                                   String of TAB                 */
/*    line_ending                           String of line ending         */
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
#ifndef NX_CRYPTO_SELF_TEST
NX_CRYPTO_KEEP VOID _nx_crypto_ec_fixed_output(NX_CRYPTO_EC *curve,
                                               INT (*output)(const CHAR *format, ...),
                                               const CHAR *tab,
                                               const CHAR *line_ending)
{
UINT                       i, j, k;
UINT                       array_index;
UINT                       array_size;
NX_CRYPTO_EC_FIXED_POINTS *fixed_points;
NX_CRYPTO_EC_POINT        *points;
NX_CRYPTO_HUGE_NUMBER     *value;
HN_UBASE                  *buffer;
UINT                       window_width;
const CHAR                *coordinate_name[] = {"x", "y"};
const CHAR                *array_name[] = {"", "_2e"};

#if (NX_CRYPTO_HUGE_NUMBER_BITS != 32)
    return;
#endif /* (NX_CRYPTO_HUGE_NUMBER_BITS != 32) */

    fixed_points = curve -> nx_crypto_ec_fixed_points;
    if (fixed_points == NX_CRYPTO_NULL)
    {

        /* Fixed points are not pre-computed. */
        return;
    }

    window_width = fixed_points -> nx_crypto_ec_fixed_points_window_width;

    /* Output data in buffer of each point. */

    for (array_index = 0; array_index < 2; array_index++)
    {
        if (array_index == 0)
        {
            array_size = (1u << window_width) - 2;
            points = fixed_points -> nx_crypto_ec_fixed_points_array;
        }
        else
        {
            array_size = (1u << window_width) - 1;
            points = fixed_points -> nx_crypto_ec_fixed_points_array_2e;
        }
        output("static NX_CRYPTO_CONST HN_UBASE %s_fixed_points%s_data[][%u >> HN_SIZE_SHIFT] =%s",
               curve -> nx_crypto_ec_name,
               array_name[array_index],
               curve -> nx_crypto_ec_g.nx_crypto_ec_point_x.nx_crypto_huge_buffer_size,
               line_ending);
        output("{%s", line_ending);
        for (i = 0; i < array_size; i++)
        {
            for (j = 0; j < 2; j++)
            {
                if (array_index == 0)
                {
                    output("%s%s/* %uG.%s */%s", line_ending, tab, i + 2, coordinate_name[j], line_ending);
                }
                else
                {
                    output("%s%s/* 2^e * %uG.%s */%s", line_ending, tab, i + 1, coordinate_name[j], line_ending);
                }
                output("%s{%s", tab, line_ending);

                if (j == 0)
                {
                    value = &points[i].nx_crypto_ec_point_x;
                }
                else
                {
                    value = &points[i].nx_crypto_ec_point_y;
                }
                buffer = (HN_UBASE *)value -> nx_crypto_huge_number_data;
                for (k = 0; k < ((value -> nx_crypto_huge_buffer_size) >> HN_SIZE_SHIFT); k++)
                {
                    if (((k + 1) & 0x1) == 1)
                    {
                        output("%s%s", tab, tab);
                    }

                    if (k < value -> nx_crypto_huge_number_size)
                    {
                        output("HN_ULONG_TO_UBASE(0x%08X)", buffer[k]);
                    }
                    else
                    {
                        output("HN_ULONG_TO_UBASE(0x00000000)");
                    }

                    if (k != ((value -> nx_crypto_huge_buffer_size) >> HN_SIZE_SHIFT) - 1)
                    {
                        output(", ");
                    }
                    if (((k + 1) & 0x1) == 0)
                    {
                        output("%s", line_ending);
                    }
                }

                if ((j == 1) && (i == (array_size - 1)))
                {
                    output("%s%s}%s", line_ending, tab, line_ending);
                }
                else
                {
                    output("%s%s},%s", line_ending, tab, line_ending);
                }
            }
        }
        output("};%s", line_ending);
    }

    /* Output each point structure. */
    for (array_index = 0; array_index < 2; array_index++)
    {
        if (array_index == 0)
        {
            array_size = (1u << window_width) - 2;
            points = fixed_points -> nx_crypto_ec_fixed_points_array;
        }
        else
        {
            array_size = (1u << window_width) - 1;
            points = fixed_points -> nx_crypto_ec_fixed_points_array_2e;
        }
        output("static NX_CRYPTO_CONST NX_CRYPTO_EC_POINT %s_fixed_points%s_array[] =%s",
               curve -> nx_crypto_ec_name,
               array_name[array_index],
               line_ending);
        output("{%s", line_ending);
        for (i = 0; i < array_size; i++)
        {
            if (array_index == 0)
            {
                output("%s%s/* %uG */%s", line_ending, tab, i + 2, line_ending);
            }
            else
            {
                output("%s%s/* 2^e * %uG */%s", line_ending, tab, i + 1, line_ending);
            }

            output("%s{%s", tab, line_ending);
            output("%s%sNX_CRYPTO_EC_POINT_AFFINE,%s", tab, tab, line_ending);

            /* Output X and Y */
            for (j = 0; j < 2; j++)
            {
                if (j == 0)
                {
                    value = &points[i].nx_crypto_ec_point_x;
                }
                else
                {
                    value = &points[i].nx_crypto_ec_point_y;
                }
                output("%s%s{%s", tab, tab, line_ending);
                output("%s%s%s(HN_UBASE *)&%s_fixed_points%s_data[%u],%s",
                       tab, tab, tab,
                       curve -> nx_crypto_ec_name, array_name[array_index], (i << 1) + j,
                       line_ending);
                output("%s%s%s%u >> HN_SIZE_SHIFT, ",
                       tab, tab, tab,
                       value -> nx_crypto_huge_number_size << HN_SIZE_SHIFT);
                output("%u, (UINT)NX_CRYPTO_FALSE%s",
                       value -> nx_crypto_huge_buffer_size,
                       line_ending);
                output("%s%s},%s", tab, tab, line_ending);
            }

            /* Output Z which is not used. */
            output("%s%s{(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}%s", tab, tab, line_ending);

            output("%s}", tab);
            if (i != (array_size - 1))
            {
                output(",%s", line_ending);
            }
            else
            {
                output("%s", line_ending);
            }
        }
        output("};%s", line_ending);
    }

    /* Output structure for fixed points. */
    output("%s%sNX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_%s_fixed_points =%s",
           line_ending, line_ending, curve -> nx_crypto_ec_name, line_ending);
    output("{%s", line_ending);
    output("%s%uu, %uu, %uu, %uu,%s", tab,
           fixed_points -> nx_crypto_ec_fixed_points_window_width,
           fixed_points -> nx_crypto_ec_fixed_points_bits,
           fixed_points -> nx_crypto_ec_fixed_points_d,
           fixed_points -> nx_crypto_ec_fixed_points_e,
           line_ending);
    output("%s(NX_CRYPTO_EC_POINT *)%s_fixed_points_array,%s",
           tab, curve -> nx_crypto_ec_name, line_ending);
    output("%s(NX_CRYPTO_EC_POINT *)%s_fixed_points_2e_array%s",
           tab, curve -> nx_crypto_ec_name, line_ending);
    output("};%s", line_ending);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_get_named_curve                       PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the named curve by ID.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    curve_id                              Curve ID                      */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), updated  */
/*                                            constants, resulting        */
/*                                            in version 6.1              */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
#ifndef NX_CRYPTO_SELF_TEST
NX_CRYPTO_KEEP UINT _nx_crypto_ec_get_named_curve(NX_CRYPTO_EC **curve, UINT curve_id)
{
UINT i;

    for (i = 0; i < sizeof(_nx_crypto_ec_named_curves) / sizeof(NX_CRYPTO_EC *); i++)
    {
        if (curve_id == _nx_crypto_ec_named_curves[i] -> nx_crypto_ec_id)
        {
            *curve = (NX_CRYPTO_EC *)_nx_crypto_ec_named_curves[i];
            return(NX_CRYPTO_SUCCESS);
        }
    }

    *curve = NX_CRYPTO_NULL;
    return(NX_CRYTPO_MISSING_ECC_CURVE);
}
#endif
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_secp192r1_operation            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the secp192r1 curve.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ec_secp192r1_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    *((NX_CRYPTO_EC **)output) = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp192r1;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_secp224r1_operation            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the secp224r1 curve.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ec_secp224r1_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    *((NX_CRYPTO_EC **)output) = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp224r1;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_secp256r1_operation            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the secp256r1 curve.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ec_secp256r1_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    *((NX_CRYPTO_EC **)output) = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp256r1;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_secp384r1_operation            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the secp384r1 curve.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ec_secp384r1_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    *((NX_CRYPTO_EC **)output) = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp384r1;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_secp521r1_operation            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the secp521r1 curve.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ec_secp521r1_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    *((NX_CRYPTO_EC **)output) = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp521r1;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_validate_public_key                   PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function validates the public key by ensuring that the point   */
/*    is a valid point on the elliptic curve. This function supports prime*/
/*    field curves only.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    public_key                            Public key to be verified     */
/*    chosen_curve                          Curve used by the key         */
/*    partial                               Perform partial validation    */
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
/*    _nx_crypto_huge_number_compare        Compare huge number           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Timothy Stapko           Initial Version 6.1           */
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
#ifndef NX_CRYPTO_ECC_DISABLE_KEY_VALIDATION
UINT _nx_crypto_ec_validate_public_key(NX_CRYPTO_EC_POINT *public_key,
                                       NX_CRYPTO_EC *chosen_curve,
                                       UINT partial,
                                       HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER temp;
NX_CRYPTO_HUGE_NUMBER right;
UINT                  compare_value;
UINT                  buffer_size = chosen_curve -> nx_crypto_ec_n.nx_crypto_huge_buffer_size;
HN_UBASE             *scratch2 = scratch;

    NX_CRYPTO_PARAMETER_NOT_USED(partial);

    /* 1. Verify Q is not the point at infinity. */ 
    if(_nx_crypto_ec_point_is_infinite(public_key))
    {
        return(NX_CRYPTO_INVALID_KEY);
    }

    /* 2. Verify that xQ and yQ are integers in the interval [0, p-1].
         (Ensures that each coordinate of the public key has the unique correct representation of
         an element in the underlying field.) */
    compare_value = _nx_crypto_huge_number_compare(&public_key -> nx_crypto_ec_point_x, &chosen_curve -> nx_crypto_ec_field.fp);
    if (compare_value != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        return(NX_CRYPTO_INVALID_KEY);
    }

    compare_value = _nx_crypto_huge_number_compare(&public_key -> nx_crypto_ec_point_y, &chosen_curve -> nx_crypto_ec_field.fp);
    if (compare_value != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        return(NX_CRYPTO_INVALID_KEY);
    }

    if (public_key -> nx_crypto_ec_point_x.nx_crypto_huge_number_is_negative ||
        public_key -> nx_crypto_ec_point_y.nx_crypto_huge_number_is_negative)
    {
        return(NX_CRYPTO_INVALID_KEY);
    }

    /* 3. Verify that (yQ)^2 = (xQ)^3 + axQ + b in GF(p) , where the arithmetic is
          performed modulo p.
          (xQ)^3 + axQ + b = ((xQ)^2 + a)xQ + b
          (This step is to ensure that the public key is on the correct elliptic curve.)
    */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch2, buffer_size * 2);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&right, scratch2, buffer_size * 2);

    _nx_crypto_huge_number_multiply(&public_key -> nx_crypto_ec_point_x, &public_key -> nx_crypto_ec_point_x, &temp);
    _nx_crypto_huge_number_modulus(&temp, &chosen_curve -> nx_crypto_ec_field.fp);
    _nx_crypto_huge_number_add_unsigned(&temp, &chosen_curve -> nx_crypto_ec_a);
    _nx_crypto_huge_number_modulus(&temp, &chosen_curve -> nx_crypto_ec_field.fp);

    _nx_crypto_huge_number_multiply(&temp, &public_key -> nx_crypto_ec_point_x, &right);
    _nx_crypto_huge_number_modulus(&right, &chosen_curve -> nx_crypto_ec_field.fp);

    _nx_crypto_huge_number_add_unsigned(&right, &chosen_curve -> nx_crypto_ec_b);
    _nx_crypto_huge_number_modulus(&right, &chosen_curve -> nx_crypto_ec_field.fp);

    _nx_crypto_huge_number_multiply(&public_key -> nx_crypto_ec_point_y, &public_key -> nx_crypto_ec_point_y, &temp);
    _nx_crypto_huge_number_modulus(&temp, &chosen_curve -> nx_crypto_ec_field.fp);

    compare_value = _nx_crypto_huge_number_compare(&temp, &right);
    if (compare_value != NX_CRYPTO_HUGE_NUMBER_EQUAL)
    {
        return(NX_CRYPTO_INVALID_KEY);
    }

    /*  4. Verify that nQ = O.
        (This step is to ensure that the public key has the correct order. Along with the
        verification in step 1, ensures that the public key is in the correct range in the correct EC
        subgroup; that is, it is in the correct EC subgroup and is not the identity element O.)
    */
    /* Removed this validation as h is 1 for all the software supported curves and nQ = O
        is implied by the checks in step 2 and 3.
    if (!partial)
    {
        NX_CRYPTO_EC_POINT_INITIALIZE(&pt, NX_CRYPTO_EC_POINT_AFFINE, scratch, buffer_size);
        chosen_curve -> nx_crypto_ec_multiple(chosen_curve, public_key, &chosen_curve -> nx_crypto_ec_n, &pt, scratch);
        if (!_nx_crypto_ec_point_is_infinite(&pt))
        {
            return(NX_CRYPTO_INVALID_KEY);
        }
    }
    */

    return(NX_CRYPTO_SUCCESS);
}
#endif /* NX_CRYPTO_ECC_DISABLE_KEY_VALIDATION */

#ifdef NX_CRYPTO_ENABLE_CURVE25519_448

/* x25519 */
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_x25519_p[] =
{

    /* p = 7fffffff ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff ffffffed */
    HN_ULONG_TO_UBASE(0xFFFFFFED), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0x7FFFFFFF),
};

static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_x25519_gx[] =
{

    /* U(P) = 9 */
    HN_ULONG_TO_UBASE(0x00000009)
};


static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_x25519_h[] =
{

    /* h = 08 */
    HN_ULONG_TO_UBASE(0x00000008)
};

/* x448 */
static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_x448_p[] =
{

    /* p = ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff fffffffe ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff */
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFE),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
    HN_ULONG_TO_UBASE(0xFFFFFFFF), HN_ULONG_TO_UBASE(0xFFFFFFFF),
};

static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_x448_gx[] =
{

    /* U(P) = 5 */
    HN_ULONG_TO_UBASE(0x00000005)
};


static NX_CRYPTO_CONST HN_UBASE _nx_crypto_ec_x448_h[] =
{

    /* h = 04 */
    HN_ULONG_TO_UBASE(0x00000004)
};


NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_x25519 =
{
    "x25519",
    NX_CRYPTO_EC_X25519,
    5,
    255,
    {
        .fp =
        {
            (HN_UBASE *)_nx_crypto_ec_x25519_p,
            sizeof(_nx_crypto_ec_x25519_p) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_x25519_p),
            (UINT)NX_CRYPTO_FALSE
        }
    },
    {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u},
    {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u},
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)_nx_crypto_ec_x25519_gx,
            sizeof(_nx_crypto_ec_x25519_gx) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_x25519_gx),
            (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u},
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },
    {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u},
    {
        (HN_UBASE *)_nx_crypto_ec_x25519_h,
        sizeof(_nx_crypto_ec_x25519_h) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_x25519_h),
        (UINT)NX_CRYPTO_FALSE
    },
    (NX_CRYPTO_EC_FIXED_POINTS *)NX_CRYPTO_NULL,
    NX_CRYPTO_NULL,
    NX_CRYPTO_NULL,
    _nx_crypto_ec_x25519_448_multiple,
    NX_CRYPTO_NULL
};

NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_x448 =
{
    "x448",
    NX_CRYPTO_EC_X448,
    5,
    448,
    {
        .fp =
        {
            (HN_UBASE *)_nx_crypto_ec_x448_p,
            sizeof(_nx_crypto_ec_x448_p) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_x448_p),
            (UINT)NX_CRYPTO_FALSE
        }
    },
    {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u},
    {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u},
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)_nx_crypto_ec_x448_gx,
            sizeof(_nx_crypto_ec_x448_gx) >> HN_SIZE_SHIFT,
            sizeof(_nx_crypto_ec_x448_gx),
            (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u},
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },
    {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u},
    {
        (HN_UBASE *)_nx_crypto_ec_x448_h,
        sizeof(_nx_crypto_ec_x448_h) >> HN_SIZE_SHIFT,
        sizeof(_nx_crypto_ec_x448_h),
        (UINT)NX_CRYPTO_FALSE
    },
    (NX_CRYPTO_EC_FIXED_POINTS *)NX_CRYPTO_NULL,
    NX_CRYPTO_NULL,
    NX_CRYPTO_NULL,
    _nx_crypto_ec_x25519_448_multiple,
    NX_CRYPTO_NULL
};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_x25519_448_multiple                   PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the scalar multiplication on X25519 and X448 */
/*    curves.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    u                                     U coordinate                  */
/*    k                                     Scalar                        */
/*    r                                     Result                        */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_EC_POINT_INITIALIZE         Initialize EC point           */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    _nx_crypto_ec_cswap                   Swap two huge numbers         */
/*    _nx_crypto_huge_number_add            Addition for huge numbers     */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_multiply       Multiply two huge numbers     */
/*    _nx_crypto_huge_number_square         Compute the square of a value */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
NX_CRYPTO_KEEP VOID _nx_crypto_ec_x25519_448_multiple(NX_CRYPTO_EC* curve,
                                                      NX_CRYPTO_EC_POINT* u,
                                                      NX_CRYPTO_HUGE_NUMBER* k,
                                                      NX_CRYPTO_EC_POINT* r,
                                                      HN_UBASE* scratch)
{
NX_CRYPTO_HUGE_NUMBER k_2;
NX_CRYPTO_HUGE_NUMBER x_1;
NX_CRYPTO_HUGE_NUMBER x_2;
NX_CRYPTO_HUGE_NUMBER z_2;
NX_CRYPTO_HUGE_NUMBER x_3;
NX_CRYPTO_HUGE_NUMBER z_3;
NX_CRYPTO_HUGE_NUMBER t_1;
NX_CRYPTO_HUGE_NUMBER t_2;
INT t;
UINT k_t;
UINT swap;
UINT clen;
ULONG a24;
HN_UBASE* k_buf;

    clen = (curve -> nx_crypto_ec_bits + 7) >> 3;

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&k_2, scratch, clen);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&x_1, scratch, clen);
    clen = clen << 1;
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&x_2, scratch, clen);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&z_2, scratch, clen);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&x_3, scratch, clen);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&z_3, scratch, clen);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&t_1, scratch, clen);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&t_2, scratch, clen);

    NX_CRYPTO_HUGE_NUMBER_COPY(&k_2, k);
    NX_CRYPTO_HUGE_NUMBER_COPY(&x_1, &u -> nx_crypto_ec_point_x);

    if (curve -> nx_crypto_ec_id == NX_CRYPTO_EC_X25519)
    {
        /* x25519 */
        x_1.nx_crypto_huge_number_data[7] &= 0x7FFFFFFF;

        k_buf = k_2.nx_crypto_huge_number_data;

        k_buf[0] &= 0xFFFFFFF8;
        k_buf[7] &= 0x7FFFFFFF;
        k_buf[7] |= 0x40000000;
        a24 = 121665;
    }
    else
    {
        /* x448 */

        k_buf = k_2.nx_crypto_huge_number_data;

        k_buf[0] &= 0xFFFFFFFC;
        k_buf[13] |= 0x80000000;
        a24 = 39081;
    }

    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&x_2, 1);
    NX_CRYPTO_HUGE_NUMBER_COPY(&x_3, &x_1);
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&z_3, 1);

    swap = 0;

    for (t = (INT)(curve -> nx_crypto_ec_bits - 1); t >= 0; t--)
    {
        k_t = 1 & (k_buf[t >> 5] >> (t & 31));
        swap ^= k_t;
        _nx_crypto_ec_cswap(swap, &x_2, &x_3);
        _nx_crypto_ec_cswap(swap, &z_2, &z_3);
        swap = k_t;

        /* A = x_2 + z_2 */
        NX_CRYPTO_HUGE_NUMBER_COPY(&t_1, &x_2);
        _nx_crypto_huge_number_add(&t_1, &z_2);
        _nx_crypto_huge_number_modulus(&t_1, &curve->nx_crypto_ec_field.fp);

        /* B = x_2 - z_2 */
        _nx_crypto_huge_number_subtract(&x_2, &z_2);

        /* C = x_3 + z_3 */
        NX_CRYPTO_HUGE_NUMBER_COPY(&t_2, &x_3);
        _nx_crypto_huge_number_add(&t_2, &z_3);

        /* D = x_3 - z_3 */
        _nx_crypto_huge_number_subtract(&x_3, &z_3);

        /* DA = D * A */
        _nx_crypto_huge_number_multiply(&x_3, &t_1, &z_3);
        _nx_crypto_huge_number_modulus(&z_3, &curve -> nx_crypto_ec_field.fp);

        /* CB = C * B */
        _nx_crypto_huge_number_multiply(&x_2, &t_2, &z_2);
        _nx_crypto_huge_number_modulus(&z_2, &curve -> nx_crypto_ec_field.fp);

        /* t_2 = DA + CB */
        NX_CRYPTO_HUGE_NUMBER_COPY(&t_2, &z_2);
        _nx_crypto_huge_number_add(&t_2, &z_3);
        _nx_crypto_huge_number_modulus(&t_2, &curve->nx_crypto_ec_field.fp);

        /* z_3 = DA - CB */
        _nx_crypto_huge_number_subtract(&z_3, &z_2);

        /* x_3 = (DA + CB)^2 */
        _nx_crypto_huge_number_square(&t_2, &x_3);
        _nx_crypto_huge_number_modulus(&x_3, &curve -> nx_crypto_ec_field.fp);

        /* t_2 = (DA - CB)^2 */
        _nx_crypto_huge_number_square(&z_3, &t_2);
        _nx_crypto_huge_number_modulus(&t_2, &curve -> nx_crypto_ec_field.fp);

        /* z_3 = x_1 * (DA - CB)^2 */
        _nx_crypto_huge_number_multiply(&t_2, &x_1, &z_3);
        _nx_crypto_huge_number_modulus(&z_3, &curve -> nx_crypto_ec_field.fp);

        /* AA = A^2 */
        _nx_crypto_huge_number_square(&t_1, &z_2);
        _nx_crypto_huge_number_modulus(&z_2, &curve -> nx_crypto_ec_field.fp);

        /* BB = B^2 */
        _nx_crypto_huge_number_square(&x_2, &t_1);
        _nx_crypto_huge_number_modulus(&t_1, &curve -> nx_crypto_ec_field.fp);

        /* E = AA - BB */
        NX_CRYPTO_HUGE_NUMBER_COPY(&t_2, &z_2);
        _nx_crypto_huge_number_subtract(&t_2, &t_1);

        /* x_2 = AA * BB */
        _nx_crypto_huge_number_multiply(&z_2, &t_1, &x_2);
        _nx_crypto_huge_number_modulus(&x_2, &curve -> nx_crypto_ec_field.fp);

        /* z_2 = E * (AA + a24 * E) */
        _nx_crypto_huge_number_multiply_digit(&t_2, a24, &t_1);

        _nx_crypto_huge_number_add(&t_1, &z_2);
        _nx_crypto_huge_number_modulus(&t_1, &curve -> nx_crypto_ec_field.fp);

        _nx_crypto_huge_number_multiply(&t_1, &t_2, &z_2);
        _nx_crypto_huge_number_modulus(&z_2, &curve -> nx_crypto_ec_field.fp);

    }

    _nx_crypto_ec_cswap(swap, &x_2, &x_3);
    _nx_crypto_ec_cswap(swap, &z_2, &z_3);

    /* Return x_2 * (z_2^(p - 2)) = x_2 * (z_2^-1) */
    _nx_crypto_huge_number_inverse_modulus_prime(&z_2, &curve -> nx_crypto_ec_field.fp, &t_1, scratch);
    _nx_crypto_huge_number_multiply(&x_2, &t_1, &r -> nx_crypto_ec_point_x);
    _nx_crypto_huge_number_modulus(&r -> nx_crypto_ec_point_x, &curve -> nx_crypto_ec_field.fp);

}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_cswap                                 PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs conditional swap of two huge number in       */
/*    constant time.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    swap                                  Swap condition                */
/*    h1                                    First huge number             */
/*    h2                                    Second huge number            */
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
/*    _nx_crypto_ec_x25519_448_multiple     Scalar multiplication         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_ec_cswap(UINT swap, NX_CRYPTO_HUGE_NUMBER *h1, NX_CRYPTO_HUGE_NUMBER *h2)
{
UINT i;
HN_UBASE hswap = (HN_UBASE)(0 - swap);
HN_UBASE dummy;


    for (i = 0; i < (h1 -> nx_crypto_huge_buffer_size >> HN_SIZE_SHIFT); i++)
    {
        dummy = hswap & (h1 -> nx_crypto_huge_number_data[i] ^ h2 -> nx_crypto_huge_number_data[i]);
        h1 -> nx_crypto_huge_number_data[i] = h1 -> nx_crypto_huge_number_data[i] ^ dummy;
        h2 -> nx_crypto_huge_number_data[i] = h2 -> nx_crypto_huge_number_data[i] ^ dummy;
    }

    i = hswap & (h1 -> nx_crypto_huge_number_size ^ h2 -> nx_crypto_huge_number_size);
    h1 -> nx_crypto_huge_number_size ^= i;
    h2 -> nx_crypto_huge_number_size ^= i;

    i = hswap & (h1 -> nx_crypto_huge_number_is_negative ^ h2 -> nx_crypto_huge_number_is_negative);
    h1 -> nx_crypto_huge_number_is_negative ^= i;
    h2 -> nx_crypto_huge_number_is_negative ^= i;

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_x25519_operation               PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the x25519 curve.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_method_ec_x25519_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    *((NX_CRYPTO_EC **)output) = (NX_CRYPTO_EC *)&_nx_crypto_ec_x25519;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_x448_operation                 PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the x448 curve.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
/*  07-29-2022     Yuxin Zhou               Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_method_ec_x448_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    *((NX_CRYPTO_EC **)output) = (NX_CRYPTO_EC *)&_nx_crypto_ec_x448;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_key_pair_generation_x25519_448        PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates an elliptic curve key pair.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    curve                                 Pointer to curve              */
/*    g                                     Base point g                  */
/*    private_key                           Private key generated         */
/*    public_key                            Public key generated          */
/*    scratch                               Pointer to scratch buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_RBG                         Generate random huge number   */
/*    [nx_crypto_ec_multiple]               Perform multiplication for EC */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecdh_setup_x25519_448      Setup ECDH local key pair     */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ec_key_pair_generation_x25519_448(NX_CRYPTO_EC *curve,
                                                                 NX_CRYPTO_EC_POINT *g,
                                                                 NX_CRYPTO_HUGE_NUMBER *private_key,
                                                                 NX_CRYPTO_EC_POINT *public_key,
                                                                 HN_UBASE *scratch)
{
UINT status;
UINT buffer_size = (curve -> nx_crypto_ec_bits + 7) >> 3;

    /* Get random number with specified length. */
    status = NX_CRYPTO_RBG(buffer_size << 3, (UCHAR*)private_key -> nx_crypto_huge_number_data);

    if (status)
    {
        return(status);
    }

    private_key -> nx_crypto_huge_number_size = buffer_size >> HN_SIZE_SHIFT;

    /* Q = dG */
    curve -> nx_crypto_ec_multiple(curve, g, private_key, public_key, scratch);

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ec_extract_fixed_size_le                 PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts huge number in little-endian order to a      */
/*    buffer with fixed size.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                                Huge number                   */
/*    byte_stream                           Output buffer                 */
/*    byte_stream_size                      Size of output buffer         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMSET                      Copy the number data memory   */
/*    NX_CRYPTO_MEMSET                      Set the memory                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecdh_setup_x25519_448      Setup ECDH local key pair     */
/*    _nx_crypto_ecdh_compute_secret_x25519_448                           */
/*                                          Compute ECDH shared secret    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_ec_extract_fixed_size_le(NX_CRYPTO_HUGE_NUMBER *number,
                                                        UCHAR *byte_stream, UINT byte_stream_size)
{
UINT number_size = number -> nx_crypto_huge_number_size << HN_SIZE_SHIFT;

    if (number_size > byte_stream_size)
    {

        /* User byte stream buffer too small. */
        return(NX_CRYPTO_SIZE_ERROR);
    }

    NX_CRYPTO_MEMCPY(byte_stream, number -> nx_crypto_huge_number_data, number_size); /* Use case of memcpy is verified. */

    if (byte_stream_size > number_size)
    {
        NX_CRYPTO_MEMSET(&byte_stream[number_size], 0, byte_stream_size - number_size);
    }

    return(NX_CRYPTO_SUCCESS);
}
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
