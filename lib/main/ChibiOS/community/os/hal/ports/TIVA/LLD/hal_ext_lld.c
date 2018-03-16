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
 * @file    Tiva/ext_lld.c
 * @brief   Tiva EXT subsystem low level driver source.
 *
 * @addtogroup EXT
 * @{
 */

#include "hal.h"

#if HAL_USE_EXT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   Generic interrupt serving code for multiple pins per interrupt
 *          handler.
 */
#define ext_lld_serve_port_interrupt(gpiop, start)                            \
  do {                                                                        \
    uint32_t mis = gpiop->MIS;                                                \
                                                                              \
    gpiop->ICR = mis;                                                         \
                                                                              \
    if (mis & (1 << 0)) {                                                     \
      EXTD1.config->channels[start + 0].cb(&EXTD1, start + 0);                \
    }                                                                         \
    if (mis & (1 << 1)) {                                                     \
      EXTD1.config->channels[start + 1].cb(&EXTD1, start + 1);                \
    }                                                                         \
    if (mis & (1 << 2)) {                                                     \
      EXTD1.config->channels[start + 2].cb(&EXTD1, start + 2);                \
    }                                                                         \
    if (mis & (1 << 3)) {                                                     \
      EXTD1.config->channels[start + 3].cb(&EXTD1, start + 3);                \
    }                                                                         \
    if (mis & (1 << 4)) {                                                     \
      EXTD1.config->channels[start + 4].cb(&EXTD1, start + 4);                \
    }                                                                         \
    if (mis & (1 << 5)) {                                                     \
      EXTD1.config->channels[start + 5].cb(&EXTD1, start + 5);                \
    }                                                                         \
    if (mis & (1 << 6)) {                                                     \
      EXTD1.config->channels[start + 6].cb(&EXTD1, start + 6);                \
    }                                                                         \
    if (mis & (1 << 7)) {                                                     \
      EXTD1.config->channels[start + 7].cb(&EXTD1, start + 7);                \
    }                                                                         \
  } while (0);

/**
 * @brief   Generic interrupt serving code for single pin per interrupt
 *          handler.
 */
#define ext_lld_serve_pin_interrupt(gpiop, start, pin)                        \
  do {                                                                        \
    gpiop->ICR = (1 << pin);                                                  \
    EXTD1.config->channels[start].cb(&EXTD1, start);                          \
  } while (0);

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   EXTD1 driver identifier.
 */
EXTDriver EXTD1;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

const ioportid_t gpio[] =
{
#if TIVA_HAS_GPIOA
  GPIOA,
#endif
#if TIVA_HAS_GPIOB
  GPIOB,
#endif
#if TIVA_HAS_GPIOC
  GPIOC,
#endif
#if TIVA_HAS_GPIOD
  GPIOD,
#endif
#if TIVA_HAS_GPIOE
  GPIOE,
#endif
#if TIVA_HAS_GPIOF
  GPIOF,
#endif
#if TIVA_HAS_GPIOG
  GPIOG,
#endif
#if TIVA_HAS_GPIOH
  GPIOH,
#endif
#if TIVA_HAS_GPIOJ
  GPIOJ,
#endif
#if TIVA_HAS_GPIOK
  GPIOK,
#endif
#if TIVA_HAS_GPIOL
  GPIOL,
#endif
#if TIVA_HAS_GPIOM
  GPIOM,
#endif
#if TIVA_HAS_GPION
  GPION,
#endif
#if TIVA_HAS_GPIOP
  GPIOP,
#endif
#if TIVA_HAS_GPIOQ
  GPIOQ,
#endif
#if TIVA_HAS_GPIOR
  GPIOR,
#endif
#if TIVA_HAS_GPIOS
  GPIOS,
#endif
#if TIVA_HAS_GPIOT
  GPIOT,
#endif
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Enables GPIO IRQ sources.
 *
 * @notapi
 */
static void ext_lld_irq_enable(void)
{
#if TIVA_HAS_GPIOA
  nvicEnableVector(TIVA_GPIOA_NUMBER, TIVA_EXT_GPIOA_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOB
  nvicEnableVector(TIVA_GPIOB_NUMBER, TIVA_EXT_GPIOB_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOC
  nvicEnableVector(TIVA_GPIOC_NUMBER, TIVA_EXT_GPIOC_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOD
  nvicEnableVector(TIVA_GPIOD_NUMBER, TIVA_EXT_GPIOD_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOE
  nvicEnableVector(TIVA_GPIOE_NUMBER, TIVA_EXT_GPIOE_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOF
  nvicEnableVector(TIVA_GPIOF_NUMBER, TIVA_EXT_GPIOF_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOG
  nvicEnableVector(TIVA_GPIOG_NUMBER, TIVA_EXT_GPIOG_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOH
  nvicEnableVector(TIVA_GPIOH_NUMBER, TIVA_EXT_GPIOH_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOJ
  nvicEnableVector(TIVA_GPIOJ_NUMBER, TIVA_EXT_GPIOJ_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOK
  nvicEnableVector(TIVA_GPIOK_NUMBER, TIVA_EXT_GPIOK_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOL
  nvicEnableVector(TIVA_GPIOL_NUMBER, TIVA_EXT_GPIOL_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOM
  nvicEnableVector(TIVA_GPIOM_NUMBER, TIVA_EXT_GPIOM_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPION
  nvicEnableVector(TIVA_GPION_NUMBER, TIVA_EXT_GPION_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOP
  nvicEnableVector(TIVA_GPIOP0_NUMBER, TIVA_EXT_GPIOP0_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP1_NUMBER, TIVA_EXT_GPIOP1_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP2_NUMBER, TIVA_EXT_GPIOP2_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP3_NUMBER, TIVA_EXT_GPIOP3_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP4_NUMBER, TIVA_EXT_GPIOP4_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP5_NUMBER, TIVA_EXT_GPIOP5_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP6_NUMBER, TIVA_EXT_GPIOP6_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOP7_NUMBER, TIVA_EXT_GPIOP7_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOQ
  nvicEnableVector(TIVA_GPIOQ0_NUMBER, TIVA_EXT_GPIOQ0_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ1_NUMBER, TIVA_EXT_GPIOQ1_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ2_NUMBER, TIVA_EXT_GPIOQ2_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ3_NUMBER, TIVA_EXT_GPIOQ3_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ4_NUMBER, TIVA_EXT_GPIOQ4_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ5_NUMBER, TIVA_EXT_GPIOQ5_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ6_NUMBER, TIVA_EXT_GPIOQ6_IRQ_PRIORITY);
  nvicEnableVector(TIVA_GPIOQ7_NUMBER, TIVA_EXT_GPIOQ7_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOR
  nvicEnableVector(TIVA_GPIOR_NUMBER, TIVA_EXT_GPIOR_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOS
  nvicEnableVector(TIVA_GPIOS_NUMBER, TIVA_EXT_GPIOS_IRQ_PRIORITY);
#endif
#if TIVA_HAS_GPIOT
  nvicEnableVector(TIVA_GPIOT_NUMBER, TIVA_EXT_GPIOT_IRQ_PRIORITY);
#endif
}

/**
 * @brief   Disables GPIO IRQ sources.
 *
 * @notapi
 */
static void ext_lld_irq_disable(void)
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

  ext_lld_serve_port_interrupt(&GPIOG, 48);

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

  ext_lld_serve_port_interrupt(&GPIOH, 56);

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

  ext_lld_serve_port_interrupt(&GPIOJ, 64);

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

  ext_lld_serve_port_interrupt(&GPIOK, 72);

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

  ext_lld_serve_port_interrupt(&GPIOL, 80);

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

  ext_lld_serve_port_interrupt(&GPIOM, 88);

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

  ext_lld_serve_port_interrupt(&GPION, 96);

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

  ext_lld_serve_pin_interrupt(&GPIOP, 104, 0);

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

  ext_lld_serve_pin_interrupt(&GPIOP, 105, 1);

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

  ext_lld_serve_pin_interrupt(&GPIOP, 106, 2);

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

  ext_lld_serve_pin_interrupt(&GPIOP, 107, 3);

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

  ext_lld_serve_pin_interrupt(&GPIOP, 108, 4);

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

  ext_lld_serve_pin_interrupt(&GPIOP, 109, 5);

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

  ext_lld_serve_pin_interrupt(&GPIOP, 110, 6);

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

  ext_lld_serve_pin_interrupt(&GPIOP, 111, 7);

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

  ext_lld_serve_pin_interrupt(&GPIOQ, 112, 0);

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

  ext_lld_serve_pin_interrupt(&GPIOQ, 113, 1);

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

  ext_lld_serve_pin_interrupt(&GPIOQ, 114, 2);

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

  ext_lld_serve_pin_interrupt(&GPIOQ, 115, 3);

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

  ext_lld_serve_pin_interrupt(&GPIOQ, 116, 4);

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

  ext_lld_serve_pin_interrupt(&GPIOQ, 117, 5);

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

  ext_lld_serve_pin_interrupt(&GPIOQ, 118, 6);

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

  ext_lld_serve_pin_interrupt(&GPIOQ, 119, 7);

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

  ext_lld_serve_port_interrupt(&GPIOR, 120);

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

  ext_lld_serve_port_interrupt(&GPIOS, 128);

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

  ext_lld_serve_port_interrupt(&GPIOT, 132);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level EXT driver initialization.
 *
 * @notapi
 */
void ext_lld_init(void)
{
  extObjectInit(&EXTD1);
}

/**
 * @brief   Configures and activates the EXT peripheral.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 *
 * @notapi
 */
void ext_lld_start(EXTDriver *extp)
{
  uint8_t i;

  if (extp->state == EXT_STOP) {
    ext_lld_irq_enable();
  }

  /* Configuration of automatic channels.*/
  for (i = 0; i < EXT_MAX_CHANNELS; i++) {
    if (extp->config->channels[i].mode & EXT_CH_MODE_AUTOSTART) {
      ext_lld_channel_enable(extp, i);
    }
    else {
      ext_lld_channel_disable(extp, i);
    }
  }
}

/**
 * @brief   Deactivates the EXT peripheral.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 *
 * @notapi
 */
void ext_lld_stop(EXTDriver *extp)
{
  if (extp->state == EXT_ACTIVE) {
    ext_lld_irq_disable();
  }

#if TIVA_HAS_GPIOA
  GPIOA->IM = 0;
#endif
#if TIVA_HAS_GPIOB
  GPIOB->IM = 0;
#endif
#if TIVA_HAS_GPIOC
  GPIOC->IM = 0;
#endif
#if TIVA_HAS_GPIOD
  GPIOD->IM = 0;
#endif
#if TIVA_HAS_GPIOE
  GPIOE->IM = 0;
#endif
#if TIVA_HAS_GPIOF
  GPIOF->IM = 0;
#endif
#if TIVA_HAS_GPIOG
  GPIOG->IM = 0;
#endif
#if TIVA_HAS_GPIOH
  GPIOH->IM = 0;
#endif
#if TIVA_HAS_GPIOJ
  GPIOJ->IM = 0;
#endif
#if TIVA_HAS_GPIOK
  GPIOK->IM = 0;
#endif
#if TIVA_HAS_GPIOL
  GPIOL->IM = 0;
#endif
#if TIVA_HAS_GPIOM
  GPIOM->IM = 0;
#endif
#if TIVA_HAS_GPION
  GPION->IM = 0;
#endif
#if TIVA_HAS_GPIOP
  GPIOP->IM = 0;
#endif
#if TIVA_HAS_GPIOQ
  GPIOQ->IM = 0;
#endif
#if TIVA_HAS_GPIOR
  GPIOR->IM = 0;
#endif
#if TIVA_HAS_GPIOS
  GPIOS->IM = 0;
#endif
#if TIVA_HAS_GPIOT
  GPIOT->IM = 0;
#endif
}

/**
 * @brief   Enables an EXT channel.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be enabled
 *
 * @notapi
 */
void ext_lld_channel_enable(EXTDriver *extp, expchannel_t channel)
{
  GPIO_TypeDef *gpiop;
  uint8_t pin;
  uint32_t im;

  pin = channel & 0x07;
  gpiop = gpio[channel >> 3];

  /* Disable interrupts */
  im = gpiop->IM;
  gpiop->IM = 0;

  /* Configure pin to be edge-sensitive.*/
  gpiop->IS &= ~(1 << pin);

  /* Programming edge registers.*/
  if ((extp->config->channels[channel].mode & EXT_CH_MODE_EDGES_MASK) ==
      EXT_CH_MODE_BOTH_EDGES) {
    gpiop->IBE |= (1 << pin);
  }
  else if ((extp->config->channels[channel].mode & EXT_CH_MODE_EDGES_MASK) ==
      EXT_CH_MODE_FALLING_EDGE) {
    gpiop->IBE &= ~(1 << pin);
    gpiop->IEV &= ~(1 << pin);
  }
  else if ((extp->config->channels[channel].mode & EXT_CH_MODE_EDGES_MASK) ==
      EXT_CH_MODE_RISING_EDGE) {
    gpiop->IBE &= ~(1 << pin);
    gpiop->IEV |= (1 << pin);
  }

  /* Programming interrupt and event registers.*/
  if ((extp->config->channels[channel].cb != NULL) &&
      ((extp->config->channels[channel].mode & EXT_CH_MODE_EDGES_MASK) !=
          EXT_CH_MODE_DISABLED)) {
    im |= (1 << pin);
  }
  else {
    im &= ~(1 << pin);
  }

  /* Restore interrupts */
  gpiop->IM = im;
}

/**
 * @brief   Disables an EXT channel.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be disabled
 *
 * @notapi
 */
void ext_lld_channel_disable(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  GPIO_TypeDef *gpiop;
  uint8_t pin;

  pin = channel & 0x07;
  gpiop = gpio[channel >> 3];

  gpiop->IM &= ~(1 << pin);
}

#endif /* HAL_USE_EXT */

/** @} */
