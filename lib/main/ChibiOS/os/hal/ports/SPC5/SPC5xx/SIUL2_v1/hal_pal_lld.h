/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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
 * @file    SPC5xx/SIUL2_v1/hal_pal_lld.h
 * @brief   SPC5xx SIUL2 low level driver header.
 *
 * @addtogroup PAL
 * @{
 */

#ifndef HAL_PAL_LLD_H
#define HAL_PAL_LLD_H

#if HAL_USE_PAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Unsupported modes and specific modes                                      */
/*===========================================================================*/

#undef PAL_MODE_RESET
#undef PAL_MODE_UNCONNECTED
#undef PAL_MODE_INPUT
#undef PAL_MODE_INPUT_PULLUP
#undef PAL_MODE_INPUT_PULLDOWN
#undef PAL_MODE_INPUT_ANALOG
#undef PAL_MODE_OUTPUT_PUSHPULL
#undef PAL_MODE_OUTPUT_OPENDRAIN

/**
 * @name    SIUL2-specific PAL modes
 * @{
 */
#define PAL_SPC5_OERC_MASK          (3U << 28)
#define PAL_SPC5_OERC(n)            ((n) << 28)
#define PAL_SPC5_OERC_WEAK          PAL_SPC5_OERC(0)
#define PAL_SPC5_OERC_MEDIUM        PAL_SPC5_OERC(1)
#define PAL_SPC5_OERC_STRONG        PAL_SPC5_OERC(2)
#define PAL_SPC5_OERC_VERY_STRONG   PAL_SPC5_OERC(3)
#if !defined(_SPC570Sxx_)
#define PAL_SPC5_ODC_MASK           (7U << 24)
#else
#define PAL_SPC5_ODC_MASK           (3U << 24)
#endif
#define PAL_SPC5_ODC(n)             ((n) << 24)
#define PAL_SPC5_ODC_DISABLED       PAL_SPC5_ODC(0)
#define PAL_SPC5_ODC_OPEN_DRAIN     PAL_SPC5_ODC(1)
#define PAL_SPC5_ODC_PUSH_PULL      PAL_SPC5_ODC(2)
#define PAL_SPC5_ODC_OPEN_SOURCE    PAL_SPC5_ODC(3)
#if !defined(_SPC570Sxx_)
#define PAL_SPC5_ODC_MSC_LVDS       PAL_SPC5_ODC(4)
#define PAL_SPC5_ODC_LFAST_LVDS     PAL_SPC5_ODC(5)
#endif
#define PAL_SPC5_SMC                (1U << 23)
#define PAL_SPC5_APC                (1U << 22)
#define PAL_SPC5_ILS_MASK           (3U << 20)
#define PAL_SPC5_ILS(n)             ((n) << 20)
#define PAL_SPC5_ILS_TTL            PAL_SPC5_ILS(0)
#define PAL_SPC5_ILS_LVDS           PAL_SPC5_ILS(1)
#define PAL_SPC5_ILS_CMOS           PAL_SPC5_ILS(2)
#define PAL_SPC5_IBE                (1U << 19)
#define PAL_SPC5_HYS                (1U << 18)
#define PAL_SPC5_WPDE               (1U << 17)
#define PAL_SPC5_WPUE               (1U << 16)
#define PAL_SPC5_INV                (1U << 15)
#define PAL_SPC5_SSS_MASK           (255U << 0)
#define PAL_SPC5_SSS(n)             ((n) << 0)
/** @} */

/**
 * @name    Pads mode constants
 * @{
 */
/**
 * @brief   After reset state.
 */
#define PAL_MODE_RESET                  (PAL_SPC5_SMC | PAL_SPC5_IBE |      \
                                         PAL_SPC5_WPUE)

/**
 * @brief   Safe state for <b>unconnected</b> pads.
 */
