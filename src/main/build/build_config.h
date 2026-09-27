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

/* Data that's only live during USB MSC boot mode (the MSC data buffer, the emfat
 * log directory). MSC mode replaces the entire boot sequence - mscWaitForButton()
 * (fc/fc_init.c) only ever exits via NVIC_SystemReset() - so nothing tagged with
 * this macro can be live at the same time as normal-mode FASTRAM data (pid.c,
 * gyro.c, imu.c, ...); see the comment at the USE_USB_MSC block in fc/fc_init.c.
 *
 * On F405/F427 (FASTRAM = CCM) and AT32F43x (FASTRAM = RAM1), FASTRAM is a
 * physically separate bank from the one .bss/stack live in, so this data is kept
 * in its own linker sub-section there (see FASTRAM_MSC_ONLY budget ASSERT in
 * stm32_flash.ld / at32_flash_f4_split.ld) instead of being anonymously mixed
 * into the general FASTRAM pool. That gives it a well-defined size and address
 * range, which is what a future OVERLAY with a normal-mode-only FASTRAM consumer
 * would need to reclaim the space. No consumer exists yet - this only reserves
 * and documents the opportunity. On chips where FASTRAM isn't a separate bank
 * (F411/F446: FASTRAM aliases plain RAM) this is just FASTRAM, since there's no
 * distinct pool to carve out or reclaim.
 */
#ifdef __APPLE__
#define FASTRAM_MSC_ONLY             __attribute__ ((section("__DATA,__.fastram_bss_msc"), aligned(8)))
#else
#define FASTRAM_MSC_ONLY             __attribute__ ((section(".fastram_bss.msc_only"), aligned(4)))
#endif

/* True only where FASTRAM_MSC_ONLY actually lands in its own linker section
 * (stm32_flash.ld / at32_flash_f4_split.ld both define __fastram_msc_only_start__/
 * __fastram_msc_only_end__ - see those files). F405/F427's CCM zero-fill loop in
 * startup_stm32f40[27]xx.s only covers __fastram_bss_start__/__end__, which no
 * longer includes this carved-out sub-section, so mscInit() must zero it
 * explicitly (usb_msc_f4xx.c) instead of relying on that loop. AT32F43x never had
 * a zero-fill loop for FASTRAM at all - its existing explicit memsets in
 * usb_msc_at32f43x.c / emfat_file.c already cover this, unaffected by the section
 * rename. F411/F446/F7/H7 don't define this: FASTRAM_MSC_ONLY there either aliases
 * plain RAM (already covered by the ordinary .bss path) or isn't used at all.
 */
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
