
/**
  ******************************************************************************
  * @file    nt35510_reg.h
  * @author  MCD Application Team
  * @brief   This file contains all the constants parameters for the NT35510
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef NT35510_REG_H
#define NT35510_REG_H

/* Includes ------------------------------------------------------------------*/
#include <cmsis_compiler.h>

#ifdef __cplusplus
 extern "C" {
#endif 

/** @addtogroup BSP
  * @{
  */
 
/** @addtogroup Components
  * @{
  */

/** @addtogroup NT35510
  * @{
  */


/** @defgroup NT35510_Exported_Constants
  * @{
  */
    
/* List of NT35510 used commands                                  */
/* Detailed in NT35510 Data Sheet v0.80                           */
/* Version of 10/28/2011                                          */
/* Command, NumberOfArguments                                     */

#define  NT35510_CMD_NOP                   0x00  /* NOP */
#define  NT35510_CMD_SWRESET               0x01  /* SW reset */
#define  NT35510_CMD_RDDID                 0x04  /* Read display ID */
#define  NT35510_CMD_RDNUMED               0x05  /* Read number of errors on DSI */
#define  NT35510_CMD_RDDPM                 0x0A  /* Read display power mode */
#define  NT35510_CMD_RDDMADCTL             0x0B  /* Read display MADCTL */
#define  NT35510_CMD_RDDCOLMOD             0x0C  /* Read display pixel format */
#define  NT35510_CMD_RDDIM                 0x0D  /* Read display image mode */
#define  NT35510_CMD_RDDSM                 0x0E  /* Read display signal mode */
#define  NT35510_CMD_RDDSDR                0x0F  /* Read display self-diagnostics result */
#define  NT35510_CMD_SLPIN                 0x10  /* Sleep in */
#define  NT35510_CMD_SLPOUT                0x11  /* Sleep out */
#define  NT35510_CMD_PTLON                 0x12  /* Partial mode on  */
#define  NT35510_CMD_NORON                 0x13  /* Normal display mode on */
#define  NT35510_CMD_INVOFF                0x20  /* Display inversion off */
#define  NT35510_CMD_INVON                 0x21  /* Display inversion on */
#define  NT35510_CMD_ALLPOFF               0x22  /* All pixel off */
#define  NT35510_CMD_ALLPON                0x23  /* All pixel on */
#define  NT35510_CMD_GAMSET                0x26  /* Gamma set */
#define  NT35510_CMD_DISPOFF               0x28  /* Display off */
#define  NT35510_CMD_DISPON                0x29  /* Display on */
#define  NT35510_CMD_CASET                 0x2A  /* Column address set */
#define  NT35510_CMD_RASET                 0x2B  /* Row address set */
#define  NT35510_CMD_RAMWR                 0x2C  /* Memory write */
#define  NT35510_CMD_RAMRD                 0x2E  /* Memory read  */
#define  NT35510_CMD_PLTAR                 0x30  /* Partial area */
#define  NT35510_CMD_TOPC                  0x32  /* Turn On Peripheral Command */
#define  NT35510_CMD_TEOFF                 0x34  /* Tearing effect line off */
#define  NT35510_CMD_TEEON                 0x35  /* Tearing effect line on */
#define  NT35510_CMD_MADCTL                0x36  /* Memory data access control */
#define  NT35510_CMD_IDMOFF                0x38  /* Idle mode off */
#define  NT35510_CMD_IDMON                 0x39  /* Idle mode on */
#define  NT35510_CMD_COLMOD                0x3A  /* Interface pixel format */
#define  NT35510_CMD_RAMWRC                0x3C  /* Memory write continue */
#define  NT35510_CMD_RAMRDC                0x3E  /* Memory read continue */
#define  NT35510_CMD_STESL                 0x44  /* Set tearing effect scan line */
#define  NT35510_CMD_GSL                   0x45  /* Get scan line */

#define  NT35510_CMD_DSTBON                0x4F  /* Deep standby mode on */
#define  NT35510_CMD_WRPFD                 0x50  /* Write profile value for display */
#define  NT35510_CMD_WRDISBV               0x51  /* Write display brightness */
#define  NT35510_CMD_RDDISBV               0x52  /* Read display brightness */
#define  NT35510_CMD_WRCTRLD               0x53  /* Write CTRL display */
#define  NT35510_CMD_RDCTRLD               0x54  /* Read CTRL display value */
#define  NT35510_CMD_WRCABC                0x55  /* Write content adaptative brightness control */
#define  NT35510_CMD_RDCABC                0x56  /* Read content adaptive brightness control */
#define  NT35510_CMD_WRHYSTE               0x57  /* Write hysteresis */
#define  NT35510_CMD_WRGAMMSET             0x58  /* Write gamme setting */
#define  NT35510_CMD_RDFSVM                0x5A  /* Read FS value MSBs */
#define  NT35510_CMD_RDFSVL                0x5B  /* Read FS value LSBs */
#define  NT35510_CMD_RDMFFSVM              0x5C  /* Read median filter FS value MSBs */
#define  NT35510_CMD_RDMFFSVL              0x5D  /* Read median filter FS value LSBs */
#define  NT35510_CMD_WRCABCMB              0x5E  /* Write CABC minimum brightness */
#define  NT35510_CMD_RDCABCMB              0x5F  /* Read CABC minimum brightness */
#define  NT35510_CMD_WRLSCC                0x65  /* Write light sensor compensation coefficient value */
#define  NT35510_CMD_RDLSCCM               0x66  /* Read light sensor compensation coefficient value MSBs */
#define  NT35510_CMD_RDLSCCL               0x67  /* Read light sensor compensation coefficient value LSBs */
#define  NT35510_CMD_RDBWLB                0x70  /* Read black/white low bits */
#define  NT35510_CMD_RDBKX                 0x71  /* Read Bkx */
#define  NT35510_CMD_RDBKY                 0x72  /* Read Bky */
#define  NT35510_CMD_RDWX                  0x73  /* Read Wx */
#define  NT35510_CMD_RDWY                  0x74  /* Read Wy */
#define  NT35510_CMD_RDRGLB                0x75  /* Read red/green low bits */
#define  NT35510_CMD_RDRX                  0x76  /* Read Rx */
#define  NT35510_CMD_RDRY                  0x77  /* Read Ry */
#define  NT35510_CMD_RDGX                  0x78  /* Read Gx */
#define  NT35510_CMD_RDGY                  0x79  /* Read Gy */
#define  NT35510_CMD_RDBALB                0x7A  /* Read blue/acolor low bits */
#define  NT35510_CMD_RDBX                  0x7B  /* Read Bx */
#define  NT35510_CMD_RDBY                  0x7C  /* Read By */
#define  NT35510_CMD_RDAX                  0x7D  /* Read Ax */
#define  NT35510_CMD_RDAY                  0x7E  /* Read Ay */
#define  NT35510_CMD_RDDDBS                0xA1  /* Read DDB start */
#define  NT35510_CMD_RDDDBC                0xA8  /* Read DDB continue */
#define  NT35510_CMD_RDDCS                 0xAA  /* Read first checksum */
#define  NT35510_CMD_RDCCS                 0xAF  /* Read continue checksum */
#define  NT35510_CMD_RDID1                 0xDA  /* Read ID1 value */
#define  NT35510_CMD_RDID2                 0xDB  /* Read ID2 value */
#define  NT35510_CMD_RDID3                 0xDC  /* Read ID3 value */

/* Parameter TELOM : Tearing Effect Line Output Mode : possible values */
#define NT35510_TEEON_TELOM_VBLANKING_INFO_ONLY            0x00
#define NT35510_TEEON_TELOM_VBLANKING_AND_HBLANKING_INFO   0x01

/* Possible used values of MADCTR */
#define NT35510_MADCTR_MODE_PORTRAIT       0x00
#define NT35510_MADCTR_MODE_LANDSCAPE      0x60  /* MY = 0, MX = 1, MV = 1, ML = 0, RGB = 0 */

/* Possible values of COLMOD parameter corresponding to used pixel formats */
#define  NT35510_COLMOD_RGB565             0x55
#define  NT35510_COLMOD_RGB888             0x77

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
   
/** @defgroup NT35510_Exported_Macros NT35510 Exported Macros
  * @{
  */ 

/**
  * @}
  */ 

/* Exported functions --------------------------------------------------------*/
  
/** @addtogroup NT35510_Exported_Functions
  * @{
  */
/************** Generic Function  *******************/

typedef int32_t (*NT35510_Write_Func)(void *, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*NT35510_Read_Func) (void *, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  NT35510_Write_Func   WriteReg;
  NT35510_Read_Func    ReadReg;
  void                  *handle;
} nt35510_ctx_t;

/*******************************************************************************
* Register      : Generic - All
* Address       : Generic - All
* Bit Group Name: None
* Permission    : W
*******************************************************************************/
int32_t nt35510_read_reg(nt35510_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint16_t length);
int32_t nt35510_write_reg(nt35510_ctx_t *ctx, uint16_t reg, const uint8_t *pdata, uint16_t length);
/**
  * @}
  */ 
#ifdef __cplusplus
}
#endif

#endif /* NT35510_REG_H */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
