/**
  ******************************************************************************
  * @file    ov5640.c
  * @author  MCD Application Team
  * @brief   This file provides the OV5640 camera driver
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
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ov5640.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup OV5640
  * @brief     This file provides a set of functions needed to drive the
  *            OV5640 Camera module.
  * @{
  */

/** @defgroup OV5640_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @defgroup OV5640_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup OV5640_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup OV5640_Private_FunctionPrototypes
  * @{
  */
static uint32_t ov5640_ConvertValue(uint32_t feature, uint32_t value);
/**
  * @}
  */

/** @defgroup OV5640_Private_Variables
  * @{
  */

CAMERA_DrvTypeDef   ov5640_drv =
{
  ov5640_Init,
  ov5640_ReadID,
  ov5640_Config,
};

const uint16_t OV5640_Init[][2] =
{
  {0x3103, 0x11},
  {0x3008, 0x82},
  {0x3103, 0x03},
  {0x3017, 0xFF},
  {0x3018, 0xf3},
  {0x3034, 0x18},
  {0x3008, 0x02},
  {0x3035, 0x41},
  {0x3036, 0x60},
  {0x3037, 0x13},
  {0x3108, 0x01},
  {0x3630, 0x36},
  {0x3631, 0x0e},
  {0x3632, 0xe2},
  {0x3633, 0x12},
  {0x3621, 0xe0},
  {0x3704, 0xa0},
  {0x3703, 0x5a},
  {0x3715, 0x78},
  {0x3717, 0x01},
  {0x370b, 0x60},
  {0x3705, 0x1a},
  {0x3905, 0x02},
  {0x3906, 0x10},
  {0x3901, 0x0a},
  {0x3731, 0x12},
  {0x3600, 0x08},
  {0x3601, 0x33},
  {0x302d, 0x60},
  {0x3620, 0x52},
  {0x371b, 0x20},
  {0x471c, 0x50},
  {0x3a13, 0x43},
  {0x3a18, 0x00},
  {0x3a19, 0xf8},
  {0x3635, 0x13},
  {0x3636, 0x03},
  {0x3634, 0x40},
  {0x3622, 0x01},
  {0x3c01, 0x34},
  {0x3c04, 0x28},
  {0x3c05, 0x98},
  {0x3c06, 0x00},
  {0x3c07, 0x00},
  {0x3c08, 0x01},
  {0x3c09, 0x2c},
  {0x3c0a, 0x9c},
  {0x3c0b, 0x40},
  {0x3820, 0x00},
  {0x3821, 0x06},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3800, 0x00},
  {0x3801, 0x00},
  {0x3802, 0x00},
  {0x3803, 0x04},
  {0x3804, 0x0a},
  {0x3805, 0x3f},
  {0x3806, 0x07},
  {0x3807, 0x9b},
  {0x3808, 0x03},
  {0x3809, 0x20},
  {0x380a, 0x02},
  {0x380b, 0x58},
  {0x380c, 0x07},
  {0x380d, 0x68},
  {0x380e, 0x03},
  {0x380f, 0xd8},
  {0x3810, 0x00},
  {0x3811, 0x10},
  {0x3812, 0x00},
  {0x3813, 0x06},
  {0x3618, 0x00},
  {0x3612, 0x29},
  {0x3708, 0x64},
  {0x3709, 0x52},
  {0x370c, 0x03},
  {0x3a02, 0x03},
  {0x3a03, 0xd8},
  {0x3a08, 0x01},
  {0x3a09, 0x27},
  {0x3a0a, 0x00},
  {0x3a0b, 0xf6},
  {0x3a0e, 0x03},
  {0x3a0d, 0x04},
  {0x3a14, 0x03},
  {0x3a15, 0xd8},
  {0x4001, 0x02},
  {0x4004, 0x02},
  {0x3000, 0x00},
  {0x3002, 0x1c},
  {0x3004, 0xff},
  {0x3006, 0xc3},
  {0x300e, 0x58},
  {0x302e, 0x00},
  {0x4740, 0x20},
  {0x4300, 0x30},
  {0x501f, 0x00},
  {0x4713, 0x03},
  {0x4407, 0x04},
  {0x440e, 0x00},
  {0x460b, 0x35},
  {0x460c, 0x23},
  {0x4837, 0x22},
  {0x3824, 0x02},
  {0x5000, 0xa7},
  {0x5001, 0xa3},
  {0x5180, 0xff},
  {0x5181, 0xf2},
  {0x5182, 0x00},
  {0x5183, 0x14},
  {0x5184, 0x25},
  {0x5185, 0x24},
  {0x5186, 0x09},
  {0x5187, 0x09},
  {0x5188, 0x09},
  {0x5189, 0x75},
  {0x518a, 0x54},
  {0x518b, 0xe0},
  {0x518c, 0xb2},
  {0x518d, 0x42},
  {0x518e, 0x3d},
  {0x518f, 0x56},
  {0x5190, 0x46},
  {0x5191, 0xf8},
  {0x5192, 0x04},
  {0x5193, 0x70},
  {0x5194, 0xf0},
  {0x5195, 0xf0},
  {0x5196, 0x03},
  {0x5197, 0x01},
  {0x5198, 0x04},
  {0x5199, 0x12},
  {0x519a, 0x04},
  {0x519b, 0x00},
  {0x519c, 0x06},
  {0x519d, 0x82},
  {0x519e, 0x38},
  {0x5381, 0x1e},
  {0x5382, 0x5b},
  {0x5383, 0x08},
  {0x5384, 0x0a},
  {0x5385, 0x7e},
  {0x5386, 0x88},
  {0x5387, 0x7c},
  {0x5388, 0x6c},
  {0x5389, 0x10},
  {0x538a, 0x01},
  {0x538b, 0x98},
  {0x5300, 0x08},
  {0x5301, 0x30},
  {0x5302, 0x10},
  {0x5303, 0x00},
  {0x5304, 0x08},
  {0x5305, 0x30},
  {0x5306, 0x08},
  {0x5307, 0x16},
  {0x5309, 0x08},
  {0x530a, 0x30},
  {0x530b, 0x04},
  {0x530c, 0x06},
  {0x5480, 0x01},
  {0x5481, 0x08},
  {0x5482, 0x14},
  {0x5483, 0x28},
  {0x5484, 0x51},
  {0x5485, 0x65},
  {0x5486, 0x71},
  {0x5487, 0x7d},
  {0x5488, 0x87},
  {0x5489, 0x91},
  {0x548a, 0x9a},
  {0x548b, 0xaa},
  {0x548c, 0xb8},
  {0x548d, 0xcd},
  {0x548e, 0xdd},
  {0x548f, 0xea},
  {0x5490, 0x1d},
  {0x5580, 0x02},
  {0x5583, 0x40},
  {0x5584, 0x10},
  {0x5589, 0x10},
  {0x558a, 0x00},
  {0x558b, 0xf8},
  {0x5800, 0x23},
  {0x5801, 0x14},
  {0x5802, 0x0f},
  {0x5803, 0x0f},
  {0x5804, 0x12},
  {0x5805, 0x26},
  {0x5806, 0x0c},
  {0x5807, 0x08},
  {0x5808, 0x05},
  {0x5809, 0x05},
  {0x580a, 0x08},
  {0x580b, 0x0d},
  {0x580c, 0x08},
  {0x580d, 0x03},
  {0x580e, 0x00},
  {0x580f, 0x00},
  {0x5810, 0x03},
  {0x5811, 0x09},
  {0x5812, 0x07},
  {0x5813, 0x03},
  {0x5814, 0x00},
  {0x5815, 0x01},
  {0x5816, 0x03},
  {0x5817, 0x08},
  {0x5818, 0x0d},
  {0x5819, 0x08},
  {0x581a, 0x05},
  {0x581b, 0x06},
  {0x581c, 0x08},
  {0x581d, 0x0e},
  {0x581e, 0x29},
  {0x581f, 0x17},
  {0x5820, 0x11},
  {0x5821, 0x11},
  {0x5822, 0x15},
  {0x5823, 0x28},
  {0x5824, 0x46},
  {0x5825, 0x26},
  {0x5826, 0x08},
  {0x5827, 0x26},
  {0x5828, 0x64},
  {0x5829, 0x26},
  {0x582a, 0x24},
  {0x582b, 0x22},
  {0x582c, 0x24},
  {0x582d, 0x24},
  {0x582e, 0x06},
  {0x582f, 0x22},
  {0x5830, 0x40},
  {0x5831, 0x42},
  {0x5832, 0x24},
  {0x5833, 0x26},
  {0x5834, 0x24},
  {0x5835, 0x22},
  {0x5836, 0x22},
  {0x5837, 0x26},
  {0x5838, 0x44},
  {0x5839, 0x24},
  {0x583a, 0x26},
  {0x583b, 0x28},
  {0x583c, 0x42},
  {0x583d, 0xce},
  {0x5025, 0x00},
  {0x3a0f, 0x30},
  {0x3a10, 0x28},
  {0x3a1b, 0x30},
  {0x3a1e, 0x26},
  {0x3a11, 0x60},
  {0x3a1f, 0x14},
  {0x3008, 0x02},
};

