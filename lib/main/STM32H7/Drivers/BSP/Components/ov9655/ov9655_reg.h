/**
  ******************************************************************************
  * @file    ov9655_reg.h
  * @author  MCD Application Team
  * @brief   Header of ov9655_reg.c
  *          
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OV9655_REG_H
#define OV9655_REG_H

#include <cmsis_compiler.h>

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/   
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup OV9655
  * @{
  */

/** @defgroup OV9655_Exported_Types
  * @{
  */
     
/**
  * @}
  */ 

/** @defgroup OV9655_Exported_Constants OV9655 Exported Constants
  * @{
  */

/** 
  * @brief  OV9655 ID  
  */  
#define  OV9655_ID                             0x9656U
#define  OV9655_ID_2                           0x9657U
/** 
  * @brief  OV9655 Registers  
  */
#define OV9655_GAIN_CTRL                       0x00U
#define OV9655_AWB_BLUE_GAIN_CTRL              0x01U
#define OV9655_AWB_RED_GAIN_CTRL               0x02U
#define OV9655_VERTIVCAL_FRAME_CTRL            0x03U
#define OV9655_COMMON_CTRL1                    0x04U
#define OV9655_UB_AVERAGE_LEVEL                0x05U
#define OV9655_YGB_AVERAGE_LEVEL               0x06U
#define OV9655_GR_AVERAGE_LEVEL                0x07U
#define OV9655_VR_AVERAGE_LEVEL                0x08U
#define OV9655_COMMON_CTRL2                    0x09U   
#define OV9655_PID_NUMBER_HIGH                 0x0AU
#define OV9655_PID_NUMBER_LOW                  0x0BU
#define OV9655_COMMON_CTRL3                    0x0CU
#define OV9655_COMMON_CTRL4                    0x0DU
#define OV9655_COMMON_CTRL5                    0x0EU
#define OV9655_COMMON_CTRL6                    0x0FU
#define OV9655_EXPOSURE_VALUE                  0x10U
#define OV9655_INTERNAL_CLOCK                  0x11U
#define OV9655_COMMON_CTRL7                    0x12U
#define OV9655_COMMON_CTRL8                    0x13U
#define OV9655_COMMON_CTRL9                    0x14U
#define OV9655_COMMON_CTRL10                   0x15U
#define OV9655_REG16_CTRL                      0x16U
#define OV9655_HORIZONTAL_FRAME_START          0x17U
#define OV9655_HORIZONTAL_FRAME_STOP           0x18U
#define OV9655_VERTICAL_FRAME_START            0x19U
#define OV9655_VERTICAL_FRAME_STOP             0x1AU
#define OV9655_PSHFT                           0x1BU
#define OV9655_MANUFACTURER_ID_HIGH            0x1CU
#define OV9655_MANUFACTURER_ID_LOW             0x1DU
#define OV9655_MIRROR_VFLIP                    0x1EU    
#define OV9655_LAEC                            0x1FU /* Reserved */
#define OV9655_BOS                             0x20U /* B Channel ADBLC Result: Offset adjustment sign  */
#define OV9655_GBOS                            0x21U /* GB Channel ADBLC Result: Offset adjustment sign */
#define OV9655_GROS                            0x22U /* GR Channel ADBLC Result: Offset adjustment sign */
#define OV9655_ROS                             0x23U /* R Channel ADBLC Result: Offset adjustment sign  */
#define OV9655_AEW                             0x24U /* AGC/AEC - Stable Operating Region (Upper Limit) */
#define OV9655_AEB                             0x25U /* AGC/AEC - Stable Operating Region (Lower Limit) */
#define OV9655_VPT                             0x26U /* AGC/AEC Fast Mode Operating Region              */
#define OV9655_BBIAS                           0x27U /* B Channel Signal Output Bias                    */
#define OV9655_GBBIAS                          0x28U /* GB Channel Signal Output Bias                   */
#define OV9655_PREGAIN                         0x29U /* RGB Channel Pre-gain                            */
#define OV9655_EXHCH                           0x2AU /* Dummy Pixel Insert MSB                          */                                                  
#define OV9655_EXHCL                           0x2BU /* Dummy Pixel Insert LSB                          */
#define OV9655_RBIAS                           0x2CU /* R Channel Signal Output Bias                    */
#define OV9655_ADVFL                           0x2DU /* LSB of insert dummy lines in vertical direction */
#define OV9655_ADVFH                           0x2EU /* MSB of insert dummy lines in vertical direction */
#define OV9655_YG_AVERAGE_VALUE                0x2FU /* Y/G Channel Average Value                       */
#define OV9655_HSYNC_RISING_EDGE_DELAY         0x30U 
#define OV9655_HSYNC_FALLING_EDGE_DELAY        0x31U
#define OV9655_HORIZONTAL_FRAME_CTRL           0x32U
#define OV9655_CHLF_CTRL                       0x33U /* ADC Reference Control Array Current Control     */
#define OV9655_AREF1                           0x34U /* Array Reference Control                         */
#define OV9655_AREF2                           0x35U /* Array Reference Control                         */
#define OV9655_AREF3                           0x36U /* Array Reference Control                         */                                                
#define OV9655_ADC1                            0x37U /* ADC Control 1                                   */ 
#define OV9655_ADC2                            0x38U /* ADC Reference Control                           */
#define OV9655_AREF4                           0x39U /* Analog Reference Control                        */
#define OV9655_TSLB                            0x3AU /* Line Buffer Test Option */
#define OV9655_COMMON_CTRL11                   0x3BU
#define OV9655_COMMON_CTRL12                   0x3CU
#define OV9655_COMMON_CTRL13                   0x3DU                                                 
#define OV9655_COMMON_CTRL14                   0x3EU
#define OV9655_EDGE                            0x3FU
#define OV9655_COMMON_CTRL15                   0x40U
#define OV9655_COMMON_CTRL16                   0x41U
#define OV9655_COMMON_CTRL17                   0x42U
#define OV9655_MATRIX_COEFFICIENT_1            0x4FU
#define OV9655_MATRIX_COEFFICIENT_2            0x50U
#define OV9655_MATRIX_COEFFICIENT_3            0x51U                                                 
#define OV9655_MATRIX_COEFFICIENT_4            0x52U
#define OV9655_MATRIX_COEFFICIENT_5            0x53U
#define OV9655_MATRIX_COEFFICIENT_6            0x54U
#define OV9655_BRIGHTNESS_ADJUSTMENT           0x55U
#define OV9655_CONTRAST_COEFFICIENT_1          0x56U
#define OV9655_CONTRAST_COEFFICIENT_2          0x57U
#define OV9655_MATRIX_COEFFICIENT_SIGN         0x58U
#define OV9655_AWB_CTRL_OPTION_1               0x59U
#define OV9655_AWB_CTRL_OPTION_2               0x5AU
#define OV9655_AWB_CTRL_OPTION_3               0x5BU
#define OV9655_AWB_CTRL_OPTION_4               0x5CU
#define OV9655_AWB_CTRL_OPTION_5               0x5DU
#define OV9655_AWB_CTRL_OPTION_6               0x5EU
#define OV9655_AWB_BLUE_GAIN_LIMIT             0x5FU
#define OV9655_AWB_RED_GAIN_LIMIT              0x60U
#define OV9655_AWB_GREEN_GAIN_LIMIT            0x61U
#define OV9655_LENS_CORRECTION_OPTION_1        0x62U
#define OV9655_LENS_CORRECTION_OPTION_2        0x63U
#define OV9655_LENS_CORRECTION_OPTION_3        0x64U
#define OV9655_LENS_CORRECTION_OPTION_4        0x65U
#define OV9655_LENS_CORRECTION_CTRL            0x66U
#define OV9655_MANU                            0x67U /* Manual U Value                            */
#define OV9655_MANV                            0x68U /* Manual V Value                            */
#define OV9655_BD50MAX                         0x6AU /* 50 Hz Banding Filter Maximum Step Setting */
#define OV9655_DBLV                            0x6BU /* Band Gap Reference Adjustment             */
#define OV9655_DNSTH                           0x70U /* De-noise Function Threshold Adjustment    */
#define OV9655_POIDX                           0x72U /* Pixel Output Index                        */
#define OV9655_PCKDV                           0x73U /* Pixel Clock Output Selection              */
#define OV9655_XINDX                           0x74U /* Horizontal Scaling Down Coefficients      */
#define OV9655_YINDX                           0x75U /* Vertical Scaling Down Coefficients        */
#define OV9655_SLOP                            0x7AU /* Gamma Curve Highest Segment Slope         */
#define OV9655_GAMMA_1                         0x7BU /* Gamma Curve 1st Segment Input End Point 0x010 Output Value */
#define OV9655_GAMMA_2                         0x7CU /* Gamma Curve 2nd Segment Input End Point 0x020 Output Value */
#define OV9655_GAMMA_3                         0x7DU /* Gamma Curve 3rd Segment Input End Point 0x040 Output Value */
#define OV9655_GAMMA_4                         0x7EU /* Gamma Curve 4th Segment Input End Point 0x080 Output Value */
#define OV9655_GAMMA_5                         0x7FU /* Gamma Curve 5th Segment Input End Point 0x0A0 Output Value */
#define OV9655_GAMMA_6                         0x80U /* Gamma Curve 6th Segment Input End Point 0x0C0 Output Value */     
#define OV9655_GAMMA_7                         0x81U /* Gamma Curve 7th Segment Input End Point 0x0E0 Output Value */   
#define OV9655_GAMMA_8                         0x82U /* Gamma Curve 8th Segment Input End Point 0x100 Output Value */
#define OV9655_GAMMA_9                         0x83U /* Gamma Curve 9th Segment Input End Point 0x120 Output Value */
#define OV9655_GAMMA_10                        0x84U /* Gamma Curve 10th Segment Input End Point 0x140 Output Value */
#define OV9655_GAMMA_11                        0x85U /* Gamma Curve 11th Segment Input End Point 0x180 Output Value */
#define OV9655_GAMMA_12                        0x86U /* Gamma Curve 12th Segment Input End Point 0x1C0 Output Value */
#define OV9655_GAMMA_13                        0x87U /* Gamma Curve 13th Segment Input End Point 0x240 Output Value */
#define OV9655_GAMMA_14                        0x88U /* Gamma Curve 14th Segment Input End Point 0x2C0 Output Value */
#define OV9655_GAMMA_15                        0x89U /* Gamma Curve 15th Segment Input End Point 0x340 Output Value */
#define OV9655_COMMON_CTRL18                   0x8BU              
#define OV9655_COMMON_CTRL19                   0x8CU              
#define OV9655_COMMON_CTRL20                   0x8DU              
#define OV9655_DMLNL                           0x92U /* Frame Dummy Line LSBs */
#define OV9655_DMLNH                           0x93U /* Frame Dummy Line MSBs */
#define OV9655_LENS_CORRECTION_OPTION_6        0x9DU
#define OV9655_LENS_CORRECTION_OPTION_7        0x9EU
#define OV9655_AECH                            0xA1U /* Exposure Value - AEC MSB 5 bits   */                  
#define OV9655_BD50                            0xA2U /* 1/100s Exposure Setting for 50 Hz Banding Filter */
#define OV9655_BD60                            0xA3U /* 1/120s Exposure Setting for 60 Hz Banding Filter */
#define OV9655_COMMON_CTRL21                   0xA4U
#define OV9655_GREEN                           0xA6U /* AWB Green Component Gain Setting */
#define OV9655_VZST                            0xA7U /* VGA Zoom Mode Vertical Start Line */
#define OV9655_REFA8                           0xA8U /* Analog Reference Control */
#define OV9655_REFA9                           0xA9U /* Analog Reference Control */              
#define OV9655_BLC_1                           0xACU /* Black Level Control 1 */              
#define OV9655_BLC_2                           0xADU /* Black Level Control 2 */              
#define OV9655_BLC_3                           0xAEU /* Black Level Control 3 */
#define OV9655_BLC_4                           0xAFU /* Black Level Control 4 */
#define OV9655_BLC_5                           0xB0U /* Black Level Control 5 */
#define OV9655_BLC_6                           0xB1U /* Black Level Control 6 */
#define OV9655_BLC_7                           0xB2U /* Black Level Control 7 */
#define OV9655_BLC_8                           0xB3U /* Black Level Control 8 */
#define OV9655_CTRLB4                          0xB4U
#define OV9655_FRSTL                           0xB7U /* One-Pin Frame Exposure Reset Time Control Low 8 Bits */
#define OV9655_FRSTH                           0xB8U /* One-Pin Frame Exposure Reset Time Control High 8 Bits */
#define OV9655_ADBOFF                          0xBCU /* ADC B channel offset setting */
#define OV9655_ADROFF                          0xBDU /* ADC R channel offset setting */
#define OV9655_ADGbOFF                         0xBEU /* ADC Gb channel offset setting */                                                  
#define OV9655_ADGrOFF                         0xBFU /* ADC Gr channel offset setting */
#define OV9655_OV9655_COMMON_CTRL23            0xC4U
#define OV9655_BD60MAX                         0xC5U /* 60 Hz Banding Filter Maximum Step Setting */
#define OV9655_OV9655_COMMON_CTRL24            0x07U
/**
  * @}
  */

/************** Generic Function  *******************/

typedef int32_t (*OV9655_Write_Func)(void *, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*OV9655_Read_Func) (void *, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  OV9655_Write_Func   WriteReg;
  OV9655_Read_Func    ReadReg;
  void                *handle;
} ov9655_ctx_t;

/*******************************************************************************
* Register      : Generic - All
* Address       : Generic - All
* Bit Group Name: None
* Permission    : W
*******************************************************************************/
int32_t ov9655_write_reg(ov9655_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint16_t length);
int32_t ov9655_read_reg(ov9655_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint16_t length);
/**
  * @}
  */    
#ifdef __cplusplus
}
#endif

#endif /* OV9655_REG_H */
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
