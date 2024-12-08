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


#ifndef NX_STM32_ETH_CONFIG_H
#define NX_STM32_ETH_CONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32NNxx_hal.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/* This define enables the call of nx_eth_init() from the interface layer.*/
/* #define NX_DRIVER_ETH_HW_IP_INIT */

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

extern ETH_HandleTypeDef heth;

#ifdef NX_DRIVER_ETH_HW_IP_INIT
extern void MX_ETH_Init(void);
#endif /* #define NX_DRIVER_ETH_HW_IP_INIT */

#define eth_handle  heth

#ifdef NX_DRIVER_ETH_HW_IP_INIT
#define nx_eth_init MX_ETH_Init
#endif /* #define NX_DRIVER_ETH_HW_IP_INIT */

/* Add Phy Ethernet specific defines */

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

#endif /* NX_STM32_ETH_CONFIG_H */