/* Initialization sequence for WVGA resolution (800x480)*/
const uint16_t OV5640_WVGA[][2]=
{
  {0x3808, 0x03},
  {0x3809, 0x20},
  {0x380a, 0x01},
  {0x380b, 0xE0},
  {0x4300, 0x6F},
  {0x4740, 0x22},
  {0x501F, 0x01},
};

/* Initialization sequence for VGA resolution (640x480)*/
const uint16_t OV5640_VGA[][2]=
{
  {0x3808, 0x02},
  {0x3809, 0x80},
  {0x380a, 0x01},
  {0x380b, 0xE0},
  {0x4300, 0x6F},
  {0x4740, 0x22},
  {0x501F, 0x01},
};

/* Initialization sequence for 480x272 resolution */
const uint16_t OV5640_480x272[][2]=
{
  {0x3808, 0x01},
  {0x3809, 0xE0},
  {0x380a, 0x01},
  {0x380b, 0x10},
  {0x4300, 0x6F},
  {0x4740, 0x22},
  {0x501f, 0x01},
};

const uint16_t OV5640_QVGA[][2] =
{
  {0x3808, 0x01},
  {0x3809, 0x40},
  {0x380a, 0x00},
  {0x380b, 0xF0},
  {0x4300, 0x6F},
  {0x4740, 0x22},
  {0x501f, 0x01},
};

