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
/**   Elliptical Curve Cryptography                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#include "nx_crypto_ec.h"
static NX_CRYPTO_CONST HN_UBASE           secp224r1_fixed_points_data[][28 >> HN_SIZE_SHIFT] =
{

    /* 2G.x */
    {
        HN_ULONG_TO_UBASE(0x666EBBE9), HN_ULONG_TO_UBASE(0x5EFD9675),
        HN_ULONG_TO_UBASE(0x664D40CE), HN_ULONG_TO_UBASE(0x2A43BCA7),
        HN_ULONG_TO_UBASE(0x42DF8D8A), HN_ULONG_TO_UBASE(0xF99BC522),
        HN_ULONG_TO_UBASE(0x1F49BBB0)
    },

    /* 2G.y */
    {
        HN_ULONG_TO_UBASE(0x92DC9C43), HN_ULONG_TO_UBASE(0x6229E0B8),
        HN_ULONG_TO_UBASE(0x608436E6), HN_ULONG_TO_UBASE(0x10D0ECE8),
        HN_ULONG_TO_UBASE(0x858F1853), HN_ULONG_TO_UBASE(0xB8D321DC),
        HN_ULONG_TO_UBASE(0x9812DD4E)
    },

    /* 3G.x */
    {
        HN_ULONG_TO_UBASE(0x8D5D8EB8), HN_ULONG_TO_UBASE(0xF16D3E67),
        HN_ULONG_TO_UBASE(0xED1CB362), HN_ULONG_TO_UBASE(0x8A3F559E),
        HN_ULONG_TO_UBASE(0xE9A3BBCE), HN_ULONG_TO_UBASE(0xC2A74816),
        HN_ULONG_TO_UBASE(0xEEDCCCD8)
    },

    /* 3G.y */
    {
        HN_ULONG_TO_UBASE(0xED50266D), HN_ULONG_TO_UBASE(0xDFF19F90),
        HN_ULONG_TO_UBASE(0xB4BF65F9), HN_ULONG_TO_UBASE(0xAFECABF2),
        HN_ULONG_TO_UBASE(0x3865468F), HN_ULONG_TO_UBASE(0x910A1731),
        HN_ULONG_TO_UBASE(0x5CB379BA)
    },

    /* 4G.x */
    {
        HN_ULONG_TO_UBASE(0x6CAB26E3), HN_ULONG_TO_UBASE(0xA0064196),
        HN_ULONG_TO_UBASE(0x2991FAB0), HN_ULONG_TO_UBASE(0x3A0B91FB),
        HN_ULONG_TO_UBASE(0xEC27A4E1), HN_ULONG_TO_UBASE(0x5F8EBEEF),
        HN_ULONG_TO_UBASE(0x0499AA8A)
    },

    /* 4G.y */
    {
        HN_ULONG_TO_UBASE(0x7766AF5D), HN_ULONG_TO_UBASE(0x50751040),
        HN_ULONG_TO_UBASE(0x29610D54), HN_ULONG_TO_UBASE(0xF70684D9),
        HN_ULONG_TO_UBASE(0xD77AAE82), HN_ULONG_TO_UBASE(0x338C5B81),
        HN_ULONG_TO_UBASE(0x6916F6D4)
    },

    /* 5G.x */
    {
        HN_ULONG_TO_UBASE(0x3B1F15C6), HN_ULONG_TO_UBASE(0xD4EA95AC),
        HN_ULONG_TO_UBASE(0x00905E82), HN_ULONG_TO_UBASE(0xC8B10860),
        HN_ULONG_TO_UBASE(0x323AE4D1), HN_ULONG_TO_UBASE(0x7685A3DD),
        HN_ULONG_TO_UBASE(0x932B56BE)
    },

    /* 5G.y */
    {
        HN_ULONG_TO_UBASE(0xEA25DBBF), HN_ULONG_TO_UBASE(0xF09EF93D),
        HN_ULONG_TO_UBASE(0x5960F390), HN_ULONG_TO_UBASE(0xA8A74166),
        HN_ULONG_TO_UBASE(0xEC76DBE2), HN_ULONG_TO_UBASE(0x19062AFD),
        HN_ULONG_TO_UBASE(0x523E80F0)
    },

    /* 6G.x */
    {
        HN_ULONG_TO_UBASE(0x26732C73), HN_ULONG_TO_UBASE(0x0F822FDD),
        HN_ULONG_TO_UBASE(0x83531B5D), HN_ULONG_TO_UBASE(0x1BA4A01C),
        HN_ULONG_TO_UBASE(0x3F37347C), HN_ULONG_TO_UBASE(0x84725C36),
        HN_ULONG_TO_UBASE(0xC391B45C)
    },

    /* 6G.y */
    {
        HN_ULONG_TO_UBASE(0xB2D6AD24), HN_ULONG_TO_UBASE(0xECBBD5E1),
        HN_ULONG_TO_UBASE(0xCDE19DFA), HN_ULONG_TO_UBASE(0x2A7FDDFB),
        HN_ULONG_TO_UBASE(0x93DA7E22), HN_ULONG_TO_UBASE(0xEDE244C3),
        HN_ULONG_TO_UBASE(0x1EFB7890)
    },

    /* 7G.x */
    {
        HN_ULONG_TO_UBASE(0xCA217DA1), HN_ULONG_TO_UBASE(0xBB4C9E90),
        HN_ULONG_TO_UBASE(0xECA79159), HN_ULONG_TO_UBASE(0x8B7CD11B),
        HN_ULONG_TO_UBASE(0x8D33C2C9), HN_ULONG_TO_UBASE(0x09F849FF),
        HN_ULONG_TO_UBASE(0x2610B394)
    },

    /* 7G.y */
    {
        HN_ULONG_TO_UBASE(0x2AC64DA0), HN_ULONG_TO_UBASE(0xFB44D135),
        HN_ULONG_TO_UBASE(0x7B2C46B4), HN_ULONG_TO_UBASE(0x3C89CDBB),
        HN_ULONG_TO_UBASE(0x6C079B75), HN_ULONG_TO_UBASE(0x20B11296),
        HN_ULONG_TO_UBASE(0xFE67E4E8)
    },

    /* 8G.x */
    {
        HN_ULONG_TO_UBASE(0x2DF5312D), HN_ULONG_TO_UBASE(0x6EE28CAE),
        HN_ULONG_TO_UBASE(0x61D16F5C), HN_ULONG_TO_UBASE(0x7C4CC71B),
        HN_ULONG_TO_UBASE(0xB7619A3E), HN_ULONG_TO_UBASE(0x899B4779),
        HN_ULONG_TO_UBASE(0x05C73240)
    },

    /* 8G.y */
    {
        HN_ULONG_TO_UBASE(0x82C73E3A), HN_ULONG_TO_UBASE(0xDA9F7F63),
        HN_ULONG_TO_UBASE(0x5165C56B), HN_ULONG_TO_UBASE(0xFD561861),
        HN_ULONG_TO_UBASE(0x1FAB2116), HN_ULONG_TO_UBASE(0xB0839464),
        HN_ULONG_TO_UBASE(0x72855882)
    },

    /* 9G.x */
    {
        HN_ULONG_TO_UBASE(0x2F161C09), HN_ULONG_TO_UBASE(0xB5046918),
        HN_ULONG_TO_UBASE(0x8CA8D00F), HN_ULONG_TO_UBASE(0xA3E074A9),
        HN_ULONG_TO_UBASE(0x9DA93489), HN_ULONG_TO_UBASE(0xFB0C1DB8),
        HN_ULONG_TO_UBASE(0x41C98768)
    },

    /* 9G.y */
    {
        HN_ULONG_TO_UBASE(0xFB32DA81), HN_ULONG_TO_UBASE(0x55E5EA05),
        HN_ULONG_TO_UBASE(0x9FFBCA68), HN_ULONG_TO_UBASE(0x59E63DCE),
        HN_ULONG_TO_UBASE(0xFE2D3FBF), HN_ULONG_TO_UBASE(0x8738A71C),
        HN_ULONG_TO_UBASE(0x0E5E0340)
    },

    /* 10G.x */
    {
        HN_ULONG_TO_UBASE(0x2333E87F), HN_ULONG_TO_UBASE(0xF6DAB22B),
        HN_ULONG_TO_UBASE(0x137A5DD2), HN_ULONG_TO_UBASE(0xBEB84430),
        HN_ULONG_TO_UBASE(0x3AB9F738), HN_ULONG_TO_UBASE(0xC34F24E0),
        HN_ULONG_TO_UBASE(0xCB0C5D0D)
    },

    /* 10G.y */
    {
        HN_ULONG_TO_UBASE(0xF0C8FDA5), HN_ULONG_TO_UBASE(0x44764A7D),
        HN_ULONG_TO_UBASE(0xA5C3FA20), HN_ULONG_TO_UBASE(0xBE50185B),
        HN_ULONG_TO_UBASE(0x81D688BC), HN_ULONG_TO_UBASE(0x89388192),
        HN_ULONG_TO_UBASE(0xC40331DF)
    },

    /* 11G.x */
    {
        HN_ULONG_TO_UBASE(0x796F0F60), HN_ULONG_TO_UBASE(0xA3B89530),
        HN_ULONG_TO_UBASE(0x2BD26909), HN_ULONG_TO_UBASE(0x84DAADE9),
        HN_ULONG_TO_UBASE(0x0C83FB48), HN_ULONG_TO_UBASE(0xA5A9841A),
        HN_ULONG_TO_UBASE(0x1765BF22)
    },

    /* 11G.y */
    {
        HN_ULONG_TO_UBASE(0xE75DB09E), HN_ULONG_TO_UBASE(0x6F772A9E),
        HN_ULONG_TO_UBASE(0x6C67CEC1), HN_ULONG_TO_UBASE(0x4E2F23BC),
        HN_ULONG_TO_UBASE(0x1EDBA8B1), HN_ULONG_TO_UBASE(0x6113694C),
        HN_ULONG_TO_UBASE(0xE2A215D9)
    },

    /* 12G.x */
    {
        HN_ULONG_TO_UBASE(0x9FB5EFB3), HN_ULONG_TO_UBASE(0x52571E50),
        HN_ULONG_TO_UBASE(0x86964105), HN_ULONG_TO_UBASE(0x74FEADE8),
        HN_ULONG_TO_UBASE(0xAE85FADA), HN_ULONG_TO_UBASE(0x3BBDE3C8),
        HN_ULONG_TO_UBASE(0x6C7E4BE8)
    },

    /* 12G.y */
    {
        HN_ULONG_TO_UBASE(0x160F4652), HN_ULONG_TO_UBASE(0x39FF9F51),
        HN_ULONG_TO_UBASE(0xE2495A65), HN_ULONG_TO_UBASE(0x82F4B47C),
        HN_ULONG_TO_UBASE(0x946C53B5), HN_ULONG_TO_UBASE(0xEE9A60A2),
        HN_ULONG_TO_UBASE(0x286D2DB3)
    },

    /* 13G.x */
    {
        HN_ULONG_TO_UBASE(0x081A44AF), HN_ULONG_TO_UBASE(0x6C40BBD5),
        HN_ULONG_TO_UBASE(0x183B1392), HN_ULONG_TO_UBASE(0xF6D00995),
        HN_ULONG_TO_UBASE(0xEFBA6F47), HN_ULONG_TO_UBASE(0xCC0057BC),
        HN_ULONG_TO_UBASE(0x215619E9)
    },

    /* 13G.y */
    {
        HN_ULONG_TO_UBASE(0x3B0DF45E), HN_ULONG_TO_UBASE(0x6F8BC94D),
        HN_ULONG_TO_UBASE(0x54A3694F), HN_ULONG_TO_UBASE(0xE8B5F11C),
        HN_ULONG_TO_UBASE(0x31B93CDF), HN_ULONG_TO_UBASE(0x982DB986),
        HN_ULONG_TO_UBASE(0xE7E3F4B0)
    },

    /* 14G.x */
    {
        HN_ULONG_TO_UBASE(0xAB3E1C7B), HN_ULONG_TO_UBASE(0xD8B17048),
        HN_ULONG_TO_UBASE(0xF36FF8A1), HN_ULONG_TO_UBASE(0xD2C6AC38),
        HN_ULONG_TO_UBASE(0x29819435), HN_ULONG_TO_UBASE(0x4C07E91C),
        HN_ULONG_TO_UBASE(0xC813132F)
    },

    /* 14G.y */
    {
        HN_ULONG_TO_UBASE(0x5503B11F), HN_ULONG_TO_UBASE(0xEA289142),
        HN_ULONG_TO_UBASE(0x1030579F), HN_ULONG_TO_UBASE(0x96740878),
        HN_ULONG_TO_UBASE(0x426BA5CC), HN_ULONG_TO_UBASE(0x8562BCF5),
        HN_ULONG_TO_UBASE(0x1E28EBF1)
    },

    /* 15G.x */
    {
        HN_ULONG_TO_UBASE(0x7CC864EB), HN_ULONG_TO_UBASE(0x4C9F3199),
        HN_ULONG_TO_UBASE(0x91D28B5E), HN_ULONG_TO_UBASE(0xA97306CD),
        HN_ULONG_TO_UBASE(0x17036691), HN_ULONG_TO_UBASE(0x497C58FF),
        HN_ULONG_TO_UBASE(0xF1AEF351)
    },

    /* 15G.y */
    {
        HN_ULONG_TO_UBASE(0x600564FF), HN_ULONG_TO_UBASE(0xDBDD1F2D),
        HN_ULONG_TO_UBASE(0x073B1402), HN_ULONG_TO_UBASE(0xD693DEAD),
        HN_ULONG_TO_UBASE(0xA684435B), HN_ULONG_TO_UBASE(0x96255874),
        HN_ULONG_TO_UBASE(0xEEA7471F)
    }
};
static NX_CRYPTO_CONST HN_UBASE           secp224r1_fixed_points_2e_data[][28 >> HN_SIZE_SHIFT] =
{

    /* 2^e * 1G.x */
    {
        HN_ULONG_TO_UBASE(0x6DDDF554), HN_ULONG_TO_UBASE(0x2D966526),
        HN_ULONG_TO_UBASE(0xD78B60EF), HN_ULONG_TO_UBASE(0xA4179613),
        HN_ULONG_TO_UBASE(0x27A34CDB), HN_ULONG_TO_UBASE(0x6AFC31CE),
        HN_ULONG_TO_UBASE(0xD35AB74D)
    },

    /* 2^e * 1G.y */
    {
        HN_ULONG_TO_UBASE(0x22DEB15E), HN_ULONG_TO_UBASE(0xAB85CCDD),
        HN_ULONG_TO_UBASE(0xE5783A6A), HN_ULONG_TO_UBASE(0x93C62137),
        HN_ULONG_TO_UBASE(0x41CFFD8C), HN_ULONG_TO_UBASE(0xE90F2DA1),
        HN_ULONG_TO_UBASE(0x355A1830)
    },

    /* 2^e * 2G.x */
    {
        HN_ULONG_TO_UBASE(0xADAADE65), HN_ULONG_TO_UBASE(0x3C1A494E),
        HN_ULONG_TO_UBASE(0x4DA77FE5), HN_ULONG_TO_UBASE(0xEC86D6DA),
        HN_ULONG_TO_UBASE(0x992996AB), HN_ULONG_TO_UBASE(0x6090E3E7),
        HN_ULONG_TO_UBASE(0x65C3553C)
    },

    /* 2^e * 2G.y */
    {
        HN_ULONG_TO_UBASE(0x1FB09346), HN_ULONG_TO_UBASE(0xAFFA610B),
        HN_ULONG_TO_UBASE(0x540B8A4A), HN_ULONG_TO_UBASE(0xCBABF1C6),
        HN_ULONG_TO_UBASE(0x1A13CCD3), HN_ULONG_TO_UBASE(0x18C28AC5),
        HN_ULONG_TO_UBASE(0x02995B1B)
    },

    /* 2^e * 3G.x */
    {
        HN_ULONG_TO_UBASE(0x8E7295EF), HN_ULONG_TO_UBASE(0x04787456),
        HN_ULONG_TO_UBASE(0x19FBE38D), HN_ULONG_TO_UBASE(0x0D9A86B4),
        HN_ULONG_TO_UBASE(0x0690A755), HN_ULONG_TO_UBASE(0xBEAC33DC),
        HN_ULONG_TO_UBASE(0xD3966A44)
    },

    /* 2^e * 3G.y */
    {
        HN_ULONG_TO_UBASE(0xEC29132F), HN_ULONG_TO_UBASE(0xF32B7280),
        HN_ULONG_TO_UBASE(0x3B6A032D), HN_ULONG_TO_UBASE(0x1200BEAA),
        HN_ULONG_TO_UBASE(0x7DD88AE4), HN_ULONG_TO_UBASE(0xE3A100DC),
        HN_ULONG_TO_UBASE(0xD25E2513)
    },

    /* 2^e * 4G.x */
    {
        HN_ULONG_TO_UBASE(0xEB2EFAFD), HN_ULONG_TO_UBASE(0x90924857),
        HN_ULONG_TO_UBASE(0xCE412231), HN_ULONG_TO_UBASE(0x53FCAC2B),
        HN_ULONG_TO_UBASE(0xDAA14455), HN_ULONG_TO_UBASE(0x3562D58E),
        HN_ULONG_TO_UBASE(0x825800FD)
    },

    /* 2^e * 4G.y */
    {
        HN_ULONG_TO_UBASE(0x8EA96621), HN_ULONG_TO_UBASE(0x8D8D7914),
        HN_ULONG_TO_UBASE(0x1C3DD9ED), HN_ULONG_TO_UBASE(0x16B523A0),
        HN_ULONG_TO_UBASE(0x8B219F94), HN_ULONG_TO_UBASE(0x77DAEAAF),
        HN_ULONG_TO_UBASE(0xD8DB0CC2)
    },

    /* 2^e * 5G.x */
    {
        HN_ULONG_TO_UBASE(0xB1A700F0), HN_ULONG_TO_UBASE(0x9176A9C3),
        HN_ULONG_TO_UBASE(0xD29BC7E6), HN_ULONG_TO_UBASE(0x0327E9AC),
        HN_ULONG_TO_UBASE(0x212D1A6B), HN_ULONG_TO_UBASE(0xE154BE69),
        HN_ULONG_TO_UBASE(0x6322E97F)
    },

    /* 2^e * 5G.y */
    {
        HN_ULONG_TO_UBASE(0x465D62AA), HN_ULONG_TO_UBASE(0x05469FC5),
        HN_ULONG_TO_UBASE(0xED18883B), HN_ULONG_TO_UBASE(0x2B888D41),
        HN_ULONG_TO_UBASE(0x8EAE66C5), HN_ULONG_TO_UBASE(0x25BE511F),
        HN_ULONG_TO_UBASE(0xE4FCBE93)
    },

    /* 2^e * 6G.x */
    {
        HN_ULONG_TO_UBASE(0x583CAC16), HN_ULONG_TO_UBASE(0x3A825FDF),
        HN_ULONG_TO_UBASE(0x857C7B02), HN_ULONG_TO_UBASE(0x0165020B),
        HN_ULONG_TO_UBASE(0x3C17744B), HN_ULONG_TO_UBASE(0xDAF2F168),
        HN_ULONG_TO_UBASE(0x14FFD0A2)
    },

    /* 2^e * 6G.y */
    {
        HN_ULONG_TO_UBASE(0x184218F9), HN_ULONG_TO_UBASE(0xD4323B36),
        HN_ULONG_TO_UBASE(0xEC4E3B47), HN_ULONG_TO_UBASE(0x1ACF4944),
        HN_ULONG_TO_UBASE(0x5B308084), HN_ULONG_TO_UBASE(0x1A28BBC1),
        HN_ULONG_TO_UBASE(0x0BCED4B0)
    },

    /* 2^e * 7G.x */
    {
        HN_ULONG_TO_UBASE(0x230DF5C4), HN_ULONG_TO_UBASE(0xA892AC22),
        HN_ULONG_TO_UBASE(0x3B4063ED), HN_ULONG_TO_UBASE(0x0C9352F3),
        HN_ULONG_TO_UBASE(0x3F19870C), HN_ULONG_TO_UBASE(0xA65233CB),
        HN_ULONG_TO_UBASE(0x40064F2B)
    },

    /* 2^e * 7G.y */
    {
        HN_ULONG_TO_UBASE(0x924F8992), HN_ULONG_TO_UBASE(0x17FE16F0),
        HN_ULONG_TO_UBASE(0xA25AF5B5), HN_ULONG_TO_UBASE(0x23A6012D),
        HN_ULONG_TO_UBASE(0x57BB24F7), HN_ULONG_TO_UBASE(0x760DEF1A),
        HN_ULONG_TO_UBASE(0x06F8BC76)
    },

    /* 2^e * 8G.x */
    {
        HN_ULONG_TO_UBASE(0xF7817CB9), HN_ULONG_TO_UBASE(0x784A7084),
        HN_ULONG_TO_UBASE(0x0738EE9A), HN_ULONG_TO_UBASE(0xC326BCAB),
        HN_ULONG_TO_UBASE(0xC11E11D9), HN_ULONG_TO_UBASE(0x0F1AAE3E),
        HN_ULONG_TO_UBASE(0xDC0FE90E)
    },

    /* 2^e * 8G.y */
    {
        HN_ULONG_TO_UBASE(0xA5F98390), HN_ULONG_TO_UBASE(0x74CF639E),
        HN_ULONG_TO_UBASE(0x0AA22FFB), HN_ULONG_TO_UBASE(0x47B75C35),
        HN_ULONG_TO_UBASE(0xFAE98A40), HN_ULONG_TO_UBASE(0x17FC459A),
        HN_ULONG_TO_UBASE(0x956EC2D6)
    },

    /* 2^e * 9G.x */
    {
        HN_ULONG_TO_UBASE(0x48C1BE6A), HN_ULONG_TO_UBASE(0x624306D6),
        HN_ULONG_TO_UBASE(0xCD8BC9A4), HN_ULONG_TO_UBASE(0x2F2E9247),
        HN_ULONG_TO_UBASE(0x595E377D), HN_ULONG_TO_UBASE(0xF1A52EF5),
        HN_ULONG_TO_UBASE(0xBD1C3CAF)
    },

    /* 2^e * 9G.y */
    {
        HN_ULONG_TO_UBASE(0x472409D0), HN_ULONG_TO_UBASE(0x73045E14),
        HN_ULONG_TO_UBASE(0xE17078F7), HN_ULONG_TO_UBASE(0x4F7D29F3),
        HN_ULONG_TO_UBASE(0x5A602B2D), HN_ULONG_TO_UBASE(0x5CDFBB74),
        HN_ULONG_TO_UBASE(0x19183768)
    },

    /* 2^e * 10G.x */
    {
        HN_ULONG_TO_UBASE(0x54A8CB79), HN_ULONG_TO_UBASE(0x265B6EE2),
        HN_ULONG_TO_UBASE(0x433F5E70), HN_ULONG_TO_UBASE(0xDEF44953),
        HN_ULONG_TO_UBASE(0x1FAEB1D1), HN_ULONG_TO_UBASE(0x5C09DEE2),
        HN_ULONG_TO_UBASE(0xC4C22578)
    },

    /* 2^e * 10G.y */
    {
        HN_ULONG_TO_UBASE(0xBBA1E518), HN_ULONG_TO_UBASE(0xB8307CE7),
        HN_ULONG_TO_UBASE(0x25B1036D), HN_ULONG_TO_UBASE(0x9E8F31B1),
        HN_ULONG_TO_UBASE(0xE9186883), HN_ULONG_TO_UBASE(0x33B9F347),
        HN_ULONG_TO_UBASE(0xC765866E)
    },

    /* 2^e * 11G.x */
    {
        HN_ULONG_TO_UBASE(0x24F96906), HN_ULONG_TO_UBASE(0x933BFECE),
        HN_ULONG_TO_UBASE(0xDA641E50), HN_ULONG_TO_UBASE(0xDB264794),
        HN_ULONG_TO_UBASE(0x5DF64F95), HN_ULONG_TO_UBASE(0x714B05DE),
        HN_ULONG_TO_UBASE(0x297ECD89)
    },

    /* 2^e * 11G.y */
    {
        HN_ULONG_TO_UBASE(0xEBB2C3AA), HN_ULONG_TO_UBASE(0xD5701BD3),
        HN_ULONG_TO_UBASE(0xB4F53CB1), HN_ULONG_TO_UBASE(0xAF167073),
        HN_ULONG_TO_UBASE(0xC5665658), HN_ULONG_TO_UBASE(0x66FE5813),
        HN_ULONG_TO_UBASE(0x9895089D)
    },

    /* 2^e * 12G.x */
    {
        HN_ULONG_TO_UBASE(0xF78C4790), HN_ULONG_TO_UBASE(0x2E0FEF05),
        HN_ULONG_TO_UBASE(0x3633B05D), HN_ULONG_TO_UBASE(0x1C942D77),
        HN_ULONG_TO_UBASE(0x229C3A95), HN_ULONG_TO_UBASE(0x4911BB94),
        HN_ULONG_TO_UBASE(0xBBBD70DF)
    },

    /* 2^e * 12G.y */
    {
        HN_ULONG_TO_UBASE(0x3D2C1168), HN_ULONG_TO_UBASE(0x73B2C696),
        HN_ULONG_TO_UBASE(0x47A72B0D), HN_ULONG_TO_UBASE(0x4080105F),
        HN_ULONG_TO_UBASE(0xDF611161), HN_ULONG_TO_UBASE(0x9E67B09F),
        HN_ULONG_TO_UBASE(0x7B7E94B3)
    },

    /* 2^e * 13G.x */
    {
        HN_ULONG_TO_UBASE(0x6EFBE2B3), HN_ULONG_TO_UBASE(0x9DAD1A7D),
        HN_ULONG_TO_UBASE(0x482C0DA6), HN_ULONG_TO_UBASE(0x8345F012),
        HN_ULONG_TO_UBASE(0x3BDF1243), HN_ULONG_TO_UBASE(0x7AA4D96B),
        HN_ULONG_TO_UBASE(0x40D7558D)
    },

    /* 2^e * 13G.y */
    {
        HN_ULONG_TO_UBASE(0xFB5C6D3D), HN_ULONG_TO_UBASE(0x388A09FF),
        HN_ULONG_TO_UBASE(0x6E5D9FFD), HN_ULONG_TO_UBASE(0x9B1C9A35),
        HN_ULONG_TO_UBASE(0x73F15F4F), HN_ULONG_TO_UBASE(0x63C3EA59),
        HN_ULONG_TO_UBASE(0xDCD5F59F)
    },

    /* 2^e * 14G.x */
    {
        HN_ULONG_TO_UBASE(0x4C5CA7AB), HN_ULONG_TO_UBASE(0x37ACF39F),
        HN_ULONG_TO_UBASE(0x71CC5FD7), HN_ULONG_TO_UBASE(0x11844C80),
        HN_ULONG_TO_UBASE(0x4E3602CD), HN_ULONG_TO_UBASE(0xC9ABBAC6),
        HN_ULONG_TO_UBASE(0x0ACD4644)
    },

    /* 2^e * 14G.y */
    {
        HN_ULONG_TO_UBASE(0x36D8BF6E), HN_ULONG_TO_UBASE(0x2A6C011A),
        HN_ULONG_TO_UBASE(0x87BA24E3), HN_ULONG_TO_UBASE(0xFAD8FECD),
        HN_ULONG_TO_UBASE(0xF6F56574), HN_ULONG_TO_UBASE(0xED940519),
        HN_ULONG_TO_UBASE(0x050B204C)
    },

    /* 2^e * 15G.x */
    {
        HN_ULONG_TO_UBASE(0xAE7D9A96), HN_ULONG_TO_UBASE(0x0AED4F1C),
        HN_ULONG_TO_UBASE(0xF7AD94C4), HN_ULONG_TO_UBASE(0xEF9B5CEE),
        HN_ULONG_TO_UBASE(0x8E4A3BF3), HN_ULONG_TO_UBASE(0xC3B55E77),
        HN_ULONG_TO_UBASE(0x7405783D)
    },

    /* 2^e * 15G.y */
    {
        HN_ULONG_TO_UBASE(0x61B6E8C6), HN_ULONG_TO_UBASE(0x8B32477C),
        HN_ULONG_TO_UBASE(0x97570F01), HN_ULONG_TO_UBASE(0x95D1B46A),
        HN_ULONG_TO_UBASE(0x176D0A7E), HN_ULONG_TO_UBASE(0x4C7D0E91),
        HN_ULONG_TO_UBASE(0x3DF90FBC)
    }
};
static NX_CRYPTO_CONST NX_CRYPTO_EC_POINT secp224r1_fixed_points_array[] =
{

    /* 2G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[0],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[1],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 3G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[2],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[3],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 4G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[4],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[5],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 5G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[6],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[7],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 6G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[8],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[9],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 7G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[10],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[11],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 8G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[12],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[13],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 9G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[14],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[15],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 10G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[16],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[17],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 11G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[18],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[19],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 12G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[20],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[21],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 13G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[22],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[23],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 14G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[24],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[25],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 15G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[26],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_data[27],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    }
};
static NX_CRYPTO_CONST NX_CRYPTO_EC_POINT secp224r1_fixed_points_2e_array[] =
{

    /* 2^e * 1G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[0],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[1],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 2G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[2],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[3],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 3G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[4],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[5],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 4G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[6],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[7],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 5G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[8],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[9],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 6G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[10],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[11],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 7G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[12],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[13],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 8G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[14],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[15],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 9G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[16],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[17],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 10G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[18],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[19],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 11G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[20],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[21],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 12G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[22],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[23],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 13G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[24],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[25],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 14G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[26],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[27],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 15G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[28],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp224r1_fixed_points_2e_data[29],
            28 >> HN_SIZE_SHIFT, 28, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    }
};


NX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_secp224r1_fixed_points =
{
    4u, 224u, 56u, 28u,
    (NX_CRYPTO_EC_POINT *)secp224r1_fixed_points_array,
    (NX_CRYPTO_EC_POINT *)secp224r1_fixed_points_2e_array
};

