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
#include "nx_stm32_custom_driver.h"

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

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

VOID  nx_stm32_custom_driver(NX_IP_DRIVER *driver_req_ptr)
{

    /* Process according to the driver request type in the IP control
       block.  */
    switch (driver_req_ptr -> nx_ip_driver_command)
    {

    case NX_LINK_INTERFACE_ATTACH:
    /* USER CODE BEGIN NX_LINK_INTERFACE_ATTACH */

    /* USER CODE END NX_LINK_INTERFACE_ATTACH */
        break;

    case NX_LINK_INITIALIZE:
    /* USER CODE BEGIN NX_LINK_INITIALIZE */

    /* USER CODE END NX_LINK_INITIALIZE */
        break;

    case NX_LINK_ENABLE:
    /* USER CODE BEGIN NX_LINK_ENABLE */

    /* USER CODE END NX_LINK_ENABLE */
        break;

    case NX_LINK_DISABLE:
    /* USER CODE BEGIN NX_LINK_DISABLE */

    /* USER CODE END NX_LINK_DISABLE */
        break;

    case NX_LINK_ARP_SEND:
    /* USER CODE BEGIN NX_LINK_ARP_SEND */

    /* USER CODE END NX_LINK_ARP_SEND */
        break;

    case NX_LINK_ARP_RESPONSE_SEND:
    /* USER CODE BEGIN NX_LINK_ARP_RESPONSE_SEND */

    /* USER CODE END NX_LINK_ARP_RESPONSE_SEND */
        break;

    case NX_LINK_PACKET_BROADCAST:
    /* USER CODE BEGIN NX_LINK_PACKET_BROADCAST */

    /* USER CODE END NX_LINK_PACKET_BROADCAST */
        break;

    case NX_LINK_RARP_SEND:
    /* USER CODE BEGIN NX_LINK_RARP_SEND */

    /* USER CODE END NX_LINK_RARP_SEND */
        break;

    case NX_LINK_PACKET_SEND:
    /* USER CODE BEGIN NX_LINK_PACKET_SEND */

    /* USER CODE END NX_LINK_PACKET_SEND */
        break;

    case NX_LINK_MULTICAST_JOIN:
    /* USER CODE BEGIN NX_LINK_MULTICAST_JOIN */

    /* USER CODE END NX_LINK_MULTICAST_JOIN */
        break;

    case NX_LINK_MULTICAST_LEAVE:
    /* USER CODE BEGIN NX_LINK_MULTICAST_LEAVE */

    /* USER CODE END NX_LINK_MULTICAST_LEAVE */
        break;

    case NX_LINK_GET_STATUS:
    /* USER CODE BEGIN NX_LINK_GET_STATUS */

    /* USER CODE END NX_LINK_GET_STATUS */
        break;

    case NX_LINK_DEFERRED_PROCESSING:
    /* USER CODE BEGIN NX_LINK_DEFERRED_PROCESSING */

    /* USER CODE END NX_LINK_DEFERRED_PROCESSING */
        break;


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    case NX_INTERFACE_CAPABILITY_GET:
    /* USER CODE BEGIN NX_INTERFACE_CAPABILITY_GET */

    /* USER CODE END NX_INTERFACE_CAPABILITY_GET */
        break;

    case NX_INTERFACE_CAPABILITY_SET:
    /* USER CODE BEGIN NX_INTERFACE_CAPABILITY_SET */

    /* USER CODE END NX_INTERFACE_CAPABILITY_SET */
        break;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    default:
    /* USER CODE BEGIN DEFAULT */

    /* USER CODE END DEFAULT */
        break;
    }
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
