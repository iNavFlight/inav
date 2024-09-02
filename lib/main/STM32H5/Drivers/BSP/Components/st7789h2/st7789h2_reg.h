/**
  ******************************************************************************
  * @file    st7789h2_reg.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the st7789h2_reg.c
  *          driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ST7789H2_REG_H
#define ST7789H2_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup ST7789H2
  * @{
  */

/** @defgroup ST7789H2_REG_Exported_Constants ST7789H2_REG Exported Constants
  * @{
  */
/**
  * @brief  ST7789H2 Registers
  */
#define ST7789H2_NOP                          0x00U
#define ST7789H2_SW_RESET                     0x01U
#define ST7789H2_READ_ID                      0x04U
#define ST7789H2_READ_STATUS                  0x09U
#define ST7789H2_READ_POWER_MODE              0x0AU
#define ST7789H2_READ_MADCTL                  0x0BU
#define ST7789H2_READ_PIXEL_FORMAT            0x0CU
#define ST7789H2_READ_IMAGE_MODE              0x0DU
#define ST7789H2_READ_SIGNAL_MODE             0x0EU
#define ST7789H2_READ_SELF_DIAGNOSTIC         0x0FU
#define ST7789H2_SLEEP_IN                     0x10U
#define ST7789H2_SLEEP_OUT                    0x11U
#define ST7789H2_PARTIAL_DISPLAY_ON           0x12U
#define ST7789H2_NORMAL_DISPLAY_OFF           0x13U
#define ST7789H2_DISPLAY_INVERSION_OFF        0x20U
#define ST7789H2_DISPLAY_INVERSION_ON         0x21U
#define ST7789H2_GAMMA_SET                    0x26U
#define ST7789H2_DISPLAY_OFF                  0x28U
#define ST7789H2_DISPLAY_ON                   0x29U
#define ST7789H2_CASET                        0x2AU
#define ST7789H2_RASET                        0x2BU
#define ST7789H2_WRITE_RAM                    0x2CU
#define ST7789H2_READ_RAM                     0x2EU
#define ST7789H2_VSCRDEF                      0x33U /* Vertical Scroll Definition */
#define ST7789H2_TE_LINE_OFF                  0x34U
#define ST7789H2_TE_LINE_ON                   0x35U
#define ST7789H2_MADCTL                       0x36U /* Memory Data Access Control */
#define ST7789H2_VSCSAD                       0x37U /* Vertical Scroll Start Address of RAM */
#define ST7789H2_IDLE_MODE_OFF                0x38U
#define ST7789H2_IDLE_MODE_ON                 0x39U
#define ST7789H2_COLOR_MODE                   0x3AU
#define ST7789H2_WRITE_RAM_CONTINUE           0x3CU
#define ST7789H2_READ_RAM_CONTINUE            0x3EU
#define ST7789H2_SET_SCANLINE                 0x44U
#define ST7789H2_GET_SCANLINE                 0x45U
#define ST7789H2_SET_BRIGHTNESS               0x51U
#define ST7789H2_GET_BRIGHTNESS               0x52U
#define ST7789H2_WRITE_CTRL_DISPLAY           0x53U
#define ST7789H2_READ_CTRL_DISPLAY            0x54U
#define ST7789H2_WRITE_CACE                   0x55U /* Write Content Adaptive Brightness Control and Color Enhancement */
#define ST7789H2_READ_CABC                    0x56U /* Read Content Adaptive Brightness Control */
#define ST7789H2_SET_CABC_MIN_BRIGHTNESS      0x5EU
#define ST7789H2_GET_CABC_MIN_BRIGHTNESS      0x5FU
#define ST7789H2_READ_ABCSDR                  0x68U /* Read Automatic Brightness Control Self-Diagnostic Result */
#define ST7789H2_READ_ID1                     0xDAU
#define ST7789H2_READ_ID2                     0xDBU
#define ST7789H2_READ_ID3                     0xDCU
#define ST7789H2_RAM_CTRL                     0xB0U
#define ST7789H2_RGB_INTERFACE_CTRL           0xB1U
#define ST7789H2_PORCH_CTRL                   0xB2U
#define ST7789H2_FRAME_RATE_CTRL1             0xB3U
#define ST7789H2_PARTIAL_CTRL                 0xB5U
#define ST7789H2_GATE_CTRL                    0xB7U
#define ST7789H2_GATE_TIMING_ADJUSTMENT       0xB8U
#define ST7789H2_DIGITAL_GAMMA_ENABLE         0xBAU
#define ST7789H2_VCOM_SET                     0xBBU
#define ST7789H2_PWR_SAVING_MODE              0xBCU
#define ST7789H2_DISPLAY_OFF_PWR_SAVE         0xBDU
#define ST7789H2_LCM_CTRL                     0xC0U
#define ST7789H2_ID_CODE_SETTING              0xC1U
#define ST7789H2_VDV_VRH_EN                   0xC2U
#define ST7789H2_VRH_SET                      0xC3U
#define ST7789H2_VDV_SET                      0xC4U
#define ST7789H2_VCOMH_OFFSET_SET             0xC5U
#define ST7789H2_FR_CTRL                      0xC6U
#define ST7789H2_CABC_CTRL                    0xC7U
#define ST7789H2_REG_VALUE_SELECTION1         0xC8U
#define ST7789H2_REG_VALUE_SELECTION2         0xCAU
#define ST7789H2_PWM_FREQ_SELECTION           0xCCU
#define ST7789H2_POWER_CTRL                   0xD0U
#define ST7789H2_EN_VAP_VAN_SIGNAL_OUTPUT     0xD2U
#define ST7789H2_COMMAND2_ENABLE              0xDFU
#define ST7789H2_PV_GAMMA_CTRL                0xE0U /* Positive voltage */
#define ST7789H2_NV_GAMMA_CTRL                0xE1U /* Negative voltage */
#define ST7789H2_GAMMA_RED_TABLE              0xE2U
#define ST7789H2_GAMMA_BLUE_TABLE             0xE3U
#define ST7789H2_GATE_CTRL2                   0xE4U
#define ST7789H2_SPI2_ENABLE                  0xE7U
#define ST7789H2_PWR_CTRL2                    0xE8U
#define ST7789H2_EQUALIZE_TIME_CTRL           0xE9U
#define ST7789H2_PROGRAM_MODE_CTRL            0xECU
#define ST7789H2_PROGRAM_MODE_ENABLE          0xFAU
#define ST7789H2_NVM_SETTING                  0xFCU
#define ST7789H2_PROGRAM_ACTION               0xFEU
/**
  * @}
  */

/** @defgroup ST7789H2_REG_Exported_Types ST7789H2_REG Exported Types
  * @{
  */
typedef int32_t (*ST7789H2_Write_Func)(const void *, uint16_t, uint8_t *, uint32_t);
typedef int32_t (*ST7789H2_Read_Func)(const void *, uint16_t, uint8_t *, uint32_t);
typedef int32_t (*ST7789H2_Send_Func)(const void *, uint8_t *, uint32_t);

typedef struct
{
  ST7789H2_Write_Func   WriteReg;
  ST7789H2_Read_Func    ReadReg;
  ST7789H2_Send_Func    SendData;
  void                  *handle;
} ST7789H2_ctx_t;
/**
  * @}
  */

/** @defgroup ST7789H2_REG_Exported_Functions ST7789H2_REG Exported Functions
  * @{
  */
int32_t st7789h2_write_reg(const ST7789H2_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint32_t length);
int32_t st7789h2_read_reg(const ST7789H2_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint32_t length);
int32_t st7789h2_send_data(const ST7789H2_ctx_t *ctx, uint8_t *pdata, uint32_t length);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* ST7789H2_REG_H */
