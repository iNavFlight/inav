/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 * Copyright (c) 2019-2020 Arm Limited.
 * Copyright (c) 2023 STMicroelectronics.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Original code taken from mcuboot project at:
 * https://github.com/mcu-tools/mcuboot
 * Git SHA of the original version: ac55554059147fff718015be9f4bd3108123f50a
 */

#include <errno.h>
#include "cmsis.h"
#include "Driver_Flash.h"
#include "sysflash/sysflash.h"
#include "flash_map/flash_map.h"
#include "flash_map_backend/flash_map_backend.h"
#include "bootutil/bootutil_log.h"

__WEAK int flash_device_base(uint8_t fd_id, uintptr_t *ret)
{
    if (fd_id == FLASH_DEVICE_ID) {
        *ret = FLASH_DEVICE_BASE;
    }
#if defined(EXT_FLASH_DEVICE_ID)
    else if (fd_id == EXT_FLASH_DEVICE_ID) {
        *ret = EXT_FLASH_BASE_ADDRESS;
    }
    else {
        BOOT_LOG_ERR("invalid flash ID %d; expected %d or %d",
                     fd_id, FLASH_DEVICE_ID, EXT_FLASH_DEVICE_ID);
        return -1;
    }
#else /* not EXT_FLASH_DEVICE_ID */
    else {
        BOOT_LOG_ERR("invalid flash ID %d; expected %d",
                     fd_id, FLASH_DEVICE_ID);
        return -1;
    }
#endif /* EXT_FLASH_DEVICE_ID */
    return 0;
}

/*
 * This depends on the mappings defined in flash_map.h.
 * MCUBoot uses continuous numbering for the primary slot, the secondary slot,
 * and the scratch while they can be numbered differently in other parts.
 */
int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    switch (slot) {
    case 0: return FLASH_AREA_IMAGE_PRIMARY(image_index);
#ifndef MCUBOOT_PRIMARY_ONLY
    case 1: return FLASH_AREA_IMAGE_SECONDARY(image_index);
    case 2: return FLASH_AREA_IMAGE_SCRATCH;
#endif
    }

    return -1; /* flash_area_open will fail on that */
}

int flash_area_id_from_image_slot(int slot)
{
    return flash_area_id_from_multi_image_slot(0, slot);
}

int flash_area_id_to_multi_image_slot(int image_index, int area_id)
{
    if (area_id == FLASH_AREA_IMAGE_PRIMARY(image_index)) {
        return 0;
    }
#ifndef MCUBOOT_PRIMARY_ONLY
    if (area_id == FLASH_AREA_IMAGE_SECONDARY(image_index)) {
        return 1;
    }
#endif
    BOOT_LOG_ERR("invalid flash area ID");
    return -1;
}

int flash_area_id_to_image_slot(int area_id)
{
    return flash_area_id_to_multi_image_slot(0, area_id);
}

uint8_t flash_area_erased_val(const struct flash_area *fap)
{
    return DRV_FLASH_AREA(fap)->GetInfo()->erased_value;
}

int flash_area_read_is_empty(const struct flash_area *fa, uint32_t off,
        void *dst, uint32_t len)
{
    uint32_t i;
    uint8_t *u8dst;
    int rc;

    BOOT_LOG_DBG("read_is_empty area=%d, off=%#x, len=%#x",
                 fa->fa_id, off, len);

    rc = DRV_FLASH_AREA(fa)->ReadData(fa->fa_off + off, dst, len);
    if (rc) {
        return -1;
    }

    u8dst = (uint8_t*)dst;

    for (i = 0; i < len; i++) {
        if (u8dst[i] != flash_area_erased_val(fa)) {
            return 0;
        }
    }

    return 1;
}
