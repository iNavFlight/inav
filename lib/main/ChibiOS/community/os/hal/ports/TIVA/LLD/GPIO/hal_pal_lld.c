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
 * @file    GPIO/hal_pal_lld.c
 * @brief   TM4C123x/TM4C129x PAL subsystem low level driver.
 *
 * @addtogroup PAL
 * @{
 */

#include "hal.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if TIVA_HAS_GPIOA || defined(__DOXYGEN__)
#define GPIOA_BIT       (1 << 0)
#else
#define GPIOA_BIT       0
#endif

#if TIVA_HAS_GPIOB || defined(__DOXYGEN__)
#define GPIOB_BIT       (1 << 1)
#else
#define GPIOB_BIT       0
#endif

#if TIVA_HAS_GPIOC || defined(__DOXYGEN__)
#define GPIOC_BIT       (1 << 2)
#else
#define GPIOC_BIT       0
#endif

#if TIVA_HAS_GPIOD || defined(__DOXYGEN__)
#define GPIOD_BIT       (1 << 3)
#else
#define GPIOD_BIT       0
#endif

#if TIVA_HAS_GPIOE || defined(__DOXYGEN__)
#define GPIOE_BIT       (1 << 4)
#else
#define GPIOE_BIT       0
#endif

#if TIVA_HAS_GPIOF || defined(__DOXYGEN__)
#define GPIOF_BIT       (1 << 5)
#else
#define GPIOF_BIT       0
#endif

#if TIVA_HAS_GPIOG || defined(__DOXYGEN__)
#define GPIOG_BIT       (1 << 6)
#else
#define GPIOG_BIT       0
#endif

#if TIVA_HAS_GPIOH || defined(__DOXYGEN__)
#define GPIOH_BIT       (1 << 7)
#else
#define GPIOH_BIT       0
#endif

#if TIVA_HAS_GPIOJ || defined(__DOXYGEN__)
#define GPIOJ_BIT       (1 << 8)
#else
#define GPIOJ_BIT       0
#endif

#if TIVA_HAS_GPIOK || defined(__DOXYGEN__)
#define GPIOK_BIT       (1 << 9)
#else
#define GPIOK_BIT       0
#endif

#if TIVA_HAS_GPIOL || defined(__DOXYGEN__)
#define GPIOL_BIT       (1 << 10)
#else
#define GPIOL_BIT       0
#endif

#if TIVA_HAS_GPIOM || defined(__DOXYGEN__)
#define GPIOM_BIT       (1 << 11)
#else
#define GPIOM_BIT       0
#endif

#if TIVA_HAS_GPION || defined(__DOXYGEN__)
#define GPION_BIT       (1 << 12)
#else
#define GPION_BIT       0
#endif

#if TIVA_HAS_GPIOP || defined(__DOXYGEN__)
#define GPIOP_BIT       (1 << 13)
#else
#define GPIOP_BIT       0
#endif

#if TIVA_HAS_GPIOQ || defined(__DOXYGEN__)
#define GPIOQ_BIT       (1 << 14)
#else
#define GPIOQ_BIT       0
#endif

#if TIVA_HAS_GPIOR || defined(__DOXYGEN__)
#define GPIOR_BIT       (1 << 15)
#else
#define GPIOR_BIT       0
#endif

#if TIVA_HAS_GPIOS || defined(__DOXYGEN__)
#define GPIOS_BIT       (1 << 16)
#else
#define GPIOS_BIT       0
#endif

#if TIVA_HAS_GPIOT || defined(__DOXYGEN__)
#define GPIOT_BIT       (1 << 17)
#else
#define GPIOT_BIT       0
#endif

#define RCGCGPIO_MASK   (GPIOA_BIT | GPIOB_BIT | GPIOC_BIT | GPIOD_BIT |      \
                         GPIOE_BIT | GPIOF_BIT | GPIOG_BIT | GPIOH_BIT |      \
                         GPIOJ_BIT | GPIOK_BIT | GPIOL_BIT | GPIOM_BIT |      \
                         GPION_BIT | GPIOP_BIT | GPIOQ_BIT | GPIOR_BIT |      \
                         GPIOS_BIT | GPIOT_BIT)

