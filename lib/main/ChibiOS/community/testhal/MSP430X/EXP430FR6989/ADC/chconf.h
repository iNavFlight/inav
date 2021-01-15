/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    nilconf.h
 * @brief   Configuration file template.
 * @details A copy of this file must be placed in each project directory, it
 *          contains the application specific kernel settings.
 *
 * @addtogroup config
 * @details Kernel related settings and hooks.
 * @{
 */

#ifndef CHCONF_H
#define CHCONF_H

#define _CHIBIOS_NIL_CONF_

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
#define CH_CFG_NUM_THREADS                 1

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
#define CH_CFG_ST_RESOLUTION               16

/**
 * @brief   System tick frequency.
 * @note    This value together with the @p CH_CFG_ST_RESOLUTION
 *          option defines the maximum amount of time allowed for
 *          timeouts.
 */
#define CH_CFG_ST_FREQUENCY                1000

/**
 * @brief   Time delta constant for the tick-less mode.
 * @note    If this value is zero then the system uses the classic
 *          periodic tick. This value represents the minimum number
 *          of ticks that is safe to specify in a timeout directive.
 *          The value one is not valid, timeouts are rounded up to
 *          this value.
 */
#define CH_CFG_ST_TIMEDELTA                0

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
#define CH_CFG_USE_SEMAPHORES               TRUE

/**
 * @brief   Mutexes APIs.
 * @details If enabled then the mutexes APIs are included in the kernel.
 *
 * @note    Feature not currently implemented.
 * @note    The default is @p FALSE.
 */
#define CH_CFG_USE_MUTEXES                  FALSE

/**
 * @brief   Events Flags APIs.
 * @details If enabled then the event flags APIs are included in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_EVENTS                   TRUE

/**
 * @brief   Mailboxes APIs.
 * @details If enabled then the asynchronous messages (mailboxes) APIs are
 *          included in the kernel.
 *
 * @note    The default is @p TRUE.
 * @note    Requires @p CH_CFG_USE_SEMAPHORES.
 */
#define CH_CFG_USE_MAILBOXES                TRUE

/**
 * @brief   Core Memory Manager APIs.
 * @details If enabled then the core memory manager APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_MEMCORE                  TRUE

/**
 * @brief   Heap Allocator APIs.
 * @details If enabled then the memory heap allocator APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_HEAP                     TRUE

/**
 * @brief   Memory Pools Allocator APIs.
 * @details If enabled then the memory pools allocator APIs are included
 *          in the kernel.
 *
 * @note    The default is @p TRUE.
 */
#define CH_CFG_USE_MEMPOOLS                 TRUE

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
#define CH_CFG_MEMCORE_SIZE                 0

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
#define CH_DBG_STATISTICS                   FALSE

/**
 * @brief   Debug option, system state check.
 *
 * @note    The default is @p FALSE.
 */
#define CH_DBG_SYSTEM_STATE_CHECK           TRUE

/**
 * @brief   Debug option, parameters checks.
 *
 * @note    The default is @p FALSE.
 */
#define CH_DBG_ENABLE_CHECKS                TRUE

/**
 * @brief   System assertions.
 * 
 * @note    The default is @p FALSE.
 */
#define CH_DBG_ENABLE_ASSERTS              TRUE

/**
 * @brief   Stack check.
 * 
 *@note     The default is @p FALSE.
 */
#define CH_DBG_ENABLE_STACK_CHECK          TRUE

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
#define CH_CFG_SYSTEM_INIT_HOOK() {                                        \
}
#endif

/**
 * @brief   Threads descriptor structure extension.
 * @details User fields added to the end of the @p thread_t structure.
 */
#define CH_CFG_THREAD_EXT_FIELDS                                           \
  /* Add threads custom fields here.*/

/**
 * @brief   Threads initialization hook.
 */
#define CH_CFG_THREAD_EXT_INIT_HOOK(tr) {                                  \
  /* Add custom threads initialization code here.*/                         \
}

/**
 * @brief   Idle thread enter hook.
 * @note    This hook is invoked within a critical zone, no OS functions
 *          should be invoked from here.
 * @note    This macro can be used to activate a power saving mode.
 */
#define CH_CFG_IDLE_ENTER_HOOK() {                                         \
}

/**
 * @brief   Idle thread leave hook.
 * @note    This hook is invoked within a critical zone, no OS functions
 *          should be invoked from here.
 * @note    This macro can be used to deactivate a power saving mode.
 */
#define CH_CFG_IDLE_LEAVE_HOOK() {                                         \
}

/**
 * @brief   System halt hook.
 */
#if !defined(CH_CFG_SYSTEM_HALT_HOOK) || defined(__DOXYGEN__)
#define CH_CFG_SYSTEM_HALT_HOOK(reason) {                                  \
}
#endif

/** @} */

/*===========================================================================*/
/* Port-specific settings (override port settings defaulted in nilcore.h).   */
/*===========================================================================*/

#endif  /* _CHCONF_H_ */

/** @} */
