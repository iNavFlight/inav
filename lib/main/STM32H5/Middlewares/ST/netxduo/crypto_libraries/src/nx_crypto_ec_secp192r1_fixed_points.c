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
static NX_CRYPTO_CONST HN_UBASE           secp192r1_fixed_points_data[][24 >> HN_SIZE_SHIFT] =
{

    /* 2G.x */
    {
        HN_ULONG_TO_UBASE(0x57B5F01D), HN_ULONG_TO_UBASE(0xF7451A72),
        HN_ULONG_TO_UBASE(0x1C31929A), HN_ULONG_TO_UBASE(0xB37D3FCE),
        HN_ULONG_TO_UBASE(0xC600C45C), HN_ULONG_TO_UBASE(0x2B6CACB0)
    },

    /* 2G.y */
    {
        HN_ULONG_TO_UBASE(0x47A577B6), HN_ULONG_TO_UBASE(0x08242B45),
        HN_ULONG_TO_UBASE(0x14505643), HN_ULONG_TO_UBASE(0x294C91BF),
        HN_ULONG_TO_UBASE(0x4DF109DD), HN_ULONG_TO_UBASE(0xE065AFCC)
    },

    /* 3G.x */
    {
        HN_ULONG_TO_UBASE(0x7C24135D), HN_ULONG_TO_UBASE(0xECB18291),
        HN_ULONG_TO_UBASE(0x2C6CFE0D), HN_ULONG_TO_UBASE(0x90281ACC),
        HN_ULONG_TO_UBASE(0xC1D40CC7), HN_ULONG_TO_UBASE(0x7265D85B)
    },

    /* 3G.y */
    {
        HN_ULONG_TO_UBASE(0xBAD94BD4), HN_ULONG_TO_UBASE(0xD1CCC399),
        HN_ULONG_TO_UBASE(0x3D4F4115), HN_ULONG_TO_UBASE(0xB5410ABE),
        HN_ULONG_TO_UBASE(0xE7AD058E), HN_ULONG_TO_UBASE(0xA747B7BB)
    },

    /* 4G.x */
    {
        HN_ULONG_TO_UBASE(0xEB97690A), HN_ULONG_TO_UBASE(0xDCB416E0),
        HN_ULONG_TO_UBASE(0xD31D3E88), HN_ULONG_TO_UBASE(0x7CD89B6E),
        HN_ULONG_TO_UBASE(0xEAAF5750), HN_ULONG_TO_UBASE(0xF4D5251B)
    },

    /* 4G.y */
    {
        HN_ULONG_TO_UBASE(0xE685E484), HN_ULONG_TO_UBASE(0xAF4FE3DE),
        HN_ULONG_TO_UBASE(0x2914DD60), HN_ULONG_TO_UBASE(0x7B0738A1),
        HN_ULONG_TO_UBASE(0xF8D8086D), HN_ULONG_TO_UBASE(0x93A49E0A)
    },

    /* 5G.x */
    {
        HN_ULONG_TO_UBASE(0x63A15DF0), HN_ULONG_TO_UBASE(0x85F39A75),
        HN_ULONG_TO_UBASE(0x7D59C96F), HN_ULONG_TO_UBASE(0xF56DAC48),
        HN_ULONG_TO_UBASE(0xE743AB91), HN_ULONG_TO_UBASE(0xA05414DD)
    },

    /* 5G.y */
    {
        HN_ULONG_TO_UBASE(0x7D75E2FE), HN_ULONG_TO_UBASE(0xED11279C),
        HN_ULONG_TO_UBASE(0xDD3780CE), HN_ULONG_TO_UBASE(0x52499912),
        HN_ULONG_TO_UBASE(0x14EA5C3B), HN_ULONG_TO_UBASE(0x55D891C6)
    },

    /* 6G.x */
    {
        HN_ULONG_TO_UBASE(0xD322681C), HN_ULONG_TO_UBASE(0xDC75BF3F),
        HN_ULONG_TO_UBASE(0x34CA0295), HN_ULONG_TO_UBASE(0x6F11267C),
        HN_ULONG_TO_UBASE(0xA1FF18DC), HN_ULONG_TO_UBASE(0xD0415D7F)
    },

    /* 6G.y */
    {
        HN_ULONG_TO_UBASE(0xFB9C3DB7), HN_ULONG_TO_UBASE(0x3A2AF705),
        HN_ULONG_TO_UBASE(0xDBB5197B), HN_ULONG_TO_UBASE(0x9597E09B),
        HN_ULONG_TO_UBASE(0x560CBBA9), HN_ULONG_TO_UBASE(0x6E2162E5)
    },

    /* 7G.x */
    {
        HN_ULONG_TO_UBASE(0x77F3F88B), HN_ULONG_TO_UBASE(0xC9C6A9B3),
        HN_ULONG_TO_UBASE(0x1E64859F), HN_ULONG_TO_UBASE(0x673FD86D),
        HN_ULONG_TO_UBASE(0x6E9F6CF1), HN_ULONG_TO_UBASE(0x8EA2B9DB)
    },

    /* 7G.y */
    {
        HN_ULONG_TO_UBASE(0xE0A0052F), HN_ULONG_TO_UBASE(0xF48FE5DE),
        HN_ULONG_TO_UBASE(0x2ABEDD5C), HN_ULONG_TO_UBASE(0x7E78EA0A),
        HN_ULONG_TO_UBASE(0xAFCA5583), HN_ULONG_TO_UBASE(0x38F59B56)
    },

    /* 8G.x */
    {
        HN_ULONG_TO_UBASE(0xBFE7D0FE), HN_ULONG_TO_UBASE(0x94CCEE34),
        HN_ULONG_TO_UBASE(0xEF999054), HN_ULONG_TO_UBASE(0x8AEB7640),
        HN_ULONG_TO_UBASE(0xEC46F6C0), HN_ULONG_TO_UBASE(0xF357F34A)
    },

    /* 8G.y */
    {
        HN_ULONG_TO_UBASE(0x15141FCE), HN_ULONG_TO_UBASE(0x04CC78E2),
        HN_ULONG_TO_UBASE(0xD0BDD150), HN_ULONG_TO_UBASE(0xDC220529),
        HN_ULONG_TO_UBASE(0x967F9AC7), HN_ULONG_TO_UBASE(0x3DE0E1CE)
    },

    /* 9G.x */
    {
        HN_ULONG_TO_UBASE(0x04BA3AAE), HN_ULONG_TO_UBASE(0x68520373),
        HN_ULONG_TO_UBASE(0xBDE031DB), HN_ULONG_TO_UBASE(0x49607803),
        HN_ULONG_TO_UBASE(0x82DB9023), HN_ULONG_TO_UBASE(0x8911E847)
    },

    /* 9G.y */
    {
        HN_ULONG_TO_UBASE(0x5287EDCF), HN_ULONG_TO_UBASE(0xED56DC39),
        HN_ULONG_TO_UBASE(0xE648DD09), HN_ULONG_TO_UBASE(0x95F640DC),
        HN_ULONG_TO_UBASE(0xDB2DE02B), HN_ULONG_TO_UBASE(0x15BDD6CF)
    },

    /* 10G.x */
    {
        HN_ULONG_TO_UBASE(0xC93BC173), HN_ULONG_TO_UBASE(0xF921D37D),
        HN_ULONG_TO_UBASE(0x68416D1F), HN_ULONG_TO_UBASE(0x5B478BA1),
        HN_ULONG_TO_UBASE(0xAE09F12D), HN_ULONG_TO_UBASE(0x5D1AAE40)
    },

    /* 10G.y */
    {
        HN_ULONG_TO_UBASE(0x3AD110C0), HN_ULONG_TO_UBASE(0x6401E81B),
        HN_ULONG_TO_UBASE(0x5ED83DC3), HN_ULONG_TO_UBASE(0x32788DB8),
        HN_ULONG_TO_UBASE(0x808AE8F8), HN_ULONG_TO_UBASE(0x1E7C5765)
    },

    /* 11G.x */
    {
        HN_ULONG_TO_UBASE(0xCF233623), HN_ULONG_TO_UBASE(0xA8E49EDC),
        HN_ULONG_TO_UBASE(0x54661A7B), HN_ULONG_TO_UBASE(0xB509F0FC),
        HN_ULONG_TO_UBASE(0x1A67D2D4), HN_ULONG_TO_UBASE(0x9B379BCD)
    },

    /* 11G.y */
    {
        HN_ULONG_TO_UBASE(0x4741649E), HN_ULONG_TO_UBASE(0x8FFC4B9A),
        HN_ULONG_TO_UBASE(0xE4F6B09E), HN_ULONG_TO_UBASE(0xEC28DBE4),
        HN_ULONG_TO_UBASE(0xCFBDACBD), HN_ULONG_TO_UBASE(0xA8204FE0)
    },

    /* 12G.x */
    {
        HN_ULONG_TO_UBASE(0x250B8857), HN_ULONG_TO_UBASE(0xBCA5231C),
        HN_ULONG_TO_UBASE(0x86F05B35), HN_ULONG_TO_UBASE(0x1623A88F),
        HN_ULONG_TO_UBASE(0x6FE1D2AE), HN_ULONG_TO_UBASE(0xD2D6B452)
    },

    /* 12G.y */
    {
        HN_ULONG_TO_UBASE(0x806F27B2), HN_ULONG_TO_UBASE(0x277D3765),
        HN_ULONG_TO_UBASE(0x453872D5), HN_ULONG_TO_UBASE(0xB03727EF),
        HN_ULONG_TO_UBASE(0xCA59599C), HN_ULONG_TO_UBASE(0xC8F9F5DD)
    },

    /* 13G.x */
    {
        HN_ULONG_TO_UBASE(0x37AC02B2), HN_ULONG_TO_UBASE(0xBDBD95EE),
        HN_ULONG_TO_UBASE(0x36CDF07B), HN_ULONG_TO_UBASE(0x0109B514),
        HN_ULONG_TO_UBASE(0xFDCCFC0B), HN_ULONG_TO_UBASE(0x122A98EE)
    },

    /* 13G.y */
    {
        HN_ULONG_TO_UBASE(0xF68871F7), HN_ULONG_TO_UBASE(0xEAD7B061),
        HN_ULONG_TO_UBASE(0xB0A86AA5), HN_ULONG_TO_UBASE(0xBC68F73B),
        HN_ULONG_TO_UBASE(0xD19486A6), HN_ULONG_TO_UBASE(0x25FE3BF0)
    },

    /* 14G.x */
    {
        HN_ULONG_TO_UBASE(0x93487CAD), HN_ULONG_TO_UBASE(0x03FC9D91),
        HN_ULONG_TO_UBASE(0xBE44BC3E), HN_ULONG_TO_UBASE(0x9A7A4878),
        HN_ULONG_TO_UBASE(0x50A9133D), HN_ULONG_TO_UBASE(0x94EF90DB)
    },

    /* 14G.y */
    {
        HN_ULONG_TO_UBASE(0xB22E7227), HN_ULONG_TO_UBASE(0x9E4BE663),
        HN_ULONG_TO_UBASE(0xF7B51234), HN_ULONG_TO_UBASE(0x712DB99C),
        HN_ULONG_TO_UBASE(0x6722EA87), HN_ULONG_TO_UBASE(0xAEE4182A)
    },

    /* 15G.x */
    {
        HN_ULONG_TO_UBASE(0x4E246A62), HN_ULONG_TO_UBASE(0x466E24C1),
        HN_ULONG_TO_UBASE(0x1A5DDDF7), HN_ULONG_TO_UBASE(0xBA5B47AA),
        HN_ULONG_TO_UBASE(0xB76ECAC6), HN_ULONG_TO_UBASE(0xC70B48A3)
    },

    /* 15G.y */
    {
        HN_ULONG_TO_UBASE(0xAF11FCDE), HN_ULONG_TO_UBASE(0x32B355E1),
        HN_ULONG_TO_UBASE(0x6E177D11), HN_ULONG_TO_UBASE(0x5D79D5D5),
        HN_ULONG_TO_UBASE(0xB07ECDED), HN_ULONG_TO_UBASE(0x4494DDAB)
    }
};
static NX_CRYPTO_CONST HN_UBASE           secp192r1_fixed_points_2e_data[][24 >> HN_SIZE_SHIFT] =
{

    /* 2^e * 1G.x */
    {
        HN_ULONG_TO_UBASE(0x44BB5883), HN_ULONG_TO_UBASE(0x43E29DEE),
        HN_ULONG_TO_UBASE(0x60B4F224), HN_ULONG_TO_UBASE(0x99C69288),
        HN_ULONG_TO_UBASE(0xA0AF9296), HN_ULONG_TO_UBASE(0xC657D599)
    },

    /* 2^e * 1G.y */
    {
        HN_ULONG_TO_UBASE(0xC1C7B573), HN_ULONG_TO_UBASE(0x983289F2),
        HN_ULONG_TO_UBASE(0x9A4B323D), HN_ULONG_TO_UBASE(0x8BDB577B),
        HN_ULONG_TO_UBASE(0x42F42E75), HN_ULONG_TO_UBASE(0x460DAB0F)
    },

    /* 2^e * 2G.x */
    {
        HN_ULONG_TO_UBASE(0x2D7E1827), HN_ULONG_TO_UBASE(0x4516CAE4),
        HN_ULONG_TO_UBASE(0x590267C8), HN_ULONG_TO_UBASE(0xE9ACCF64),
        HN_ULONG_TO_UBASE(0xB5AD4207), HN_ULONG_TO_UBASE(0xE7897F12)
    },

    /* 2^e * 2G.y */
    {
        HN_ULONG_TO_UBASE(0xBE19E9D6), HN_ULONG_TO_UBASE(0x1EF20C93),
        HN_ULONG_TO_UBASE(0xD7F7C011), HN_ULONG_TO_UBASE(0x7C5D3FCB),
        HN_ULONG_TO_UBASE(0x9BA0DF2B), HN_ULONG_TO_UBASE(0x68B64D8B)
    },

    /* 2^e * 3G.x */
    {
        HN_ULONG_TO_UBASE(0x930ED817), HN_ULONG_TO_UBASE(0x787C0783),
        HN_ULONG_TO_UBASE(0xB42B708E), HN_ULONG_TO_UBASE(0xC3C56CFC),
        HN_ULONG_TO_UBASE(0x1C56846C), HN_ULONG_TO_UBASE(0xD522ABD8)
    },

    /* 2^e * 3G.y */
    {
        HN_ULONG_TO_UBASE(0xE35398F7), HN_ULONG_TO_UBASE(0x064452EE),
        HN_ULONG_TO_UBASE(0x29E400EC), HN_ULONG_TO_UBASE(0xEF936452),
        HN_ULONG_TO_UBASE(0x6D70CBB0), HN_ULONG_TO_UBASE(0x8FF8A03F)
    },

    /* 2^e * 4G.x */
    {
        HN_ULONG_TO_UBASE(0x26984CEF), HN_ULONG_TO_UBASE(0x2C537FA4),
        HN_ULONG_TO_UBASE(0x911E7D81), HN_ULONG_TO_UBASE(0xAAFA4F23),
        HN_ULONG_TO_UBASE(0x75E3554B), HN_ULONG_TO_UBASE(0x028D15E2)
    },

    /* 2^e * 4G.y */
    {
        HN_ULONG_TO_UBASE(0x99913442), HN_ULONG_TO_UBASE(0x4A23A729),
        HN_ULONG_TO_UBASE(0x6CF3E242), HN_ULONG_TO_UBASE(0x0EBA886E),
        HN_ULONG_TO_UBASE(0x9C451670), HN_ULONG_TO_UBASE(0x6FC74BCF)
    },

    /* 2^e * 5G.x */
    {
        HN_ULONG_TO_UBASE(0xF2406FA6), HN_ULONG_TO_UBASE(0x508E7692),
        HN_ULONG_TO_UBASE(0x027DD7D4), HN_ULONG_TO_UBASE(0xC52BF70D),
        HN_ULONG_TO_UBASE(0x5F1EFC4E), HN_ULONG_TO_UBASE(0x6B4A073F)
    },

    /* 2^e * 5G.y */
    {
        HN_ULONG_TO_UBASE(0xFD894650), HN_ULONG_TO_UBASE(0xC910CC89),
        HN_ULONG_TO_UBASE(0x2F982605), HN_ULONG_TO_UBASE(0x1CD5520D),
        HN_ULONG_TO_UBASE(0x9926A466), HN_ULONG_TO_UBASE(0x0CDEADEB)
    },

    /* 2^e * 6G.x */
    {
        HN_ULONG_TO_UBASE(0x15DFAAA5), HN_ULONG_TO_UBASE(0x401102B6),
        HN_ULONG_TO_UBASE(0xE3E453F3), HN_ULONG_TO_UBASE(0xEEA15E1C),
        HN_ULONG_TO_UBASE(0xD15E69CC), HN_ULONG_TO_UBASE(0x8BF49642)
    },

    /* 2^e * 6G.y */
    {
        HN_ULONG_TO_UBASE(0x9B3EBEDD), HN_ULONG_TO_UBASE(0x955C2C62),
        HN_ULONG_TO_UBASE(0xC7C9017E), HN_ULONG_TO_UBASE(0xABA0DED2),
        HN_ULONG_TO_UBASE(0x6F999205), HN_ULONG_TO_UBASE(0x957C2689)
    },

    /* 2^e * 7G.x */
    {
        HN_ULONG_TO_UBASE(0xED0A68AB), HN_ULONG_TO_UBASE(0x3F394D59),
        HN_ULONG_TO_UBASE(0x871DE2C7), HN_ULONG_TO_UBASE(0x921125DB),
        HN_ULONG_TO_UBASE(0xA64524B1), HN_ULONG_TO_UBASE(0xBA68427E)
    },

    /* 2^e * 7G.y */
    {
        HN_ULONG_TO_UBASE(0xF1B72362), HN_ULONG_TO_UBASE(0x5FE500A8),
        HN_ULONG_TO_UBASE(0x019F0084), HN_ULONG_TO_UBASE(0x47DB28B7),
        HN_ULONG_TO_UBASE(0x6F936172), HN_ULONG_TO_UBASE(0x97A78F13)
    },

    /* 2^e * 8G.x */
    {
        HN_ULONG_TO_UBASE(0xCBDF060C), HN_ULONG_TO_UBASE(0x15B007FF),
        HN_ULONG_TO_UBASE(0x6541F6F4), HN_ULONG_TO_UBASE(0x3B20E563),
        HN_ULONG_TO_UBASE(0x0C56D3C6), HN_ULONG_TO_UBASE(0x920F9A3C)
    },

    /* 2^e * 8G.y */
    {
        HN_ULONG_TO_UBASE(0x0D0028D3), HN_ULONG_TO_UBASE(0xDD7B77D0),
        HN_ULONG_TO_UBASE(0x5192D915), HN_ULONG_TO_UBASE(0xCEE00DDA),
        HN_ULONG_TO_UBASE(0x81F6B3B6), HN_ULONG_TO_UBASE(0xCD7902F7)
    },

    /* 2^e * 9G.x */
    {
        HN_ULONG_TO_UBASE(0x360628CD), HN_ULONG_TO_UBASE(0x81C8FF32),
        HN_ULONG_TO_UBASE(0x1A3704A7), HN_ULONG_TO_UBASE(0xCDF22E64),
        HN_ULONG_TO_UBASE(0xB83BF30D), HN_ULONG_TO_UBASE(0xFF915D40)
    },

    /* 2^e * 9G.y */
    {
        HN_ULONG_TO_UBASE(0x55876906), HN_ULONG_TO_UBASE(0xCFA29C4A),
        HN_ULONG_TO_UBASE(0x7E66ACA2), HN_ULONG_TO_UBASE(0xF8DC5DF3),
        HN_ULONG_TO_UBASE(0x10846EA4), HN_ULONG_TO_UBASE(0xD489FC2D)
    },

    /* 2^e * 10G.x */
    {
        HN_ULONG_TO_UBASE(0x660237B3), HN_ULONG_TO_UBASE(0x9DAB3F5F),
        HN_ULONG_TO_UBASE(0xA7C43E04), HN_ULONG_TO_UBASE(0xFFF8E01D),
        HN_ULONG_TO_UBASE(0xB60C1C4D), HN_ULONG_TO_UBASE(0xC432F4F1)
    },

    /* 2^e * 10G.y */
    {
        HN_ULONG_TO_UBASE(0xF065C804), HN_ULONG_TO_UBASE(0x07288012),
        HN_ULONG_TO_UBASE(0xBF214D87), HN_ULONG_TO_UBASE(0x0AABB920),
        HN_ULONG_TO_UBASE(0xB3B811E0), HN_ULONG_TO_UBASE(0x27EB5A71)
    },

    /* 2^e * 11G.x */
    {
        HN_ULONG_TO_UBASE(0x0C0F37E2), HN_ULONG_TO_UBASE(0x355701B7),
        HN_ULONG_TO_UBASE(0x23289564), HN_ULONG_TO_UBASE(0xBAEBE73F),
        HN_ULONG_TO_UBASE(0xB1631987), HN_ULONG_TO_UBASE(0xBC898ABF)
    },

    /* 2^e * 11G.y */
    {
        HN_ULONG_TO_UBASE(0x73037073), HN_ULONG_TO_UBASE(0x57453BA7),
        HN_ULONG_TO_UBASE(0x13E2013F), HN_ULONG_TO_UBASE(0x44DA9223),
        HN_ULONG_TO_UBASE(0x85DDF1D4), HN_ULONG_TO_UBASE(0xADD1BC83)
    },

    /* 2^e * 12G.x */
    {
        HN_ULONG_TO_UBASE(0x73A5BC11), HN_ULONG_TO_UBASE(0x158A8A8B),
        HN_ULONG_TO_UBASE(0xF78E7F80), HN_ULONG_TO_UBASE(0x8DCF5CFE),
        HN_ULONG_TO_UBASE(0x45E1F467), HN_ULONG_TO_UBASE(0xCF523028)
    },

    /* 2^e * 12G.y */
    {
        HN_ULONG_TO_UBASE(0x10966C87), HN_ULONG_TO_UBASE(0xB3D06D8A),
        HN_ULONG_TO_UBASE(0xC2569071), HN_ULONG_TO_UBASE(0x8D0FAFA5),
        HN_ULONG_TO_UBASE(0x7D227D75), HN_ULONG_TO_UBASE(0x89521916)
    },

    /* 2^e * 13G.x */
    {
        HN_ULONG_TO_UBASE(0xFB55FB12), HN_ULONG_TO_UBASE(0x659091CD),
        HN_ULONG_TO_UBASE(0x3E2EA468), HN_ULONG_TO_UBASE(0x50A85BEC),
        HN_ULONG_TO_UBASE(0x5801DFF4), HN_ULONG_TO_UBASE(0xECF1469E)
    },

    /* 2^e * 13G.y */
    {
        HN_ULONG_TO_UBASE(0x6294C83B), HN_ULONG_TO_UBASE(0xAEE2EE4D),
        HN_ULONG_TO_UBASE(0x910958E4), HN_ULONG_TO_UBASE(0x568D1C50),
        HN_ULONG_TO_UBASE(0x565BA314), HN_ULONG_TO_UBASE(0x87FEF198)
    },

    /* 2^e * 14G.x */
    {
        HN_ULONG_TO_UBASE(0x29230F47), HN_ULONG_TO_UBASE(0x3B1DCF03),
        HN_ULONG_TO_UBASE(0xD4971DEF), HN_ULONG_TO_UBASE(0x20E1E1AD),
        HN_ULONG_TO_UBASE(0xA6069364), HN_ULONG_TO_UBASE(0xBF863D0F)
    },

    /* 2^e * 14G.y */
    {
        HN_ULONG_TO_UBASE(0x74CCAE45), HN_ULONG_TO_UBASE(0x4187A9DF),
        HN_ULONG_TO_UBASE(0x62E227BE), HN_ULONG_TO_UBASE(0x60CD8E2B),
        HN_ULONG_TO_UBASE(0x47C3405E), HN_ULONG_TO_UBASE(0xA6043F75)
    },

    /* 2^e * 15G.x */
    {
        HN_ULONG_TO_UBASE(0x6E761455), HN_ULONG_TO_UBASE(0x40423BC2),
        HN_ULONG_TO_UBASE(0x023E182D), HN_ULONG_TO_UBASE(0x575E0245),
        HN_ULONG_TO_UBASE(0x7ACABD8A), HN_ULONG_TO_UBASE(0xA346AB39)
    },

    /* 2^e * 15G.y */
    {
        HN_ULONG_TO_UBASE(0x1C5A3750), HN_ULONG_TO_UBASE(0x688A166C),
        HN_ULONG_TO_UBASE(0x5780BE48), HN_ULONG_TO_UBASE(0xAB3E9AC3),
        HN_ULONG_TO_UBASE(0xAC72B5F3), HN_ULONG_TO_UBASE(0x2A1FAD07)
    }
};
static NX_CRYPTO_CONST NX_CRYPTO_EC_POINT secp192r1_fixed_points_array[] =
{

    /* 2G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[0],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[1],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 3G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[2],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[3],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 4G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[4],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[5],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 5G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[6],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[7],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 6G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[8],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[9],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 7G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[10],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[11],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 8G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[12],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[13],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 9G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[14],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[15],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 10G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[16],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[17],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 11G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[18],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[19],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 12G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[20],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[21],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 13G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[22],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[23],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 14G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[24],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[25],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 15G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[26],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_data[27],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    }
};
static NX_CRYPTO_CONST NX_CRYPTO_EC_POINT secp192r1_fixed_points_2e_array[] =
{

    /* 2^e * 1G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[0],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[1],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 2G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[2],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[3],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 3G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[4],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[5],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 4G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[6],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[7],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 5G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[8],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[9],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 6G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[10],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[11],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 7G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[12],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[13],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 8G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[14],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[15],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 9G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[16],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[17],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 10G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[18],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[19],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 11G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[20],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[21],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 12G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[22],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[23],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 13G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[24],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[25],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 14G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[26],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[27],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    },

    /* 2^e * 15G */
    {
        NX_CRYPTO_EC_POINT_AFFINE,
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[28],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {
            (HN_UBASE *)&secp192r1_fixed_points_2e_data[29],
            24 >> HN_SIZE_SHIFT, 24, (UINT)NX_CRYPTO_FALSE
        },
        {(HN_UBASE *)NX_CRYPTO_NULL, 0u, 0u, 0u}
    }
};


NX_CRYPTO_CONST NX_CRYPTO_EC_FIXED_POINTS _nx_crypto_ec_secp192r1_fixed_points =
{
    4u, 196u, 49u, 25u,
    (NX_CRYPTO_EC_POINT *)secp192r1_fixed_points_array,
    (NX_CRYPTO_EC_POINT *)secp192r1_fixed_points_2e_array
};

