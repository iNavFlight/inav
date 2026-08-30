/**
  ******************************************************************************
  * @file    stm32h750b_discovery_lcd.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h750b_discovery_lcd.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H750B_DK_LCD_H
#define STM32H750B_DK_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h750b_discovery_conf.h"
#include "stm32h750b_discovery_errno.h"

#ifndef USE_LCD_CTRL_RK043FN48H
  #define USE_LCD_CTRL_RK043FN48H 1
#endif

#if (USE_LCD_CTRL_RK043FN48H == 1)
/* Include RK043FN48H LCD Driver IC driver code */
#include "../Components/rk043fn48h/rk043fn48h.h"
#endif
#include "lcd.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H750B_DK
  * @{
  */

/** @defgroup STM32H750B_DK_LCD LCD
  * @{
  */

/** @defgroup STM32H750B_DK_LCD_Exported_Constants Exported Constants
  * @{
  */
#define LCD_INSTANCES_NBR          1U

#define LCD_ORIENTATION_LANDSCAPE        0x01U /* Landscape orientation choice of LCD screen              */

#define LCD_DEFAULT_WIDTH                480U
#define LCD_DEFAULT_HEIGHT               272U

#define BSP_LCD_RELOAD_NONE              0U                            /* No reload executed       */
#define BSP_LCD_RELOAD_IMMEDIATE         LTDC_RELOAD_IMMEDIATE         /* Immediate Reload         */
#define BSP_LCD_RELOAD_VERTICAL_BLANKING LTDC_RELOAD_VERTICAL_BLANKING /* Vertical Blanking Reload */

/**
  * @brief LCD special pins
  */
/* LCD reset pin */
#define LCD_RESET_PIN                         GPIO_PIN_2
#define LCD_RESET_PULL                        GPIO_NOPULL
#define LCD_RESET_GPIO_PORT                   GPIOA
#define LCD_RESET_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define LCD_RESET_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

/* LCD display enable pin */
#define LCD_DISP_EN_PIN                       GPIO_PIN_7
#define LCD_DISP_EN_GPIO_PORT                 GPIOK
#define LCD_DISP_EN_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOK_CLK_ENABLE()
#define LCD_DISP_EN_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOK_CLK_DISABLE()

/* Back-light control pin */
#define LCD_BL_CTRL_PIN                       GPIO_PIN_0
#define LCD_BL_CTRL_GPIO_PORT                 GPIOK
#define LCD_BL_CTRL_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOK_CLK_ENABLE()
#define LCD_BL_CTRL_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOK_CLK_DISABLE()
/**
  * @brief Definition for LCD Timer used to control the Brightnes
  */
#define LCD_TIMx                           TIM8
#define LCD_TIMx_CLK_ENABLE()              __HAL_RCC_TIM8_CLK_ENABLE()
#define LCD_TIMx_CLK_DISABLE()             __HAL_RCC_TIM8_CLK_DISABLE()
#define LCD_TIMx_CHANNEL                   TIM_CHANNEL_3
#define LCD_TIMx_CHANNEL_AF                GPIO_AF3_TIM8
#define LCD_TIMX_PERIOD_VALUE              ((uint32_t)50000) /* Period Value    */
#define LCD_TIMX_PRESCALER_VALUE           ((uint32_t)4)     /* Prescaler Value */

/* RGB565 colors definitions */
#define LCD_COLOR_RGB565_BLUE                 0x001FU
#define LCD_COLOR_RGB565_GREEN                0x07E0U
#define LCD_COLOR_RGB565_RED                  0xF800U
#define LCD_COLOR_RGB565_CYAN                 0x07FFU
#define LCD_COLOR_RGB565_MAGENTA              0xF81FU
#define LCD_COLOR_RGB565_YELLOW               0xFFE0U
#define LCD_COLOR_RGB565_LIGHTBLUE            0x841FU
#define LCD_COLOR_RGB565_LIGHTGREEN           0x87F0U
#define LCD_COLOR_RGB565_LIGHTRED             0xFC10U
#define LCD_COLOR_RGB565_LIGHTCYAN            0x87FFU
#define LCD_COLOR_RGB565_LIGHTMAGENTA         0xFC1FU
#define LCD_COLOR_RGB565_LIGHTYELLOW          0xFFF0U
#define LCD_COLOR_RGB565_DARKBLUE             0x0010U
#define LCD_COLOR_RGB565_DARKGREEN            0x0400U
#define LCD_COLOR_RGB565_DARKRED              0x8000U
#define LCD_COLOR_RGB565_DARKCYAN             0x0410U
#define LCD_COLOR_RGB565_DARKMAGENTA          0x8010U
#define LCD_COLOR_RGB565_DARKYELLOW           0x8400U
#define LCD_COLOR_RGB565_WHITE                0xFFFFU
#define LCD_COLOR_RGB565_LIGHTGRAY            0xD69AU
#define LCD_COLOR_RGB565_GRAY                 0x8410U
#define LCD_COLOR_RGB565_DARKGRAY             0x4208U
#define LCD_COLOR_RGB565_BLACK                0x0000U
#define LCD_COLOR_RGB565_BROWN                0xA145U
#define LCD_COLOR_RGB565_ORANGE               0xFD20U
/* Definition of Official ST COLOR */
#define LCD_COLOR_RGB565_ST_BLUE_DARK         0x0001U
#define LCD_COLOR_RGB565_ST_BLUE              0x01EBU
#define LCD_COLOR_RGB565_ST_BLUE_LIGHT        0x06A7U
#define LCD_COLOR_RGB565_ST_GREEN_LIGHT       0x05ECU
#define LCD_COLOR_RGB565_ST_GREEN_DARK        0x001CU
#define LCD_COLOR_RGB565_ST_YELLOW            0x07F0U
#define LCD_COLOR_RGB565_ST_BROWN             0x02C8U
#define LCD_COLOR_RGB565_ST_PINK              0x0681U
#define LCD_COLOR_RGB565_ST_PURPLE            0x02CDU
#define LCD_COLOR_RGB565_ST_GRAY_DARK         0x0251U
#define LCD_COLOR_RGB565_ST_GRAY              0x04BAU
#define LCD_COLOR_RGB565_ST_GRAY_LIGHT        0x05E7U

/* ARGB8888 colors definitions */
#define LCD_COLOR_ARGB8888_BLUE               0xFF0000FFUL
#define LCD_COLOR_ARGB8888_GREEN              0xFF00FF00UL
#define LCD_COLOR_ARGB8888_RED                0xFFFF0000UL
#define LCD_COLOR_ARGB8888_CYAN               0xFF00FFFFUL
#define LCD_COLOR_ARGB8888_MAGENTA            0xFFFF00FFUL
#define LCD_COLOR_ARGB8888_YELLOW             0xFFFFFF00UL
#define LCD_COLOR_ARGB8888_LIGHTBLUE          0xFF8080FFUL
#define LCD_COLOR_ARGB8888_LIGHTGREEN         0xFF80FF80UL
#define LCD_COLOR_ARGB8888_LIGHTRED           0xFFFF8080UL
#define LCD_COLOR_ARGB8888_LIGHTCYAN          0xFF80FFFFUL
#define LCD_COLOR_ARGB8888_LIGHTMAGENTA       0xFFFF80FFUL
#define LCD_COLOR_ARGB8888_LIGHTYELLOW        0xFFFFFF80UL
#define LCD_COLOR_ARGB8888_DARKBLUE           0xFF000080UL
#define LCD_COLOR_ARGB8888_DARKGREEN          0xFF008000UL
#define LCD_COLOR_ARGB8888_DARKRED            0xFF800000UL
#define LCD_COLOR_ARGB8888_DARKCYAN           0xFF008080UL
#define LCD_COLOR_ARGB8888_DARKMAGENTA        0xFF800080UL
#define LCD_COLOR_ARGB8888_DARKYELLOW         0xFF808000UL
#define LCD_COLOR_ARGB8888_WHITE              0xFFFFFFFFUL
#define LCD_COLOR_ARGB8888_LIGHTGRAY          0xFFD3D3D3UL
#define LCD_COLOR_ARGB8888_GRAY               0xFF808080UL
#define LCD_COLOR_ARGB8888_DARKGRAY           0xFF404040UL
#define LCD_COLOR_ARGB8888_BLACK              0xFF000000UL
#define LCD_COLOR_ARGB8888_BROWN              0xFFA52A2AUL
#define LCD_COLOR_ARGB8888_ORANGE             0xFFFFA500UL
/* Definition of Official ST Colors */
#define LCD_COLOR_ARGB8888_ST_BLUE_DARK       0xFF002052UL
#define LCD_COLOR_ARGB8888_ST_BLUE            0xFF39A9DCUL
#define LCD_COLOR_ARGB8888_ST_BLUE_LIGHT      0xFFD1E4F3UL
#define LCD_COLOR_ARGB8888_ST_GREEN_LIGHT     0xFFBBCC01UL
#define LCD_COLOR_ARGB8888_ST_GREEN_DARK      0xFF003D14UL
#define LCD_COLOR_ARGB8888_ST_YELLOW          0xFFFFD300UL
#define LCD_COLOR_ARGB8888_ST_BROWN           0xFF5C0915UL
#define LCD_COLOR_ARGB8888_ST_PINK            0xFFD4007AUL
#define LCD_COLOR_ARGB8888_ST_PURPLE          0xFF590D58UL
#define LCD_COLOR_ARGB8888_ST_GRAY_DARK       0xFF4F5251UL
#define LCD_COLOR_ARGB8888_ST_GRAY            0xFF90989EUL
#define LCD_COLOR_ARGB8888_ST_GRAY_LIGHT      0xFFB9C4CAUL
/**
  * @}
  */

/** @defgroup STM32H750B_DK_LCD_Exported_Types Exported Types
  * @{
  */
typedef struct
{
uint32_t XSize;
uint32_t YSize;
uint32_t ActiveLayer;
uint32_t PixelFormat;
uint32_t BppFactor;
uint32_t IsMspCallbacksValid;
uint32_t ReloadEnable;
uint32_t Brightness;
} BSP_LCD_Ctx_t;

typedef struct
{
uint32_t X0;
uint32_t X1;
uint32_t Y0;
uint32_t Y1;
uint32_t PixelFormat;
uint32_t Address;
}MX_LTDC_LayerConfig_t;

#define BSP_LCD_LayerConfig_t MX_LTDC_LayerConfig_t

#if (USE_HAL_LTDC_REGISTER_CALLBACKS == 1)
typedef struct
{
  pLTDC_CallbackTypeDef            pMspLtdcInitCb;
  pLTDC_CallbackTypeDef            pMspLtdcDeInitCb;
}BSP_LCD_Cb_t;
#endif /*(USE_HAL_LTDC_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */

/** @addtogroup STM32H750B_DK_LCD_Exported_Variables
* @{
*/
extern const LCD_UTILS_Drv_t LCD_Driver;
extern DMA2D_HandleTypeDef hlcd_dma2d;
extern LTDC_HandleTypeDef  hlcd_ltdc;
extern BSP_LCD_Ctx_t       LcdCtx[];

/**
  * @}
  */

/** @defgroup STM32H750B_DK_LCD_Exported_FunctionsPrototypes Exported Functions Prototypes
  * @{
  */
/* Initialization APIs */
int32_t BSP_LCD_Init(uint32_t Instance, uint32_t Orientation);
int32_t BSP_LCD_InitEx(uint32_t Instance, uint32_t Orientation, uint32_t PixelFormat, uint32_t Width, uint32_t Height);
int32_t BSP_LCD_DeInit(uint32_t Instance);

/* Register Callbacks APIs */
#if (USE_HAL_LTDC_REGISTER_CALLBACKS == 1)
int32_t BSP_LCD_RegisterDefaultMspCallbacks (uint32_t Instance);
int32_t BSP_LCD_RegisterMspCallbacks (uint32_t Instance, BSP_LCD_Cb_t *CallBacks);
#endif /*(USE_HAL_LTDC_REGISTER_CALLBACKS == 1) */

/* LCD specific APIs: Layer control & LCD HW reset */
int32_t BSP_LCD_Relaod(uint32_t Instance, uint32_t ReloadType);
int32_t BSP_LCD_ConfigLayer(uint32_t Instance, uint32_t LayerIndex, BSP_LCD_LayerConfig_t *Config);
int32_t BSP_LCD_SetLayerVisible(uint32_t Instance, uint32_t LayerIndex, FunctionalState State);
int32_t BSP_LCD_SetTransparency(uint32_t Instance, uint32_t LayerIndex, uint8_t Transparency);
int32_t BSP_LCD_SetLayerAddress(uint32_t Instance, uint32_t LayerIndex, uint32_t Address);
int32_t BSP_LCD_SetLayerWindow(uint32_t Instance, uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
int32_t BSP_LCD_SetColorKeying(uint32_t Instance, uint32_t LayerIndex, uint32_t Color);
int32_t BSP_LCD_ResetColorKeying(uint32_t Instance, uint32_t LayerIndex);

/* LCD generic APIs: Display control */
int32_t BSP_LCD_DisplayOn(uint32_t Instance);
int32_t BSP_LCD_DisplayOff(uint32_t Instance);
int32_t BSP_LCD_SetBrightness(uint32_t Instance, uint32_t Brightness);
int32_t BSP_LCD_GetBrightness(uint32_t Instance, uint32_t *Brightness);
int32_t BSP_LCD_GetXSize(uint32_t Instance, uint32_t *XSize);
int32_t BSP_LCD_GetYSize(uint32_t Instance, uint32_t *YSize);

/* LCD generic APIs: Draw operations. This list of APIs is required for
lcd gfx utilities */
int32_t BSP_LCD_SetActiveLayer(uint32_t Instance, uint32_t LayerIndex);
int32_t BSP_LCD_GetPixelFormat(uint32_t Instance, uint32_t *PixelFormat);
int32_t BSP_LCD_DrawBitmap(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp);
int32_t BSP_LCD_DrawHLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t BSP_LCD_DrawVLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t BSP_LCD_FillRGBRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height);
int32_t BSP_LCD_FillRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);
int32_t BSP_LCD_ReadPixel(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t *Color);
int32_t BSP_LCD_WritePixel(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Color);

/* LCD MX APIs */
HAL_StatusTypeDef MX_LTDC_ConfigLayer(LTDC_HandleTypeDef *hltdc, uint32_t LayerIndex, MX_LTDC_LayerConfig_t *Config);
HAL_StatusTypeDef MX_LTDC_ClockConfig(LTDC_HandleTypeDef *hltdc);
HAL_StatusTypeDef MX_LTDC_Init(LTDC_HandleTypeDef *hltdc, uint32_t Width, uint32_t Height);
HAL_StatusTypeDef MX_LTDC_ClockConfig2(LTDC_HandleTypeDef *hltdc);

/**
  * @}
  */

/* Private constants --------------------------------------------------------*/
/** @defgroup STM32H750B_DK_LCD_Private_Constants Private Constants
  * @{
  */
/* Legacy defines */
#define LCD_ORIENTATION_PORTRAIT         0x00U /* Portrait orientation choice of LCD screen               */
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

#endif /* STM32H750B_DK_LCD_H */
