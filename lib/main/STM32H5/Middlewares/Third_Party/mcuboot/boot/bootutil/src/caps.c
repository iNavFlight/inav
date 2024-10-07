/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Linaro Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bootutil/caps.h>
#include "mcuboot_config/mcuboot_config.h"

uint32_t bootutil_get_caps(void)
{
    uint32_t res = 0;

#if defined(MCUBOOT_SIGN_RSA)
#if MCUBOOT_SIGN_RSA_LEN == 2048
    res |= BOOTUTIL_CAP_RSA2048;
#endif
#if MCUBOOT_SIGN_RSA_LEN == 3072
    res |= BOOTUTIL_CAP_RSA3072;
#endif
#endif
#if defined(MCUBOOT_SIGN_EC)
    res |= BOOTUTIL_CAP_ECDSA_P224;
#endif
#if defined(MCUBOOT_SIGN_EC256)
    res |= BOOTUTIL_CAP_ECDSA_P256;
#endif
#if defined(MCUBOOT_SIGN_ED25519)
    res |= BOOTUTIL_CAP_ED25519;
#endif
#if defined(MCUBOOT_OVERWRITE_ONLY)
    res |= BOOTUTIL_CAP_OVERWRITE_UPGRADE;
#elif defined(MCUBOOT_SWAP_USING_MOVE)
    res |= BOOTUTIL_CAP_SWAP_USING_MOVE;
#else
    res |= BOOTUTIL_CAP_SWAP_USING_SCRATCH;
#endif
#if defined(MCUBOOT_ENCRYPT_RSA)
    res |= BOOTUTIL_CAP_ENC_RSA;
#endif
#if defined(MCUBOOT_ENCRYPT_KW)
    res |= BOOTUTIL_CAP_ENC_KW;
#endif
#if defined(MCUBOOT_ENCRYPT_EC256)
    res |= BOOTUTIL_CAP_ENC_EC256;
#endif
#if defined(MCUBOOT_ENCRYPT_X25519)
    res |= BOOTUTIL_CAP_ENC_X25519;
#endif
#if defined(MCUBOOT_VALIDATE_PRIMARY_SLOT)
    res |= BOOTUTIL_CAP_VALIDATE_PRIMARY_SLOT;
#endif
#if defined(MCUBOOT_DOWNGRADE_PREVENTION)
    res |= BOOTUTIL_CAP_DOWNGRADE_PREVENTION;
#endif
#if defined(MCUBOOT_BOOTSTRAP)
    res |= BOOTUTIL_CAP_BOOTSTRAP;
#endif

    return res;
}

uint32_t bootutil_get_num_images(void)
{
#if defined(MCUBOOT_IMAGE_NUMBER)
    return MCUBOOT_IMAGE_NUMBER;
#else
    return 1;
#endif
}
