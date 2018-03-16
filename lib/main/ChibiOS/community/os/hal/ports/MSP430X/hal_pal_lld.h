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
 * @file    MSP430X/hal_pal_lld.h
 * @brief   MSP430X PAL subsystem low level driver header.
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

#undef PAL_MODE_INPUT_ANALOG /* configure this through the ALTERNATE macros */
#undef PAL_MODE_OUTPUT_OPENDRAIN

/**
 * @name    MSP430X-specific I/O mode flags
 * @{
 */

/**
 * @brief   Alternate mode 1
 */
#define PAL_MSP430X_ALTERNATE_1 8

/**
 * @brief   Alternate mode 2
 */
#define PAL_MSP430X_ALTERNATE_2 9

/**
 * @brief   Alternate mode 3
 */
#define PAL_MSP430X_ALTERNATE_3 10

#define ALTERNATE_HELP(n) (PAL_MSP430X_ALTERNATE_##n)
/**
 * @brief   Alternate function.
 *
 * @param[in] n   alternate function selector - 1 through 3
 */
#define PAL_MODE_ALTERNATE(n) (ALTERNATE_HELP(n))

/** @} */

/*===========================================================================*/
/* I/O Ports Types and constants.                                            */
/*===========================================================================*/

/**
 * @name    Port related definitions
 * @{
 */
/**
 * @brief   Width, in bits, of an I/O port.
 */
#define PAL_IOPORTS_WIDTH 16U

/**
 * @brief   Whole port mask.
 * @details This macro specifies all the valid bits into a port.
 */
#define PAL_WHOLE_PORT ((ioportmask_t)0xFFFFU)

/** @} */

/**
 * @name    Line handling macros
 * @{
 */
/**
 * @brief   Forms a line identifier.
 * @details A port/pad pair are encoded into an @p ioline_t type. The encoding
 *          of this type is platform-dependent.
 * @note    In this driver the pad number is encoded in the upper 4 bits of
 *          the GPIO address which are guaranteed to be zero.
 */
#define PAL_LINE(port, pad)                                                    \
  ((ioline_t)((uint16_t)(port)) | (((uint16_t)(pad)) << 12))

/**
 * @brief   Decodes a port identifier from a line identifier.
 */
#define PAL_PORT(line)                                                         \
  ((msp430x_gpio_registers_t *)(((uint16_t)(line)) & 0x0FFFU))

/**
 * @brief   Decodes a pad identifier from a line identifier.
 */
#define PAL_PAD(line) ((uint16_t)((uint16_t)(line) >> 12))

/**
 * @brief   Value identifying an invalid line.
 */
#define PAL_NOLINE 0U
/** @} */

/**
 * @brief   MSP430X register initialization
 */
typedef struct {
  /** Initial value for OUT register.*/
  uint16_t out;
  /** Initial value for DIR register.*/
  uint16_t dir;
  /** Initial value for REN register.*/
  uint16_t ren;
  /** Initial value for SEL0 register.*/
  uint16_t sel0;
  /** Initial value for SEL1 register.*/
  uint16_t sel1;
  /** Initial value for IES register.*/
  uint16_t ies;
  /** Initial value for IE register.*/
  uint16_t ie;
} msp430x_gpio_setup_t;

/**
 * @brief   MSP430X registers block
 * @note    Some ports do not support all of these fields.
 */
typedef struct {
  volatile uint16_t in;
  volatile uint16_t out;
  volatile uint16_t dir;
  volatile uint16_t _padding;
  volatile uint16_t ren;
  volatile uint16_t sel0;
  volatile uint16_t sel1;
  volatile uint16_t _padding1;
  volatile uint16_t _padding2;
  volatile uint16_t _padding3;
  volatile uint16_t _padding4;
  volatile uint16_t selc;
  volatile uint16_t ies;
  volatile uint16_t ie;
  volatile uint16_t ifg;
} msp430x_gpio_registers_t;

/**
 * @brief   MSP430X I/O ports static initializer.
 * @details An instance of this structure must be passed to @p palInit() at
 *          system startup time in order to initialized the digital I/O
 *          subsystem. This represents only the initial setup, specific pads
 *          or whole ports can be reprogrammed at later time.
 */
