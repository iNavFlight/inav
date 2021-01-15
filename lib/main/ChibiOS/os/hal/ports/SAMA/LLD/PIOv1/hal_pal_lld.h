/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    PIOv1/hal_pal_lld.h
 * @brief   SAMA PAL low level driver header.
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
/* Specifies palInit() without parameter, required until all platforms will
   be updated to the new style.*/
#define PAL_NEW_INIT

#undef PAL_MODE_RESET
#undef PAL_MODE_UNCONNECTED
#undef PAL_MODE_INPUT
#undef PAL_MODE_INPUT_PULLUP
#undef PAL_MODE_INPUT_PULLDOWN
#undef PAL_MODE_INPUT_ANALOG
#undef PAL_MODE_OUTPUT_PUSHPULL
#undef PAL_MODE_OUTPUT_OPENDRAIN

/**
 * @name    SAMA-specific I/O mode flags
 * @{
 */
#define PAL_SAMA_FUNC_MASK             (7U << 0U)
#define PAL_SAMA_FUNC_GPIO             (0U << 0U)
#define PAL_SAMA_FUNC_PERIPH_A         (1U << 0U)
#define PAL_SAMA_FUNC_PERIPH_B         (2U << 0U)
#define PAL_SAMA_FUNC_PERIPH_C         (3U << 0U)
#define PAL_SAMA_FUNC_PERIPH_D         (4U << 0U)
#define PAL_SAMA_FUNC_PERIPH_E         (5U << 0U)
#define PAL_SAMA_FUNC_PERIPH_F         (6U << 0U)
#define PAL_SAMA_FUNC_PERIPH_G         (7U << 0U)

#define PAL_SAMA_DIR_MASK              (1U << 8U)
#define PAL_SAMA_DIR_INPUT             (0U << 8U)
#define PAL_SAMA_DIR_OUTPUT            (1U << 8U)

#define PAL_SAMA_PUEN_MASK             (1U << 9U)
#define PAL_SAMA_PUEN_PULLUP           (1U << 9U)

#define PAL_SAMA_PDEN_MASK             (1U << 10U)
#define PAL_SAMA_PDEN_PULLDOWN         (1U << 10U)

#define PAL_SAMA_IFEN_MASK             (1U << 12U)
#define PAL_SAMA_PDEN_INPUTFILTER      (1U << 12U)

#define PAL_SAMA_IFSCEN_MASK           (1U << 13U)
#define PAL_SAMA_IFSCEN_MCK_2          (0U << 13U)
#define PAL_SAMA_IFSCEN_SLCK_2         (1U << 13U)

#define PAL_SAMA_OPD_MASK              (1U << 14U)
#define PAL_SAMA_OPD_PUSHPULL          (0U << 14U)
#define PAL_SAMA_OPD_OPENDRAIN         (1U << 14U)

#define PAL_SAMA_SCHMITT_MASK          (1U << 15U)
#define PAL_SAMA_SCHMITT               (1U << 15U)

#define PAL_SAMA_DRVSTR_MASK           (3U << 16U)
#define PAL_SAMA_DRVSTR_LO             (1U << 16U)
#define PAL_SAMA_DRVSTR_ME             (2U << 16U)
#define PAL_SAMA_DRVSTR_HI             (3U << 16U)

#define PAL_SAMA_EVTSEL_MASK           (7U << 24U)
#define PAL_SAMA_EVTSEL_FALLING        (0U << 24U)
#define PAL_SAMA_EVTSEL_RISING         (1U << 24U)
#define PAL_SAMA_EVTSEL_BOTH           (2U << 24U)
#define PAL_SAMA_EVTSEL_LOW            (3U << 24U)
#define PAL_SAMA_EVTSEL_HIGH           (4U << 24U)

#define PAL_SAMA_PCFS_MASK             (1U << 29U)
#define PAL_SAMA_ICFS_MASK             (1U << 30U)


