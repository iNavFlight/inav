/**
  ******************************************************************************
  * @file    gt911_conf.h
  * @author  MCD Application Team
  * @brief   This file contains specific configuration for the
  *          gt911.c that can be modified by user.
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
#ifndef GT911_CONF_H
#define GT911_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Macros --------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define GT911_AUTO_CALIBRATION_ENABLED      0U
#define GT911_MAX_X_LENGTH                  800U
#define GT911_MAX_Y_LENGTH                  480U

#ifdef __cplusplus
}
#endif
#endif /* GT911_CONF_H */
