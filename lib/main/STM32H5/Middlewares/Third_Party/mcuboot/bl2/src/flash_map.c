/*
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 * Copyright (c) 2023 STMicroelectronics. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdbool.h>
#include "flash_map/flash_map.h"
#include "flash_map_backend/flash_map_backend.h"
#include "bootutil_priv.h"
#include "bootutil/bootutil_log.h"
#include "Driver_Flash.h"

/* When undefined FLASH_DEV_NAME_0 or FLASH_DEVICE_ID_0 , default */
#if !defined(FLASH_DEV_NAME_0) || !defined(FLASH_DEVICE_ID_0)
#define FLASH_DEV_NAME_0  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_0 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_1 or FLASH_DEVICE_ID_1 , default */
#if !defined(FLASH_DEV_NAME_1) || !defined(FLASH_DEVICE_ID_1)
#define FLASH_DEV_NAME_1  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_1 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_2 or FLASH_DEVICE_ID_2 , default */
#if !defined(FLASH_DEV_NAME_2) || !defined(FLASH_DEVICE_ID_2)
#define FLASH_DEV_NAME_2  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_2 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_3 or FLASH_DEVICE_ID_3 , default */
#if !defined(FLASH_DEV_NAME_3) || !defined(FLASH_DEVICE_ID_3)
#define FLASH_DEV_NAME_3  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_3 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_4 or FLASH_DEVICE_ID_4 , default */
#if !defined(FLASH_DEV_NAME_4) || !defined(FLASH_DEVICE_ID_4)
#define FLASH_DEV_NAME_4  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_4 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_5 or FLASH_DEVICE_ID_5 , default */
#if !defined(FLASH_DEV_NAME_5) || !defined(FLASH_DEVICE_ID_5)
#define FLASH_DEV_NAME_5  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_5 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_6 or FLASH_DEVICE_ID_6 , default */
#if !defined(FLASH_DEV_NAME_6) || !defined(FLASH_DEVICE_ID_6)
#define FLASH_DEV_NAME_6  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_6 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_7 or FLASH_DEVICE_ID_7 , default */
#if !defined(FLASH_DEV_NAME_7) || !defined(FLASH_DEVICE_ID_7)
#define FLASH_DEV_NAME_7  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_7 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_SCRATCH or FLASH_DEVICE_ID_SCRATCH , default */
#if !defined(FLASH_DEV_NAME_SCRATCH) || !defined(FLASH_DEVICE_ID_SCRATCH)
#define FLASH_DEV_NAME_SCRATCH  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_SCRATCH FLASH_DEVICE_ID
#endif

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

/* Flash device names must be specified by target */
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_0;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_1;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_2;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_3;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_4;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_5;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_6;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_7;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_SCRATCH;

static const struct flash_area flash_map[] = {
    {
        .fa_id = FLASH_AREA_0_ID,
        .fa_device_id = FLASH_DEVICE_ID_0,
        .fa_driver = &FLASH_DEV_NAME_0,
        .fa_off = FLASH_AREA_0_OFFSET,
        .fa_size = FLASH_AREA_0_SIZE,
    },
#ifndef MCUBOOT_PRIMARY_ONLY
    {
        .fa_id = FLASH_AREA_2_ID,
        .fa_device_id = FLASH_DEVICE_ID_2,
        .fa_driver = &FLASH_DEV_NAME_2,
        .fa_off = FLASH_AREA_2_OFFSET,
        .fa_size = FLASH_AREA_2_SIZE,
    },
#endif
#if (MCUBOOT_APP_IMAGE_NUMBER == 2)
    {
        .fa_id = FLASH_AREA_1_ID,
        .fa_device_id = FLASH_DEVICE_ID_1,
        .fa_driver = &FLASH_DEV_NAME_1,
        .fa_off = FLASH_AREA_1_OFFSET,
        .fa_size = FLASH_AREA_1_SIZE,
    },
#ifndef MCUBOOT_PRIMARY_ONLY
    {
        .fa_id = FLASH_AREA_3_ID,
        .fa_device_id = FLASH_DEVICE_ID_3,
        .fa_driver = &FLASH_DEV_NAME_3,
        .fa_off = FLASH_AREA_3_OFFSET,
        .fa_size = FLASH_AREA_3_SIZE,
    },
#endif
#endif
#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
    {
        .fa_id = FLASH_AREA_4_ID,
        .fa_device_id = FLASH_DEVICE_ID_4,
        .fa_driver = &FLASH_DEV_NAME_4,
        .fa_off = FLASH_AREA_4_OFFSET,
        .fa_size = FLASH_AREA_4_SIZE,
    },
#ifndef MCUBOOT_PRIMARY_ONLY
    {
        .fa_id = FLASH_AREA_6_ID,
        .fa_device_id = FLASH_DEVICE_ID_6,
        .fa_driver = &FLASH_DEV_NAME_6,
        .fa_off = FLASH_AREA_6_OFFSET,
        .fa_size = FLASH_AREA_6_SIZE,
    },
#endif
#endif
#if (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1)
    {
        .fa_id = FLASH_AREA_5_ID,
        .fa_device_id = FLASH_DEVICE_ID_5,
        .fa_driver = &FLASH_DEV_NAME_5,
        .fa_off = FLASH_AREA_5_OFFSET,
        .fa_size = FLASH_AREA_5_SIZE,
    },
#ifndef MCUBOOT_PRIMARY_ONLY
    {
        .fa_id = FLASH_AREA_7_ID,
        .fa_device_id = FLASH_DEVICE_ID_7,
        .fa_driver = &FLASH_DEV_NAME_7,
        .fa_off = FLASH_AREA_7_OFFSET,
        .fa_size = FLASH_AREA_7_SIZE,
    },
#endif
#endif
#ifndef MCUBOOT_PRIMARY_ONLY
    {
        .fa_id = FLASH_AREA_SCRATCH_ID,
        .fa_device_id = FLASH_DEVICE_ID_SCRATCH,
        .fa_driver = &FLASH_DEV_NAME_SCRATCH,
        .fa_off = FLASH_AREA_SCRATCH_OFFSET,
        .fa_size = FLASH_AREA_SCRATCH_SIZE,
    },
#endif
};