/* Initialization sequence for QQVGA resolution (160x120) */
const uint16_t OV5640_QQVGA[][2]=
{
  {0x3808, 0x00},
  {0x3809, 0xA0},
  {0x380a, 0x00},
  {0x380b, 0x78},
  {0x4300, 0x6F},
  {0x4740, 0x22},
  {0x501f, 0x01},
};

/* OV5640 Light Mode setting */
const uint16_t OV5640_LightModeAuto[][2]=
{
  {0x3406, 0x00},
  {0x3400, 0x04},
  {0x3401, 0x00},
  {0x3402, 0x04},
  {0x3403, 0x00},
  {0x3404, 0x04},
  {0x3405, 0x00},
};

const uint16_t OV5640_LightModeCloudy[][2]=
{
  {0x3406, 0x01},
  {0x3400, 0x06},
  {0x3401, 0x48},
  {0x3402, 0x04},
  {0x3403, 0x00},
  {0x3404, 0x04},
  {0x3405, 0xd3},
};

const uint16_t OV5640_LightModeOffice[][2]=
{
  {0x3406, 0x01},
  {0x3400, 0x05},
  {0x3401, 0x48},
  {0x3402, 0x04},
  {0x3403, 0x00},
  {0x3404, 0x07},
  {0x3405, 0xcf},
};

const uint16_t OV5640_LightModeHome[][2]=
{
  {0x3406, 0x01},
  {0x3400, 0x04},
  {0x3401, 0x10},
  {0x3402, 0x04},
  {0x3403, 0x00},
  {0x3404, 0x08},
  {0x3405, 0xB6},
};

const uint16_t OV5640_LightModeSunny[][2]=
{
  {0x3406, 0x01},
  {0x3400, 0x06},
  {0x3401, 0x1c},
  {0x3402, 0x04},
  {0x3403, 0x00},
  {0x3404, 0x04},
  {0x3405, 0xf3},
};

/**
  * @}
  */


/**
  * @brief  Initializes the OV5640 CAMERA component.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  resolution: Camera resolution
  * @retval None
  */
