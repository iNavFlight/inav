/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 * Copyright (c) 2023 STMicroelectronics. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <string.h>
#include "bootutil/boot_record.h"
#include "bootutil/boot_status.h"
#include "bootutil/image.h"
#include "flash_map/flash_map.h"
#include "sysflash/sysflash.h"

/* Firmware Update specific macros */
#define TLV_MAJOR_FWU   0x2
#define MODULE_MASK     0x3F           /* 6 bit */
#define CLAIM_MASK      0x3F           /* 6 bit */

#define SET_FWU_MINOR(sw_module, claim)  \
                     ((uint16_t)((sw_module & MODULE_MASK) << 6) | \
                      (uint16_t)(claim & CLAIM_MASK))

extern int
boot_add_data_to_shared_area(uint8_t        major_type,
                             uint16_t       minor_type,
                             size_t         size,
                             const uint8_t *data);

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
                          const struct flash_area *fap)
{
    uint16_t fwu_minor;
    struct image_version image_ver;
    const struct flash_area *temp_fap;
    uint8_t mcuboot_image_id = 0;
    uint8_t i;

    if (hdr == NULL || fap == NULL) {
        return -1;
    }

    for (i = 0; i < MCUBOOT_IMAGE_NUMBER; i++) {
        if (flash_area_open(FLASH_AREA_IMAGE_PRIMARY(i),
                            &temp_fap) != 0) {
            return -1;
        }

        if (fap == temp_fap) {
            mcuboot_image_id = i;
            break;
        }
    }

    if (i == MCUBOOT_IMAGE_NUMBER) {
        return -1;
    }

    image_ver = hdr->ih_ver;

    /* TODO: add the module identifier into the fwu_minor after the module
     * arg is supported.
     */
    /* Currently hardcode it to 0 which indicates the full image. */
    fwu_minor = SET_FWU_MINOR(mcuboot_image_id, SW_VERSION);
    return boot_add_data_to_shared_area(TLV_MAJOR_FWU,
                                        fwu_minor,
                                        sizeof(image_ver),
                                        (const uint8_t *)&image_ver);
}
