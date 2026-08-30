/**
  ******************************************************************************
  * @file    mt25tl01g_conf_template.h
  * @author  MCD Application Team
  * @brief   This file contains all the description of the
  *          MT25TL01G QSPI memory.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MT25TL01G_CONF_H
#define MT25TL01G_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32xxxx.h"
#include "stm32xxxx_hal.h"

/** @addtogroup BSP
  * @{
  */

#define CONF_MT25TL01G_READ_ENHANCE      0                       /* MMP performance enhance reade enable/disable */

#define CONF_QSPI_ODS                   MT25TL01G_CR_ODS_15

#define CONF_QSPI_DUMMY_CLOCK                 8U

/* Dummy cycles for STR read mode */
#define MT25TL01G_DUMMY_CYCLES_READ_QUAD      8U
#define MT25TL01G_DUMMY_CYCLES_READ           8U
/* Dummy cycles for DTR read mode */
#define MT25TL01G_DUMMY_CYCLES_READ_DTR       6U
#define MT25TL01G_DUMMY_CYCLES_READ_QUAD_DTR  8U

#ifdef __cplusplus
}
#endif

#endif /* MT25TL01G_CONF_H */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
