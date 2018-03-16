/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    pal.h
 * @brief   I/O Ports Abstraction Layer macros, types and structures.
 *
 * @addtogroup PAL
 * @{
 */

#ifndef _PAL_H_
#define _PAL_H_

#if (HAL_USE_PAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Pads mode constants
 * @{
 */
/**
 * @brief   After reset state.
 * @details The state itself is not specified and is architecture dependent,
 *          it is guaranteed to be equal to the after-reset state. It is
 *          usually an input state.
 */
#define PAL_MODE_RESET                  0U

/**
 * @brief   Safe state for <b>unconnected</b> pads.
 * @details The state itself is not specified and is architecture dependent,
 *          it may be mapped on @p PAL_MODE_INPUT_PULLUP,
 *          @p PAL_MODE_INPUT_PULLDOWN or @p PAL_MODE_OUTPUT_PUSHPULL for
 *          example.
 */
#define PAL_MODE_UNCONNECTED            1U

/**
 * @brief   Regular input high-Z pad.
 */
#define PAL_MODE_INPUT                  2U

/**
 * @brief   Input pad with weak pull up resistor.
 */
#define PAL_MODE_INPUT_PULLUP           3U

/**
 * @brief   Input pad with weak pull down resistor.
 */
#define PAL_MODE_INPUT_PULLDOWN         4U

/**
 * @brief   Analog input mode.
 */
#define PAL_MODE_INPUT_ANALOG           5U

/**
 * @brief   Push-pull output pad.
 */
#define PAL_MODE_OUTPUT_PUSHPULL        6U

/**
 * @brief   Open-drain output pad.
 */
#define PAL_MODE_OUTPUT_OPENDRAIN       7U
/** @} */

/**
 * @name    Logic level constants
 * @{
 */
/**
 * @brief   Logical low state.
 */
#define PAL_LOW                         0U

/**
 * @brief   Logical high state.
 */
#define PAL_HIGH                        1U
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

#include "pal_lld.h"

/**
 * @brief   I/O bus descriptor.
 * @details This structure describes a group of contiguous digital I/O lines
 *          that have to be handled as bus.
 * @note    I/O operations on a bus do not affect I/O lines on the same port but
 *          not belonging to the bus.
 */
typedef struct {
  /**
   * @brief Port identifier.
   */
  ioportid_t            portid;
  /**
   * @brief Bus mask aligned to port bit 0.
   * @note  The bus mask implicitly define the bus width. A logic AND is
   *        performed on the bus data.
   */
  ioportmask_t          mask;
  /**
   * @brief Offset, within the port, of the least significant bit of the bus.
   */
  uint_fast8_t          offset;
} IOBus;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Port bit helper macro.
 * @details This macro calculates the mask of a bit within a port.
 *
 * @param[in] n         bit position within the port
 * @return              The bit mask.
 */
#if !defined(PAL_PORT_BIT) || defined(__DOXYGEN__)
#define PAL_PORT_BIT(n) ((ioportmask_t)(1U << (n)))
#endif

/**
 * @brief   Bits group mask helper.
 * @details This macro calculates the mask of a bits group.
 *
 * @param[in] width         group width
 * @return                  The group mask.
 */
#if !defined(PAL_GROUP_MASK) || defined(__DOXYGEN__)
#define PAL_GROUP_MASK(width) ((ioportmask_t)(1U << (width)) - 1U)
#endif

/**
 * @brief   Data part of a static I/O bus initializer.
 * @details This macro should be used when statically initializing an I/O bus
 *          that is part of a bigger structure.
 *
 * @param[in] name      name of the IOBus variable
 * @param[in] port      I/O port descriptor
 * @param[in] width     bus width in bits
 * @param[in] offset     bus bit offset within the port
 */
#define _IOBUS_DATA(name, port, width, offset)                              \
  {port, PAL_GROUP_MASK(width), offset}

/**
 * @brief   Static I/O bus initializer.
 *
 * @param[in] name      name of the IOBus variable
 * @param[in] port      I/O port descriptor
 * @param[in] width     bus width in bits
 * @param[in] offset    bus bit offset within the port
 */
#define IOBUS_DECL(name, port, width, offset)                               \
  IOBus name = _IOBUS_DATA(name, port, width, offset)

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   PAL subsystem initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @param[in] config pointer to an architecture specific configuration
 *            structure. This structure is defined in the low level driver
 *            header.
 *
 * @init
 */
#define palInit(config) pal_lld_init(config)

/**
 * @brief   Reads the physical I/O port states.
 * @note    The default implementation always return zero and computes the
 *          parameter eventual side effects.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @return              The port logic states.
 *
 * @special
 */
#if !defined(pal_lld_readport) || defined(__DOXYGEN__)
#define palReadPort(port) ((void)(port), 0U)
#else
#define palReadPort(port) pal_lld_readport(port)
#endif

/**
 * @brief   Reads the output latch.
 * @details The purpose of this function is to read back the latched output
 *          value.
 * @note    The default implementation always return zero and computes the
 *          parameter eventual side effects.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @return              The latched logic states.
 *
 * @special
 */
#if !defined(pal_lld_readlatch) || defined(__DOXYGEN__)
#define palReadLatch(port) ((void)(port), 0U)
#else
#define palReadLatch(port) pal_lld_readlatch(port)
#endif

/**
 * @brief   Writes a bits mask on a I/O port.
 * @note    The default implementation does nothing except computing the
 *          parameters eventual side effects.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be written on the specified port
 *
 * @special
 */
#if !defined(pal_lld_writeport) || defined(__DOXYGEN__)
#define palWritePort(port, bits) ((void)(port), (void)(bits))
#else
#define palWritePort(port, bits) pal_lld_writeport(port, bits)
#endif

/**
 * @brief   Sets a bits mask on a I/O port.
 * @note    The operation is not guaranteed to be atomic on all the
 *          architectures, for atomicity and/or portability reasons you may
 *          need to enclose port I/O operations between @p osalSysLock() and
 *          @p osalSysUnlock().
 * @note    The default implementation is non atomic and not necessarily
 *          optimal. Low level drivers may  optimize the function by using
 *          specific hardware or coding.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be ORed on the specified port
 *
 * @special
 */
#if !defined(pal_lld_setport) || defined(__DOXYGEN__)
#define palSetPort(port, bits)                                              \
  palWritePort(port, palReadLatch(port) | (bits))
#else
#define palSetPort(port, bits) pal_lld_setport(port, bits)
#endif

/**
 * @brief   Clears a bits mask on a I/O port.
 * @note    The operation is not guaranteed to be atomic on all the
 *          architectures,  for atomicity and/or portability reasons you may
 *          need to enclose port I/O operations between @p osalSysLock() and
 *          @p osalSysUnlock().
 * @note    The default implementation is non atomic and not necessarily
 *          optimal. Low level drivers may  optimize the function by using
 *          specific hardware or coding.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be cleared on the specified port
 *
 * @special
 */
#if !defined(pal_lld_clearport) || defined(__DOXYGEN__)
#define palClearPort(port, bits)                                            \
  palWritePort(port, palReadLatch(port) & ~(bits))
#else
#define palClearPort(port, bits) pal_lld_clearport(port, bits)
#endif

/**
 * @brief   Toggles a bits mask on a I/O port.
 * @note    The operation is not guaranteed to be atomic on all the
 *          architectures, for atomicity and/or portability reasons you may
 *          need to enclose port I/O operations between @p osalSysLock() and
 *          @p osalSysUnlock().
 * @note    The default implementation is non atomic and not necessarily
 *          optimal. Low level drivers may  optimize the function by using
 *          specific hardware or coding.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be XORed on the specified port
 *
 * @special
 */
#if !defined(pal_lld_toggleport) || defined(__DOXYGEN__)
#define palTogglePort(port, bits)                                           \
  palWritePort(port, palReadLatch(port) ^ (bits))
#else
#define palTogglePort(port, bits) pal_lld_toggleport(port, bits)
#endif

/**
 * @brief   Reads a group of bits.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask, a logic AND is performed on the input
 *                      data
 * @param[in] offset    group bit offset within the port
 * @return              The group logic states.
 *
 * @special
 */
#if !defined(pal_lld_readgroup) || defined(__DOXYGEN__)
#define palReadGroup(port, mask, offset)                                    \
  ((palReadPort(port) >> (offset)) & (mask))
#else
#define palReadGroup(port, mask, offset) pal_lld_readgroup(port, mask, offset)
#endif

/**
 * @brief   Writes a group of bits.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask, a logic AND is performed on the
 *                      output  data
 * @param[in] offset    group bit offset within the port
 * @param[in] bits      bits to be written. Values exceeding the group
 *                      width are masked.
 *
 * @special
 */
#if !defined(pal_lld_writegroup) || defined(__DOXYGEN__)
#define palWriteGroup(port, mask, offset, bits)                             \
  palWritePort(port, (palReadLatch(port) & ~((mask) << (offset))) |         \
                     (((bits) & (mask)) << (offset)))
#else
#define palWriteGroup(port, mask, offset, bits)                             \
  pal_lld_writegroup(port, mask, offset, bits)
#endif


/**
 * @brief   Pads group mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 * @note    Programming an unknown or unsupported mode is silently ignored.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask
 * @param[in] offset    group bit offset within the port
 * @param[in] mode      group mode
 *
 * @special
 */
#if !defined(pal_lld_setgroupmode) || defined(__DOXYGEN__)
#define palSetGroupMode(port, mask, offset, mode)
#else
#define palSetGroupMode(port, mask, offset, mode)                           \
  pal_lld_setgroupmode(port, mask, offset, mode)
#endif

/**
 * @brief   Reads an input pad logic state.
 * @note    The default implementation not necessarily optimal. Low level
 *          drivers may  optimize the function by using specific hardware
 *          or coding.
 * @note    The default implementation internally uses the @p palReadPort().
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @return              The logic state.
 * @retval PAL_LOW      low logic state.
 * @retval PAL_HIGH     high logic state.
 *
 * @special
 */
#if !defined(pal_lld_readpad) || defined(__DOXYGEN__)
#define palReadPad(port, pad) ((palReadPort(port) >> (pad)) & 1U)
#else
#define palReadPad(port, pad) pal_lld_readpad(port, pad)
#endif

/**
 * @brief   Writes a logic state on an output pad.
 * @note    The operation is not guaranteed to be atomic on all the
 *          architectures, for atomicity and/or portability reasons you may
 *          need to enclose port I/O operations between @p osalSysLock() and
 *          @p osalSysUnlock().
 * @note    The default implementation is non atomic and not necessarily
 *          optimal. Low level drivers may  optimize the function by using
 *          specific hardware or coding.
 * @note    The default implementation internally uses the @p palReadLatch()
 *          and @p palWritePort().
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] bit       logic value, the value must be @p PAL_LOW or
 *                      @p PAL_HIGH
 *
 * @special
 */
#if !defined(pal_lld_writepad) || defined(__DOXYGEN__)
#define palWritePad(port, pad, bit)                                         \
  palWritePort(port, (palReadLatch(port) & ~PAL_PORT_BIT(pad)) |            \
                     (((bit) & 1U) << pad))
#else
#define palWritePad(port, pad, bit) pal_lld_writepad(port, pad, bit)
#endif

/**
 * @brief   Sets a pad logic state to @p PAL_HIGH.
 * @note    The operation is not guaranteed to be atomic on all the
 *          architectures, for atomicity and/or portability reasons you may
 *          need to enclose port I/O operations between @p osalSysLock() and
 *          @p osalSysUnlock().
 * @note    The default implementation is non atomic and not necessarily
 *          optimal. Low level drivers may  optimize the function by using
 *          specific hardware or coding.
 * @note    The default implementation internally uses the @p palSetPort().
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @special
 */
#if !defined(pal_lld_setpad) || defined(__DOXYGEN__)
#define palSetPad(port, pad) palSetPort(port, PAL_PORT_BIT(pad))
#else
#define palSetPad(port, pad) pal_lld_setpad(port, pad)
#endif

/**
 * @brief   Clears a pad logic state to @p PAL_LOW.
 * @note    The operation is not guaranteed to be atomic on all the
 *          architectures, for atomicity and/or portability reasons you may
 *          need to enclose port I/O operations between @p osalSysLock() and
 *          @p osalSysUnlock().
 * @note    The default implementation is non atomic and not necessarily
 *          optimal. Low level drivers may  optimize the function by using
 *          specific hardware or coding.
 * @note    The default implementation internally uses the @p palClearPort().
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @special
 */
#if !defined(pal_lld_clearpad) || defined(__DOXYGEN__)
#define palClearPad(port, pad) palClearPort(port, PAL_PORT_BIT(pad))
#else
#define palClearPad(port, pad) pal_lld_clearpad(port, pad)
#endif

/**
 * @brief   Toggles a pad logic state.
 * @note    The operation is not guaranteed to be atomic on all the
 *          architectures, for atomicity and/or portability reasons you may
 *          need to enclose port I/O operations between @p osalSysLock() and
 *          @p osalSysUnlock().
 * @note    The default implementation is non atomic and not necessarily
 *          optimal. Low level drivers may  optimize the function by using
 *          specific hardware or coding.
 * @note    The default implementation internally uses the @p palTogglePort().
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @special
 */
#if !defined(pal_lld_togglepad) || defined(__DOXYGEN__)
#define palTogglePad(port, pad) palTogglePort(port, PAL_PORT_BIT(pad))
#else
#define palTogglePad(port, pad) pal_lld_togglepad(port, pad)
#endif

/**
 * @brief   Pad mode setup.
 * @details This function programs a pad with the specified mode.
 * @note    The default implementation not necessarily optimal. Low level
 *          drivers may  optimize the function by using specific hardware
 *          or coding.
 * @note    Programming an unknown or unsupported mode is silently ignored.
 * @note    The function can be called from any context.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      pad mode
 *
 * @special
 */
#if !defined(pal_lld_setpadmode) || defined(__DOXYGEN__)
#define palSetPadMode(port, pad, mode)                                      \
  palSetGroupMode(port, PAL_PORT_BIT(pad), 0U, mode)
#else
#define palSetPadMode(port, pad, mode) pal_lld_setpadmode(port, pad, mode)
#endif

/**
 * @brief   Reads an input line logic state.
 * @note    The function can be called from any context.
 *
 * @param[in] line      line identifier
 * @return              The logic state.
 * @retval PAL_LOW      low logic state.
 * @retval PAL_HIGH     high logic state.
 *
 * @special
 */
#if !defined(pal_lld_readline) || defined(__DOXYGEN__)
#define palReadLine(line) palReadPad(PAL_PORT(line), PAL_PAD(line))
#else
#define palReadLine(line) pal_lld_readline(line)
#endif

/**
 * @brief   Writes a logic state on an output line.
 * @note    The function can be called from any context.
 *
 * @param[in] line      line identifier
 * @param[in] bit       logic value, the value must be @p PAL_LOW or
 *                      @p PAL_HIGH
 *
 * @special
 */
#if !defined(pal_lld_writeline) || defined(__DOXYGEN__)
#define palWriteLine(line, bit) palWritePad(PAL_PORT(line), PAL_PAD(line), bit)
#else
#define palWriteLine(line, bit) pal_lld_writeline(line, bit)
#endif

/**
 * @brief   Sets a line logic state to @p PAL_HIGH.
 * @note    The function can be called from any context.
 *
 * @param[in] line      line identifier
 *
 * @special
 */
#if !defined(pal_lld_setline) || defined(__DOXYGEN__)
#define palSetLine(line) palSetPad(PAL_PORT(line), PAL_PAD(line))
#else
#define palSetLine(line) pal_lld_setline(line)
#endif

/**
 * @brief   Clears a line logic state to @p PAL_LOW.
 * @note    The function can be called from any context.
 *
 * @param[in] line      line identifier
 *
 * @special
 */
#if !defined(pal_lld_clearline) || defined(__DOXYGEN__)
#define palClearLine(line) palClearPad(PAL_PORT(line), PAL_PAD(line))
#else
#define palClearLine(line) pal_lld_clearline(line)
#endif

/**
 * @brief   Toggles a line logic state.
 * @note    The function can be called from any context.
 *
 * @param[in] line      line identifier
 *
 * @special
 */
#if !defined(pal_lld_toggleline) || defined(__DOXYGEN__)
#define palToggleLine(line) palTogglePad(PAL_PORT(line), PAL_PAD(line))
#else
#define palToggleLine(line) pal_lld_toggleline(line)
#endif

/**
 * @brief   Line mode setup.
 * @note    The function can be called from any context.
 *
 * @param[in] line      line identifier
 * @param[in] mode      pad mode
 *
 * @special
 */
#if !defined(pal_lld_setlinemode) || defined(__DOXYGEN__)
#define palSetLineMode(line, mode)                                          \
  palSetPadMode(PAL_PORT(line), PAL_PAD(line), mode)
#else
#define palSetLineMode(line, mode) pal_lld_setlinemode(line, mode)
#endif
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  ioportmask_t palReadBus(IOBus *bus);
  void palWriteBus(IOBus *bus, ioportmask_t bits);
  void palSetBusMode(IOBus *bus, iomode_t mode);
#ifdef __cplusplus
}
#endif

#endif /* _PAL_H_ */

#endif /* HAL_USE_PAL == TRUE */

/** @} */
