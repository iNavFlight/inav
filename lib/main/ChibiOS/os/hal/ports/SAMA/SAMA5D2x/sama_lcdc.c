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
 * @file    SAMA5D2x/sama_lcdc.c
 * @brief   SAMA LCDC support code.
 *
 * @addtogroup SAMA5D2x_LCDC
 * @{
 */

#include "hal.h"

#if (SAMA_USE_LCDC) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
/*
 * @brief    NO CACHE attribute
 */
#if !defined(NO_CACHE)
#define NO_CACHE                        __attribute__((section (".nocache")))
#endif
/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/
/*
 * @name    Configuration Macros
 * @{
 */
/*
 * @brief    Transfer Descriptor Fetch Enable
 */
#define LCDC_CTRL_DFETCH                (0x1u << 0)
/*
 * @brief    Channel Enable Register
 */
#define LCDC_CHER_CHEN                  (0x1u << 0)
/*
 * @brief    Channel Disable Register
 */
#define LCDC_CHDR_CHDIS                 (0x1u << 0)
/*
 * @brief    Update Overlay Attributes Enable Register
 */
#define LCDC_CHER_UPDATEEN              (0x1u << 1)
/*
 * @brief    Blender DMA Layer Enable
 */
#define LCDC_CFG_DMA                    (0x1u << 8)
/*
 * @brief    Blender Overlay Layer Enable
 */
#define LCDC_CFG_OVR                    (0x1u << 7)
/*
 * @brief    Pixel Stride
 */
#define LCDC_CFG_PSTRIDE_Pos            0
#define LCDC_CFG_PSTRIDE_Msk            (0xffffffffu << LCDC_CFG_PSTRIDE_Pos)
#define LCDC_CFG_PSTRIDE(value)         ((LCDC_CFG_PSTRIDE_Msk & ((value) << \
                                                      LCDC_CFG_PSTRIDE_Pos)))
/*
 * @brief    Horizontal Stride
 */
#define LCDC_CFG_XSTRIDE_Pos            0
#define LCDC_CFG_XSTRIDE_Msk            (0xffffffffu << LCDC_CFG_XSTRIDE_Pos)
#define LCDC_CFG_XSTRIDE(value)         ((LCDC_CFG_XSTRIDE_Msk & ((value) << \
                                                      LCDC_CFG_XSTRIDE_Pos)))
/*
 * @brief    Hardware Rotation Optimization Disable
 */
#define LCDC_CFG_ROTDIS                 (0x1u << 12)

/*
 * @brief    Horizontal Window Position
 */
#define LCDC_CFG_XPOS_Pos 0
#define LCDC_CFG_XPOS_Msk (0x7ffu << LCDC_CFG_XPOS_Pos)
#define LCDC_CFG_XPOS(value) ((LCDC_CFG_XPOS_Msk & ((value) << LCDC_CFG_XPOS_Pos)))

/*
 * @brief    Vertical Window Position
 */
#define LCDC_CFG_YPOS_Pos 16
#define LCDC_CFG_YPOS_Msk (0x7ffu << LCDC_CFG_YPOS_Pos)
#define LCDC_CFG_YPOS(value) ((LCDC_CFG_YPOS_Msk & ((value) << LCDC_CFG_YPOS_Pos)))

/*
 * @brief    Horizontal Window Size
 */
#define LCDC_CFG_XSIZE_Pos 0
#define LCDC_CFG_XSIZE_Msk (0x7ffu << LCDC_CFG_XSIZE_Pos)
#define LCDC_CFG_XSIZE(value) ((LCDC_CFG_XSIZE_Msk & ((value) << LCDC_CFG_XSIZE_Pos)))

/*
 * @brief    Vertical Window Size
 */
#define LCDC_CFG_YSIZE_Pos 16
#define LCDC_CFG_YSIZE_Msk (0x7ffu << LCDC_CFG_YSIZE_Pos)
#define LCDC_CFG_YSIZE(value) ((LCDC_CFG_YSIZE_Msk & ((value) << LCDC_CFG_YSIZE_Pos)))

/*
 * @brief    Horizontal image Size in Memory
 */
#define LCDC_CFG_XMEMSIZE_Pos 0
#define LCDC_CFG_XMEMSIZE_Msk (0x7ffu << LCDC_CFG_XMEMSIZE_Pos)
#define LCDC_CFG_XMEMSIZE(value) ((LCDC_CFG_XMEMSIZE_Msk & ((value) << LCDC_CFG_XMEMSIZE_Pos)))

/*
 * @brief    Vertical image Size in Memory
 */
