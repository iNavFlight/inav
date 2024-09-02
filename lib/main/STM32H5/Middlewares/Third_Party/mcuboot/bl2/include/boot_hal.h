/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 * Copyright (c) 2020-2023 STMicroelectronics. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __BOOT_HAL_H__
#define __BOOT_HAL_H__

/* Include header section */

#ifdef __cplusplus
extern "C" {
#endif

struct boot_arm_vector_table {
    uint32_t msp;
    uint32_t reset;
};

/*
 * \brief It clears that part of the RAM which was used by MCUBoot, expect the
 *        SHARED_DATA area, which is used to pass data to the next stage.
 *
 * \note  This function must be implemented per target platform by system
 *        integrator. If the bootloader has not loaded any secret to the shared
 *        RAM then this function can immediately return to shorten the boot-up
 *        time. Clearing RAM area can be done several way, it is platform
 *        dependent:
 *        - Overwritten with a pre-defined constant value (i.e.: 0).
 *        - Overwritten with a random value.
 *        - Change the secret if its location is known.
 *        - Set a register which can hide some part of the flash/RAM against
 *          next stage software components.
 *        - Etc.
 */
void boot_clear_bl2_ram_area(void);

/*
 * \brief Chain-loading the next image in the boot sequence.
 *        Can be overridden for platform specific initialization.
 * \param[in] jump function beeing call again
 * \param[in] reset_handler_addr Address of next image's Reset_Handler() in
                                 the boot chain
 */
void boot_jump_to_next_image(uint32_t boot_jump_addr, uint32_t reset_handler_addr);

/**
 * \brief Platform peripherals and devices initialization.
 *        Can be overridden for platform specific initialization.
 *
 * \return Returns 0 on success, non-zero otherwise
 */
int32_t boot_platform_init(void);

/**
 * \brief Platform operation to start secure image.
 *        Can be overridden for platform specific initialization.
 *
 * \param[in] vt  pointer to secure application vector table descriptor
 */
void boot_platform_quit(struct boot_arm_vector_table *vt) __NO_RETURN;


/**
 * \brief Platform operation to execute when no image is valid.
 *        Can be overridden for platform specific initialization.
 *
 */
void boot_platform_noimage(void) __NO_RETURN;

#ifdef __cplusplus
}
#endif

#endif /* __BOOT_HAL_H__ */
