/*
    Copyright (C) 2014..2016 Marco Veeneman

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
 * @file    TIVA/LLD/pal_lld.h
 * @brief   TM4C123x/TM4C129x PAL subsystem low level driver header.
 *
 * @addtogroup PAL
 * @{
 */

#ifndef HAL_PAL_LLD_H
#define HAL_PAL_LLD_H

#if HAL_USE_PAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
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
 * @name    TIVA-specific I/O mode flags
 * @{
 */
#define PAL_TIVA_DIR_MASK           (1 << 0)
#define PAL_TIVA_DIR_INPUT          (0 << 0)
#define PAL_TIVA_DIR_OUTPUT         (1 << 0)

#define PAL_TIVA_AFSEL_MASK         (1 << 1)
#define PAL_TIVA_AFSEL_GPIO         (0 << 1)
#define PAL_TIVA_AFSEL_ALTERNATE    (1 << 1)

#define PAL_TIVA_DR2R_MASK          (1 << 2)
#define PAL_TIVA_DR2R_DISABLE       (0 << 2)
#define PAL_TIVA_DR2R_ENABLE        (1 << 2)

#define PAL_TIVA_DR4R_MASK          (1 << 3)
#define PAL_TIVA_DR4R_DISABLE       (0 << 3)
#define PAL_TIVA_DR4R_ENABLE        (1 << 3)

#define PAL_TIVA_DR8R_MASK          (1 << 4)
#define PAL_TIVA_DR8R_DISABLE       (0 << 4)
#define PAL_TIVA_DR8R_ENABLE        (1 << 4)

#define PAL_TIVA_ODR_MASK           (1 << 5)
#define PAL_TIVA_ODR_PUSHPULL       (0 << 5)
#define PAL_TIVA_ODR_OPENDRAIN      (1 << 5)

#define PAL_TIVA_PUR_MASK           (1 << 6)
#define PAL_TIVA_PUR_DISABLE        (0 << 6)
#define PAL_TIVA_PUR_ENABLE         (1 << 6)

#define PAL_TIVA_PDR_MASK           (1 << 7)
#define PAL_TIVA_PDR_DISABLE        (0 << 7)
#define PAL_TIVA_PDR_ENABLE         (1 << 7)

#define PAL_TIVA_SLR_MASK           (1 << 8)
#define PAL_TIVA_SLR_DISABLE        (0 << 8)
#define PAL_TIVA_SLR_ENABLE         (1 << 8)

#define PAL_TIVA_DEN_MASK           (1 << 9)
#define PAL_TIVA_DEN_DISABLE        (0 << 9)
#define PAL_TIVA_DEN_ENABLE         (1 << 9)

#define PAL_TIVA_AMSEL_MASK         (1 << 10)
#define PAL_TIVA_AMSEL_DISABLE      (0 << 10)
#define PAL_TIVA_AMSEL_ENABLE       (1 << 10)

#define PAL_TIVA_PCTL_MASK          (7 << 11)
#define PAL_TIVA_PCTL(n)            ((n) << 11)

/**
 * @brief   Alternate function.
 *
 * @param[in] n         alternate function selector
 */
#define PAL_MODE_ALTERNATE(n)       (PAL_TIVA_AFSEL_ALTERNATE |               \
                                     PAL_TIVA_PCTL(n))
/**
 * @}
 */

/**
 * @name    Standard I/O mode flags
 * @{
 */
/**
 * @brief   This mode is implemented as input.
 */
#define PAL_MODE_RESET                  PAL_MODE_INPUT

/**
 * @brief   This mode is implemented as input with pull-up.
 */
#define PAL_MODE_UNCONNECTED            PAL_MODE_INPUT_PULLUP

/**
 * @brief   Regular input high-Z pad.
 */
#define PAL_MODE_INPUT                  (PAL_TIVA_DEN_ENABLE |                \
                                         PAL_TIVA_DIR_INPUT)

/**
 * @brief   Input pad with weak pull up resistor.
 */
