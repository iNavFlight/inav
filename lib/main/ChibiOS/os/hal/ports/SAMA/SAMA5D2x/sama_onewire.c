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
 * @file    SAMA5D2x/sama_onewire.c
 * @brief   SAMA ONEWIRE support code.
 *
 * @addtogroup SAMA5D2x_ONEWIRE
 * @{
 */

#include "hal.h"

#if (SAMA_USE_ONEWIRE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
/**
 * @name    Delays in standard speed mode.
 * @{
 */
#define A                                      US2RTC(SAMA_PCK, 6)
#define B                                      US2RTC(SAMA_PCK, 64)
#define C                                      US2RTC(SAMA_PCK, 60)
#define D                                      US2RTC(SAMA_PCK, 10)
#define E                                      US2RTC(SAMA_PCK, 9)
#define F                                      US2RTC(SAMA_PCK, 55)
#define G                                      US2RTC(SAMA_PCK, 0)
#define H                                      US2RTC(SAMA_PCK, 480)
#define I                                      US2RTC(SAMA_PCK, 70)
#define J                                      US2RTC(SAMA_PCK, 410)
/** @} */

#if SAMA_HAL_IS_SECURE
#define PAD_INPUT_MODE                         PAL_SAMA_FUNC_GPIO |          \
                                               PAL_SAMA_DIR_INPUT |          \
                                               PAL_SAMA_OPD_OPENDRAIN |      \
                                               PAL_SAMA_PUEN_PULLUP |        \
                                               PAL_MODE_SECURE

#define PAD_OUTPUT_MODE                        PAL_SAMA_FUNC_GPIO |          \
                                               PAL_SAMA_DIR_OUTPUT |         \
                                               PAL_SAMA_OPD_OPENDRAIN |      \
                                               PAL_SAMA_PUEN_PULLUP |        \
                                               PAL_MODE_SECURE
#else
#define PAD_INPUT_MODE                         PAL_SAMA_FUNC_GPIO |          \
                                               PAL_SAMA_DIR_INPUT |          \
                                               PAL_SAMA_OPD_OPENDRAIN |      \
                                               PAL_SAMA_PUEN_PULLUP

#define PAD_OUTPUT_MODE                        PAL_SAMA_FUNC_GPIO |          \
                                               PAL_SAMA_DIR_OUTPUT |         \
                                               PAL_SAMA_OPD_OPENDRAIN |      \
                                               PAL_SAMA_PUEN_PULLUP
#endif /* SAMA_HAL_IS_SECURE */

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

/**
 * @brief    Set ONEWIRE pin in output mode.
 *
 * @param[in] onewp    pointer to a ONEWIRE driver.
 *
 * @notapi
 */
#define onewireSetPinOutput(onewp) {                                         \
  palSetLineMode(onewp->config->line, PAD_OUTPUT_MODE);                      \
}

/**
 * @brief   Set ONEWIRE pin in input mode.
 *
 * @param[in] onewp    pointer to a ONEWIRE driver.
 *
 * @notapi
 */
#define onewireSetPinInput(onewp) {                                          \
  palSetLineMode(onewp->config->line, PAD_INPUT_MODE);                       \
}

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
ONEWIREDriver ONEWD0;

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   Low level ONEWIRE driver initialization.
 *
 * @notapi
 */
void onewire_lld_init(void) {

  onewireObjectInit(&ONEWD0);
}

/**
 * @brief   Configures and activates the ONEWIRE pin.
 *
 * @param[in] onewp  pointer to the @p ONEWIREDriver object
 *
 * @notapi
 */
void onewire_lld_start(ONEWIREDriver *onewp) {

  /* Set the ONEWIRE pin in output mode. */
  onewireSetPinOutput(onewp);

}

/**
 * @brief   Deactivates the ONEWIRE driver.
 *
 * @param[in] onewp  pointer to the @p ONEWIREDriver object
 *
 * @notapi
 */
void onewire_lld_stop(ONEWIREDriver *onewp) {
  (void) onewp;
}

/**
 * @brief   Send a Reset on ONEWIRE pin.
 *          The reset detect the slave presence on the pin
 *          and ready it for a command.
 *
 * @param[in] onewp    pointer to the @p ONEWIREDriver object
 * @return    result   result of the reset, if 0 a slave is detected
 *
 * @notapi
 */
bool onewire_lld_reset(ONEWIREDriver *onewp) {

  bool result = TRUE;

  /* At the beginning set the pin in output mode. */
  onewireSetPinOutput(onewp);

  /* Wait 0 microseconds. */
  chSysPolledDelayX(G);
  /* Drives pin low. */
  palClearLine(onewp->config->line);
  /* Wait 480 microseconds. */
  chSysPolledDelayX(H);
  /* Drives pin high. */
  palSetLine(onewp->config->line);
  /* Wait 70 microseconds. */
  chSysPolledDelayX(I);
  /* Set the pin in input mode. */
  onewireSetPinInput(onewp);
  /* Read the pin logic state. */
  result = palReadLine(onewp->config->line);
  /* Wait 410 microseconds. */
  chSysPolledDelayX(J);

  return result;
}

