/**
  ******************************************************************************
  * @file    engibytes_interface.h
  * @author  MCD Application Team
  * @brief   Header for engibytes_interface.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ENGIBYTES_INTERFACE_H
#define ENGIBYTES_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t OPENBL_EB_Read(uint32_t Address);

#ifdef __cplusplus
}
#endif

#endif /* ENGIBYTES_INTERFACE_H */