#define GPIOHBCTL_MASK  (GPIOA_BIT | GPIOB_BIT | GPIOC_BIT | GPIOD_BIT |      \
                         GPIOE_BIT | GPIOF_BIT | GPIOG_BIT | GPIOH_BIT |      \
                         GPIOJ_BIT)

#define GPIOC_JTAG_MASK     (0x0F)
#define GPIOD_NMI_MASK      (0x80)
#define GPIOF_NMI_MASK      (0x01)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   Event records for all GPIO channels.
 */
palevent_t _pal_events[TIVA_GPIO_PINS];

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Initializes the port with the port configuration.
 *
 * @param[in] port      the port identifier
 * @param[in] config    the port configuration
 */
static void gpio_init(ioportid_t port, const tiva_gpio_setup_t *config)
{
  HWREG(port + GPIO_O_DATA)  = config->data;
  HWREG(port + GPIO_O_DIR)   = config->dir;
  HWREG(port + GPIO_O_AFSEL) = config->afsel;
  HWREG(port + GPIO_O_DR2R)  = config->dr2r;
  HWREG(port + GPIO_O_DR4R)  = config->dr4r;
  HWREG(port + GPIO_O_DR8R)  = config->dr8r;
  HWREG(port + GPIO_O_ODR)   = config->odr;
  HWREG(port + GPIO_O_PUR)   = config->pur;
  HWREG(port + GPIO_O_PDR)   = config->pdr;
  HWREG(port + GPIO_O_SLR)   = config->slr;
  HWREG(port + GPIO_O_DEN)   = config->den;
  HWREG(port + GPIO_O_AMSEL) = config->amsel;
  HWREG(port + GPIO_O_PCTL)  = config->pctl;
}

/**
 * @brief   Unlocks the masked pins of the GPIO peripheral.
 * @note    This function is only useful for PORTC0-3, PORTD7 and PORTF0.
 *
 * @param[in] port      the port identifier
 * @param[in] mask      the pin mask
 */
static void gpio_unlock(ioportid_t port, ioportmask_t mask)
{

  HWREG(port + GPIO_O_LOCK) = GPIO_LOCK_KEY;
  HWREG(port + GPIO_O_CR) = mask;
}

#if PAL_USE_CALLBACKS || PAL_USE_WAIT
/**
 * @brief   Enables GPIO IRQ sources.
 */
