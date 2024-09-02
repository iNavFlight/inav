/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 * Copyright (c) 2023 STMicroelectronics. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "bootutil/security_cnt.h"
#include "../../platform/include/plat_nv_counters.h"
#include "bootutil/fault_injection_hardening.h"
#include <stdint.h>

#define BOOT_NV_COUNTER_0    PLAT_NV_COUNTER_3   /* NV counter of Image 0 */
#define BOOT_NV_COUNTER_1    PLAT_NV_COUNTER_4   /* NV counter of Image 1 */
#define BOOT_NV_COUNTER_2    PLAT_NV_COUNTER_5   /* NV counter of Image 2 */
#define BOOT_NV_COUNTER_3    PLAT_NV_COUNTER_6   /* NV counter of Image 3 */
#define BOOT_NV_COUNTER_MAX  PLAT_NV_COUNTER_MAX

static enum nv_counter_t get_nv_counter_from_image_id(uint32_t image_id)
{
    uint32_t nv_counter;

    nv_counter = BOOT_NV_COUNTER_0 + image_id;

    /* Check the existence of the enumerated counter value */
    if (nv_counter >= BOOT_NV_COUNTER_MAX) {
        return BOOT_NV_COUNTER_MAX;
    }

    return (enum nv_counter_t)nv_counter;
}

fih_int boot_nv_security_counter_init(void)
{
    fih_int fih_rc = FIH_FAILURE;

    fih_rc = fih_int_encode_zero_equality(plat_init_nv_counter());

    FIH_RET(fih_rc);
}

fih_int boot_nv_security_counter_get(uint32_t image_id, fih_int *security_cnt)
{
    enum nv_counter_t nv_counter;
    fih_int fih_rc = FIH_FAILURE;
    uint32_t security_cnt_soft;

    /* Check if it's a null-pointer. */
    if (!security_cnt) {
        FIH_RET(FIH_FAILURE);
    }

    nv_counter = get_nv_counter_from_image_id(image_id);
    if (nv_counter == BOOT_NV_COUNTER_MAX) {
        FIH_RET(FIH_FAILURE);
    }

    fih_rc = fih_int_encode_zero_equality(
             plat_read_nv_counter(nv_counter,
                                  sizeof(security_cnt_soft),
                                  (uint8_t *)&security_cnt_soft));
    *security_cnt = fih_int_encode(security_cnt_soft);

    FIH_RET(fih_rc);
}

int32_t boot_nv_security_counter_update(uint32_t image_id,
                                        uint32_t img_security_cnt,
                                        uint32_t *updated)
{
    enum nv_counter_t nv_counter;
    HAL_StatusTypeDef err;

    nv_counter = get_nv_counter_from_image_id(image_id);
    if (nv_counter == BOOT_NV_COUNTER_MAX) {
        return -1;
    }

    err = plat_set_nv_counter(nv_counter, img_security_cnt, updated);
    if (err != HAL_OK) {
        return -1;
    }

    return 0;
}