/**
 * @brief   Write a bit through ONEWIRE pin.
 *
 * @param[in] onewp     pointer to the @p ONEWIREDriver object
 * @param[in] value     bit value to write
 *
 * @notapi
 */
void onewire_lld_write_bit(ONEWIREDriver *onewp, uint8_t value) {

  osalDbgAssert((value == 0u) || (value == 1u),
                "invalid value");

  /* Set the pin in output mode. */
  onewireSetPinOutput(onewp);

  if (value) {
    /* Write '1' bit */
    /* Drives pin low. */
    palClearLine(onewp->config->line);
    /* Wait 6 microsecond. */
    chSysPolledDelayX(A);
    /* Drives pin high. */
    palSetLine(onewp->config->line);
    /* Wait 64 microseconds to complete the time slot and recovery. */
    chSysPolledDelayX(B);
  }
  else {
    /* Write '0' bit */
    /* Drives pin low. */
    palClearLine(onewp->config->line);
    /* Wait 60 microsecond. */
    chSysPolledDelayX(C);
    /* Drives pin high. */
    palSetLine(onewp->config->line);
    /* Wait 10 microseconds for recovery. */
    chSysPolledDelayX(D);
  }
}

/**
 * @brief   Read a bit through ONEWIRE pin.
 *
 * @param[in] onewp    pointer to the @p ONEWIREDriver object
 * @return    value    bit read
 *
 * @notapi
 */
uint8_t onewire_lld_read_bit(ONEWIREDriver *onewp) {

  uint8_t value;

  /* At the beginning set the pin in output mode. */
  onewireSetPinOutput(onewp);

  /* Drives pin low. */
  palClearLine(onewp->config->line);
  /* Wait 6 microsecond. */
  chSysPolledDelayX(A);
  /* Drives pin high. */
  palSetLine(onewp->config->line);
  /* Wait 9 microseconds. */
  chSysPolledDelayX(E);
  /* Set the pin in input mode. */
  onewireSetPinInput(onewp);
  /* Read the pin logic state. */
  value = palReadLine(onewp->config->line);
  /* Wait 55 microseconds. */
  chSysPolledDelayX(F);

  return value;
}

/**
 * @brief   Write a byte through ONEWIRE pin.
 *
 * @param[in] onewp       pointer to the @p ONEWIREDriver object
 * @param[in] byte        byte to write
 *
 * @notapi
 */
void onewire_lld_write_byte(ONEWIREDriver *onewp, uint8_t byte) {

  uint8_t i;

  /* Loop to write each bit in the byte, LS-bit first */
  for (i = 0; i < 8; i++) {
    onewire_lld_write_bit(onewp, (byte & 0x01));
    /* Shift the data byte for the next bit */
    byte >>= 1;
  }
}

/**
 * @brief   Read a byte through ONEWIRE pin.
 *
 * @param[in] onewp       pointer to the @p ONEWIREDriver object
 * return     value       byte read
 *
 * @notapi
 */
