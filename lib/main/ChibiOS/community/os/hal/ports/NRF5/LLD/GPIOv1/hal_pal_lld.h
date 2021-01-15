/*
    Copyright (C) 2018 Konstantin Oblaukhov
    Copyright (C) 2015 Fabio Utzig

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
 * @file    GPIOv1/hal_pal_lld.h
 * @brief   NRF5 PAL subsystem low level driver header.
 *
 * @addtogroup PAL
 * @{
 */

#ifndef HAL_PAL_LLD_H
#define HAL_PAL_LLD_H

#if (HAL_USE_PAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Unsupported modes and specific modes                                      */
/*===========================================================================*/

/*===========================================================================*/
/* I/O Ports Types and constants.                                            */
/*===========================================================================*/

#define TOTAL_GPIO_PADS 32

/**
 * @name    Port related definitions
 * @{
 */
/**
 * @brief   Width, in bits, of an I/O port.
 */
#define PAL_IOPORTS_WIDTH 32U

/**
 * @brief   Whole port mask.
 * @brief   This macro specifies all the valid bits into a port.
 */
#define PAL_WHOLE_PORT ((ioportmask_t)0xFFFFFFFFU)
/** @} */

/**
 * @name    Line handling macros
 * @{
 */
/**
 * @brief   Forms a line identifier.
 * @details A port/pad pair are encoded into an @p ioline_t type. The encoding
 *          of this type is platform-dependent.
 */
#define PAL_LINE(port, pad)                                                 \
  ((ioline_t)((uint32_t)(pad)))

/**
 * @brief   Decodes a port identifier from a line identifier.
 */
#define PAL_PORT(line)                                                      \
  ((ioportid_t)(IOPORT1))

/**
 * @brief   Decodes a pad identifier from a line identifier.
 */
#define PAL_PAD(line)                                                       \
  ((uint32_t)(line))

/**
 * @brief   Value identifying an invalid line.
 */
#define PAL_NOLINE              ((ioline_t)-1)
/** @} */

/**
 * @brief   Generic I/O ports static initializer.
 * @details An instance of this structure must be passed to @p palInit() at
 *          system startup time in order to initialized the digital I/O
 *          subsystem. This represents only the initial setup, specific pads
 *          or whole ports can be reprogrammed at later time.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  uint32_t pads[TOTAL_GPIO_PADS];
} PALConfig;

/**
 * @brief   Digital I/O port sized unsigned type.
 */
typedef uint32_t ioportmask_t;

/**
 * @brief   Digital I/O modes.
 */
typedef uint8_t iomode_t;

/**
 * @brief   Type of an I/O line.
 */
typedef uint32_t ioline_t;

/**
 * @brief   Type of an event mode.
 */
typedef uint32_t ioeventmode_t;

/**
 * @brief   Port Identifier.
 * @details This type can be a scalar or some kind of pointer, do not make
 *          any assumption about it, use the provided macros when populating
 *          variables of this type.
 */
typedef NRF_GPIO_Type *ioportid_t;

/**
 * @brief   Type of an pad identifier.
 */
typedef uint32_t iopadid_t;

/*===========================================================================*/
/* I/O Ports Identifiers.                                                    */
/*===========================================================================*/

/**
 * @brief   First I/O port identifier.
 * @details Low level drivers can define multiple ports, it is suggested to
 *          use this naming convention.
 */
#if   NRF_SERIES == 51
#define IOPORT1         NRF_GPIO
#else
#define IOPORT1         NRF_P0
#endif

/**
 * @brief   Number of PAL events.
 * @details Maximum number of GPIO events supported by GPIOTE peripheral
 */
#if NRF_SERIES == 51
#define NRF5_GPIOTE_NUM_CHANNELS (4)
#else
#define NRF5_GPIOTE_NUM_CHANNELS (8)
#endif

/**
 * @brief   Pad to event number
 * @details Converts pad to GPIOTE peripheral pad event number
 */
#if !defined(NRF5_PAL_PAD_TO_EVENT) || defined(__DOXYGEN__)
#define NRF5_PAL_PAD_TO_EVENT(pad) ((pad) % NRF5_GPIOTE_NUM_CHANNELS)
#endif

/*===========================================================================*/
/* Implementation, some of the following macros could be implemented as      */
/* functions, if so please put them in pal_lld.c.                            */
/*===========================================================================*/

/**
 * @brief   Low level PAL subsystem initialization.
 *
 * @param[in] config    architecture-dependent ports configuration
 *
 * @notapi
 */
#define pal_lld_init(config) _pal_lld_init(config)

/**
 * @brief   Reads the physical I/O port states.
 *
 * @param[in] port      port identifier
 * @return              The port bits.
 *
 * @notapi
 */
#define pal_lld_readport(port) (IOPORT1->IN)

/**
 * @brief   Reads the output latch.
 * @details The purpose of this function is to read back the latched output
 *          value.
 *
 * @param[in] port      port identifier
 * @return              The latched logical states.
 *
 * @notapi
 */
#define pal_lld_readlatch(port) (IOPORT1->OUT)

