/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    mmc_spi.h
 * @brief   MMC over SPI driver header.
 *
 * @addtogroup MMC_SPI
 * @{
 */

#ifndef _MMC_SPI_H_
#define _MMC_SPI_H_

#if (HAL_USE_MMC_SPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define MMC_CMD0_RETRY              10U
#define MMC_CMD1_RETRY              100U
#define MMC_ACMD41_RETRY            100U
#define MMC_WAIT_DATA               10000U

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    MMC_SPI configuration options
 * @{
 */
/**
 * @brief   Delays insertions.
 * @details If enabled this options inserts delays into the MMC waiting
 *          routines releasing some extra CPU time for the threads with
 *          lower priority, this may slow down the driver a bit however.
 *          This option is recommended also if the SPI driver does not
 *          use a DMA channel and heavily loads the CPU.
 */
#if !defined(MMC_NICE_WAITING) || defined(__DOXYGEN__)
#define MMC_NICE_WAITING            TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (HAL_USE_SPI == FALSE) || (SPI_USE_WAIT == FALSE)
#error "MMC_SPI driver requires HAL_USE_SPI and SPI_USE_WAIT"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   MMC/SD over SPI driver configuration structure.
 */
typedef struct {
  /**
   * @brief SPI driver associated to this MMC driver.
   */
  SPIDriver             *spip;
  /**
   * @brief SPI low speed configuration used during initialization.
   */
  const SPIConfig       *lscfg;
  /**
   * @brief SPI high speed configuration used during transfers.
   */
  const SPIConfig       *hscfg;
} MMCConfig;

/**
 * @brief   @p MMCDriver specific methods.
 */
#define _mmc_driver_methods                                                 \
  _mmcsd_block_device_methods

/**
 * @extends MMCSDBlockDeviceVMT
 *
 * @brief   @p MMCDriver virtual methods table.
 */
struct MMCDriverVMT {
  _mmc_driver_methods
};

/**
 * @extends MMCSDBlockDevice
 *
 * @brief   Structure representing a MMC/SD over SPI driver.
 */
typedef struct {
  /**
   * @brief Virtual Methods Table.
   */
  const struct MMCDriverVMT *vmt;
  _mmcsd_block_device_data
  /**
   * @brief Current configuration data.
   */
  const MMCConfig       *config;
  /***
   * @brief Addresses use blocks instead of bytes.
   */
  bool                  block_addresses;
} MMCDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Returns the card insertion status.
 * @note    This macro wraps a low level function named
 *          @p sdc_lld_is_card_inserted(), this function must be
 *          provided by the application because it is not part of the
 *          SDC driver.
 *
 * @param[in] mmcp      pointer to the @p MMCDriver object
 * @return              The card state.
 * @retval FALSE        card not inserted.
 * @retval TRUE         card inserted.
 *
 * @api
 */
#define mmcIsCardInserted(mmcp) mmc_lld_is_card_inserted(mmcp)

/**
 * @brief   Returns the write protect status.
 *
 * @param[in] mmcp      pointer to the @p MMCDriver object
 * @return              The card state.
 * @retval FALSE        card not inserted.
 * @retval TRUE         card inserted.
 *
 * @api
 */
#define mmcIsWriteProtected(mmcp) mmc_lld_is_write_protected(mmcp)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void mmcInit(void);
  void mmcObjectInit(MMCDriver *mmcp);
  void mmcStart(MMCDriver *mmcp, const MMCConfig *config);
  void mmcStop(MMCDriver *mmcp);
  bool mmcConnect(MMCDriver *mmcp);
  bool mmcDisconnect(MMCDriver *mmcp);
  bool mmcStartSequentialRead(MMCDriver *mmcp, uint32_t startblk);
  bool mmcSequentialRead(MMCDriver *mmcp, uint8_t *buffer);
  bool mmcStopSequentialRead(MMCDriver *mmcp);
  bool mmcStartSequentialWrite(MMCDriver *mmcp, uint32_t startblk);
  bool mmcSequentialWrite(MMCDriver *mmcp, const uint8_t *buffer);
  bool mmcStopSequentialWrite(MMCDriver *mmcp);
  bool mmcSync(MMCDriver *mmcp);
  bool mmcGetInfo(MMCDriver *mmcp, BlockDeviceInfo *bdip);
  bool mmcErase(MMCDriver *mmcp, uint32_t startblk, uint32_t endblk);
  bool mmc_lld_is_card_inserted(MMCDriver *mmcp);
  bool mmc_lld_is_write_protected(MMCDriver *mmcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_MMC_SPI == TRUE */

#endif /* _MMC_SPI_H_ */

/** @} */
