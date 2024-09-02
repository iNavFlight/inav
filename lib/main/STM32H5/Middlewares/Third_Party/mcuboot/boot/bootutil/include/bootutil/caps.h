/*
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

#ifndef H_BOOTUTIL_CAPS_H_
#define H_BOOTUTIL_CAPS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The bootloader can be compile with different capabilities selected
 * at compile time.  This function provides runtime access to these
 * capabilities.  This is intended primarily for testing, although
 * these will possibly be available at runtime to the application
 * running within the bootloader.
 */
uint32_t bootutil_get_caps(void);

#define BOOTUTIL_CAP_RSA2048                (1<<0)
#define BOOTUTIL_CAP_ECDSA_P224             (1<<1)
#define BOOTUTIL_CAP_ECDSA_P256             (1<<2)
#define BOOTUTIL_CAP_SWAP_USING_SCRATCH     (1<<3)
#define BOOTUTIL_CAP_OVERWRITE_UPGRADE      (1<<4)
#define BOOTUTIL_CAP_ENC_RSA                (1<<5)
#define BOOTUTIL_CAP_ENC_KW                 (1<<6)
#define BOOTUTIL_CAP_VALIDATE_PRIMARY_SLOT  (1<<7)
#define BOOTUTIL_CAP_RSA3072                (1<<8)
#define BOOTUTIL_CAP_ED25519                (1<<9)
#define BOOTUTIL_CAP_ENC_EC256              (1<<10)
#define BOOTUTIL_CAP_SWAP_USING_MOVE        (1<<11)
#define BOOTUTIL_CAP_DOWNGRADE_PREVENTION   (1<<12)
#define BOOTUTIL_CAP_ENC_X25519             (1<<13)
#define BOOTUTIL_CAP_BOOTSTRAP              (1<<14)

/*
 * Query the number of images this bootloader is configured for.  This
 * is also primarily used for testing.
 */
uint32_t bootutil_get_num_images(void);

#ifdef __cplusplus
}
#endif

#endif