uint8_t onewire_lld_read_byte(ONEWIREDriver *onewp) {

  uint8_t i;
  uint8_t value = 0;

  for (i = 0; i < 8; i++) {
    /* Shift the result to get it ready for the next bit */
    value >>= 1;
    /* If result is one, then set MS bit */
    if (onewire_lld_read_bit(onewp))
      value |= 0x80;
  }
  return value;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/
/**
 * @brief   ONEWIRE driver initialization.
 *
 * @api
 */
void onewireInit(void) {

  onewire_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p ONEWIREDriver structure.
 *
 * @param[out] onewp     pointer to the @p ONEWIREDriver object
 *
 * @init
 */
void onewireObjectInit(ONEWIREDriver *onewp) {

  onewp->state = ONEW_STOP;
  onewp->config = NULL;

  osalMutexObjectInit(&onewp->mutex);
}

/**
 * @brief   Configures and activates the ONEWIRE pin.
 *
 * @param[in] onewp pointer to the @p ONEWIREDriver object
 * @param[in] config    pointer to the @p ONEWIREConfig object
 *
 * @api
 */
void onewireStart(ONEWIREDriver *onewp, const ONEWIREConfig *config) {

  osalDbgCheck((onewp != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((onewp->state == ONEW_STOP) || (onewp->state == ONEW_READY),
                "invalid state");
  onewp->config = config;
  onewire_lld_start(onewp);
  onewp->state = ONEW_READY;
  osalSysUnlock();

}

/**
 * @brief   Deactivates the ONEWIRE driver.
 *
 * @param[in] onewp    pointer to the @p ONEWIREDriver object
 *
 * @api
 */
void onewireStop(ONEWIREDriver *onewp) {

  osalDbgCheck(onewp != NULL);

  osalSysLock();

  osalDbgAssert((onewp->state == ONEW_STOP) || (onewp->state == ONEW_READY),
                "invalid state");

  onewire_lld_stop(onewp);
  onewp->config = NULL;
  onewp->state  = ONEW_STOP;

  osalSysUnlock();
}

/**
 * @brief   Write a block of bytes through ONEWIRE pin.
 *
 * @param[in] onewp       pointer to the @p ONEWIREDriver object
 * @param[in] txbuf       the pointer to the transmit buffer
 * @param[in] n           number of bytes to write
 *
 * @api
 */
void onewireWriteBlockI(ONEWIREDriver *onewp, uint8_t *txbuf, size_t n) {

  uint32_t i;
  (onewp)->state = ONEW_ACTIVE;
  for (i = 0; i < n; i++) {
    onewire_lld_write_byte(onewp, txbuf[i]);
  }
}

/**
 * @brief   Write a block of bytes through ONEWIRE pin.
 *
 * @param[in] onewp       pointer to the @p ONEWIREDriver object
 * @param[in] txbuf       the pointer to the transmit buffer
 * @param[in] n           number of bytes to write
 *
 * @api
 */
void onewireWriteBlock(ONEWIREDriver *onewp, uint8_t *txbuf, size_t n) {

  osalDbgCheck(onewp != NULL);

  osalSysLock();
  osalDbgAssert(onewp->state == ONEW_READY, "not ready");
  onewireWriteBlockI(onewp, txbuf, n);

  (onewp)->state = ONEW_READY;
  osalSysUnlock();
}

/**
 * @brief   Read a block of bytes through ONEWIRE pin.
 *
 * @param[in] onewp       pointer to the @p ONEWIREDriver object
 * @param[out]rxbuf       pointer to the receive buffer
 * @param[in] n           number of bytes to read
 *
 * @api
 */
void onewireReadBlockI(ONEWIREDriver *onewp, uint8_t *rxbuf, size_t n) {

  uint32_t i;
  (onewp)->state = ONEW_ACTIVE;
  for (i = 0; i < n; i++) {
    rxbuf[i] = onewire_lld_read_byte(onewp);
  }
}

/**
 * @brief   Read a block of bytes through ONEWIRE pin.
 *
 * @param[in] onewirep    pointer to the @p ONEWIREDriver object
 * @param[out]rxbuf       pointer to the receive buffer
 * @param[in] n           number of bytes to read
 *
 * @api
 */
void onewireReadBlock(ONEWIREDriver *onewp, uint8_t *rxbuf, size_t n) {

  osalDbgCheck(onewp != NULL);

//  osalSysLock();
  osalDbgAssert(onewp->state == ONEW_READY, "not ready");
  onewireReadBlockI(onewp, rxbuf, n);
  (onewp)->state = ONEW_READY;
//  osalSysUnlock();
}

/**
 * @brief   Send a Reset on ONEWIRE pin.
 *          The reset detect the slave presence on the pin
 *          and ready it for a command.
 *
 * @param[in] onewp    pointer to the @p ONEWIREDriver object
 * @return    result   result of the reset, if 0 a slave is detected
 *
 * @api
 */
bool onewireReset(ONEWIREDriver *onewp) {

  bool detect = TRUE;

  osalDbgCheck(onewp != NULL);

  osalSysLock();
  osalDbgAssert(onewp->state == ONEW_READY,
                "invalid state");
  detect = onewire_lld_reset(onewp);
  osalSysUnlock();

  return detect;
}

/*
 * @brief   Sends a command.
 *
 * @param[in] onewp     pointer to the @p ONEWIREDriver object
 * @param[in] cmdp      pointer command byte
 *
 * @api
 */
void onewireCommandI(ONEWIREDriver *onewp, uint8_t *cmdp, size_t n) {

  uint32_t i;
  (onewp)->state = ONEW_ACTIVE;
  for (i = 0; i < n; i++) {
    onewire_lld_write_byte(onewp, cmdp[i]);
  }
}

/*
 * @brief   Sends a command.
 *
 * @param[in] onewp     pointer to the @p ONEWIREDriver object
 * @param[in] cmdp       pointer to command
 *
 * @api
 */
void onewireCommand(ONEWIREDriver *onewp, uint8_t *cmdp, size_t n) {

 osalDbgCheck((onewp != NULL) && (cmdp != NULL));

// osalSysLock();

 osalDbgAssert(onewp->state == ONEW_READY, "not ready");

 onewireCommandI(onewp, cmdp, n);
 (onewp)->state = ONEW_READY;
// osalSysUnlock();
}


/**
 * @brief   Gains exclusive access to the ONEWIRE bus.
 * @details This function tries to gain ownership to the ONEWIRE bus, if the bus
 *          is already being used then the invoking thread is queued.
 *
 * @param[in] onewp      pointer to the @p ONEWIREDriver object
 *
 * @api
 */
void onewireAcquireBus(ONEWIREDriver *onewp) {

  osalDbgCheck(onewp != NULL);

  osalMutexLock(&onewp->mutex);
}

/**
 * @brief   Releases exclusive access to the ONEWIRE bus.
 *
 * @param[in] onewp      pointer to the @p ONEWIREDriver object
 *
 * @api
 */
void onewireReleaseBus(ONEWIREDriver *onewp) {

  osalDbgCheck(onewp != NULL);

  osalMutexUnlock(&onewp->mutex);
}

#endif /* SAMA_USE_ONEWIRE == TRUE */

/** @} */
