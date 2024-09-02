/**
  ******************************************************************************
  * @file    stm32h573i_discovery_ospi.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h573i_discovery_ospi.c driver.
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
#ifndef STM32H573I_DK_OSPI_H
#define STM32H573I_DK_OSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h573i_discovery_conf.h"
#include "stm32h573i_discovery_errno.h"
#include "../Components/mx25lm51245g/mx25lm51245g.h"


/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H573I_DK
  * @{
  */

/** @addtogroup STM32H573I_DK_OSPI
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup STM32H573I_DK_OSPI_Exported_Types OSPI Exported Types
  * @{
  */
typedef enum
{
  OSPI_ACCESS_NONE = 0,          /*!<  Instance not initialized,              */
  OSPI_ACCESS_INDIRECT,          /*!<  Instance use indirect mode access      */
  OSPI_ACCESS_MMP                /*!<  Instance use Memory Mapped Mode read   */
} OSPI_Access_t;

#if (USE_HAL_XSPI_REGISTER_CALLBACKS == 1)
typedef struct
{
  pXSPI_CallbackTypeDef  pMspInitCb;
  pXSPI_CallbackTypeDef  pMspDeInitCb;
} BSP_OSPI_Cb_t;
#endif /* (USE_HAL_XSPI_REGISTER_CALLBACKS == 1) */

typedef struct
{
  uint32_t MemorySize;
  uint32_t ClockPrescaler;
  uint32_t SampleShifting;
  uint32_t TransferRate;
} MX_OSPI_InitTypeDef;
/**
  * @}
  */

/** @defgroup STM32H573I_DK_OSPI_NOR_Exported_Types OSPI_NOR Exported Types
  * @{
  */
#define BSP_OSPI_NOR_Info_t                MX25LM51245G_Info_t
#define BSP_OSPI_NOR_Interface_t           MX25LM51245G_Interface_t
#define BSP_OSPI_NOR_Transfer_t            MX25LM51245G_Transfer_t
#define BSP_OSPI_NOR_Erase_t               MX25LM51245G_Erase_t

typedef struct
{
  OSPI_Access_t              IsInitialized;  /*!<  Instance access Flash method     */
  BSP_OSPI_NOR_Interface_t   InterfaceMode;  /*!<  Flash Interface mode of Instance */
  BSP_OSPI_NOR_Transfer_t    TransferRate;   /*!<  Flash Transfer mode of Instance  */
} OSPI_NOR_Ctx_t;

typedef struct
{
  BSP_OSPI_NOR_Interface_t   InterfaceMode;      /*!<  Current Flash Interface mode */
  BSP_OSPI_NOR_Transfer_t    TransferRate;       /*!<  Current Flash Transfer rate  */
} BSP_OSPI_NOR_Init_t;
/**
  * @}
  */


/* Exported constants --------------------------------------------------------*/
/** @defgroup STM32H573I_DK_OSPI_Exported_Constants OSPI Exported Constants
  * @{
  */

/* Definition for OSPI clock resources */
#define OSPI_NOR_CLK_ENABLE()                 __HAL_RCC_OSPI1_CLK_ENABLE()
#define OSPI_NOR_CLK_DISABLE()                __HAL_RCC_OSPI1_CLK_DISABLE()
#define OSPI_NOR_FORCE_RESET()                __HAL_RCC_OSPI1_FORCE_RESET()
#define OSPI_NOR_RELEASE_RESET()              __HAL_RCC_OSPI1_RELEASE_RESET()

/* Definition for OSPI Pins */
/* OSPI_CS */
#define OSPI_NOR_CS_PIN                       GPIO_PIN_6
#define OSPI_NOR_CS_GPIO_PORT                 GPIOG
#define OSPI_NOR_CS_PIN_AF                    GPIO_AF10_OCTOSPI1
#define OSPI_NOR_CS_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOG_CLK_ENABLE()

/* OSPI_CLK */
#define OSPI_NOR_CLK_PIN                      GPIO_PIN_10
#define OSPI_NOR_CLK_GPIO_PORT                GPIOF
#define OSPI_NOR_CLK_PIN_AF                   GPIO_AF9_OCTOSPI1
#define OSPI_NOR_CLK_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOF_CLK_ENABLE()

