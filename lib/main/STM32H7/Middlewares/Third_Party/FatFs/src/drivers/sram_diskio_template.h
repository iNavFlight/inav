/**
  ******************************************************************************
  * @file    sram_diskio_template.h
  * @author  MCD Application Team
  * @brief   Header for sram_diskio_template.c module.This file has to be
             customized and copied under the application project
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
**/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SRAM_DISKIO_H
#define __SRAM_DISKIO_H

/* Includes ------------------------------------------------------------------*/
#include "stm32xxxxx_{eval}{discovery}_sram.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern const Diskio_drvTypeDef  SRAMDISK_Driver;

#endif /* __SRAM_DISKIO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

