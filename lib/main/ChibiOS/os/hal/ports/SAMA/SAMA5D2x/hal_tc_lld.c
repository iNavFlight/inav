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
 * @file    SAMA5D2x/hal_tc_lld.c
 * @brief   SAMA TC support code.
 *
 * @addtogroup TC
 * @{
 */

#include "hal.h"

#if HAL_USE_TC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

/**
 * @brief   Enable write protection on TC registers block.
 *
 * @param[in] tcp    pointer to a TC register block
 *
 * @notapi
 */
#define tcEnableWP(tcp) {                                                     \
  tcp->TC_WPMR = TC_WPMR_WPKEY_PASSWD | TC_WPMR_WPEN;                         \
}

/**
 * @brief   Disable write protection on TC registers block.
 *
 * @param[in] tcp    pointer to a TC register block
 *
 * @notapi
 */
#define tcDisableWP(tcp) {                                                    \
  tcp->TC_WPMR = TC_WPMR_WPKEY_PASSWD;                                        \
}

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   TCD0 driver identifier.
 * @note    The driver TCD0 allocates the timer TC0 when enabled.
 */
#if SAMA_USE_TC0 || defined(__DOXYGEN__)
TCDriver TCD0;
#endif

/**
 * @brief   TCD1 driver identifier.
 * @note    The driver TCD1 allocates the timer TC1 when enabled.
 */
#if SAMA_USE_TC1 || defined(__DOXYGEN__)
TCDriver TCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   Common IRQ handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @param[in] tcp      pointer to a @p TCDriver object
 *
 * @notapi
 */
void tc_lld_serve_interrupt(TCDriver *tcp) {
  uint32_t sr, imr, i;

  for (i = 0; i < TC_CHANNELS; i++) {
    sr  = tcp->tim->TC_CHANNEL[i].TC_SR;
    imr = tcp->tim->TC_CHANNEL[i].TC_IMR;
    if (((sr & TC_SR_CPCS) != 0) && ((imr & TC_IMR_CPCS) != 0) &&
       (tcp->config->channels[i].callback != NULL)) {
      tcp->config->channels[i].callback(tcp);
    }
  }
}
/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SAMA_USE_TC0 || defined(__DOXYGEN__)
#if !defined(SAMA_TC0_SUPPRESS_ISR)
/**
 * @brief   TC0 interrupt handler.
 * @note    It is assumed that this interrupt is only activated if the callback
 *          pointer is not equal to @p NULL in order to not perform an extra
 *          check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_TC0_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  tc_lld_serve_interrupt(&TCD0);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif /* !defined(SAMA_TC0_SUPPRESS_ISR) */
#endif /* SAMA_USE_TC0 */

#if SAMA_USE_TC1 || defined(__DOXYGEN__)
#if !defined(SAMA_TC1_SUPPRESS_ISR)
/**
 * @brief   TC1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_TC1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  tc_lld_serve_interrupt(&TCD1);
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif /* !defined(SAMA_TC1_SUPPRESS_ISR) */
#endif /* SAMA_USE_TC1 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level TC driver initialization.
 *
 * @notapi
 */
void tc_lld_init(void) {

#if SAMA_USE_TC0
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_TC0, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  /* Driver initialization.*/
  tcObjectInit(&TCD0);
  TCD0.channels = TC_CHANNELS;
  TCD0.tim = TC0;
  TCD0.clock = SAMA_TC0CLK;
#endif

#if SAMA_USE_TC1
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_TC1, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  /* Driver initialization.*/
  tcObjectInit(&TCD1);
  TCD1.channels = TC_CHANNELS;
  TCD1.tim = TC1;
  TCD1.clock = SAMA_TC1CLK;
#endif

}

/**
 * @brief   Configures and activates the TC peripheral.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 *
 * @notapi
 */
