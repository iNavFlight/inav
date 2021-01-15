/*
    ChibiOS - Copyright (C) 2014 Derek Mulcahy

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
 * @file    PORTv1/hal_ext_lld.c
 * @brief   KINETIS EXT subsystem low level driver source.
 *
 * @addtogroup EXT
 * @{
 */

#include "hal.h"

#if HAL_USE_EXT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define PCR_IRQC_DISABLED           0x0
#define PCR_IRQC_DMA_RISING_EDGE    0x1
#define PCR_IRQC_DMA_FALLING_EDGE   0x2
#define PCR_IRQC_DMA_EITHER_EDGE    0x3

#define PCR_IRQC_LOGIC_ZERO         0x8
#define PCR_IRQC_RISING_EDGE        0x9
#define PCR_IRQC_FALLING_EDGE       0xA
#define PCR_IRQC_EITHER_EDGE        0xB
#define PCR_IRQC_LOGIC_ONE          0xC

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

/* A channel map for each channel.
 *
 * The index is the pin number.
 * The result is the channel for that pin.
 */
#if KINETIS_EXT_PORTA_WIDTH > 0
uint8_t porta_channel_map[KINETIS_EXT_PORTA_WIDTH];
#endif
#if KINETIS_EXT_PORTB_WIDTH > 0
uint8_t portb_channel_map[KINETIS_EXT_PORTB_WIDTH];
#endif
#if KINETIS_EXT_PORTC_WIDTH > 0
uint8_t portc_channel_map[KINETIS_EXT_PORTC_WIDTH];
#endif
#if KINETIS_EXT_PORTD_WIDTH > 0
uint8_t portd_channel_map[KINETIS_EXT_PORTD_WIDTH];
#endif
#if KINETIS_EXT_PORTE_WIDTH > 0
uint8_t porte_channel_map[KINETIS_EXT_PORTE_WIDTH];
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Enables EXTI IRQ sources.
 *
 * @notapi
 */
static void ext_lld_exti_irq_enable(void) {

#if KINETIS_EXT_PORTA_WIDTH > 0
  nvicEnableVector(PINA_IRQn, KINETIS_EXT_PORTA_IRQ_PRIORITY);
#endif

#if KINETIS_EXT_HAS_COMMON_BCDE_IRQ
#if  (KINETIS_EXT_PORTB_WIDTH > 0) || (KINETIS_EXT_PORTC_WIDTH > 0) \
  || (KINETIS_EXT_PORTD_WIDTH > 0) || (KINETIS_EXT_PORTE_WIDTH > 0)
  nvicEnableVector(PINBCDE_IRQn, KINETIS_EXT_PORTD_IRQ_PRIORITY);
#endif

#elif KINETIS_EXT_HAS_COMMON_CD_IRQ /* KINETIS_EXT_HAS_COMMON_BCDE_IRQ */
#if (KINETIS_EXT_PORTC_WIDTH > 0) || (KINETIS_EXT_PORTD_WIDTH > 0)
  nvicEnableVector(PINCD_IRQn, KINETIS_EXT_PORTD_IRQ_PRIORITY);
#endif

#else /* KINETIS_EXT_HAS_COMMON_CD_IRQ */
#if KINETIS_EXT_PORTB_WIDTH > 0
  nvicEnableVector(PINB_IRQn, KINETIS_EXT_PORTB_IRQ_PRIORITY);
#endif
#if KINETIS_EXT_PORTC_WIDTH > 0
  nvicEnableVector(PINC_IRQn, KINETIS_EXT_PORTC_IRQ_PRIORITY);
#endif
#if KINETIS_EXT_PORTD_WIDTH > 0
  nvicEnableVector(PIND_IRQn, KINETIS_EXT_PORTD_IRQ_PRIORITY);
#endif
#if KINETIS_EXT_PORTE_WIDTH > 0
  nvicEnableVector(PINE_IRQn, KINETIS_EXT_PORTE_IRQ_PRIORITY);
#endif
#endif /* !KINETIS_EXT_HAS_COMMON_CD_IRQ */
}

/**
 * @brief   Disables EXTI IRQ sources.
 *
 * @notapi
 */