#define LCDC_CFG_YMEMSIZE_Pos 16
#define LCDC_CFG_YMEMSIZE_Msk (0x7ffu << LCDC_CFG_YMEMSIZE_Pos)
#define LCDC_CFG_YMEMSIZE(value) ((LCDC_CFG_YMEMSIZE_Msk & ((value) << LCDC_CFG_YMEMSIZE_Pos)))

/** @} */

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
LCDCDriver LCDCD0;

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/
/**
 * @brief   DMA Channel Descriptor.
 */
typedef struct {
  /**
   * @brief   Frame Buffer base address register.
   */
  uint32_t                  addr;
  /**
   * @brief   Transfer Control register.
   */
  uint32_t                  ctrl;
  /**
   * @brief   Next Descriptor Address register.
   */
  uint32_t                  next;
} lcdc_dma_descriptor_t;

/* Variable layer data */
typedef struct {
  lcdc_dma_descriptor_t *dma_desc;
  lcdc_dma_descriptor_t *dma_u_desc;
  lcdc_dma_descriptor_t *dma_v_desc;
  void                  *buffer;
  uint8_t                bpp;
  uint8_t                num_colors;
} layerdata_t;

/*
 * @brief  Hardware info about the layers
 */
typedef struct {
  layerdata_t        *data;
  bool                stride_supported;
  /* regs: _ER, _DR, _SR, _IER, _IDR, _IMR, _ISR */
  volatile uint32_t  *reg_enable;
  /* regs: blender */
  volatile uint32_t  *reg_blender;
  /* _HEAD, _ADDRESS, _CONTROL, _NEXT */
  volatile uint32_t  *reg_dma_head;
  /* _HEAD, _ADDRESS, _CONTROL, _NEXT */
  volatile uint32_t  *reg_dma_u_head;
  /* _HEAD, _ADDRESS, _CONTROL, _NEXT */
  volatile uint32_t  *reg_dma_v_head;
  /* regs: _CFG0, _CFG1 (RGB mode...) */
  volatile uint32_t  *reg_cfg;
  /* X Y register, W H register */
  volatile uint32_t  *reg_win;
  /* regs: stride */
  volatile uint32_t  *reg_stride;
  /* regs: RGB Default, RGB Key, RGB Mask */
  volatile uint32_t  *reg_color;
  /* regs: scale */
  volatile uint32_t  *reg_scale;
  /* regs: CLUT */
  volatile uint32_t  *reg_clut;
} layerinfo_t;

/* Base Layer */
static layerdata_t lcdd_base;
/* OVR1 Layer */
static layerdata_t lcdd_ovr1;
/* OVR2 Layer */
static layerdata_t lcdd_ovr2;
/* HEO Layer */
static layerdata_t lcdd_heo;
/* HCC Layer */
static layerdata_t lcdd_hcc;

/*
 * @brief    DMA descriptor
 * @note The DMA Channel Descriptor (DSCR) must be aligned on a 64-bit boundary.
 */
ALIGNED_VAR(8)
/* DMA descriptor for Base Layer */
NO_CACHE static lcdc_dma_descriptor_t base_dma_desc;

ALIGNED_VAR(8)
/* DMA descriptor for OVR1 Layer */
NO_CACHE static lcdc_dma_descriptor_t ovr1_dma_desc;

ALIGNED_VAR(8)
/* DMA descriptor for OVR2 Layer */
NO_CACHE static lcdc_dma_descriptor_t ovr2_dma_desc;

ALIGNED_VAR(8)
/* DMA descriptor for HEO Layer */
NO_CACHE static lcdc_dma_descriptor_t heo_dma_desc;
ALIGNED_VAR(8)
/* DMA descriptor for HEO U-UV Layer */
NO_CACHE static lcdc_dma_descriptor_t heo_dma_u_desc;
ALIGNED_VAR(8)
/* DMA descriptor for HEO V Layer */
NO_CACHE static lcdc_dma_descriptor_t heo_dma_v_desc;

ALIGNED_VAR(8)
/* DMA descriptor for HCC Layer */
NO_CACHE static lcdc_dma_descriptor_t hcc_dma_desc;

/**
 * @brief  Information about layers
 */
