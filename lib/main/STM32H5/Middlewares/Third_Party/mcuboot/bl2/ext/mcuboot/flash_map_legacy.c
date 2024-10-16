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
 * Modifications are Copyright (c) 2019 Arm Limited.
 *                   Copyright (c) 2023 STMicroelectronics.
 */

#include "bootutil/bootutil_log.h"
#include "flash_map/flash_map.h"
#include <inttypes.h>

/*
 * Lookup the sector map for a given flash area.  This should fill in
 * `ret` with all of the sectors in the area.  `*cnt` will be set to
 * the storage at `ret` and should be set to the final number of
 * sectors in this area.
 */
int flash_area_get_sectors(int idx, uint32_t *cnt, struct flash_sector *ret)
{
    const struct flash_area *fa;
    uint32_t max_cnt = *cnt;
    uint32_t rem_len;
    int rc = -1;

    if (flash_area_open(idx, &fa)) {
        goto out;
    }

    BOOT_LOG_DBG("area %d: offset=0x%x, length=0x%x", idx, fa->fa_off,
                 fa->fa_size);

    if (*cnt < 1) {
        goto fa_close_out;
    }

    rem_len = fa->fa_size;
    *cnt = 0;
    while (rem_len > 0 && *cnt < max_cnt) {
        if (rem_len < FLASH_AREA_IMAGE_SECTOR_SIZE) {
            BOOT_LOG_ERR("area %d size 0x%x not divisible by sector size 0x%x",
                         idx, (int)fa->fa_size, FLASH_AREA_IMAGE_SECTOR_SIZE);
            goto fa_close_out;
        }

        ret[*cnt].fs_off = FLASH_AREA_IMAGE_SECTOR_SIZE * (*cnt);
        ret[*cnt].fs_size = FLASH_AREA_IMAGE_SECTOR_SIZE;
        *cnt = *cnt + 1;
        rem_len -= FLASH_AREA_IMAGE_SECTOR_SIZE;
    }

    if (*cnt > max_cnt) {
        BOOT_LOG_ERR("flash area %d sector count overflow", idx);
        goto fa_close_out;
    }

    rc = 0;

fa_close_out:
    flash_area_close(fa);
out:
    return rc;
}
