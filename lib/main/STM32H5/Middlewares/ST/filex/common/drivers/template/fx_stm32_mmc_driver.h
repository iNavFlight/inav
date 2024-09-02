
/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FX_STM32_MMC_DRIVER_H
#define FX_STM32_MMC_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"

#include "stm32NNxx_hal.h"
/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/* Default timeout used to wait for fx operations */
#define FX_STM32_MMC_DEFAULT_TIMEOUT                                  (10 * 1000)

/* Default MMC sector size typically 512 for uMMC */
#define FX_STM32_MMC_DEFAULT_SECTOR_SIZE                              512

/* let the filex low-level driver initialize the MMC driver */
#define FX_STM32_MMC_INIT                                             0

/* Use the MMC DMA API, when enabled cache maintenance
 * may be required
 */
#define FX_STM32_MMC_DMA_API                                          0

/* Enable the cache maintenance, needed when using SD DMA
 * and accessing buffers in cacheable area.
 */
#define FX_STM32_SD_CACHE_MAINTENANCE                                 0

/* MMC instance default to 0 */
#define FX_STM32_MMC_INSTANCE                                         0

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Get the current time in ticks */

#define FX_STM32_MMC_CURRENT_TIME

/* Macro called before initializing the MMC driver
 * for example to create a semaphore used for
 * transfer notification
 */

#define  FX_STM32_MMC_PRE_INIT(_media_ptr)

/* Macro called after initializing the MMC driver */

#define FX_STM32_MMC_POST_INIT(_media_ptr)


/* Macro called after the MMC deinit */
#define FX_STM32_MMC_POST_DEINIT(_media_ptr)

/* Macro called after the abort request */
#define FX_STM32_MMC_POST_ABORT(_media_ptr)

/* Macro called before performing read operation */
#define FX_STM32_MMC_PRE_READ_TRANSFER(_media_ptr)

/* Macro called after performing read operation */
#define FX_STM32_MMC_POST_READ_TRANSFER(_media_ptr)

/* Macro for read error handling */
#define FX_STM32_MMC_READ_TRANSFER_ERROR(_status_)

/* Define how to notify about Read completion operation */
#define FX_STM32_MMC_READ_CPLT_NOTIFY()

/* Define how to notify about write completion operation */
#define FX_STM32_MMC_WRITE_CPLT_NOTIFY()

/* Macro called before performing write operation */
#define FX_STM32_MMC_PRE_WRITE_TRANSFER(__media_ptr__)

/* Macro called after performing write operation */
#define FX_STM32_MMC_POST_WRITE_TRANSFER(__media_ptr__)

/* Macro for write error handling */
#define FX_STM32_MMC_WRITE_TRANSFER_ERROR(__status__)

/* Exported functions prototypes ---------------------------------------------*/

INT fx_stm32_mmc_init(UINT Instance);
INT fx_stm32_mmc_deinit(UINT Instance);

INT fx_stm32_mmc_get_status(UINT Instance);

INT fx_stm32_mmc_read_blocks(UINT Instance, UINT *Buffer, UINT StartSector, UINT NbrOfBlocks);
INT fx_stm32_mmc_write_blocks(UINT Instance, UINT *Buffer, UINT StartSector, UINT NbrOfBlocks);

VOID  fx_stm32_mmc_driver(FX_MEDIA *media_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif

#endif /* FX_STM32_MMC_DRIVER_H */