static const layerinfo_t lcdd_layers[] = {
  /* 0: LCDD_CONTROLLER */
  {
    .stride_supported = false,
    .reg_enable = &LCDC->LCDC_LCDEN,
  },

  /* 1: LCDD_BASE */
  {
    .data = &lcdd_base,
    .stride_supported = false,
    .reg_enable = &LCDC->LCDC_BASECHER,
    .reg_blender = &LCDC->LCDC_BASECFG4,
    .reg_dma_head = &LCDC->LCDC_BASEHEAD,
    .reg_cfg = &LCDC->LCDC_BASECFG0,
    .reg_stride = &LCDC->LCDC_BASECFG2,
    .reg_color = &LCDC->LCDC_BASECFG3,
    .reg_clut = &LCDC->LCDC_BASECLUT[0]
  },

  /* 2: LCDD_OVR1 */
  {
    .data = &lcdd_ovr1,
    .stride_supported = true,
    .reg_enable = &LCDC->LCDC_OVR1CHER,
    .reg_blender = &LCDC->LCDC_OVR1CFG9,
    .reg_dma_head = &LCDC->LCDC_OVR1HEAD,
    .reg_cfg = &LCDC->LCDC_OVR1CFG0,
    .reg_win = &LCDC->LCDC_OVR1CFG2,
    .reg_stride = &LCDC->LCDC_OVR1CFG4,
    .reg_color = &LCDC->LCDC_OVR1CFG6,
    .reg_clut = &LCDC->LCDC_OVR1CLUT[0],
  },

  /* 3: LCDD_HEO */
  {
    .data = &lcdd_heo,
    .stride_supported = true,
    .reg_enable = &LCDC->LCDC_HEOCHER,
    .reg_blender = &LCDC->LCDC_HEOCFG12,
    .reg_dma_head = &LCDC->LCDC_HEOHEAD,
    .reg_dma_u_head = &LCDC->LCDC_HEOUHEAD,
    .reg_dma_v_head = &LCDC->LCDC_HEOVHEAD,
    .reg_cfg = &LCDC->LCDC_HEOCFG0,
    .reg_win = &LCDC->LCDC_HEOCFG2,
    .reg_stride = &LCDC->LCDC_HEOCFG5,
    .reg_color = &LCDC->LCDC_HEOCFG9,
    .reg_scale = &LCDC->LCDC_HEOCFG13,
    .reg_clut = &LCDC->LCDC_HEOCLUT[0],
  },

  /* 4: LCDD_OVR2 */
  {
    .data = &lcdd_ovr2,
    .stride_supported = true,
    .reg_enable = &LCDC->LCDC_OVR2CHER,
    .reg_blender = &LCDC->LCDC_OVR2CFG9,
    .reg_dma_head = &LCDC->LCDC_OVR2HEAD,
    .reg_cfg = &LCDC->LCDC_OVR2CFG0,
    .reg_win = &LCDC->LCDC_OVR2CFG2,
    .reg_stride = &LCDC->LCDC_OVR2CFG4,
    .reg_color = &LCDC->LCDC_OVR2CFG6,
    .reg_clut = &LCDC->LCDC_OVR2CLUT[0],
  }
  ,
  /* 5: N/A */
  {
    .data = NULL,
  },

  /* 6: LCDD_CUR */
  {
    .data = &lcdd_hcc,
    .stride_supported = false,
  },
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/*
 * @brief    Clear DMA channel descriptor
 *
 * @param[in] descp    pointer to lcdc_dma_descriptor
 * @param[in] dma_regp pointer to LCDC leyer register
 *
 * @notapi
 */
static void clear_dma_desc(lcdc_dma_descriptor_t *descp, volatile uint32_t *dma_regp) {
  /* Modify descriptor */
  if (descp) {
    descp->ctrl &= ~LCDC_CTRL_DFETCH;
    descp->next = (uint32_t)descp;
    //cacheCleanRegion(descp, sizeof(lcdc_dma_descriptor_t));
  }

  /* Modify registers */
  dma_regp[2] &= ~LCDC_CTRL_DFETCH;
  dma_regp[3] = (uint32_t)descp;
}

/**
 * @brief   Computing scaling factor.
 *
 * @param[in] layerp      pointer to a layerinfo_t struct
 * @param[out] xfactorp   pointer to xfactor scaling factor
 * @param[out] yfactorp   pointer to yfactor scaling factor
 *
 * @notapi
 */
static void compute_scaling_factors(const layerinfo_t *layerp,
    uint16_t* xfactorp, uint16_t* yfactorp)
{
  uint16_t xmemsize, ymemsize;
  uint16_t xsize, ysize;

#ifdef LCDC_HEOCFG41_XPHIDEF
  uint16_t xfactor_1st, yfactor_1st;
#endif

  xmemsize = (layerp->reg_win[2] & LCDC_CFG_XMEMSIZE_Msk) >> LCDC_CFG_XMEMSIZE_Pos;
  ymemsize = (layerp->reg_win[2] & LCDC_CFG_YMEMSIZE_Msk) >> LCDC_CFG_YMEMSIZE_Pos;
  xsize = (layerp->reg_win[1] & LCDC_CFG_XSIZE_Msk) >> LCDC_CFG_XSIZE_Pos;
  ysize = (layerp->reg_win[1] & LCDC_CFG_YSIZE_Msk) >> LCDC_CFG_YSIZE_Pos;

#ifdef LCDC_HEOCFG41_XPHIDEF
  /* we assume that XPHIDEF & YPHIDEF are 0 */
  xfactor_1st = (2048 * xmemsize / xsize) + 1;
  yfactor_1st = (2048 * ymemsize / ysize) + 1;

  if ((xfactor_1st * xsize / 2048) > xmemsize)
    *xfactorp = xfactor_1st - 1;
  else
    *xfactorp = xfactor_1st;

  if ((yfactor_1st * ysize / 2048) > ymemsize)
    *yfactorp = yfactor_1st - 1;
  else
    *yfactorp = yfactor_1st;
#else
  *xfactorp = 1024 * (xmemsize + 1) / (xsize + 1);
  *yfactorp = 1024 * (ymemsize + 1) / (ysize + 1);
#endif
}

/**
 * @brief   Configures LCDC layers according to configuration struct.
 *
 * @param[in] listp     pointer to a LCDCLayerConfig array
 * @param[in] length    length of array
 *
 * @notapi
 */
void layer_config(LCDCLayerConfig *listp, size_t length) {
  uint8_t i;
  uint8_t bpp_bit;
  uint8_t bpp;
  uint32_t index;

  uint32_t padding = 0;
  uint32_t src_w, src_h, img_w, img_h;
  uint32_t bits_per_row, bytes_per_row;

  LCDCLayerConfig *layerp;

  for (i = 0; i < length; i++) {
    index = listp[i].layer_id;

    osalDbgAssert((index != LCDD_CONTROLLER) || (index != LCDD_CUR), "This is not a real layer");

    layerp = &listp[i];
    uint16_t w, h, x, y;

    bpp = layerp->bpp;
    w = layerp->width;
    h = layerp->height;
    x = layerp->x_pos;
    y = layerp->y_pos;
    img_w = layerp->w_img;
    img_h = layerp->h_img;

    /* Bpp settings */
    lcdd_layers[index].reg_cfg[1] = layerp->bpp;
    bpp = bpp >> 4;

    if (bpp == 1 || bpp < 5) {
      bpp_bit = 16;
    }
    else if (bpp == 5 || bpp == 6) {
      bpp_bit = 18;
    }
    else if (bpp == 7 || bpp == 8) {
      bpp_bit = 19;
    }
    else if (bpp == 9 || bpp == 10) {
      bpp_bit = 24;
    }
    else if (bpp == 11) {
      bpp_bit = 25;
    }
    else if (bpp == 12 || bpp == 13) {
      bpp_bit = 32;
    }
    else {
      bpp_bit = 12;
    }

    /* Set display buffer & mode */
    bits_per_row = img_w * bpp_bit;
    bytes_per_row = bits_per_row >> 3;

    if (bits_per_row & 0x7) {
      bytes_per_row++;
    }
    if (bytes_per_row & 0x3) {
      padding = 4 - (bytes_per_row & 0x3);
    }
    /* No rotation optimization */
    lcdd_layers[index].reg_cfg[0] |= LCDC_CFG_ROTDIS;

    /* Configure PSTRIDE if supported */
    if (lcdd_layers[index].stride_supported)
      lcdd_layers[index].reg_stride[1] = LCDC_CFG_PSTRIDE(0);
    /* Configure XSTRIDE if supported */
    lcdd_layers[index].reg_stride[0] = LCDC_CFG_XSTRIDE(padding);

    /* Set window & position */
    if (lcdd_layers[index].reg_win) {

      /* Re - calculate to eliminate hardware overflow */
      if (x + w > LCDCD0.config->width) {
        w = LCDCD0.config->width - x;
      }
      if (y + h > LCDCD0.config->height) {
        h = LCDCD0.config->height - y;
      }

      if (w == 0)
        w++;

      if (h == 0)
        h++;

      lcdd_layers[index].reg_win[0] = LCDC_CFG_XPOS(x) | LCDC_CFG_YPOS(y);
      lcdd_layers[index].reg_win[1] = LCDC_CFG_XSIZE(w - 1) | LCDC_CFG_YSIZE(h - 1);
    }

    /* Scaling setup, only HEO layer has scaling register */
    if (lcdd_layers[index].reg_win && lcdd_layers[index].reg_scale) {
      src_w = img_w;
      src_h = img_h;

      lcdd_layers[index].reg_win[2] = LCDC_CFG_XMEMSIZE(src_w - 1) |
                                       LCDC_CFG_YMEMSIZE(src_h - 1);
      /* Scaled */
      if (w != src_w || h != src_h) {
        uint16_t scale_w, scale_h;
        compute_scaling_factors(&lcdd_layers[index], &scale_w, &scale_h);
        lcdd_layers[index].reg_scale[0] = LCDC_HEOCFG13_YFACTOR(scale_h) |
                                           LCDC_HEOCFG13_XFACTOR(scale_w) |
                                           LCDC_HEOCFG13_SCALEN;
      }
      /* Disable scaling */
      else {
        lcdd_layers[index].reg_scale[0] = 0;
      }
    }

    /* Configure Descriptor */
    lcdd_layers[index].data->dma_desc->addr = (uint32_t)layerp->buffer;
    lcdd_layers[index].data->dma_desc->ctrl = LCDC_CTRL_DFETCH;
    lcdd_layers[index].data->dma_desc->next = (uint32_t)lcdd_layers[index].data->dma_desc;

    lcdd_layers[index].reg_dma_head[1] = (uint32_t)lcdd_layers[index].data->dma_desc->addr;
    lcdd_layers[index].reg_dma_head[2] = LCDC_CTRL_DFETCH;
    lcdd_layers[index].reg_dma_head[3] = (uint32_t)lcdd_layers[index].data->dma_desc;

    /* Configure layer */
    lcdd_layers[index].reg_enable[0] = LCDC_CHER_UPDATEEN;
    lcdd_layers[index].reg_blender[0] |= LCDC_CFG_DMA | LCDC_CFG_OVR;
  }
}

/**
 * @brief   Enable Display.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 */
static void lcdc_on(LCDCDriver *lcdcp) {

  uint32_t pixel_clock = lcdcp->config->framerate;
  pixel_clock *= lcdcp->config->timing_hpw + lcdcp->config->timing_hbp +
                 lcdcp->config->width + lcdcp->config->timing_hfp;
  pixel_clock *= lcdcp->config->timing_vpw + lcdcp->config->timing_vbp +
                 lcdcp->config->height + lcdcp->config->timing_vfp;

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  /* Configure LCD timing parameters, signal polarity and clock period. */
  if( LCDC->LCDC_LCDCFG0 & LCDC_LCDCFG0_CLKSEL) {
    LCDC->LCDC_LCDCFG0 = LCDC_LCDCFG0_CLKDIV((SAMA_MCK * 2) / pixel_clock - 2) |
                         LCDC_LCDCFG0_CGDISHEO | LCDC_LCDCFG0_CGDISOVR1 |
                         LCDC_LCDCFG0_CGDISOVR2 | LCDC_LCDCFG0_CGDISBASE |
                         LCDC_LCDCFG0_CLKPWMSEL | LCDC_LCDCFG0_CLKSEL;
  }
  else {
    LCDC->LCDC_LCDCFG0 = LCDC_LCDCFG0_CLKDIV(SAMA_MCK / pixel_clock - 2) |
                         LCDC_LCDCFG0_CGDISBASE | LCDC_LCDCFG0_CGDISHEO |
                         LCDC_LCDCFG0_CLKPWMSEL;
  }

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  LCDC->LCDC_LCDCFG1 = LCDC_LCDCFG1_VSPW(lcdcp->config->timing_vpw - 1) |
                       LCDC_LCDCFG1_HSPW(lcdcp->config->timing_hpw - 1);

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  LCDC->LCDC_LCDCFG2 = LCDC_LCDCFG2_VBPW(lcdcp->config->timing_vbp) |
                       LCDC_LCDCFG2_VFPW(lcdcp->config->timing_vfp - 1);

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  LCDC->LCDC_LCDCFG3 = LCDC_LCDCFG3_HBPW(lcdcp->config->timing_hbp - 1) |
                       LCDC_LCDCFG3_HFPW(lcdcp->config->timing_hfp - 1);

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  LCDC->LCDC_LCDCFG4 = LCDC_LCDCFG4_RPF(lcdcp->config->height - 1) |
                       LCDC_LCDCFG4_PPL(lcdcp->config->width - 1);

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  LCDC->LCDC_LCDCFG5 = LCDC_LCDCFG5_GUARDTIME(30) | LCDC_LCDCFG5_MODE_OUTPUT_24BPP |
                       LCDC_LCDCFG5_DISPDLY | LCDC_LCDCFG5_VSPDLYS | LCDC_LCDCFG5_VSPOL |
                       LCDC_LCDCFG5_HSPOL;

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  LCDC->LCDC_LCDCFG6 = LCDC_LCDCFG6_PWMCVAL(0xF0) | LCDC_LCDCFG6_PWMPOL |
                       LCDC_LCDCFG6_PWMPS(6);

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));

  /* Enable the Pixel Clock. */
  LCDC->LCDC_LCDEN = LCDC_LCDEN_CLKEN;

  /* Poll to check that clock is running. */
  while (!(LCDC->LCDC_LCDSR & LCDC_LCDSR_CLKSTS));

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  /* Enable Horizontal and Vertical Synchronization. */
  LCDC->LCDC_LCDEN = LCDC_LCDEN_SYNCEN;
  /* Poll to check that the synchronization is up. */
  while (!(LCDC->LCDC_LCDSR & LCDC_LCDSR_LCDSTS));

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));
  /* Enable the display power signal. */
  LCDC->LCDC_LCDEN = LCDC_LCDEN_DISPEN;
  /* Poll to check that the power signal is activated. */
  while (!(LCDC->LCDC_LCDSR & LCDC_LCDSR_DISPSTS));

  /* Wait for clock domain synchronization to be complete. */
  while ((LCDC->LCDC_LCDSR & LCDC_LCDSR_SIPSTS));

  /* Enable backlight */
  LCDC->LCDC_LCDEN = LCDC_LCDEN_PWMEN;
}

