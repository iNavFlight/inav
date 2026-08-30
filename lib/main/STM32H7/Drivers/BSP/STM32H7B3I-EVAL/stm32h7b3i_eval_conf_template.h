/**
  ******************************************************************************
  * @file    stm32h7b3i_eval_conf.h
  * @author  MCD Application Team
  * @brief   STM32H7B3I_EVAL board configuration file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H7B3I_EVAL_CONF_H
#define STM32H7B3I_EVAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* COM define */
#define USE_COM_LOG                         0U
#define USE_BSP_COM_FEATURE                 0U
#define USE_DMA2D_TO_FILL_RGB_RECT          0U

/* POT define */
#define USE_BSP_POT_FEATURE                 0U

/* IO CLASS define */
#define USE_BSP_IO_CLASS                    1U

/* I2C BUS timing define */
#define I2C_VALID_TIMING_NBR                128U

/* LCD defines */
#define LCD_LAYER_0_ADDRESS                 0xD0000000U
#define LCD_LAYER_1_ADDRESS                 0xD0200000U
#define USE_DMA2D_TO_FILL_RGB_RECT          0U

/* Camera sensors defines */
#define USE_CAMERA_SENSOR_OV5640            1U
#define USE_CAMERA_SENSOR_S5K5CAG           1U

/* Audio codecs defines */
#define USE_AUDIO_CODEC_CS42L51             1U

/* Default Audio IN internal buffer size */
#define DEFAULT_AUDIO_IN_BUFFER_SIZE        2048U
#define USE_BSP_CPU_CACHE_MAINTENANCE       1U

/* TS defines */
#define USE_TS_GESTURE                      1U
#define USE_TS_MULTI_TOUCH                  1U
#define TS_TOUCH_NBR                        2U

/* Default EEPROM max trials */
#define EEPROM_MAX_TRIALS                   3000U

/* IRQ priorities */
#define BSP_SRAM_IT_PRIORITY                15U
#define BSP_SDRAM_IT_PRIORITY               15U
#define BSP_CAMERA_IT_PRIORITY              15U
#define BSP_IOEXPANDER_IT_PRIORITY          15U
#define BSP_BUTTON_USER_IT_PRIORITY         15U
#define BSP_BUTTON_WAKEUP_IT_PRIORITY       15U
#define BSP_BUTTON_TAMPER_IT_PRIORITY       15U
#define BSP_AUDIO_OUT_IT_PRIORITY           14U
#define BSP_AUDIO_IN_IT_PRIORITY            15U
#define BSP_SD_IT_PRIORITY                  14U
#define BSP_SD_RX_IT_PRIORITY               14U
#define BSP_SD_TX_IT_PRIORITY               15U
#define BSP_TS_IT_PRIORITY                  15U

#define BSP_JOY1_SEL_IT_PRIORITY            15U
#define BSP_JOY1_DOWN_IT_PRIORITY           15U
#define BSP_JOY1_LEFT_IT_PRIORITY           15U
#define BSP_JOY1_RIGHT_IT_PRIORITY          15U
#define BSP_JOY1_UP_IT_PRIORITY             15U

#ifdef __cplusplus
}
#endif

#endif /* STM32H7B3I_EVAL_CONF_H */
