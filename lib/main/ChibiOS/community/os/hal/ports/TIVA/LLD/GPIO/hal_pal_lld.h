/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    GPIO/hal_pal_lld.h
 * @brief   TM4C123x/TM4C129x PAL subsystem low level driver header.
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
/** @} */

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
#define PAL_IOPORTS_WIDTH 8

/**
 * @brief   Whole port mask.
 * @brief   This macro specifies all the valid bits into a port.
 */
#define PAL_WHOLE_PORT ((ioportmask_t)0xFF)
/** @} */

/**
 * @name    Line handling macros
 * @{
 */
/**
 * @brief   Forms a line identifier.
 * @details A port/pad pair are encoded into an @p ioline_t type. The encoding
 *          of this type is platform-dependent.
 * @note    In this driver the pad number is encoded in the lower 4 bits of
 *          the GPIO address which are guaranteed to be zero.
 */
#define PAL_LINE(port, pad)                                                 \
  ((ioline_t)((uint32_t)(port)) | ((uint32_t)(pad)))

/**
 * @brief   Decodes a port identifier from a line identifier.
 */
#define PAL_PORT(line)                                                      \
  ((ioportid_t)(((uint32_t)(line)) & 0xFFFFFFF0U))

/**
 * @brief   Decodes a pad identifier from a line identifier.
 */
#define PAL_PAD(line)                                                       \
  ((uint32_t)((uint32_t)(line) & 0x0000000FU))

/**
 * @brief   Value identifying an invalid line.
 */
#define PAL_NOLINE                      0U
/** @} */

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
  /** @brief GPIO port A setup data.*/
  tiva_gpio_setup_t     PAData;
  /** @brief GPIO port B setup data.*/
  tiva_gpio_setup_t     PBData;
  /** @brief GPIO port C setup data.*/
  tiva_gpio_setup_t     PCData;
  /** @brief GPIO port D setup data.*/
  tiva_gpio_setup_t     PDData;
  /** @brief GPIO port E setup data.*/
  tiva_gpio_setup_t     PEData;
  /** @brief GPIO port F setup data.*/
  tiva_gpio_setup_t     PFData;
#if TIVA_HAS_GPIOG || defined(__DOXYGEN__)
  /** @brief GPIO port G setup data.*/
  tiva_gpio_setup_t     PGData;
#endif
#if TIVA_HAS_GPIOH || defined(__DOXYGEN__)
  /** @brief GPIO port H setup data.*/
  tiva_gpio_setup_t     PHData;
#endif
#if TIVA_HAS_GPIOJ || defined(__DOXYGEN__)
  /** @brief GPIO port J setup data.*/
  tiva_gpio_setup_t     PJData;
#endif
#if TIVA_HAS_GPIOK || defined(__DOXYGEN__)
  /** @brief GPIO port K setup data.*/
  tiva_gpio_setup_t     PKData;
#endif
#if TIVA_HAS_GPIOL || defined(__DOXYGEN__)
  /** @brief GPIO port L setup data.*/
  tiva_gpio_setup_t     PLData;
#endif
#if TIVA_HAS_GPIOM || defined(__DOXYGEN__)
  /** @brief GPIO port M setup data.*/
  tiva_gpio_setup_t     PMData;
#endif
#if TIVA_HAS_GPION || defined(__DOXYGEN__)
  /** @brief GPIO port N setup data.*/
  tiva_gpio_setup_t     PNData;
#endif
#if TIVA_HAS_GPIOP || defined(__DOXYGEN__)
  /** @brief GPIO port P setup data.*/
  tiva_gpio_setup_t     PPData;
#endif
#if TIVA_HAS_GPIOQ || defined(__DOXYGEN__)
  /** @brief GPIO port Q setup data.*/
  tiva_gpio_setup_t     PQData;
#endif
#if TIVA_HAS_GPIOR || defined(__DOXYGEN__)
  /** @brief GPIO port R setup data.*/
  tiva_gpio_setup_t     PRData;
#endif
#if TIVA_HAS_GPIOS || defined(__DOXYGEN__)
  /** @brief GPIO port S setup data.*/
  tiva_gpio_setup_t     PSData;
#endif
#if TIVA_HAS_GPIOT || defined(__DOXYGEN__)
  /** @brief GPIO port T setup data.*/
  tiva_gpio_setup_t     PTData;
#endif
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
 * @brief   Type of an I/O line.
 */
typedef uint32_t ioline_t;

/**
 * @brief   Type of an event mode.
 */
typedef uint32_t ioeventmode_t;

/**
 * @brief   Port Identifier.
 */
typedef uint32_t ioportid_t;

/**
 * @brief   Type of an pad identifier.
 */
