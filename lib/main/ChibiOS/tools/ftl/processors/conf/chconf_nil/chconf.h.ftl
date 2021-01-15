[#ftl]
[#--
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
  --]
[@pp.dropOutputFile /]
[#import "/@lib/libutils.ftl" as utils /]
[#import "/@lib/liblicense.ftl" as license /]
[@pp.changeOutputFile name="chconf.h" /]
/*
[@license.EmitLicenseAsText /]
*/

/**
 * @file    nil/templates/chconf.h
 * @brief   Configuration file template.
 * @details A copy of this file must be placed in each project directory, it
 *          contains the application specific kernel settings.
 *
 * @addtogroup NIL_CONFIG
 * @details Kernel related settings and hooks.
 * @{
 */

#ifndef CHCONF_H
#define CHCONF_H

#define _CHIBIOS_NIL_CONF_
#define _CHIBIOS_NIL_CONF_VER_3_2_

/*===========================================================================*/
/**
 * @name Kernel parameters and options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   Number of user threads in the application.
 * @note    This number is not inclusive of the idle thread which is
 *          Implicitly handled.
 */
#define CH_CFG_NUM_THREADS                  ${doc.CH_CFG_NUM_THREADS!"1"}

/** @} */

/*===========================================================================*/
/**
 * @name System timer settings
 * @{
 */
/*===========================================================================*/

/**
 * @brief   System time counter resolution.
 * @note    Allowed values are 16 or 32 bits.
 */
#define CH_CFG_ST_RESOLUTION                ${doc.CH_CFG_ST_RESOLUTION!"32"}

/**
 * @brief   System tick frequency.
 * @note    This value together with the @p CH_CFG_ST_RESOLUTION
 *          option defines the maximum amount of time allowed for
 *          timeouts.
 */
#define CH_CFG_ST_FREQUENCY                 ${doc.CH_CFG_ST_FREQUENCY!"1000"}

/**
 * @brief   Time delta constant for the tick-less mode.
 * @note    If this value is zero then the system uses the classic
 *          periodic tick. This value represents the minimum number
 *          of ticks that is safe to specify in a timeout directive.
 *          The value one is not valid, timeouts are rounded up to
 *          this value.
 */
#define CH_CFG_ST_TIMEDELTA                 ${doc.CH_CFG_ST_TIMEDELTA!"2"}

/** @} */

/*===========================================================================*/
/**
 * @name Subsystem options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   Semaphores APIs.
 * @details If enabled then the Semaphores APIs are included in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_SEMAPHORES               ${doc.CH_CFG_USE_SEMAPHORES!"TRUE"}

/**
 * @brief   Mutexes APIs.
 * @details If enabled then the mutexes APIs are included in the kernel.
 *
 * @note    Feature not currently implemented.
 * @note    The default is @p FALSE.
 */
#define CH_CFG_USE_MUTEXES                  ${doc.CH_CFG_USE_MUTEXES!"FALSE"}

/**
 * @brief   Events Flags APIs.
 * @details If enabled then the event flags APIs are included in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_EVENTS                   ${doc.CH_CFG_USE_EVENTS!"TRUE"}

/**
 * @brief   Mailboxes APIs.
 * @details If enabled then the asynchronous messages (mailboxes) APIs are
 *          included in the kernel.
 *
 * @note    The default is @p TRUE.
 * @note    Requires @p CH_CFG_USE_SEMAPHORES.
 */
#define CH_CFG_USE_MAILBOXES                ${doc.CH_CFG_USE_MAILBOXES!"TRUE"}

/**
 * @brief   Core Memory Manager APIs.
 * @details If enabled then the core memory manager APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_MEMCORE                  ${doc.CH_CFG_USE_MEMCORE!"TRUE"}

/**
 * @brief   Heap Allocator APIs.
 * @details If enabled then the memory heap allocator APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_HEAP                     ${doc.CH_CFG_USE_HEAP!"TRUE"}

/**
 * @brief   Memory Pools Allocator APIs.
 * @details If enabled then the memory pools allocator APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_MEMPOOLS                 ${doc.CH_CFG_USE_MEMPOOLS!"TRUE"}

/**
 * @brief  Objects FIFOs APIs.
 * @details If enabled then the objects FIFOs APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_OBJ_FIFOS                ${doc.CH_CFG_USE_OBJ_FIFOS!"TRUE"}

/**
 * @brief   Pipes APIs.
 * @details If enabled then the pipes APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_PIPES                    ${doc.CH_CFG_USE_PIPES!"TRUE"}

/**
 * @brief   Managed RAM size.
 * @details Size of the RAM area to be managed by the OS. If set to zero
 *          then the whole available RAM is used. The core memory is made
 *          available to the heap allocator and/or can be used directly through
 *          the simplified core memory allocator.
 *
 * @note    In order to let the OS manage the whole RAM the linker script must
 *          provide the @p __heap_base__ and @p __heap_end__ symbols.
 * @note    Requires @p CH_CFG_USE_MEMCORE.
 */
#define CH_CFG_MEMCORE_SIZE                 ${doc.CH_CFG_MEMCORE_SIZE!"0"}

/** @} */

/*===========================================================================*/
/**
 * @name Objects factory options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   Objects Factory APIs.
 * @details If enabled then the objects factory APIs are included in the
 *          kernel.
 *
 * @note    The default is @p FALSE.
 */
#define CH_CFG_USE_FACTORY                  ${doc.CH_CFG_USE_FACTORY!"TRUE"}

/**
 * @brief   Maximum length for object names.
 * @details If the specified length is zero then the name is stored by
 *          pointer but this could have unintended side effects.
 */
#define CH_CFG_FACTORY_MAX_NAMES_LENGTH     ${doc.CH_CFG_FACTORY_MAX_NAMES_LENGTH!"8"}

/**
 * @brief   Enables the registry of generic objects.
 */
#define CH_CFG_FACTORY_OBJECTS_REGISTRY     ${doc.CH_CFG_FACTORY_OBJECTS_REGISTRY!"TRUE"}

/**
 * @brief   Enables factory for generic buffers.
 */
#define CH_CFG_FACTORY_GENERIC_BUFFERS      ${doc.CH_CFG_FACTORY_GENERIC_BUFFERS!"TRUE"}

/**
 * @brief   Enables factory for semaphores.
 */
#define CH_CFG_FACTORY_SEMAPHORES           ${doc.CH_CFG_FACTORY_SEMAPHORES!"TRUE"}

/**
 * @brief   Enables factory for mailboxes.
 */
#define CH_CFG_FACTORY_MAILBOXES            ${doc.CH_CFG_FACTORY_MAILBOXES!"TRUE"}

/**
 * @brief   Enables factory for objects FIFOs.
 */
#define CH_CFG_FACTORY_OBJ_FIFOS            ${doc.CH_CFG_FACTORY_OBJ_FIFOS!"TRUE"}

/**
 * @brief   Enables factory for Pipes.
 */
#define CH_CFG_FACTORY_PIPES                ${doc.CH_CFG_FACTORY_PIPES!"TRUE"}

/** @} */

/*===========================================================================*/
/**
 * @name Debug options
 * @{
 */
/*===========================================================================*/

/**
 * @brief   Debug option, kernel statistics.
 *
 * @note    Feature not currently implemented.
 * @note    The default is @p FALSE.
 */
#define CH_DBG_STATISTICS                   ${doc.CH_DBG_STATISTICS!"FALSE"}

/**
 * @brief   Debug option, system state check.
 *
 * @note    The default is @p FALSE.
 */
#define CH_DBG_SYSTEM_STATE_CHECK           ${doc.CH_DBG_SYSTEM_STATE_CHECK!"FALSE"}

/**
 * @brief   Debug option, parameters checks.
 *
 * @note    The default is @p FALSE.
 */
#define CH_DBG_ENABLE_CHECKS                ${doc.CH_DBG_ENABLE_CHECKS!"FALSE"}

/**
 * @brief   System assertions.
 *
 * @note    The default is @p FALSE.
 */
#define CH_DBG_ENABLE_ASSERTS               ${doc.CH_DBG_ENABLE_ASSERTS!"FALSE"}

/**
 * @brief   Stack check.
 *
 * @note    The default is @p FALSE.
 */
#define CH_DBG_ENABLE_STACK_CHECK           ${doc.CH_DBG_ENABLE_STACK_CHECK!"FALSE"}

/** @} */

/*===========================================================================*/
/**
 * @name Kernel hooks
 * @{
 */
/*===========================================================================*/

/**
 * @brief   System initialization hook.
 */
#if !defined(CH_CFG_SYSTEM_INIT_HOOK) || defined(__DOXYGEN__)
#define CH_CFG_SYSTEM_INIT_HOOK() {                                         \
}
#endif

/**
 * @brief   Threads descriptor structure extension.
 * @details User fields added to the end of the @p thread_t structure.
 */
#define CH_CFG_THREAD_EXT_FIELDS                                            \
  /* Add threads custom fields here.*/

/**
 * @brief   Threads initialization hook.
 */
#define CH_CFG_THREAD_EXT_INIT_HOOK(tr) {                                   \
  /* Add custom threads initialization code here.*/                         \
}

/**
 * @brief   Idle thread enter hook.
 * @note    This hook is invoked within a critical zone, no OS functions
 *          should be invoked from here.
 * @note    This macro can be used to activate a power saving mode.
 */
#define CH_CFG_IDLE_ENTER_HOOK() {                                          \
}

/**
 * @brief   Idle thread leave hook.
 * @note    This hook is invoked within a critical zone, no OS functions
 *          should be invoked from here.
 * @note    This macro can be used to deactivate a power saving mode.
 */
#define CH_CFG_IDLE_LEAVE_HOOK() {                                          \
}

/**
 * @brief   System halt hook.
 */
#if !defined(CH_CFG_SYSTEM_HALT_HOOK) || defined(__DOXYGEN__)
#define CH_CFG_SYSTEM_HALT_HOOK(reason) {                                   \
}
#endif

/** @} */

/*===========================================================================*/
/* Port-specific settings (override port settings defaulted in nilcore.h).   */
/*===========================================================================*/

#endif  /* CHCONF_H */

/** @} */
