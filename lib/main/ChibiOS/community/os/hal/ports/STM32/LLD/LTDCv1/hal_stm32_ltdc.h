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
 * @file    hal_stm32_ltdc.h
 * @brief   LCD-TFT Controller Driver.
 *
 * @addtogroup ltdc
 * @{
 */

#ifndef HAL_STM32_LTDC_H_
#define HAL_STM32_LTDC_H_

/**
 * @brief   Using the LTDC driver.
 */
#if !defined(STM32_LTDC_USE_LTDC) || defined(__DOXYGEN__)
#define STM32_LTDC_USE_LTDC     (FALSE)
#endif

#if (TRUE == STM32_LTDC_USE_LTDC) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    LTDC enable flags
 * @{
 */
#define LTDC_EF_ENABLE          (1 <<  0)   /**< LTDC enabled.*/
#define LTDC_EF_DITHER          (1 << 16)   /**< Dithering enabled.*/
#define LTDC_EF_PIXCLK_INVERT   (1 << 28)   /**< Inverted pixel clock.*/
#define LTDC_EF_DATAEN_HIGH     (1 << 29)   /**< Active-high data enable.*/
#define LTDC_EF_VSYNC_HIGH      (1 << 30)   /**< Active-high vsync.*/
#define LTDC_EF_HSYNC_HIGH      (1 << 31)   /**< Active-high hsync.*/

#define LTDC_EF_MASK \
  (LTDC_EF_ENABLE | LTDC_EF_DITHER | LTDC_EF_PIXCLK_INVERT | \
   LTDC_EF_DATAEN_HIGH | LTDC_EF_VSYNC_HIGH | LTDC_EF_HSYNC_HIGH)
/** @} */

/**
 * @name    LTDC layer enable flags
 * @{
 */
#define LTDC_LEF_ENABLE         (1 << 0)    /**< Layer enabled*/
#define LTDC_LEF_KEYING         (1 << 1)    /**< Color keying enabled.*/
#define LTDC_LEF_PALETTE        (1 << 4)    /**< Palette enabled.*/

#define LTDC_LEF_MASK \
  (LTDC_LEF_ENABLE | LTDC_LEF_KEYING | LTDC_LEF_PALETTE)
/** @} */

/**
 * @name    LTDC pixel formats
 * @{
 */
#define LTDC_FMT_ARGB8888       (0)         /**< ARGB-8888 format.*/
#define LTDC_FMT_RGB888         (1)         /**< RGB-888 format.*/
#define LTDC_FMT_RGB565         (2)         /**< RGB-565 format.*/
#define LTDC_FMT_ARGB1555       (3)         /**< ARGB-1555 format.*/
#define LTDC_FMT_ARGB4444       (4)         /**< ARGB-4444 format.*/
#define LTDC_FMT_L8             (5)         /**< L-8 format.*/
#define LTDC_FMT_AL44           (6)         /**< AL-44 format.*/
#define LTDC_FMT_AL88           (7)         /**< AL-88 format.*/
/** @} */

/**
 * @name    LTDC pixel format aliased raw masks
 * @{
 */
#define LTDC_XMASK_ARGB8888     (0xFFFFFFFF)  /**< ARGB-8888 aliased mask.*/
#define LTDC_XMASK_RGB888       (0x00FFFFFF)  /**< RGB-888 aliased mask.*/
#define LTDC_XMASK_RGB565       (0x00F8FCF8)  /**< RGB-565 aliased mask.*/
#define LTDC_XMASK_ARGB1555     (0x80F8F8F8)  /**< ARGB-1555 aliased mask.*/
#define LTDC_XMASK_ARGB4444     (0xF0F0F0F0)  /**< ARGB-4444 aliased mask.*/
#define LTDC_XMASK_L8           (0x000000FF)  /**< L-8 aliased mask.*/
#define LTDC_XMASK_AL44         (0xF00000F0)  /**< AL-44 aliased mask.*/
#define LTDC_XMASK_AL88         (0xFF0000FF)  /**< AL-88 aliased mask.*/
/** @} */

/**
 * @name    LTDC blending factors
 * @{
 */
#define LTDC_BLEND_FIX1_FIX2    (0x0405)    /**<      cnst1; 1 -      cnst2 */
#define LTDC_BLEND_FIX1_MOD2    (0x0407)    /**<      cnst1; 1 - a2 * cnst2 */
#define LTDC_BLEND_MOD1_FIX2    (0x0605)    /**< a1 * cnst1; 1 -      cnst2 */
#define LTDC_BLEND_MOD1_MOD2    (0x0607)    /**< a1 * cnst1; 1 - a2 * cnst2 */
/** @} */

/**
 * @name    LTDC parameter bounds
 * @{
 */

#define LTDC_MIN_SCREEN_WIDTH           (1)
#define LTDC_MIN_SCREEN_HEIGHT          (1)
#define LTDC_MAX_SCREEN_WIDTH           (800)
#define LTDC_MAX_SCREEN_HEIGHT          (600)

#define LTDC_MIN_HSYNC_WIDTH            (1)
#define LTDC_MIN_VSYNC_HEIGHT           (1)
#define LTDC_MAX_HSYNC_WIDTH            (1 << 12)
#define LTDC_MAX_VSYNC_HEIGHT           (1 << 11)

#define LTDC_MIN_HBP_WIDTH              (0)
#define LTDC_MIN_VBP_HEIGHT             (0)
#define LTDC_MAX_HBP_WIDTH              (1 << 12)
#define LTDC_MAX_VBP_HEIGHT             (1 << 11)

#define LTDC_MIN_ACC_HBP_WIDTH          (1)
#define LTDC_MIN_ACC_VBP_HEIGHT         (1)
#define LTDC_MAX_ACC_HBP_WIDTH          (1 << 12)
#define LTDC_MAX_ACC_VBP_HEIGHT         (1 << 11)

#define LTDC_MIN_HFP_WIDTH              (0)
#define LTDC_MIN_VFP_HEIGHT             (0)
#define LTDC_MAX_HFP_WIDTH              (1 << 12)
#define LTDC_MAX_VFP_HEIGHT             (1 << 11)

#define LTDC_MIN_ACTIVE_WIDTH           (0)
#define LTDC_MIN_ACTIVE_HEIGHT          (0)
#define LTDC_MAX_ACTIVE_WIDTH           (1 << 12)
#define LTDC_MAX_ACTIVE_HEIGHT          (1 << 11)

#define LTDC_MIN_ACC_ACTIVE_WIDTH       (1)
#define LTDC_MIN_ACC_ACTIVE_HEIGHT      (1)
#define LTDC_MAX_ACC_ACTIVE_WIDTH       (1 << 12)
#define LTDC_MAX_ACC_ACTIVE_HEIGHT      (1 << 11)

#define LTDC_MIN_ACC_TOTAL_WIDTH        (1)
#define LTDC_MIN_ACC_TOTAL_HEIGHT       (1)
#define LTDC_MAX_ACC_TOTAL_WIDTH        (1 << 12)
#define LTDC_MAX_ACC_TOTAL_HEIGHT       (1 << 11)

#define LTDC_MIN_LINE_INTERRUPT_POS     (0)
#define LTDC_MAX_LINE_INTERRUPT_POS     ((1 << 11) - 1)

#define LTDC_MIN_WINDOW_HSTART          (0)
#define LTDC_MIN_WINDOW_HSTART          (0)
#define LTDC_MAX_WINDOW_HSTOP           ((1 << 12) - 1)
#define LTDC_MAX_WINDOW_HSTOP           ((1 << 12) - 1)

#define LTDC_MIN_WINDOW_VSTART          (0)
#define LTDC_MIN_WINDOW_VSTART          (0)
#define LTDC_MAX_WINDOW_VSTOP           ((1 << 11) - 1)
#define LTDC_MAX_WINDOW_VSTOP           ((1 << 11) - 1)

#define LTDC_MIN_FRAME_WIDTH_BYTES      (0)
#define LTDC_MIN_FRAME_HEIGHT_LINES     (0)
#define LTDC_MIN_FRAME_PITCH_BYTES      (0)
#define LTDC_MAX_FRAME_WIDTH_BYTES      ((1 << 13) - 1 - 3)
#define LTDC_MAX_FRAME_HEIGHT_LINES     ((1 << 11) - 1)
#define LTDC_MAX_FRAME_PITCH_BYTES      ((1 << 13) - 1)

#define LTDC_MIN_PIXFMT_ID              (0)
#define LTDC_MAX_PIXFMT_ID              (7)

#define LTDC_MAX_PALETTE_LENGTH         (256)

/** @} */

/**
 * @name    LTDC basic ARGB-8888 colors.
 * @{
 */
/* Microsoft Windows default 16-color palette.*/
#define LTDC_COLOR_BLACK        (0xFF000000)
#define LTDC_COLOR_MAROON       (0xFF800000)
#define LTDC_COLOR_GREEN        (0xFF008000)
#define LTDC_COLOR_OLIVE        (0xFF808000)
#define LTDC_COLOR_NAVY         (0xFF000080)
#define LTDC_COLOR_PURPLE       (0xFF800080)
#define LTDC_COLOR_TEAL         (0xFF008080)
#define LTDC_COLOR_SILVER       (0xFFC0C0C0)
#define LTDC_COLOR_GRAY         (0xFF808080)
#define LTDC_COLOR_RED          (0xFFFF0000)
#define LTDC_COLOR_LIME         (0xFF00FF00)
#define LTDC_COLOR_YELLOW       (0xFFFFFF00)
#define LTDC_COLOR_BLUE         (0xFF0000FF)
#define LTDC_COLOR_FUCHSIA      (0xFFFF00FF)
#define LTDC_COLOR_AQUA         (0xFF00FFFF)
#define LTDC_COLOR_WHITE        (0xFFFFFFFF)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    LTDC configuration options
 * @{
 */

/**
 * @brief   LTDC event interrupt priority level setting.
 */
#if !defined(STM32_LTDC_EV_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_LTDC_EV_IRQ_PRIORITY          (11)
#endif

/**
 * @brief   LTDC error interrupt priority level setting.
 */
#if !defined(STM32_LTDC_ER_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_LTDC_ER_IRQ_PRIORITY          (11)
#endif

/**
 * @brief   Enables synchronous APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(LTDC_USE_WAIT) || defined(__DOXYGEN__)
#define LTDC_USE_WAIT                       (TRUE)
#endif

/**
 * @brief   Enables the @p ltdcAcquireBus() and @p ltdcReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(LTDC_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define LTDC_USE_MUTUAL_EXCLUSION           (TRUE)
#endif

/**
 * @brief   Provides software color conversion functions.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(LTDC_USE_SOFTWARE_CONVERSIONS) || defined(__DOXYGEN__)
#define LTDC_USE_SOFTWARE_CONVERSIONS       (TRUE)
#endif

/**
 * @brief   Enables checks for LTDC functions.
 * @note    Disabling this option saves both code and data space.
 * @note    Disabling checks by ChibiOS will automatically disable LTDC checks.
 */
#if !defined(LTDC_USE_CHECKS) || defined(__DOXYGEN__)
#define LTDC_USE_CHECKS                     (TRUE)
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (TRUE != STM32_HAS_LTDC)
#error "LTDC must be present when using the LTDC subsystem"
#endif

#if (TRUE == STM32_LTDC_USE_LTDC) && (TRUE != STM32_HAS_LTDC)
#error "LTDC not present in the selected device"
#endif

#if (TRUE == LTDC_USE_MUTUAL_EXCLUSION)
#if (TRUE != CH_CFG_USE_MUTEXES) && (TRUE != CH_CFG_USE_SEMAPHORES)
#error "LTDC_USE_MUTUAL_EXCLUSION requires CH_CFG_USE_MUTEXES and/or CH_CFG_USE_SEMAPHORES"
#endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/* Complex types forwarding.*/
typedef union ltdc_coloralias_t ltdc_coloralias_t;
typedef struct ltdc_window_t ltdc_window_t;
typedef struct ltdc_frame_t ltdc_frame_t;
typedef struct ltdc_laycfg_t ltdc_laycfg_t;
typedef struct LTDCConfig LTDCConfig;
typedef enum ltdc_state_t ltdc_state_t;
typedef struct LTDCDriver LTDCDriver;

/**
 * @name    LTDC Data types
 * @{
 */

/**
 * @brief   LTDC generic color.
 */
typedef uint32_t ltdc_color_t;

/**
 * @brief   LTDC color aliases.
 * @detail  Mapped with ARGB-8888, except for luminance (L mapped onto B).
 *          Padding fields are prefixed with <tt>'x'</tt>, and should be clear
 *          (all 0) before compression and set (all 1) after expansion.
 */
typedef union ltdc_coloralias_t {
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
  ltdc_color_t  aliased;            /**< Aliased raw bits.*/
} ltdc_coloralias_t;

/**
 * @brief   LTDC layer identifier.
 */
typedef uint32_t ltdc_layerid_t;

/**
 * @brief   LTDC pixel format.
 */
typedef uint32_t ltdc_pixfmt_t;

/**
 * @brief   LTDC blending factor.
 */
typedef uint32_t ltdc_blendf_t;

/**
 * @brief   LTDC ISR callback.
 */
typedef void (*ltdc_isrcb_t)(LTDCDriver *ltdcp);

/**
 * @brief   LTDC window specifications.
 */
typedef struct ltdc_window_t {
  uint16_t      hstart;             /**< Horizontal start pixel (left).*/
  uint16_t      hstop;              /**< Horizontal stop pixel (right).*/
  uint16_t      vstart;             /**< Vertical start pixel (top).*/
  uint16_t      vstop;              /**< Vertical stop pixel (bottom).*/
} ltdc_window_t;

/**
 * @brief   LTDC frame specifications.
 */
typedef struct ltdc_frame_t {
  void          *bufferp;           /**< Frame buffer address.*/
  uint16_t      width;              /**< Frame width, in pixels.*/
  uint16_t      height;             /**< Frame height, in pixels.*/
  size_t        pitch;              /**< Line pitch, in bytes.*/
  ltdc_pixfmt_t fmt;                /**< Pixel format.*/
} ltdc_frame_t;

/**
 * @brief   LTDC configuration flags.
 */
typedef uint8_t ltdc_flags_t;

/**
 * @brief   LTDC startup layer configuration.
 */
typedef struct ltdc_laycfg_t {
  const ltdc_frame_t  *frame;       /**< Frame buffer specifications.*/
  const ltdc_window_t *window;      /**< Window specifications.*/
  ltdc_color_t        def_color;    /**< Default color, ARGB-8888.*/
  uint8_t             const_alpha;  /**< Constant alpha factor.*/
  ltdc_color_t        key_color;    /**< Color key.*/
  const ltdc_color_t  *pal_colors;  /**< Palette colors, or @p NULL.*/
  uint16_t            pal_length;   /**< Palette length, or @p 0.*/
  ltdc_blendf_t       blending;     /**< Blending factors.*/
  ltdc_flags_t        flags;        /**< Layer configuration flags.*/
} ltdc_laycfg_t;

/**
 * @brief   LTDC driver configuration.
 */
typedef struct LTDCConfig {
  /* Display specifications.*/
  uint16_t      screen_width;       /**< Screen pixel width.*/
  uint16_t      screen_height;      /**< Screen pixel height.*/
  uint16_t      hsync_width;        /**< Horizontal sync pixel width.*/
  uint16_t      vsync_height;       /**< Vertical sync pixel height.*/
  uint16_t      hbp_width;          /**< Horizontal back porch pixel width.*/
  uint16_t      vbp_height;         /**< Vertical back porch pixel height.*/
  uint16_t      hfp_width;          /**< Horizontal front porch pixel width.*/
  uint16_t      vfp_height;         /**< Vertical front porch pixel height.*/
  ltdc_flags_t  flags;              /**< Driver configuration flags.*/

  /* ISR callbacks.*/
  ltdc_isrcb_t  line_isr;           /**< Line Interrupt ISR, or @p NULL.*/
  ltdc_isrcb_t  rr_isr;             /**< Register Reload ISR, or @p NULL.*/
  ltdc_isrcb_t  fuerr_isr;          /**< FIFO Underrun ISR, or @p NULL.*/
  ltdc_isrcb_t  terr_isr;           /**< Transfer Error ISR, or @p NULL.*/

  /* Layer and color settings.*/
  ltdc_color_t  clear_color;        /**< Clear screen color, RGB-888.*/
  const ltdc_laycfg_t *bg_laycfg;   /**< Background layer specs, or @p NULL.*/
  const ltdc_laycfg_t *fg_laycfg;   /**< Foreground layer specs, or @p NULL.*/
} LTDCConfig;

/**
 * @brief   LTDC driver state.
 */
typedef enum ltdc_state_t {
  LTDC_UNINIT   = (0),              /**< Not initialized.*/
  LTDC_STOP     = (1),              /**< Stopped.*/
  LTDC_READY    = (2),              /**< Ready.*/
  LTDC_ACTIVE   = (3),              /**< Executing commands.*/
} ltdc_state_t;

/**
 * @brief   LTDC driver.
 */
typedef struct LTDCDriver {
  ltdc_state_t      state;          /**< Driver state.*/
  const LTDCConfig  *config;        /**< Driver configuration.*/

  /* Handy computations.*/
  ltdc_window_t     active_window;  /**< Active window coordinates.*/

  /* Multithreading stuff.*/
#if (TRUE == LTDC_USE_WAIT) || defined(__DOXYGEN__)
  thread_t          *thread;        /**< Waiting thread.*/
#endif  /* LTDC_USE_WAIT */
#if (TRUE == LTDC_USE_MUTUAL_EXCLUSION)
#if (TRUE == CH_CFG_USE_MUTEXES)
  mutex_t           lock;           /**< Multithreading lock.*/
#elif (TRUE == CH_CFG_USE_SEMAPHORES)
  semaphore_t       lock;           /**< Multithreading lock.*/
#endif
#endif  /* LTDC_USE_MUTUAL_EXCLUSION */
} LTDCDriver;

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
#define ltdcMakeARGB8888(a, r, g, b) \
  ((((ltdc_color_t)(a) & 0xFF) << 24) | \
   (((ltdc_color_t)(r) & 0xFF) << 16) | \
   (((ltdc_color_t)(g) & 0xFF) <<  8) | \
   (((ltdc_color_t)(b) & 0xFF) <<  0))

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
#define ltdcBytesPerPixel(fmt) \
  ((ltdcBitsPerPixel(fmt) + 7) >> 3)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern LTDCDriver LTDCD1;

#ifdef __cplusplus
extern "C" {
#endif
  /* Driver methods.*/
  void ltdcInit(void);
  void ltdcObjectInit(LTDCDriver *ltdcp);
  ltdc_state_t ltdcGetStateI(LTDCDriver *ltdcp);
  ltdc_state_t ltdcGetState(LTDCDriver *ltdcp);
  void ltdcStart(LTDCDriver *ltdcp, const LTDCConfig *configp);
  void ltdcStop(LTDCDriver *ltdcp);
#if (TRUE == LTDC_USE_MUTUAL_EXCLUSION)
  void ltdcAcquireBusS(LTDCDriver *ltdcp);
  void ltdcAcquireBus(LTDCDriver *ltdcp);
  void ltdcReleaseBusS(LTDCDriver *ltdcp);
  void ltdcReleaseBus(LTDCDriver *ltdcp);
#endif  /* LTDC_USE_MUTUAL_EXCLUSION */

  /* Global methods.*/
  ltdc_flags_t ltdcGetEnableFlagsI(LTDCDriver *ltdcp);
  ltdc_flags_t ltdcGetEnableFlags(LTDCDriver *ltdcp);
  void ltdcSetEnableFlagsI(LTDCDriver *ltdcp, ltdc_flags_t flags);
  void ltdcSetEnableFlags(LTDCDriver *ltdcp, ltdc_flags_t flags);
  bool ltdcIsReloadingI(LTDCDriver *ltdcp);
  bool ltdcIsReloading(LTDCDriver *ltdcp);
  void ltdcStartReloadI(LTDCDriver *ltdcp, bool immediately);
  void ltdcStartReload(LTDCDriver *ltdcp, bool immediately);
  void ltdcReloadS(LTDCDriver *ltdcp, bool immediately);
  void ltdcReload(LTDCDriver *ltdcp, bool immediately);
  bool ltdcIsDitheringEnabledI(LTDCDriver *ltdcp);
  bool ltdcIsDitheringEnabled(LTDCDriver *ltdcp);
  void ltdcEnableDitheringI(LTDCDriver *ltdcp);
  void ltdcEnableDithering(LTDCDriver *ltdcp);
  void ltdcDisableDitheringI(LTDCDriver *ltdcp);
  void ltdcDisableDithering(LTDCDriver *ltdcp);
  ltdc_color_t ltdcGetClearColorI(LTDCDriver *ltdcp);
  ltdc_color_t ltdcGetClearColor(LTDCDriver *ltdcp);
  void ltdcSetClearColorI(LTDCDriver *ltdcp, ltdc_color_t c);
  void ltdcSetClearColor(LTDCDriver *ltdcp, ltdc_color_t c);
  uint16_t ltdcGetLineInterruptPosI(LTDCDriver *ltdcp);
  uint16_t ltdcGetLineInterruptPos(LTDCDriver *ltdcp);
  void ltdcSetLineInterruptPosI(LTDCDriver *ltdcp, uint16_t line);
  void ltdcSetLineInterruptPos(LTDCDriver *ltdcp, uint16_t line);
  bool ltdcIsLineInterruptEnabledI(LTDCDriver *ltdcp);
  bool ltdcIsLineInterruptEnabled(LTDCDriver *ltdcp);
  void ltdcEnableLineInterruptI(LTDCDriver *ltdcp);
  void ltdcEnableLineInterrupt(LTDCDriver *ltdcp);
  void ltdcDisableLineInterruptI(LTDCDriver *ltdcp);
  void ltdcDisableLineInterrupt(LTDCDriver *ltdcp);
  void ltdcGetCurrentPosI(LTDCDriver *ltdcp, uint16_t *xp, uint16_t *yp);
  void ltdcGetCurrentPos(LTDCDriver *ltdcp, uint16_t *xp, uint16_t *yp);

  /* Background layer methods.*/
  ltdc_flags_t ltdcBgGetEnableFlagsI(LTDCDriver *ltdcp);
  ltdc_flags_t ltdcBgGetEnableFlags(LTDCDriver *ltdcp);
  void ltdcBgSetEnableFlagsI(LTDCDriver *ltdcp, ltdc_flags_t flags);
  void ltdcBgSetEnableFlags(LTDCDriver *ltdcp, ltdc_flags_t flags);
  bool ltdcBgIsEnabledI(LTDCDriver *ltdcp);
  bool ltdcBgIsEnabled(LTDCDriver *ltdcp);
  void ltdcBgEnableI(LTDCDriver *ltdcp);
  void ltdcBgEnable(LTDCDriver *ltdcp);
  void ltdcBgDisableI(LTDCDriver *ltdcp);
  void ltdcBgDisable(LTDCDriver *ltdcp);
  bool ltdcBgIsPaletteEnabledI(LTDCDriver *ltdcp);
  bool ltdcBgIsPaletteEnabled(LTDCDriver *ltdcp);
  void ltdcBgEnablePaletteI(LTDCDriver *ltdcp);
  void ltdcBgEnablePalette(LTDCDriver *ltdcp);
  void ltdcBgDisablePaletteI(LTDCDriver *ltdcp);
  void ltdcBgDisablePalette(LTDCDriver *ltdcp);
  void ltdcBgSetPaletteColorI(LTDCDriver *ltdcp, uint8_t slot, ltdc_color_t c);
  void ltdcBgSetPaletteColor(LTDCDriver *ltdcp, uint8_t slot, ltdc_color_t c);
  void ltdcBgSetPaletteI(LTDCDriver *ltdcp, const ltdc_color_t colors[],
                         uint16_t length);
  void ltdcBgSetPalette(LTDCDriver *ltdcp, const ltdc_color_t colors[],
                        uint16_t length);
  ltdc_pixfmt_t ltdcBgGetPixelFormatI(LTDCDriver *ltdcp);
  ltdc_pixfmt_t ltdcBgGetPixelFormat(LTDCDriver *ltdcp);
  void ltdcBgSetPixelFormatI(LTDCDriver *ltdcp, ltdc_pixfmt_t fmt);
  void ltdcBgSetPixelFormat(LTDCDriver *ltdcp, ltdc_pixfmt_t fmt);
  bool ltdcBgIsKeyingEnabledI(LTDCDriver *ltdcp);
  bool ltdcBgIsKeyingEnabled(LTDCDriver *ltdcp);
  void ltdcBgEnableKeyingI(LTDCDriver *ltdcp);
  void ltdcBgEnableKeying(LTDCDriver *ltdcp);
  void ltdcBgDisableKeyingI(LTDCDriver *ltdcp);
  void ltdcBgDisableKeying(LTDCDriver *ltdcp);
  ltdc_color_t ltdcBgGetKeyingColorI(LTDCDriver *ltdcp);
  ltdc_color_t ltdcBgGetKeyingColor(LTDCDriver *ltdcp);
  void ltdcBgSetKeyingColorI(LTDCDriver *ltdcp, ltdc_color_t c);
  void ltdcBgSetKeyingColor(LTDCDriver *ltdcp, ltdc_color_t c);
  uint8_t ltdcBgGetConstantAlphaI(LTDCDriver *ltdcp);
  uint8_t ltdcBgGetConstantAlpha(LTDCDriver *ltdcp);
  void ltdcBgSetConstantAlphaI(LTDCDriver *ltdcp, uint8_t a);
  void ltdcBgSetConstantAlpha(LTDCDriver *ltdcp, uint8_t a);
  ltdc_color_t ltdcBgGetDefaultColorI(LTDCDriver *ltdcp);
  ltdc_color_t ltdcBgGetDefaultColor(LTDCDriver *ltdcp);
  void ltdcBgSetDefaultColorI(LTDCDriver *ltdcp, ltdc_color_t c);
  void ltdcBgSetDefaultColor(LTDCDriver *ltdcp, ltdc_color_t c);
  ltdc_blendf_t ltdcBgGetBlendingFactorsI(LTDCDriver *ltdcp);
  ltdc_blendf_t ltdcBgGetBlendingFactors(LTDCDriver *ltdcp);
  void ltdcBgSetBlendingFactorsI(LTDCDriver *ltdcp, ltdc_blendf_t bf);
  void ltdcBgSetBlendingFactors(LTDCDriver *ltdcp, ltdc_blendf_t bf);
  void ltdcBgGetWindowI(LTDCDriver *ltdcp, ltdc_window_t *windowp);
  void ltdcBgGetWindow(LTDCDriver *ltdcp, ltdc_window_t *windowp);
  void ltdcBgSetWindowI(LTDCDriver *ltdcp, const ltdc_window_t *windowp);
  void ltdcBgSetWindow(LTDCDriver *ltdcp, const ltdc_window_t *windowp);
  void ltdcBgSetInvalidWindowI(LTDCDriver *ltdcp);
  void ltdcBgSetInvalidWindow(LTDCDriver *ltdcp);
  void ltdcBgGetFrameI(LTDCDriver *ltdcp, ltdc_frame_t *framep);
  void ltdcBgGetFrame(LTDCDriver *ltdcp, ltdc_frame_t *framep);
  void ltdcBgSetFrameI(LTDCDriver *ltdcp, const ltdc_frame_t *framep);
  void ltdcBgSetFrame(LTDCDriver *ltdcp, const ltdc_frame_t *framep);
  void *ltdcBgGetFrameAddressI(LTDCDriver *ltdcp);
  void *ltdcBgGetFrameAddress(LTDCDriver *ltdcp);
  void ltdcBgSetFrameAddressI(LTDCDriver *ltdcp, void *bufferp);
  void ltdcBgSetFrameAddress(LTDCDriver *ltdcp, void *bufferp);
  void ltdcBgGetLayerI(LTDCDriver *ltdcp, ltdc_laycfg_t *cfgp);
  void ltdcBgGetLayer(LTDCDriver *ltdcp, ltdc_laycfg_t *cfgp);
  void ltdcBgSetConfigI(LTDCDriver *ltdcp, const ltdc_laycfg_t *cfgp);
  void ltdcBgSetConfig(LTDCDriver *ltdcp, const ltdc_laycfg_t *cfgp);

  /* Foreground layer methods.*/
  ltdc_flags_t ltdcFgGetEnableFlagsI(LTDCDriver *ltdcp);
  ltdc_flags_t ltdcFgGetEnableFlags(LTDCDriver *ltdcp);
  void ltdcFgSetEnableFlagsI(LTDCDriver *ltdcp, ltdc_flags_t flags);
  void ltdcFgSetEnableFlags(LTDCDriver *ltdcp, ltdc_flags_t flags);
  bool ltdcFgIsEnabledI(LTDCDriver *ltdcp);
  bool ltdcFgIsEnabled(LTDCDriver *ltdcp);
  void ltdcFgEnableI(LTDCDriver *ltdcp);
  void ltdcFgEnable(LTDCDriver *ltdcp);
  void ltdcFgDisableI(LTDCDriver *ltdcp);
  void ltdcFgDisable(LTDCDriver *ltdcp);
  bool ltdcFgIsPaletteEnabledI(LTDCDriver *ltdcp);
  bool ltdcFgIsPaletteEnabled(LTDCDriver *ltdcp);
  void ltdcFgEnablePaletteI(LTDCDriver *ltdcp);
  void ltdcFgEnablePalette(LTDCDriver *ltdcp);
  void ltdcFgDisablePaletteI(LTDCDriver *ltdcp);
  void ltdcFgDisablePalette(LTDCDriver *ltdcp);
  void ltdcFgSetPaletteColorI(LTDCDriver *ltdcp, uint8_t slot, ltdc_color_t c);
  void ltdcFgSetPaletteColor(LTDCDriver *ltdcp, uint8_t slot, ltdc_color_t c);
  void ltdcFgSetPaletteI(LTDCDriver *ltdcp, const ltdc_color_t colors[],
                         uint16_t length);
  void ltdcFgSetPalette(LTDCDriver *ltdcp, const ltdc_color_t colors[],
                        uint16_t length);
  ltdc_pixfmt_t ltdcFgGetPixelFormatI(LTDCDriver *ltdcp);
  ltdc_pixfmt_t ltdcFgGetPixelFormat(LTDCDriver *ltdcp);
  void ltdcFgSetPixelFormatI(LTDCDriver *ltdcp, ltdc_pixfmt_t fmt);
  void ltdcFgSetPixelFormat(LTDCDriver *ltdcp, ltdc_pixfmt_t fmt);
  bool ltdcFgIsKeyingEnabledI(LTDCDriver *ltdcp);
  bool ltdcFgIsKeyingEnabled(LTDCDriver *ltdcp);
  void ltdcFgEnableKeyingI(LTDCDriver *ltdcp);
  void ltdcFgEnableKeying(LTDCDriver *ltdcp);
  void ltdcFgDisableKeyingI(LTDCDriver *ltdcp);
  void ltdcFgDisableKeying(LTDCDriver *ltdcp);
  ltdc_color_t ltdcFgGetKeyingColorI(LTDCDriver *ltdcp);
  ltdc_color_t ltdcFgGetKeyingColor(LTDCDriver *ltdcp);
  void ltdcFgSetKeyingColorI(LTDCDriver *ltdcp, ltdc_color_t c);
  void ltdcFgSetKeyingColor(LTDCDriver *ltdcp, ltdc_color_t c);
  uint8_t ltdcFgGetConstantAlphaI(LTDCDriver *ltdcp);
  uint8_t ltdcFgGetConstantAlpha(LTDCDriver *ltdcp);
  void ltdcFgSetConstantAlphaI(LTDCDriver *ltdcp, uint8_t a);
  void ltdcFgSetConstantAlpha(LTDCDriver *ltdcp, uint8_t a);
  ltdc_color_t ltdcFgGetDefaultColorI(LTDCDriver *ltdcp);
  ltdc_color_t ltdcFgGetDefaultColor(LTDCDriver *ltdcp);
  void ltdcFgSetDefaultColorI(LTDCDriver *ltdcp, ltdc_color_t c);
  void ltdcFgSetDefaultColor(LTDCDriver *ltdcp, ltdc_color_t c);
  ltdc_blendf_t ltdcFgGetBlendingFactorsI(LTDCDriver *ltdcp);
  ltdc_blendf_t ltdcFgGetBlendingFactors(LTDCDriver *ltdcp);
  void ltdcFgSetBlendingFactorsI(LTDCDriver *ltdcp, ltdc_blendf_t bf);
  void ltdcFgSetBlendingFactors(LTDCDriver *ltdcp, ltdc_blendf_t bf);
  void ltdcFgGetWindowI(LTDCDriver *ltdcp, ltdc_window_t *windowp);
  void ltdcFgGetWindow(LTDCDriver *ltdcp, ltdc_window_t *windowp);
  void ltdcFgSetWindowI(LTDCDriver *ltdcp, const ltdc_window_t *windowp);
  void ltdcFgSetWindow(LTDCDriver *ltdcp, const ltdc_window_t *windowp);
  void ltdcFgSetInvalidWindowI(LTDCDriver *ltdcp);
  void ltdcFgSetInvalidWindow(LTDCDriver *ltdcp);
  void ltdcFgGetFrameI(LTDCDriver *ltdcp, ltdc_frame_t *framep);
  void ltdcFgGetFrame(LTDCDriver *ltdcp, ltdc_frame_t *framep);
  void ltdcFgSetFrameI(LTDCDriver *ltdcp, const ltdc_frame_t *framep);
  void ltdcFgSetFrame(LTDCDriver *ltdcp, const ltdc_frame_t *framep);
  void *ltdcFgGetFrameAddressI(LTDCDriver *ltdcp);
  void *ltdcFgGetFrameAddress(LTDCDriver *ltdcp);
  void ltdcFgSetFrameAddressI(LTDCDriver *ltdcp, void *bufferp);
  void ltdcFgSetFrameAddress(LTDCDriver *ltdcp, void *bufferp);
  void ltdcFgGetLayerI(LTDCDriver *ltdcp, ltdc_laycfg_t *cfgp);
  void ltdcFgGetLayer(LTDCDriver *ltdcp, ltdc_laycfg_t *cfgp);
  void ltdcFgSetConfigI(LTDCDriver *ltdcp, const ltdc_laycfg_t *cfgp);
  void ltdcFgSetConfig(LTDCDriver *ltdcp, const ltdc_laycfg_t *cfgp);

  /* Helper functions.*/
  size_t ltdcBitsPerPixel(ltdc_pixfmt_t fmt);
#if (TRUE == LTDC_USE_SOFTWARE_CONVERSIONS) || defined(__DOXYGEN__)
  ltdc_color_t ltdcFromARGB8888(ltdc_color_t c, ltdc_pixfmt_t fmt);
  ltdc_color_t ltdcToARGB8888(ltdc_color_t c, ltdc_pixfmt_t fmt);
#endif  /* LTDC_USE_SOFTWARE_CONVERSIONS */

#ifdef __cplusplus
}
#endif

#endif  /* STM32_LTDC_USE_LTDC */

#endif  /* HAL_STM32_LTDC_H_ */

/** @} */
