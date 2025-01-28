/**
  ******************************************************************************
  * @file    ov9655.c
  * @author  MCD Application Team
  * @brief   This file provides the OV9655 camera driver
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

/* Includes ------------------------------------------------------------------*/
#include "ov9655.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup OV9655
  * @brief     This file provides a set of functions needed to drive the
  *            OV9655 Camera module.
  * @{
  */

/** @defgroup OV9655_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */
/** @defgroup OV9655_Private_Variables
  * @{
  */

OV9655_CAMERA_Drv_t   OV9655_CAMERA_Driver =
{
  OV9655_Init,
  OV9655_DeInit,
  OV9655_ReadID,
  OV9655_GetCapabilities,
  OV9655_SetLightMode,
  OV9655_SetColorEffect,
  OV9655_SetBrightness,
  OV9655_SetSaturation,
  OV9655_SetContrast,
  OV9655_SetHueDegree,
  OV9655_MirrorFlipConfig,
  OV9655_ZoomConfig,
  OV9655_SetResolution,
  OV9655_GetResolution,
  OV9655_SetPixelFormat,
  OV9655_GetPixelFormat,
  OV9655_NightModeConfig
};

/**
  * @}
  */

/** @defgroup OV9655_Function_Prototypes
  * @{
  */
static int32_t OV9655_ReadRegWrap(void *handle, uint16_t Reg, uint8_t* Data, uint16_t Length);
static int32_t OV9655_WriteRegWrap(void *handle, uint16_t Reg, uint8_t* Data, uint16_t Length);
static int32_t OV9655_Delay(OV9655_Object_t *pObj, uint32_t Delay);

/**
  * @}
  */

/** @defgroup OV9655_Private_Functions OV9655 Private Functions
  * @{
  */
/**
  * @brief  Register component IO bus
  * @param  Component object pointer
  * @retval Component status
  */
int32_t OV9655_RegisterBusIO (OV9655_Object_t *pObj, OV9655_IO_t *pIO)
{
  int32_t ret;

  if (pObj == NULL)
  {
    ret = OV9655_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.GetTick   = pIO->GetTick;

    pObj->Ctx.ReadReg  = OV9655_ReadRegWrap;
    pObj->Ctx.WriteReg = OV9655_WriteRegWrap;
    pObj->Ctx.handle   = pObj;

    if(pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = OV9655_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Initializes the OV9655 CAMERA component.
  * @param  pObj  pointer to component object
  * @param  Resolution  Camera resolution
  * @param  PixelFormat pixel format to be configured
  * @retval Component status
  */
int32_t OV9655_Init(OV9655_Object_t *pObj, uint32_t Resolution, uint32_t PixelFormat)
{
  int32_t ret = OV9655_OK;
  uint8_t tmp;

  if(pObj->IsInitialized == 0U)
  {
    /* Check if resolution is supported */
    if((Resolution > OV9655_R640x480) || ((PixelFormat != OV9655_RGB565) && (PixelFormat != OV9655_YUV422)))
    {
      ret = OV9655_ERROR;
    }
    else
    {
      tmp = 0x80U;
      if(ov9655_write_reg(&pObj->Ctx, OV9655_COMMON_CTRL7, &tmp, 1) != OV9655_OK)
      {
        ret = OV9655_ERROR;
      }

      else
      {
        OV9655_Delay(pObj, 200);
        /* Set specific parameters for each resolution */
        if(OV9655_SetResolution(pObj, Resolution)!= OV9655_OK)
        {
          ret = OV9655_ERROR;
        }/* Set specific parameters for each pixel format */
        else if(OV9655_SetPixelFormat(pObj, PixelFormat)!= OV9655_OK)
        {
          ret = OV9655_ERROR;
        }
        else
        {
          pObj->IsInitialized = 1U;
        }
      }
    }
  }

  return ret;
}

/**
  * @brief  De-initializes the camera sensor.
  * @param  pObj  pointer to component object
  * @retval Component status
  */
int32_t OV9655_DeInit(OV9655_Object_t *pObj)
{
  if(pObj->IsInitialized == 1U)
  {
    pObj->IsInitialized = 0U;
  }

  return OV9655_OK;
}

/**
  * @brief  Set OV9655 camera Pixel Format.
  * @param  pObj  pointer to component object
  * @param  PixelFormat pixel format to be configured
  * @retval Component status
  */
int32_t OV9655_SetPixelFormat(OV9655_Object_t *pObj, uint32_t PixelFormat)
{
  int32_t ret = OV9655_OK;
  uint8_t tmp;

  /* Check if PixelFormat is supported */
  if((PixelFormat != OV9655_RGB565) && (PixelFormat != OV9655_YUV422))
  {
    /* Pixel format not supported */
    ret = OV9655_ERROR;
  }
  else if(ov9655_read_reg(&pObj->Ctx, OV9655_COMMON_CTRL7, &tmp, 1) != OV9655_OK)
  {
    ret = OV9655_ERROR;
  }
  else
  {
    /* Set specific parameters for each PixelFormat */
    tmp &= ~0x03U; /* Reset Bit[0:1] corresponding to pixel format selection */

    switch (PixelFormat)
    {
    case OV9655_YUV422:
      tmp |= 0x02U;
      if(ov9655_write_reg(&pObj->Ctx, OV9655_COMMON_CTRL7, &tmp, 1) != OV9655_OK)
      {
        ret = OV9655_ERROR;
      }
      else
      {
        if(ov9655_read_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1) != OV9655_OK)
        {
          ret = OV9655_ERROR;
        }
        else
        {
          tmp &= ~(1 << 5); /* Clear bit 5: Output bit-wise reverse */
          tmp &= ~(3 << 2); /* Clear bits 3:2: YUV output sequence is 0:yuyv */

          if(ov9655_write_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1) != OV9655_OK)
          {
            ret = OV9655_ERROR;
          }
          else
          {
            if(ov9655_read_reg(&pObj->Ctx, OV9655_COMMON_CTRL15, &tmp, 1) != OV9655_OK)
            {
              ret = OV9655_ERROR;
            }
            else
            {
                tmp |=  (3 << 6); /* Data format - output full range enable 3: [00] to [FF] */
                tmp &= ~(3 << 4); /* Clear bits 5:4: RGB 555/565 option */
            }
            if(ov9655_write_reg(&pObj->Ctx, OV9655_COMMON_CTRL15, &tmp, 1) != OV9655_OK)
            {
              ret = OV9655_ERROR;
            }
          }
        }
      }
      break;
    case OV9655_RGB565:
    default:
      tmp |= 0x03U;
      if(ov9655_write_reg(&pObj->Ctx, OV9655_COMMON_CTRL7, &tmp, 1) != OV9655_OK)
      {
        ret = OV9655_ERROR;
      }
      else if(ov9655_read_reg(&pObj->Ctx, OV9655_COMMON_CTRL15, &tmp, 1) != OV9655_OK)
      {
        ret = OV9655_ERROR;
      }
      else
      {
        tmp |= 0x10U;
        if(ov9655_write_reg(&pObj->Ctx, OV9655_COMMON_CTRL15, &tmp, 1) != OV9655_OK)
        {
          ret = OV9655_ERROR;
        }
      }
      break;
    }
  }

  return ret;
}

/**
  * @brief  Set OV9655 camera Pixel Format.
  * @param  pObj  pointer to component object
  * @param  PixelFormat pixel format to be configured
  * @retval Component status
  */
int32_t OV9655_GetPixelFormat(OV9655_Object_t *pObj, uint32_t *PixelFormat)
{
  (void)(pObj);
  (void)(PixelFormat);

  return OV9655_ERROR;
}

/**
  * @brief  Get OV9655 camera resolution.
  * @param  pObj  pointer to component object
  * @param  Resolution  Camera resolution
  * @retval Component status
  */
int32_t OV9655_SetResolution(OV9655_Object_t *pObj, uint32_t Resolution)
{
  int32_t ret = OV9655_OK;
  uint32_t index;
  uint8_t tmp;

  /* Initialization sequence for VGA resolution (640x480)*/
  static const uint8_t OV9655_VGA[][2]=
  {
    {0x00, 0x00},
    {0x01, 0x80},
    {0x02, 0x80},
    {0xb5, 0x00},
    {0x35, 0x00},
    {0xa8, 0xc1},
    {0x3a, 0xcc},
    {0x3d, 0x99},
    {0x77, 0x02},
    {0x13, 0xe7},
    {0x26, 0x72},
    {0x27, 0x08},
    {0x28, 0x08},
    {0x2c, 0x08},
    {0xab, 0x04},
    {0x6e, 0x00},
    {0x6d, 0x55},
    {0x00, 0x11},
    {0x10, 0x7b},
    {0xbb, 0xae},
    {0x11, 0x03},
    {0x72, 0x00},
    {0x3e, 0x0c},
    {0x74, 0x3a},
    {0x76, 0x01},
    {0x75, 0x35},
    {0x73, 0x00},
    {0xc7, 0x80},
    {0x62, 0x00},
    {0x63, 0x00},
    {0x64, 0x02},
    {0x65, 0x20},
    {0x66, 0x01},
    {0xc3, 0x4e},
    {0x33, 0x00},
    {0xa4, 0x50},
    {0xaa, 0x92},
    {0xc2, 0x01},
    {0xc1, 0xc8},
    {0x1e, 0x04},
    {0xa9, 0xef},
    {0x0e, 0x61},
    {0x39, 0x57},
    {0x0f, 0x48},
    {0x24, 0x3c},
    {0x25, 0x36},
    {0x12, 0x63},
    {OV9655_HORIZONTAL_FRAME_CTRL, 0xff},
    {OV9655_HORIZONTAL_FRAME_START, 0x16},
    {OV9655_HORIZONTAL_FRAME_STOP, 0x02},
    {OV9655_VERTIVCAL_FRAME_CTRL, 0x12},
    {OV9655_VERTICAL_FRAME_START, 0x01},
    {OV9655_VERTICAL_FRAME_STOP, 0x3d},
    {0x36, 0xfa},
    {0x69, 0x0a},
    {0x8c, 0x8d},
    {0xc0, 0xaa},
    {0x40, 0xd0},
    {0x43, 0x14},
    {0x44, 0xf0},
    {0x45, 0x46},
    {0x46, 0x62},
    {0x47, 0x2a},
    {0x48, 0x3c},
    {0x59, 0x85},
    {0x5a, 0xa9},
    {0x5b, 0x64},
    {0x5c, 0x84},
    {0x5d, 0x53},
    {0x5e, 0x0e},
    {0x6c, 0x0c},
    {0xc6, 0x85},
    {0xcb, 0xf0},
    {0xcc, 0xd8},
    {0x71, 0x78},
    {0xa5, 0x68},
    {0x6f, 0x9e},
    {0x42, 0xc0},
    {0x3f, 0x82},
    {0x8a, 0x23},
    {0x14, 0x3a},
    {0x3b, 0xcc},
    {0x34, 0x3d},
    {0x41, 0x40},
    {0xc9, 0xe0},
    {0xca, 0xe8},
    {0xcd, 0x93},
    {0x7a, 0x20},
    {0x7b, 0x1c},
    {0x7c, 0x28},
    {0x7d, 0x3c},
    {0x7e, 0x5a},
    {0x7f, 0x68},
    {0x80, 0x76},
    {0x81, 0x80},
    {0x82, 0x88},
    {0x83, 0x8f},
    {0x84, 0x96},
    {0x85, 0xa3},
    {0x86, 0xaf},
    {0x87, 0xc4},
    {0x88, 0xd7},
    {0x89, 0xe8},
    {0x4f, 0x98},
    {0x50, 0x98},
    {0x51, 0x00},
    {0x52, 0x28},
    {0x53, 0x70},
    {0x54, 0x98},
    {0x58, 0x1a},
    {0x6b, 0x5a},
    {0x90, 0x92},
    {0x91, 0x92},
    {0x9f, 0x90},
    {0xa0, 0x90},
    {0x16, 0x24},
    {0x2a, 0x00},
    {0x2b, 0x00},
    {0xac, 0x80},
    {0xad, 0x80},
    {0xae, 0x80},
    {0xaf, 0x80},
    {0xb2, 0xf2},
    {0xb3, 0x20},
    {0xb4, 0x20},
    {0xb6, 0xaf},
    {0x29, 0x15},
    {0x9d, 0x02},
    {0x9e, 0x02},
    {0x9e, 0x02},
    {0x04, 0x03},
    {0x05, 0x2e},
    {0x06, 0x2e},
    {0x07, 0x2e},
    {0x08, 0x2e},
    {0x2f, 0x2e},
    {0x4a, 0xe9},
    {0x4b, 0xdd},
    {0x4c, 0xdd},
    {0x4d, 0xdd},
    {0x4e, 0xdd},
    {0x70, 0x06},
    {0xa6, 0x40},
    {0xbc, 0x02},
    {0xbd, 0x01},
    {0xbe, 0x02},
    {0xbf, 0x01},
  };

  /* Initialization sequence for QQVGA resolution (160x120) */
  static const uint8_t OV9655_QVGA_QQVGA[][2]=
  {
    {0x00, 0x00},
    {0x01, 0x80},
    {0x02, 0x80},
    {0x04, 0x03},
    {0x09, 0x01},
    {0x0b, 0x57},
    {0x0e, 0x61},
    {0x0f, 0x40},
    {0x11, 0x01},
    {0x12, 0x62},
    {0x13, 0xc7},
    {0x14, 0x3a},
    {0x16, 0x24},
    {OV9655_HORIZONTAL_FRAME_START, 0x18},
    {OV9655_HORIZONTAL_FRAME_STOP, 0x04},
    {OV9655_VERTIVCAL_FRAME_CTRL, 0x02},
    {OV9655_VERTICAL_FRAME_START, 0x01},
    {OV9655_VERTICAL_FRAME_STOP, 0x81},
    {0x1e, 0x00},
    {0x24, 0x3c},
    {0x25, 0x36},
    {0x26, 0x72},
    {0x27, 0x08},
    {0x28, 0x08},
    {0x29, 0x15},
    {0x2a, 0x00},
    {0x2b, 0x00},
    {0x2c, 0x08},
    {0x33, 0x00},
    {0x34, 0x3f},
    {0x35, 0x00},
    {0x36, 0x3a},
    {0x38, 0x72},
    {0x39, 0x57},
    {0x3a, 0xcc},
    {0x3b, 0x04},
    {0x3d, 0x99},
    {0x3f, 0xc1},
    {0x40, 0xc0},
    {0x41, 0x41},
    {0x42, 0xc0},
    {0x43, 0x0a},
    {0x44, 0xf0},
    {0x45, 0x46},
    {0x46, 0x62},
    {0x47, 0x2a},
    {0x48, 0x3c},
    {0x4a, 0xfc},
    {0x4b, 0xfc},
    {0x4c, 0x7f},
    {0x4d, 0x7f},
    {0x4e, 0x7f},
    {0x4f, 0x98},
    {0x50, 0x98},
    {0x51, 0x00},
    {0x52, 0x28},
    {0x53, 0x70},
    {0x54, 0x98},
    {0x58, 0x1a},
    {0x59, 0x85},
    {0x5a, 0xa9},
    {0x5b, 0x64},
    {0x5c, 0x84},
    {0x5d, 0x53},
    {0x5e, 0x0e},
    {0x5f, 0xf0},
    {0x60, 0xf0},
    {0x61, 0xf0},
    {0x62, 0x00},
    {0x63, 0x00},
    {0x64, 0x02},
    {0x65, 0x20},
    {0x66, 0x00},
    {0x69, 0x0a},
    {0x6b, 0x5a},
    {0x6c, 0x04},
    {0x6d, 0x55},
    {0x6e, 0x00},
    {0x6f, 0x9d},
    {0x70, 0x21},
    {0x71, 0x78},
    {0x74, 0x10},
    {0x75, 0x10},
    {0x76, 0x01},
    {0x77, 0x02},
    {0x7A, 0x12},
    {0x7B, 0x08},
    {0x7C, 0x16},
    {0x7D, 0x30},
    {0x7E, 0x5e},
    {0x7F, 0x72},
    {0x80, 0x82},
    {0x81, 0x8e},
    {0x82, 0x9a},
    {0x83, 0xa4},
    {0x84, 0xac},
    {0x85, 0xb8},
    {0x86, 0xc3},
    {0x87, 0xd6},
    {0x88, 0xe6},
    {0x89, 0xf2},
    {0x8a, 0x24},
    {0x8c, 0x80},
    {0x90, 0x7d},
    {0x91, 0x7b},
    {0x9d, 0x02},
    {0x9e, 0x02},
    {0x9f, 0x7a},
    {0xa0, 0x79},
    {0xa1, 0x40},
    {0xa4, 0x50},
    {0xa5, 0x68},
    {0xa6, 0x4a},
    {0xa8, 0xc1},
    {0xa9, 0xef},
    {0xaa, 0x92},
    {0xab, 0x04},
    {0xac, 0x80},
    {0xad, 0x80},
    {0xae, 0x80},
    {0xaf, 0x80},
    {0xb2, 0xf2},
    {0xb3, 0x20},
    {0xb4, 0x20},
    {0xb5, 0x00},
    {0xb6, 0xaf},
    {0xb6, 0xaf},
    {0xbb, 0xae},
    {0xbc, 0x7f},
    {0xbd, 0x7f},
    {0xbe, 0x7f},
    {0xbf, 0x7f},
    {0xbf, 0x7f},
    {0xc0, 0xaa},
    {0xc1, 0xc0},
    {0xc2, 0x01},
    {0xc3, 0x4e},
    {0xc6, 0x05},
    {0xc9, 0xe0},
    {0xca, 0xe8},
    {0xcb, 0xf0},
    {0xcc, 0xd8},
    {0xcd, 0x93},
    {0x12, 0x63},
    {0x40, 0x10},
  };

  /* Initialization sequence for QVGA resolution (320x240) */
  static const uint8_t OV9655_QVGA[][2] =
  {
    {OV9655_HORIZONTAL_FRAME_CTRL, 0x12},
    {0x3e, 0x02},
    {0x72, 0x11},
    {0x73, 0x01},
    {0xc7, 0x81},
  };

  /* Initialization sequence for QQVGA resolution (160x120) */
  static const uint8_t OV9655_QQVGA[][2]=
  {
    {OV9655_HORIZONTAL_FRAME_CTRL, 0xA4},
    {0x3e, 0x0E},
    {0x72, 0x22},
    {0x73, 0x02},
    {0xc7, 0x82},
  };

  /* Check if resolution is supported */
  if (Resolution > OV9655_R640x480)
  {
    ret = OV9655_ERROR;
  }
  else
  {
    /* Initialize OV9655 */
    switch (Resolution)
    {
    case OV9655_R160x120:
      for(index=0; index<(sizeof(OV9655_QVGA_QQVGA)/2U); index++)
      {
        if(ret != OV9655_ERROR)
        {
          tmp = OV9655_QVGA_QQVGA[index][1];
          if(ov9655_write_reg(&pObj->Ctx, OV9655_QVGA_QQVGA[index][0], &tmp, 1) != OV9655_OK)
          {
            ret = OV9655_ERROR;
          }
          (void)OV9655_Delay(pObj, 2);
        }
      }
      for(index=0; index<(sizeof(OV9655_QQVGA)/2U); index++)
      {
        if(ret != OV9655_ERROR)
        {
          tmp = OV9655_QQVGA[index][1];
          if(ov9655_write_reg(&pObj->Ctx, OV9655_QQVGA[index][0], &tmp, 1) != OV9655_OK)
          {
            ret = OV9655_ERROR;
          }
          (void)OV9655_Delay(pObj, 2);
        }
      }
      break;
    case OV9655_R320x240:
      for(index=0; index<(sizeof(OV9655_QVGA_QQVGA)/2U); index++)
      {
        if(ret != OV9655_ERROR)
        {
          tmp = OV9655_QVGA_QQVGA[index][1];
          if(ov9655_write_reg(&pObj->Ctx, OV9655_QVGA_QQVGA[index][0], &tmp, 1) != OV9655_OK)
          {
            ret = OV9655_ERROR;
          }
          (void)OV9655_Delay(pObj, 2);
        }
      }
      for(index=0; index< (sizeof(OV9655_QVGA)/2U); index++)
      {
        if(ret != OV9655_ERROR)
        {
          tmp = OV9655_QVGA[index][1];
          if(ov9655_write_reg(&pObj->Ctx, OV9655_QVGA[index][0], &tmp, 1) != OV9655_OK)
          {
            ret = OV9655_ERROR;
          }
          (void)OV9655_Delay(pObj, 2);
        }
      }
      break;
    case OV9655_R480x272:
    case OV9655_R640x480:
      for(index=0; index<(sizeof(OV9655_VGA)/2U); index++)
      {
        if(ret != OV9655_ERROR)
        {
          tmp = OV9655_VGA[index][1];
          if(ov9655_write_reg(&pObj->Ctx, OV9655_VGA[index][0], &tmp, 1) != OV9655_OK)
          {
            ret = OV9655_ERROR;
          }
          (void)OV9655_Delay(pObj, 2);
        }
      }
      break;
    default:
      ret = OV9655_ERROR;
      break;
    }
  }

  return ret;
}

/**
  * @brief  Get OV9655 camera resolution.
  * @param  pObj  pointer to component object
  * @param  Resolution  Camera resolution
  * @retval Component status
  */
int32_t OV9655_GetResolution(OV9655_Object_t *pObj, uint32_t *Resolution)
{
  int32_t ret = OV9655_OK;
  uint8_t tmp;

  if(ov9655_read_reg(&pObj->Ctx, OV9655_HORIZONTAL_FRAME_CTRL, &tmp, 1) != OV9655_OK)
  {
    ret = OV9655_ERROR;
  }
  else
  {
    switch (tmp)
    {
    case 0xFF:
      *Resolution = OV9655_R640x480;
      break;
    case 0x12:
      *Resolution = OV9655_R320x240;
      break;
    case 0xA4:
      *Resolution = OV9655_R160x120;
      break;
    default:
      ret = OV9655_ERROR;
      break;
    }
  }

  return ret;
}

/**
  * @brief  Read the OV9655 Camera identity.
  * @param  pObj  pointer to component object
  * @param  Id    pointer to component ID
  * @retval Component status
  */
int32_t OV9655_ReadID(OV9655_Object_t *pObj, uint32_t *Id)
{
  int32_t ret;
  uint8_t tmp;

  /* Initialize I2C */
  pObj->IO.Init();

    if(ov9655_read_reg(&pObj->Ctx, OV9655_PID_NUMBER_HIGH, &tmp, 1)!= OV9655_OK)
    {
      ret = OV9655_ERROR;
    }
    else
    {
      *Id = (uint32_t)tmp << 8U;
      if(ov9655_read_reg(&pObj->Ctx, OV9655_PID_NUMBER_LOW, &tmp, 1)!= OV9655_OK)
      {
        ret = OV9655_ERROR;
      }
      else
      {
        *Id |= tmp;
        ret = OV9655_OK;
      }
    }

  /* Component status */
  return ret;
}

/**
  * @brief  Read the OV9655 Camera Capabilities.
  * @param  pObj          pointer to component object
  * @param  Capabilities  pointer to component Capabilities
  * @retval Component status
  */
int32_t OV9655_GetCapabilities(OV9655_Object_t *pObj, OV9655_Capabilities_t *Capabilities)
{
  int32_t ret;

  if(pObj == NULL)
  {
    ret = OV9655_ERROR;
  }
  else
  {
    Capabilities->Config_Brightness    = 1;
    Capabilities->Config_Contrast      = 1;
    Capabilities->Config_HueDegree     = 0;
    Capabilities->Config_LightMode     = 0;
    Capabilities->Config_MirrorFlip    = 1;
    Capabilities->Config_NightMode     = 1;
    Capabilities->Config_Resolution    = 1;
    Capabilities->Config_Saturation    = 0;
    Capabilities->Config_SpecialEffect = 1;
    Capabilities->Config_Zoom          = 0;

    ret = OV9655_OK;
  }

  return ret;
}

/**
  * @brief  Set the OV9655 camera Light Mode.
  * @param  pObj  pointer to component object
  * @param  Effect  Effect to be configured
  * @retval Component status
  */
int32_t OV9655_SetLightMode(OV9655_Object_t *pObj, uint32_t LightMode)
{
  (void)(pObj);
  (void)(LightMode);

  return OV9655_ERROR;
}

/**
  * @brief  Set the OV9655 camera Special Effect.
  * @param  pObj  pointer to component object
  * @param  Effect  Effect to be configured
  * @retval Component status
  */
int32_t OV9655_SetColorEffect(OV9655_Object_t *pObj, uint32_t Effect)
{
  int32_t ret;
  uint8_t tmp;

  switch(Effect)
  {
  case OV9655_COLOR_EFFECT_BLUE:
    tmp = 0xCC;
    ret = ov9655_write_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_1, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_2, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_3, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_4, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_5, &tmp, 1);
    tmp = 0x60;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_6, &tmp, 1);
    break;

  case OV9655_COLOR_EFFECT_RED:
    tmp = 0xCC;
    ret = ov9655_write_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1);
    tmp = 0x60;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_1, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_2, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_3, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_4, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_5, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_6, &tmp, 1);
    break;

  case OV9655_COLOR_EFFECT_GREEN:
    tmp = 0xCC;
    ret = ov9655_write_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_1, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_2, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_3, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_4, &tmp, 1);
    tmp = 0x80;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_5, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_6, &tmp, 1);
    break;

  case OV9655_COLOR_EFFECT_BW:
    tmp = 0xCC;
    ret = ov9655_write_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_1, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_2, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_3, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_4, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_5, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_6, &tmp, 1);
    break;

  case OV9655_COLOR_EFFECT_SEPIA:
    tmp = 0xCC;
    ret = ov9655_write_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_1, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_2, &tmp, 1);
    tmp = 0x20;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_3, &tmp, 1);
    tmp = 0xF0;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_4, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_5, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_6, &tmp, 1);
    break;

  case OV9655_COLOR_EFFECT_NEGATIVE:
    tmp = 0xEC;
    ret = ov9655_write_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1);
    tmp = 0x80;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_1, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_2, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_3, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_4, &tmp, 1);
    tmp = 0x80;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_5, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_6, &tmp, 1);
    break;

  case OV9655_COLOR_EFFECT_NONE:
  default :
    tmp = 0xCC;
    ret = ov9655_write_reg(&pObj->Ctx, OV9655_TSLB, &tmp, 1);
    tmp = 0x80;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_1, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_2, &tmp, 1);
    tmp = 0x00;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_3, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_4, &tmp, 1);
    tmp = 0x80;
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_5, &tmp, 1);
    ret += ov9655_write_reg(&pObj->Ctx, OV9655_MATRIX_COEFFICIENT_6, &tmp, 1);
    break;
  }

  if(ret != OV9655_OK)
  {
    ret = OV9655_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the OV9655 camera Brightness Level.
  * @note   The brightness of OV9655 could be adjusted. Higher brightness will
  *         make the picture more bright. The side effect of higher brightness
  *         is the picture looks foggy.
  * @param  pObj  pointer to component object
  * @param  Level Value to be configured
  * @retval Component status
  */
int32_t OV9655_SetBrightness(OV9655_Object_t *pObj, int32_t Level)
{
  int32_t ret = OV9655_OK;
  const uint8_t brightness_level[] = {0xB0U, 0xB0U, 0xB0U, 0x98U, 0x00U, 0x18U, 0x30U, 0x30U, 0x30U};
  uint8_t tmp;

  tmp = brightness_level[Level + 4];
  if(ov9655_write_reg(&pObj->Ctx, OV9655_BRIGHTNESS_ADJUSTMENT, &tmp, 1) != OV9655_OK)
  {
    ret = OV9655_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the OV9655 camera Saturation Level.
  * @note   The color saturation of OV9655 could be adjusted. High color saturation
  *         would make the picture looks more vivid, but the side effect is the
  *         bigger noise and not accurate skin color.
  * @param  pObj  pointer to component object
  * @param  Level Value to be configured
  * @retval Component status
  */
int32_t OV9655_SetSaturation(OV9655_Object_t *pObj, int32_t Level)
{
  (void)(pObj);
  (void)(Level);

  return OV9655_ERROR;
}

/**
  * @brief  Set the OV9655 camera Contrast Level.
  * @note   The contrast of OV9655 could be adjusted. Higher contrast will make
  *         the picture sharp. But the side effect is loosing dynamic range.
  * @param  pObj  pointer to component object
  * @param  Level Value to be configured
  * @retval Component status
  */
int32_t OV9655_SetContrast(OV9655_Object_t *pObj, int32_t Level)
{
  int32_t ret = OV9655_OK;
  const uint8_t contrast_level[] = {0x30U, 0x30U, 0x30U, 0x38U, 0x40U, 0x50U, 0x60U, 0x60U, 0x60U};
  uint8_t tmp;

  tmp = contrast_level[Level + 4];
  if(ov9655_write_reg(&pObj->Ctx, OV9655_CONTRAST_COEFFICIENT_1, &tmp, 1) != OV9655_OK)
  {
    ret = OV9655_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the OV9655 camera Hue degree.
  * @param  pObj  pointer to component object
  * @param  Level Value to be configured
  * @retval Component status
  */
int32_t OV9655_SetHueDegree(OV9655_Object_t *pObj, int32_t Degree)
{
  (void)(pObj);
  (void)(Degree);

  return OV9655_ERROR;
}

/**
  * @brief  Control OV9655 camera mirror/vflip.
  * @param  pObj  pointer to component object
  * @param  Config To configure mirror, flip, both or none
  * @retval Component status
  */
int32_t OV9655_MirrorFlipConfig(OV9655_Object_t *pObj, uint32_t Config)
{
  int32_t ret = OV9655_OK;
  uint8_t tmp;

  if(Config > OV9655_MIRROR_FLIP)
  {
    ret = OV9655_ERROR;
  }
  else
  {
    if(ov9655_read_reg(&pObj->Ctx, OV9655_MIRROR_VFLIP, &tmp, 1) != OV9655_OK)
    {
      ret = OV9655_ERROR;
    }
    else
    {
      tmp &= ~(3U << 4U);    /* Clear Bit[5:4] Mirror/VFlip */
      tmp |= (Config << 4U); /* Configure Bit[5:4] Mirror/VFlip */
      if(ov9655_write_reg(&pObj->Ctx, OV9655_MIRROR_VFLIP, &tmp, 1) != OV9655_OK)
      {
        ret = OV9655_ERROR;
      }
    }
  }

  return ret;
}

/**
  * @brief  Control OV9655 camera zooming.
  * @param  pObj  pointer to component object
  * @param  Zoom  Zoom to be configured
  * @retval Component status
  */
int32_t OV9655_ZoomConfig(OV9655_Object_t *pObj, uint32_t Zoom)
{
  (void)(pObj);
  (void)(Zoom);

  return OV9655_ERROR;
}

/**
  * @brief  Enable/disable the OV9655 camera night mode.
  * @param  pObj  pointer to component object
  * @param  Cmd   Enable disable night mode
  * @retval Component status
  */
int32_t OV9655_NightModeConfig(OV9655_Object_t *pObj, uint32_t Cmd)
{
  int32_t ret = OV9655_OK;
  uint8_t tmp;

  if(ov9655_read_reg(&pObj->Ctx, OV9655_COMMON_CTRL11, &tmp, 1) != OV9655_OK)
  {
    ret = OV9655_ERROR;
  }
  else
  {
    if(Cmd == NIGHT_MODE_ENABLE)
    {
      tmp |= 0x80U;
    }
    else
    {
      tmp &= 0x7FU;
    }

    if(ov9655_write_reg(&pObj->Ctx, OV9655_COMMON_CTRL11 ,&tmp, 1) != OV9655_OK)
    {
      ret = OV9655_ERROR;
    }
  }

  return ret;
}

/******************** Static functions ****************************************/
/**
  * @brief This function provides accurate delay (in milliseconds)
  * @param pObj   pointer to component object
  * @param Delay  specifies the delay time length, in milliseconds
  * @retval OV9655_OK
  */
static int32_t OV9655_Delay(OV9655_Object_t *pObj, uint32_t Delay)
{
  uint32_t tickstart;
  tickstart = pObj->IO.GetTick();
  while((pObj->IO.GetTick() - tickstart) < Delay)
  {
  }
  return OV9655_OK;
}

/**
  * @brief  Wrap component ReadReg to Bus Read function
  * @param  handle  Component object handle
  * @param  Reg  The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length  buffer size to be written
  * @retval error status
  */
static int32_t OV9655_ReadRegWrap(void *handle, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
  OV9655_Object_t *pObj = (OV9655_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Wrap component WriteReg to Bus Write function
  * @param  handle  Component object handle
  * @param  Reg  The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length  buffer size to be written
  * @retval error status
  */
static int32_t OV9655_WriteRegWrap(void *handle, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
  OV9655_Object_t *pObj = (OV9655_Object_t *)handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
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