#define PAL_SAMA_CFGR_MASK             PAL_SAMA_FUNC_MASK |                 \
                                       PAL_SAMA_DIR_MASK |                  \
                                       PAL_SAMA_PUEN_MASK |                 \
                                       PAL_SAMA_PDEN_MASK |                 \
                                       PAL_SAMA_IFEN_MASK |                 \
                                       PAL_SAMA_IFSCEN_MASK |               \
                                       PAL_SAMA_OPD_MASK |                  \
                                       PAL_SAMA_SCHMITT_MASK |              \
                                       PAL_SAMA_DRVSTR_MASK |               \
                                       PAL_SAMA_EVTSEL_MASK

#if SAMA_HAL_IS_SECURE
#define PAL_SAMA_SECURE_MASK           (1U << 31U)

#define PAL_SAMA_NON_SECURE            (0U << 31U)
#define PAL_SAMA_SECURE                (1U << 31U)
#endif /* SAMA_HAL_IS_SECURE */

/**
 * @name    Standard I/O mode flags
 * @{
 */
/**
 * @brief   Implemented as input.
 */
#define PAL_MODE_RESET                  (PAL_SAMA_DIR_INPUT |               \
                                         PAL_SAMA_SCHMITT)

/**
 * @brief   Implemented as input with pull-up.
 */
#define PAL_MODE_UNCONNECTED            (PAL_SAMA_DIR_INPUT |               \
                                         PAL_SAMA_SCHMITT |                 \
                                         PAL_SAMA_PUEN_PULLUP)

/**
 * @brief   Regular input high-Z pad.
 */
#define PAL_MODE_INPUT                  (PAL_SAMA_DIR_INPUT |               \
                                         PAL_SAMA_SCHMITT)

/**
 * @brief   Input pad with weak pull up resistor.
 */
#define PAL_MODE_INPUT_PULLUP           (PAL_SAMA_DIR_INPUT |               \
                                         PAL_SAMA_SCHMITT |                 \
                                         PAL_SAMA_PUEN_PULLUP)

/**
 * @brief   Input pad with weak pull down resistor.
 */
#define PAL_MODE_INPUT_PULLDOWN         (PAL_SAMA_DIR_INPUT |               \
                                         PAL_SAMA_SCHMITT |                 \
                                         PAL_SAMA_PDEN_PULLDOWN)

/**
 * @brief   Analog input mode.
 */
#define PAL_MODE_INPUT_ANALOG           PAL_SAMA_DIR_INPUT

/**
 * @brief   Push-pull output pad.
 */
#define PAL_MODE_OUTPUT_PUSHPULL        PAL_SAMA_DIR_OUTPUT

/**
 * @brief   Open-drain output pad.
 */
#define PAL_MODE_OUTPUT_OPENDRAIN       (PAL_SAMA_DIR_OUTPUT |              \
                                         PAL_SAMA_OPD_OPENDRAIN)

#if SAMA_HAL_IS_SECURE || defined(__DOXYGEN__)
/**
 * @brief   Secure pad.
 * @note    Available only on Secure HAL.
 */
#define PAL_MODE_SECURE                 PAL_SAMA_SECURE

/**
 * @brief   Non secure pad.
 * @note    Available only on Secure HAL.
 */
#define PAL_MODE_NON_SECURE             PAL_SAMA_NON_SECURE
#endif
/** @} */

/* Discarded definitions from the Atmel headers, the PAL driver uses its own
   definitions in order to have an unified handling for all devices.
   Unfortunately the ST headers have no uniform definitions for the same
   objects across the various sub-families.*/
#undef PIOA

/**
 * @name    PIO ports definitions
 * @{
 */
#define PIOA                            ((sama_pio_t *)PIOA_BASE)
#define PIOB                            ((sama_pio_t *)PIOB_BASE)
#define PIOC                            ((sama_pio_t *)PIOC_BASE)
#define PIOD                            ((sama_pio_t *)PIOD_BASE)
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
#define PAL_IOPORTS_WIDTH 32

/**
 * @brief   Whole port mask.
 * @details This macro specifies all the valid bits into a port.
 */
#define PAL_WHOLE_PORT ((ioportmask_t)0xFFFF)
/** @} */

/**
 * @name    Line handling macros
 * @{
 */
