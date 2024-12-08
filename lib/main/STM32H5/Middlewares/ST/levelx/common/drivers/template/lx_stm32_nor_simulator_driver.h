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

#ifndef LX_STM32_NOR_SIMULATOR_DRIVER_H
#define LX_STM32_NOR_SIMULATOR_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "lx_api.h"
#include "stm32xxxx.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* define the NOR Flash base address */
#define LX_NOR_SIMULATOR_FLASH_BASE_ADDRESS  D1_AXISRAM_BASE

/* define the size of the NOR flash*/
#define LX_NOR_SIMULATOR_FLASH_SIZE          (1024 * 256)

/* define the size of the NOR SECTOR size */
#define LX_NOR_SIMULATOR_SECTOR_SIZE         512

/* define the number of sectors per block */
#define LX_NOR_SIMULATOR_SECTORS_PER_BLOCK   16

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

UINT  lx_stm32_nor_simulator_initialize(LX_NOR_FLASH *nor_flash);

#ifdef __cplusplus
}
#endif
#endif /* LX_STM32_NOR_SIMULATOR_DRIVER_H */

