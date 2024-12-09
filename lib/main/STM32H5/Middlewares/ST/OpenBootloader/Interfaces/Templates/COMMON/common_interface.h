/**
  ******************************************************************************
  * @file    common_interface.h
  * @author  MCD Application Team
  * @brief   Header for common_interface.c module
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
#ifndef COMMON_INTERFACE_H
#define COMMON_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef void (*Function_Pointer)(void);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Common_SetMsp(uint32_t TopOfMainStack);
void Common_EnableIrq(void);
void Common_DisableIrq(void);
FlagStatus Common_GetProtectionStatus(void);
void Common_SetPostProcessingCallback(Function_Pointer Callback);
void Common_StartPostProcessing(void);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_INTERFACE_H */