/**
 * @brief   Forms a line identifier.
 * @details A port/pad pair are encoded into an @p ioline_t type. The encoding
 *          of this type is platform-dependent.
 * @note    In this driver the pad number is encoded in the lower 5 bits of
 *          the PIO address which are guaranteed to be zero.
 */
#define PAL_LINE(port, pad)                                                 \
  ((ioline_t)((uint32_t)(port)) | ((uint32_t)(pad)))

/**
 * @brief   Decodes a port identifier from a line identifier.
 */
#define PAL_PORT(line)                                                      \
  ((sama_pio_t *)(((uint32_t)(line)) & 0xFFFFFFE0U))

/**
 * @brief   Decodes a pad identifier from a line identifier.
 */
#define PAL_PAD(line)                                                       \
  ((uint32_t)((uint32_t)(line) & 0x0000001FU))

/**
 * @brief   Value identifying an invalid line.
 */
#define PAL_NOLINE                      0U
/** @} */

/**
 * @brief   SAMA PIO registers block.
 */
#if SAMA_HAL_IS_SECURE
typedef struct {
  volatile uint32_t MSKR;
  volatile uint32_t CFGR;
  volatile uint32_t PDSR;
  volatile uint32_t LOCKSR;
  volatile uint32_t SODR;
  volatile uint32_t CODR;
  volatile uint32_t ODSR;
  volatile uint32_t Reserved1;
  volatile uint32_t IER; 
  volatile uint32_t IDR;
  volatile uint32_t IMR;
  volatile uint32_t ISR;
  volatile uint32_t SIONR;
  volatile uint32_t SIOSR;
  volatile uint32_t IOSSR;
  volatile uint32_t IOFR;
} sama_pio_t;
#else
typedef struct {
  volatile uint32_t MSKR;
  volatile uint32_t CFGR;
  volatile uint32_t PDSR;
  volatile uint32_t LOCKSR;
  volatile uint32_t SODR;
  volatile uint32_t CODR;
  volatile uint32_t ODSR;
  volatile uint32_t Reserved1;
  volatile uint32_t IER; 
  volatile uint32_t IDR;
  volatile uint32_t IMR;
  volatile uint32_t ISR;
  volatile uint32_t Reserved2[3];
  volatile uint32_t IOFR;
} sama_pio_t;  
#endif

/**
 * @brief   SAMA PIO static initializer.
 * @details It is a dummy.
 */
typedef uint32_t PALConfig;

/**
 * @brief   Type of digital I/O port sized unsigned integer.
 */
typedef uint32_t ioportmask_t;

/**
 * @brief   Type of digital I/O modes.
 */
typedef uint32_t iomode_t;

/**
 * @brief   Type of an I/O line.
 */
typedef uint32_t ioline_t;

/**
 * @brief   Type of an event mode.
 */
typedef uint32_t ioeventmode_t;

/**
 * @brief   Type of a port Identifier.
 * @details This type can be a scalar or some kind of pointer, do not make
 *          any assumption about it, use the provided macros when populating
 *          variables of this type.
 */
typedef sama_pio_t * ioportid_t;

/**
 * @brief   Type of an pad identifier.
 */
typedef uint32_t iopadid_t;

/*===========================================================================*/
/* I/O Ports Identifiers.                                                    */
/* The low level driver wraps the definitions already present in the SAMA   */
/* firmware library.                                                         */
/*===========================================================================*/

/**
 * @brief   PIO port A identifier.
 */
#if SAMA_HAS_PIOA || defined(__DOXYGEN__)
#define IOPORT1         PIOA
#endif

/**
 * @brief   PIO port B identifier.
 */
#if SAMA_HAS_PIOB || defined(__DOXYGEN__)
#define IOPORT2         PIOB
#endif

/**
 * @brief   PIO port C identifier.
 */
#if SAMA_HAS_PIOC || defined(__DOXYGEN__)
#define IOPORT3         PIOC
#endif

/**
 * @brief   PIO port D identifier.
 */
#if SAMA_HAS_PIOD || defined(__DOXYGEN__)
#define IOPORT4         PIOD
#endif

/*===========================================================================*/
/* Implementation, some of the following macros could be implemented as      */
/* functions, if so please put them in pal_lld.c.                            */
/*===========================================================================*/
/**
 * @brief   GPIO ports subsystem initialization.
 *
 * @notapi
 */
