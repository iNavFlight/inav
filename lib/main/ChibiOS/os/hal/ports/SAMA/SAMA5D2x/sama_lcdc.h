/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    SAMA5D2x/sama_lcdc.h
 * @brief   SAMA LCDC support macros and structures.
 *
 * @addtogroup SAMA5D2x_LCDC
 * @{
 */

#ifndef SAMA_LCDC_LLD_H
#define SAMA_LCDC_LLD_H

/**
 * @brief   Using the LCDC driver.
 */
#if !defined(SAMA_USE_LCDC) || defined(__DOXYGEN__)
#define SAMA_USE_LCDC                          FALSE
#endif

#if (SAMA_USE_LCDC) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    LCDC ID LAYERS
 * @{
 */
/* LCD controller ID, no display, configuration ONLY */
#define LCDD_CONTROLLER     0

/* LCD base layer, display fixed size image */
#define LCDD_BASE           1

/* LCD Overlay 1 */
#define LCDD_OVR1           2

/* LCD Overlay 2 */
#define LCDD_OVR2           4

/* LCD HighEndOverlay, support resize */
#define LCDD_HEO            3

/* LCD Cursor, max size 128x128 */
#define LCDD_CUR            6
/** @} */

/**
 * @name    BPP MODE
 * @{
 */
#define   LCDC_CFG_RGBMODE_12BPP_RGB_444          (0x0u << 4)
#define   LCDC_CFG_RGBMODE_16BPP_ARGB_4444        (0x1u << 4)
#define   LCDC_CFG_RGBMODE_16BPP_RGBA_4444        (0x2u << 4)
#define   LCDC_CFG_RGBMODE_16BPP_RGB_565          (0x3u << 4)
#define   LCDC_CFG_RGBMODE_16BPP_TRGB_1555        (0x4u << 4)
#define   LCDC_CFG_RGBMODE_18BPP_RGB_666          (0x5u << 4)
#define   LCDC_CFG_RGBMODE_18BPP_RGB_666PACKED    (0x6u << 4)
#define   LCDC_CFG_RGBMODE_19BPP_TRGB_1666        (0x7u << 4)
#define   LCDC_CFG_RGBMODE_19BPP_TRGB_PACKED      (0x8u << 4)
#define   LCDC_CFG_RGBMODE_24BPP_RGB_888          (0x9u << 4)
#define   LCDC_CFG_RGBMODE_24BPP_RGB_888_PACKED   (0xAu << 4)
#define   LCDC_CFG_RGBMODE_25BPP_TRGB_1888        (0xBu << 4)
#define   LCDC_CFG_RGBMODE_32BPP_ARGB_8888        (0xCu << 4)
#define   LCDC_CFG_RGBMODE_32BPP_RGBA_8888        (0xDu << 4)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
/**
 * @brief   LCDC_USE_WAIT.
 */
#if !defined(LCDC_USE_WAIT) || defined(__DOXYGEN__)
#define LCDC_USE_WAIT                         FALSE
#endif

/**
 * @brief   LCDC_USE_MUTUAL_EXCLUSION.
 */
#if !defined(LCDC_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define LCDC_USE_MUTUAL_EXCLUSION             FALSE
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  LCDC_UNINIT   = 0,                /**< Not initialized.*/
  LCDC_STOP     = 1,                /**< Stopped.*/
  LCDC_READY    = 2,                /**< Ready.*/
  LCDC_ACTIVE   = 3,                /**< Executing commands.*/
} lcdcstate_t;

/**
 * @brief   Type of a structure representing an LCDC driver.
 */
typedef struct LCDCDriver LCDCDriver;

/**
 * @brief   LCD display layer information.
 */
typedef struct {
  /**
   * @brief   Display image buffer.
   */
  void                      *buffer;
  /**
   * @brief   Display image width.
   */
  uint16_t                  width;
  /**
   * @brief   Display image height.
   */
  uint16_t                  height;
  /**
   * @brief   Display image x position.
   */
  uint16_t                  x_pos;
  /**
   * @brief   Display image y_pos.
   */
  uint16_t                  y_pos;
  /**
   * @brief   Horizontal image Size in Memory.
   */
  uint16_t                  w_img;
  /**
   * @brief   Vertical image Size in Memory.
   */
  uint16_t                  h_img;
  /**
   * @brief   BPP mode.
   */
  uint8_t                   bpp;
  /**
   * @brief   Layer ID.
   */
  uint8_t                   layer_id;
} LCDCLayerConfig;

/**
 * @brief   Driver LCD configuration structure.
 */
typedef struct {
  /**
   * @brief   Display image width.
   */
  uint16_t                  width;
  /**
   * @brief   Display image height.
   */
  uint16_t                  height;
  /**
   * @brief   Frame rate in Hz.
   */
  uint8_t                   framerate;
  /**
   * @brief   Vertical front porch in number of lines.
   */
  uint8_t                   timing_vfp;
  /**
   * @brief   Vertical back porch in number of lines.
   */
  uint8_t                   timing_vbp;
  /**
   * @brief   Vertical pulse width in number of lines.
   */
  uint8_t                   timing_vpw;
  /**
   * @brief   Horizontal front porch in LCDDOTCLK cycles.
   */
  uint8_t                   timing_hfp;
  /**
   * @brief   Horizontal back porch in LCDDOTCLK cycles.
   */
  uint8_t                   timing_hbp;
  /**
   * @brief   Horizontal pulse width in LCDDOTCLK cycles.
   */
  uint8_t                   timing_hpw;
  /**
   * @brief lenght of LCDCLayerConfig array
   * @note  Number of layers to configure
   */
  size_t                    length;
  /**
   * @brief pointer to LCDCLayerConfig array
   */
  LCDCLayerConfig           *listp;
} LCDCConfig;

/**
 * @brief   Structure representing an LCDC driver.
 */
struct LCDCDriver {
  /**
   * @brief   Driver state.
   */
  lcdcstate_t               state;
  /**
   * @brief   Current configuration lcd data.
   */
  const LCDCConfig          *config;
  /**
   * @brief   Pointer to the LCDC registers block.
   */
  Lcdc                      *lcdc;
  /* Multithreading stuff.*/
#if (LCDC_USE_WAIT == TRUE) || defined(__DOXYGEN__)
  thread_t          *thread;
#endif  /* LCDC_USE_WAIT */
#if (LCDC_USE_MUTUAL_EXCLUSION == TRUE)
#if (CH_CFG_USE_MUTEXES == TRUE)
  mutex_t           lock;
#elif (CH_CFG_USE_SEMAPHORES == TRUE)
  semaphore_t       lock;
#endif
#endif  /* LCDC_USE_MUTUAL_EXCLUSION */
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern LCDCDriver LCDCD0;

#ifdef __cplusplus
extern "C" {
#endif
  void lcdcObjectInit(LCDCDriver *lcdcp);
  void lcdcInit(void);
  void lcdcStart(LCDCDriver *lcdcp, const LCDCConfig *configp);
  void lcdcStop(LCDCDriver *lcdcp);
  void lcdcShowLayer(LCDCDriver *lcdcp, uint8_t id, bool enable);
  void lcdcSetBacklight(uint32_t level);
#if (LCDC_USE_MUTUAL_EXCLUSION)
  void lcdcAcquireBusS(LCDCDriver *lcdcp);
  void lcdcAcquireBus(LCDCDriver *lcdcp);
  void lcdcReleaseBusS(LCDCDriver *lcdcp);
  void lcdcReleaseBus(LCDCDriver *lcdcp);
#endif  /* LCDC_USE_MUTUAL_EXCLUSION */

#ifdef __cplusplus
}
#endif

#endif /* SAMA_USE_LCDC */

#endif /* SAMA_LCDC_LLD_H */

/** @} */