#define PAL_MODE_INPUT_PULLUP           (PAL_TIVA_DIR_INPUT |                 \
                                         PAL_TIVA_PUR_ENABLE |                \
                                         PAL_TIVA_DEN_ENABLE)

/**
 * @brief   Input pad with weak pull down resistor.
 */
#define PAL_MODE_INPUT_PULLDOWN         (PAL_TIVA_DIR_INPUT |                 \
                                         PAL_TIVA_PDR_ENABLE |                \
                                         PAL_TIVA_DEN_ENABLE)

/**
 * @brief   Analog input mode.
 */
#define PAL_MODE_INPUT_ANALOG           (PAL_TIVA_DEN_DISABLE |               \
                                         PAL_TIVA_AMSEL_ENABLE)

/**
 * @brief   Push-pull output pad.
 */
#define PAL_MODE_OUTPUT_PUSHPULL        (PAL_TIVA_DIR_OUTPUT |                \
                                         PAL_TIVA_DR2R_ENABLE |               \
                                         PAL_TIVA_ODR_PUSHPULL |              \
                                         PAL_TIVA_DEN_ENABLE)

/**
 * @brief   Open-drain output pad.
 */
#define PAL_MODE_OUTPUT_OPENDRAIN       (PAL_TIVA_DIR_OUTPUT |                \
                                         PAL_TIVA_DR2R_ENABLE |               \
                                         PAL_TIVA_ODR_OPENDRAIN |             \
                                         PAL_TIVA_DEN_ENABLE)
/**
 * @}
 */

/** @brief   GPIOA port identifier.*/
#define IOPORT1         GPIOA

/** @brief   GPIOB port identifier.*/
#define IOPORT2         GPIOB

/** @brief   GPIOC port identifier.*/
#define IOPORT3         GPIOC

/** @brief   GPIOD port identifier.*/
#define IOPORT4         GPIOD

/** @brief   GPIOE port identifier.*/
#define IOPORT5         GPIOE

/** @brief   GPIOF port identifier.*/
#define IOPORT6         GPIOF

#if TIVA_HAS_GPIOG || defined(__DOXYGEN__)
/** @brief Port G setup data.*/
#define IOPORT7         GPIOG
#endif /* TIVA_HAS_GPIOG.*/

#if TIVA_HAS_GPIOH || defined(__DOXYGEN__)
/** @brief Port H setup data.*/
#define IOPORT8         GPIOH
#endif /* TIVA_HAS_GPIOH.*/

#if TIVA_HAS_GPIOJ || defined(__DOXYGEN__)
/** @brief Port J setup data.*/
#define IOPORT9         GPIOJ
#endif /* TIVA_HAS_GPIOJ.*/

#if TIVA_HAS_GPIOK || defined(__DOXYGEN__)
/** @brief Port K setup data.*/
#define IOPORT10        GPIOK
#endif /* TIVA_HAS_GPIOK.*/

#if TIVA_HAS_GPIOL || defined(__DOXYGEN__)
/** @brief Port L setup data.*/
#define IOPORT11        GPIOL
#endif /* TIVA_HAS_GPIOL.*/

#if TIVA_HAS_GPIOM || defined(__DOXYGEN__)
/** @brief Port M setup data.*/
#define IOPORT12        GPIOM
#endif /* TIVA_HAS_GPIOM.*/

#if TIVA_HAS_GPION || defined(__DOXYGEN__)
/** @brief Port N setup data.*/
#define IOPORT13        GPION
#endif /* TIVA_HAS_GPION.*/

#if TIVA_HAS_GPIOP || defined(__DOXYGEN__)
/** @brief Port P setup data.*/
#define IOPORT14        GPIOP
#endif /* TIVA_HAS_GPIOP.*/

#if TIVA_HAS_GPIOQ || defined(__DOXYGEN__)
/** @brief Port Q setup data.*/
#define IOPORT15        GPIOQ
#endif /* TIVA_HAS_GPIOQ.*/