#define pal_lld_init() _pal_lld_init()

/**
 * @brief   Reads an I/O port.
 * @details This function is implemented by reading the PIO PDSR register, the
 *          implementation has no side effects.
 * @note    This function is not meant to be invoked directly by the application
 *          code.
 *
 * @param[in] port      port identifier
 * @return              The port bits.
 *
 * @notapi
 */
#define pal_lld_readport(port) ((port)->PDSR)

/**
 * @brief   Reads the output latch.
 * @details This function is implemented by reading the PIO ODSR register, the
 *          implementation has no side effects.
 * @note    This function is not meant to be invoked directly by the application
 *          code.
 *
 * @param[in] port      port identifier
 * @return              The latched logical states.
 *
 * @notapi
 */
#define pal_lld_readlatch(port) ((port)->ODSR)

/**
 * @brief   Writes on a I/O port.
 * @details This function is implemented by writing the PIO ODR register, the
 *          implementation has no side effects.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be written on the specified port
 *
 * @notapi
 */
#define pal_lld_writeport(port, bits)                                       \
  do {                                                                      \
    (port)->MSKR = 0xFFFFFFFFU;                                             \
    (port)->ODSR = (bits);                                                   \
  } while (false) 

/**
 * @brief   Sets a bits mask on a I/O port.
 * @details This function is implemented by writing the PIO BSRR register, the
 *          implementation has no side effects.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be ORed on the specified port
 *
 * @notapi
 */
#define pal_lld_setport(port, bits) ((port)->SODR = (bits))

/**
 * @brief   Clears a bits mask on a I/O port.
 * @details This function is implemented by writing the PIO BSRR register, the
 *          implementation has no side effects.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be cleared on the specified port
 *
 * @notapi
 */
#define pal_lld_clearport(port, bits) ((port)->CODR = (bits))

/**
 * @brief   Writes a group of bits.
 * @details This function is implemented by writing the PIO BSRR register, the
 *          implementation has no side effects.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask
 * @param[in] offset    the group bit offset within the port
 * @param[in] bits      bits to be written. Values exceeding the group
 *                      width are masked.
 *
 * @notapi
 */
#define pal_lld_writegroup(port, mask, offset, bits)                        \
  do {                                                                      \
    (port)->MSKR = ((mask) << (offset));                                    \
    (port)->ODSR = (((bits) & (mask)) << (offset));                         \
  } while (false)

/**
 * @brief   Pads group mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
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
 * @brief   Writes a logical state on an output pad.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] bit       logical value, the value must be @p PAL_LOW or
 *                      @p PAL_HIGH
 *
 * @notapi
 */
#define pal_lld_writepad(port, pad, bit) pal_lld_writegroup(port, 1, pad, bit)

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
 * @brief   Returns a PAL event structure associated to a line.
 *
 * @param[in] line      line identifier
 *
 * @notapi
 */
#define pal_lld_get_line_event(line)                                        \
  &_pal_events[PAL_PAD(line)]

#if !defined(__DOXYGEN__)
extern palevent_t _pal_events[4 * 32];
#endif

#ifdef __cplusplus
extern "C" {
#endif
   void _pal_lld_init(void);
   void _pal_lld_setgroupmode(ioportid_t port,
                              ioportmask_t mask,
                              iomode_t mode);
   void _pal_lld_enablepadevent(ioportid_t port,
                                iopadid_t pad,
                                ioeventmode_t mode);
   void _pal_lld_disablepadevent(ioportid_t port, iopadid_t pad);
   palevent_t* pal_lld_get_pad_event(ioportid_t port, iopadid_t pad);
   /* LLD only functions */
#if SAMA_HAL_IS_SECURE
   void pal_lld_cfg_debouncing_time(uint32_t db_time);
#endif
   uint32_t pal_lld_read_status(ioportid_t port);
   uint32_t pal_lld_read_int_mask(ioportid_t port);
   uint32_t pal_lld_read_cfgr(ioportid_t port);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PAL */

#endif /* HAL_PAL_LLD_H */

/** @} */