#define PAL_MODE_UNCONNECTED            (PAL_SPC5_SMC | PAL_SPC5_IBE |      \
                                         PAL_SPC5_WPUE)

/**
 * @brief   Regular input high-Z pad.
 */
#define PAL_MODE_INPUT                  (PAL_SPC5_SMC | PAL_SPC5_IBE)

/**
 * @brief   Input pad with weak pull up resistor.
 */
#define PAL_MODE_INPUT_PULLUP           (PAL_SPC5_SMC | PAL_SPC5_IBE |      \
                                         PAL_SPC5_WPUE)

/**
 * @brief   Input pad with weak pull down resistor.
 */
#define PAL_MODE_INPUT_PULLDOWN         (PAL_SPC5_SMC | PAL_SPC5_IBE |      \
                                         PAL_SPC5_WPDE)

/**
 * @brief   Analog input mode.
 */
#define PAL_MODE_INPUT_ANALOG           (PAL_SPC5_SMC | PAL_SPC5_APC)

/**
 * @brief   Push-pull output pad.
 */
#define PAL_MODE_OUTPUT_PUSHPULL        (PAL_SPC5_SMC |                     \
                                         PAL_SPC5_ODC_PUSH_PULL |           \
                                         PAL_SPC5_IBE)

/**
 * @brief   Open-drain output pad.
 */
#define PAL_MODE_OUTPUT_OPENDRAIN       (PAL_SPC5_SMC |                     \
                                         PAL_SPC5_ODC_OPEN_DRAIN |          \
                                         PAL_SPC5_IBE)
/**
 * @brief   Alternate "n" output pad.
 * @note    Both the IBE and ODC bits are specified in this mask.
 */
#define PAL_MODE_OUTPUT_ALTERNATE(n)    (PAL_SPC5_SMC |                     \
                                         PAL_SPC5_ODC_PUSH_PULL |           \
                                         PAL_SPC5_IBE |                     \
                                         PAL_SPC5_SSS(n))
/** @} */

/*===========================================================================*/
/* I/O Ports Types and constants.                                            */
/*===========================================================================*/

/**
 * @brief   Width, in bits, of an I/O port.
 */
#define PAL_IOPORTS_WIDTH 16

/**
 * @brief   Whole port mask.
 * @brief   This macro specifies all the valid bits into a port.
 */
#define PAL_WHOLE_PORT ((ioportmask_t)0xFFFF)

/**
 * @brief   Digital I/O port sized unsigned type.
 */
typedef uint16_t ioportmask_t;

/**
 * @brief   Digital I/O modes.
 */
typedef uint32_t iomode_t;

/**
 * @brief   Port Identifier.
 * @details This type can be a scalar or some kind of pointer, do not make
 *          any assumption about it, use the provided macros when populating
 *          variables of this type.
 */
typedef uint32_t ioportid_t;

/**
 * @brief   SIUL2 MSCR_IO register initializer type.
 */
typedef struct {
  int16_t                   mscr_index;
  uint8_t                   gpdo_value;
  iomode_t                  mscr_value;
} spc_mscr_io_init_t;

/**
 * @brief   SIUL2 MSCR_MUX register initializer type.
 */
typedef struct {
  int16_t                   mscr_index;
  uint16_t                  mscr_value;
} spc_mscr_mux_init_t;

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
  const spc_mscr_io_init_t  *mscr_io;
  const spc_mscr_mux_init_t *mscr_mux;
} PALConfig;

/*===========================================================================*/
/* I/O Ports Identifiers.                                                    */
/*===========================================================================*/

/**
 * @brief   I/O port A identifier.
 */
#define PORT_A          0

/**
 * @brief   I/O port B identifier.
 */
#define PORT_B          1

/**
 * @brief   I/O port C identifier.
 */
#define PORT_C          2

/**
 * @brief   I/O port D identifier.
 */
#define PORT_D          3

/**
 * @brief   I/O port E identifier.
 */
#define PORT_E          4