/* OSPI_DQS */
#define OSPI_NOR_DQS_PIN                      GPIO_PIN_2
#define OSPI_NOR_DQS_GPIO_PORT                GPIOB
#define OSPI_NOR_DQS_PIN_AF                   GPIO_AF10_OCTOSPI1
#define OSPI_NOR_DQS_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()

/* OSPI_D0 */
#define OSPI_NOR_D0_PIN                       GPIO_PIN_1
#define OSPI_NOR_D0_GPIO_PORT                 GPIOB
#define OSPI_NOR_D0_PIN_AF                    GPIO_AF6_OCTOSPI1
#define OSPI_NOR_D0_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()

/* OSPI_D1 */
#define OSPI_NOR_D1_PIN                       GPIO_PIN_12
#define OSPI_NOR_D1_GPIO_PORT                 GPIOD
#define OSPI_NOR_D1_PIN_AF                    GPIO_AF9_OCTOSPI1
#define OSPI_NOR_D1_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOD_CLK_ENABLE()

/* OSPI_D2 */
#define OSPI_NOR_D2_PIN                       GPIO_PIN_2
#define OSPI_NOR_D2_GPIO_PORT                 GPIOC
#define OSPI_NOR_D2_PIN_AF                    GPIO_AF9_OCTOSPI1
#define OSPI_NOR_D2_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOC_CLK_ENABLE()

/* OSPI_D3 */
#define OSPI_NOR_D3_PIN                       GPIO_PIN_13
#define OSPI_NOR_D3_GPIO_PORT                 GPIOD
#define OSPI_NOR_D3_PIN_AF                    GPIO_AF9_OCTOSPI1
#define OSPI_NOR_D3_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOD_CLK_ENABLE()

/* OSPI_D4 */
#define OSPI_NOR_D4_PIN                       GPIO_PIN_2
#define OSPI_NOR_D4_GPIO_PORT                 GPIOH
#define OSPI_NOR_D4_PIN_AF                    GPIO_AF9_OCTOSPI1
#define OSPI_NOR_D4_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOH_CLK_ENABLE()

/* OSPI_D5 */
#define OSPI_NOR_D5_PIN                       GPIO_PIN_3
#define OSPI_NOR_D5_GPIO_PORT                 GPIOH
#define OSPI_NOR_D5_PIN_AF                    GPIO_AF9_OCTOSPI1
#define OSPI_NOR_D5_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOH_CLK_ENABLE()

/* OSPI_D6 */
#define OSPI_NOR_D6_PIN                       GPIO_PIN_9
#define OSPI_NOR_D6_GPIO_PORT                 GPIOG
#define OSPI_NOR_D6_PIN_AF                    GPIO_AF9_OCTOSPI1
#define OSPI_NOR_D6_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOG_CLK_ENABLE()

/* OSPI_D7 */
#define OSPI_NOR_D7_PIN                       GPIO_PIN_0
#define OSPI_NOR_D7_GPIO_PORT                 GPIOC
#define OSPI_NOR_D7_PIN_AF                    GPIO_AF10_OCTOSPI1
#define OSPI_NOR_D7_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOC_CLK_ENABLE()

/**
  * @}
  */

/** @defgroup STM32H573I_DK_OSPI_NOR_Exported_Constants OSPI_NOR Exported Constants
  * @{
  */
#define OSPI_NOR_INSTANCES_NUMBER         1U

/* Definition for OSPI modes */
#define BSP_OSPI_NOR_SPI_MODE (BSP_OSPI_NOR_Interface_t)MX25LM51245G_SPI_MODE /* 1 Cmd, 1 Address and 1 Data Lines */
#define BSP_OSPI_NOR_OPI_MODE (BSP_OSPI_NOR_Interface_t)MX25LM51245G_OPI_MODE /* 8 Cmd, 8 Address and 8 Data Lines */

