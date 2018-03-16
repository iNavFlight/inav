/*
    ChibiOS - Copyright (C) 2015 Michael D. Spradling

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
 * @file    STM32/CRCv1/crc_lld.c
 * @brief   STM32 CRC subsystem low level driver source.
 *
 * @addtogroup CRC
 * @{
 */

#include "hal.h"

#if (HAL_USE_CRC == TRUE) || defined(__DOXYGEN__)

/**
 * Allow CRC Software override for ST drivers.  Some ST CRC implimentations
 * have limited capabilities.
 */
#if CRCSW_USE_CRC1 != TRUE

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   CRC default configuration.
 */
static const CRCConfig default_config = {
  .poly_size         = 32,
  .poly              = 0x04C11DB7,
  .initial_val       = 0xFFFFFFFF,
  .final_val         = 0xFFFFFFFF,
  .reflect_data      = 1,
  .reflect_remainder = 1
};

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief CRC1 driver identifier.*/
#if STM32_CRC_USE_CRC1 || defined(__DOXYGEN__)
CRCDriver CRCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

void _crc_lld_calc_byte(CRCDriver *crcp, uint8_t data) {
  __IO uint8_t *crc8 = (__IO uint8_t*)&(crcp->crc->DR);
  *crc8 = data;
}

/*
 * @brief   Returns calculated CRC from last reset
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] data      data to be added to crc
 *
 * @notapi
 */
void _crc_lld_calc_halfword(CRCDriver *crcp, uint16_t data) {
  __IO uint16_t *crc16 = (__IO uint16_t*)&(crcp->crc->DR);
  *crc16 = data;
}

/*
 * @brief   Returns calculated CRC from last reset
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] data      data to be added to crc
 *
 * @notapi
 */
void _crc_lld_calc_word(CRCDriver *crcp, uint32_t data) {
  crcp->crc->DR = data;
}


/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   Shared end-of-rx service routine.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
#if CRC_USE_DMA == TRUE
static void crc_lld_serve_interrupt(CRCDriver *crcp, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(STM32_CRC_DMA_ERROR_HOOK)
  if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0) {
    STM32_CRC_DMA_ERROR_HOOK(crcp);
  }
#else
  (void)flags;
#endif

  /* Stop everything.*/
  dmaStreamDisable(crcp->dma);

  /* Portable CRC ISR code defined in the high level driver, note, it is
     a macro.*/
  _crc_isr_code(crcp, crcp->crc->DR ^ crcp->config->final_val);
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level CRC driver initialization.
 *
 * @notapi
 */
void crc_lld_init(void) {
  crcObjectInit(&CRCD1);
  CRCD1.crc    = CRC;
#if CRC_USE_DMA == TRUE
  CRCD1.dma    = STM32_CRC_CRC1_DMA_STREAM;
#endif
}

/**
 * @brief   Configures and activates the CRC peripheral.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @notapi
 */
void crc_lld_start(CRCDriver *crcp) {
  if (crcp->config == NULL)
    crcp->config = &default_config;

  rccEnableCRC(FALSE);

#if STM32_CRC_PROGRAMMABLE == TRUE
  crcp->crc->INIT = crcp->config->initial_val;
  crcp->crc->POL = crcp->config->poly;

  crcp->crc->CR = 0;
  switch(crcp->config->poly_size) {
    case 32:
      break;
    case 16:
      crcp->crc->CR |= CRC_CR_POLYSIZE_0;
      break;
    case 8:
      crcp->crc->CR |= CRC_CR_POLYSIZE_1;
      break;
    case 7:
      crcp->crc->CR |= CRC_CR_POLYSIZE_1 | CRC_CR_POLYSIZE_0;
      break;
    default:
      osalDbgAssert(false, "hardware doesn't support polynomial size");
      break;
  };
  if (crcp->config->reflect_data) {
    crcp->crc->CR |= CRC_CR_REV_IN_1 | CRC_CR_REV_IN_0;
  }
  if (crcp->config->reflect_remainder) {
    crcp->crc->CR |= CRC_CR_REV_OUT;
  }
#else
  osalDbgAssert(crcp->config->initial_val != default_config.initial_val,
      "hardware doesn't support programmable initial value");
  osalDbgAssert(crcp->config->poly_size != default_config.poly_size,
      "hardware doesn't support programmable polynomial size");
  osalDbgAssert(crcp->config->poly != default_config.poly,
      "hardware doesn't support programmable polynomial");
  osalDbgAssert(crcp->config->reflect_data != default_config.reflect_data,
      "hardware doesn't support reflect of input data");
  osalDbgAssert(crcp->config->reflect_remainder != default_config.reflect_remainder,
      "hardware doesn't support reflect of output remainder");
#endif

#if CRC_USE_DMA == TRUE
#if STM32_CRC_PROGRAMMABLE == TRUE
  crcp->dmamode = STM32_DMA_CR_DIR_M2M    | STM32_DMA_CR_PINC |
                  STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE |
                  STM32_DMA_CR_TEIE       | STM32_DMA_CR_TCIE |
                  STM32_DMA_CR_PL(STM32_CRC_CRC1_DMA_PRIORITY);
#else
  crcp->dmamode = STM32_DMA_CR_DIR_M2M    | STM32_DMA_CR_PINC |
                  STM32_DMA_CR_MSIZE_WORD | STM32_DMA_CR_PSIZE_WORD |
                  STM32_DMA_CR_TEIE       | STM32_DMA_CR_TCIE |
                  STM32_DMA_CR_PL(STM32_CRC_CRC1_DMA_PRIORITY);
#endif
  {
    bool b;
    b = dmaStreamAllocate(crcp->dma,
                          STM32_CRC_CRC1_DMA_IRQ_PRIORITY,
                          (stm32_dmaisr_t)crc_lld_serve_interrupt,
                          (void *)crcp);
    osalDbgAssert(!b, "stream already allocated");
  }
#endif
}