static const int flash_map_entry_num = ARRAY_SIZE(flash_map);

/*
 * Check the target address in the flash_area_xxx operation.
 */
static bool is_range_valid(const struct flash_area *area,
                           uint32_t off,
                           uint32_t len)
{
    uint32_t size;

    if (!area) {
        return false;
    }

    if (!boot_u32_safe_add(&size, off, len)) {
        return false;
    }

    if (area->fa_size < size) {
        return false;
    }

    return true;
}

/*
 * `open` a flash area.  The `area` in this case is not the individual
 * sectors, but describes the particular flash area in question.
 */
int flash_area_open(uint8_t id, const struct flash_area **area)
{
    int i;

    BOOT_LOG_DBG("area %d", id);

    for (i = 0; i < flash_map_entry_num; i++) {
        if (id == flash_map[i].fa_id) {
            break;
        }
    }
    if (i == flash_map_entry_num) {
        return -1;
    }

    *area = &flash_map[i];
    return 0;
}

void flash_area_close(const struct flash_area *area)
{
    /* Nothing to do. */
}

int flash_area_read(const struct flash_area *area, uint32_t off, void *dst,
                    uint32_t len)
{
    BOOT_LOG_DBG("read area=%d, off=%#x, len=%#x", area->fa_id, off, len);

    if (!is_range_valid(area, off, len)) {
        return -1;
    }

    return DRV_FLASH_AREA(area)->ReadData(area->fa_off + off, dst, len);
}

int flash_area_write(const struct flash_area *area, uint32_t off,
                     const void *src, uint32_t len)
{
    BOOT_LOG_DBG("write area=%d, off=%#x, len=%#x", area->fa_id, off, len);

    if (!is_range_valid(area, off, len)) {
        return -1;
    }

    return DRV_FLASH_AREA(area)->ProgramData(area->fa_off + off, src, len);
}

int flash_area_erase(const struct flash_area *area, uint32_t off, uint32_t len)
{
    ARM_FLASH_INFO *flash_info;
    uint32_t deleted_len = 0;
    int32_t rc = 0;

    BOOT_LOG_DBG("erase area=%d, off=%#x, len=%#x", area->fa_id, off, len);

    if (!is_range_valid(area, off, len)) {
        return -1;
    }

    flash_info = DRV_FLASH_AREA(area)->GetInfo();

    if (flash_info->sector_info == NULL) {
        /* Uniform sector layout */
        while (deleted_len < len) {
            rc = DRV_FLASH_AREA(area)->EraseSector(area->fa_off + off);
            if (rc != 0) {
                break;
            }
            deleted_len += flash_info->sector_size;
            off         += flash_info->sector_size;
        }
    } else {
        /* Inhomogeneous sector layout, explicitly defined
         * Currently not supported.
         */
    }

    return rc;
}

uint32_t flash_area_align(const struct flash_area *area)
{
    ARM_FLASH_INFO *flash_info;

    flash_info = DRV_FLASH_AREA(area)->GetInfo();
    return flash_info->program_unit;
}
