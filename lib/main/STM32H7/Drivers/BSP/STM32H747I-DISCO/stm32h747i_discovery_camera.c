/**
  ******************************************************************************
  * @file    stm32h747i_discovery_camera.c
  * @author  MCD Application Team
  * @brief   This file includes the driver for Camera modules mounted on
  *          STM32H747I_DISCO board.
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

/* File Info: ------------------------------------------------------------------
                                   User NOTES
1. How to use this driver:
--------------------------
   - This driver is used to drive the camera.
   - The   or OV9655 component driver MUST be included with this driver.

2. Driver description:
---------------------
     o Initialize the camera instance using the BSP_CAMERA_Init() function with the required
       Resolution and Pixel format where:
       - Instance: Is the physical camera interface and is always 0 on this board.
       - Resolution: The camera resolution
       - PixelFormat: The camera Pixel format

     o DeInitialize the camera instance using the BSP_CAMERA_Init() . This
       function will firstly stop the camera to insure the data transfer complete.
       Then de-initializes the dcmi peripheral.

     o Get the camera instance capabilities using the BSP_CAMERA_GetCapabilities().
       This function must be called after the BSP_CAMERA_Init() to get the right
       sensor capabilities

     o Start the camera using the CAMERA_Start() function by specifying the capture Mode:
       - CAMERA_MODE_CONTINUOUS: For continuous capture
       - CAMERA_MODE_SNAPSHOT  : For on shot capture

     o Suspend, resume or stop the camera capture using the following functions:
      - BSP_CAMERA_Suspend()
      - BSP_CAMERA_Resume()
      - BSP_CAMERA_Stop()

     o Call BSP_CAMERA_SetResolution()/BSP_CAMERA_GetResolution() to set/get the camera resolution
       Resolution: - CAMERA_R160x120
                   - CAMERA_R320x240
                   - CAMERA_R480x272
                   - CAMERA_R640x480
                   - CAMERA_R800x480

     o Call BSP_CAMERA_SetPixelFormat()/BSP_CAMERA_GetPixelFormat() to set/get the camera pixel format
       PixelFormat: - CAMERA_PF_RGB565
                    - CAMERA_PF_RGB888
                    - CAMERA_PF_YUV422

     o Call BSP_CAMERA_SetLightMode()/BSP_CAMERA_GetLightMode() to set/get the camera light mode
       LightMode: - CAMERA_LIGHT_AUTO
                  - CAMERA_LIGHT_SUNNY
                  - CAMERA_LIGHT_OFFICE
                  - CAMERA_LIGHT_HOME
                  - CAMERA_LIGHT_CLOUDY

     o Call BSP_CAMERA_SetColorEffect()/BSP_CAMERA_GetColorEffect() to set/get the camera color effects
       Effect: - CAMERA_COLOR_EFFECT_NONE
               - CAMERA_COLOR_EFFECT_BLUE
               - CAMERA_COLOR_EFFECT_RED
               - CAMERA_COLOR_EFFECT_GREEN
               - CAMERA_COLOR_EFFECT_BW
               - CAMERA_COLOR_EFFECT_SEPIA
               - CAMERA_COLOR_EFFECT_NEGATIVE

     o Call BSP_CAMERA_SetBrightness()/BSP_CAMERA_GetBrightness() to set/get the camera Brightness
       Brightness is value between -4(Level 4 negative) and 4(Level 4 positive).

     o Call BSP_CAMERA_SetSaturation()/BSP_CAMERA_GetSaturation() to set/get the camera Saturation
       Saturation is value between -4(Level 4 negative) and 4(Level 4 positive).

     o Call BSP_CAMERA_SetContrast()/BSP_CAMERA_GetContrast() to set/get the camera Contrast
       Contrast is value between -4(Level 4 negative) and 4(Level 4 positive).

     o Call BSP_CAMERA_SetHueDegree()/BSP_CAMERA_GetHueDegree() to set/get the camera Hue Degree
       HueDegree is value between -4(180 degree negative) and 4(150 degree positive).

     o Call BSP_CAMERA_SetMirrorFlip()/BSP_CAMERA_GetMirrorFlip() to set/get the camera mirror and flip
       MirrorFlip could be any combination of: - CAMERA_MIRRORFLIP_NONE
                                               - CAMERA_MIRRORFLIP_FLIP
                                               - CAMERA_MIRRORFLIP_MIRROR
       Note that This feature is only supported with   sensor.

     o Call BSP_CAMERA_SetZoom()/BSP_CAMERA_GetZoom() to set/get the camera zooming
       Zoom is supported only with   sensor could be any value of:
       - CAMERA_ZOOM_x8 for CAMERA_R160x120 resolution only
       - CAMERA_ZOOM_x4 For all resolutions except CAMERA_R640x480 and CAMERA_R800x480
       - CAMERA_ZOOM_x2 For all resolutions except CAMERA_R800x480
       - CAMERA_ZOOM_x1 For all resolutions

     o Call BSP_CAMERA_EnableNightMode() to enable night mode. This feature is only supported
       with   sensor

     o Call BSP_CAMERA_DisableNightMode() to disable night mode. This feature is only supported
       with   sensor

    o Call BSP_CAMERA_RegisterDefaultMspCallbacks() to register Msp default callbacks
    o Call BSP_CAMERA_RegisterMspCallbacks() to register application Msp callbacks.

    o Error, line event, vsync event and frame event are handled through dedicated weak
      callbacks that can be override at application level: BSP_CAMERA_LineEventCallback(),
      BSP_CAMERA_FrameEventCallback(), BSP_CAMERA_VsyncEventCallback(), BSP_CAMERA_ErrorCallback()

  Known Limitations:
  ------------------
   1- CAMERA_PF_RGB888 resolution is not supported with   sensor.
   2- The following feature are only supported through   sensor:
      o LightMode setting
      o Saturation setting
      o HueDegree setting
      o Mirror/Flip setting
      o Zoom setting
      o NightMode enable/disable
------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "stm32h747i_discovery_camera.h"
#include "stm32h747i_discovery_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H747I_DISCO
  * @{
  */