void tc_lld_start(TCDriver *tcp) {
  uint32_t rc = 0;

  if (tcp->state == TC_STOP) {
    /* Clock activation.*/
#if SAMA_USE_TC0
    if (&TCD0 == tcp) {
      pmcEnableTC0();
#if !defined(SAMA_TC0_SUPPRESS_ISR)
      aicSetSourcePriority(ID_TC0, SAMA_TC0_IRQ_PRIORITY);
      aicSetSourceHandler(ID_TC0, SAMA_TC0_HANDLER);
      aicEnableInt(ID_TC0);
#endif
    }
#endif

#if SAMA_USE_TC1
    if (&TCD1 == tcp) {
      pmcEnableTC1();
#if !defined(SAMA_TC1_SUPPRESS_ISR)
      aicSetSourcePriority(ID_TC1, SAMA_TC1_IRQ_PRIORITY);
      aicSetSourceHandler(ID_TC1, SAMA_TC1_HANDLER);
      aicEnableInt(ID_TC1);
#endif
    }
#endif
  }
  /* Disable Write Protection */
  tcDisableWP(tcp->tim);
  /* Output enables*/
   switch (tcp->config->channels[0].mode & TC_OUTPUT_MASK) {
   case TC_OUTPUT_ACTIVE:
     rc = (tcp->clock) / (tcp->config->channels[0].frequency);
     tcp->tim->TC_CHANNEL[0].TC_EMR = TC_EMR_NODIVCLK;
     tcp->tim->TC_CHANNEL[0].TC_CMR = TC_CMR_WAVE | TC_CMR_ACPA_SET |
                                      TC_CMR_ACPC_CLEAR | TC_CMR_WAVSEL_UP_RC;

     tcp->tim->TC_CHANNEL[0].TC_RC = TC_RC_RC(rc);
     tcp->tim->TC_CHANNEL[0].TC_SR;          /* Clear pending IRQs.          */
   default:
     ;
   }
   switch (tcp->config->channels[1].mode & TC_OUTPUT_MASK) {
   case TC_OUTPUT_ACTIVE:
     rc = (tcp->clock) / (tcp->config->channels[1].frequency);
     tcp->tim->TC_CHANNEL[1].TC_EMR = TC_EMR_NODIVCLK;
     tcp->tim->TC_CHANNEL[1].TC_CMR = TC_CMR_WAVE | TC_CMR_ACPA_SET |
                                      TC_CMR_ACPC_CLEAR | TC_CMR_WAVSEL_UP_RC;

     tcp->tim->TC_CHANNEL[1].TC_RC = TC_RC_RC(rc);
     tcp->tim->TC_CHANNEL[1].TC_SR;          /* Clear pending IRQs.          */
   default:
     ;
   }
   switch (tcp->config->channels[2].mode & TC_OUTPUT_MASK) {
   case TC_OUTPUT_ACTIVE:
     rc = (tcp->clock) / (tcp->config->channels[2].frequency);
     tcp->tim->TC_CHANNEL[2].TC_EMR = TC_EMR_NODIVCLK;
     tcp->tim->TC_CHANNEL[2].TC_CMR = TC_CMR_WAVE | TC_CMR_ACPA_SET |
                                      TC_CMR_ACPC_CLEAR | TC_CMR_WAVSEL_UP_RC;

     tcp->tim->TC_CHANNEL[2].TC_RC = TC_RC_RC(rc);
     tcp->tim->TC_CHANNEL[2].TC_SR;                       /* Clear pending IRQs.          */
   default:
     ;
   }
   /* Enable Write Protection */
   tcEnableWP(tcp->tim);
}

/**
 * @brief   Deactivates the TC peripheral.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 *
 * @notapi
 */
void tc_lld_stop(TCDriver *tcp) {

  /* If in ready state then disables the TC clock.*/
  if (tcp->state == TC_READY) {
#if SAMA_USE_TC0
    if (&TCD0 == tcp) {
      aicDisableInt(ID_TC0);
      pmcDisableTC0();
    }
#endif

#if SAMA_USE_TC1
    if (&TCD1 == tcp) {
      aicDisableInt(ID_TC1);
      pmcDisableTC1();
    }
#endif
  }
}

/**
 * @brief   Enables a TC channel.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @post    The channel is active using the specified configuration.
 * @note    The function has effect at the next cycle start.
 * @note    Channel notification is not enabled.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 * @param[in] width     TC pulse width as clock pulses number
 *
 * @notapi
 */
void tc_lld_enable_channel(TCDriver *tcp,
                           tcchannel_t channel,
                           tccnt_t width) {
  /* Disable Write Protection */
  tcDisableWP(tcp->tim);

  /* Changing channel duty cycle on the fly.*/
  uint32_t rc = tcp->tim->TC_CHANNEL[channel].TC_RC;
  tcp->tim->TC_CHANNEL[channel].TC_RA = TC_RA_RA((100 - width) * rc / 100);
  tcp->tim->TC_CHANNEL[channel].TC_CCR = TC_CCR_CLKEN;
  tcp->tim->TC_CHANNEL[channel].TC_CCR = TC_CCR_SWTRG;

  /* Enable Write Protection */
  tcEnableWP(tcp->tim);
}

/**
 * @brief   Disables a TC channel and its notification.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    The function has effect at the next cycle start.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @notapi
 */
void tc_lld_disable_channel(TCDriver *tcp, tcchannel_t channel) {

  tcp->tim->TC_CHANNEL[channel].TC_CCR = TC_CCR_CLKDIS;
}

/**
 * @brief   Enables a channel de-activation edge notification.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @pre     The channel must have been activated using @p tcEnableChannel().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @notapi
 */
