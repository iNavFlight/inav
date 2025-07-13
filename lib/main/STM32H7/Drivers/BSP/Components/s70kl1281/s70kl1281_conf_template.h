/**
  ******************************************************************************
  * @file    s70kl1281_conf.h
  * @author  MCD Application Team
  * @brief   S70KL1281 OctalSPI memory configuration template file.
  *          This file should be copied to the application folder and renamed
  *          to s70kl1281_conf.h
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#ifndef S70KL1281_CONF_H
#define S70KL1281_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32xxxx_hal.h"

/** @addtogroup BSP
  * @{
  */
#define CONF_OSPI_RAM_ODS                         S70KL1281_CR0_DS_34   /* S70KL1281 Output Driver Strength */

#define RW_RECOVERY_TIME                          3U /* 40ns @ 60MHz */
#define DEFAULT_INITIAL_LATENCY                   6U
#define OPTIMAL_FIXED_INITIAL_LATENCY             3U
#define OPTIMAL_FIXED_INITIAL_LATENCY_REG_VAL     S70KL1281_CR0_IL_3_CLOCK
#define OPTIMAL_VARIABLE_INITIAL_LATENCY          6U
#define OPTIMAL_VARIABLE_INITIAL_LATENCY_REG_VAL  S70KL1281_CR0_IL_6_CLOCK

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* S70KL1281_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
