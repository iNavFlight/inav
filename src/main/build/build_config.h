/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#ifdef UNIT_TEST
// make these visible to unit test
#define STATIC_UNIT_TESTED
#define STATIC_INLINE_UNIT_TESTED
#define INLINE_UNIT_TESTED
#define UNIT_TESTED
#else
#define STATIC_UNIT_TESTED static
#define STATIC_INLINE_UNIT_TESTED static inline
#define INLINE_UNIT_TESTED inline
#define UNIT_TESTED
#endif

//#define SOFT_I2C // enable to test software i2c

#ifndef __CC_ARM
#define REQUIRE_CC_ARM_PRINTF_SUPPORT
#define REQUIRE_PRINTF_LONG_SUPPORT
#endif

#ifdef __APPLE__
#define FASTRAM                     __attribute__ ((section("__DATA,__.fastram_bss"), aligned(8)))
#else
#define FASTRAM                     __attribute__ ((section(".fastram_bss"), aligned(4)))
#endif

/* FASTRAM that's only live during USB MSC boot mode - a one-way path
 * (mscWaitForButton(), fc/fc_init.c, only exits via reset), so it never
 * overlaps normal-mode FASTRAM (pid.c, gyro.c, imu.c, ...) in time. On
 * F405/F427 (FASTRAM=CCM) and AT32F43x (FASTRAM=RAM1) that's real
 * zero-wait-state space sitting idle outside MSC mode, so it's kept in its
 * own linker sub-section (stm32_flash.ld / at32_flash_f4_split.ld) instead of
 * the general FASTRAM pool - a future OVERLAY can reclaim it for a
 * normal-mode-only consumer, e.g.:
 *
 *   OVERLAY : NOLOAD {
 *     .fastram_bss_msc_only { *(.fastram_bss.msc_only) }  // existing, MSC-only
 *     .fastram_bss_my_thing { *(.fastram_bss.my_thing) }  // new, normal-mode-only
 *   } >FASTRAM
 *
 * ld sizes an OVERLAY as max() of its members, not sum() - .fastram_bss_my_thing
 * only costs space beyond what MSC already uses, not on top of it. No consumer
 * exists yet, this only reserves the space. F411/F446 (FASTRAM aliases plain
 * RAM, no separate bank) get plain FASTRAM here - nothing to reclaim.
 *
 * .fastram_bss_my_thing would sit outside the startup zero-fill loop too, same
 * as .fastram_bss_msc_only - whoever adds that consumer needs their own
 * explicit zero on the MSC path's model (FASTRAM_MSC_ONLY_NEEDS_EXPLICIT_ZERO,
 * mscZeroOnlyRegion() below), not bss zero-init.
 */
#ifdef __APPLE__
#define FASTRAM_MSC_ONLY             __attribute__ ((section("__DATA,__.fastram_bss_msc"), aligned(8)))
#else
#define FASTRAM_MSC_ONLY             __attribute__ ((section(".fastram_bss.msc_only"), aligned(4)))
#endif

// This sub-section sits ahead of .fastram_bss in the linker scripts
// (stm32_flash.ld / at32_flash_f4_split.ld), outside the range F405/F427's CCM
// zero-fill loop (startup_stm32f40[27]xx.s) covers - mscZeroOnlyRegion()
// (usb_msc_f4xx.c) zeroes it explicitly instead, on the MSC path only
// (fc/fc_init.c). AT32F43x already does its own explicit zeroing
// (usb_msc_at32f43x.c / emfat_file.c); F411/F446/F7/H7 don't need this at all.
#if defined(STM32F405xx) || defined(STM32F427_437xx)
#define FASTRAM_MSC_ONLY_NEEDS_EXPLICIT_ZERO
#endif

#if defined (STM32F4) || defined (STM32F7)
#define EXTENDED_FASTRAM FASTRAM
#else
#define EXTENDED_FASTRAM
#endif

#if defined (STM32H7)
#define DMA_RAM __attribute__ ((section(".DMA_RAM")))
#define SLOW_RAM __attribute__ ((section(".SLOW_RAM")))
#elif defined (AT32F43x)
#define DMA_RAM __attribute__ ((section(".DMA_RAM")))
#define SLOW_RAM __attribute__ ((section(".SLOW_RAM")))
#else
#define DMA_RAM
#define SLOW_RAM
#endif

#define STATIC_FASTRAM              static FASTRAM
#define STATIC_FASTRAM_UNIT_TESTED  STATIC_UNIT_TESTED FASTRAM
#define STATIC_FASTRAM_MSC_ONLY     static FASTRAM_MSC_ONLY
