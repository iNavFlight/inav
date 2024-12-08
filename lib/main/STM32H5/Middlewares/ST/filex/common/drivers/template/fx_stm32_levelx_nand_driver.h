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
#ifndef FX_STM32_LX_NAND_DRIVER_H
#define FX_STM32_LX_NAND_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"
#include "lx_api.h"

/* enable the driver to be used */

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

#ifdef  LX_NAND_SIMULATOR_DRIVER
#include "lx_stm32_nand_simulator_driver.h"

#define  LX_NAND_SIMULATOR_DRIVER_ID        0x01
#define  LX_NAND_SIMULATOR_DRIVER_NAME      "FX Levelx NAND flash Simulator"
#endif


#ifdef  LX_NAND_CUSTOM_DRIVER
/*
 * define the Custom levelx nand drivers to be supported by the filex
 *  for example:

#define CUSTOM_DRIVER_ID           0xABCDEF
#define NAND_CUSTOM_DRIVER_NAME    "NAND CUSTOM DRIVER"
#include "lx_nand_custom_driver.h"
#define LX_NAND_CUSTOM_DRIVERS   {.name = NAND_CUSTOM_DRIVER_NAME,  .id = CUSTOM_DRIVER_ID, .nand_driver_initialize = lx_nand_flash_initialize_driver}
 */

/* USER CODE BEGIN CUSTOM_DRIVER */

/* USER CODE END CUSTOM_DRIVER */

#endif

#define MAX_LX_NAND_DRIVERS     8
#define UNKNOWN_DRIVER_ID        0xFFFFFFFF

/* to enable a default NAND driver:
  - define the flags LX_NAND_DEFAULT_DRIVER
  - Provide the driver ID in the NOR_DEFAULT_DRIVER for example
  #define NAND_DEFAULT_DRIVER LX_NAND_SIMULATOR_DRIVER_ID
*/


/* USER CODE BEGIN DEFAULT_DRIVER */

/* USER CODE END DEFAULT_DRIVER */

#ifdef LX_NAND_DEFAULT_DRIVER

/* USER CODE BEGIN DEFAULT_DRIVER */

/* USER CODE END DEFAULT_DRIVER */
#endif

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */
#if !defined(LX_NAND_DEFAULT_DRIVER) && !defined (LX_NAND_CUSTOM_DRIVERS) && !defined(LX_NAND_SIMULATOR_DRIVER)
#error "[This error was thrown on purpose] : No NAND lowlevel driver defined"
#endif
/* Exported functions prototypes ---------------------------------------------*/
VOID  fx_stm32_levelx_nand_driver(FX_MEDIA *media_ptr);

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

#endif /*FX_STM32_LX_NAND_DRIVER_H*/
