/*
 * Copyright (c) 2018-2021 Arm Limited
 * Copyright (c) 2023 STMicroelectronics
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

#ifndef __BOOT_RECORD_H__
#define __BOOT_RECORD_H__

#include <stdint.h>
#include "bootutil/image.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Add a data item to the shared data area between bootloader and
 *        runtime SW
 *
 * @param[in] major_type  TLV major type, identify consumer
 * @param[in] minor_type  TLV minor type, identify TLV type
 * @param[in] size        length of added data
 * @param[in] data        pointer to data
 *
 * @return                0 on success; nonzero on failure.
 */
int boot_add_data_to_shared_area(uint8_t        major_type,
                             uint16_t       minor_type,
                             size_t         size,
                             const uint8_t *data);
/**
 * @brief Add a data item to the shared data area between bootloader and
 *        runtime SW
 *
 * @param[in] major_type  TLV major type, identify consumer
 * @param[in] minor_type  TLV minor type, identify TLV type
 * @param[in] size        length of added data
 * @param[in] data        pointer to data
 *
 * @return                0 on success; nonzero on failure.
 */
int boot_add_data_to_shared_area(uint8_t        major_type,
                                 uint16_t       minor_type,
                                 size_t         size,
                                 const uint8_t *data);

/**
 * Add an image's all boot status information to the shared memory area
 * between the bootloader and runtime SW.
 *
 * @param[in]  sw_module  Identifier of the SW component.
 * @param[in]  hdr        Pointer to the image header stored in RAM.
 * @param[in]  fap        Pointer to the flash area where image is stored.
 *
 * @return                0 on success; nonzero on failure.
 */
int boot_save_boot_status(uint8_t sw_module,
                          const struct image_header *hdr,
                          const struct flash_area *fap);

/**
 * Add application specific data to the shared memory area between the
 * bootloader and runtime SW.
 *
 * @param[in]  hdr        Pointer to the image header stored in RAM.
 * @param[in]  fap        Pointer to the flash area where image is stored.
 *
 * @return                0 on success; nonzero on failure.
 */
int boot_save_shared_data(const struct image_header *hdr,
                          const struct flash_area *fap);

#ifdef __cplusplus
}
#endif

#endif /* __BOOT_RECORD_H__ */