void ov5640_Init(uint16_t DeviceAddr, uint32_t resolution)
{
  uint32_t index = 0;

  for(index=0; index< (sizeof(OV5640_Init)/4) ; index++)
  {
    CAMERA_IO_Write(DeviceAddr, OV5640_Init[index][0], OV5640_Init[index][1]);
  }

  /* Initialize OV5640 */
  switch (resolution)
  {
  case CAMERA_R160x120:
    {
      for(index=0; index<(sizeof(OV5640_QQVGA)/4); index++)
      {
        CAMERA_IO_Write(DeviceAddr, OV5640_QQVGA[index][0], OV5640_QQVGA[index][1]);
      }
      break;
    }
  case CAMERA_R320x240:
    {
      for(index=0; index< (sizeof(OV5640_QVGA)/4); index++)
      {
        CAMERA_IO_Write(DeviceAddr, OV5640_QVGA[index][0], OV5640_QVGA[index][1]);
      }

      break;
    }
  case CAMERA_R480x272:
    {
      for(index=0; index<(sizeof(OV5640_480x272)/4); index++)
      {
        CAMERA_IO_Write(DeviceAddr, OV5640_480x272[index][0], OV5640_480x272[index][1]);
      }
      break;
    }
  case CAMERA_R640x480:
    {
      for(index=0; index<(sizeof(OV5640_VGA)/4); index++)
      {
        CAMERA_IO_Write(DeviceAddr, OV5640_VGA[index][0], OV5640_VGA[index][1]);
      }
      break;
    }
  default:
    {
      break;
    }
  }
}

/**
  * @brief  Set the OV5640 camera Light Mode.
  * @param  DeviceAddr : Device address on communication Bus.
  * @param  Effect : Effect to be configured
  * @retval None
  */
void OV5640_SetLightMode(uint16_t DeviceAddr, uint8_t LightMode)
{
  uint32_t index = 0;

  CAMERA_IO_Write(DeviceAddr, 0x3406, 0x00);
  CAMERA_IO_Write(DeviceAddr, 0x5190, 0x46);
  CAMERA_IO_Write(DeviceAddr, 0x5191, 0xf8);
  CAMERA_IO_Write(DeviceAddr, 0x5192, 0x04);

  switch(LightMode)
  {
  case OV5640_LIGHT_AUTO:
    for(index=0; index< (sizeof(OV5640_LightModeAuto)/4) ; index++)
    {
      CAMERA_IO_Write(DeviceAddr, OV5640_LightModeAuto[index][0], OV5640_LightModeAuto[index][1]);
    }
    break;
  case OV5640_LIGHT_SUNNY:
    for(index=0; index< (sizeof(OV5640_LightModeSunny)/4) ; index++)
    {
      CAMERA_IO_Write(DeviceAddr, OV5640_LightModeSunny[index][0], OV5640_LightModeSunny[index][1]);
    }
    break;
  case OV5640_LIGHT_OFFICE:
    for(index=0; index< (sizeof(OV5640_LightModeOffice)/4) ; index++)
    {
      CAMERA_IO_Write(DeviceAddr, OV5640_LightModeOffice[index][0], OV5640_LightModeOffice[index][1]);
    }
    break;
  case OV5640_LIGHT_CLOUDY:
    for(index=0; index< (sizeof(OV5640_LightModeCloudy)/4) ; index++)
    {
      CAMERA_IO_Write(DeviceAddr, OV5640_LightModeCloudy[index][0], OV5640_LightModeCloudy[index][1]);
    }
    break;
  case OV5640_LIGHT_HOME:
    for(index=0; index< (sizeof(OV5640_LightModeHome)/4) ; index++)
    {
      CAMERA_IO_Write(DeviceAddr, OV5640_LightModeHome[index][0], OV5640_LightModeHome[index][1]);
    }
    break;
  default :
    /* Auto light mode used */
    for(index=0; index< (sizeof(OV5640_LightModeAuto)/4) ; index++)
    {
      CAMERA_IO_Write(DeviceAddr, OV5640_LightModeAuto[index][0], OV5640_LightModeAuto[index][1]);
    }
    break;
  }
}

/**
  * @brief  Set the OV5640 camera Special Effect.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  Effect: Effect to be configured
  * @retval None
  */
