/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2017-2020 Arm Limited.
 * Copyright (c) 2023 STMicroelectronics.
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

#include "mcuboot_config/mcuboot_config.h"
#include <assert.h>
#if !defined (MCUBOOT_USE_HAL)
#include "mbedtls/memory_buffer_alloc.h"
#endif /* MCUBOOT_USE_HAL */
#include "bootutil/security_cnt.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "bootutil/boot_record.h"
#include "bootutil/fault_injection_hardening.h"
#include "flash_map_backend/flash_map_backend.h"
#include "boot_hal.h"
#include "uart_stdout.h"
#include "boot_hal.h"

/* Avoids the semihosting issue */
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
__asm("  .global __ARM_use_no_argv\n");
#endif

#if !defined (MCUBOOT_USE_HAL)
#ifdef MCUBOOT_ENCRYPT_RSA
#define BL2_MBEDTLS_MEM_BUF_LEN 0x2500
#else
#define BL2_MBEDTLS_MEM_BUF_LEN 0x2000
#endif

/* Static buffer to be used by mbedtls for memory allocation */
static uint8_t mbedtls_mem_buf[BL2_MBEDTLS_MEM_BUF_LEN];
#endif /* MCUBOOT_USE_HAL */

static void do_boot(struct boot_rsp *rsp)
{
    struct boot_arm_vector_table *vt;
    uintptr_t flash_base;
    int rc;

    /* The beginning of the image is the ARM vector table, containing
     * the initial stack pointer address and the reset vector
     * consecutively. Manually set the stack pointer and jump into the
     * reset vector
     */
    rc = flash_device_base(rsp->br_flash_dev_id, &flash_base);
    (void)rc;
    assert(rc == 0);

    if (rsp->br_hdr->ih_flags & IMAGE_F_RAM_LOAD) {
       /* The image has been copied to SRAM, find the vector table
        * at the load address instead of image's address in flash
        */
        vt = (struct boot_arm_vector_table *)(rsp->br_hdr->ih_load_addr +
                                         rsp->br_hdr->ih_hdr_size);
    } else {
        /* Using the flash address as not executing in SRAM */
        vt = (struct boot_arm_vector_table *)(flash_base +
                                         rsp->br_image_off +
                                         rsp->br_hdr->ih_hdr_size);
    }

#if MCUBOOT_LOG_LEVEL > MCUBOOT_LOG_LEVEL_OFF
    stdio_uninit();
#endif

    /* This function never returns, because it calls the secure application
     * Reset_Handler().
     */
    boot_platform_quit(vt);
}

int main(void)
{
    struct boot_rsp rsp;
    fih_int fih_rc = FIH_FAILURE;

#if !defined (MCUBOOT_USE_HAL)
    /* Initialise the mbedtls static memory allocator so that mbedtls allocates
     * memory from the provided static buffer instead of from the heap.
     */
    mbedtls_memory_buffer_alloc_init(mbedtls_mem_buf, BL2_MBEDTLS_MEM_BUF_LEN);
#endif /* MCUBOOT_USE_HAL */

#if MCUBOOT_LOG_LEVEL > MCUBOOT_LOG_LEVEL_OFF
    stdio_init();
#endif

    /* Perform platform specific initialization */
    if (boot_platform_init() != 0) {
        BOOT_LOG_ERR("Platform init failed");
        FIH_PANIC;
    }
#ifdef OEMUROT_ENABLE
    struct image_header *hdr = (struct image_header *)(FLASH_BASE_S + FLASH_AREA_BL2_OFFSET);
    BOOT_LOG_INF("Starting bootloader OEMuROT %x.%x.%x",hdr->ih_ver.iv_major, hdr->ih_ver.iv_minor, hdr->ih_ver.iv_revision);
#else
    BOOT_LOG_INF("Starting bootloader OEMiROT");
#endif

    FIH_CALL(boot_nv_security_counter_init, fih_rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BOOT_LOG_ERR("Error while initializing the security counter");
        FIH_PANIC;
    }

    FIH_CALL(boot_go, fih_rc, &rsp);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BOOT_LOG_ERR("Unable to find bootable image");
#if defined(MCUBOOT_EXT_LOADER)
        boot_platform_noimage();
#else
        FIH_PANIC;
#endif
    }

    BOOT_LOG_INF("Bootloader chainload address offset: 0x%x",
                 (int)rsp.br_image_off);
    BOOT_LOG_INF("Jumping to the first image slot");
    do_boot(&rsp);

    BOOT_LOG_ERR("Never should get here");
    FIH_PANIC;
}