/**
 * @brief   I/O port F identifier.
 */
#define PORT_F          5

/**
 * @brief   I/O port G identifier.
 */
#define PORT_G          6

/**
 * @brief   I/O port H identifier.
 */
#define PORT_H          7

/**
 * @brief   I/O port I identifier.
 */
#define PORT_I          8

/**
 * @brief   I/O port J identifier.
 */
#define PORT_J          9

/**
 * @brief   I/O port K identifier.
 */
#define PORT_K          10

/**
 * @brief   I/O port L identifier.
 */
#define PORT_L          11

/**
 * @brief   I/O port M identifier.
 */
#define PORT_M          12

/**
 * @brief   I/O port N identifier.
 */
#define PORT_N          13

/**
 * @brief   I/O port O identifier.
 */
#define PORT_O          14

/**
 * @brief   I/O port P identifier.
 */
#define PORT_P          15

/*===========================================================================*/
/* Implementation, some of the following macros could be implemented as      */
/* functions, if so please put them in pal_lld.c.                            */
/*===========================================================================*/

/**
 * @brief   Port bit helper macro.
 * @note    Overrides the one in @p pal.h.
 *
 * @param[in] n         bit position within the port
 *
 * @return              The bit mask.
 */
#define PAL_PORT_BIT(n) ((ioportmask_t)(0x8000U >> (n)))

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
#define pal_lld_readport(port)                                              \
  (SIUL2.PGPDI[port].R)

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
#define pal_lld_readlatch(port)                                             \
  (SIUL2.PGPDO[port].R)

/**
 * @brief   Writes a bits mask on a I/O port.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be written on the specified port
 *
 * @notapi
 */
#define pal_lld_writeport(port, bits)                                       \
  ((SIUL2.PGPDO)[port].R = (bits))

/**
 * @brief   Reads a group of bits.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask
 * @param[in] offset    group bit offset within the port
 * @return              The group logical states.
 *
 * @notapi
 */
#define pal_lld_readgroup(port, mask, offset)                               \
  _pal_lld_readgroup(port, mask, offset)

/**
 * @brief   Writes a group of bits.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask
 * @param[in] offset    group bit offset within the port
 * @param[in] bits      bits to be written. Values exceeding the group width
 *                      are masked.
 *
 * @notapi
 */
#define pal_lld_writegroup(port, mask, offset, bits)                        \
  _pal_lld_writegroup(port, mask, offset, bits)

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
  (SIUL2.GPDI[((port) * 16) + (pad)].R)

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
  (SIUL2.GPDO[((port) * 16) + (pad)].R = (bit))

/**
 * @brief   Sets a pad logical state to @p PAL_HIGH.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
#define pal_lld_setpad(port, pad)                                           \
  (SIUL2.GPDO[((port) * 16) + (pad)].R = 1)

/**
 * @brief   Clears a pad logical state to @p PAL_LOW.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
#define pal_lld_clearpad(port, pad)                                         \
  (SIUL2.GPDO[((port) * 16) + (pad)].R = 0)

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
  (SIUL2.GPDO[((port) * 16) + (pad)].R = ~SIUL2.GPDO[((port) * 16) + (pad)].R)

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
#define pal_lld_setpadmode(port, pad, mode)

extern const PALConfig pal_default_config;

#ifdef __cplusplus
extern "C" {
#endif
  void _pal_lld_init(const PALConfig *config);
  ioportmask_t _pal_lld_readgroup(ioportid_t port,
                                  ioportmask_t mask,
                                  uint_fast8_t offset);
  void _pal_lld_writegroup(ioportid_t port,
                           ioportmask_t mask,
                           uint_fast8_t offset,
                           ioportmask_t bits);
  void _pal_lld_setgroupmode(ioportid_t port,
                             ioportmask_t mask,
                             iomode_t mode);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PAL */

#endif /* HAL_PAL_LLD_H */

/** @} */