void OV5640_SetEffect(uint16_t DeviceAddr, uint32_t Effect)
{
  switch(Effect)
  {
  case OV5640_COLOR_EFFECT_NONE:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0x7F);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x00);
    break;

  case OV5640_COLOR_EFFECT_BLUE:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x18);
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0xA0);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x40);
    break;

  case OV5640_COLOR_EFFECT_RED:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x18);
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x80);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0xC0);
    break;

  case OV5640_COLOR_EFFECT_GREEN:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x18);
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x60);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x60);
    break;

  case OV5640_COLOR_EFFECT_BW:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x18);
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x80);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x80);
    break;

  case OV5640_COLOR_EFFECT_SEPIA:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x18);
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0xA0);
    break;

  case OV5640_COLOR_EFFECT_NEGATIVE:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x40);
    break;

  case OV5640_COLOR_EFFECT_BW_NEGATIVE:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x58);
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x40);
    break;

  case OV5640_COLOR_EFFECT_OVEREXPOSURE:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x18);
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0xF0);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0xF0);
    break;

  case OV5640_COLOR_EFFECT_SOLARIZE:
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x06);
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x10);
    CAMERA_IO_Write(DeviceAddr, 0x5003, 0x09);
    break;

  default :
    /* No effect */
    CAMERA_IO_Write(DeviceAddr, 0x5001, 0x7F);
    CAMERA_IO_Write(DeviceAddr, 0x5580, 0x00);
    break;
  }
}

/**
  * @brief  Set the OV5640 camera Brightness Level.
  * @note   The brightness of OV5640 could be adjusted. Higher brightness will
  *         make the picture more bright. The side effect of higher brightness
  *         is the picture looks foggy.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  Level: Value to be configured
  * @retval None
  */
void OV5640_SetBrightness(uint16_t DeviceAddr, uint8_t Level)
{
  CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);

  switch(Level)
  {
  case OV5640_BRIGHTNESS_LEVEL4P:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x40);
    break;

  case OV5640_BRIGHTNESS_LEVEL3P:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x30);
    break;

  case OV5640_BRIGHTNESS_LEVEL2P:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x20);
    break;

  case OV5640_BRIGHTNESS_LEVEL1P:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x10);
    break;

  case OV5640_BRIGHTNESS_LEVEL0:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x00);
    break;

  case OV5640_BRIGHTNESS_LEVEL1N:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x10);
    break;

  case OV5640_BRIGHTNESS_LEVEL2N:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x20);
    break;

  case OV5640_BRIGHTNESS_LEVEL3N:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x30);
    break;

  case OV5640_BRIGHTNESS_LEVEL4N:
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x40);
    break;

  default:
    /* Level 0 as default */
    Level = OV5640_BRIGHTNESS_LEVEL0;
    CAMERA_IO_Write(DeviceAddr, 0x5587, 0x00);
    break;
  }

  CAMERA_IO_Write(DeviceAddr, 0x5580, 0x04);

  if(Level < OV5640_SATURATION_LEVEL1N)
  {
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x01);
  }
  else
  {
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x09);
  }
}

/**
  * @brief  Set the OV5640 camera Saturation Level.
  * @note   The color saturation of OV5640 could be adjusted. High color saturation
  *         would make the picture looks more vivid, but the side effect is the
  *         bigger noise and not accurate skin color.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  Level: Value to be configured
  * @retval None
  */
void OV5640_SetSaturation(uint16_t DeviceAddr, uint8_t Level)
{
  CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);

  switch(Level)
  {
  case OV5640_SATURATION_LEVEL4P:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x40);
    break;

  case OV5640_SATURATION_LEVEL3P:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x50);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x50);
    break;

  case OV5640_SATURATION_LEVEL2P:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x60);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x60);
    break;

  case OV5640_SATURATION_LEVEL1P:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x70);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x70);
    break;

  case OV5640_SATURATION_LEVEL0:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x80);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x80);
    break;

  case OV5640_SATURATION_LEVEL1N:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x30);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x30);
    break;

  case OV5640_SATURATION_LEVEL2N:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x20);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x20);
    break;

  case OV5640_SATURATION_LEVEL3N:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x10);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x10);
    break;

  case OV5640_SATURATION_LEVEL4N:
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x00);
    break;

  default:
    /* Level 0 as default */
    CAMERA_IO_Write(DeviceAddr, 0x5583, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5584, 0x40);
    break;
  }

  CAMERA_IO_Write(DeviceAddr, 0x5580, 0x02);
  CAMERA_IO_Write(DeviceAddr, 0x5588, 0x41);
}

