/**
  ******************************************************************************
  * @file    stm32h5xx_nucleo_usbpd_pwr.h
  * @author  MCD Application Team
  * @brief   Header file for stm32h5xx_nucleo_usbpd_pwr.c file
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
#ifndef STM32H5XX_NUCLEO_USBPD_PWR_H
#define STM32H5XX_NUCLEO_USBPD_PWR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_nucleo_conf.h"
#include "stm32h5xx_nucleo_errno.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H5XX_NUCLEO
  * @{
  */

/** @addtogroup STM32H5XX_NUCLEO_USBPD_PWR
  * @{
  */

/** @defgroup STM32H5XX_NUCLEO_USBPD_PWR_Exported_Types Exported Types
  * @{
  */

/**
  * @brief  VBUS connection status
  */
typedef enum
{
  VBUS_CONNECTED = 0,
  VBUS_NOT_CONNECTED
} USBPD_PWR_VBUSConnectionStatusTypeDef;

/**
  * @brief VBUS Detection Callback
  */
typedef void USBPD_PWR_VBUSDetectCallbackFunc(uint32_t PortNum,
                                              USBPD_PWR_VBUSConnectionStatusTypeDef VBUSConnectionStatus);

/**
  * @}
  */

/** @defgroup STM32H5XX_NUCLEO_USBPD_PWR_Exported_Constants Exported Constants
  * @{
  */
/**
  * @brief  Number of TypeC ports
  */
#define USBPD_PWR_INSTANCES_NBR           (1U)

/**
  * @brief  Type-C port identifier
  */
#define USBPD_PWR_TYPE_C_PORT_1           (0U)

/**
  * @brief  CC pin identifier
  */
#define USBPD_PWR_TYPE_C_CC1              (1U)
#define USBPD_PWR_TYPE_C_CC2              (2U)

/**
  * @brief  VBUS disconnection threshold values (in mV)
  */
#define USBPD_PWR_HIGH_VBUS_THRESHOLD     (2800U)
#define USBPD_PWR_LOW_VBUS_THRESHOLD      (750U)
#define USBPD_PWR_VBUS_THRESHOLD_5V       (2150U)
#define USBPD_PWR_VBUS_THRESHOLD_9V       (7000U)
#define USBPD_PWR_VBUS_THRESHOLD_15V      (12500U)
#define USBPD_PWR_VBUS_THRESHOLD_20V      (17000U)
#define USBPD_PWR_VBUS_THRESHOLD_APDO     (2150U)

/**
  * @brief  VBUS discharge parameters
  */
#define USBPD_PWR_DISCHARGE_MARGIN        (500U)
#define USBPD_PWR_DISCHARGE_TIME          (6U)

/**
  * @brief  Standard VBUS voltage levels
  */
#define USBPD_PWR_VBUS_5V                 5000U
#define USBPD_PWR_VBUS_9V                 9000U
#define USBPD_PWR_VBUS_15V                15000U

/**
  * @brief  power timeout
  */
#define USBPD_PWR_TIMEOUT_PDO             250U         /* Timeout for PDO to PDO or PDO to APDO at 250ms */
#define USBPD_PWR_TIMEOUT_APDO            25U          /* Timeout for APDO to APDO at 25ms */

/**
  * @brief  Invalid value set during issue with voltage setting
  */
#define USBPD_PWR_INVALID_VALUE           0xFFFFFFFFU

/* On STM32H5xx-Nucleo Kit (MB1404), following setup has been used :
      VSENSE      => PA4 */
#define VSENSE_GPIO_PORT                  GPIOA
#define VSENSE_GPIO_PIN                   LL_GPIO_PIN_4
#define VSENSE_GPIO_ENABLE_CLOCK()        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);


/* PA4 is used as ADC1_IN18 input for ADC measurement of VBUS voltage :
   ADC1 Channel 18.
*/
#define VSENSE_ADC_INSTANCE               ADC1
#define VSENSE_ADC_RANK                   ADC_REGULAR_RANK_1
#define VSENSE_ADC_CHANNEL                ADC_CHANNEL_18

/* Enable ADC clock (core clock) */
#define VSENSE_ADC_ENABLE_CLOCK()         LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC);

