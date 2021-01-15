/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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
 * @file    nand.c
 * @brief   NAND Driver code.
 *
 * @addtogroup NAND
 * @{
 */

#include "hal.h"

#if (HAL_USE_NAND == TRUE) || defined(__DOXYGEN__)

#include "string.h" /* for memset */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Check page size.
 *
 * @param[in] page_data_size      size of page data area
 *
 * @notapi
 */
static void pagesize_check(size_t page_data_size) {

  /* Page size out of bounds.*/
  osalDbgCheck((page_data_size >= NAND_MIN_PAGE_SIZE) &&
      (page_data_size <= NAND_MAX_PAGE_SIZE));

  /* Page size must be power of 2.*/
  osalDbgCheck(((page_data_size - 1) & page_data_size) == 0);
}

/**
 * @brief   Translate block-page-offset scheme to NAND internal address.
 *
 * @param[in] cfg         pointer to the @p NANDConfig from
 *                        corresponding NAND driver
 * @param[in] block       block number
 * @param[in] page        page number related to begin of block
 * @param[in] page_offset data offset related to begin of page
 * @param[out] addr       buffer to store calculated address
 * @param[in] addr_len    length of address buffer
 *
 * @notapi
 */
static void calc_addr(const NANDConfig *cfg, uint32_t block, uint32_t page,
                      uint32_t page_offset, uint8_t *addr, size_t addr_len) {
  size_t i;
  uint32_t row;

  osalDbgCheck(cfg->rowcycles + cfg->colcycles == addr_len);
  osalDbgCheck((block < cfg->blocks) && (page < cfg->pages_per_block) &&
             (page_offset < cfg->page_data_size + cfg->page_spare_size));

  row = (block * cfg->pages_per_block) + page;
  for (i=0; i<cfg->colcycles; i++){
    addr[i] = page_offset & 0xFF;
    page_offset = page_offset >> 8;
  }
  for (; i<addr_len; i++){
    addr[i] = row & 0xFF;
    row = row >> 8;
  }
}

/**
 * @brief   Translate block number to NAND internal address.
 * @note    This function designed for erasing purpose.
 *
 * @param[in] cfg       pointer to the @p NANDConfig from
 *                      corresponding NAND driver
 * @param[in] block     block number
 * @param[out] addr     buffer to store calculated address
 * @param[in] addr_len  length of address buffer
 *
 * @notapi
 */
static void calc_blk_addr(const NANDConfig *cfg, uint32_t block,
                          uint8_t *addr, size_t addr_len) {
  size_t i;
  uint32_t row;

  osalDbgCheck(cfg->rowcycles == addr_len); /* Incorrect buffer length  */
  osalDbgCheck(block < cfg->blocks);        /* Overflow                 */

  row = block * cfg->pages_per_block;
  for (i=0; i<addr_len; i++) {
    addr[i] = row & 0xFF;
    row = row >> 8;
  }
}

/**
 * @brief   Read block badness mark directly from NAND memory array.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 *
 * @return                  block condition
 * @retval true             if the block is bad.
 * @retval false            if the block is good.
 *
 * @notapi
 */
static bool read_is_block_bad(NANDDriver *nandp, size_t block) {

  uint16_t badmark0 = nandReadBadMark(nandp, block, 0);
  uint16_t badmark1 = nandReadBadMark(nandp, block, 1);

  if ((0xFFFF != badmark0) || (0xFFFF != badmark1))
    return true;
  else
    return false;
}

/**
 * @brief   Scan for bad blocks and fill map with their numbers.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 *
 * @notapi
 */
static void scan_bad_blocks(NANDDriver *nandp) {

  const size_t blocks = nandp->config->blocks;
  size_t b;

  osalDbgCheck(bitmapGetBitsCount(nandp->bb_map) >= blocks);

  /* clear map just to be safe */
  bitmapObjectInit(nandp->bb_map, 0);

  /* now write numbers of bad block to map */
  for (b=0; b<blocks; b++) {
    if (read_is_block_bad(nandp, b)) {
      bitmapSet(nandp->bb_map, b);
    }
  }
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   NAND Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void nandInit(void) {

  nand_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p NANDDriver structure.
 *
 * @param[out] nandp        pointer to the @p NANDDriver object
 *
 * @init
 */
void nandObjectInit(NANDDriver *nandp) {

#if NAND_USE_MUTUAL_EXCLUSION
#if CH_CFG_USE_MUTEXES
  chMtxObjectInit(&nandp->mutex);
#else
  chSemObjectInit(&nandp->semaphore, 1);
#endif /* CH_CFG_USE_MUTEXES */
#endif /* NAND_USE_MUTUAL_EXCLUSION */

  nandp->state  = NAND_STOP;
  nandp->config = NULL;
}

/**
 * @brief   Configures and activates the NAND peripheral.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] config        pointer to the @p NANDConfig object
 * @param[in] bb_map        pointer to the bad block map or @NULL if not need
 *
 * @api
 */
void nandStart(NANDDriver *nandp, const NANDConfig *config, bitmap_t *bb_map) {

  osalDbgCheck((nandp != NULL) && (config != NULL));
  osalDbgAssert((nandp->state == NAND_STOP) ||
      (nandp->state == NAND_READY),
      "invalid state");

  nandp->config = config;
  pagesize_check(nandp->config->page_data_size);
  nand_lld_start(nandp);
  nandp->state = NAND_READY;
  nand_lld_reset(nandp);

  if (NULL != bb_map) {
    nandp->bb_map = bb_map;
    scan_bad_blocks(nandp);
  }
}

/**
 * @brief   Deactivates the NAND peripheral.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 *
 * @api
 */
void nandStop(NANDDriver *nandp) {

  osalDbgCheck(nandp != NULL);
  osalDbgAssert((nandp->state == NAND_STOP) ||
      (nandp->state == NAND_READY),
      "invalid state");
  nand_lld_stop(nandp);
  nandp->state = NAND_STOP;
}

/**
 * @brief   Read whole page.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 * @param[in] page          page number related to begin of block
 * @param[out] data         buffer to store data, half word aligned
 * @param[in] datalen       length of data buffer in bytes, half word aligned
 *
 * @api
 */
void nandReadPageWhole(NANDDriver *nandp, uint32_t block, uint32_t page,
                       void *data, size_t datalen) {

  const NANDConfig *cfg = nandp->config;
  const size_t addrlen = cfg->rowcycles + cfg->colcycles;
  uint8_t addr[addrlen];

  osalDbgCheck((nandp != NULL) && (data != NULL));
  osalDbgCheck((datalen <= (cfg->page_data_size + cfg->page_spare_size)));
  osalDbgAssert(nandp->state == NAND_READY, "invalid state");

  calc_addr(cfg, block, page, 0, addr, addrlen);
  nand_lld_read_data(nandp, data, datalen, addr, addrlen, NULL);
}

/**
 * @brief   Write whole page.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 * @param[in] page          page number related to begin of block
 * @param[in] data          buffer with data to be written, half word aligned
 * @param[in] datalen       length of data buffer in bytes, half word aligned
 *
 * @return    The operation status reported by NAND IC (0x70 command).
 *
 * @api
 */
uint8_t nandWritePageWhole(NANDDriver *nandp, uint32_t block, uint32_t page,
                           const void *data, size_t datalen) {

  uint8_t retval;
  const NANDConfig *cfg = nandp->config;
  const size_t addrlen = cfg->rowcycles + cfg->colcycles;
  uint8_t addr[addrlen];

  osalDbgCheck((nandp != NULL) && (data != NULL));
  osalDbgCheck((datalen <= (cfg->page_data_size + cfg->page_spare_size)));
  osalDbgAssert(nandp->state == NAND_READY, "invalid state");

  calc_addr(cfg, block, page, 0, addr, addrlen);
  retval = nand_lld_write_data(nandp, data, datalen, addr, addrlen, NULL);
  return retval;
}

/**
 * @brief   Read page data without spare area.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 * @param[in] page          page number related to begin of block
 * @param[out] data         buffer to store data, half word aligned
 * @param[in] datalen       length of data buffer in bytes, half word aligned
 * @param[out] ecc          pointer to calculated ECC. Ignored when NULL.
 *
 * @api
 */
void nandReadPageData(NANDDriver *nandp, uint32_t block, uint32_t page,
                         void *data, size_t datalen, uint32_t *ecc) {

  const NANDConfig *cfg = nandp->config;
  const size_t addrlen = cfg->rowcycles + cfg->colcycles;
  uint8_t addr[addrlen];

  osalDbgCheck((nandp != NULL) && (data != NULL));
  osalDbgCheck((datalen <= cfg->page_data_size));
  osalDbgAssert(nandp->state == NAND_READY, "invalid state");

  calc_addr(cfg, block, page, 0, addr, addrlen);
  nand_lld_read_data(nandp, data, datalen, addr, addrlen, ecc);
}

/**
 * @brief   Write page data without spare area.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 * @param[in] page          page number related to begin of block
 * @param[in] data          buffer with data to be written, half word aligned
 * @param[in] datalen       length of data buffer in bytes, half word aligned
 * @param[out] ecc          pointer to calculated ECC. Ignored when NULL.
 *
 * @return    The operation status reported by NAND IC (0x70 command).
 *
 * @api
 */
uint8_t nandWritePageData(NANDDriver *nandp, uint32_t block, uint32_t page,
                          const void *data, size_t datalen, uint32_t *ecc) {

  uint8_t retval;
  const NANDConfig *cfg = nandp->config;
  const size_t addrlen = cfg->rowcycles + cfg->colcycles;
  uint8_t addr[addrlen];

  osalDbgCheck((nandp != NULL) && (data != NULL));
  osalDbgCheck((datalen <= cfg->page_data_size));
  osalDbgAssert(nandp->state == NAND_READY, "invalid state");

  calc_addr(cfg, block, page, 0, addr, addrlen);
  retval = nand_lld_write_data(nandp, data, datalen, addr, addrlen, ecc);
  return retval;
}

/**
 * @brief   Read page spare area.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 * @param[in] page          page number related to begin of block
 * @param[out] spare        buffer to store data, half word aligned
 * @param[in] sparelen      length of data buffer in bytes, half word aligned
 *
 * @api
 */
void nandReadPageSpare(NANDDriver *nandp, uint32_t block, uint32_t page,
                       void *spare, size_t sparelen) {

  const NANDConfig *cfg = nandp->config;
  const size_t addrlen = cfg->rowcycles + cfg->colcycles;
  uint8_t addr[addrlen];

  osalDbgCheck((NULL != spare) && (nandp != NULL));
  osalDbgCheck(sparelen <= cfg->page_spare_size);
  osalDbgAssert(nandp->state == NAND_READY, "invalid state");

  calc_addr(cfg, block, page, cfg->page_data_size, addr, addrlen);
  nand_lld_read_data(nandp, spare, sparelen, addr, addrlen, NULL);
}

/**
 * @brief   Write page spare area.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 * @param[in] page          page number related to begin of block
 * @param[in] spare         buffer with spare data to be written, half word aligned
 * @param[in] sparelen      length of data buffer in bytes, half word aligned
 *
 * @return    The operation status reported by NAND IC (0x70 command).
 *
 * @api
 */
uint8_t nandWritePageSpare(NANDDriver *nandp, uint32_t block, uint32_t page,
                           const void *spare, size_t sparelen) {

  const NANDConfig *cfg = nandp->config;
  const size_t addrlen = cfg->rowcycles + cfg->colcycles;
  uint8_t addr[addrlen];

  osalDbgCheck((NULL != spare) && (nandp != NULL));
  osalDbgCheck(sparelen <= cfg->page_spare_size);
  osalDbgAssert(nandp->state == NAND_READY, "invalid state");

  calc_addr(cfg, block, page, cfg->page_data_size, addr, addrlen);
  return nand_lld_write_data(nandp, spare, sparelen, addr, addrlen, NULL);
}

/**
 * @brief   Mark block as bad.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 *
 * @api
 */
void nandMarkBad(NANDDriver *nandp, uint32_t block) {

  uint16_t bb_mark = 0;

  nandWritePageSpare(nandp, block, 0, &bb_mark, sizeof(bb_mark));
  nandWritePageSpare(nandp, block, 1, &bb_mark, sizeof(bb_mark));

  if (NULL != nandp->bb_map)
    bitmapSet(nandp->bb_map, block);
}

/**
 * @brief   Read bad mark out.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 * @param[in] page          page number related to begin of block
 *
 * @return                  Bad mark.
 *
 * @api
 */
uint16_t nandReadBadMark(NANDDriver *nandp, uint32_t block, uint32_t page) {
  uint16_t bb_mark;

  nandReadPageSpare(nandp, block, page, &bb_mark, sizeof(bb_mark));
  return bb_mark;
}

/**
 * @brief   Erase block.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 *
 * @return    The operation status reported by NAND IC (0x70 command).
 *
 * @api
 */
uint8_t nandErase(NANDDriver *nandp, uint32_t block) {

  const NANDConfig *cfg = nandp->config;
  const size_t addrlen = cfg->rowcycles;
  uint8_t addr[addrlen];

  osalDbgCheck(nandp != NULL);
  osalDbgAssert(nandp->state == NAND_READY, "invalid state");

  calc_blk_addr(cfg, block, addr, addrlen);
  return nand_lld_erase(nandp, addr, addrlen);
}

/**
 * @brief   Check block badness.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 * @param[in] block         block number
 *
 * @return                  block condition
 * @retval true             if the block is bad.
 * @retval false            if the block is good.
 *
 * @api
 */
bool nandIsBad(NANDDriver *nandp, uint32_t block) {

  osalDbgCheck(nandp != NULL);
  osalDbgAssert(nandp->state == NAND_READY, "invalid state");

  if (NULL != nandp->bb_map)
    return 1 == bitmapGet(nandp->bb_map, block);
  else
    return read_is_block_bad(nandp, block);
}

#if NAND_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the NAND bus.
 * @details This function tries to gain ownership to the NAND bus, if the bus
 *          is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p NAND_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 *
 * @api
 */
void nandAcquireBus(NANDDriver *nandp) {

  osalDbgCheck(nandp != NULL);

#if CH_CFG_USE_MUTEXES
  chMtxLock(&nandp->mutex);
#elif CH_CFG_USE_SEMAPHORES
  chSemWait(&nandp->semaphore);
#endif
}

/**
 * @brief   Releases exclusive access to the NAND bus.
 * @pre     In order to use this function the option
 *          @p NAND_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] nandp         pointer to the @p NANDDriver object
 *
 * @api
 */
void nandReleaseBus(NANDDriver *nandp) {

  osalDbgCheck(nandp != NULL);

#if CH_CFG_USE_MUTEXES
  chMtxUnlock(&nandp->mutex);
#elif CH_CFG_USE_SEMAPHORES
  chSemSignal(&nandp->semaphore);
#endif
}
#endif /* NAND_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_NAND */

/** @} */