/**
  * @brief  Set the OV5640 camera Contrast Level.
  * @note   The contrast of OV5640 could be adjusted. Higher contrast will make
  *         the picture sharp. But the side effect is loosing dynamic range.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  Level: Value to be configured
  * @retval None
  */
void OV5640_SetContrast(uint16_t DeviceAddr, uint8_t Level)
{
  CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
  CAMERA_IO_Write(DeviceAddr, 0x5580, 0x04);

  switch(Level)
  {
  case OV5640_CONTRAST_LEVEL4P:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x30);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x30);
    break;

  case OV5640_CONTRAST_LEVEL3P:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x2C);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x2C);
    break;

  case OV5640_CONTRAST_LEVEL2P:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x28);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x28);
    break;

  case OV5640_CONTRAST_LEVEL1P:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x24);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x24);
    break;

  case OV5640_CONTRAST_LEVEL0:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x20);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x20);
    break;

  case OV5640_CONTRAST_LEVEL1N:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x1C);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x1C);
    break;

  case OV5640_CONTRAST_LEVEL2N:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x18);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x18);
    break;

  case OV5640_CONTRAST_LEVEL3N:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x14);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x14);
    break;

  case OV5640_CONTRAST_LEVEL4N:
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x10);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x10);
    break;

  default:
    /* Level 0 as default */
    CAMERA_IO_Write(DeviceAddr, 0x5586, 0x20);
    CAMERA_IO_Write(DeviceAddr, 0x5585, 0x20);
    break;
  }

  CAMERA_IO_Write(DeviceAddr, 0x5588, 0x41);
}

/**
  * @brief  Set the OV5640 camera Hue degree.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  Level: Value to be configured
  * @retval None
  */
void OV5640_SetHueDegree(uint16_t DeviceAddr, uint16_t Degree)
{
  CAMERA_IO_Write(DeviceAddr, 0x5001, 0xFF);
  CAMERA_IO_Write(DeviceAddr, 0x5580, 0x01);

  switch(Degree)
  {
  case OV5640_HUE_150P:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x6F);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x31);
    break;

  case OV5640_HUE_120P:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x6F);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x31);
    break;

  case OV5640_HUE_90P:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x80);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x31);
    break;

  case OV5640_HUE_60P:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x6F);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x01);
    break;

  case OV5640_HUE_30P:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x6F);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x01);
    break;

  case OV5640_HUE_0:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x80);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x01);
    break;

  case OV5640_HUE_30N:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x6F);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x02);
    break;

  case OV5640_HUE_60N:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x6F);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x02);
    break;

  case OV5640_HUE_90N:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x80);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x02);
    break;

  case OV5640_HUE_120N:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x6F);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x32);
    break;

  case OV5640_HUE_150N:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x6F);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x40);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x32);
    break;

  case OV5640_HUE_180N:
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x80);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x32);
    break;

  default:
    /* Hue degree 0 as default */
    CAMERA_IO_Write(DeviceAddr, 0x5581, 0x80);
    CAMERA_IO_Write(DeviceAddr, 0x5582, 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x5588, 0x01);
    break;
  }
}

/**
  * @brief  Control OV5640 camera mirror/vflip.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  Config: To configure mirror, flip, both or nothing
  * @retval None
  */
void OV5640_MirrorFlipConfig(uint16_t DeviceAddr, uint8_t Config)
{
  uint8_t tmp3820 = 0, tmp3821;

  tmp3820 = CAMERA_IO_Read(DeviceAddr, 0x3820);
  tmp3820 &= 0xF9;
  tmp3821 = CAMERA_IO_Read(DeviceAddr, 0x3821);
  tmp3821 &= 0xF9;

  switch (Config)
  {
  case OV5640_MIRROR:
    CAMERA_IO_Write(DeviceAddr, 0x3820, tmp3820 | 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x3821, tmp3821 | 0x06);
    break;

  case OV5640_FLIP:
    CAMERA_IO_Write(DeviceAddr, 0x3820, tmp3820 | 0x06);
    CAMERA_IO_Write(DeviceAddr, 0x3821, tmp3821 | 0x00);
    break;
  case OV5640_MIRROR_FLIP:
    CAMERA_IO_Write(DeviceAddr, 0x3820, tmp3820 | 0x06);
    CAMERA_IO_Write(DeviceAddr, 0x3821, tmp3821 | 0x06);
    break;

  case OV5640_MIRROR_FLIP_NORMAL:
    CAMERA_IO_Write(DeviceAddr, 0x3820, tmp3820 | 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x3821, tmp3821 | 0x06);
    break;

  default:
    CAMERA_IO_Write(DeviceAddr, 0x3820, tmp3820 | 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x3821, tmp3821 | 0x00);
    break;
  }
}