/** @addtogroup STM32H747I_DISCO_CAMERA
  * @{
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Exported_Variables Exported Variables
  * @{
  */
void                *Camera_CompObj = NULL;
DCMI_HandleTypeDef  hcamera_dcmi;
CAMERA_Ctx_t        Camera_Ctx[CAMERA_INSTANCES_NBR];
/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Private_Variables Private Variables
  * @{
  */
static CAMERA_Drv_t *Camera_Drv = NULL;
static CAMERA_Capabilities_t Camera_Cap;
static uint32_t HSPolarity = DCMI_HSPOLARITY_LOW;
static uint32_t CameraId;
/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Private_FunctionsPrototypes Private Functions Prototypes
  * @{
  */
static int32_t GetSize(uint32_t Resolution, uint32_t PixelFormat);
static void DCMI_MspInit(DCMI_HandleTypeDef *hdcmi);
static void DCMI_MspDeInit(DCMI_HandleTypeDef *hdcmi);
#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 1)
static void DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi);
static void DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi);
static void DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi);
static void DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi);
#endif /* (USE_HAL_DCMI_REGISTER_CALLBACKS == 1) */

#if (USE_CAMERA_SENSOR_OV9655 == 1)
static int32_t OV9655_Probe(uint32_t Resolution, uint32_t PixelFormat);
#endif

#if (USE_CAMERA_SENSOR_OV5640 == 1)
static int32_t OV5640_Probe(uint32_t Resolution, uint32_t PixelFormat);
#endif
/**
  * @}
  */
#define CAMERA_VGA_RES_X          640
#define CAMERA_VGA_RES_Y          480
#define CAMERA_480x272_RES_X      480
#define CAMERA_480x272_RES_Y      272

/** @defgroup STM32H747I_DISCO_CAMERA_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  Initializes the camera.
  * @param  Instance    Camera instance.
  * @param  Resolution  Camera sensor requested resolution (x, y) : standard resolution
  *         naming QQVGA, QVGA, VGA ...
  * @param  PixelFormat Capture pixel format
  * @retval BSP status
  */
