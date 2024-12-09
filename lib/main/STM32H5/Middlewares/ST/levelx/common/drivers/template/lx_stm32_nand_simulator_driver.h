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

#ifndef LX_STM32_NAND_SIMULATOR_DRIVER_H
#define LX_STM32_NAND_SIMULATOR_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lx_api.h"
#include "stm32XXX_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

#define TOTAL_BLOCKS                        8

#define PHYSICAL_PAGES_PER_BLOCK            16        /* Min value of 2            */
#define BYTES_PER_PHYSICAL_PAGE             2048      /* 2048 bytes per page       */

#define WORDS_PER_PHYSICAL_PAGE             2048/4    /* Words per page            */
#define SPARE_BYTES_PER_PAGE                64        /* 64 "spare" bytes per page */

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* Define the function prototypes of the LevelX driver entry function.  */

UINT  lx_stm32_nand_simulator_initialize(LX_NAND_FLASH *nor_flash);

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
#endif /* LX_STM32_NAND_SIMULATOR_DRIVER_H */

