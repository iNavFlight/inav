/*
    Copyright (C) 2013-2015 Andrea Zoppi

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
 * @file    ili9341.c
 * @brief   ILI9341 TFT LCD diaplay controller driver.
 * @note    Does not support multiple calling threads natively.
 */

#include "ch.h"
#include "hal.h"
#include "ili9341.h"

/**
 * @addtogroup ili9341
 * @{
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if !ILI9341_USE_CHECKS && !defined(__DOXYGEN__)
/* Disable checks as needed.*/

#ifdef osalDbgCheck
#undef osalDbgCheck
#endif
#define osalDbgCheck(c, func) {                                               \
  (void)(c), (void)__QUOTE_THIS(func)"()";                                  \
}

#ifdef osalDbgAssert
#undef osalDbgAssert
#endif
#define osalDbgAssert(c, m, r) {                                              \
  (void)(c);                                                                \
}

#ifdef osalDbgCheckClassS
#undef osalDbgCheckClassS
#endif
#define osalDbgCheckClassS() {}

#ifdef osalDbgCheckClassS
#undef osalDbgCheckClassS
#endif
#define osalDbgCheckClassI() {}

#endif /* ILI9341_USE_CHECKS */

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief ILI9341D1 driver identifier.*/
ILI9341Driver ILI9341D1;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initializes the standard part of a @p ILI9341Driver structure.
 *
 * @param[out] driverp  pointer to the @p ILI9341Driver object
 *
 * @init
 */
void ili9341ObjectInit(ILI9341Driver *driverp) {

  osalDbgCheck(driverp != NULL);

  driverp->state = ILI9341_STOP;
  driverp->config = NULL;
#if (TRUE == ILI9341_USE_MUTUAL_EXCLUSION)
#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxObjectInit(&driverp->lock);
#else
  chSemObjectInit(&driverp->lock, 1);
#endif
#endif /* (TRUE == ILI9341_USE_MUTUAL_EXCLUSION) */
}

/**
 * @brief   Configures and activates the ILI9341 peripheral.
 * @pre     ILI9341 is stopped.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 * @param[in] configp   pointer to the @p ILI9341Config object
 *
 * @api
 */
void ili9341Start(ILI9341Driver *driverp, const ILI9341Config *configp) {

  chSysLock();
  osalDbgCheck(driverp != NULL);
  osalDbgCheck(configp != NULL);
  osalDbgCheck(configp->spi != NULL);
  osalDbgAssert(driverp->state == ILI9341_STOP, "invalid state");

  spiSelectI(configp->spi);
  spiUnselectI(configp->spi);
  driverp->config = configp;
  driverp->state = ILI9341_READY;
  chSysUnlock();
}

/**
 * @brief   Deactivates the ILI9341 peripheral.
 * @pre     ILI9341 is ready.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @api
 */
void ili9341Stop(ILI9341Driver *driverp) {

  chSysLock();
  osalDbgCheck(driverp != NULL);
  osalDbgAssert(driverp->state == ILI9341_READY, "invalid state");

  driverp->state = ILI9341_STOP;
  chSysUnlock();
}

#if ILI9341_USE_MUTUAL_EXCLUSION

/**
 * @brief   Gains exclusive access to the ILI9341 module.
 * @details This function tries to gain ownership to the ILI9341 module, if the
 *          module is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p ILI9341_USE_MUTUAL_EXCLUSION must be enabled.
 * @pre     ILI9341 is ready.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @sclass
 */
void ili9341AcquireBusS(ILI9341Driver *driverp) {

  osalDbgCheckClassS();
  osalDbgCheck(driverp == &ILI9341D1);
  osalDbgAssert(driverp->state == ILI9341_READY, "not ready");

#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxLockS(&driverp->lock);
#else
  chSemWaitS(&driverp->lock);
#endif
}

/**
 * @brief   Gains exclusive access to the ILI9341 module.
 * @details This function tries to gain ownership to the ILI9341 module, if the
 *          module is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p ILI9341_USE_MUTUAL_EXCLUSION must be enabled.
 * @pre     ILI9341 is ready.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @api
 */
void ili9341AcquireBus(ILI9341Driver *driverp) {

  chSysLock();
  ili9341AcquireBusS(driverp);
  chSysUnlock();
}

/**
 * @brief   Releases exclusive access to the ILI9341 module.
 * @pre     In order to use this function the option
 *          @p ILI9341_USE_MUTUAL_EXCLUSION must be enabled.
 * @pre     ILI9341 is ready.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @sclass
 */
void ili9341ReleaseBusS(ILI9341Driver *driverp) {

  osalDbgCheckClassS();
  osalDbgCheck(driverp == &ILI9341D1);
  osalDbgAssert(driverp->state == ILI9341_READY, "not ready");

#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxUnlockS(&driverp->lock);
#else
  chSemSignalI(&driverp->lock);
#endif
}

/**
 * @brief   Releases exclusive access to the ILI9341 module.
 * @pre     In order to use this function the option
 *          @p ILI9341_USE_MUTUAL_EXCLUSION must be enabled.
 * @pre     ILI9341 is ready.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @api
 */
void ili9341ReleaseBus(ILI9341Driver *driverp) {

  chSysLock();
  ili9341ReleaseBusS(driverp);
  chSysUnlock();
}