typedef struct {
#if defined(PA_BASE) || defined(__DOXYGEN__)
  msp430x_gpio_setup_t porta;
#endif
#if defined(PB_BASE) || defined(__DOXYGEN__)
  msp430x_gpio_setup_t portb;
#endif
#if defined(PC_BASE) || defined(__DOXYGEN__)
  msp430x_gpio_setup_t portc;
#endif
#if defined(PD_BASE) || defined(__DOXYGEN__)
  msp430x_gpio_setup_t portd;
#endif
#if defined(PE_BASE) || defined(__DOXYGEN__)
  msp430x_gpio_setup_t porte;
#endif
#if defined(PF_BASE) || defined(__DOXYGEN__)
  msp430x_gpio_setup_t portf;
#endif
  msp430x_gpio_setup_t portj;
} PALConfig;

/**
 * @brief   Digital I/O port sized unsigned type.
 */
typedef uint16_t ioportmask_t;

/**
 * @brief   Digital I/O modes.
 */
typedef uint16_t iomode_t;

/**
 * @brief   Type of an I/O line.
 */
typedef uint16_t ioline_t;

/**
 * @brief   Port Identifier.
 * @details This type can be a scalar or some kind of pointer, do not make
 *          any assumption about it, use the provided macros when populating
 *          variables of this type.
 */
typedef msp430x_gpio_registers_t * ioportid_t;

/*===========================================================================*/
/* I/O Ports Identifiers.                                                    */
/*===========================================================================*/

/**
 * @brief   GPIO port A identifier.
 */
#if defined(PA_BASE) || defined(__DOXYGEN__)
#define IOPORT1 ((volatile msp430x_gpio_registers_t *)PA_BASE)
#endif

/**
 * @brief   GPIO port B identifier.
 */
#if defined(PB_BASE) || defined(__DOXYGEN__)
#define IOPORT2 ((volatile msp430x_gpio_registers_t *)PB_BASE)
#endif

/**
 * @brief   GPIO port C identifier.
 */
#if defined(PC_BASE) || defined(__DOXYGEN__)
#define IOPORT3 ((volatile msp430x_gpio_registers_t *)PC_BASE)
#endif

/**
 * @brief   GPIO port D identifier.
 */
#if defined(PD_BASE) || defined(__DOXYGEN__)
#define IOPORT4 ((volatile msp430x_gpio_registers_t *)PD_BASE)
#endif

/**
 * @brief   GPIO port E identifier.
 */
#if defined(PE_BASE) || defined(__DOXYGEN__)
#define IOPORT5 ((volatile msp430x_gpio_registers_t *)PE_BASE)
#endif

/**
 * @brief   GPIO port F identifier.
 */
#if defined(PF_BASE) || defined(__DOXYGEN__)
#define IOPORT6   ((volatile msp430x_gpio_registers_t *)PF_BASE
#endif

/**
 * @brief   GPIO port J identifier.
 */
#define IOPORT0 ((volatile msp430x_gpio_registers_t *)PJ_BASE)

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
#define pal_lld_readport(port) ((port)->in)

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
#define pal_lld_readlatch(port) ((port)->out)

/**
 * @brief   Writes a bits mask on a I/O port.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be written on the specified port
 *
 * @notapi
 */
#define pal_lld_writeport(port, bits) ((port)->out = (bits))

/**
 * @brief   Sets a bits mask on a I/O port.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be ORed on the specified port
 *
 * @notapi
 */
#define pal_lld_setport(port, bits) ((port)->out |= (bits))

/**
 * @brief   Clears a bits mask on a I/O port.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be cleared on the specified port
 *
 * @notapi
 */
#define pal_lld_clearport(port, bits) ((port)->out &= ~(bits))

/**
 * @brief   Toggles a bits mask on a I/O port.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be XORed on the specified port
 *
 * @notapi
 */
#define pal_lld_toggleport(port, bits) ((port)->out ^= (bits))

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
#define pal_lld_setgroupmode(port, mask, offset, mode)                         \
  _pal_lld_setgroupmode(port, mask << offset, mode)

/**
 * @brief   Clears a pad logical state to @p PAL_LOW.
 * @details This function is implemented in a way which should
 *          produce a BIC instruction rather than an AND
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
#define pal_lld_clearpad(port, pad) ((port)->out &= ~(1 << pad))

#if !defined(__DOXYGEN__)
extern const PALConfig pal_default_config;
#endif

#ifdef __cplusplus
extern "C" {
#endif
void _pal_lld_init(const PALConfig * config);
void _pal_lld_setgroupmode(ioportid_t port, ioportmask_t mask, iomode_t mode);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PAL == TRUE */

#endif /* _PAL_LLD_H_ */

/** @} */
