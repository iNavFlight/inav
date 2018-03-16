/*
    ChibiOS/HAL - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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
 * @file    nand_lld.c
 * @brief   NAND Driver subsystem low level driver source.
 *
 * @addtogroup NAND
 * @{
 */

#include "hal.h"

#if (HAL_USE_NAND == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define NAND_DMA_CHANNEL                                                  \
  STM32_DMA_GETCHANNEL(STM32_NAND_DMA_STREAM,                             \
                       STM32_FSMC_DMA_CHN)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   NAND1 driver identifier.
 */
#if STM32_NAND_USE_FSMC_NAND1 || defined(__DOXYGEN__)
NANDDriver NANDD1;
#endif

/**
 * @brief   NAND2 driver identifier.
 */
#if STM32_NAND_USE_FSMC_NAND2 || defined(__DOXYGEN__)
NANDDriver NANDD2;
#endif

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   Wakes up the waiting thread.
 *
 * @param[in] nandp  pointer to the @p NANDDriver object
 * @param[in] msg       wakeup message
 *
 * @notapi
 */
static void wakeup_isr(NANDDriver *nandp) {

  osalDbgCheck(nandp->thread != NULL);
  osalThreadResumeI(&nandp->thread, MSG_OK);
}

/**
 * @brief   Put calling thread in suspend and switch driver state
 *
 * @param[in] nandp    pointer to the @p NANDDriver object
 */
static void nand_lld_suspend_thread(NANDDriver *nandp) {

  osalThreadSuspendS(&nandp->thread);
}

/**
 * @brief   Caclulate ECCPS register value
 *
 * @param[in] nandp    pointer to the @p NANDDriver object
 */