#endif /* ILI9341_USE_MUTUAL_EXCLUSION */

#if ILI9341_IM == ILI9341_IM_4LSI_1 /* 4-wire, half-duplex */

/**
 * @brief   Asserts the slave select signal and prepares for transfers.
 * @pre     ILI9341 is ready.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @iclass
 */
void ili9341SelectI(ILI9341Driver *driverp) {

  osalDbgCheckClassI();
  osalDbgCheck(driverp != NULL);
  osalDbgAssert(driverp->state == ILI9341_READY, "invalid state");

  driverp->state = ILI9341_ACTIVE;
  spiSelectI(driverp->config->spi);
}

/**
 * @brief   Asserts the slave select signal and prepares for transfers.
 * @pre     ILI9341 is ready.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @api
 */
void ili9341Select(ILI9341Driver *driverp) {

  chSysLock();
  ili9341SelectI(driverp);
  chSysUnlock();
}

/**
 * @brief   Deasserts the slave select signal.
 * @details The previously selected peripheral is unselected.
 * @pre     ILI9341 is active.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @iclass
 */
void ili9341UnselectI(ILI9341Driver *driverp) {

  osalDbgCheckClassI();
  osalDbgCheck(driverp != NULL);
  osalDbgAssert(driverp->state == ILI9341_ACTIVE, "invalid state");

  spiUnselectI(driverp->config->spi);
  driverp->state = ILI9341_READY;
}

/**
 * @brief   Deasserts the slave select signal.
 * @details The previously selected peripheral is unselected.
 * @pre     ILI9341 is active.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @iclass
 */
void ili9341Unselect(ILI9341Driver *driverp) {

  chSysLock();
  ili9341UnselectI(driverp);
  chSysUnlock();
}

/**
 * @brief   Write command byte.
 * @details Sends a command byte via SPI.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 * @param[in] cmd       command byte
 *
 * @api
 */
void ili9341WriteCommand(ILI9341Driver *driverp, uint8_t cmd) {

  osalDbgCheck(driverp != NULL);
  osalDbgAssert(driverp->state == ILI9341_ACTIVE, "invalid state");

  driverp->value = cmd;
  palClearPad(driverp->config->dcx_port, driverp->config->dcx_pad);  /* !Cmd */
  spiSend(driverp->config->spi, 1, &driverp->value);
}

/**
 * @brief   Write data byte.
 * @details Sends a data byte via SPI.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 * @param[in] value     data byte
 *
 * @api
 */
void ili9341WriteByte(ILI9341Driver *driverp, uint8_t value) {

  osalDbgCheck(driverp != NULL);
  osalDbgAssert(driverp->state == ILI9341_ACTIVE, "invalid state");

  driverp->value = value;
  palSetPad(driverp->config->dcx_port, driverp->config->dcx_pad);  /* Data */
  spiSend(driverp->config->spi, 1, &driverp->value);
}

/**
 * @brief   Read data byte.
 * @details Receives a data byte via SPI.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 *
 * @return              data byte
 *
 * @api
 */
uint8_t ili9341ReadByte(ILI9341Driver *driverp) {

  osalDbgAssert(FALSE, "should not be used");

  osalDbgCheck(driverp != NULL);
  osalDbgAssert(driverp->state == ILI9341_ACTIVE, "invalid state");

  palSetPad(driverp->config->dcx_port, driverp->config->dcx_pad);  /* Data */
  spiReceive(driverp->config->spi, 1, &driverp->value);
  return driverp->value;
}

/**
 * @brief   Write data chunk.
 * @details Sends a data chunk via SPI.
 * @pre     The chunk must be accessed by DMA.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 * @param[in] chunk     chunk bytes
 * @param[in] length    chunk length
 *
 * @api
 */
void ili9341WriteChunk(ILI9341Driver *driverp, const uint8_t chunk[],
                       size_t length) {

  osalDbgCheck(driverp != NULL);
  osalDbgCheck(chunk != NULL);
  osalDbgAssert(driverp->state == ILI9341_ACTIVE, "invalid state");

  if (length != 0) {
    palSetPad(driverp->config->dcx_port, driverp->config->dcx_pad);  /* Data */
    spiSend(driverp->config->spi, length, chunk);
  }
}

/**
 * @brief   Read data chunk.
 * @details Receives a data chunk via SPI.
 * @pre     The chunk must be accessed by DMA.
 *
 * @param[in] driverp   pointer to the @p ILI9341Driver object
 * @param[out] chunk    chunk bytes
 * @param[in] length    chunk length
 *
 * @api
 */
void ili9341ReadChunk(ILI9341Driver *driverp, uint8_t chunk[],
                      size_t length) {

  osalDbgCheck(driverp != NULL);
  osalDbgCheck(chunk != NULL);
  osalDbgAssert(driverp->state == ILI9341_ACTIVE, "invalid state");

  if (length != 0) {
    palSetPad(driverp->config->dcx_port, driverp->config->dcx_pad);  /* Data */
    spiReceive(driverp->config->spi, length, chunk);
  }
}

#else /* ILI9341_IM == * */
#error "Only the ILI9341_IM_4LSI_1 interface mode is currently supported"
#endif /* ILI9341_IM == * */

/** @} */