/**
  * @brief  Control OV5640 camera mirror/vflip.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  Zoom: Zoom to be configured
  * @retval None
  */
void OV5640_ZoomConfig(uint16_t DeviceAddr, uint8_t Zoom)
{
  int32_t res = 0;

  /* Get camera resolution */
  res = OV5640_GetResolution(DeviceAddr);

  if(Zoom == OV5640_ZOOM_x1)
  {
    CAMERA_IO_Write(DeviceAddr, 0x5600, 0x10);
  }
  else
  {
    switch (res)
    {
    case CAMERA_R320x240:
    case CAMERA_R480x272:
      Zoom = Zoom >> 1;
      break;
    case CAMERA_R640x480:
      Zoom = Zoom >> 2;
      break;
    default:
      break;
    }

    CAMERA_IO_Write(DeviceAddr, 0x5600, 0x00);
    CAMERA_IO_Write(DeviceAddr, 0x5601, Zoom);
  }
}

/**
  * @brief  Get OV5640 camera resolution.
  * @param  DeviceAddr: Device address on communication Bus.
  * @retval Camera resolution else 0xFF
  */
int32_t OV5640_GetResolution(uint16_t DeviceAddr)
{
  uint16_t x_size = 0, y_size = 0;
  int32_t res = CAMERA_R640x480;

  x_size = CAMERA_IO_Read(DeviceAddr, 0x3808) << 8;
  x_size |= CAMERA_IO_Read(DeviceAddr, 0x3809);
  y_size = CAMERA_IO_Read(DeviceAddr, 0x380A) << 8;
  y_size |= CAMERA_IO_Read(DeviceAddr, 0x380B);

  if((x_size == 640) && (y_size == 480))
  {
    res = CAMERA_R640x480;
  }
  else if((x_size == 480) && (y_size == 272))
  {
    res = CAMERA_R480x272;
  }
  else if((x_size == 320) && (y_size == 240))
  {
    res = CAMERA_R320x240;
  }
  else if((x_size == 160) && (y_size == 120))
  {
    res = CAMERA_R160x120;
  }
  else
  {
    res = 0xFF;
  }
  return res;
}

/**
  * @brief  Configures the OV5640 camera feature.
  * @param  DeviceAddr: Device address on communication Bus.
  * @param  feature: Camera feature to be configured
  * @param  value: Value to be configured
  * @param  brightness_value: Brightness value to be configured
  * @retval None
  */
void ov5640_Config(uint16_t DeviceAddr, uint32_t feature, uint32_t value, uint32_t brightness_value)
{
  uint32_t value_tmp;
  uint32_t br_value;

  /* Convert the input value into ov5640 parameters */
  value_tmp = ov5640_ConvertValue(feature, value);
  br_value = ov5640_ConvertValue(CAMERA_CONTRAST_BRIGHTNESS, brightness_value);

  switch(feature)
  {
  case CAMERA_CONTRAST_BRIGHTNESS:
    {      
      OV5640_SetContrast(DeviceAddr, value_tmp);
      OV5640_SetBrightness(DeviceAddr, br_value);   
      break;
    }
  case CAMERA_BLACK_WHITE:  
  case CAMERA_COLOR_EFFECT:
    {
      OV5640_SetEffect(DeviceAddr, value_tmp);
      break;
    }
  default:
    {
      break;
    }
  }
}

/**
  * @brief  Read the OV5640 Camera identity.
  * @param  DeviceAddr: Device address on communication Bus.
  * @retval the OV5640 ID
  */
