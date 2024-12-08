/**
  ******************************************************************************
  * @file    cs42l51_reg.h
  * @author  MCD Application Team
  * @brief   Header of cs42l51_reg.c
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CS42L51_REG_H
#define CS42L51_REG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup CS42L51
  * @{
  */


/** @defgroup CS42L51_Exported_Constants CS42L51 Exported Constants
  * @{
  */
/******************************************************************************/
/****************************** REGISTER MAPPING ******************************/
/******************************************************************************/
#define CS42L51_CHIP_ID                     0x01U
#define CS42L51_PWR_CTRL_1                  0x02U
#define CS42L51_MIC_PWR_SPEED_CTRL          0x03U
#define CS42L51_INTERFACE_CTRL              0x04U
#define CS42L51_MIC_CTRL                    0x05U
#define CS42L51_ADC_CTRL                    0x06U
#define CS42L51_ADCX_INPUT_SELECT           0x07U
#define CS42L51_DAC_OUTPUT_CTRL             0x08U
#define CS42L51_DAC_CTRL                    0x09U
#define CS42L51_ALCA_PGAA_CTRL              0x0AU
#define CS42L51_ALCB_PGAB_CTRL              0x0BU
#define CS42L51_ADCA_ATTENUATOR             0x0CU
#define CS42L51_ADCB_ATTENUATOR             0x0DU
#define CS42L51_ADCA_MIXER_VOL_CTRL         0x0EU
#define CS42L51_ADCB_MIXER_VOL_CTRL         0x0FU
#define CS42L51_PCMA_MIXER_VOL_CTRL         0x10U
#define CS42L51_PCMB_MIXER_VOL_CTRL         0x11U
#define CS42L51_BEEP_FREQ_AND_TIMING_CFG    0x12U
#define CS42L51_BEEP_OFF_TIME_AND_VOL       0x13U
#define CS42L51_BEEP_CFG_AND_TONE_CFG       0x14U
#define CS42L51_TONE_CTRL                   0x15U
#define CS42L51_AOUTA_VOL_CTRL              0x16U
#define CS42L51_AOUTB_VOL_CTRL              0x17U
#define CS42L51_ADC_PCM_CHANNEL_MIXER       0x18U
#define CS42L51_LIMITER_THR_SZC_DISABLE     0x19U
#define CS42L51_LIMITER_RELEASE_RATE_REG    0x1AU
#define CS42L51_LIMITER_ATTACK_RATE_REG     0x1BU
#define CS42L51_ALC_ENABLE_AND_ATTACK_RATE  0x1CU
#define CS42L51_ALC_RELEASE_RATE            0x1DU
#define CS42L51_ALC_THR                     0x1EU
#define CS42L51_NOISE_GATE_CFG_AND_MISC     0x1FU
#define CS42L51_STATUS                      0x20U
#define CS42L51_CHARGE_PUMP_FREQ            0x21U

/**
  * @}
  */

/************** Generic Function  *******************/

typedef int32_t (*CS42L51_Write_Func)(const void *, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*CS42L51_Read_Func)(const void *, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  CS42L51_Write_Func   WriteReg;
  CS42L51_Read_Func    ReadReg;
  void                *handle;
} cs42l51_ctx_t;

/*******************************************************************************
  * Register      : Generic - All
  * Address       : Generic - All
  * Bit Group Name: None
  * Permission    : W
  *******************************************************************************/
int32_t cs42l51_write_reg(const cs42l51_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length);
int32_t cs42l51_read_reg(const cs42l51_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* CS42L51_REG_H */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
