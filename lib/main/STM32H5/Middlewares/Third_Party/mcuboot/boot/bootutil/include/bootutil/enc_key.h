/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2018-2019 JUUL Labs
 * Copyright (c) 2019 Arm Limited
 *
 * Original license:
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef BOOTUTIL_ENC_KEY_H
#define BOOTUTIL_ENC_KEY_H

#include <stdbool.h>
#include <stdint.h>
#include <flash_map_backend/flash_map_backend.h>
#include "bootutil/crypto/aes_ctr.h"
#include "bootutil/image.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOOT_ENC_KEY_SIZE       16
#define BOOT_ENC_KEY_SIZE_BITS  (BOOT_ENC_KEY_SIZE * 8)

#define TLV_ENC_RSA_SZ    256
#define TLV_ENC_KW_SZ     24
#define TLV_ENC_EC256_SZ  (65 + 32 + 16)
#define TLV_ENC_X25519_SZ (32 + 32 + 16)

#if defined(MCUBOOT_ENCRYPT_RSA)
#define BOOT_ENC_TLV_SIZE TLV_ENC_RSA_SZ
#elif defined(MCUBOOT_ENCRYPT_EC256)
#define BOOT_ENC_TLV_SIZE TLV_ENC_EC256_SZ
#elif defined(MCUBOOT_ENCRYPT_X25519)
#define BOOT_ENC_TLV_SIZE TLV_ENC_X25519_SZ
#else
#define BOOT_ENC_TLV_SIZE TLV_ENC_KW_SZ
#endif

#define BOOT_ENC_TLV_ALIGN_SIZE \
    ((((BOOT_ENC_TLV_SIZE - 1) / BOOT_MAX_ALIGN) + 1) * BOOT_MAX_ALIGN)

struct enc_key_data {
    uint8_t valid;
    bootutil_aes_ctr_context aes_ctr;
};

extern const struct bootutil_key bootutil_enc_key;
struct boot_status;

int boot_enc_init(struct enc_key_data *enc_state, uint8_t slot);
int boot_enc_drop(struct enc_key_data *enc_state, uint8_t slot);
int boot_enc_set_key(struct enc_key_data *enc_state, uint8_t slot,
        const struct boot_status *bs);
int boot_enc_load(struct enc_key_data *enc_state, int image_index,
        const struct image_header *hdr, const struct flash_area *fap,
        struct boot_status *bs);
int boot_enc_decrypt(const uint8_t *buf, uint8_t *enckey);
bool boot_enc_valid(struct enc_key_data *enc_state, int image_index,
        const struct flash_area *fap);
void boot_encrypt(struct enc_key_data *enc_state, int image_index,
        const struct flash_area *fap, uint32_t off, uint32_t sz,
        uint32_t blk_off, uint8_t *buf);
void boot_enc_zeroize(struct enc_key_data *enc_state);

#ifdef __cplusplus
}
#endif

#endif /* BOOTUTIL_ENC_KEY_H */
