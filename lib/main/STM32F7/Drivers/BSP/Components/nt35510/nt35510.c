/**
  ******************************************************************************
  * @file    nt35510.c
  * @author  MCD Application Team
  * @brief   This file provides the LCD Driver for Frida Techshine 3K138 (WVGA)
  *          DSI LCD Display NT35510.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "nt35510.h"
#include <stdio.h>
#include <stdarg.h>


/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @defgroup NT35510 NT35510
  * @brief     This file provides a set of functions needed to drive the
  *            NT35510 IC display driver.
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup NT35510_Private_Constants NT35510 Private Constants
  * @{
  */

/*
 * @brief Constant tables of register settings used to transmit DSI
 * command packets as power up initialization sequence of the Frida 3K138
 */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @defgroup NT35510_Exported_Variables
  * @{
  */

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/
/** @defgroup NT35510_Exported_Functions OTM8009A Exported Functions
  * @{
  */

/**
  * @brief  Initializes the LCD Frida display part by communication in DSI mode in Video Mode
  *         with IC Display Driver NT35510 (see IC Driver specification for more information).
  * @param  hdsi_eval : pointer on DSI configuration structure
  * @param  hdsivideo_handle : pointer on DSI video mode configuration structure
  * @retval Status
  */
uint8_t NT35510_Init(uint32_t ColorCoding, uint32_t orientation)
{
  NT35510_IO_Delay(120);

/* ************************************************************************** */
/* Proprietary Initialization                                                 */
/* ************************************************************************** */
  const uint8_t nt35510_reg[]   = {0x55, 0xAA, 0x52, 0x08, 0x01, 0xF0};
  const uint8_t nt35510_reg1[]  = {0x03, 0x03, 0x03, 0xB0};
  const uint8_t nt35510_reg2[]  = {0x46, 0x46, 0x46, 0xB6};
  const uint8_t nt35510_reg3[]  = {0x03, 0x03, 0x03, 0xB1};
  const uint8_t nt35510_reg4[]  = {0x36, 0x36, 0x36, 0xB7};
  const uint8_t nt35510_reg5[]  = {0x00, 0x00, 0x02, 0xB2};
  const uint8_t nt35510_reg6[]  = {0x26, 0x26, 0x26, 0xB8};
  const uint8_t nt35510_reg7[]  = {0xBF, 0x01};
  const uint8_t nt35510_reg8[]  = {0x09, 0x09, 0x09, 0xB3};
  const uint8_t nt35510_reg9[]  = {0x36, 0x36, 0x36, 0xB9};
  const uint8_t nt35510_reg10[] = {0x08, 0x08, 0x08, 0xB5};
  const uint8_t nt35510_reg12[] = {0x26, 0x26, 0x26, 0xBA};
  const uint8_t nt35510_reg13[] = {0x00, 0x80, 0x00, 0xBC};
  const uint8_t nt35510_reg14[] = {0x00, 0x80, 0x00, 0xBD};
  const uint8_t nt35510_reg15[] = {0x00, 0x50, 0xBE};
  const uint8_t nt35510_reg16[] = {0x55, 0xAA, 0x52, 0x08, 0x00, 0xF0};
  const uint8_t nt35510_reg17[] = {0xFC, 0x00, 0xB1};
  const uint8_t nt35510_reg18[] = {0xB6, 0x03};
  const uint8_t nt35510_reg19[] = {0xB5, 0x50};
  const uint8_t nt35510_reg20[] = {0x00, 0x00, 0xB7};
  const uint8_t nt35510_reg21[] = {0x01, 0x02, 0x02, 0x02, 0xB8};
  const uint8_t nt35510_reg22[] = {0x00, 0x00, 0x00, 0xBC};
  const uint8_t nt35510_reg23[] = {0x03, 0x00, 0x00, 0xCC};
  const uint8_t nt35510_reg24[] = {0xBA, 0x01};
  const uint8_t nt35510_madctl_portrait[] = {NT35510_CMD_MADCTL ,0x00};
  const uint8_t nt35510_caset_portrait[] = {0x00, 0x00, 0x01, 0xDF ,NT35510_CMD_CASET};
  const uint8_t nt35510_raset_portrait[] = {0x00, 0x00, 0x03, 0x1F ,NT35510_CMD_RASET};
  const uint8_t nt35510_madctl_landscape[] = {NT35510_CMD_MADCTL, 0x60};
  const uint8_t nt35510_caset_landscape[] = {0x00, 0x00, 0x03, 0x1F ,NT35510_CMD_CASET};
  const uint8_t nt35510_raset_landscape[] = {0x00, 0x00, 0x01, 0xDF ,NT35510_CMD_RASET};
  const uint8_t nt35510_reg26[] = {NT35510_CMD_TEEON, 0x00};  /* Tear on */
  const uint8_t nt35510_reg27[] = {NT35510_CMD_SLPOUT, 0x00}; /* Sleep out */
  const uint8_t nt35510_reg30[] = {NT35510_CMD_DISPON, 0x00};

  const uint8_t nt35510_reg31[] = {NT35510_CMD_WRDISBV, 0x7F};
  const uint8_t nt35510_reg32[] = {NT35510_CMD_WRCTRLD, 0x2C};
  const uint8_t nt35510_reg33[] = {NT35510_CMD_WRCABC, 0x02};
  const uint8_t nt35510_reg34[] = {NT35510_CMD_WRCABCMB, 0xFF};
  const uint8_t nt35510_reg35[] = {NT35510_CMD_RAMWR, 0x00};
  const uint8_t nt35510_reg36[] = {NT35510_CMD_COLMOD, NT35510_COLMOD_RGB565};
  const uint8_t nt35510_reg37[] = {NT35510_CMD_COLMOD, NT35510_COLMOD_RGB888};

  DSI_IO_WriteCmd(5, (uint8_t *)nt35510_reg); /* LV2:  Page 1 enable */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg1);/* AVDD: 5.2V */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg2);/* AVDD: Ratio */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg3);/* AVEE: -5.2V */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg4);/* AVEE: Ratio */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg5);/* VCL: -2.5V */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg6);/* VCL: Ratio */
  DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg7);/* VGH: 15V (Free Pump) */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg8);
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg9);/* VGH: Ratio */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg10);/* VGL_REG: -10V */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg12);/* VGLX: Ratio */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg13);/* VGMP/VGSP: 4.5V/0V */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg14);/* VGMN/VGSN:-4.5V/0V */
  DSI_IO_WriteCmd(2, (uint8_t *)nt35510_reg15);/* VCOM: -1.325V */

