/*
    Copyright (C) 2013-2015 Andrea Zoppi

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
 * @file    hal_stm32_dma2d.h
 * @brief   DMA2D/Chrom-ART driver.
 *
 * @addtogroup dma2d
 * @{
 */

#ifndef HAL_STM32_DMA2D_H_
#define HAL_STM32_DMA2D_H_

/**
 * @brief   Using the DMA2D driver.
 */
#if !defined(STM32_DMA2D_USE_DMA2D) || defined(__DOXYGEN__)
#define STM32_DMA2D_USE_DMA2D   (FALSE)
#endif

#if (TRUE == STM32_DMA2D_USE_DMA2D) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    DMA2D job modes
 * @{
 */
#define DMA2D_JOB_COPY          (0 << 16)   /**< Copy, replace(FG only).*/
#define DMA2D_JOB_CONVERT       (1 << 16)   /**< Copy, convert (FG + PFC).*/
#define DMA2D_JOB_BLEND         (2 << 16)   /**< Copy, blend (FG + BG + PFC).*/
#define DMA2D_JOB_CONST         (3 << 16)   /**< Default color only (FG REG).*/
/** @} */

/**
 * @name    DMA2D enable flag
 * @{
 */
#define DMA2D_EF_ENABLE         (1 <<  0)   /**< DMA2D enabled.*/
#define DMA2D_EF_DITHER         (1 << 16)   /**< Dithering enabled.*/
#define DMA2D_EF_PIXCLK_INVERT  (1 << 28)   /**< Inverted pixel clock.*/
#define DMA2D_EF_DATAEN_HIGH    (1 << 29)   /**< Active-high data enable.*/
#define DMA2D_EF_VSYNC_HIGH     (1 << 30)   /**< Active-high vsync.*/
#define DMA2D_EF_HSYNC_HIGH     (1 << 31)   /**< Active-high hsync.*/

/** Enable flags mask. */
#define DMA2D_EF_MASK \
  (DMA2D_EF_ENABLE | DMA2D_EF_DITHER | DMA2D_EF_PIXCLK_INVERT | \
   DMA2D_EF_DATAEN_HIGH | DMA2D_EF_VSYNC_HIGH | DMA2D_EF_HSYNC_HIGH)
/** @} */

/**
 * @name    DMA2D layer enable flags
 * @{
 */
#define DMA2D_LEF_ENABLE        (1 << 0)    /**< Layer enabled*/
#define DMA2D_LEF_KEYING        (1 << 1)    /**< Color keying enabled.*/
#define DMA2D_LEF_PALETTE       (1 << 4)    /**< Palette enabled.*/

/** Layer enable flag masks. */
#define DMA2D_LEF_MASK \
  (DMA2D_LEF_ENABLE | DMA2D_LEF_KEYING | DMA2D_LEF_PALETTE)
/** @} */

/**
 * @name    DMA2D pixel formats
 * @{
 */
#define DMA2D_FMT_ARGB8888      (0)           /**< ARGB-8888 format.*/
#define DMA2D_FMT_RGB888        (1)           /**< RGB-888 format.*/
#define DMA2D_FMT_RGB565        (2)           /**< RGB-565 format.*/
#define DMA2D_FMT_ARGB1555      (3)           /**< ARGB-1555 format.*/
#define DMA2D_FMT_ARGB4444      (4)           /**< ARGB-4444 format.*/
#define DMA2D_FMT_L8            (5)           /**< L-8 format.*/
#define DMA2D_FMT_AL44          (6)           /**< AL-44 format.*/
#define DMA2D_FMT_AL88          (7)           /**< AL-88 format.*/
#define DMA2D_FMT_L4            (8)           /**< L-4 format.*/
#define DMA2D_FMT_A8            (9)           /**< A-8 format.*/
#define DMA2D_FMT_A4            (10)          /**< A-4 format.*/
/** @} */

/**
 * @name    DMA2D pixel format aliased raw masks
 * @{
 */
#define DMA2D_XMASK_ARGB8888    (0xFFFFFFFF)  /**< ARGB-8888 aliased mask.*/
#define DMA2D_XMASK_RGB888      (0x00FFFFFF)  /**< RGB-888 aliased mask.*/
#define DMA2D_XMASK_RGB565      (0x00F8FCF8)  /**< RGB-565 aliased mask.*/
#define DMA2D_XMASK_ARGB1555    (0x80F8F8F8)  /**< ARGB-1555 aliased mask.*/
#define DMA2D_XMASK_ARGB4444    (0xF0F0F0F0)  /**< ARGB-4444 aliased mask.*/
#define DMA2D_XMASK_L8          (0x000000FF)  /**< L-8 aliased mask.*/
#define DMA2D_XMASK_AL44        (0xF00000F0)  /**< AL-44 aliased mask.*/
#define DMA2D_XMASK_AL88        (0xFF0000FF)  /**< AL-88 aliased mask.*/
#define DMA2D_XMASK_L4          (0x0000000F)  /**< L-4 aliased mask.*/
#define DMA2D_XMASK_A8          (0xFF000000)  /**< A-8 aliased mask.*/
#define DMA2D_XMASK_A4          (0xF0000000)  /**< A-4 aliased mask.*/
/** @} */

/**
 * @name    DMA2D alpha modes
 * @{
 */
#define DMA2D_ALPHA_KEEP        (0x00000000)  /**< Original alpha channel.*/
#define DMA2D_ALPHA_REPLACE     (0x00010000)  /**< Replace with constant.*/
#define DMA2D_ALPHA_MODULATE    (0x00020000)  /**< Modulate with constant.*/
/** @} */

/**
 * @name    DMA2D parameter bounds
 * @{
 */

#define DMA2D_MIN_PIXFMT_ID             (0)   /**< Minimum pixel format ID.*/
#define DMA2D_MAX_PIXFMT_ID             (11)  /**< Maximum pixel format ID.*/
#define DMA2D_MIN_OUTPIXFMT_ID          (0)   /**< Minimum output pixel format ID.*/
#define DMA2D_MAX_OUTPIXFMT_ID          (4)   /**< Maximum output pixel format ID.*/

#define DMA2D_MAX_OFFSET                ((1 << 14) - 1)

#define DMA2D_MAX_PALETTE_LENGTH        (256) /***/

#define DMA2D_MAX_WIDTH                 ((1 << 14) - 1)
#define DMA2D_MAX_HEIGHT                ((1 << 16) - 1)

#define DMA2D_MAX_WATERMARK_POS         ((1 << 16) - 1)

#define DMA2D_MAX_DEADTIME_CYCLES       ((1 << 8) - 1)

/** @} */

/**
 * @name    DMA2D basic ARGB-8888 colors.
 * @{
 */
/* Microsoft Windows default 16-color palette.*/
#define DMA2D_COLOR_BLACK       (0xFF000000)
#define DMA2D_COLOR_MAROON      (0xFF800000)
#define DMA2D_COLOR_GREEN       (0xFF008000)
#define DMA2D_COLOR_OLIVE       (0xFF808000)
#define DMA2D_COLOR_NAVY        (0xFF000080)
#define DMA2D_COLOR_PURPLE      (0xFF800080)
#define DMA2D_COLOR_TEAL        (0xFF008080)
#define DMA2D_COLOR_SILVER      (0xFFC0C0C0)
#define DMA2D_COLOR_GRAY        (0xFF808080)
#define DMA2D_COLOR_RED         (0xFFFF0000)
#define DMA2D_COLOR_LIME        (0xFF00FF00)
#define DMA2D_COLOR_YELLOW      (0xFFFFFF00)
#define DMA2D_COLOR_BLUE        (0xFF0000FF)
#define DMA2D_COLOR_FUCHSIA     (0xFFFF00FF)
#define DMA2D_COLOR_AQUA        (0xFF00FFFF)
#define DMA2D_COLOR_WHITE       (0xFFFFFFFF)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    DMA2D configuration options
 * @{
 */

/**
 * @brief   DMA2D event interrupt priority level setting.
 */
#if !defined(STM32_DMA2D_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_DMA2D_IRQ_PRIORITY            (11)
#endif

/**
 * @brief   Enables synchronous APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(DMA2D_USE_WAIT) || defined(__DOXYGEN__)
#define DMA2D_USE_WAIT                      (TRUE)
#endif

/**
 * @brief   Enables the @p dma2dAcquireBus() and @p dma2dReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(DMA2D_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define DMA2D_USE_MUTUAL_EXCLUSION          (TRUE)
#endif

/**
 * @brief   Provides software color conversion functions.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(DMA2D_USE_SOFTWARE_CONVERSIONS) || defined(__DOXYGEN__)
#define DMA2D_USE_SOFTWARE_CONVERSIONS      (TRUE)
#endif

/**
 * @brief   Enables checks for DMA2D functions.
 * @note    Disabling this option saves both code and data space.
 * @note    Disabling checks by ChibiOS will automatically disable DMA2D checks.
 */
#if !defined(DMA2D_USE_CHECKS) || defined(__DOXYGEN__)
#define DMA2D_USE_CHECKS                    (TRUE)
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (TRUE != STM32_HAS_DMA2D)
#error "DMA2D must be present when using the DMA2D subsystem"
#endif

#if (TRUE != STM32_DMA2D_USE_DMA2D) && (TRUE != STM32_HAS_DMA2D)
#error "DMA2D not present in the selected device"
#endif

#if (TRUE == DMA2D_USE_MUTUAL_EXCLUSION)
#if (TRUE != CH_CFG_USE_MUTEXES) && (TRUE != CH_CFG_USE_SEMAPHORES)
#error "DMA2D_USE_MUTUAL_EXCLUSION requires CH_CFG_USE_MUTEXES and/or CH_CFG_USE_SEMAPHORES"
#endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/* Complex types forwarding.*/
typedef union dma2d_coloralias_t dma2d_coloralias_t;
typedef struct dma2d_palcfg_t dma2d_palcfg_t;
typedef struct dma2d_laycfg_t dma2d_layercfg_t;
typedef struct DMA2DConfig DMA2DConfig;
typedef enum dma2d_state_t dma2d_state_t;
typedef struct DMA2DDriver DMA2DDriver;

/**
 * @name    DMA2D Data types
 * @{
 */

/**
 * @brief   DMA2D generic color.
 */
typedef uint32_t dma2d_color_t;

/**
 * @brief   DMA2D color aliases.
 * @detail  Mapped with ARGB-8888, except for luminance (L mapped onto B).
 *          Padding fields are prefixed with <tt>'x'</tt>, and should be clear
 *          (all 0) before compression and set (all 1) after expansion.
 */
typedef union dma2d_coloralias_t {
  struct {
    unsigned  b     :  8;
    unsigned  g     :  8;
    unsigned  r     :  8;
    unsigned  a     :  8;
  }             argb8888;           /**< Mapped ARGB-8888 bits.*/
  struct {
    unsigned  b     :  8;
    unsigned  g     :  8;
    unsigned  r     :  8;
    unsigned  xa    :  8;
  }             rgb888;             /**< Mapped RGB-888 bits.*/
  struct {
    unsigned  xb    :  3;
    unsigned  b     :  5;
    unsigned  xg    :  2;
    unsigned  g     :  6;
    unsigned  xr    :  3;
    unsigned  r     :  5;
    unsigned  xa    :  8;
  }             rgb565;             /**< Mapped RGB-565 bits.*/
  struct {
    unsigned  xb    :  3;
    unsigned  b     :  5;
    unsigned  xg    :  3;
    unsigned  g     :  5;
    unsigned  xr    :  3;
    unsigned  r     :  5;
    unsigned  xa    :  7;
    unsigned  a     :  1;
  }             argb1555;           /**< Mapped ARGB-1555 values.*/
  struct {
    unsigned  xb    :  4;
    unsigned  b     :  4;
    unsigned  xg    :  4;
    unsigned  g     :  4;
    unsigned  xr    :  4;
    unsigned  r     :  4;
    unsigned  xa    :  4;
    unsigned  a     :  4;
  }             argb4444;           /**< Mapped ARGB-4444 values.*/
  struct {
    unsigned  l     :  8;
    unsigned  x     : 16;
    unsigned  xa    :  8;
  }             l8;                 /**< Mapped L-8 bits.*/
  struct {
    unsigned  xl    :  4;
    unsigned  l     :  4;
    unsigned  x     : 16;
    unsigned  xa    :  4;
    unsigned  a     :  4;
  }             al44;               /**< Mapped AL-44 bits.*/
  struct {
    unsigned  l     :  8;
    unsigned  x     : 16;
    unsigned  a     :  8;
  }             al88;               /**< Mapped AL-88 bits.*/
  struct {
    unsigned  l     :  4;
    unsigned  xl    :  4;
    unsigned  x     : 16;
    unsigned  xa    :  8;
  }             l4;                 /**< Mapped L-4 bits.*/
  struct {
    unsigned  x     : 24;
    unsigned  a     :  8;
  }             a8;                 /**< Mapped A-8 bits.*/
  struct {
    unsigned  x     : 24;
    unsigned  xa    :  4;
    unsigned  a     :  4;
  }             a4;                 /**< Mapped A-4 bits.*/
  dma2d_color_t  aliased;           /**< Aliased raw bits.*/
} dma2d_coloralias_t;

/**
 * @brief   DMA2D job (transfer) mode.
 */
typedef uint32_t dma2d_jobmode_t;

/**
 * @brief   DMA2D pixel format.
 */
typedef uint32_t dma2d_pixfmt_t;

/**
 * @brief   DMA2D alpha mode.
 */
typedef uint32_t dma2d_amode_t;

/**
 * @brief   DMA2D ISR callback.
 */
typedef void (*dma2d_isrcb_t)(DMA2DDriver *dma2dp);

/**
 * @brief   DMA2D palette specifications.
 */
typedef struct dma2d_palcfg_t {
  const void        *colorsp;         /**< Pointer to color entries.*/
  uint16_t          length;           /**< Number of color entries.*/
  dma2d_pixfmt_t    fmt;              /**< Format, RGB-888 or ARGB-8888.*/
} dma2d_palcfg_t;

/**
 * @brief   DMA2D layer specifications.
 */
typedef struct dma2d_layercfg_t {
  void                  *bufferp;     /**< Frame buffer address.*/
  size_t                wrap_offset;  /**< Offset between lines, in pixels.*/
  dma2d_pixfmt_t        fmt;          /**< Pixel format.*/
  dma2d_color_t         def_color;    /**< Default color, RGB-888.*/
  uint8_t               const_alpha;  /**< Constant alpha factor.*/
  const dma2d_palcfg_t  *palettep;    /**< Palette specs, or @p NULL.*/
} dma2d_laycfg_t;

/**
 * @brief   DMA2D driver configuration.
 */
typedef struct DMA2DConfig {
  /* ISR callbacks.*/
  dma2d_isrcb_t     cfgerr_isr;       /**< Configuration error, or @p NULL.*/
  dma2d_isrcb_t     paltrfdone_isr;   /**< Palette transfer done, or @p NULL.*/
  dma2d_isrcb_t     palacserr_isr;    /**< Palette access error, or @p NULL.*/
  dma2d_isrcb_t     trfwmark_isr;     /**< Transfer watermark, or @p NULL.*/
  dma2d_isrcb_t     trfdone_isr;      /**< Transfer complete, or @p NULL.*/
  dma2d_isrcb_t     trferr_isr;       /**< Transfer error, or @p NULL.*/
} DMA2DConfig;

/**
 * @brief   DMA2D driver state.
 */
typedef enum dma2d_state_t {
  DMA2D_UNINIT      = (0),            /**< Not initialized.*/
  DMA2D_STOP        = (1),            /**< Stopped.*/
  DMA2D_READY       = (2),            /**< Ready.*/
  DMA2D_ACTIVE      = (3),            /**< Executing commands.*/
  DMA2D_PAUSED      = (4),            /**< Transfer suspended.*/
} dma2d_state_t;

/**
 * @brief   DMA2D driver.
 */
typedef struct DMA2DDriver {
  dma2d_state_t     state;          /**< Driver state.*/
  const DMA2DConfig *config;        /**< Driver configuration.*/

  /* Multithreading stuff.*/
#if (TRUE == DMA2D_USE_WAIT) || defined(__DOXYGEN__)
  thread_t          *thread;        /**< Waiting thread.*/
#endif  /* DMA2D_USE_WAIT */
#if (TRUE == DMA2D_USE_MUTUAL_EXCLUSION)
#if (TRUE == CH_CFG_USE_MUTEXES)
  mutex_t           lock;           /**< Multithreading lock.*/
#elif (TRUE == CH_CFG_USE_SEMAPHORES)
  semaphore_t       lock;           /**< Multithreading lock.*/
#endif
#endif  /* DMA2D_USE_MUTUAL_EXCLUSION */
} DMA2DDriver;

/** @} */

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Makes an ARGB-8888 value from byte components.
 *
 * @param[in] a         alpha byte component
 * @param[in] r         red byte component
 * @param[in] g         green byte component
 * @param[in] b         blue byte component
 *
 * @return              color in ARGB-8888 format
 *
 * @api
 */
#define dma2dMakeARGB8888(a, r, g, b) \
  ((((dma2d_color_t)(a) & 0xFF) << 24) | \
   (((dma2d_color_t)(r) & 0xFF) << 16) | \
   (((dma2d_color_t)(g) & 0xFF) <<  8) | \
   (((dma2d_color_t)(b) & 0xFF) <<  0))

/**
 * @brief   Compute bytes per pixel.
 * @details Computes the bytes per pixel for the specified pixel format.
 *          Rounds to the ceiling.
 *
 * @param[in] fmt       pixel format
 *
 * @return              bytes per pixel
 *
 * @api
 */
#define dma2dBytesPerPixel(fmt) \
  ((dma2dBitsPerPixel(fmt) + 7) >> 3)

/**
 * @brief   Compute pixel address.
 * @details Computes the buffer address of a pixel, given the buffer
 *          specifications.
 *
 * @param[in] originp   buffer origin address
 * @param[in] pitch     buffer pitch, in bytes
 * @param[in] fmt       buffer pixel format
 * @param[in] x         horizontal pixel coordinate
 * @param[in] y         vertical pixel coordinate
 *
 * @return              pixel address
 *
 * @api
 */
#define dma2dComputeAddress(originp, pitch, fmt, x, y) \
  ((void *)dma2dComputeAddressConst(originp, pitch, fmt, x, y))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern DMA2DDriver DMA2DD1;

#ifdef __cplusplus
extern "C" {
#endif

  /* Driver methods.*/
  void dma2dInit(void);
  void dma2dObjectInit(DMA2DDriver *dma2dp);
  dma2d_state_t dma2dGetStateI(DMA2DDriver *dma2dp);
  dma2d_state_t dma2dGetState(DMA2DDriver *dma2dp);
  void dma2dStart(DMA2DDriver *dma2dp, const DMA2DConfig *configp);
  void dma2dStop(DMA2DDriver *dma2dp);
#if (TRUE == DMA2D_USE_MUTUAL_EXCLUSION)
  void dma2dAcquireBusS(DMA2DDriver *dma2dp);
  void dma2dAcquireBus(DMA2DDriver *dma2dp);
  void dma2dReleaseBusS(DMA2DDriver *dma2dp);
  void dma2dReleaseBus(DMA2DDriver *dma2dp);
#endif  /* DMA2D_USE_MUTUAL_EXCLUSION */

  /* Global methods.*/
  uint16_t dma2dGetWatermarkPosI(DMA2DDriver *dma2dp);
  uint16_t dma2dGetWatermarkPos(DMA2DDriver *dma2dp);
  void dma2dSetWatermarkPosI(DMA2DDriver *dma2dp, uint16_t line);
  void dma2dSetWatermarkPos(DMA2DDriver *dma2dp, uint16_t line);
  bool dma2dIsWatermarkEnabledI(DMA2DDriver *dma2dp);
  bool dma2dIsWatermarkEnabled(DMA2DDriver *dma2dp);
  void dma2dEnableWatermarkI(DMA2DDriver *dma2dp);
  void dma2dEnableWatermark(DMA2DDriver *dma2dp);
  void dma2dDisableWatermarkI(DMA2DDriver *dma2dp);
  void dma2dDisableWatermark(DMA2DDriver *dma2dp);
  uint32_t dma2dGetDeadTimeI(DMA2DDriver *dma2dp);
  uint32_t dma2dGetDeadTime(DMA2DDriver *dma2dp);
  void dma2dSetDeadTimeI(DMA2DDriver *dma2dp, uint32_t cycles);
  void dma2dSetDeadTime(DMA2DDriver *dma2dp, uint32_t cycles);
  bool dma2dIsDeadTimeEnabledI(DMA2DDriver *dma2dp);
  bool dma2dIsDeadTimeEnabled(DMA2DDriver *dma2dp);
  void dma2dEnableDeadTimeI(DMA2DDriver *dma2dp);
  void dma2dEnableDeadTime(DMA2DDriver *dma2dp);
  void dma2dDisableDeadTimeI(DMA2DDriver *dma2dp);
  void dma2dDisableDeadTime(DMA2DDriver *dma2dp);

  /* Job methods.*/
  dma2d_jobmode_t dma2dJobGetModeI(DMA2DDriver *dma2dp);
  dma2d_jobmode_t dma2dJobGetMode(DMA2DDriver *dma2dp);
  void dma2dJobSetModeI(DMA2DDriver *dma2dp, dma2d_jobmode_t mode);
  void dma2dJobSetMode(DMA2DDriver *dma2dp, dma2d_jobmode_t mode);
  void dma2dJobGetSizeI(DMA2DDriver *dma2dp,
                        uint16_t *widthp, uint16_t *heightp);
  void dma2dJobGetSize(DMA2DDriver *dma2dp,
                       uint16_t *widthp, uint16_t *heightp);
  void dma2dJobSetSizeI(DMA2DDriver *dma2dp, uint16_t width, uint16_t height);
  void dma2dJobSetSize(DMA2DDriver *dma2dp, uint16_t width, uint16_t height);
  bool dma2dJobIsExecutingI(DMA2DDriver *dma2dp);
  bool dma2dJobIsExecuting(DMA2DDriver *dma2dp);
  void dma2dJobStartI(DMA2DDriver *dma2dp);
  void dma2dJobStart(DMA2DDriver *dma2dp);
  void dma2dJobExecuteS(DMA2DDriver *dma2dp);
  void dma2dJobExecute(DMA2DDriver *dma2dp);
  void dma2dJobSuspendI(DMA2DDriver *dma2dp);
  void dma2dJobSuspend(DMA2DDriver *dma2dp);
  void dma2dJobResumeI(DMA2DDriver *dma2dp);
  void dma2dJobResume(DMA2DDriver *dma2dp);
  void dma2dJobAbortI(DMA2DDriver *dma2dp);
  void dma2dJobAbort(DMA2DDriver *dma2dp);

  /* Background layer methods.*/
  void *dma2dBgGetAddressI(DMA2DDriver *dma2dp);
  void *dma2dBgGetAddress(DMA2DDriver *dma2dp);
  void dma2dBgSetAddressI(DMA2DDriver *dma2dp, void *bufferp);
  void dma2dBgSetAddress(DMA2DDriver *dma2dp, void *bufferp);
  size_t dma2dBgGetWrapOffsetI(DMA2DDriver *dma2dp);
  size_t dma2dBgGetWrapOffset(DMA2DDriver *dma2dp);
  void dma2dBgSetWrapOffsetI(DMA2DDriver *dma2dp, size_t offset);
  void dma2dBgSetWrapOffset(DMA2DDriver *dma2dp, size_t offset);
  uint8_t dma2dBgGetConstantAlphaI(DMA2DDriver *dma2dp);
  uint8_t dma2dBgGetConstantAlpha(DMA2DDriver *dma2dp);
  void dma2dBgSetConstantAlphaI(DMA2DDriver *dma2dp, uint8_t a);
  void dma2dBgSetConstantAlpha(DMA2DDriver *dma2dp, uint8_t a);
  dma2d_amode_t dma2dBgGetAlphaModeI(DMA2DDriver *dma2dp);
  dma2d_amode_t dma2dBgGetAlphaMode(DMA2DDriver *dma2dp);
  void dma2dBgSetAlphaModeI(DMA2DDriver *dma2dp, dma2d_amode_t mode);
  void dma2dBgSetAlphaMode(DMA2DDriver *dma2dp, dma2d_amode_t mode);
  dma2d_pixfmt_t dma2dBgGetPixelFormatI(DMA2DDriver *dma2dp);
  dma2d_pixfmt_t dma2dBgGetPixelFormat(DMA2DDriver *dma2dp);
  void dma2dBgSetPixelFormatI(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt);
  void dma2dBgSetPixelFormat(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt);
  dma2d_color_t dma2dBgGetDefaultColorI(DMA2DDriver *dma2dp);
  dma2d_color_t dma2dBgGetDefaultColor(DMA2DDriver *dma2dp);
  void dma2dBgSetDefaultColorI(DMA2DDriver *dma2dp, dma2d_color_t c);
  void dma2dBgSetDefaultColor(DMA2DDriver *dma2dp, dma2d_color_t c);
  void dma2dBgGetPaletteI(DMA2DDriver *dma2dp, dma2d_palcfg_t *palettep);
  void dma2dBgGetPalette(DMA2DDriver *dma2dp, dma2d_palcfg_t *palettep);
  void dma2dBgSetPaletteS(DMA2DDriver *dma2dp, const dma2d_palcfg_t *palettep);
  void dma2dBgSetPalette(DMA2DDriver *dma2dp, const dma2d_palcfg_t *palettep);
  void dma2dBgGetLayerI(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp);
  void dma2dBgGetLayer(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp);
  void dma2dBgSetConfigS(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp);
  void dma2dBgSetConfig(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp);

  /* Foreground layer methods.*/
  void *dma2dFgGetAddressI(DMA2DDriver *dma2dp);
  void *dma2dFgGetAddress(DMA2DDriver *dma2dp);
  void dma2dFgSetAddressI(DMA2DDriver *dma2dp, void *bufferp);
  void dma2dFgSetAddress(DMA2DDriver *dma2dp, void *bufferp);
  size_t dma2dFgGetWrapOffsetI(DMA2DDriver *dma2dp);
  size_t dma2dFgGetWrapOffset(DMA2DDriver *dma2dp);
  void dma2dFgSetWrapOffsetI(DMA2DDriver *dma2dp, size_t offset);
  void dma2dFgSetWrapOffset(DMA2DDriver *dma2dp, size_t offset);
  uint8_t dma2dFgGetConstantAlphaI(DMA2DDriver *dma2dp);
  uint8_t dma2dFgGetConstantAlpha(DMA2DDriver *dma2dp);
  void dma2dFgSetConstantAlphaI(DMA2DDriver *dma2dp, uint8_t a);
  void dma2dFgSetConstantAlpha(DMA2DDriver *dma2dp, uint8_t a);
  dma2d_amode_t dma2dFgGetAlphaModeI(DMA2DDriver *dma2dp);
  dma2d_amode_t dma2dFgGetAlphaMode(DMA2DDriver *dma2dp);
  void dma2dFgSetAlphaModeI(DMA2DDriver *dma2dp, dma2d_amode_t mode);
  void dma2dFgSetAlphaMode(DMA2DDriver *dma2dp, dma2d_amode_t mode);
  dma2d_pixfmt_t dma2dFgGetPixelFormatI(DMA2DDriver *dma2dp);
  dma2d_pixfmt_t dma2dFgGetPixelFormat(DMA2DDriver *dma2dp);
  void dma2dFgSetPixelFormatI(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt);
  void dma2dFgSetPixelFormat(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt);
  dma2d_color_t dma2dFgGetDefaultColorI(DMA2DDriver *dma2dp);
  dma2d_color_t dma2dFgGetDefaultColor(DMA2DDriver *dma2dp);
  void dma2dFgSetDefaultColorI(DMA2DDriver *dma2dp, dma2d_color_t c);
  void dma2dFgSetDefaultColor(DMA2DDriver *dma2dp, dma2d_color_t c);
  void dma2dFgGetPaletteI(DMA2DDriver *dma2dp, dma2d_palcfg_t *palettep);
  void dma2dFgGetPalette(DMA2DDriver *dma2dp, dma2d_palcfg_t *palettep);
  void dma2dFgSetPaletteS(DMA2DDriver *dma2dp, const dma2d_palcfg_t *palettep);
  void dma2dFgSetPalette(DMA2DDriver *dma2dp, const dma2d_palcfg_t *palettep);
  void dma2dFgGetLayerI(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp);
  void dma2dFgGetLayer(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp);
  void dma2dFgSetConfigS(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp);
  void dma2dFgSetConfig(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp);

  /* Output layer methods.*/
  void *dma2dOutGetAddressI(DMA2DDriver *dma2dp);
  void *dma2dOutGetAddress(DMA2DDriver *dma2dp);
  void dma2dOutSetAddressI(DMA2DDriver *dma2dp, void *bufferp);
  void dma2dOutSetAddress(DMA2DDriver *dma2dp, void *bufferp);
  size_t dma2dOutGetWrapOffsetI(DMA2DDriver *dma2dp);
  size_t dma2dOutGetWrapOffset(DMA2DDriver *dma2dp);
  void dma2dOutSetWrapOffsetI(DMA2DDriver *dma2dp, size_t offset);
  void dma2dOutSetWrapOffset(DMA2DDriver *dma2dp, size_t offset);
  dma2d_pixfmt_t dma2dOutGetPixelFormatI(DMA2DDriver *dma2dp);
  dma2d_pixfmt_t dma2dOutGetPixelFormat(DMA2DDriver *dma2dp);
  void dma2dOutSetPixelFormatI(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt);
  void dma2dOutSetPixelFormat(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt);
  dma2d_color_t dma2dOutGetDefaultColorI(DMA2DDriver *dma2dp);
  dma2d_color_t dma2dOutGetDefaultColor(DMA2DDriver *dma2dp);
  void dma2dOutSetDefaultColorI(DMA2DDriver *dma2dp, dma2d_color_t c);
  void dma2dOutSetDefaultColor(DMA2DDriver *dma2dp, dma2d_color_t c);
  void dma2dOutGetLayerI(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp);
  void dma2dOutGetLayer(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp);
  void dma2dOutSetConfigI(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp);
  void dma2dOutSetConfig(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp);

  /* Helper functions.*/
  const void *dma2dComputeAddressConst(const void *originp, size_t pitch,
                                       dma2d_pixfmt_t fmt,
                                       uint16_t x, uint16_t y);
  bool dma2dIsAligned(const void *bufferp, dma2d_pixfmt_t fmt);
  size_t dma2dBitsPerPixel(dma2d_pixfmt_t fmt);
#if (TRUE == DMA2D_USE_SOFTWARE_CONVERSIONS) || defined(__DOXYGEN__)
  dma2d_color_t dma2dFromARGB8888(dma2d_color_t c, dma2d_pixfmt_t fmt);
  dma2d_color_t dma2dToARGB8888(dma2d_color_t c, dma2d_pixfmt_t fmt);
#endif  /* DMA2D_USE_SOFTWARE_CONVERSIONS */

#ifdef __cplusplus
}
#endif

#endif  /* STM32_DMA2D_USE_DMA2D */

#endif  /* HAL_STM32_DMA2D_H_ */

/** @} */
