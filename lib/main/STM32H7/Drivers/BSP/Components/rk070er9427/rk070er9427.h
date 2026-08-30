/**
  ******************************************************************************
  * @file    rk070er9427.h
  * @author  MCD Application Team
  * @brief   This file contains all the constants parameters for the RK070ER9427-CT672B
  *          LCD component.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
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
#ifndef RK070ER9427_H
#define RK070ER9427_H

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
  
/** @addtogroup rk070er9427
  * @{
  */

/** @defgroup RK070ER9427_Exported_Types
  * @{
  */
   
/**
  * @}
  */ 

/** @defgroup RK070ER9427_Exported_Constants
  * @{
  */

/** 
  * @brief  RK070ER9427 Size  
  */     
#define  RK070ER9427_WIDTH          800U          /* LCD PIXEL WIDTH            */
#define  RK070ER9427_HEIGHT         480U          /* LCD PIXEL HEIGHT           */

/** 
  * @brief  RK070ER9427 Timing  
  */     
#define  RK070ER9427_HSYNC          46U    /* Horizontal synchronization */
#define  RK070ER9427_HBP            2U     /* Horizontal back porch      */
#define  RK070ER9427_HFP            210U   /* Horizontal front porch     */
#define  RK070ER9427_VSYNC          23U    /* Vertical synchronization   */
#define  RK070ER9427_VBP            2U     /* Vertical back porch        */
#define  RK070ER9427_VFP            22U    /* Vertical front porch       */
  
/** @defgroup RK070ER9427_Exported_Functions
  * @{
  */

/**
  * @}
  */    
#ifdef __cplusplus
}
#endif

#endif /* __RK070ER9427_H */
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