/* ************************************************************************** */
/* Proprietary DCS Initialization                                             */
/* ************************************************************************** */
  DSI_IO_WriteCmd(5, (uint8_t *)nt35510_reg16);/* LV2: Page 0 enable */
  DSI_IO_WriteCmd(2, (uint8_t *)nt35510_reg17);/* Display control */
  DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg18);/* Src hold time */
  DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg19);
  DSI_IO_WriteCmd(2, (uint8_t *)nt35510_reg20);/* Gate EQ control */
  DSI_IO_WriteCmd(4, (uint8_t *)nt35510_reg21);/* Src EQ control(Mode2) */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg22);/* Inv. mode(2-dot) */
  DSI_IO_WriteCmd(3, (uint8_t *)nt35510_reg23);
  DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg24);
  /* Tear on */
  DSI_IO_WriteCmd(0, (uint8_t *)nt35510_reg26);
  /* Set Pixel color format to RGB888 */
  DSI_IO_WriteCmd(0, (uint8_t *)nt35510_reg37);

/* ************************************************************************** */
/* Standard DCS Initialization                                                */
/* ************************************************************************** */

  /* Add a delay, otherwise MADCTL not taken */
  NT35510_IO_Delay(200);

  /* Configure orientation */
  if(orientation == NT35510_ORIENTATION_PORTRAIT)
  {
    DSI_IO_WriteCmd(1, (uint8_t *)nt35510_madctl_portrait);
    DSI_IO_WriteCmd(4, (uint8_t *)nt35510_caset_portrait);
    DSI_IO_WriteCmd(4, (uint8_t *)nt35510_raset_portrait);
  }
  else
  {
    DSI_IO_WriteCmd(1, (uint8_t *)nt35510_madctl_landscape);
    DSI_IO_WriteCmd(4, (uint8_t *)nt35510_caset_landscape);
    DSI_IO_WriteCmd(4, (uint8_t *)nt35510_raset_landscape);
  }

  DSI_IO_WriteCmd(0, (uint8_t *)nt35510_reg27);
  /* Wait for sleep out exit */
  NT35510_IO_Delay(120);

  switch(ColorCoding)
  {
    case NT35510_FORMAT_RBG565 :
      /* Set Pixel color format to RGB565 */
      DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg36);
      break;
    case NT35510_FORMAT_RGB888 :
      /* Set Pixel color format to RGB888 */
      DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg37);
      break;
    default :
      /* Set Pixel color format to RGB888 */
      DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg37);
      break;
  }

  /** CABC : Content Adaptive Backlight Control section start >> */
  /* Note : defaut is 0 (lowest Brightness), 0xFF is highest Brightness, try 0x7F : intermediate value */
  DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg31);
  /* defaut is 0, try 0x2C - Brightness Control Block, Display Dimming & BackLight on */
  DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg32);
  /* defaut is 0, try 0x02 - image Content based Adaptive Brightness [Still Picture] */
  DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg33);
  /* defaut is 0 (lowest Brightness), 0xFF is highest Brightness */
  DSI_IO_WriteCmd(1, (uint8_t *)nt35510_reg34);
  /** CABC : Content Adaptive Backlight Control section end << */

  /* Display on */
  DSI_IO_WriteCmd(0, (uint8_t *)nt35510_reg30);

  /* Send Command GRAM memory write (no parameters) : this initiates frame write via other DSI commands sent by */
  /* DSI host from LTDC incoming pixels in video mode */
  DSI_IO_WriteCmd(0, (uint8_t *)nt35510_reg35);

  return 0;
}


/**
  * @}
  */

uint8_t NT35510_DeInit(void)
{
  const uint8_t nt35510_reg30b[] = {NT35510_CMD_DISPOFF, 0x00};
  const uint8_t nt35510_reg27b[] = {NT35510_CMD_SLPIN, 0x00};
  /* Display off */
  DSI_IO_WriteCmd(0, (uint8_t *)nt35510_reg30b);
  NT35510_IO_Delay(120);
  /* Sleep in */
  DSI_IO_WriteCmd(0, (uint8_t *)nt35510_reg27b);
  return 0;
}

/**
  * @brief  Read the component ID.
  * @retval Component ID
  */
uint16_t NT35510_ReadID(void)
{
  uint8_t pData=0;
  DSI_IO_ReadCmd(NT35510_CMD_RDID2, &pData, 1);
  return pData;
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