typedef uint32_t iopadid_t;

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   GPIOA interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOA_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOB interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOB_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOB_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOC interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOC_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOC_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOD interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOD_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOD_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOE interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOE_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOE_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOF interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOF_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOF_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOG interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOG_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOG_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOH interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOH_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOH_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOJ interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOJ_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOJ_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOK interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOK_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOK_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOL interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOL_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOL_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOM interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOM_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOM_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPION interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPION_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPION_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOP0 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOP0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOP0_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP1 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOP1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOP1_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP2 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOP2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOP2_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP3 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOP3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOP3_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP4 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOP4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOP4_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP5 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOP5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOP5_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP6 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOP6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOP6_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP7 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOP7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOP7_IRQ_PRIORITY        3
#endif
/** @} */

/**
 * @brief   GPIOQ0 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOQ0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOQ0_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ1 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOQ1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOQ1_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ2 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOQ2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOQ2_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ3 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOQ3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOQ3_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ4 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOQ4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOQ4_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ5 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOQ5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOQ5_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ6 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOQ6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOQ6_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ7 interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOQ7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOQ7_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOR interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOR_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOR_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOS interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOS_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOS_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOT interrupt priority level setting.
 */
#if !defined(TIVA_PAL_GPIOT_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PAL_GPIOT_IRQ_PRIORITY         3
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#define GPIOA                               GPIO_PORTA_AHB_BASE
#define GPIOB                               GPIO_PORTB_AHB_BASE
#define GPIOC                               GPIO_PORTC_AHB_BASE
#define GPIOD                               GPIO_PORTD_AHB_BASE
#define GPIOE                               GPIO_PORTE_AHB_BASE
#define GPIOF                               GPIO_PORTF_AHB_BASE
#define GPIOG                               GPIO_PORTG_AHB_BASE
#define GPIOH                               GPIO_PORTH_AHB_BASE
#define GPIOJ                               GPIO_PORTJ_AHB_BASE
#define GPIOK                               GPIO_PORTK_BASE
#define GPIOL                               GPIO_PORTL_BASE
#define GPIOM                               GPIO_PORTM_BASE
#define GPION                               GPIO_PORTN_BASE
#define GPIOP                               GPIO_PORTP_BASE
#define GPIOQ                               GPIO_PORTQ_BASE
#define GPIOR                               GPIO_PORTR_BASE
#define GPIOS                               GPIO_PORTS_BASE
#define GPIOT                               GPIO_PORTT_BASE

#if TIVA_HAS_GPIOA &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOA_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOA"
#endif

#if TIVA_HAS_GPIOB &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOB_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOB"
#endif

#if TIVA_HAS_GPIOC &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOC_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOC"
#endif

#if TIVA_HAS_GPIOD &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOD_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOD"
#endif

#if TIVA_HAS_GPIOE &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOE_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOE"
#endif

#if TIVA_HAS_GPIOF &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOF_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOF"
#endif

#if TIVA_HAS_GPIOG &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOG_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOG"
#endif

#if TIVA_HAS_GPIOH &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOH_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOH"
#endif

#if TIVA_HAS_GPIOJ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOJ_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOJ"
#endif

#if TIVA_HAS_GPIOK &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOK_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOK"
#endif

#if TIVA_HAS_GPIOL &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOL_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOL"
#endif

#if TIVA_HAS_GPIOM &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOM_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOM"
#endif

#if TIVA_HAS_GPION &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPION_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPION"
#endif

#if TIVA_HAS_GPIOP &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOP0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP0"
#endif

#if TIVA_HAS_GPIOP &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOP1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP1"
#endif

#if TIVA_HAS_GPIOP &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOP2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP2"
#endif

#if TIVA_HAS_GPIOP &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOP3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP3"
#endif

#if TIVA_HAS_GPIOP &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOP4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP4"
#endif

#if TIVA_HAS_GPIOP &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOP5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP5"
#endif

#if TIVA_HAS_GPIOP &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOP6_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP6"
#endif

#if TIVA_HAS_GPIOP &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOP7_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP7"
#endif

#if TIVA_HAS_GPIOQ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOQ0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ0"
#endif

#if TIVA_HAS_GPIOQ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOQ1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ1"
#endif

#if TIVA_HAS_GPIOQ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOQ2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ2"
#endif

#if TIVA_HAS_GPIOQ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOQ3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ3"
#endif

#if TIVA_HAS_GPIOQ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOQ4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ4"
#endif

#if TIVA_HAS_GPIOQ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOQ5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ5"
#endif

#if TIVA_HAS_GPIOQ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOQ6_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ6"
#endif

#if TIVA_HAS_GPIOQ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOQ7_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ7"
#endif

#if TIVA_HAS_GPIOR &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOR_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOR"
#endif

#if TIVA_HAS_GPIOS &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOS_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOS"
#endif

#if TIVA_HAS_GPIOT &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PAL_GPIOT_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOT"
#endif

/*===========================================================================*/
/* I/O Ports Identifiers.                                                    */
/*===========================================================================*/

/**
 * @brief   GPIO port A identifier.
 */
#define IOPORT1         GPIOA

/**
 * @brief   GPIO port B identifier.
 */
#define IOPORT2         GPIOB

/**
 * @brief   GPIO port C identifier.
 */
#define IOPORT3         GPIOC

/**
 * @brief   GPIO port D identifier.
 */
#define IOPORT4         GPIOD

/**
 * @brief   GPIO port E identifier.
 */
