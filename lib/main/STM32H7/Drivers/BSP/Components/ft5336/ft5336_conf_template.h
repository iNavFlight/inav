/**
  ******************************************************************************
  * @file    ft5336_conf.h
  * @author  MCD Application Team
  * @brief   This file contains specific configuration for the
  *          ft5336.c that can be modified by user.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FT5336_CONF_H
#define FT5336_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Macros --------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define FT5336_AUTO_CALIBRATION_ENABLED      0U
#define FT5336_MAX_X_LENGTH                  800U
#define FT5336_MAX_Y_LENGTH                  480U
  
#ifdef __cplusplus
}
#endif
#endif /* FT5336_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