int32_t BSP_CAMERA_Init(uint32_t Instance, uint32_t Resolution, uint32_t PixelFormat)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* DCMI Initialization */
#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 1)
    /* Register the DCMI MSP Callbacks */
    if(Camera_Ctx[Instance].IsMspCallbacksValid == 0U)
    {
      if(BSP_CAMERA_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#else
    /* DCMI Initialization */
    DCMI_MspInit(&hcamera_dcmi);
#endif
    /* Initialize the camera driver structure */
    if(MX_DCMI_Init(&hcamera_dcmi) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(BSP_CAMERA_HwReset(0) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_BUS_FAILURE;
    }
    else
    {
#if (USE_CAMERA_SENSOR_OV9655 == 1)
      ret= OV9655_Probe(Resolution, PixelFormat);
#endif
#if (USE_CAMERA_SENSOR_OV5640 == 1)
      if(ret != BSP_ERROR_NONE)
      {
        ret = OV5640_Probe(Resolution, PixelFormat);
      }
#endif
      if(ret != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_UNKNOWN_COMPONENT;
      }
      else
      {
        if((CameraId == OV9655_ID) || (CameraId == OV9655_ID_2))
        {
          if(Resolution == CAMERA_R480x272)
          {
            if(HAL_DCMI_ConfigCROP(&hcamera_dcmi,           /* Crop in the middle of the VGA picture */
                                   (CAMERA_VGA_RES_X - CAMERA_480x272_RES_X)/2,
                                   (CAMERA_VGA_RES_Y - CAMERA_480x272_RES_Y)/2,
                                   (CAMERA_480x272_RES_X * 2) - 1,
                                   CAMERA_480x272_RES_Y - 1) != HAL_OK)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
            else
            {
              if(HAL_DCMI_EnableCROP(&hcamera_dcmi) != HAL_OK)
              {
                ret = BSP_ERROR_PERIPH_FAILURE;
              }
            }
          }
        }
        else
        {
          HSPolarity = DCMI_HSPOLARITY_HIGH;
          /* Initialize the camera driver structure */
          if(MX_DCMI_Init(&hcamera_dcmi) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
        }

        if(ret == BSP_ERROR_NONE)
        {
          if(BSP_CAMERA_HwReset(0) != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_BUS_FAILURE;
          }
        }

        if(ret == BSP_ERROR_NONE)
        {
          Camera_Ctx[Instance].CameraId  = CameraId;
#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 1)
          /* Register DCMI LineEvent, FrameEvent and Error callbacks */
          if(HAL_DCMI_RegisterCallback(&hcamera_dcmi, HAL_DCMI_LINE_EVENT_CB_ID, DCMI_LineEventCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else if(HAL_DCMI_RegisterCallback(&hcamera_dcmi, HAL_DCMI_FRAME_EVENT_CB_ID, DCMI_FrameEventCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else if(HAL_DCMI_RegisterCallback(&hcamera_dcmi, HAL_DCMI_VSYNC_EVENT_CB_ID, DCMI_VsyncEventCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else if(HAL_DCMI_RegisterCallback(&hcamera_dcmi, HAL_DCMI_ERROR_CB_ID, DCMI_ErrorCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else
          {
            ret = BSP_ERROR_NONE;
          }
#endif /* (USE_HAL_DCMI_REGISTER_CALLBACKS == 1) */
        }
        if(ret == BSP_ERROR_NONE)
        {
          Camera_Ctx[Instance].Resolution  = Resolution;
          Camera_Ctx[Instance].PixelFormat = PixelFormat;
        }
      }
    }
  }

  /* BSP status */
  return ret;
}
/**
  * @brief  DeInitializes the camera.
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_DeInit(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if((Camera_CompObj == NULL)||(Camera_Drv == NULL))
    {
      ret = BSP_ERROR_NO_INIT;
    }
    else
    {
      hcamera_dcmi.Instance = DCMI;

      /* First stop the camera to insure all data are transferred */
      if(BSP_CAMERA_Stop(Instance) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_DCMI_DisableCROP(&hcamera_dcmi)!= HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_DCMI_DeInit(&hcamera_dcmi) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 0)
        DCMI_MspDeInit(&hcamera_dcmi);
#endif /* (USE_HAL_DCMI_REGISTER_CALLBACKS == 0) */

        if(Camera_Drv->DeInit(Camera_CompObj) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }/* Set Camera in Power Down */
        else if(BSP_CAMERA_PwrDown(0) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_BUS_FAILURE;
        }
        else
        {
          ret = BSP_ERROR_NONE;
        }
      }
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Initializes the DCMI peripheral
  * @param  hdcmi  DCMI handle
  * @note   Being __weak it can be overwritten by the application
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_DCMI_Init(DCMI_HandleTypeDef* hdcmi)
{
  /*** Configures the DCMI to interface with the camera module ***/
  /* DCMI configuration */
  hdcmi->Instance              = DCMI;
  hdcmi->Init.CaptureRate      = DCMI_CR_ALL_FRAME;
  hdcmi->Init.HSPolarity       = HSPolarity;
  hdcmi->Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
  hdcmi->Init.VSPolarity       = DCMI_VSPOLARITY_HIGH;
  hdcmi->Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  hdcmi->Init.PCKPolarity      = DCMI_PCKPOLARITY_RISING;

  if(HAL_DCMI_Init(hdcmi) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}

#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP CAMERA Msp Callbacks
  * @param Instance CAMERA Instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_RegisterDefaultMspCallbacks (uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_DCMI_RESET_HANDLE_STATE(&hcamera_dcmi);

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_DCMI_RegisterCallback(&hcamera_dcmi, HAL_DCMI_MSPINIT_CB_ID, DCMI_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_DCMI_RegisterCallback(&hcamera_dcmi, HAL_DCMI_MSPDEINIT_CB_ID, DCMI_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Camera_Ctx[Instance].IsMspCallbacksValid = 1;
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP CAMERA Msp Callback registering
  * @param Instance     CAMERA Instance.
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_CAMERA_RegisterMspCallbacks(uint32_t Instance, BSP_CAMERA_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_DCMI_RESET_HANDLE_STATE(&hcamera_dcmi);

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_DCMI_RegisterCallback(&hcamera_dcmi, HAL_DCMI_MSPINIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_DCMI_RegisterCallback(&hcamera_dcmi, HAL_DCMI_MSPDEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Camera_Ctx[Instance].IsMspCallbacksValid = 1;
    }
  }
  /* Return BSP status */
  return ret;
}
#endif /* (USE_HAL_DCMI_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Starts the camera capture in continuous mode.
  * @param  Instance Camera instance.
  * @param  pBff     pointer to the camera output buffer
  * @param  Mode CAMERA_MODE_CONTINUOUS or CAMERA_MODE_SNAPSHOT
  * @retval BSP status
  */
int32_t BSP_CAMERA_Start(uint32_t Instance, uint8_t *pBff, uint32_t Mode)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_DCMI_Start_DMA(&hcamera_dcmi, Mode, (uint32_t)pBff, (uint32_t)GetSize(Camera_Ctx[Instance].Resolution, Camera_Ctx[Instance].PixelFormat)) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Stop the CAMERA capture
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_Stop(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_DCMI_Stop(&hcamera_dcmi) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief Suspend the CAMERA capture
  * @param  Instance Camera instance.
  */
int32_t BSP_CAMERA_Suspend(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_DCMI_Suspend(&hcamera_dcmi) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief Resume the CAMERA capture
  * @param  Instance Camera instance.
  */
int32_t BSP_CAMERA_Resume(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(HAL_DCMI_Resume(&hcamera_dcmi) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the Camera Capabilities.
  * @param  Instance  Camera instance.
  * @param  Capabilities  pointer to camera Capabilities
  * @note   This function should be called after the init. This to get Capabilities
  *         from the right camera sensor(OV9655)
  * @retval Component status
  */
int32_t BSP_CAMERA_GetCapabilities(uint32_t Instance, CAMERA_Capabilities_t *Capabilities)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Drv->GetCapabilities(Camera_CompObj, Capabilities) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera pixel format.
  * @param  Instance  Camera instance.
  * @param  PixelFormat pixel format to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetPixelFormat(uint32_t Instance, uint32_t PixelFormat)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Drv->SetPixelFormat(Camera_CompObj, PixelFormat) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].PixelFormat = PixelFormat;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera pixel format.
  * @param  Instance  Camera instance.
  * @param  PixelFormat pixel format to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetPixelFormat(uint32_t Instance, uint32_t *PixelFormat)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    *PixelFormat = Camera_Ctx[Instance].PixelFormat;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}


/**
  * @brief  Set the camera Resolution.
  * @param  Instance  Camera instance.
  * @param  Resolution Resolution to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetResolution(uint32_t Instance, uint32_t Resolution)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Resolution == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetResolution(Camera_CompObj, Resolution) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Resolution = Resolution;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Resolution.
  * @param  Instance  Camera instance.
  * @param  Resolution Resolution to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetResolution(uint32_t Instance, uint32_t *Resolution)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Resolution == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Resolution = Camera_Ctx[Instance].Resolution;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Light Mode.
  * @param  Instance  Camera instance.
  * @param  LightMode Light Mode to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetLightMode(uint32_t Instance, uint32_t LightMode)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.LightMode == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetLightMode(Camera_CompObj, LightMode) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].LightMode = LightMode;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Light Mode.
  * @param  Instance  Camera instance.
  * @param  LightMode Light Mode to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetLightMode(uint32_t Instance, uint32_t *LightMode)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.LightMode == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *LightMode = Camera_Ctx[Instance].LightMode;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Special Effect.
  * @param  Instance Camera instance.
  * @param  ColorEffect Effect to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetColorEffect(uint32_t Instance, uint32_t ColorEffect)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.ColorEffect == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetColorEffect(Camera_CompObj, ColorEffect) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].ColorEffect = ColorEffect;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Special Effect.
  * @param  Instance Camera instance.
  * @param  ColorEffect Effect to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetColorEffect(uint32_t Instance, uint32_t *ColorEffect)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.ColorEffect == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *ColorEffect = Camera_Ctx[Instance].ColorEffect;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Brightness Level.
  * @param  Instance   Camera instance.
  * @param  Brightness Brightness Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetBrightness(uint32_t Instance, int32_t Brightness)
{
  int32_t ret;

  if((Instance >= CAMERA_INSTANCES_NBR) || ((Brightness < CAMERA_BRIGHTNESS_MIN) && (Brightness > CAMERA_BRIGHTNESS_MAX)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Brightness == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetBrightness(Camera_CompObj, Brightness) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Brightness = Brightness;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Brightness Level.
  * @param  Instance Camera instance.
  * @param  Brightness  Brightness Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetBrightness(uint32_t Instance, int32_t *Brightness)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Brightness == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Brightness = Camera_Ctx[Instance].Brightness;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Saturation Level.
  * @param  Instance    Camera instance.
  * @param  Saturation  Saturation Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetSaturation(uint32_t Instance, int32_t Saturation)
{
  int32_t ret;

  if((Instance >= CAMERA_INSTANCES_NBR) || ((Saturation < CAMERA_SATURATION_MIN) && (Saturation > CAMERA_SATURATION_MAX)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Saturation == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetSaturation(Camera_CompObj, Saturation)  < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Saturation = Saturation;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Saturation Level.
  * @param  Instance    Camera instance.
  * @param  Saturation  Saturation Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetSaturation(uint32_t Instance, int32_t *Saturation)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Saturation == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Saturation = Camera_Ctx[Instance].Saturation;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Contrast Level.
  * @param  Instance Camera instance.
  * @param  Contrast Contrast Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetContrast(uint32_t Instance, int32_t Contrast)
{
  int32_t ret;

  if((Instance >= CAMERA_INSTANCES_NBR) || ((Contrast < CAMERA_CONTRAST_MIN) && (Contrast > CAMERA_CONTRAST_MAX)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Contrast == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetContrast(Camera_CompObj, Contrast)  < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Contrast = Contrast;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Contrast Level.
  * @param  Instance Camera instance.
  * @param  Contrast Contrast Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetContrast(uint32_t Instance, int32_t *Contrast)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Contrast == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Contrast = Camera_Ctx[Instance].Contrast;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Hue Degree.
  * @param  Instance   Camera instance.
  * @param  HueDegree  Hue Degree
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetHueDegree(uint32_t Instance, int32_t HueDegree)
{
  int32_t ret;

  if((Instance >= CAMERA_INSTANCES_NBR) || ((HueDegree < CAMERA_HUEDEGREE_MIN) && (HueDegree > CAMERA_HUEDEGREE_MAX)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.HueDegree == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetHueDegree(Camera_CompObj, HueDegree) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].HueDegree = HueDegree;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Hue Degree.
  * @param  Instance   Camera instance.
  * @param  HueDegree  Hue Degree
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetHueDegree(uint32_t Instance, int32_t *HueDegree)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.HueDegree == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *HueDegree = Camera_Ctx[Instance].HueDegree;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Mirror/Flip.
  * @param  Instance  Camera instance.
  * @param  MirrorFlip CAMERA_MIRRORFLIP_NONE or any combination of
  *                    CAMERA_MIRRORFLIP_FLIP and CAMERA_MIRRORFLIP_MIRROR
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetMirrorFlip(uint32_t Instance, uint32_t MirrorFlip)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.MirrorFlip == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->MirrorFlipConfig(Camera_CompObj, MirrorFlip)  < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].MirrorFlip = MirrorFlip;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Mirror/Flip.
  * @param  Instance   Camera instance.
  * @param  MirrorFlip Mirror/Flip config
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetMirrorFlip(uint32_t Instance, uint32_t *MirrorFlip)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.MirrorFlip == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *MirrorFlip = Camera_Ctx[Instance].MirrorFlip;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera zoom
  * @param  Instance Camera instance.
  * @param  Zoom     Zoom to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetZoom(uint32_t Instance, uint32_t Zoom)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Zoom == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->ZoomConfig(Camera_CompObj, Zoom) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Zoom = Zoom;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera zoom
  * @param  Instance Camera instance.
  * @param  Zoom     Zoom to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetZoom(uint32_t Instance, uint32_t *Zoom)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Zoom == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Zoom = Camera_Ctx[Instance].Zoom;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Enable the camera night mode
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_EnableNightMode(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.NightMode == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->NightModeConfig(Camera_CompObj, CAMERA_NIGHT_MODE_SET) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Disable the camera night mode
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_DisableNightMode(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.NightMode == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->NightModeConfig(Camera_CompObj, CAMERA_NIGHT_MODE_RESET) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  CAMERA hardware reset
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_HwReset(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef gpio_init_structure;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Init DCMI PWR_ENABLE Pin */
    /* Enable GPIO clock */
    __HAL_RCC_GPIOJ_CLK_ENABLE();

    gpio_init_structure.Pin       = GPIO_PIN_14;
    gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

    /* De-assert the camera POWER_DOWN pin (active high) */
    HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14, GPIO_PIN_SET);

    HAL_Delay(100);     /* POWER_DOWN de-asserted during 100 ms */

    /* Assert the camera POWER_DOWN pin (active high) */
    HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_Delay(20);
  }

  return ret;
}

/**
  * @brief  CAMERA power down
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_PwrDown(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef gpio_init_structure;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Init DCMI PWR_ENABLE Pin */
    /* Enable GPIO clock */
    __HAL_RCC_GPIOJ_CLK_ENABLE();

    gpio_init_structure.Pin       = GPIO_PIN_14;
    gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

    /* Assert the camera POWER_DOWN pin (active high) */
    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_14, GPIO_PIN_SET);
  }

  return ret;
}

/**
  * @brief  CAMERA power up
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_PwrUp(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef gpio_init_structure;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Init DCMI PWR_ENABLE Pin */
    /* Enable GPIO clock */
    __HAL_RCC_GPIOJ_CLK_ENABLE();

    gpio_init_structure.Pin       = GPIO_PIN_14;
    gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

    /* Assert the camera POWER_DOWN pin (active high) */
    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_14, GPIO_PIN_RESET);
  }

  return ret;
}
/**
  * @brief  This function handles DCMI interrupt request.
  * @param  Instance Camera instance
  * @retval None
  */
void BSP_CAMERA_IRQHandler(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  HAL_DCMI_IRQHandler(&hcamera_dcmi);
}

/**
  * @brief  This function handles DCMI DMA interrupt request.
  * @param  Instance Camera instance
  * @retval None
  */
void BSP_CAMERA_DMA_IRQHandler(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  HAL_DMA_IRQHandler(hcamera_dcmi.DMA_Handle);
}

/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Private_Functions Private Functions
  * @{
  */

/**
  * @brief  Get the capture size in pixels unit.
  * @param  Resolution  the current resolution.
  * @param  PixelFormat Camera pixel format
  * @retval capture size in 32-bit words.
  */
static int32_t GetSize(uint32_t Resolution, uint32_t PixelFormat)
{
  uint32_t size = 0;
  uint32_t pf_div;
  if(PixelFormat == CAMERA_PF_RGB888)
  {
    pf_div = 3; /* each pixel on 3 bytes so 3/4 words */
  }
  else
  {
    pf_div = 2; /* each pixel on 2 bytes so 1/2 words*/
  }
  /* Get capture size */
  switch (Resolution)
  {
  case CAMERA_R160x120:
    size =  ((uint32_t)(160*120)*pf_div)/4U;
    break;
  case CAMERA_R320x240:
    size =  ((uint32_t)(320*240)*pf_div)/4U;
    break;
  case CAMERA_R480x272:
    size =  ((uint32_t)(480*272)*pf_div)/4U;
    break;
  case CAMERA_R640x480:
    size =  ((uint32_t)(640*480)*pf_div)/4U;
    break;
  case CAMERA_R800x480:
    size =  ((uint32_t)(800*480)*pf_div)/4U;
    break;
  default:
    break;
  }

  return (int32_t)size;
}

/**
  * @brief  Initializes the DCMI MSP.
  * @param  hdcmi  DCMI handle
  * @retval None
  */
static void DCMI_MspInit(DCMI_HandleTypeDef *hdcmi)
{
  static DMA_HandleTypeDef hdma_handler;
  GPIO_InitTypeDef gpio_init_structure;

  /*** Enable peripherals and GPIO clocks ***/
  /* Enable DCMI clock */
  __HAL_RCC_DCMI_CLK_ENABLE();

  /* Enable DMA2 clock */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* Enable GPIO clocks */

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  gpio_init_structure.Pin       = GPIO_PIN_4 | GPIO_PIN_6;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &gpio_init_structure);

  gpio_init_structure.Pin       = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOB, &gpio_init_structure);

  gpio_init_structure.Pin       = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_11;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOC, &gpio_init_structure);

  gpio_init_structure.Pin       = GPIO_PIN_3;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  gpio_init_structure.Pin       = GPIO_PIN_10;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);

  /*** Configure the DMA ***/
  /* Set the parameters to be configured */
  hdma_handler.Init.Request             = DMA_REQUEST_DCMI;
  hdma_handler.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_handler.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_handler.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_handler.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
  hdma_handler.Init.Mode                = DMA_CIRCULAR;
  hdma_handler.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma_handler.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
  hdma_handler.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_handler.Init.MemBurst            = DMA_MBURST_SINGLE;
  hdma_handler.Init.PeriphBurst         = DMA_PBURST_SINGLE;
  hdma_handler.Instance                 = DMA2_Stream3;

  /* Associate the initialized DMA handle to the DCMI handle */
  __HAL_LINKDMA(hdcmi, DMA_Handle, hdma_handler);

  /*** Configure the NVIC for DCMI and DMA ***/
  /* NVIC configuration for DCMI transfer complete interrupt */
  HAL_NVIC_SetPriority(DCMI_IRQn, BSP_CAMERA_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(DCMI_IRQn);

  /* NVIC configuration for DMA2D transfer complete interrupt */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, BSP_CAMERA_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

  /* Configure the DMA stream */
  (void)HAL_DMA_Init(hdcmi->DMA_Handle);
}

/**
  * @brief  DeInitializes the DCMI MSP.
  * @param  hdcmi  DCMI handle
  * @retval None
  */
static void DCMI_MspDeInit(DCMI_HandleTypeDef *hdcmi)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Disable NVIC  for DCMI transfer complete interrupt */
  HAL_NVIC_DisableIRQ(DCMI_IRQn);

  /* Disable NVIC for DMA2 transfer complete interrupt */
  HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);

  /* Configure the DMA stream */
  (void)HAL_DMA_DeInit(hdcmi->DMA_Handle);

  /* DeInit DCMI GPIOs */
  gpio_init_structure.Pin       = GPIO_PIN_4 | GPIO_PIN_6;
  HAL_GPIO_DeInit(GPIOA, gpio_init_structure.Pin);

  gpio_init_structure.Pin       = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
  HAL_GPIO_DeInit(GPIOB, gpio_init_structure.Pin);

  gpio_init_structure.Pin       = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_11;
  HAL_GPIO_DeInit(GPIOC, gpio_init_structure.Pin);

  gpio_init_structure.Pin       = GPIO_PIN_3;
  HAL_GPIO_DeInit(GPIOD, gpio_init_structure.Pin);

  gpio_init_structure.Pin       = GPIO_PIN_10;
  HAL_GPIO_DeInit(GPIOG, gpio_init_structure.Pin);



  /* Disable DCMI clock */
  __HAL_RCC_DCMI_CLK_DISABLE();
}

#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 0)
/**
  * @brief  Line event callback
  * @param  hdcmi  pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  BSP_CAMERA_LineEventCallback(0);
}

/**
  * @brief  Frame event callback
  * @param  hdcmi pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  BSP_CAMERA_FrameEventCallback(0);
}

/**
  * @brief  Vsync event callback
  * @param  hdcmi pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  BSP_CAMERA_VsyncEventCallback(0);
}

/**
  * @brief  Error callback
  * @param  hdcmi pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  BSP_CAMERA_ErrorCallback(0);
}
#else
/**
  * @brief  Line event callback
  * @param  hdcmi  pointer to the DCMI handle
  * @retval None
  */
static void DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  BSP_CAMERA_LineEventCallback(0);
}

/**
  * @brief  Frame event callback
  * @param  hdcmi pointer to the DCMI handle
  * @retval None
  */
static void DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  BSP_CAMERA_FrameEventCallback(0);
}