static uint32_t calc_eccps(NANDDriver *nandp) {

  uint32_t i = 0;
  uint32_t eccps = nandp->config->page_data_size;

  eccps = eccps >> 9;
  while (eccps > 0){
    i++;
    eccps >>= 1;
  }

  return i << 17;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   Enable interrupts from NAND
 *
 * @param[in] nandp    pointer to the @p NANDDriver object
 *
 * @notapi
 */
static void nand_ready_isr_enable(NANDDriver *nandp) {
#if STM32_NAND_USE_EXT_INT
  nandp->config->ext_nand_isr_enable();
#else
  nandp->nand->SR &= ~(FSMC_SR_IRS | FSMC_SR_ILS | FSMC_SR_IFS |
                                          FSMC_SR_ILEN | FSMC_SR_IFEN);
  nandp->nand->SR |= FSMC_SR_IREN;
#endif
}

/**
 * @brief   Disable interrupts from NAND
 *
 * @param[in] nandp    pointer to the @p NANDDriver object
 *
 * @notapi
 */
static void nand_ready_isr_disable(NANDDriver *nandp) {
#if STM32_NAND_USE_EXT_INT
  nandp->config->ext_nand_isr_disable();
#else
  nandp->nand->SR &= ~FSMC_SR_IREN;
#endif
}

/**
 * @brief   Ready interrupt handler
 *
 * @param[in] nandp    pointer to the @p NANDDriver object
 *
 * @notapi
 */
static void nand_isr_handler (NANDDriver *nandp) {

  osalSysLockFromISR();

#if !STM32_NAND_USE_EXT_INT
  osalDbgCheck(nandp->nand->SR & FSMC_SR_IRS); /* spurious interrupt happened */
  nandp->nand->SR &= ~FSMC_SR_IRS;
#endif

  switch (nandp->state){
  case NAND_READ:
    nandp->state = NAND_DMA_RX;
    dmaStartMemCopy(nandp->dma, nandp->dmamode,
                    nandp->map_data, nandp->rxdata, nandp->datalen);
    /* thread will be waked up from DMA ISR */
    break;

  case NAND_ERASE:
    /* NAND reports about erase finish */
    nandp->state = NAND_READY;
    wakeup_isr(nandp);
    break;

  case NAND_PROGRAM:
    /* NAND reports about page programming finish */
    nandp->state = NAND_READY;
    wakeup_isr(nandp);
    break;

  default:
    osalSysHalt("Unhandled case");
    break;
  }
  osalSysUnlockFromISR();
}

/**
 * @brief   DMA RX end IRQ handler.
 *
 * @param[in] nandp    pointer to the @p NANDDriver object
 * @param[in] flags    pre-shifted content of the ISR register
 *
 * @notapi
 */
static void nand_lld_serve_transfer_end_irq(NANDDriver *nandp, uint32_t flags) {
  /* DMA errors handling.*/
#if defined(STM32_NAND_DMA_ERROR_HOOK)
  if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0) {
    STM32_NAND_DMA_ERROR_HOOK(nandp);
  }
#else
  (void)flags;
#endif

  osalSysLockFromISR();

  dmaStreamDisable(nandp->dma);

  switch (nandp->state){
  case NAND_DMA_TX:
    nandp->state = NAND_PROGRAM;
    nandp->map_cmd[0] = NAND_CMD_PAGEPROG;
    /* thread will be woken from ready_isr() */
    break;

  case NAND_DMA_RX:
    nandp->state = NAND_READY;
    nandp->rxdata = NULL;
    nandp->datalen = 0;
    wakeup_isr(nandp);
    break;

  default:
    osalSysHalt("Unhandled case");
    break;
  }

  osalSysUnlockFromISR();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level NAND driver initialization.
 *
 * @notapi
 */
void nand_lld_init(void) {

  fsmc_init();

#if STM32_NAND_USE_FSMC_NAND1
  /* Driver initialization.*/
  nandObjectInit(&NANDD1);
  NANDD1.rxdata   = NULL;
  NANDD1.datalen  = 0;
  NANDD1.thread   = NULL;
  NANDD1.dma      = STM32_DMA_STREAM(STM32_NAND_DMA_STREAM);
  NANDD1.nand     = FSMCD1.nand1;
  NANDD1.map_data = (uint8_t*)FSMC_Bank2_MAP_COMMON_DATA;
  NANDD1.map_cmd  = (uint8_t*)FSMC_Bank2_MAP_COMMON_CMD;
  NANDD1.map_addr = (uint8_t*)FSMC_Bank2_MAP_COMMON_ADDR;
  NANDD1.bb_map   = NULL;
#endif /* STM32_NAND_USE_FSMC_NAND1 */

#if STM32_NAND_USE_FSMC_NAND2
  /* Driver initialization.*/
  nandObjectInit(&NANDD2);
  NANDD2.rxdata   = NULL;
  NANDD2.datalen  = 0;
  NANDD2.thread   = NULL;
  NANDD2.dma      = STM32_DMA_STREAM(STM32_NAND_DMA_STREAM);
  NANDD2.nand     = FSMCD1.nand2;
  NANDD2.map_data = (uint8_t*)FSMC_Bank3_MAP_COMMON_DATA;
  NANDD2.map_cmd  = (uint8_t*)FSMC_Bank3_MAP_COMMON_CMD;
  NANDD2.map_addr = (uint8_t*)FSMC_Bank3_MAP_COMMON_ADDR;
  NANDD2.bb_map   = NULL;
#endif /* STM32_NAND_USE_FSMC_NAND2 */
}

/**
 * @brief   Configures and activates the NAND peripheral.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 *
 * @notapi
 */
void nand_lld_start(NANDDriver *nandp) {

  bool b;

  if (FSMCD1.state == FSMC_STOP)
    fsmc_start(&FSMCD1);

  if (nandp->state == NAND_STOP) {
    b = dmaStreamAllocate(nandp->dma,
                          STM32_EMC_FSMC1_IRQ_PRIORITY,
                          (stm32_dmaisr_t)nand_lld_serve_transfer_end_irq,
                          (void *)nandp);
    osalDbgAssert(!b, "stream already allocated");
    nandp->dmamode = STM32_DMA_CR_CHSEL(NAND_DMA_CHANNEL) |
                     STM32_DMA_CR_PL(STM32_NAND_NAND1_DMA_PRIORITY) |
                     STM32_DMA_CR_PSIZE_BYTE |
                     STM32_DMA_CR_MSIZE_BYTE |
                     STM32_DMA_CR_DMEIE |
                     STM32_DMA_CR_TEIE |
                     STM32_DMA_CR_TCIE;
    /* dmaStreamSetFIFO(nandp->dma,
                    STM32_DMA_FCR_DMDIS | NAND_STM32_DMA_FCR_FTH_LVL); */
    nandp->nand->PCR = calc_eccps(nandp) | FSMC_PCR_PTYP | FSMC_PCR_PBKEN;
    nandp->nand->PMEM = nandp->config->pmem;
    nandp->nand->PATT = nandp->config->pmem;
    nandp->isr_handler = nand_isr_handler;
    nand_ready_isr_enable(nandp);
  }
}

/**
 * @brief   Deactivates the NAND peripheral.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 *
 * @notapi
 */
void nand_lld_stop(NANDDriver *nandp) {

  if (nandp->state == NAND_READY) {
    dmaStreamRelease(nandp->dma);
    nandp->nand->PCR &= ~FSMC_PCR_PBKEN;
    nand_ready_isr_disable(nandp);
    nandp->isr_handler = NULL;
  }
}

/**
 * @brief   Read data from NAND.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[out] data         pointer to data buffer
 * @param[in] datalen       size of data buffer
 * @param[in] addr          pointer to address buffer
 * @param[in] addrlen       length of address
 * @param[out] ecc          pointer to store computed ECC. Ignored when NULL.
 *
 * @notapi
 */
void nand_lld_read_data(NANDDriver *nandp, uint8_t *data, size_t datalen,
                        uint8_t *addr, size_t addrlen, uint32_t *ecc){

  nandp->state = NAND_READ;
  nandp->rxdata = data;
  nandp->datalen = datalen;

  nand_lld_write_cmd (nandp, NAND_CMD_READ0);
  nand_lld_write_addr(nandp, addr, addrlen);
  osalSysLock();
  nand_lld_write_cmd (nandp, NAND_CMD_READ0_CONFIRM);

  /* Here NAND asserts busy signal and starts transferring from memory
     array to page buffer. After the end of transmission ready_isr functions
     starts DMA transfer from page buffer to MCU's RAM.*/
  osalDbgAssert((nandp->nand->PCR & FSMC_PCR_ECCEN) == 0,
          "State machine broken. ECCEN must be previously disabled.");

  if (NULL != ecc){
    nandp->nand->PCR |= FSMC_PCR_ECCEN;
  }

  nand_lld_suspend_thread(nandp);
  osalSysUnlock();

  /* thread was woken up from DMA ISR */
  if (NULL != ecc){
    while (! (nandp->nand->SR & FSMC_SR_FEMPT))
      ;
    *ecc = nandp->nand->ECCR;
    nandp->nand->PCR &= ~FSMC_PCR_ECCEN;
  }
}

/**
 * @brief   Write data to NAND.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] data          buffer with data to be written
 * @param[in] datalen       size of data buffer
 * @param[in] addr          pointer to address buffer
 * @param[in] addrlen       length of address
 * @param[out] ecc          pointer to store computed ECC. Ignored when NULL.
 *
 * @return    The operation status reported by NAND IC (0x70 command).
 *
 * @notapi
 */
uint8_t nand_lld_write_data(NANDDriver *nandp, const uint8_t *data,
                size_t datalen, uint8_t *addr, size_t addrlen, uint32_t *ecc) {

  nandp->state = NAND_WRITE;

  nand_lld_write_cmd (nandp, NAND_CMD_WRITE);
  osalSysLock();
  nand_lld_write_addr(nandp, addr, addrlen);

  /* Now start DMA transfer to NAND buffer and put thread in sleep state.
     Tread will be woken up from ready ISR. */
  nandp->state = NAND_DMA_TX;
  osalDbgAssert((nandp->nand->PCR & FSMC_PCR_ECCEN) == 0,
          "State machine broken. ECCEN must be previously disabled.");

  if (NULL != ecc){
    nandp->nand->PCR |= FSMC_PCR_ECCEN;
  }

  dmaStartMemCopy(nandp->dma, nandp->dmamode, data, nandp->map_data, datalen);

  nand_lld_suspend_thread(nandp);
  osalSysUnlock();

  if (NULL != ecc){
    while (! (nandp->nand->SR & FSMC_SR_FEMPT))
      ;
    *ecc = nandp->nand->ECCR;
    nandp->nand->PCR &= ~FSMC_PCR_ECCEN;
  }

  return nand_lld_read_status(nandp);
}

/**
 * @brief   Erase block.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] addr          pointer to address buffer
 * @param[in] addrlen       length of address
 *
 * @return    The operation status reported by NAND IC (0x70 command).
 *
 * @notapi
 */
uint8_t nand_lld_erase(NANDDriver *nandp, uint8_t *addr, size_t addrlen) {

  nandp->state = NAND_ERASE;

  nand_lld_write_cmd (nandp, NAND_CMD_ERASE);
  nand_lld_write_addr(nandp, addr, addrlen);
  osalSysLock();
  nand_lld_write_cmd (nandp, NAND_CMD_ERASE_CONFIRM);
  nand_lld_suspend_thread(nandp);
  osalSysUnlock();

  return nand_lld_read_status(nandp);
}

/**
 * @brief     Read data from NAND using polling approach.
 *
 * @detatils  Use this function to read data when no waiting expected. For
 *            Example read status word after 0x70 command
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[out] data         pointer to output buffer
 * @param[in] len           length of data to be read
 *
 * @notapi
 */
void nand_lld_polled_read_data(NANDDriver *nandp, uint8_t *data, size_t len) {
  size_t i = 0;

  for (i=0; i<len; i++)
    data[i] = nandp->map_data[i];
}

/**
 * @brief   Send addres to NAND.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] len           length of address array
 * @param[in] addr          pointer to address array
 *
 * @notapi
 */
void nand_lld_write_addr(NANDDriver *nandp, const uint8_t *addr, size_t len) {
  size_t i = 0;

  for (i=0; i<len; i++)
    nandp->map_addr[i] = addr[i];
}

/**
 * @brief   Send command to NAND.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] cmd           command value
 *
 * @notapi
 */
void nand_lld_write_cmd(NANDDriver *nandp, uint8_t cmd) {
  nandp->map_cmd[0] = cmd;
}

/**
 * @brief   Read status byte from NAND.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 *
 * @return    Status byte.
 *
 * @notapi
 */
uint8_t nand_lld_read_status(NANDDriver *nandp) {

  uint8_t status[1] = {0x01}; /* presume worse */

  nand_lld_write_cmd(nandp, NAND_CMD_STATUS);
  nand_lld_polled_read_data(nandp, status, 1);

  return status[0];
}

#endif /* HAL_USE_NAND */

/** @} */