#if TIVA_HAS_GPIOR || defined(__DOXYGEN__)
/** @brief Port R setup data.*/
#define IOPORT16        GPIOR
#endif /* TIVA_HAS_GPIOR.*/

#if TIVA_HAS_GPIOS || defined(__DOXYGEN__)
/** @brief Port S setup data.*/
#define IOPORT17        GPIOS
#endif /* TIVA_HAS_GPIOS.*/

#if TIVA_HAS_GPIOT || defined(__DOXYGEN__)
/** @brief Port T setup data.*/
#define IOPORT18        GPIOT
#endif /* TIVA_HAS_GPIOT.*/

/**
 * @brief   Width, in bits, of an I/O port.
 */
#define PAL_IOPORTS_WIDTH 8

/**
 * @brief   Whole port mask.
 * @brief   This macro specifies all the valid bits into a port.
 */
#define PAL_WHOLE_PORT ((ioportmask_t)0xFF)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

#if defined(TM4C123x)

/**
 * @brief   GPIOA AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOA. When set
 *          to @p FALSE the APB bus is used to access GPIOA.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOA_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOA_USE_AHB             TRUE
#endif

/**
 * @brief   GPIOB AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOB. When set
 *          to @p FALSE the APB bus is used to access GPIOB.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOB_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOB_USE_AHB             TRUE
#endif

/**
 * @brief   GPIOC AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOC. When set
 *          to @p FALSE the APB bus is used to access GPIOC.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOC_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOC_USE_AHB             TRUE
#endif

/**
 * @brief   GPIOD AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOD. When set
 *          to @p FALSE the APB bus is used to access GPIOD.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOD_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOD_USE_AHB             TRUE
#endif

/**
 * @brief   GPIOE AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOE. When set
 *          to @p FALSE the APB bus is used to access GPIOE.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOE_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOE_USE_AHB             TRUE
#endif

/**
 * @brief   GPIOF AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOF. When set
 *          to @p FALSE the APB bus is used to access GPIOF.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOF_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOF_USE_AHB             TRUE
#endif

/**
 * @brief   GPIOG AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOG. When set
 *          to @p FALSE the APB bus is used to access GPIOG.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOG_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOG_USE_AHB             TRUE
#endif

/**
 * @brief   GPIOH AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOH. When set
 *          to @p FALSE the APB bus is used to access GPIOH.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOH_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOH_USE_AHB             TRUE
#endif

/**
 * @brief   GPIOJ AHB enable switch.
 * @details When set to @p TRUE the AHB bus is used to access GPIOJ. When set
 *          to @p FALSE the APB bus is used to access GPIOJ.
 * @note    The default is TRUE.
 */
#if !defined(TIVA_GPIO_GPIOJ_USE_AHB) || defined(__DOXYGEN__)
#define TIVA_GPIO_GPIOJ_USE_AHB             TRUE
#endif

#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if defined(TM4C123x)

#if TIVA_GPIO_GPIOA_USE_AHB
#define GPIOA                               GPIOA_AHB
#else
#define GPIOA                               GPIOA_APB
#endif

#if TIVA_GPIO_GPIOB_USE_AHB
#define GPIOB                               GPIOB_AHB
#else
#define GPIOB                               GPIOB_APB
#endif

#if TIVA_GPIO_GPIOC_USE_AHB
#define GPIOC                               GPIOC_AHB
#else
#define GPIOC                               GPIOC_APB
#endif

#if TIVA_GPIO_GPIOD_USE_AHB
#define GPIOD                               GPIOD_AHB
#else
#define GPIOD                               GPIOD_APB
#endif

#if TIVA_GPIO_GPIOE_USE_AHB
#define GPIOE                               GPIOE_AHB
#else
#define GPIOE                               GPIOE_APB
#endif

#if TIVA_GPIO_GPIOF_USE_AHB
#define GPIOF                               GPIOF_AHB
#else
#define GPIOF                               GPIOF_APB
#endif