/* Enable ADC clock (core clock) */
#define VSENSE_ADC_DISABLE_CLOCK()        do {                                                       \
                                               LL_AHB2_GRP1_ForceReset(LL_AHB2_GRP1_PERIPH_ADC);     \
                                               LL_AHB2_GRP1_ReleaseReset(LL_AHB2_GRP1_PERIPH_ADC);   \
                                               LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_ADC);   \
                                             } while (0);

/* Clock enabling for TCPP01 DB signal : PA9 */
#define GPIO_TCPP01_DB_PORT               GPIOA
#define GPIO_TCPP01_DB_PIN                LL_GPIO_PIN_9
#define GPIO_TCPP01_DB_CLK_ENABLE()       LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

/**
  * @}
  */

/** @addtogroup STM32H5XX_NUCLEO_USBPD_PWR_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32H5XX_NUCLEO_USBPD_PWR_Exported_Functions STM32H5xx-Nucleo USBPD PWR Exported Functions
  * @{
  */
/* Common functions */
int32_t BSP_USBPD_PWR_Init(uint32_t PortNum);

int32_t BSP_USBPD_PWR_Deinit(uint32_t PortNum);

int32_t BSP_USBPD_PWR_VBUSInit(uint32_t PortNum);

int32_t BSP_USBPD_PWR_VBUSDeInit(uint32_t PortNum);

int32_t BSP_USBPD_PWR_VBUSOn(uint32_t PortNum);

int32_t BSP_USBPD_PWR_VBUSOff(uint32_t PortNum);

int32_t BSP_USBPD_PWR_VBUSIsOn(uint32_t PortNum, uint8_t *pState);

int32_t BSP_USBPD_PWR_VBUSSetVoltage_Fixed(uint32_t PortNum,
                                           uint32_t VbusTargetInmv,
                                           uint32_t OperatingCurrent,
                                           uint32_t MaxOperatingCurrent);

int32_t BSP_USBPD_PWR_VBUSSetVoltage_Variable(uint32_t PortNum,
                                              uint32_t VbusTargetMaxInmv,
                                              uint32_t VbusTargetMinInmv,
                                              uint32_t OperatingCurrent,
                                              uint32_t MaxOperatingCurrent);

int32_t BSP_USBPD_PWR_VBUSSetVoltage_Battery(uint32_t PortNum,
                                             uint32_t VbusTargetMin,
                                             uint32_t VbusTargetMax,
                                             uint32_t OperatingPower,
                                             uint32_t MaxOperatingPower);

int32_t BSP_USBPD_PWR_VBUSSetVoltage_APDO(uint32_t PortNum,
                                          uint32_t VbusTargetInmv,
                                          uint32_t OperatingCurrent,
                                          int32_t Delta);

int32_t BSP_USBPD_PWR_SetVBUSDisconnectionThreshold(uint32_t PortNum,
                                                    uint32_t VoltageThreshold);

int32_t BSP_USBPD_PWR_RegisterVBUSDetectCallback(uint32_t PortNum,
                                                 USBPD_PWR_VBUSDetectCallbackFunc *pfnVBUSDetectCallback);

int32_t BSP_USBPD_PWR_VBUSGetVoltage(uint32_t PortNum, uint32_t *pVoltage);

int32_t BSP_USBPD_PWR_VBUSGetCurrent(uint32_t PortNum, int32_t *pCurrent);

int32_t BSP_USBPD_PWR_VBUSDischargeOn(uint32_t PortNum);

int32_t BSP_USBPD_PWR_VBUSDischargeOff(uint32_t PortNum);

int32_t BSP_USBPD_PWR_VCONNInit(uint32_t PortNum,
                                uint32_t CCPinId);
int32_t BSP_USBPD_PWR_VCONNDeInit(uint32_t PortNum,
                                  uint32_t CCPinId);
int32_t BSP_USBPD_PWR_VCONNOn(uint32_t PortNum,
                              uint32_t CCPinId);
int32_t BSP_USBPD_PWR_VCONNOff(uint32_t PortNum,
                               uint32_t CCPinId);

int32_t BSP_USBPD_PWR_VCONNIsOn(uint32_t PortNum,
                                uint32_t CCPinId, uint8_t *pState);

int32_t BSP_USBPD_PWR_VCONNDischargeOn(uint32_t PortNum);

int32_t BSP_USBPD_PWR_VCONNDischargeOff(uint32_t PortNum);

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

#endif /* STM32H5XX_NUCLEO_USBPD_PWR_H */
