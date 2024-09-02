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

/* Private includes ----------------------------------------------------------*/
#include "nx_stm32_phy_driver.h"
#include "nx_stm32_eth_config.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */


/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int32_t nx_eth_phy_init(void)
{

/* USER CODE BEGIN PHY_INIT_0 */

/* USER CODE END PHY_INIT_0 */


    int32_t ret = ETH_PHY_STATUS_OK;

/* USER CODE BEGIN PHY_INIT_1 */

/* USER CODE END PHY_INIT_1 */
    return ret;
}

int32_t nx_eth_phy_get_link_state(void)
{

  /* USER CODE BEGIN LINK_STATE_0 */

  /* USER CODE END LINK_STATE_0 */

  int32_t  linkstate = ETH_PHY_STATUS_LINK_ERROR;


  /* USER CODE BEGIN LINK_STATE_1 */

  /* USER CODE END LINK_STATE_1 */
  return linkstate;
}

nx_eth_phy_handle_t nx_eth_phy_get_handle(void)
{

 nx_eth_phy_handle_t handle = NULL;
  /* USER CODE BEGIN GET_HANDLE */

  /* USER CODE END GET_HANDLE */
    return handle;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
