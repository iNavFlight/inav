
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
#ifndef FX_STM32_SD_DRIVER_H
#define FX_STM32_SD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"

#include "stm32NNxx_hal.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

extern __IO UINT sd_rx_cplt;
extern __IO UINT sd_tx_cplt;

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */
/* Default timeout used to wait for fx operations */
#define FX_STM32_SD_DEFAULT_TIMEOUT                            (10 * 1000)

/* let the filex low-level driver initialize the SD driver */
#define FX_STM32_SD_INIT                                       0

/* Enable the cache maintenance, needed when using SD DMA
 * and accessing buffers in cacheable area
 */
#define FX_STM32_SD_CACHE_MAINTENANCE                          0

/* Use the SD DMA API */
#define FX_STM32_SD_DMA_API                                    1

/* SDIO instance to be used by FileX */
#define FX_STM32_SD_INSTANCE                                   0

/* Default sector size, used by the driver */
#define FX_STM32_SD_DEFAULT_SECTOR_SIZE                        512

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Define the macro to get the current time in ticks */
/* USER CODE BEGIN FX_STM32_SD_CURRENT_TIME_TX */
#define FX_STM32_SD_CURRENT_TIME()                            HAL_GetTick()

/* USER CODE END FX_STM32_SD_CURRENT_TIME_TX */

/* Macro called before initializing the SD driver
 * e.g. create a semaphore used for transfer notification */

/* USER CODE BEGIN FX_STM32_SD_PRE_INIT */

#define  FX_STM32_SD_PRE_INIT(_media_ptr)

 /* USER CODE END FX_STM32_SD_PRE_INIT*/

/* Macro called after initializing the SD driver */
/* USER CODE BEGIN FX_STM32_SD_POST_INIT */

#define FX_STM32_SD_POST_INIT(_media_ptr)

/* USER CODE END FX_STM32_SD_POST_INIT */

/* Macro called after the SD deinit */

/* Macro called after the SD deinit */
/* USER CODE BEGIN FX_STM32_SD_POST_DEINIT */

/* Macro called after the SD deinit */
#define FX_STM32_SD_POST_DEINIT(_media_ptr)

/* USER CODE END FX_STM32_SD_POST_DEINIT */

/* Macro called after the abort request */
/* USER CODE BEGIN FX_STM32_SD_POST_ABORT */

#define FX_STM32_SD_POST_ABORT(_media_ptr)

/* USER CODE END FX_STM32_SD_POST_ABORT */

/* Macro called before performing read operation */
/* USER CODE BEGIN FX_STM32_SD_PRE_READ_TRANSFER */

/* Macro called before performing read operation */
#define FX_STM32_SD_PRE_READ_TRANSFER(_media_ptr)

/* USER CODE END FX_STM32_SD_PRE_READ_TRANSFER */

/* Macro called after performing read operation */
/* USER CODE BEGIN FX_STM32_SD_POST_READ_TRANSFER */

/* Macro called after performing read operation */
#define FX_STM32_SD_POST_READ_TRANSFER(_media_ptr)

/* USER CODE END FX_STM32_SD_POST_READ_TRANSFER */

/* Macro for read error handling */
/* USER CODE BEGIN FX_STM32_SD_READ_TRANSFER_ERROR */

/* Macro for read error handling */
#define FX_STM32_SD_READ_TRANSFER_ERROR(_status_)

/* USER CODE END FX_STM32_SD_READ_TRANSFER_ERROR */

/* Define how to notify about Read completion operation */
/* USER CODE BEGIN FX_STM32_SD_READ_CPLT_NOTIFY */

/* Define how to notify about Read completion operation */
#define FX_STM32_SD_READ_CPLT_NOTIFY()

/* USER CODE END FX_STM32_SD_READ_CPLT_NOTIFY */

/* Define how to notify about write completion operation */
/* USER CODE BEGIN FX_STM32_SD_WRITE_CPLT_NOTIFY */

/* Define how to notify about write completion operation */
#define FX_STM32_SD_WRITE_CPLT_NOTIFY()

/* USER CODE END FX_STM32_SD_WRITE_CPLT_NOTIFY */

/* Macro called before performing write operation */
/* USER CODE BEGIN FX_STM32_SD_PRE_WRITE_TRANSFER */

/* Macro called before performing write operation */
#define FX_STM32_SD_PRE_WRITE_TRANSFER(__media_ptr__)

/* USER CODE END FX_STM32_SD_PRE_WRITE_TRANSFER */

/* Macro called after performing write operation */
/* USER CODE BEGIN FX_STM32_SD_POST_WRITE_TRANSFER */

#define FX_STM32_SD_POST_WRITE_TRANSFER(_media_ptr)

/* USER CODE END FX_STM32_SD_POST_WRITE_TRANSFER */

/* Macro for write error handling */
/* USER CODE BEGIN FX_STM32_SD_WRITE_TRANSFER_ERROR */

/* Macro for write error handling */
#define FX_STM32_SD_WRITE_TRANSFER_ERROR(__status__)

/* USER CODE END FX_STM32_SD_WRITE_TRANFSER_ERROR */

/* Exported functions prototypes ---------------------------------------------*/

INT fx_stm32_sd_init(UINT instance);
INT fx_stm32_sd_deinit(UINT instance);

INT fx_stm32_sd_get_status(UINT instance);

INT fx_stm32_sd_read_blocks(UINT instance, UINT *buffer, UINT start_block, UINT total_blocks);
INT fx_stm32_sd_write_blocks(UINT instance, UINT *buffer, UINT start_block, UINT total_blocks);

VOID fx_stm32_sd_driver(FX_MEDIA *media_ptr);

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

#endif /* FX_STM32_SD_DRIVER_H */