/**
 * @brief   Writes a bits mask on a I/O port.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be written on the specified port
 *
 * @notapi
 */
#define pal_lld_writeport(port, bits) (IOPORT1->OUT = (bits))

/**
 * @brief   Sets a bits mask on a I/O port.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be ORed on the specified port
 *
 * @notapi
 */
#define pal_lld_setport(port, bits) (IOPORT1->OUTSET = (bits))


/**
 * @brief   Clears a bits mask on a I/O port.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be cleared on the specified port
 *
 * @notapi
 */
#define pal_lld_clearport(port, bits) (IOPORT1->OUTCLR = (bits))

/**
 * @brief   Pads group mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 * @note    Programming an unknown or unsupported mode is silently ignored.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask
 * @param[in] offset    group bit offset within the port
 * @param[in] mode      group mode
 *
 * @notapi
 */
#define pal_lld_setgroupmode(port, mask, offset, mode)                      \
  _pal_lld_setgroupmode(port, mask << offset, mode)

/**
 * @brief   Reads a logical state from an I/O pad.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @return              The logical state.
 * @retval PAL_LOW      low logical state.
 * @retval PAL_HIGH     high logical state.
 *
 * @notapi
 */
#define pal_lld_readpad(port, pad)                                          \
  ((IOPORT1->IN & ((uint32_t) 1 << pad)) ? PAL_HIGH : PAL_LOW)

/**
 * @brief   Writes a logical state on an output pad.
 * @note    This function is not meant to be invoked directly by the
 *          application  code.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] bit       logical value, the value must be @p PAL_LOW or
 *                      @p PAL_HIGH
 *
 * @notapi
 */
#define pal_lld_writepad(port, pad, bit)                                    \
  do {                                                                      \
    (void)port;                                                             \
    if (bit == PAL_HIGH)                                                    \
      IOPORT1->OUTSET = ((uint32_t) 1 << pad);                              \
    else                                                                    \
      IOPORT1->OUTCLR = ((uint32_t) 1 << pad);                              \
  } while (false)

/**
 * @brief   Sets a pad logical state to @p PAL_HIGH.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
#define pal_lld_setpad(port, pad) (IOPORT1->OUTSET = (uint32_t) 1 << (pad))

/**
 * @brief   Clears a pad logical state to @p PAL_LOW.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
#define pal_lld_clearpad(port, pad) (IOPORT1->OUTCLR = (uint32_t) 1 << (pad))

/**
 * @brief   Toggles a pad logical state.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
#define pal_lld_togglepad(port, pad)                                        \
  do {                                                                      \
    uint8_t bit = (IOPORT1->OUT >> (pad)) & 1;                              \
    if (bit)                                                                \
      IOPORT1->OUTCLR = 1 << (pad);                                         \
    else                                                                    \
      IOPORT1->OUTSET = 1 << (pad);                                         \
  } while (0)

/**
 * @brief   Pad mode setup.
 * @details This function programs a pad with the specified mode.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 * @note    Programming an unknown or unsupported mode is silently ignored.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      pad mode
 *
 * @notapi
 */
#define pal_lld_setpadmode(port, pad, mode) _pal_lld_setpadmode(port, pad, mode)

/**
 * @brief   Pad event enable.
 * @note    Programming an unknown or unsupported mode is silently ignored.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      pad event mode
 *
 * @notapi
 */
#define pal_lld_enablepadevent(port, pad, mode)                             \
  _pal_lld_enablepadevent(port, pad, mode)

/**
 * @brief   Pad event disable.
 * @details This function disables previously programmed event callbacks.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
#define pal_lld_disablepadevent(port, pad)                                  \
  _pal_lld_disablepadevent(port, pad)

/**
 * @brief   Returns a PAL event structure associated to a pad.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
#define pal_lld_get_pad_event(port, pad)                                    \
  &_pal_events[NRF5_PAL_PAD_TO_EVENT(pad)]; (void)(port)

/**
 * @brief   Returns a PAL event structure associated to a line.
 *
 * @param[in] line      line identifier
 *
 * @notapi
 */
#define pal_lld_get_line_event(line)                                        \
  &_pal_events[NRF5_PAL_PAD_TO_EVENT(PAL_PAD(line))]

#if !defined(__DOXYGEN__)
extern const PALConfig pal_default_config;
extern palevent_t _pal_events[NRF5_GPIOTE_NUM_CHANNELS];
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void _pal_lld_init(const PALConfig *config);
  void _pal_lld_setgroupmode(ioportid_t port,
                             ioportmask_t mask,
                             iomode_t mode);
  void _pal_lld_setpadmode(ioportid_t port,
                           uint8_t pad,
                           iomode_t mode);
#if PAL_USE_CALLBACKS || PAL_USE_WAIT
  void _pal_lld_enablepadevent(ioportid_t port,
                               iopadid_t pad,
                               ioeventmode_t mode);
  void _pal_lld_disablepadevent(ioportid_t port, iopadid_t pad);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PAL == TRUE */

#endif /* HAL_PAL_LLD_H */

/** @} */