/**
 * @brief   Deactivates the CRC peripheral.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @notapi
 */
void crc_lld_stop(CRCDriver *crcp) {
#if CRC_USE_DMA == TRUE
  dmaStreamRelease(crcp->dma);
#else
  (void)crcp;
#endif
  rccDisableCRC(FALSE);
}

/**
 * @brief   Resets current CRC calculation.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @notapi
 */
void crc_lld_reset(CRCDriver *crcp) {
  crcp->crc->CR |= CRC_CR_RESET;
}

/**
 * @brief   Returns calculated CRC from last reset
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] n         size of buf in bytes
 * @param[in] buf       @p buffer location
 *
 * @notapi
 */
uint32_t crc_lld_calc(CRCDriver *crcp, size_t n, const void *buf) {
#if CRC_USE_DMA == TRUE
  crc_lld_start_calc(crcp, n, buf);
  (void) osalThreadSuspendS(&crcp->thread);
#else
  /**
   * BUG: Only peform byte writes to DR reg if reflect_data is disabled.
   * The STM32 hardware unit seems to incorrectly calculate CRCs when all
   * of the following is true: reflect_data(rev_in) is 0, dma is disable, and
   * you are writing more than a byte into the DR register.
   */
  if (crcp->config->reflect_data != 0) {
    while(n > 3) {
      _crc_lld_calc_word(crcp, *(uint32_t*)buf);
      buf+=4;
      n-=4;
    }
  }

#if STM32_CRC_PROGRAMMABLE == TRUE
  /* Programmable CRC units allow variable register width accesses.*/

  /**
   * BUG: Only peform byte writes to DR reg if reflect_data is disabled.
   * The STM32 hardware unit seems to incorrectly calculate CRCs when all
   * of the following is true: reflect_data(rev_in) is 0, dma is disable, and
   * you are writing more than a byte into the DR register.
   */
  if (crcp->config->reflect_data != 0) {
    while(n > 1) {
      _crc_lld_calc_halfword(crcp, *(uint16_t*)buf);
      buf+=2;
      n-=2;
    }
  }

  while(n > 0) {
    _crc_lld_calc_byte(crcp, *(uint8_t*)buf);
    buf++;
    n--;
  }
#else
  osalDbgAssert(n != 0, "STM32 CRC Unit only supports WORD accesses");
#endif

#endif
  return crcp->crc->DR ^ crcp->config->final_val;
}

#if CRC_USE_DMA == TRUE
void crc_lld_start_calc(CRCDriver *crcp, size_t n, const void *buf) {
  dmaStreamSetPeripheral(crcp->dma, buf);
  dmaStreamSetMemory0(crcp->dma, &crcp->crc->DR);
#if STM32_CRC_PROGRAMMABLE == TRUE
  dmaStreamSetTransactionSize(crcp->dma, n);
#else
  dmaStreamSetTransactionSize(crcp->dma, (n / 4));
#endif
  dmaStreamSetMode(crcp->dma, crcp->dmamode);

  dmaStreamEnable(crcp->dma);
}
#endif

#endif /* CRCSW_USE_CRC1 */

#endif /* HAL_USE_CRC */

/** @} */