/* Definition for OSPI transfer rates */
#define BSP_OSPI_NOR_STR_TRANSFER  (BSP_OSPI_NOR_Transfer_t)MX25LM51245G_STR_TRANSFER   /* Single Transfer Rate */
#define BSP_OSPI_NOR_DTR_TRANSFER  (BSP_OSPI_NOR_Transfer_t)MX25LM51245G_DTR_TRANSFER   /* Double Transfer Rate */

/* OSPI erase types */
#define BSP_OSPI_NOR_ERASE_4K             MX25LM51245G_ERASE_4K
#define BSP_OSPI_NOR_ERASE_64K            MX25LM51245G_ERASE_64K
#define BSP_OSPI_NOR_ERASE_CHIP           MX25LM51245G_ERASE_BULK

/* OSPI block sizes */
#define BSP_OSPI_NOR_BLOCK_4K             MX25LM51245G_SUBSECTOR_4K
#define BSP_OSPI_NOR_BLOCK_64K            MX25LM51245G_SECTOR_64K
/**
  * @}
  */


/* Exported variables --------------------------------------------------------*/
/** @defgroup STM32H573I_DK_OSPI_NOR_Exported_Variables OSPI_NOR Exported Variables
  * @{
  */
extern XSPI_HandleTypeDef hospi_nor[OSPI_NOR_INSTANCES_NUMBER];
extern OSPI_NOR_Ctx_t Ospi_Nor_Ctx[OSPI_NOR_INSTANCES_NUMBER];
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup STM32H573I_DK_OSPI_NOR_Exported_Functions OSPI_NOR Exported Functions
  * @{
  */
int32_t BSP_OSPI_NOR_Init(uint32_t Instance, BSP_OSPI_NOR_Init_t *Init);
int32_t BSP_OSPI_NOR_DeInit(uint32_t Instance);
#if (USE_HAL_XSPI_REGISTER_CALLBACKS == 1)
int32_t BSP_OSPI_NOR_RegisterMspCallbacks(uint32_t Instance, BSP_OSPI_Cb_t *CallBacks);
int32_t BSP_OSPI_NOR_RegisterDefaultMspCallbacks(uint32_t Instance);
#endif /* (USE_HAL_XSPI_REGISTER_CALLBACKS == 1) */
int32_t BSP_OSPI_NOR_Read(uint32_t Instance, uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
int32_t BSP_OSPI_NOR_Write(uint32_t Instance, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size);
int32_t BSP_OSPI_NOR_Erase_Block(uint32_t Instance, uint32_t BlockAddress, BSP_OSPI_NOR_Erase_t BlockSize);
int32_t BSP_OSPI_NOR_Erase_Chip(uint32_t Instance);
int32_t BSP_OSPI_NOR_GetStatus(uint32_t Instance);
int32_t BSP_OSPI_NOR_GetInfo(uint32_t Instance, BSP_OSPI_NOR_Info_t *pInfo);
int32_t BSP_OSPI_NOR_EnableMemoryMappedMode(uint32_t Instance);
int32_t BSP_OSPI_NOR_DisableMemoryMappedMode(uint32_t Instance);
int32_t BSP_OSPI_NOR_ReadID(uint32_t Instance, uint8_t *Id);
int32_t BSP_OSPI_NOR_ConfigFlash(uint32_t Instance, BSP_OSPI_NOR_Interface_t Mode, BSP_OSPI_NOR_Transfer_t Rate);
int32_t BSP_OSPI_NOR_SuspendErase(uint32_t Instance);
int32_t BSP_OSPI_NOR_ResumeErase(uint32_t Instance);
int32_t BSP_OSPI_NOR_EnterDeepPowerDown(uint32_t Instance);
int32_t BSP_OSPI_NOR_LeaveDeepPowerDown(uint32_t Instance);

/**
  * @}
  */


/* These functions can be modified in case the current settings
   need to be changed for specific application needs */
HAL_StatusTypeDef MX_OSPI_NOR_Init(XSPI_HandleTypeDef *hospi, MX_OSPI_InitTypeDef *Init);
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

#endif /* STM32H573I_DK_OSPI_H */