static void ext_lld_exti_irq_disable(void) {

#if KINETIS_EXT_PORTA_WIDTH > 0
  nvicDisableVector(PINA_IRQn);
#endif

#if KINETIS_EXT_HAS_COMMON_BCDE_IRQ
#if  (KINETIS_EXT_PORTB_WIDTH > 0) || (KINETIS_EXT_PORTC_WIDTH > 0) \
  || (KINETIS_EXT_PORTD_WIDTH > 0) || (KINETIS_EXT_PORTE_WIDTH > 0)
  nvicDisableVector(PINBCDE_IRQn);
#endif

#elif KINETIS_EXT_HAS_COMMON_CD_IRQ /* KINETIS_EXT_HAS_COMMON_BCDE_IRQ */
#if (KINETIS_EXT_PORTC_WIDTH > 0) || (KINETIS_EXT_PORTD_WIDTH > 0)
  nvicDisableVector(PINCD_IRQn);
#endif

#else /* KINETIS_EXT_HAS_COMMON_CD_IRQ */
#if KINETIS_EXT_PORTB_WIDTH > 0
  nvicDisableVector(PINB_IRQn);
#endif
#if KINETIS_EXT_PORTC_WIDTH > 0
  nvicDisableVector(PINC_IRQn);
#endif
#if KINETIS_EXT_PORTD_WIDTH > 0
  nvicDisableVector(PIND_IRQn);
#endif
#if KINETIS_EXT_PORTE_WIDTH > 0
  nvicDisableVector(PINE_IRQn);
#endif
#endif /* !KINETIS_EXT_HAS_COMMON_CD_IRQ */
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*
 * Generic interrupt handler.
 */
static inline void irq_handler(PORT_TypeDef * const port, const unsigned port_width, const uint8_t *channel_map) {
  unsigned pin;
  uint32_t isfr = port->ISFR;

  /* Clear all pending interrupts on this port. */
  port->ISFR = 0xFFFFFFFF;

  for (pin = 0; pin < port_width; pin++) {
    if (isfr & (1 << pin)) {
      expchannel_t channel = channel_map[pin];
      EXTD1.config->channels[channel].cb(&EXTD1, channel);
    }
  }
}

/**
 * @brief   PORTA interrupt handler.
 *
 * @isr
 */
#if defined(KINETIS_PORTA_IRQ_VECTOR) && KINETIS_EXT_PORTA_WIDTH > 0
OSAL_IRQ_HANDLER(KINETIS_PORTA_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  irq_handler(PORTA, KINETIS_EXT_PORTA_WIDTH, porta_channel_map);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_EXT_PORTA_WIDTH > 0 */

#if KINETIS_EXT_HAS_COMMON_BCDE_IRQ

#if defined(KINETIS_PORTD_IRQ_VECTOR)
OSAL_IRQ_HANDLER(KINETIS_PORTD_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

#if (KINETIS_EXT_PORTB_WIDTH > 0)
  irq_handler(PORTB, KINETIS_EXT_PORTB_WIDTH, portb_channel_map);
#endif
#if (KINETIS_EXT_PORTC_WIDTH > 0)
  irq_handler(PORTC, KINETIS_EXT_PORTC_WIDTH, portc_channel_map);
#endif
#if (KINETIS_EXT_PORTD_WIDTH > 0)
  irq_handler(PORTD, KINETIS_EXT_PORTD_WIDTH, portd_channel_map);
#endif
#if (KINETIS_EXT_PORTE_WIDTH > 0)
  irq_handler(PORTE, KINETIS_EXT_PORTE_WIDTH, porte_channel_map);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* defined(KINETIS_PORTD_IRQ_VECTOR) */

#elif KINETIS_EXT_HAS_COMMON_CD_IRQ /* KINETIS_EXT_HAS_COMMON_BCDE_IRQ */

#if defined(KINETIS_PORTD_IRQ_VECTOR)
OSAL_IRQ_HANDLER(KINETIS_PORTD_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

#if (KINETIS_EXT_PORTC_WIDTH > 0)
  irq_handler(PORTC, KINETIS_EXT_PORTC_WIDTH, portc_channel_map);
#endif
#if (KINETIS_EXT_PORTD_WIDTH > 0)
  irq_handler(PORTD, KINETIS_EXT_PORTD_WIDTH, portd_channel_map);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* defined(KINETIS_PORTD_IRQ_VECTOR) */


#else /* KINETIS_EXT_HAS_COMMON_CD_IRQ */

/**
 * @brief   PORTB interrupt handler.
 *
 * @isr
 */
#if defined(KINETIS_PORTB_IRQ_VECTOR) && KINETIS_EXT_PORTB_WIDTH > 0
OSAL_IRQ_HANDLER(KINETIS_PORTB_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  irq_handler(PORTB, KINETIS_EXT_PORTB_WIDTH, portb_channel_map);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_EXT_PORTB_WIDTH > 0 */

/**
 * @brief   PORTC interrupt handler.
 *
 * @isr
 */
#if defined(KINETIS_PORTC_IRQ_VECTOR) && KINETIS_EXT_PORTC_WIDTH > 0
OSAL_IRQ_HANDLER(KINETIS_PORTC_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  irq_handler(PORTC, KINETIS_EXT_PORTC_WIDTH, portc_channel_map);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_EXT_PORTC_WIDTH > 0 */

/**
 * @brief   PORTD interrupt handler.
 *
 * @isr
 */
#if defined(KINETIS_PORTD_IRQ_VECTOR) && KINETIS_EXT_PORTD_WIDTH > 0
OSAL_IRQ_HANDLER(KINETIS_PORTD_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  irq_handler(PORTD, KINETIS_EXT_PORTD_WIDTH, portd_channel_map);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_EXT_PORTD_WIDTH > 0 */

/**
 * @brief   PORTE interrupt handler.
 *
 * @isr
 */
#if defined(KINETIS_PORTE_IRQ_VECTOR) && KINETIS_EXT_PORTE_WIDTH > 0
OSAL_IRQ_HANDLER(KINETIS_PORTE_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  irq_handler(PORTE, KINETIS_EXT_PORTE_WIDTH, porte_channel_map);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_EXT_PORTE_WIDTH > 0 */

#endif /* !KINETIS_EXT_HAS_COMMON_CD_IRQ */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level EXT driver initialization.
 *
 * @notapi
 */
void ext_lld_init(void) {

  /* Driver initialization.*/
  extObjectInit(&EXTD1);
}

/**
 * @brief   Configures and activates the EXT peripheral.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 *
 * @notapi
 */
void ext_lld_start(EXTDriver *extp) {
  expchannel_t channel;

  if (extp->state == EXT_STOP)
    ext_lld_exti_irq_enable();

  /* Configuration of automatic channels.*/
  for (channel = 0; channel < EXT_MAX_CHANNELS; channel++) {

    uint32_t mode = extp->config->channels[channel].mode;
    PORT_TypeDef *port = extp->config->channels[channel].port;
    uint32_t pin = extp->config->channels[channel].pin;

    /* Initialize the channel map */
#if KINETIS_EXT_PORTA_WIDTH > 0
    if (port == PORTA)
      porta_channel_map[pin] = channel;
    else
#endif
#if KINETIS_EXT_PORTB_WIDTH > 0
    if (port == PORTB)
      portb_channel_map[pin] = channel;
    else
#endif
#if KINETIS_EXT_PORTC_WIDTH > 0
    if (port == PORTC)
      portc_channel_map[pin] = channel;
    else
#endif
#if KINETIS_EXT_PORTD_WIDTH > 0
    if (port == PORTD)
      portd_channel_map[pin] = channel;
    else
#endif
#if KINETIS_EXT_PORTE_WIDTH > 0
    if (port == PORTE)
      porte_channel_map[pin] = channel;
    else
#endif
    {}

    if (mode & EXT_CH_MODE_AUTOSTART)
      ext_lld_channel_enable(extp, channel);
    else if (port != NULL)
      ext_lld_channel_disable(extp, channel);
  }
}

/**
 * @brief   Deactivates the EXT peripheral.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 *
 * @notapi
 */
void ext_lld_stop(EXTDriver *extp) {

  if (extp->state == EXT_ACTIVE)
    ext_lld_exti_irq_disable();
}

/**
 * @brief   Enables an EXT channel.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be enabled
 *
 * @notapi
 */
void ext_lld_channel_enable(EXTDriver *extp, expchannel_t channel) {

  uint32_t irqc;
  uint32_t mode = extp->config->channels[channel].mode;
  if (mode & EXT_CH_MODE_RISING_EDGE)
    irqc = PCR_IRQC_RISING_EDGE;
  else if (extp->config->channels[channel].mode & EXT_CH_MODE_FALLING_EDGE)
    irqc = PCR_IRQC_FALLING_EDGE;
  else if (extp->config->channels[channel].mode & EXT_CH_MODE_BOTH_EDGES)
    irqc = PCR_IRQC_EITHER_EDGE;
  else
    irqc = PCR_IRQC_DISABLED;

  PORT_TypeDef *port = extp->config->channels[channel].port;
  uint32_t pin = extp->config->channels[channel].pin;

  uint32_t pcr = port->PCR[pin];

  /* Clear all the IRQC bits */
  pcr &= ~PORTx_PCRn_IRQC_MASK;
  /* Set the required IRQC bits */
  pcr |= PORTx_PCRn_IRQC(irqc);

  port->PCR[pin] = pcr;
}

/**
 * @brief   Disables an EXT channel.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be disabled
 *
 * @notapi
 */
void ext_lld_channel_disable(EXTDriver *extp, expchannel_t channel) {

  PORT_TypeDef *port = extp->config->channels[channel].port;
  uint32_t pin = extp->config->channels[channel].pin;
  port->PCR[pin] |= PORTx_PCRn_IRQC(PCR_IRQC_DISABLED);
}

#endif /* HAL_USE_EXT */

/** @} */
