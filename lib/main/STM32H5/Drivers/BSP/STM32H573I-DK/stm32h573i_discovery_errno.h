/**
  ******************************************************************************
  * @file    stm32h573i_discovery_errno.h
  * @author  MCD Application Team
  * @brief   Error Code.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H573I_DK_ERRNO_H
#define STM32H573I_DK_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Common Error codes */
#define BSP_ERROR_NONE                        0
#define BSP_ERROR_NO_INIT                    -1
#define BSP_ERROR_WRONG_PARAM                -2
#define BSP_ERROR_BUSY                       -3
#define BSP_ERROR_PERIPH_FAILURE             -4
#define BSP_ERROR_COMPONENT_FAILURE          -5
#define BSP_ERROR_UNKNOWN_FAILURE            -6
#define BSP_ERROR_UNKNOWN_COMPONENT          -7
#define BSP_ERROR_BUS_FAILURE                -8
#define BSP_ERROR_CLOCK_FAILURE              -9
#define BSP_ERROR_MSP_FAILURE                -10
#define BSP_ERROR_FEATURE_NOT_SUPPORTED      -11

/* BSP OSPI error codes */
#define BSP_ERROR_OSPI_SUSPENDED             -20
#define BSP_ERROR_OSPI_ASSIGN_FAILURE        -24
#define BSP_ERROR_OSPI_SETUP_FAILURE         -25
#define BSP_ERROR_OSPI_MMP_LOCK_FAILURE      -26
#define BSP_ERROR_OSPI_MMP_UNLOCK_FAILURE    -27

/* BSP BUS error codes */
#define BSP_ERROR_BUS_TRANSACTION_FAILURE    -100
#define BSP_ERROR_BUS_ARBITRATION_LOSS       -101
#define BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE    -102
#define BSP_ERROR_BUS_PROTOCOL_FAILURE       -103

#define BSP_ERROR_BUS_MODE_FAULT             -104
#define BSP_ERROR_BUS_FRAME_ERROR            -105
#define BSP_ERROR_BUS_CRC_ERROR              -106
#define BSP_ERROR_BUS_DMA_FAILURE            -107

#ifdef __cplusplus
}
#endif

#endif /* STM32H573I_DK_ERRNO_H */