void tc_lld_enable_channel_notification(TCDriver *tcp,
                                        tcchannel_t channel) {
  tcp->tim->TC_CHANNEL[channel].TC_IER |= TC_IER_CPCS;
}

/**
 * @brief   Disables a channel de-activation edge notification.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @pre     The channel must have been activated using @p tcEnableChannel().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @notapi
 */
void tc_lld_disable_channel_notification(TCDriver *tcp,
                                         tcchannel_t channel) {

  tcp->tim->TC_CHANNEL[channel].TC_IDR |= TC_IDR_CPCS;
}

/**
 * @brief   Changes TC channel's frequency.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @post    The channel must be enabled using @p tcEnableChannel().
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 */

void tcChangeChannelFrequency(TCDriver *tcp,
                              tcchannel_t channel,uint32_t frequency) {
  tcDisableWP(tcp->tim);
  uint32_t rc =(tcp->clock) / (frequency);
  tcp->tim->TC_CHANNEL[channel].TC_RC = TC_RC_RC(rc);
  tcEnableWP(tcp->tim);
}
/**
 * @brief   TC Driver initialization.
 *
 * @init
 */
void tcInit(void) {

  tc_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p TCDriver structure.
 *
 * @param[out] tcp      pointer to a @p TCDriver object
 *
 * @init
 */
void tcObjectInit(TCDriver *tcp) {

  tcp->state    = TC_STOP;
  tcp->config   = NULL;
  tcp->enabled  = 0;
  tcp->channels = 0;
}


/**
 * @brief   Configures and activates the TC peripheral.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] config    pointer to a @p TCConfig object
 *
 * @api
 */
void tcStart(TCDriver *tcp, const TCConfig *config) {

  osalDbgCheck((tcp != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((tcp->state == TC_STOP) || (tcp->state == TC_READY),
                "invalid state");
  tcp->config = config;
  tc_lld_start(tcp);
  tcp->enabled = 0;
  tcp->state = TC_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the TC peripheral.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 *
 * @api
 */
void tcStop(TCDriver *tcp) {

  osalDbgCheck(tcp != NULL);

  osalSysLock();

  osalDbgAssert((tcp->state == TC_STOP) || (tcp->state == TC_READY),
                "invalid state");

  tc_lld_stop(tcp);
  tcp->enabled = 0;
  tcp->config  = NULL;
  tcp->state   = TC_STOP;

  osalSysUnlock();
}

/**
 * @brief   Enables a TC channel.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 * @param[in] width     TC pulse width as clock pulses number
 *
 * @api
 */
void tcEnableChannel(TCDriver *tcp,
                     tcchannel_t channel,
                     tccnt_t width) {

  osalDbgCheck((tcp != NULL) && (channel < tcp->channels));

  osalSysLock();

  osalDbgAssert(tcp->state == TC_READY, "not ready");

  tcEnableChannelI(tcp, channel, width);

  osalSysUnlock();
}

/**
 * @brief   Disables a TC channel and its notification.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @api
 */
void tcDisableChannel(TCDriver *tcp, tcchannel_t channel) {

  osalDbgCheck((tcp != NULL) && (channel < tcp->channels));

  osalSysLock();

  osalDbgAssert(tcp->state == TC_READY, "not ready");

  tcDisableChannelI(tcp, channel);

  osalSysUnlock();
}

/**
 * @brief   Enables a channel de-activation edge notification.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @pre     The channel must have been activated using @p tcEnableChannel().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @api
 */
void tcEnableChannelNotification(TCDriver *tcp, tcchannel_t channel) {

  osalDbgCheck((tcp != NULL) && (channel < tcp->channels));

  osalSysLock();

  osalDbgAssert(tcp->state == TC_READY, "not ready");
  osalDbgAssert((tcp->enabled & ((tcchnmsk_t)1U << (tcchnmsk_t)channel)) != 0U,
                "channel not enabled");
  osalDbgAssert(tcp->config->channels[channel].callback != NULL,
                "undefined channel callback");

  tcEnableChannelNotificationI(tcp, channel);

  osalSysUnlock();
}

/**
 * @brief   Disables a channel de-activation edge notification.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @pre     The channel must have been activated using @p tcEnableChannel().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @api
 */
void tcDisableChannelNotification(TCDriver *tcp, tcchannel_t channel) {

  osalDbgCheck((tcp != NULL) && (channel < tcp->channels));

  osalSysLock();

  osalDbgAssert(tcp->state == TC_READY, "not ready");
  osalDbgAssert((tcp->enabled & ((tcchnmsk_t)1U << (tcchnmsk_t)channel)) != 0U,
                "channel not enabled");
  osalDbgAssert(tcp->config->channels[channel].callback != NULL,
                "undefined channel callback");

  tcDisableChannelNotificationI(tcp, channel);

  osalSysUnlock();
}

#endif /* HAL_USE_TC */

/** @} */