#if TIVA_GPIO_GPIOG_USE_AHB
#define GPIOG                               GPIOG_AHB
#else
#define GPIOG                               GPIOG_APB
#endif

#if TIVA_GPIO_GPIOH_USE_AHB
#define GPIOH                               GPIOH_AHB
#else
#define GPIOH                               GPIOH_APB
#endif

#if TIVA_GPIO_GPIOJ_USE_AHB
#define GPIOJ                               GPIOJ_AHB
#else
#define GPIOJ                               GPIOJ_APB
#endif

#define GPIOK                               GPIOK_AHB
#define GPIOL                               GPIOL_AHB
#define GPIOM                               GPIOM_AHB
#define GPION                               GPION_AHB
#define GPIOP                               GPIOP_AHB
#define GPIOQ                               GPIOQ_AHB

#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   GPIO port setup info.
 */
typedef struct 
{
  /** @brief Initial value for DATA register.*/
  uint32_t data;
  /** @brief Initial value for DIR register.*/
  uint32_t dir;
  /** @brief Initial value for AFSEL register.*/
  uint32_t afsel;
  /** @brief Initial value for DR2R register.*/
  uint32_t dr2r;
  /** @brief Initial value for DR4R register.*/
  uint32_t dr4r;
  /** @brief Initial value for DR8R register.*/
  uint32_t dr8r;
  /** @brief Initial value for ODR register.*/
  uint32_t odr;
  /** @brief Initial value for PUR register.*/
  uint32_t pur;
  /** @brief Initial value for PDR register.*/
  uint32_t pdr;
  /** @brief Initial value for SLR register.*/
  uint32_t slr;
  /** @brief Initial value for DEN register.*/
  uint32_t den;
  /** @brief Initial value for AMSEL register.*/
  uint32_t amsel;
  /** @brief Initial value for PCTL register.*/
  uint32_t pctl;
} tiva_gpio_setup_t;

/**
 * @brief   Tiva GPIO static initializer.
 * @details An instance of this structure must be passed to @p palInit() at
 *          system startup time in order to initialized the digital I/O
 *          subsystem. This represents only the initial setup, specific pads
 *          or whole ports can be reprogrammed at later time.
 */
typedef struct
{
  /** @brief Port A setup data.*/
  tiva_gpio_setup_t     PAData;
  /** @brief Port B setup data.*/
  tiva_gpio_setup_t     PBData;
  /** @brief Port C setup data.*/
  tiva_gpio_setup_t     PCData;
  /** @brief Port D setup data.*/
  tiva_gpio_setup_t     PDData;
  /** @brief Port E setup data.*/
  tiva_gpio_setup_t     PEData;
  /** @brief Port F setup data.*/
  tiva_gpio_setup_t     PFData;

#if TIVA_HAS_GPIOG || defined(__DOXYGEN__)
  /** @brief Port G setup data.*/
  tiva_gpio_setup_t     PGData;
#endif /* TIVA_HAS_GPIOG.*/

#if TIVA_HAS_GPIOH || defined(__DOXYGEN__)
  /** @brief Port H setup data.*/
  tiva_gpio_setup_t     PHData;
#endif /* TIVA_HAS_GPIOH.*/

#if TIVA_HAS_GPIOJ || defined(__DOXYGEN__)
  /** @brief Port J setup data.*/
  tiva_gpio_setup_t     PJData;
#endif /* TIVA_HAS_GPIOJ.*/

#if TIVA_HAS_GPIOK || defined(__DOXYGEN__)
  /** @brief Port K setup data.*/
  tiva_gpio_setup_t     PKData;
#endif /* TIVA_HAS_GPIOK.*/

#if TIVA_HAS_GPIOL || defined(__DOXYGEN__)
  /** @brief Port L setup data.*/
  tiva_gpio_setup_t     PLData;
#endif /* TIVA_HAS_GPIOL.*/

#if TIVA_HAS_GPIOM || defined(__DOXYGEN__)
  /** @brief Port M setup data.*/
  tiva_gpio_setup_t     PMData;
#endif /* TIVA_HAS_GPIOM.*/

#if TIVA_HAS_GPION || defined(__DOXYGEN__)
  /** @brief Port N setup data.*/
  tiva_gpio_setup_t     PNData;
#endif /* TIVA_HAS_GPION.*/

#if TIVA_HAS_GPIOP || defined(__DOXYGEN__)
  /** @brief Port P setup data.*/
  tiva_gpio_setup_t     PPData;
#endif /* TIVA_HAS_GPIOP.*/

#if TIVA_HAS_GPIOQ || defined(__DOXYGEN__)
  /** @brief Port Q setup data.*/
  tiva_gpio_setup_t     PQData;
#endif /* TIVA_HAS_GPIOQ.*/

#if TIVA_HAS_GPIOR || defined(__DOXYGEN__)
  /** @brief Port R setup data.*/
  tiva_gpio_setup_t     PRData;
#endif /* TIVA_HAS_GPIOR.*/

#if TIVA_HAS_GPIOS || defined(__DOXYGEN__)
  /** @brief Port S setup data.*/
  tiva_gpio_setup_t     PSData;
#endif /* TIVA_HAS_GPIOS.*/

#if TIVA_HAS_GPIOT || defined(__DOXYGEN__)
  /** @brief Port T setup data.*/
  tiva_gpio_setup_t     PTData;
#endif /* TIVA_HAS_GPIOT.*/
} PALConfig;