/**
  * @brief  Vsync event callback
  * @param  hdcmi  pointer to the DCMI handle
  * @retval None
  */
static void DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  BSP_CAMERA_VsyncEventCallback(0);
}

/**
  * @brief  Error callback
  * @param  hdcmi pointer to the DCMI handle
  * @retval None
  */
static void DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  BSP_CAMERA_ErrorCallback(0);
}
#endif /* (USE_HAL_DCMI_REGISTER_CALLBACKS == 0) */

/**
  * @brief  Line Event callback.
  * @param  Instance Camera instance.
  * @retval None
  */
__weak void BSP_CAMERA_LineEventCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_LineEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Frame Event callback.
  * @param  Instance Camera instance.
  * @retval None
  */
__weak void BSP_CAMERA_FrameEventCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_FrameEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Vsync Event callback.
  * @param  Instance Camera instance.
  * @retval None
  */
__weak void BSP_CAMERA_VsyncEventCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_FrameEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Error callback.
  * @param  Instance Camera instance.
  * @retval None
  */
__weak void BSP_CAMERA_ErrorCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_ErrorCallback could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Private_Functions Private Functions
  * @{
  */

#if (USE_CAMERA_SENSOR_OV9655 == 1)
/**
  * @brief  Register Bus IOs if component ID is OK
  * @retval error status
  */
static int32_t OV9655_Probe(uint32_t Resolution, uint32_t PixelFormat)
{
  int32_t ret;
  OV9655_IO_t              IOCtx;
  static OV9655_Object_t   OV9655Obj;

  /* Configure the audio driver */
  IOCtx.Address     = CAMERA_OV9655_ADDRESS;
  IOCtx.Init        = BSP_I2C4_Init;
  IOCtx.DeInit      = BSP_I2C4_DeInit;
  IOCtx.ReadReg     = BSP_I2C4_ReadReg;
  IOCtx.WriteReg    = BSP_I2C4_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  if(OV9655_RegisterBusIO (&OV9655Obj, &IOCtx) != OV9655_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if(OV9655_ReadID(&OV9655Obj, &CameraId) != OV9655_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    if((CameraId != OV9655_ID) && (CameraId != OV9655_ID_2))
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else
    {
      Camera_Drv = (CAMERA_Drv_t *) &OV9655_CAMERA_Driver;
      Camera_CompObj = &OV9655Obj;
      if(Camera_Drv->Init(Camera_CompObj, Resolution, PixelFormat) != OV9655_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else if(Camera_Drv->GetCapabilities(Camera_CompObj, &Camera_Cap) != OV9655_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        ret = BSP_ERROR_NONE;
      }
    }
  }

  return ret;
}
#endif

#if (USE_CAMERA_SENSOR_OV5640 == 1)
/**
  * @brief  Register Bus IOs if component ID is OK
  * @retval error status
  */
static int32_t OV5640_Probe(uint32_t Resolution, uint32_t PixelFormat)
{
   int32_t ret;
  OV5640_IO_t              IOCtx;
  static OV5640_Object_t   OV5640Obj;

  /* Configure the audio driver */
  IOCtx.Address     = CAMERA_OV5640_ADDRESS;
  IOCtx.Init        = BSP_I2C4_Init;
  IOCtx.DeInit      = BSP_I2C4_DeInit;
  IOCtx.ReadReg     = BSP_I2C4_ReadReg16;
  IOCtx.WriteReg    = BSP_I2C4_WriteReg16;
  IOCtx.GetTick     = BSP_GetTick;

  if(OV5640_RegisterBusIO (&OV5640Obj, &IOCtx) != OV5640_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if(OV5640_ReadID(&OV5640Obj, &CameraId) != OV5640_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    if(CameraId != OV5640_ID)
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else
    {
      Camera_Drv = (CAMERA_Drv_t *) &OV5640_CAMERA_Driver;
      Camera_CompObj = &OV5640Obj;
      if(Camera_Drv->Init(Camera_CompObj, Resolution, PixelFormat) != OV5640_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else if(Camera_Drv->GetCapabilities(Camera_CompObj, &Camera_Cap) != OV5640_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {

        ret = BSP_ERROR_NONE;
      }
    }
  }

  return ret;
}
#endif


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
