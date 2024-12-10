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
static NX_CRYPTO_CONST HN_UBASE           secp256r1_fixed_points_data[][32 >> HN_SIZE_SHIFT] =
{

    /* 2G.x */
    {
        HN_ULONG_TO_UBASE(0x8E14DB63), HN_ULONG_TO_UBASE(0x90E75CB4),
        HN_ULONG_TO_UBASE(0xAD651F7E), HN_ULONG_TO_UBASE(0x29493BAA),
        HN_ULONG_TO_UBASE(0x326E25DE), HN_ULONG_TO_UBASE(0x8492592E),
        HN_ULONG_TO_UBASE(0x2811AAA5), HN_ULONG_TO_UBASE(0x0FA822BC)
    },

    /* 2G.y */
    {
        HN_ULONG_TO_UBASE(0x5F462EE7), HN_ULONG_TO_UBASE(0xE4112454),
        HN_ULONG_TO_UBASE(0x50FE82F5), HN_ULONG_TO_UBASE(0x34B1A650),
        HN_ULONG_TO_UBASE(0xB3DF188B), HN_ULONG_TO_UBASE(0x6F4AD4BC),
        HN_ULONG_TO_UBASE(0xF5DBA80D), HN_ULONG_TO_UBASE(0xBFF44AE8)
    },

    /* 3G.x */
    {
        HN_ULONG_TO_UBASE(0x097992AF), HN_ULONG_TO_UBASE(0x93391CE2),
        HN_ULONG_TO_UBASE(0x0D35F1FA), HN_ULONG_TO_UBASE(0xE96C98FD),
        HN_ULONG_TO_UBASE(0x95E02789), HN_ULONG_TO_UBASE(0xB257C0DE),
        HN_ULONG_TO_UBASE(0x89D6726F), HN_ULONG_TO_UBASE(0x300A4BBC)
    },

    /* 3G.y */
    {
        HN_ULONG_TO_UBASE(0xC08127A0), HN_ULONG_TO_UBASE(0xAA54A291),
        HN_ULONG_TO_UBASE(0xA9D806A5), HN_ULONG_TO_UBASE(0x5BB1EEAD),
        HN_ULONG_TO_UBASE(0xFF1E3C6F), HN_ULONG_TO_UBASE(0x7F1DDB25),
        HN_ULONG_TO_UBASE(0xD09B4644), HN_ULONG_TO_UBASE(0x72AAC7E0)
    },

    /* 4G.x */
    {
        HN_ULONG_TO_UBASE(0xD789BD85), HN_ULONG_TO_UBASE(0x57C84FC9),
        HN_ULONG_TO_UBASE(0xC297EAC3), HN_ULONG_TO_UBASE(0xFC35FF7D),
        HN_ULONG_TO_UBASE(0x88C6766E), HN_ULONG_TO_UBASE(0xFB982FD5),
        HN_ULONG_TO_UBASE(0xEEDB5E67), HN_ULONG_TO_UBASE(0x447D739B)
    },

    /* 4G.y */
    {
        HN_ULONG_TO_UBASE(0x72E25B32), HN_ULONG_TO_UBASE(0x0C7E33C9),
        HN_ULONG_TO_UBASE(0xA7FAE500), HN_ULONG_TO_UBASE(0x3D349B95),
        HN_ULONG_TO_UBASE(0x3A4AAFF7), HN_ULONG_TO_UBASE(0xE12E9D95),
        HN_ULONG_TO_UBASE(0x834131EE), HN_ULONG_TO_UBASE(0x2D4825AB)
    },

    /* 5G.x */
    {
        HN_ULONG_TO_UBASE(0x2A1D367F), HN_ULONG_TO_UBASE(0x13949C93),
        HN_ULONG_TO_UBASE(0x1A0A11B7), HN_ULONG_TO_UBASE(0xEF7FBD2B),
        HN_ULONG_TO_UBASE(0xB91DFC60), HN_ULONG_TO_UBASE(0xDDC6068B),
        HN_ULONG_TO_UBASE(0x8A9C72FF), HN_ULONG_TO_UBASE(0xEF951932)
    },

    /* 5G.y */
    {
        HN_ULONG_TO_UBASE(0x7376D8A8), HN_ULONG_TO_UBASE(0x196035A7),
        HN_ULONG_TO_UBASE(0x95CA1740), HN_ULONG_TO_UBASE(0x23183B08),
        HN_ULONG_TO_UBASE(0x022C219C), HN_ULONG_TO_UBASE(0xC1EE9807),
        HN_ULONG_TO_UBASE(0x7DBB2C9B), HN_ULONG_TO_UBASE(0x611E9FC3)
    },

    /* 6G.x */
    {
        HN_ULONG_TO_UBASE(0x0B57F4BC), HN_ULONG_TO_UBASE(0xCAE2B192),
        HN_ULONG_TO_UBASE(0xC6C9BC36), HN_ULONG_TO_UBASE(0x2936DF5E),
        HN_ULONG_TO_UBASE(0xE11238BF), HN_ULONG_TO_UBASE(0x7DEA6482),
        HN_ULONG_TO_UBASE(0x7B51F5D8), HN_ULONG_TO_UBASE(0x55066379)
    },

    /* 6G.y */
    {
        HN_ULONG_TO_UBASE(0x348A964C), HN_ULONG_TO_UBASE(0x44FFE216),
        HN_ULONG_TO_UBASE(0xDBDEFBE1), HN_ULONG_TO_UBASE(0x9FB3D576),
        HN_ULONG_TO_UBASE(0x8D9D50E5), HN_ULONG_TO_UBASE(0x0AFA4001),
        HN_ULONG_TO_UBASE(0x8AECB851), HN_ULONG_TO_UBASE(0x15716484)
    },

    /* 7G.x */
    {
        HN_ULONG_TO_UBASE(0xFC5CDE01), HN_ULONG_TO_UBASE(0xE48ECAFF),
        HN_ULONG_TO_UBASE(0x0D715F26), HN_ULONG_TO_UBASE(0x7CCD84E7),
        HN_ULONG_TO_UBASE(0xF43E4391), HN_ULONG_TO_UBASE(0xA2E8F483),
        HN_ULONG_TO_UBASE(0xB21141EA), HN_ULONG_TO_UBASE(0xEB5D7745)
    },

    /* 7G.y */
    {
        HN_ULONG_TO_UBASE(0x731A3479), HN_ULONG_TO_UBASE(0xCAC917E2),
        HN_ULONG_TO_UBASE(0x2844B645), HN_ULONG_TO_UBASE(0x85F22CFE),
        HN_ULONG_TO_UBASE(0x58006CEE), HN_ULONG_TO_UBASE(0x0990E6A1),
        HN_ULONG_TO_UBASE(0xDBECC17B), HN_ULONG_TO_UBASE(0xEAFD72EB)
    },

    /* 8G.x */
    {
        HN_ULONG_TO_UBASE(0x313728BE), HN_ULONG_TO_UBASE(0x6CF20FFB),
        HN_ULONG_TO_UBASE(0xA3C6B94A), HN_ULONG_TO_UBASE(0x96439591),
        HN_ULONG_TO_UBASE(0x44315FC5), HN_ULONG_TO_UBASE(0x2736FF83),
        HN_ULONG_TO_UBASE(0xA7849276), HN_ULONG_TO_UBASE(0xA6D39677)
    },

    /* 8G.y */
    {
        HN_ULONG_TO_UBASE(0xC357F5F4), HN_ULONG_TO_UBASE(0xF2BAB833),
        HN_ULONG_TO_UBASE(0x2284059B), HN_ULONG_TO_UBASE(0x824A920C),
        HN_ULONG_TO_UBASE(0x2D27ECDF), HN_ULONG_TO_UBASE(0x66B8BABD),
        HN_ULONG_TO_UBASE(0x9B0B8816), HN_ULONG_TO_UBASE(0x674F8474)
    },

    /* 9G.x */
    {
        HN_ULONG_TO_UBASE(0x677C8A3E), HN_ULONG_TO_UBASE(0x2DF48C04),
        HN_ULONG_TO_UBASE(0x0203A56B), HN_ULONG_TO_UBASE(0x74E02F08),
        HN_ULONG_TO_UBASE(0xB8C7FEDB), HN_ULONG_TO_UBASE(0x31855F7D),
        HN_ULONG_TO_UBASE(0x72C9DDAD), HN_ULONG_TO_UBASE(0x4E769E76)
    },

    /* 9G.y */
    {
        HN_ULONG_TO_UBASE(0xB824BBB0), HN_ULONG_TO_UBASE(0xA4C36165),
        HN_ULONG_TO_UBASE(0x3B9122A5), HN_ULONG_TO_UBASE(0xFB9AE16F),
        HN_ULONG_TO_UBASE(0x06947281), HN_ULONG_TO_UBASE(0x1EC00572),
        HN_ULONG_TO_UBASE(0xDE830663), HN_ULONG_TO_UBASE(0x42B99082)
    },

    /* 10G.x */
    {
        HN_ULONG_TO_UBASE(0xDDA868B9), HN_ULONG_TO_UBASE(0x6EF95150),
        HN_ULONG_TO_UBASE(0x9C0CE131), HN_ULONG_TO_UBASE(0xD1F89E79),
        HN_ULONG_TO_UBASE(0x08A1C478), HN_ULONG_TO_UBASE(0x7FDC1CA0),
        HN_ULONG_TO_UBASE(0x1C6CE04D), HN_ULONG_TO_UBASE(0x78878EF6)
    },

    /* 10G.y */
    {
        HN_ULONG_TO_UBASE(0x1FE0D976), HN_ULONG_TO_UBASE(0x9C62B912),
        HN_ULONG_TO_UBASE(0xBDE08D4F), HN_ULONG_TO_UBASE(0x6ACE570E),
        HN_ULONG_TO_UBASE(0x12309DEF), HN_ULONG_TO_UBASE(0xDE53142C),
        HN_ULONG_TO_UBASE(0x7B72C321), HN_ULONG_TO_UBASE(0xB6CB3F5D)
    },

    /* 11G.x */
    {
        HN_ULONG_TO_UBASE(0xC31A3573), HN_ULONG_TO_UBASE(0x7F991ED2),
        HN_ULONG_TO_UBASE(0xD54FB496), HN_ULONG_TO_UBASE(0x5B82DD5B),
        HN_ULONG_TO_UBASE(0x812FFCAE), HN_ULONG_TO_UBASE(0x595C5220),
        HN_ULONG_TO_UBASE(0x716B1287), HN_ULONG_TO_UBASE(0x0C88BC4D)
    },

    /* 11G.y */
    {
        HN_ULONG_TO_UBASE(0x5F48ACA8), HN_ULONG_TO_UBASE(0x3A57BF63),
        HN_ULONG_TO_UBASE(0xDF2564F3), HN_ULONG_TO_UBASE(0x7C8181F4),
        HN_ULONG_TO_UBASE(0x9C04E6AA), HN_ULONG_TO_UBASE(0x18D1B5B3),
        HN_ULONG_TO_UBASE(0xF3901DC6), HN_ULONG_TO_UBASE(0xDD5DDEA3)
    },

    /* 12G.x */
    {
        HN_ULONG_TO_UBASE(0x3E72AD0C), HN_ULONG_TO_UBASE(0xE96A79FB),
        HN_ULONG_TO_UBASE(0x42BA792F), HN_ULONG_TO_UBASE(0x43A0A28C),
        HN_ULONG_TO_UBASE(0x083E49F3), HN_ULONG_TO_UBASE(0xEFE0A423),
        HN_ULONG_TO_UBASE(0x6B317466), HN_ULONG_TO_UBASE(0x68F344AF)
    },

    /* 12G.y */
    {
        HN_ULONG_TO_UBASE(0x3FB24D4A), HN_ULONG_TO_UBASE(0xCDFE17DB),
        HN_ULONG_TO_UBASE(0x71F5C626), HN_ULONG_TO_UBASE(0x668BFC22),
        HN_ULONG_TO_UBASE(0x24D67FF3), HN_ULONG_TO_UBASE(0x604ED93C),
        HN_ULONG_TO_UBASE(0xF8540A20), HN_ULONG_TO_UBASE(0x31B9C405)
    },

    /* 13G.x */
    {
        HN_ULONG_TO_UBASE(0xA2582E7F), HN_ULONG_TO_UBASE(0xD36B4789),
        HN_ULONG_TO_UBASE(0x4EC39C28), HN_ULONG_TO_UBASE(0x0D1A1014),
        HN_ULONG_TO_UBASE(0xEDBAD7A0), HN_ULONG_TO_UBASE(0x663C62C3),
        HN_ULONG_TO_UBASE(0x6F461DB9), HN_ULONG_TO_UBASE(0x4052BF4B)
    },

    /* 13G.y */
    {
        HN_ULONG_TO_UBASE(0x188D25EB), HN_ULONG_TO_UBASE(0x235A27C3),
        HN_ULONG_TO_UBASE(0x99BFCC5B), HN_ULONG_TO_UBASE(0xE724F339),
        HN_ULONG_TO_UBASE(0x71D70CC8), HN_ULONG_TO_UBASE(0x862BE6BD),
        HN_ULONG_TO_UBASE(0x90B0FC61), HN_ULONG_TO_UBASE(0xFECF4D51)
    },

    /* 14G.x */
    {
        HN_ULONG_TO_UBASE(0xA1D4CFAC), HN_ULONG_TO_UBASE(0x74346C10),
        HN_ULONG_TO_UBASE(0x8526A7A4), HN_ULONG_TO_UBASE(0xAFDF5CC0),
        HN_ULONG_TO_UBASE(0xF62BFF7A), HN_ULONG_TO_UBASE(0x123202A8),
        HN_ULONG_TO_UBASE(0xC802E41A), HN_ULONG_TO_UBASE(0x1EDDBAE2)
    },

    /* 14G.y */
    {
        HN_ULONG_TO_UBASE(0xD603F844), HN_ULONG_TO_UBASE(0x8FA0AF2D),
        HN_ULONG_TO_UBASE(0x4C701917), HN_ULONG_TO_UBASE(0x36E06B7E),
        HN_ULONG_TO_UBASE(0x73DB33A0), HN_ULONG_TO_UBASE(0x0C45F452),
        HN_ULONG_TO_UBASE(0x560EBCFC), HN_ULONG_TO_UBASE(0x43104D86)
    },

    /* 15G.x */
    {
        HN_ULONG_TO_UBASE(0x0D1D78E5), HN_ULONG_TO_UBASE(0x9615B511),
        HN_ULONG_TO_UBASE(0x25C4744B), HN_ULONG_TO_UBASE(0x66B0DE32),
        HN_ULONG_TO_UBASE(0x6AAF363A), HN_ULONG_TO_UBASE(0x0A4A46FB),
        HN_ULONG_TO_UBASE(0x84F7A21C), HN_ULONG_TO_UBASE(0xB48E26B4)
    },

    /* 15G.y */
    {
        HN_ULONG_TO_UBASE(0x21A01B2D), HN_ULONG_TO_UBASE(0x06EBB0F6),
        HN_ULONG_TO_UBASE(0x8B7B0F98), HN_ULONG_TO_UBASE(0xC004E404),
        HN_ULONG_TO_UBASE(0xFED6F668), HN_ULONG_TO_UBASE(0x64131BCD),
        HN_ULONG_TO_UBASE(0x4D4D3DAB), HN_ULONG_TO_UBASE(0xFAC01540)
    }
};
static NX_CRYPTO_CONST HN_UBASE           secp256r1_fixed_points_2e_data[][32 >> HN_SIZE_SHIFT] =
{

    /* 2^e * 1G.x */
    {
        HN_ULONG_TO_UBASE(0x185A5943), HN_ULONG_TO_UBASE(0x3A5A9E22),
        HN_ULONG_TO_UBASE(0x5C65DFB6), HN_ULONG_TO_UBASE(0x1AB91936),
        HN_ULONG_TO_UBASE(0x262C71DA), HN_ULONG_TO_UBASE(0x21656B32),
        HN_ULONG_TO_UBASE(0xAF22AF89), HN_ULONG_TO_UBASE(0x7FE36B40)
    },

    /* 2^e * 1G.y */
    {
        HN_ULONG_TO_UBASE(0x699CA101), HN_ULONG_TO_UBASE(0xD50D152C),
        HN_ULONG_TO_UBASE(0x7B8AF212), HN_ULONG_TO_UBASE(0x74B3D586),
        HN_ULONG_TO_UBASE(0x07DCA6F1), HN_ULONG_TO_UBASE(0x9F09F404),
        HN_ULONG_TO_UBASE(0x25B63624), HN_ULONG_TO_UBASE(0xE697D458)
    },

    /* 2^e * 2G.x */
    {
        HN_ULONG_TO_UBASE(0x7512218E), HN_ULONG_TO_UBASE(0xA84AA939),
        HN_ULONG_TO_UBASE(0x74CA0141), HN_ULONG_TO_UBASE(0xE9A521B0),
        HN_ULONG_TO_UBASE(0x18A2E902), HN_ULONG_TO_UBASE(0x57880B3A),
        HN_ULONG_TO_UBASE(0x12A677A6), HN_ULONG_TO_UBASE(0x4A5B5066)
    },

    /* 2^e * 2G.y */
    {
        HN_ULONG_TO_UBASE(0x4C4F3840), HN_ULONG_TO_UBASE(0x0BEADA7A),
        HN_ULONG_TO_UBASE(0x19E26D9D), HN_ULONG_TO_UBASE(0x626DB154),
        HN_ULONG_TO_UBASE(0xE1627D40), HN_ULONG_TO_UBASE(0xC42604FB),
        HN_ULONG_TO_UBASE(0xEAC089F1), HN_ULONG_TO_UBASE(0xEB13461C)
    },

    /* 2^e * 3G.x */
    {
        HN_ULONG_TO_UBASE(0x27A43281), HN_ULONG_TO_UBASE(0xF9FAED09),
        HN_ULONG_TO_UBASE(0x4103ECBC), HN_ULONG_TO_UBASE(0x5E52C414),
        HN_ULONG_TO_UBASE(0xA815C857), HN_ULONG_TO_UBASE(0xC342967A),
        HN_ULONG_TO_UBASE(0x1C6A220A), HN_ULONG_TO_UBASE(0x0781B829)
    },

    /* 2^e * 3G.y */
    {
        HN_ULONG_TO_UBASE(0xEAC55F80), HN_ULONG_TO_UBASE(0x5A8343CE),
        HN_ULONG_TO_UBASE(0xE54A05E3), HN_ULONG_TO_UBASE(0x88F80EEE),
        HN_ULONG_TO_UBASE(0x12916434), HN_ULONG_TO_UBASE(0x97B2A14F),
        HN_ULONG_TO_UBASE(0xF0151593), HN_ULONG_TO_UBASE(0x690CDE8D)
    },

    /* 2^e * 4G.x */
    {
        HN_ULONG_TO_UBASE(0xF7F82F2A), HN_ULONG_TO_UBASE(0xAEE9C75D),
        HN_ULONG_TO_UBASE(0x4AFDF43A), HN_ULONG_TO_UBASE(0x9E4C3587),
        HN_ULONG_TO_UBASE(0x37371326), HN_ULONG_TO_UBASE(0xF5622DF4),
        HN_ULONG_TO_UBASE(0x6EC73617), HN_ULONG_TO_UBASE(0x8A535F56)
    },

    /* 2^e * 4G.y */
    {
        HN_ULONG_TO_UBASE(0x223094B7), HN_ULONG_TO_UBASE(0xC5F9A0AC),
        HN_ULONG_TO_UBASE(0x4C8C7669), HN_ULONG_TO_UBASE(0xCDE53386),
        HN_ULONG_TO_UBASE(0x085A92BF), HN_ULONG_TO_UBASE(0x37E02819),
        HN_ULONG_TO_UBASE(0x68B08BD7), HN_ULONG_TO_UBASE(0x0455C084)
    },

    /* 2^e * 5G.x */
    {
        HN_ULONG_TO_UBASE(0x9477B5D9), HN_ULONG_TO_UBASE(0x0C0A6E2C),
        HN_ULONG_TO_UBASE(0x876DC444), HN_ULONG_TO_UBASE(0xF9A4BF62),
        HN_ULONG_TO_UBASE(0xB6CDC279), HN_ULONG_TO_UBASE(0x5050A949),
        HN_ULONG_TO_UBASE(0xB77F8276), HN_ULONG_TO_UBASE(0x06BADA7A)
    },

    /* 2^e * 5G.y */
    {
        HN_ULONG_TO_UBASE(0xEA48DAC9), HN_ULONG_TO_UBASE(0xC8B4AED1),
        HN_ULONG_TO_UBASE(0x7EA1070F), HN_ULONG_TO_UBASE(0xDEBD8A4B),
        HN_ULONG_TO_UBASE(0x1366EB70), HN_ULONG_TO_UBASE(0x427D4910),
        HN_ULONG_TO_UBASE(0x0E6CB18A), HN_ULONG_TO_UBASE(0x5B476DFD)
    },

    /* 2^e * 6G.x */
    {
        HN_ULONG_TO_UBASE(0x278C340A), HN_ULONG_TO_UBASE(0x7C5C3E44),
        HN_ULONG_TO_UBASE(0x12D66F3B), HN_ULONG_TO_UBASE(0x4D546068),
        HN_ULONG_TO_UBASE(0xAE23C5D8), HN_ULONG_TO_UBASE(0x29A751B1),
        HN_ULONG_TO_UBASE(0x8A2EC908), HN_ULONG_TO_UBASE(0x3E29864E)
    },

    /* 2^e * 6G.y */
    {
        HN_ULONG_TO_UBASE(0x26DBB850), HN_ULONG_TO_UBASE(0x142D2A66),
        HN_ULONG_TO_UBASE(0x765BD780), HN_ULONG_TO_UBASE(0xAD1744C4),
        HN_ULONG_TO_UBASE(0xE322D1ED), HN_ULONG_TO_UBASE(0x1F150E68),
        HN_ULONG_TO_UBASE(0x3DC31E7E), HN_ULONG_TO_UBASE(0x239B90EA)
    },

    /* 2^e * 7G.x */
    {
        HN_ULONG_TO_UBASE(0x7A53322A), HN_ULONG_TO_UBASE(0x78C41652),
        HN_ULONG_TO_UBASE(0x09776F8E), HN_ULONG_TO_UBASE(0x305DDE67),
        HN_ULONG_TO_UBASE(0xF8862ED4), HN_ULONG_TO_UBASE(0xDBCAB759),
        HN_ULONG_TO_UBASE(0x49F72FF7), HN_ULONG_TO_UBASE(0x820F4DD9)
    },

    /* 2^e * 7G.y */
    {
        HN_ULONG_TO_UBASE(0x2B5DEBD4), HN_ULONG_TO_UBASE(0x6CC544A6),
        HN_ULONG_TO_UBASE(0x7B4E8CC4), HN_ULONG_TO_UBASE(0x75BE5D93),
        HN_ULONG_TO_UBASE(0x215C14D3), HN_ULONG_TO_UBASE(0x1B481B1B),
        HN_ULONG_TO_UBASE(0x783A05EC), HN_ULONG_TO_UBASE(0x140406EC)
    },

    /* 2^e * 8G.x */
    {
        HN_ULONG_TO_UBASE(0xE895DF07), HN_ULONG_TO_UBASE(0x6A703F10),
        HN_ULONG_TO_UBASE(0x01876BD8), HN_ULONG_TO_UBASE(0xFD75F3FA),
        HN_ULONG_TO_UBASE(0x0CE08FFE), HN_ULONG_TO_UBASE(0xEB5B06E7),
        HN_ULONG_TO_UBASE(0x2783DFEE), HN_ULONG_TO_UBASE(0x68F6B854)
    },

    /* 2^e * 8G.y */
    {
        HN_ULONG_TO_UBASE(0x78712655), HN_ULONG_TO_UBASE(0x90C76F8A),
        HN_ULONG_TO_UBASE(0xF310BF7F), HN_ULONG_TO_UBASE(0xCF5293D2),
        HN_ULONG_TO_UBASE(0xFDA45028), HN_ULONG_TO_UBASE(0xFBC8044D),
        HN_ULONG_TO_UBASE(0x92E40CE6), HN_ULONG_TO_UBASE(0xCBE1FEBA)
    },

    /* 2^e * 9G.x */
    {
        HN_ULONG_TO_UBASE(0x4396E4C1), HN_ULONG_TO_UBASE(0xE998CEEA),
        HN_ULONG_TO_UBASE(0x6ACEA274), HN_ULONG_TO_UBASE(0xFC82EF0B),
        HN_ULONG_TO_UBASE(0x2250E927), HN_ULONG_TO_UBASE(0x230F729F),
        HN_ULONG_TO_UBASE(0x2F420109), HN_ULONG_TO_UBASE(0xD0B2F94D)
    },

    /* 2^e * 9G.y */
    {
        HN_ULONG_TO_UBASE(0xB38D4966), HN_ULONG_TO_UBASE(0x4305ADDD),
        HN_ULONG_TO_UBASE(0x624C3B45), HN_ULONG_TO_UBASE(0x10B838F8),
        HN_ULONG_TO_UBASE(0x58954E7A), HN_ULONG_TO_UBASE(0x7DB26366),
        HN_ULONG_TO_UBASE(0x8B0719E5), HN_ULONG_TO_UBASE(0x97145982)
    },

    /* 2^e * 10G.x */
    {
        HN_ULONG_TO_UBASE(0x23369FC9), HN_ULONG_TO_UBASE(0x4BD6B726),
        HN_ULONG_TO_UBASE(0x53D0B876), HN_ULONG_TO_UBASE(0x57F2929E),
        HN_ULONG_TO_UBASE(0xF2340687), HN_ULONG_TO_UBASE(0xC2D5CBA4),
        HN_ULONG_TO_UBASE(0x4A866ABA), HN_ULONG_TO_UBASE(0x96161000)
    },

    /* 2^e * 10G.y */
    {
        HN_ULONG_TO_UBASE(0x2E407A5E), HN_ULONG_TO_UBASE(0x49997BCD),
        HN_ULONG_TO_UBASE(0x92DDCB24), HN_ULONG_TO_UBASE(0x69AB197D),
        HN_ULONG_TO_UBASE(0x8FE5131C), HN_ULONG_TO_UBASE(0x2CF1F243),
        HN_ULONG_TO_UBASE(0xCEE75E44), HN_ULONG_TO_UBASE(0x7ACB9FAD)
    },

    /* 2^e * 11G.x */
    {
        HN_ULONG_TO_UBASE(0x23D2D4C0), HN_ULONG_TO_UBASE(0x254E8394),
        HN_ULONG_TO_UBASE(0x7AEA685B), HN_ULONG_TO_UBASE(0xF57F0C91),
        HN_ULONG_TO_UBASE(0x6F75AAEA), HN_ULONG_TO_UBASE(0xA60D880F),
        HN_ULONG_TO_UBASE(0xA333BF5B), HN_ULONG_TO_UBASE(0x24EB9ACC)
    },

    /* 2^e * 11G.y */
    {
        HN_ULONG_TO_UBASE(0x1CDA5DEA), HN_ULONG_TO_UBASE(0xE3DE4CCB),
        HN_ULONG_TO_UBASE(0xC51A6B4F), HN_ULONG_TO_UBASE(0xFEEF9341),
        HN_ULONG_TO_UBASE(0x8BAC4C4D), HN_ULONG_TO_UBASE(0x743125F8),
        HN_ULONG_TO_UBASE(0xACD079CC), HN_ULONG_TO_UBASE(0x69F891C5)
    },

    /* 2^e * 12G.x */
    {
        HN_ULONG_TO_UBASE(0x702476B5), HN_ULONG_TO_UBASE(0xEEE44B35),
        HN_ULONG_TO_UBASE(0xE45C2258), HN_ULONG_TO_UBASE(0x7ED031A0),
        HN_ULONG_TO_UBASE(0xBD6F8514), HN_ULONG_TO_UBASE(0xB422D1E7),
        HN_ULONG_TO_UBASE(0x5972A107), HN_ULONG_TO_UBASE(0xE51F547C)
    },

    /* 2^e * 12G.y */
    {
        HN_ULONG_TO_UBASE(0xC9CF343D), HN_ULONG_TO_UBASE(0xA25BCD6F),
        HN_ULONG_TO_UBASE(0x097C184E), HN_ULONG_TO_UBASE(0x8CA922EE),
        HN_ULONG_TO_UBASE(0xA9FE9A06), HN_ULONG_TO_UBASE(0xA62F98B3),
        HN_ULONG_TO_UBASE(0x25BB1387), HN_ULONG_TO_UBASE(0x1C309A2B)
    },

    /* 2^e * 13G.x */
    {
        HN_ULONG_TO_UBASE(0x1967C459), HN_ULONG_TO_UBASE(0x9295DBEB),
        HN_ULONG_TO_UBASE(0x3472C98E), HN_ULONG_TO_UBASE(0xB0014883),
        HN_ULONG_TO_UBASE(0x08011828), HN_ULONG_TO_UBASE(0xC5049777),
        HN_ULONG_TO_UBASE(0xA2C4E503), HN_ULONG_TO_UBASE(0x20B87B8A)
    },

    /* 2^e * 13G.y */
    {
        HN_ULONG_TO_UBASE(0xE057C277), HN_ULONG_TO_UBASE(0x3063175D),
        HN_ULONG_TO_UBASE(0x8FE582DD), HN_ULONG_TO_UBASE(0x1BD53933),
        HN_ULONG_TO_UBASE(0x5F69A044), HN_ULONG_TO_UBASE(0x0D11ADEF),
        HN_ULONG_TO_UBASE(0x919776BE), HN_ULONG_TO_UBASE(0xF5C6FA49)
    },

    /* 2^e * 14G.x */
    {
        HN_ULONG_TO_UBASE(0x0FD59E11), HN_ULONG_TO_UBASE(0x8C944E76),
        HN_ULONG_TO_UBASE(0x102FAD5F), HN_ULONG_TO_UBASE(0x3876CBA1),
        HN_ULONG_TO_UBASE(0xD83FAA56), HN_ULONG_TO_UBASE(0xA454C3FA),
        HN_ULONG_TO_UBASE(0x332010B9), HN_ULONG_TO_UBASE(0x1ED7D1B9)
    },

    /* 2^e * 14G.y */
    {
        HN_ULONG_TO_UBASE(0x0024B889), HN_ULONG_TO_UBASE(0xA1011A27),
        HN_ULONG_TO_UBASE(0xAC0CD344), HN_ULONG_TO_UBASE(0x05E4D0DC),
        HN_ULONG_TO_UBASE(0xEB6A2A24), HN_ULONG_TO_UBASE(0x52B520F0),
        HN_ULONG_TO_UBASE(0x3217257A), HN_ULONG_TO_UBASE(0x3A2B03F0)
    },

    /* 2^e * 15G.x */
    {
        HN_ULONG_TO_UBASE(0xDF1D043D), HN_ULONG_TO_UBASE(0xF20FC2AF),
        HN_ULONG_TO_UBASE(0xB58D5A62), HN_ULONG_TO_UBASE(0xF330240D),
        HN_ULONG_TO_UBASE(0xA0058C3B), HN_ULONG_TO_UBASE(0xFC7D229C),
        HN_ULONG_TO_UBASE(0xC78DD9F6), HN_ULONG_TO_UBASE(0x15FEE545)
    },

    /* 2^e * 15G.y */
    {
        HN_ULONG_TO_UBASE(0x5BC98CDA), HN_ULONG_TO_UBASE(0x501E8288),
        HN_ULONG_TO_UBASE(0xD046AC04), HN_ULONG_TO_UBASE(0x41EF80E5),
        HN_ULONG_TO_UBASE(0x461210FB), HN_ULONG_TO_UBASE(0x557D9F49),
        HN_ULONG_TO_UBASE(0xB8753F81), HN_ULONG_TO_UBASE(0x4AB5B6B2)
    }
};
static NX_CRYPTO_CONST NX_CRYPTO_EC_POINT secp256r1_fixed_points_array[] =
{

    /* 2G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[0],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[1],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 3G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[2],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[3],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 4G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[4],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[5],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 5G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[6],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[7],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 6G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[8],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[9],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 7G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[10],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[11],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 8G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[12],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[13],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 9G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[14],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[15],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 10G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[16],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[17],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 11G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[18],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[19],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 12G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[20],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[21],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 13G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[22],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[23],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 14G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[24],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[25],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 15G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[26],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_data[27],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    }
};
static NX_CRYPTO_CONST NX_CRYPTO_EC_POINT secp256r1_fixed_points_2e_array[] =
{

    /* 2^e * 1G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[0],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[1],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 2G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[2],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[3],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 3G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[4],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[5],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 4G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[6],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[7],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 5G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[8],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[9],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 6G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[10],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[11],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 7G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[12],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[13],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 8G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[14],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[15],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 9G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[16],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[17],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 10G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[18],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[19],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 11G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[20],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[21],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 12G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[22],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[23],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 13G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[24],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[25],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 14G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[26],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[27],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 15G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[28],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp256r1_fixed_points_2e_data[29],
            32 >> HN_SIZE_SHIFT, 32, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    }
};


NX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_secp256r1_fixed_points =
{
    4u, 256u, 64u, 32u,
    (NX_CRYPTO_EC_POINT *)secp256r1_fixed_points_array,
    (NX_CRYPTO_EC_POINT *)secp256r1_fixed_points_2e_array
};

