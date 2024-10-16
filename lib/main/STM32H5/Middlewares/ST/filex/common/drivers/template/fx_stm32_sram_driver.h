
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
#ifndef FX_STM32_SRAM_DRIVER_H
#define FX_STM32_SRAM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"

#include "stm32XXXX.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

#define FX_SRAM_DISK_BASE_ADDRESS         D1_AXISRAM_BASE

/* define the SRAM DISK size */
#define FX_SRAM_DISK_SIZE                 (32 * 1024)


/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

VOID fx_stm32_ram_driver(FX_MEDIA *media_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifdef __cplusplus
}
#endif
#endif /* FX_STM32_SRAM_DRIVER_H */
