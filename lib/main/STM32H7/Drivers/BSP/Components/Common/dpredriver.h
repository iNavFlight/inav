/**
  ******************************************************************************
  * @file    dpredriver.h
  * @author  MCD Application Team
  * @brief   This header file contains the functions prototypes for the  
  *          DisplayPort Linear Redriver.
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
#ifndef __DPREDRIVER_H
#define __DPREDRIVER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */
    
/** @addtogroup DPREDRIVER
  * @{
  */

/** @defgroup DPREDRIVER_Exported_Types
  * @{
  */ 

/** @defgroup DPREDRIVER_Channel_Identifier  Channel Identifier
  * @{
  */
 typedef enum {
   CHANNEL_DP0 = 0,
   CHANNEL_DP1,
   CHANNEL_DP2,
   CHANNEL_DP3,
   CHANNEL_RX1,
   CHANNEL_RX2,
   CHANNEL_SSTX
 } DPREDRIVER_ChannelId_t;
/**
  * @}
  */
  
 /** @defgroup DPREDRIVER_Driver_structure  DisplayPort Linear Redriver Driver structure
  * @{
  */
typedef struct
{
  uint32_t  (*Init)(uint16_t);
  void      (*DeInit)(uint16_t); 
  uint32_t  (*PowerOn)(uint16_t);
  uint32_t  (*PowerOff)(uint16_t);
  uint32_t  (*SetEQGain)(uint16_t, DPREDRIVER_ChannelId_t, uint8_t);
  uint32_t  (*EnableChannel)(uint16_t, DPREDRIVER_ChannelId_t);
  uint32_t  (*DisableChannel)(uint16_t, DPREDRIVER_ChannelId_t);
}DPREDRIVER_Drv_t;
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

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __DPREDRIVER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