#define IOPORT5         GPIOE

/**
 * @brief   GPIO port F identifier.
 */
#define IOPORT6         GPIOF

/**
 * @brief   GPIO port G identifier.
 */
#if TIVA_HAS_GPIOG || defined(__DOXYGEN__)
#define IOPORT7         GPIOG
#endif

/**
 * @brief   GPIO port H identifier.
 */
#if TIVA_HAS_GPIOH || defined(__DOXYGEN__)
#define IOPORT8         GPIOH
#endif

/**
 * @brief   GPIO port J identifier.
 */
#if TIVA_HAS_GPIOJ || defined(__DOXYGEN__)
#define IOPORT9         GPIOJ
#endif

/**
 * @brief   GPIO port K identifier.
 */
#if TIVA_HAS_GPIOK || defined(__DOXYGEN__)
#define IOPORT10        GPIOK
#endif

/**
 * @brief   GPIO port L identifier.
 */
#if TIVA_HAS_GPIOL || defined(__DOXYGEN__)
#define IOPORT11        GPIOL
#endif

/**
 * @brief   GPIO port M identifier.
 */
#if TIVA_HAS_GPIOM || defined(__DOXYGEN__)
#define IOPORT12        GPIOM
#endif

/**
 * @brief   GPIO port N identifier.
 */
#if TIVA_HAS_GPION || defined(__DOXYGEN__)
#define IOPORT13        GPION
#endif

/**
 * @brief   GPIO port P identifier.
 */
#if TIVA_HAS_GPIOP || defined(__DOXYGEN__)
#define IOPORT14        GPIOP
#endif

/**
 * @brief   GPIO port Q identifier.
 */
#if TIVA_HAS_GPIOQ || defined(__DOXYGEN__)
#define IOPORT15        GPIOQ
#endif

/**
 * @brief   GPIO port R identifier.
 */
#if TIVA_HAS_GPIOR || defined(__DOXYGEN__)
#define IOPORT16        GPIOR
#endif

/**
 * @brief   GPIO port S identifier.
 */
#if TIVA_HAS_GPIOS || defined(__DOXYGEN__)
#define IOPORT17        GPIOS
#endif

/**
 * @brief   GPIO port T identifier.
 */
#if TIVA_HAS_GPIOT || defined(__DOXYGEN__)
#define IOPORT18        GPIOT
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
#define pal_lld_readport(port) (HWREG((port) + GPIO_O_DATA + (0xff << 2)))

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
#define pal_lld_readlatch(port) pal_lld_readport(port)

/**
 * @brief   Writes a bits mask on a I/O port.
 *
 * @param[in] port      port identifier
 * @param[in] bits      bits to be written on the specified port
 *
 * @notapi
 */
#define pal_lld_writeport(port, bits) (HWREG((port) + GPIO_O_DATA + (0xff << 2)) = (bits))

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
#define pal_lld_setport(port, bits) (HWREG((port) + (GPIO_O_DATA + (bits << 2))) = 0xFF)

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
#define pal_lld_clearport(port, bits) (HWREG((port) + (GPIO_O_DATA + (bits << 2))) = 0)

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
  (HWREG((port) + (GPIO_O_DATA + (((mask) << (offset)) << 2))))

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
  (HWREG((port) + (GPIO_O_DATA + (((mask) << (offset)) << 2))) = (bits))

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
#define pal_lld_readpad(port, pad) (HWREG((port) + (GPIO_O_DATA + ((1 << (pad)) << 2))))

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
  (HWREG((port) + (GPIO_O_DATA + ((1 << (pad)) << 2))) = 1 << (bit))

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
  (HWREG((port) + (GPIO_O_DATA + ((1 << (pad)) << 2))) = 1 << (pad))

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
  (HWREG((port) + (GPIO_O_DATA + ((1 << (pad)) << 2))) = 0)

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
  &_pal_events[((((((uint32_t)port - (uint32_t)GPIOA) >> 12) & 0x1FU) * 8) + pad)];

/**
 * @brief   Returns a PAL event structure associated to a line.
 *
 * @param[in] line      line identifier
 *
 * @notapi
 */
#define pal_lld_get_line_event(line)                                        \
  &_pal_events[((((((uint32_t)PAL_PORT(line) - (uint32_t)GPIOA) >> 12) & 0x1FU) * 8) + PAL_PAD(line))]

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern const PALConfig pal_default_config;
extern palevent_t _pal_events[TIVA_GPIO_PINS];
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void _pal_lld_init(const PALConfig *config);
  void _pal_lld_setgroupmode(ioportid_t port,
                             ioportmask_t mask,
                             iomode_t mode);
#if PAL_USE_CALLBACKS || PAL_USE_WAIT
  void _pal_lld_enablepadevent(ioportid_t port,
                               iopadid_t pad,
                               ioeventmode_t mode);
  void _pal_lld_disablepadevent(ioportid_t port, iopadid_t pad);
  void pal_lld_disable_irqs(void);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PAL */

#endif /* HAL_PAL_LLD_H */

/** @} */