static void gpio_irq_enable(void)
{
#if TIVA_HAS_GPIOA
  nvicEnableVector(TIVA_GPIOA_NUMBER, TIVA_PAL_GPIOA_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOB
  nvicEnableVector(TIVA_GPIOB_NUMBER, TIVA_PAL_GPIOB_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOC
  nvicEnableVector(TIVA_GPIOC_NUMBER, TIVA_PAL_GPIOC_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOD
  nvicEnableVector(TIVA_GPIOD_NUMBER, TIVA_PAL_GPIOD_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOE
  nvicEnableVector(TIVA_GPIOE_NUMBER, TIVA_PAL_GPIOE_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOF
  nvicEnableVector(TIVA_GPIOF_NUMBER, TIVA_PAL_GPIOF_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOG
  nvicEnableVector(TIVA_GPIOG_NUMBER, TIVA_PAL_GPIOG_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOH
  nvicEnableVector(TIVA_GPIOH_NUMBER, TIVA_PAL_GPIOH_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOJ
  nvicEnableVector(TIVA_GPIOJ_NUMBER, TIVA_PAL_GPIOJ_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOK
  nvicEnableVector(TIVA_GPIOK_NUMBER, TIVA_PAL_GPIOK_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOL
  nvicEnableVector(TIVA_GPIOL_NUMBER, TIVA_PAL_GPIOL_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOM
  nvicEnableVector(TIVA_GPIOM_NUMBER, TIVA_PAL_GPIOM_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPION
  nvicEnableVector(TIVA_GPION_NUMBER, TIVA_PAL_GPION_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOP
  nvicEnableVector(TIVA_GPIOP0_NUMBER, TIVA_PAL_GPIOP0_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP1_NUMBER, TIVA_PAL_GPIOP1_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP2_NUMBER, TIVA_PAL_GPIOP2_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP3_NUMBER, TIVA_PAL_GPIOP3_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP4_NUMBER, TIVA_PAL_GPIOP4_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP5_NUMBER, TIVA_PAL_GPIOP5_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP6_NUMBER, TIVA_PAL_GPIOP6_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP7_NUMBER, TIVA_PAL_GPIOP7_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOQ
  nvicEnableVector(TIVA_GPIOQ0_NUMBER, TIVA_PAL_GPIOQ0_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ1_NUMBER, TIVA_PAL_GPIOQ1_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ2_NUMBER, TIVA_PAL_GPIOQ2_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ3_NUMBER, TIVA_PAL_GPIOQ3_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ4_NUMBER, TIVA_PAL_GPIOQ4_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ5_NUMBER, TIVA_PAL_GPIOQ5_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ6_NUMBER, TIVA_PAL_GPIOQ6_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ7_NUMBER, TIVA_PAL_GPIOQ7_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOR
  nvicEnableVector(TIVA_GPIOR_NUMBER, TIVA_PAL_GPIOR_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOS
  nvicEnableVector(TIVA_GPIOS_NUMBER, TIVA_PAL_GPIOS_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOT
  nvicEnableVector(TIVA_GPIOT_NUMBER, TIVA_PAL_GPIOT_IRQ_PRIORITY);
#endif
}
#endif

#define gpio_serve_irq(mask, pin, channel) {                                \
                                                                            \
  if ((mask) & (1U << (pin))) {                                         \
    _pal_isr_code(channel);                                                 \
  }                                                                         \
}

/**
 * @brief   Generic interrupt serving code for multiple pins per interrupt
 *          handler.
 */
#define ext_lld_serve_port_interrupt(gpio, start)                             \
  do {                                                                        \
    uint32_t mis = HWREG(gpio + GPIO_O_MIS);                                  \
                                                                              \
    HWREG(gpio + GPIO_O_ICR) = mis;                                           \
                                                                              \
    gpio_serve_irq(mis, 0, start + 0);                                        \
    gpio_serve_irq(mis, 1, start + 1);                                        \
    gpio_serve_irq(mis, 2, start + 2);                                        \
    gpio_serve_irq(mis, 3, start + 3);                                        \
    gpio_serve_irq(mis, 4, start + 4);                                        \
    gpio_serve_irq(mis, 5, start + 5);                                        \
    gpio_serve_irq(mis, 6, start + 6);                                        \
    gpio_serve_irq(mis, 7, start + 7);                                        \
  } while (0);

/**
 * @brief   Generic interrupt serving code for single pin per interrupt
 *          handler.
 */
#define ext_lld_serve_pin_interrupt(gpio, start, pin)                         \
  do {                                                                        \
    HWREG(gpio + GPIO_O_ICR) = (1 << pin);                                    \
    gpio_serve_irq((1 << pin), pin, start)                                    \
  } while (0);

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if TIVA_HAS_GPIOA || defined(__DOXYGEN__)
/**
 * @brief   GPIOA interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOA_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOA, 0);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOB || defined(__DOXYGEN__)
/**
 * @brief   GPIOB interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOB_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOB, 8);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOC || defined(__DOXYGEN__)
/**
 * @brief   GPIOC interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOC_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOC, 16);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOD || defined(__DOXYGEN__)
/**
 * @brief   GPIOD interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOD_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOD, 24);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOE || defined(__DOXYGEN__)
/**
 * @brief   GPIOE interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOE_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOE, 32);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOF || defined(__DOXYGEN__)
/**
 * @brief   GPIOF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOF_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOF, 40);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOG || defined(__DOXYGEN__)
/**
 * @brief   GPIOG interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOG_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOG, 48);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOH || defined(__DOXYGEN__)
/**
 * @brief   GPIOH interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOH_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOH, 56);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOJ || defined(__DOXYGEN__)
/**
 * @brief   GPIOJ interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOJ_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOJ, 64);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOK || defined(__DOXYGEN__)
/**
 * @brief   GPIOK interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOK_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOK, 72);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOL || defined(__DOXYGEN__)
/**
 * @brief   GPIOL interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOL_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOL, 80);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOM || defined(__DOXYGEN__)
/**
 * @brief   GPIOM interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOM_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOM, 88);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPION || defined(__DOXYGEN__)
/**
 * @brief   GPION interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPION_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPION, 96);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOP || defined(__DOXYGEN__)
/**
 * @brief   GPIOP0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOP0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOP, 104, 0);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOP1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOP1_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOP, 105, 1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOP2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOP2_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOP, 106, 2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOP3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOP3_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOP, 107, 3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOP4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOP4_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOP, 108, 4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOP5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOP5_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOP, 109, 5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOP6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOP6_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOP, 110, 6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOP7 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOP7_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOP, 111, 7);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOQ || defined(__DOXYGEN__)
/**
 * @brief   GPIOQ0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOQ0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOQ, 112, 0);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOQ1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOQ1_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOQ, 113, 1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOQ2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOQ2_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOQ, 114, 2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOQ3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOQ3_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOQ, 115, 3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOQ4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOQ4_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOQ, 116, 4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOQ5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOQ5_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOQ, 117, 5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOQ6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOQ6_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOQ, 118, 6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   GPIOQ7 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOQ7_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_pin_interrupt(GPIOQ, 119, 7);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOR || defined(__DOXYGEN__)
/**
 * @brief   GPIOR interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOR_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOR, 120);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOS || defined(__DOXYGEN__)
/**
 * @brief   GPIOS interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOS_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOS, 128);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_HAS_GPIOT || defined(__DOXYGEN__)
/**
 * @brief   GPIOT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPIOT_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  ext_lld_serve_port_interrupt(GPIOT, 132);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Tiva I/O ports configuration.
 * @details Ports A-F (G, H, J, K, L, M, N, P, Q, R, S, T) clocks enabled.
 *
 * @param[in] config    the Tiva ports configuration
 *
 * @notapi
 */
void _pal_lld_init(const PALConfig *config)
{
#if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__)
  unsigned i;

  for (i = 0; i < TIVA_GPIO_PINS; i++) {
    _pal_init_event(i);
  }
#endif

  /*
   * Enables all GPIO clocks.
   */
  HWREG(SYSCTL_RCGCGPIO) = RCGCGPIO_MASK;
#if defined(TM4C123x)
  HWREG(SYSCTL_GPIOHBCTL) = GPIOHBCTL_MASK;
#endif

  /* Wait until all GPIO modules are ready */
  while (!((HWREG(SYSCTL_PRGPIO) & RCGCGPIO_MASK) == RCGCGPIO_MASK))
    ;
  
#if TIVA_HAS_GPIOA
  gpio_init(GPIOA, &config->PAData);
#endif
#if TIVA_HAS_GPIOB
  gpio_init(GPIOB, &config->PBData);
#endif
#if TIVA_HAS_GPIOC
  /* Unlock JTAG pins.*/
  gpio_unlock(GPIOC, GPIOC_JTAG_MASK);
  gpio_init(GPIOC, &config->PCData);
#endif
#if TIVA_HAS_GPIOD
  /* Unlock NMI pin.*/
  gpio_unlock(GPIOD, GPIOD_NMI_MASK);
  gpio_init(GPIOD, &config->PDData);
#endif
#if TIVA_HAS_GPIOE
  gpio_init(GPIOE, &config->PEData);
#endif
#if TIVA_HAS_GPIOF
  /* Unlock NMI pin.*/
  gpio_unlock(GPIOF, GPIOF_NMI_MASK);
  gpio_init(GPIOF, &config->PFData);
#endif
#if TIVA_HAS_GPIOG || defined(__DOXYGEN__)
  gpio_init(GPIOG, &config->PGData);
#endif
#if TIVA_HAS_GPIOH || defined(__DOXYGEN__)
  gpio_init(GPIOH, &config->PHData);
#endif
#if TIVA_HAS_GPIOJ || defined(__DOXYGEN__)
  gpio_init(GPIOJ, &config->PJData);
#endif
#if TIVA_HAS_GPIOK || defined(__DOXYGEN__)
  gpio_init(GPIOK, &config->PKData);
#endif
#if TIVA_HAS_GPIOL || defined(__DOXYGEN__)
  gpio_init(GPIOL, &config->PLData);
#endif
#if TIVA_HAS_GPIOM || defined(__DOXYGEN__)
  gpio_init(GPIOM, &config->PMData);
#endif
#if TIVA_HAS_GPION || defined(__DOXYGEN__)
  gpio_init(GPION, &config->PNData);
#endif
#if TIVA_HAS_GPIOP || defined(__DOXYGEN__)
  gpio_init(GPIOP, &config->PPData);
#endif
#if TIVA_HAS_GPIOQ || defined(__DOXYGEN__)
  gpio_init(GPIOQ, &config->PQData);
#endif
#if TIVA_HAS_GPIOR || defined(__DOXYGEN__)
  gpio_init(GPIOR, &config->PRData);
#endif
#if TIVA_HAS_GPIOS || defined(__DOXYGEN__)
  gpio_init(GPIOS, &config->PSData);
#endif
#if TIVA_HAS_GPIOT || defined(__DOXYGEN__)
  gpio_init(GPIOT, &config->PTData);
#endif
#if PAL_USE_CALLBACKS || PAL_USE_WAIT
  gpio_irq_enable();
#endif
}

/**
 * @brief   Pads mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 *
 * @param[in] port      the port identifier
 * @param[in] mask      the group mask
 * @param[in] mode      the mode
 *
 * @notapi
 */
void _pal_lld_setgroupmode(ioportid_t port, ioportmask_t mask, iomode_t mode)
{
  uint32_t dir   = (mode & PAL_TIVA_DIR_MASK)   >> 0;
  uint32_t afsel = (mode & PAL_TIVA_AFSEL_MASK) >> 1;
  uint32_t dr2r  = (mode & PAL_TIVA_DR2R_MASK)  >> 2;
  uint32_t dr4r  = (mode & PAL_TIVA_DR4R_MASK)  >> 3;
  uint32_t dr8r  = (mode & PAL_TIVA_DR8R_MASK)  >> 4;
  uint32_t odr   = (mode & PAL_TIVA_ODR_MASK)   >> 5;
  uint32_t pur   = (mode & PAL_TIVA_PUR_MASK)   >> 6;
  uint32_t pdr   = (mode & PAL_TIVA_PDR_MASK)   >> 7;
  uint32_t slr   = (mode & PAL_TIVA_SLR_MASK)   >> 8;
  uint32_t den   = (mode & PAL_TIVA_DEN_MASK)   >> 9;
  uint32_t amsel = (mode & PAL_TIVA_AMSEL_MASK) >> 10;
  uint32_t pctl  = (mode & PAL_TIVA_PCTL_MASK)  >> 11;
  uint32_t bit   = 0;

  while(TRUE) {
    uint32_t pctl_mask = (7 << (4 * bit));
    uint32_t bit_mask = (1 << bit);

    if ((mask & 1) != 0) {
      HWREG(port + GPIO_O_DIR)   = (HWREG(port + GPIO_O_DIR)   & ~bit_mask)  | dir;
      HWREG(port + GPIO_O_AFSEL) = (HWREG(port + GPIO_O_AFSEL) & ~bit_mask)  | afsel;
      HWREG(port + GPIO_O_DR2R)  = (HWREG(port + GPIO_O_DR2R)  & ~bit_mask)  | dr2r;
      HWREG(port + GPIO_O_DR4R)  = (HWREG(port + GPIO_O_DR4R)  & ~bit_mask)  | dr4r;
      HWREG(port + GPIO_O_DR8R)  = (HWREG(port + GPIO_O_DR8R)  & ~bit_mask)  | dr8r;
      HWREG(port + GPIO_O_ODR)   = (HWREG(port + GPIO_O_ODR)   & ~bit_mask)  | odr;
      HWREG(port + GPIO_O_PUR)   = (HWREG(port + GPIO_O_PUR)   & ~bit_mask)  | pur;
      HWREG(port + GPIO_O_PDR)   = (HWREG(port + GPIO_O_PDR)   & ~bit_mask)  | pdr;
      HWREG(port + GPIO_O_SLR)   = (HWREG(port + GPIO_O_SLR)   & ~bit_mask)  | slr;
      HWREG(port + GPIO_O_DEN)   = (HWREG(port + GPIO_O_DEN)   & ~bit_mask)  | den;
      HWREG(port + GPIO_O_AMSEL) = (HWREG(port + GPIO_O_AMSEL) & ~bit_mask)  | amsel;
      HWREG(port + GPIO_O_PCTL)  = (HWREG(port + GPIO_O_PCTL)  & ~pctl_mask) | pctl;
    }

    mask >>= 1;
    if (!mask) {
      return;
    }

    dir   <<= 1;
    afsel <<= 1;
    dr2r  <<= 1;
    dr4r  <<= 1;
    dr8r  <<= 1;
    odr   <<= 1;
    pur   <<= 1;
    pdr   <<= 1;
    slr   <<= 1;
    den   <<= 1;
    amsel <<= 1;
    pctl  <<= 4;

    bit++;
  }
}

#if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__)
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
void _pal_lld_enablepadevent(ioportid_t port,
                             iopadid_t pad,
                             ioeventmode_t mode)
{
  //uint8_t portidx;
  uint32_t padmask;

  //portidx = (((uint32_t)port - (uint32_t)GPIOA) >> 12) & 0x1FU;
  padmask = (1 << pad);

  /* Disable interrupt before changing edge configuration.*/
  HWREG(port + GPIO_O_IM) &= ~padmask;

  /* Configure pin to be edge-sensitive.*/
  HWREG(port + GPIO_O_IS) &= ~(1 << pad);

  /* Configure edges */
  switch(mode & PAL_EVENT_MODE_EDGES_MASK) {
  case PAL_EVENT_MODE_BOTH_EDGES:
    HWREG(port + GPIO_O_IBE) |= padmask;
    break;
  case PAL_EVENT_MODE_RISING_EDGE:
    HWREG(port + GPIO_O_IBE) &= ~padmask;
    HWREG(port + GPIO_O_IEV) &= ~padmask;
    break;
  case PAL_EVENT_MODE_FALLING_EDGE:
    HWREG(port + GPIO_O_IBE) &= ~padmask;
    HWREG(port + GPIO_O_IEV) |= padmask;
    break;
  default:
    /* Interrupt is already disabled */
    break;
  }

  if (mode & PAL_EVENT_MODE_EDGES_MASK) {
    /* Enable interrupt for this pad */
    HWREG(port + GPIO_O_IM) |= padmask;
  }
}

/**
 * @brief   Pad event disable.
 * @details This function disables previously programmed event callbacks.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
void _pal_lld_disablepadevent(ioportid_t port, iopadid_t pad)
{
  uint8_t portidx;
  uint8_t eventidx;

  portidx = (((uint32_t)port - (uint32_t)GPIOA) >> 12) & 0x1FU;

  eventidx = portidx * 8 + pad;

  HWREG(port + GPIO_O_IM) &= ~(1 << pad);

#if PAL_USE_CALLBACKS || PAL_USE_WAIT
  /* Callback cleared and/or thread reset.*/
  _pal_clear_event(eventidx);
#endif
}

/**
 * @brief   Disables GPIO IRQ sources.
 */
void pal_lld_disable_irqs(void)
{
#if TIVA_HAS_GPIOA
  nvicDisableVector(TIVA_GPIOA_NUMBER);
#endif
#if TIVA_HAS_GPIOB
  nvicDisableVector(TIVA_GPIOB_NUMBER);
#endif
#if TIVA_HAS_GPIOC
  nvicDisableVector(TIVA_GPIOC_NUMBER);
#endif
#if TIVA_HAS_GPIOD
  nvicDisableVector(TIVA_GPIOD_NUMBER);
#endif
#if TIVA_HAS_GPIOE
  nvicDisableVector(TIVA_GPIOE_NUMBER);
#endif
#if TIVA_HAS_GPIOF
  nvicDisableVector(TIVA_GPIOF_NUMBER);
#endif
#if TIVA_HAS_GPIOG
  nvicDisableVector(TIVA_GPIOG_NUMBER);
#endif
#if TIVA_HAS_GPIOH
  nvicDisableVector(TIVA_GPIOH_NUMBER);
#endif
#if TIVA_HAS_GPIOJ
  nvicDisableVector(TIVA_GPIOJ_NUMBER);
#endif
#if TIVA_HAS_GPIOK
  nvicDisableVector(TIVA_GPIOK_NUMBER);
#endif
#if TIVA_HAS_GPIOL
  nvicDisableVector(TIVA_GPIOL_NUMBER);
#endif
#if TIVA_HAS_GPIOM
  nvicDisableVector(TIVA_GPIOM_NUMBER);
#endif
#if TIVA_HAS_GPION
  nvicDisableVector(TIVA_GPION_NUMBER);
#endif
#if TIVA_HAS_GPIOP
  nvicDisableVector(TIVA_GPIOP0_NUMBER);
  nvicDisableVector(TIVA_GPIOP1_NUMBER);
  nvicDisableVector(TIVA_GPIOP2_NUMBER);
  nvicDisableVector(TIVA_GPIOP3_NUMBER);
  nvicDisableVector(TIVA_GPIOP4_NUMBER);
  nvicDisableVector(TIVA_GPIOP5_NUMBER);
  nvicDisableVector(TIVA_GPIOP6_NUMBER);
  nvicDisableVector(TIVA_GPIOP7_NUMBER);
#endif
#if TIVA_HAS_GPIOQ
  nvicDisableVector(TIVA_GPIOQ0_NUMBER);
  nvicDisableVector(TIVA_GPIOQ1_NUMBER);
  nvicDisableVector(TIVA_GPIOQ2_NUMBER);
  nvicDisableVector(TIVA_GPIOQ3_NUMBER);
  nvicDisableVector(TIVA_GPIOQ4_NUMBER);
  nvicDisableVector(TIVA_GPIOQ5_NUMBER);
  nvicDisableVector(TIVA_GPIOQ6_NUMBER);
  nvicDisableVector(TIVA_GPIOQ7_NUMBER);
#endif
#if TIVA_HAS_GPIOR
  nvicDisableVector(TIVA_GPIOR_NUMBER);
#endif
#if TIVA_HAS_GPIOS
  nvicDisableVector(TIVA_GPIOS_NUMBER);
#endif
#if TIVA_HAS_GPIOT
  nvicDisableVector(TIVA_GPIOT_NUMBER);
#endif
}
#endif /* PAL_USE_CALLBACKS || PAL_USE_WAIT */

#endif /* HAL_USE_PAL */

/**
 * @}
 */
