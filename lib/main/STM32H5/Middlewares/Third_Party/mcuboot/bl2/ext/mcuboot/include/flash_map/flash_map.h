/*
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

/*
 * Original code taken from mcuboot project at:
 * https://github.com/mcu-tools/mcuboot
 * Git SHA of the original version: ac55554059147fff718015be9f4bd3108123f50a
 * Modifications are Copyright (c) 2018-2020 Arm Limited.
 *                   Copyright (c) 2023 STMicroelectronics.
 */

#ifndef H_UTIL_FLASH_MAP_
#define H_UTIL_FLASH_MAP_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * Provides abstraction of flash regions for type of use.
 * I.e. dude where's my image?
 *
 * System will contain a map which contains flash areas. Every
 * region will contain flash identifier, offset within flash and length.
 *
 * 1. This system map could be in a file within filesystem (Initializer
 * must know/figure out where the filesystem is at).
 * 2. Map could be at fixed location for project (compiled to code)
 * 3. Map could be at specific place in flash (put in place at mfg time).
 *
 * Note that the map you use must be valid for BSP it's for,
 * match the linker scripts when platform executes from flash,
 * and match the target offset specified in download script.
 */
#include <inttypes.h>
#include "region_defs.h"
#include "Driver_Flash.h"

/*
 * For now, we only support one flash device.
 *
 * Pick a random device ID for it that's unlikely to collide with
 * anything "real".
 */
#define FLASH_DEVICE_ID                 100
#define FLASH_DEVICE_BASE               FLASH_BASE_ADDRESS

/*
 * Shared data area between bootloader and runtime firmware.
 */
#if (defined(BOOT_SHARED_DATA_BASE) && defined(BOOT_SHARED_DATA_SIZE))
#define MCUBOOT_SHARED_DATA_BASE    BOOT_SHARED_DATA_BASE
#define MCUBOOT_SHARED_DATA_SIZE    BOOT_SHARED_DATA_SIZE
#else
#error "BOOT_SHARED_DATA_* must be defined by target."
#endif

/**
 * @brief Structure describing an area on a flash device.
 *
 * Multiple flash devices may be available in the system, each of
 * which may have its own areas. For this reason, flash areas track
 * which flash device they are part of.
 */
struct flash_area {
    /**
     * This flash area's ID; unique in the system.
     */
    uint8_t fa_id;

    /**
     * ID of the flash device this area is a part of.
     */
    uint8_t fa_device_id;

    uint16_t pad16;

    /**
     * Pointer to driver
     */
    ARM_DRIVER_FLASH *fa_driver;

    /**
     * This area's offset, relative to the beginning of its flash
     * device's storage.
     */
    uint32_t fa_off;

    /**
     * This area's size, in bytes.
     */
    uint32_t fa_size;
};

/**
 * @brief Structure describing a sector within a flash area.
 *
 * Each sector has an offset relative to the start of its flash area
 * (NOT relative to the start of its flash device), and a size. A
 * flash area may contain sectors with different sizes.
 */
struct flash_sector {
    /**
     * Offset of this sector, from the start of its flash area (not device).
     */
    uint32_t fs_off;

    /**
     * Size of this sector, in bytes.
     */
    uint32_t fs_size;
};

/**
 * @brief Macro retrieving driver from struct flash area
 *
 */
#define DRV_FLASH_AREA(area) ((area)->fa_driver)

/*
 * Start using flash area.
 */
int flash_area_open(uint8_t id, const struct flash_area **area);

void flash_area_close(const struct flash_area *area);

/*
 * Read/write/erase. Offset is relative from beginning of flash area.
 */
int flash_area_read(const struct flash_area *area, uint32_t off, void *dst,
                    uint32_t len);

int flash_area_write(const struct flash_area *area, uint32_t off,
                     const void *src, uint32_t len);

int flash_area_erase(const struct flash_area *area, uint32_t off, uint32_t len);

/*
 * Alignment restriction for flash writes.
 */
uint32_t flash_area_align(const struct flash_area *area);

/*
 * Given flash area ID, return info about sectors within the area.
 */
int flash_area_get_sectors(int fa_id, uint32_t *count,
  struct flash_sector *sectors);

/*
 * Similar to flash_area_get_sectors(), but return the values in an
 * array of struct flash_area instead.
 */
__attribute__((deprecated))
int flash_area_to_sectors(int idx, int *cnt, struct flash_area *ret);

#ifdef __cplusplus
}
#endif

#endif /* H_UTIL_FLASH_MAP_ */