/**
 * @brief   Digital I/O port sized unsigned type.
 */
typedef uint32_t ioportmask_t;

/**
 * @brief   Digital I/O modes.
 */
typedef uint32_t iomode_t;

/**
 * @brief   Port Identifier.
 */
typedef GPIO_TypeDef *ioportid_t;

/*===========================================================================*/
/* Driver macros.                                                            */
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
#define pal_lld_readport(port)  ((port)->DATA)

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
#define pal_lld_readlatch(port) ((port)->DATA)

/**
 * @brief   Writes a bits mask on a I/O port.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be written on the specified port
 *
 * @notapi
 */
#define pal_lld_writeport(port, bits)   ((port)->DATA = (bits))

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
#define pal_lld_setport(port, bits) ((port)->MASKED_ACCESS[bits] = 0xFF)

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
#define pal_lld_clearport(port, bits)   ((port)->MASKED_ACCESS[bits] = 0)

/**
 * @brief   Reads a group of bits.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask
 * @param[in] offset    group bit offset within the port
 * @return              The group logical states.
 *
 * @notapi
 */
#define pal_lld_readgroup(port, mask, offset)   \
  ((port)->MASKED_ACCESS[(mask) << (offset)])

/**
 * @brief   Writes a group of bits.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] mask      group mask
 * @param[in] offset    group bit offset within the port
 * @param[in] bits      bits to be written. Values exceeding the group width
 *                      are masked.
 *
 * @notapi
 */
#define pal_lld_writegroup(port, mask, offset, bits)    \
  ((port)->MASKED_ACCESS[(mask) << (offset)] = (bits))

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
#define pal_lld_readpad(port, pad) ((port)->MASKED_ACCESS[1 << (pad)])

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
#define pal_lld_writepad(port, pad, bit)    \
  ((port)->MASKED_ACCESS[1 << (pad)] = (bit))

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
#define pal_lld_setpad(port, pad)   \
  ((port)->MASKED_ACCESS[1 << (pad)] = 1 << (pad))

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
#define pal_lld_clearpad(port, pad) \
  ((port)->MASKED_ACCESS[1 << (pad)] = 0)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern const PALConfig pal_default_config;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void _pal_lld_init(const PALConfig *config);
  void _pal_lld_setgroupmode(ioportid_t port,
                             ioportmask_t mask,
                             iomode_t mode);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PAL */

#endif /* HAL_PAL_LLD_H */

/**
 * @}
 */
