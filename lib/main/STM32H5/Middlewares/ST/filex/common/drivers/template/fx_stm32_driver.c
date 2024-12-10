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
#include "fx_stm32_custom_driver.h"

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

/* USER CODE BEGIN USER_CODE_SECTION_0 */

/* USER CODE END USER_CODE_SECTION_0 */

VOID  fx_stm32_custom_driver(FX_MEDIA *media_ptr)
{
  /* USER CODE BEGIN USER_CODE_SECTION_1 */
  
  /* USER CODE END USER_CODE_SECTION_1 */

  switch (media_ptr->fx_media_driver_request)
  {
    case FX_DRIVER_INIT:
    {

     /* USER CODE BEGIN DRIVER_INIT */

     /* USER CODE END DRIVER_INIT */

      media_ptr->fx_media_driver_status = FX_SUCCESS;


     /* USER CODE BEGIN POST_DRIVER_INIT */

     /* USER CODE END POST_DRIVER_INIT */
      break;
    }

    case FX_DRIVER_UNINIT:
    {
     /* USER CODE BEGIN DRIVER_UNINIT */

     /* USER CODE END DRIVER_UNINIT */

      media_ptr->fx_media_driver_status = FX_SUCCESS;

    /* USER CODE BEGIN POST_DRIVER_UNINIT */

     /* USER CODE END POST_DRIVER_UNINIT */
      break;
    }

    case FX_DRIVER_BOOT_READ:
    {
    /* USER CODE BEGIN DRIVER_BOOT_READ */

     /* USER CODE END DRIVER_BOOT_READ */

      media_ptr->fx_media_driver_status = FX_SUCCESS;

    /* USER CODE BEGIN POST_DRIVER_BOOT_READ */

     /* USER CODE END POST_DRIVER_BOOT_READ */
      break;
    }

    case FX_DRIVER_READ:
    {
    /* USER CODE BEGIN DRIVER_READ */

     /* USER CODE END DRIVER_READ */

      media_ptr->fx_media_driver_status = FX_SUCCESS;

    /* USER CODE BEGIN POST_DRIVER_READ */

     /* USER CODE END POST_DRIVER_READ */
      break;
    }

    case FX_DRIVER_BOOT_WRITE:
    {
    /* USER CODE BEGIN DRIVER_BOOT_WRITE */

     /* USER CODE END DRIVER_BOOT_WRITE */

      media_ptr->fx_media_driver_status = FX_SUCCESS;

    /* USER CODE BEGIN DRIVER_BOOT_WRITE */

     /* USER CODE END DRIVER_BOOT_WRITE */
      break;
    }

    case FX_DRIVER_WRITE:
    {

    /* USER CODE BEGIN DRIVER_WRITE */

     /* USER CODE END DRIVER_WRITE */

      media_ptr->fx_media_driver_status = FX_SUCCESS;

     /* USER CODE BEGIN POST_DRIVER_WRITE */

     /* USER CODE END POST_DRIVER_WRITE */
      break;
    }

    case FX_DRIVER_FLUSH:
    {
    /* USER CODE BEGIN DRIVER_FLUSH */

     /* USER CODE END DRIVER_FLUSH */

      media_ptr->fx_media_driver_status = FX_SUCCESS;

    /* USER CODE BEGIN POST_DRIVER_FLUSH */

     /* USER CODE END POST_DRIVER_FLUSH */
      break;
    }

    case FX_DRIVER_ABORT:
    {

    /* USER CODE BEGIN DRIVER_ABORT */

     /* USER CODE END DRIVER_ABORT */

      media_ptr->fx_media_driver_status = FX_SUCCESS;

    /* USER CODE BEGIN POST_DRIVER_ABORT */

     /* USER CODE END POST_DRIVER_ABORT */
      break;
    }

    default:
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        break;
    }
  }
}

/* USER CODE BEGIN USER_CODE_SECTION_2 */

/* USER CODE END USER_CODE_SECTION_2 */

