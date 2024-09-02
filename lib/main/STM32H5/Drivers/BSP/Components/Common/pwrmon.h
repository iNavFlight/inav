/**
  ******************************************************************************
  * @file    pwrmon.h
  * @author  MCD Application Team
  * @brief   This header file contains the functions prototypes for the
  *          Current/Power Monitor device driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PWRMON_H
#define __PWRMON_H

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

/** @addtogroup PWRMON
  * @{
  */

/** @defgroup PWRMON_Exported_Types
  * @{
  */

/** @defgroup PWRMON_Operating_Mode_enum  Power Monitor Operating Mode enums
  * @{
  */
typedef enum
{
  OPERATING_MODE_TRIGGERED = 0,
  OPERATING_MODE_CONTINUOUS,
  OPERATING_MODE_NB
} PWRMON_OperatingMode_t;
/**
  * @}
  */

/** @defgroup PWRMON_Conversion_Time_enum  Power Monitor Conversion_Time enums
  * @{
  */
typedef enum
{
  CONVERT_TIME_140 = 0,
  CONVERT_TIME_204,
  CONVERT_TIME_332,
  CONVERT_TIME_588,
  CONVERT_TIME_1100,
  CONVERT_TIME_2116,
  CONVERT_TIME_4156,
  CONVERT_TIME_8244,
  CONVERT_TIME_NB
} PWRMON_ConvertTime_t;
/**
  * @}
  */

/** @defgroup PWRMON_Conversion_Time_enum  Power Monitor Conversion_Time enums
  * @{
  */
typedef enum
{
  AVERAGING_MODE_1 = 0,
  AVERAGING_MODE_4,
  AVERAGING_MODE_16,
  AVERAGING_MODE_64,
  AVERAGING_MODE_128,
  AVERAGING_MODE_256,
  AVERAGING_MODE_512,
  AVERAGING_MODE_1024,
  AVERAGING_MODE_NB
} PWRMON_AveragingMode_t;
/**
  * @}
  */

/** @defgroup PWRMON_Device_Configuration_structure  Power Monitor Device Configuration structure
  * @{
  */
typedef struct
{
  PWRMON_ConvertTime_t    ShuntConvertTime;
  PWRMON_ConvertTime_t    BusConvertTime;
  PWRMON_AveragingMode_t  AveragingMode;
} PWRMON_Config_t;
/**
  * @}
  */

/** @defgroup PWRMON_Alert_Polarity_enum  Power Monitor Alert Polarity enums
  * @{
  */
typedef enum
{
  ALERT_POLARITY_NORMAL = 0,
  ALERT_POLARITY_INVERTED,
  ALERT_POLARITY_NB
} PWRMON_AlertPolarity_t;
/**
  * @}
  */

/** @defgroup PWRMON_Alert_Latch_Enable_enum  Power Monitor Alert Latch Enable enums
  * @{
  */
typedef enum
{
  ALERT_LATCH_DISABLE = 0,
  ALERT_LATCH_ENABLE,
  ALERT_LATCH_NB
} PWRMON_AlertLatchEnable_t;
/**
  * @}
  */

/** @defgroup PWRMON_Alert_Function_enum  Power Monitor Alert Function enums
  * @{
  */
typedef enum
{
  ALERT_FUNCTION_NONE = 0,
  ALERT_FUNCTION_SOL,
  ALERT_FUNCTION_SUL,
  ALERT_FUNCTION_BOL,
  ALERT_FUNCTION_BUL,
  ALERT_FUNCTION_POL,
  ALERT_FUNCTION_NB,
} PWRMON_AlertFunction_t;
/**
  * @}
  */

/** @defgroup PWRMON_Alert_Configuration_structure  Power Monitor Alert Configuration structure
  * @{
  */
typedef struct
{
  PWRMON_AlertPolarity_t    Polarity;
  PWRMON_AlertLatchEnable_t LatchEnable;
} PWRMON_AlertPinConfig_t;
/**
  * @}
  */

/** @defgroup PWRMON_Voltage_Input_enum  Power Monitor Voltage Input enums
  * @{
  */
typedef enum
{
  VOLTAGE_INPUT_SHUNT = 0,
  VOLTAGE_INPUT_BUS,
  VOLTAGE_INPUT_ALL,
  VOLTAGE_INPUT_NB
} PWRMON_InputSignal_t;
/**
  * @}
  */

/** @defgroup PWRMON_Flag_enum  Power Monitor Flag enums
  * @{
  */
typedef enum
{
  FLAG_ALERT_FUNCTION = 0,
  FLAG_CONVERSION_READY,
  FLAG_MATH_OVERFLOW,
  FLAG_NB
} PWRMON_Flag_t;
/**
  * @}
  */

/** @defgroup PWRMON_Driver_structure  Power Monitor Driver structure
  * @{
  */
typedef struct
{
  void (*Init)(uint16_t, PWRMON_Config_t *);
  void (*DeInit)(uint16_t);
  uint16_t (*ReadId)(uint16_t);
  void (*Reset)(uint16_t);
  void (*SetCalibration)(uint16_t, uint16_t);
  uint16_t (*GetCalibration)(uint16_t);
  void (*SetAlertFunction)(uint16_t, PWRMON_AlertFunction_t);
  PWRMON_AlertFunction_t (*GetAlertFunction)(uint16_t);
  void (*AlertPinConfig)(uint16_t, PWRMON_AlertPinConfig_t *);
  void (*SetVBusThreshold)(uint16_t, uint16_t);
  uint16_t (*GetVBusThreshold)(uint16_t);
  void (*SetVShuntThreshold)(uint16_t, int16_t);
  int16_t (*GetVShuntThreshold)(uint16_t);
  void (*SetPowerThreshold)(uint16_t, uint32_t);
  uint32_t (*GetPowerThreshold)(uint16_t);
  void (*AlertThresholdEnableIT)(uint16_t);
  void (*AlertThresholdDisableIT)(uint16_t);
  void (*ConversionReadyEnableIT)(uint16_t);
  void (*ConversionReadyDisableIT)(uint16_t);
  void (*StartConversion)(uint16_t, PWRMON_InputSignal_t, PWRMON_OperatingMode_t);
  void (*StopConversion)(uint16_t);
  uint16_t (*GetVBus)(uint16_t);
  int16_t (*GetVShunt)(uint16_t);
  uint16_t (*GetPower)(uint16_t);
  int16_t (*GetCurrent)(uint16_t);
  uint8_t (*GetFlag)(uint16_t, PWRMON_Flag_t);
} PWRMON_Drv_t;
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

#endif /* __PWRMON_H */