uint16_t ov5640_ReadID(uint16_t DeviceAddr)
{
  uint16_t read_val = 0;

  /* Initialize I2C */
  CAMERA_IO_Init();

  /* Prepare the camera to be configured */
  CAMERA_IO_Write(DeviceAddr, 0x3008, 0x80);
  CAMERA_Delay(500);

  read_val = CAMERA_IO_Read(DeviceAddr, 0x300A);
  read_val = read_val << 8;
  read_val |= CAMERA_IO_Read(DeviceAddr, 0x300B);
  /* Get the camera ID */
  return read_val;
}

/******************************************************************************
                            Static Functions
*******************************************************************************/
/**
  * @brief  Convert input values into ov5640 parameters.
  * @param  feature: Camera feature to be configured
  * @param  value: Value to be configured
  * @retval The converted value
  */
static uint32_t ov5640_ConvertValue(uint32_t feature, uint32_t value)
{
  uint32_t ret = 0;

  switch(feature)
  {
  case CAMERA_BLACK_WHITE:
    {
      switch(value)
      {
      case CAMERA_BLACK_WHITE_BW:
        {
          ret =  OV5640_COLOR_EFFECT_BW;
          break;
        }
      case CAMERA_BLACK_WHITE_NEGATIVE:
        {
          ret =  OV5640_COLOR_EFFECT_NEGATIVE;
          break;
        }
      case CAMERA_BLACK_WHITE_BW_NEGATIVE:
        {
          ret =  OV5640_COLOR_EFFECT_BW_NEGATIVE;
          break;
        }
      case CAMERA_BLACK_WHITE_NORMAL:
      default:
        {
          ret =  OV5640_COLOR_EFFECT_NONE;
          break;
        }
      }
      break;
    }
  case CAMERA_CONTRAST_BRIGHTNESS:
    {
      switch(value)
      {
      case CAMERA_BRIGHTNESS_LEVEL0:
        {
          ret =  OV5640_BRIGHTNESS_LEVEL4N;
          break;
        }
      case CAMERA_BRIGHTNESS_LEVEL1:
        {
          ret =  OV5640_BRIGHTNESS_LEVEL2N;
          break;
        }
      case CAMERA_BRIGHTNESS_LEVEL2:
        {
          ret =  OV5640_BRIGHTNESS_LEVEL0;
          break;
        }
      case CAMERA_BRIGHTNESS_LEVEL3:
        {
          ret =  OV5640_BRIGHTNESS_LEVEL2P;
          break;
        }
      case CAMERA_BRIGHTNESS_LEVEL4:
        {
          ret =  OV5640_BRIGHTNESS_LEVEL4P;
          break;
        }
      case CAMERA_CONTRAST_LEVEL0:
        {
          ret =  OV5640_CONTRAST_LEVEL4N;
          break;
        }
      case CAMERA_CONTRAST_LEVEL1:
        {
          ret =  OV5640_CONTRAST_LEVEL2N;
          break;
        }
      case CAMERA_CONTRAST_LEVEL2:
        {
          ret =  OV5640_CONTRAST_LEVEL0;
          break;
        }
      case CAMERA_CONTRAST_LEVEL3:
        {
          ret =  OV5640_CONTRAST_LEVEL2P;
          break;
        }
      case CAMERA_CONTRAST_LEVEL4:
        {
          ret =  OV5640_CONTRAST_LEVEL4P;
          break;
        }
      default:
        {
          ret =  OV5640_CONTRAST_LEVEL0;
          break;
        }        
      }
      break;
    }
  case CAMERA_COLOR_EFFECT:
    {
      switch(value)
      {
      case CAMERA_COLOR_EFFECT_ANTIQUE:
        {
          ret =  OV5640_COLOR_EFFECT_SEPIA;
          break;
        }
      case CAMERA_COLOR_EFFECT_BLUE:
        {
          ret =  OV5640_COLOR_EFFECT_BLUE;
          break;
        }
      case CAMERA_COLOR_EFFECT_GREEN:
        {
          ret =  OV5640_COLOR_EFFECT_GREEN;
          break;
        }
      case CAMERA_COLOR_EFFECT_RED:
        {
          ret =  OV5640_COLOR_EFFECT_RED;
          break;
        }        
      case CAMERA_COLOR_EFFECT_NONE:  
      default:
        {
          ret =  OV5640_COLOR_EFFECT_NONE;
          break;
        }
      }
      break;
    default:
      {
        ret = 0;
        break;
      }
    }
  }

  return ret;
}

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