/**
 * @brief   Disable Display.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 */
static void lcdc_off(void) {

  /* Disable all DMA channel descriptors */
  clear_dma_desc(&base_dma_desc, &LCDC->LCDC_BASEADDR);
  clear_dma_desc(&ovr1_dma_desc, &LCDC->LCDC_OVR1ADDR);
  clear_dma_desc(&ovr2_dma_desc, &LCDC->LCDC_OVR2ADDR);
  clear_dma_desc(&heo_dma_desc, &LCDC->LCDC_HEOADDR);
  clear_dma_desc(&heo_dma_u_desc, &LCDC->LCDC_HEOUADDR);
  clear_dma_desc(&heo_dma_v_desc, &LCDC->LCDC_HEOVADDR);

  /* Disable DMA channels */
  LCDC->LCDC_BASECHDR = LCDC_BASECHDR_CHDIS;
  LCDC->LCDC_OVR1CHDR = LCDC_OVR1CHDR_CHDIS;
  LCDC->LCDC_OVR2CHDR = LCDC_OVR2CHDR_CHDIS;
  LCDC->LCDC_HEOCHDR = LCDC_HEOCHDR_CHDIS;
  LCDC->LCDC_BASECFG4 = 0;

  /* Poll CHSR until the channel is successfully disabled. */
  while (LCDC->LCDC_BASECHSR & LCDC_BASECHSR_CHSR);
  while (LCDC->LCDC_OVR1CHSR & LCDC_OVR1CHSR_CHSR);
  while (LCDC->LCDC_OVR2CHSR & LCDC_OVR1CHSR_CHSR);
  while (LCDC->LCDC_HEOCHSR & LCDC_HEOCHSR_CHSR);

  /* Disable backlight */
  LCDC->LCDC_LCDDIS = LCDC_LCDDIS_PWMDIS;
  /* Poll PWMSTS: field of the LCDC_LCDSR register to verify that the PWM
     is no activated. */
  while (LCDC->LCDC_LCDSR & LCDC_LCDSR_PWMSTS);

  /* Disable the DISP signal. */
  LCDC->LCDC_LCDDIS = LCDC_LCDDIS_DISPDIS;
  /* Poll DISPSTS field of the LCDC_LCDSR register to verify that the DISP
     is no longer activated. */
  while (LCDC->LCDC_LCDSR & LCDC_LCDSR_DISPSTS);

  /* Disable the hsync and vsync signals. */
  LCDC->LCDC_LCDDIS = LCDC_LCDDIS_SYNCDIS;
  /* Poll LCDSTS field of the LCDC_LCDSR register to check that the
     synchronization is off. */
  while (LCDC->LCDC_LCDSR & LCDC_LCDSR_LCDSTS);

  /* Disable the Pixel clock. */
  LCDC->LCDC_LCDDIS = LCDC_LCDDIS_CLKDIS;
  /* Poll CLKSTS field of the LCDC_LCDSR register to check that Pixel Clock
     is disabled. */
  while (LCDC->LCDC_LCDSR & LCDC_LCDSR_CLKSTS);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level LCDC driver initialization.
 *
 * @notapi
 */
void lcdc_lld_init(void) {
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX0, ID_LCDC, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */

  /* Driver initialization.*/
  lcdcObjectInit(&LCDCD0);
  LCDCD0.lcdc = LCDC;

  /* Reset layer information */
  lcdd_base.bpp = 0;
  lcdd_base.buffer = NULL;
  lcdd_base.dma_desc = &base_dma_desc;

  lcdd_ovr1.bpp = 0;
  lcdd_ovr1.buffer = NULL;
  lcdd_ovr1.dma_desc = &ovr1_dma_desc;

  lcdd_ovr2.bpp = 0;
  lcdd_ovr2.buffer = NULL;
  lcdd_ovr2.dma_desc = &ovr2_dma_desc;

  lcdd_heo.bpp = 0;
  lcdd_heo.buffer = NULL;
  lcdd_heo.dma_desc = &heo_dma_desc;
  lcdd_heo.dma_u_desc = &heo_dma_u_desc;
  lcdd_heo.dma_v_desc = &heo_dma_v_desc;

  lcdd_hcc.bpp = 0;
  lcdd_base.buffer = NULL;
  lcdd_hcc.dma_desc = &hcc_dma_desc;

  /* Disable LCD controller */
  lcdc_off();

  /* Timing Engine Configuration */

  /* Disable interrupt */
  LCDC->LCDC_LCDIDR = 0xFFFFFFFF;

  /* Base */
  LCDC->LCDC_BASECFG0 = LCDC_BASECFG0_DLBO | LCDC_BASECFG0_BLEN_AHB_INCR16;

  /* Overlay 1, GA 0xFF */
  LCDC->LCDC_OVR1CFG0 = LCDC_OVR1CFG0_DLBO | LCDC_OVR1CFG0_BLEN_AHB_BLEN_INCR16 |
                               LCDC_OVR1CFG0_ROTDIS;

  LCDC->LCDC_OVR1CFG9 = LCDC_OVR1CFG9_GA(0xFF) | LCDC_OVR1CFG9_GAEN;

  /* Overlay 2, GA 0xFF */
  LCDC->LCDC_OVR2CFG0 = LCDC_OVR2CFG0_DLBO | LCDC_OVR2CFG0_BLEN_AHB_INCR16 |
                               LCDC_OVR2CFG0_ROTDIS;
  LCDC->LCDC_OVR2CFG9 = LCDC_OVR2CFG9_GA(0xFF) | LCDC_OVR2CFG9_GAEN;

  /* High End Overlay, GA 0xFF */
  LCDC->LCDC_HEOCFG0 =  LCDC_HEOCFG0_DLBO | LCDC_HEOCFG0_BLEN_AHB_BLEN_INCR16 |
                               LCDC_HEOCFG0_ROTDIS;
  LCDC->LCDC_HEOCFG12 = LCDC_HEOCFG12_GA(0xFF) | LCDC_HEOCFG12_GAEN;
  LCDC->LCDC_HEOCFG14 = LCDC_HEOCFG14_CSCRY(0x94) | LCDC_HEOCFG14_CSCRU(0xCC) |
                               LCDC_HEOCFG14_CSCRV(0) | LCDC_HEOCFG14_CSCYOFF;
  LCDC->LCDC_HEOCFG15 = LCDC_HEOCFG15_CSCGY(0x94) | LCDC_HEOCFG15_CSCGU(0x387) |
                               LCDC_HEOCFG15_CSCGV(0x3CD) | LCDC_HEOCFG15_CSCUOFF;
  LCDC->LCDC_HEOCFG16 = LCDC_HEOCFG16_CSCBY(0x94)| LCDC_HEOCFG16_CSCBU(0) |
                               LCDC_HEOCFG16_CSCBV(0x102) | LCDC_HEOCFG16_CSCVOFF;
}

/**
 * @brief   Configures and activates the LCDC peripheral.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 *
 * @notapi
 */
void lcdc_lld_start(LCDCDriver *lcdcp) {

  /* Enable the LCDC peripheral clock. */
  pmcEnableLCDC();

  /* Configure overlays */
  layer_config(lcdcp->config->listp, lcdcp->config->length);

}

/**
 * @brief   Deactivates the LCDC peripheral.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 *
 * @notapi
 */
void lcdc_lld_stop(LCDCDriver *lcdcp) {

  if (lcdcp->state == LCDC_READY) {

    /* Disable display. */
    lcdc_off();

    /* Disable the LCDC clock. */
    pmcDisableLCDC();
  }
}

/**
 *
 * @brief   Initializes the standard part of a @p LCDCDriver structure.
 *
 * @param[out] lcdcp  pointer to a @p LCDCDriver object
 *
 * @init
 */
void lcdcObjectInit(LCDCDriver *lcdcp) {
  lcdcp->state = LCDC_STOP;
  lcdcp->config = NULL;
}

/**
 * @brief   LCDC driver initialization.
 *
 * @notapi
 */
void lcdcInit(void) {

  lcdc_lld_init();
}

/**
 * @brief   Configures and activates the LCDC peripheral.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 * @param[in] configp     pointer to the LCDCConfig struct
 *
 * @api
 */
void lcdcStart(LCDCDriver *lcdcp, const LCDCConfig *configp) {

 osalDbgCheck(lcdcp != NULL);

  osalSysLock();
  osalDbgAssert((lcdcp->state == LCDC_STOP) , "invalid state");
  lcdcp->config = configp;
  lcdc_lld_start(lcdcp);
  lcdcp->state = LCDC_READY;
  osalSysUnlock();

  /* Enable display. */
  lcdc_on(lcdcp);
}

/**
 * @brief   Deactivates the LCDC peripheral.
 *
 * @param[in] lcdcp    pointer to the @p LCDCDriver object
 *
 * @api
 */
void lcdcStop(LCDCDriver *lcdcp) {

  osalDbgCheck(lcdcp != NULL);

  osalSysLock();
  osalDbgAssert((lcdcp->state == LCDC_READY), "invalid state");

  lcdc_lld_stop(lcdcp);
  lcdcp->state = LCDC_STOP;
  osalSysUnlock();
}

void lcdcShowLayer(LCDCDriver *lcdcp, uint8_t id, bool enable) {
  (void)lcdcp;

  if(enable) {
    lcdd_layers[id].reg_enable[0] = LCDC_CHER_CHEN;
  }
  else {
    lcdd_layers[id].reg_enable[1] = LCDC_CHDR_CHDIS;
  }
}

/*
 * @brief    brief Set the backlight of the LCD.
 *
 * param[in] level   Backlight brightness level [1..255],
 *                   255 means maximum brightness.
 *
 * @api
 */
void lcdcSetBacklight(uint32_t level) {
  uint32_t cfg = LCDC->LCDC_LCDCFG6 & ~LCDC_LCDCFG6_PWMCVAL_Msk;
  LCDC->LCDC_LCDCFG6 = cfg | LCDC_LCDCFG6_PWMCVAL(level);
}

#if (TRUE == LCDC_USE_MUTUAL_EXCLUSION)

/**
 * @brief   Gains exclusive access to the LCDC module.
 * @details This function tries to gain ownership to the LCDC module, if the
 *          module is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p LCDC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 *
 * @sclass
 */
void lcdcAcquireBusS(LCDCDriver *lcdcp) {

  osalDbgCheckClassS();
  osalDbgCheck(lcdcp == &LCDCD0);

#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxLockS(&lcdcp->lock);
#else
  chSemWaitS(&lcdcp->lock);
#endif
}

/**
 * @brief   Gains exclusive access to the LCDC module.
 * @details This function tries to gain ownership to the LTDC module, if the
 *          module is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p LCDC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 *
 * @api
 */
void lcdcAcquireBus(LCDCDriver *lcdcp) {

  osalSysLock();
  lcdcAcquireBusS(lcdcp);
  osalSysUnlock();
}

/**
 * @brief   Releases exclusive access to the LCDC module.
 * @pre     In order to use this function the option
 *          @p LCDC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 *
 * @sclass
 */
void lcdcReleaseBusS(LCDCDriver *lcdcp) {

  osalDbgCheckClassS();
  osalDbgCheck(lcdcp == &LCDCD0);

#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxUnlockS(&lcdcp->lock);
#else
  chSemSignalI(&lcdcp->lock);
#endif
}

/**
 * @brief   Releases exclusive access to the LCDC module.
 * @pre     In order to use this function the option
 *          @p LCDC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] lcdcp     pointer to the @p LCDCDriver object
 *
 * @api
 */
void lcdcReleaseBus(LCDCDriver *lcdcp) {

  osalSysLock();
  lcdcReleaseBusS(lcdcp);
  osalSysUnlock();
}

#endif  /* LCDC_USE_MUTUAL_EXCLUSION */

#endif /* SAMA_USE_LCDC == TRUE */

/** @} */
