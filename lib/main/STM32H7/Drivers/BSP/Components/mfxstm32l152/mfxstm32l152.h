/**
  ******************************************************************************
  * @file    mfxstm32l152.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          mfxstm32l152.c IO expander driver.
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
#ifndef MFXSTM32L152_H
#define MFXSTM32L152_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mfxstm32l152_reg.h"
#include <stddef.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup MFXSTM32L152
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup MFXSTM32L152_Exported_Types MFXSTM32L152 Exported Types
  * @{
  */
typedef struct
{
  uint32_t Pin;       /*!< Specifies the GPIO pins to be configured.
                           This parameter can be any value of @ref GPIO_pins_define */

  uint32_t Mode;      /*!< Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref IO_ModeTypedef */

  uint32_t Pull;      /*!< Specifies the Pull-up or Pull-Down activation for the selected pins.
                           This parameter can be a value of @ref GPIO_pull_define */
}MFXSTM32L152_IO_Init_t;

typedef struct
{
  uint16_t AmpliGain;       /*!< Specifies ampli gain value
                                 */
  uint16_t VddMin;          /*!< Specifies minimum MCU VDD can reach to protect MCU from reset
                                  */
  uint16_t Shunt0Value;     /*!< Specifies value of Shunt 0 if existing
                                 */
  uint16_t Shunt1Value;     /*!< Specifies value of Shunt 1 if existing
                                 */
  uint16_t Shunt2Value;     /*!< Specifies value of Shunt 2 if existing
                                 */
  uint16_t Shunt3Value;     /*!< Specifies value of Shunt 3 if existing
                                 */
  uint16_t Shunt4Value;     /*!< Specifies value of Shunt 4 if existing
                                  */
  uint16_t Shunt0StabDelay; /*!< Specifies delay of Shunt 0 stabilization if existing
                                  */
  uint16_t Shunt1StabDelay; /*!< Specifies delay of Shunt 1 stabilization if existing
                                  */
  uint16_t Shunt2StabDelay; /*!< Specifies delay of Shunt 2 stabilization if existing
                                  */
  uint16_t Shunt3StabDelay; /*!< Specifies delay of Shunt 3 stabilization if existing
                                  */
  uint16_t Shunt4StabDelay; /*!< Specifies delay of Shunt 4 stabilization if existing
                                  */
  uint8_t ShuntNbOnBoard;   /*!< Specifies number of shunts that are present on board
                                 This parameter can be a value of @ref IDD_shunt_number */
  uint8_t ShuntNbUsed;      /*!< Specifies number of shunts used for measurement
                                 This parameter can be a value of @ref IDD_shunt_number */
  uint8_t VrefMeasurement;  /*!< Specifies if Vref is automatically measured before each Idd measurement
                                 This parameter can be a value of @ref IDD_Vref_Measurement */
  uint8_t Calibration;      /*!< Specifies if calibration is done before each Idd measurement
                                  */
  uint8_t PreDelayUnit;     /*!< Specifies Pre delay unit
                                 This parameter can be a value of @ref IDD_PreDelay */
  uint8_t PreDelayValue;    /*!< Specifies Pre delay value in selected unit
                                  */
  uint8_t MeasureNb;        /*!< Specifies number of Measure to be performed
                                 This parameter can be a value between 1 and 256 */
  uint8_t DeltaDelayUnit;   /*!< Specifies Delta delay unit
                                  This parameter can be a value of @ref IDD_DeltaDelay */
  uint8_t DeltaDelayValue;  /*!< Specifies Delta delay between 2 measures
                                  value can be between 1 and 128 */
}MFXSTM32L152_IDD_Config_t;

typedef int32_t (*MFXSTM32L152_Init_Func)    (void);
typedef int32_t (*MFXSTM32L152_DeInit_Func)  (void);
typedef int32_t (*MFXSTM32L152_GetTick_Func) (void);
typedef int32_t (*MFXSTM32L152_Delay_Func)   (uint32_t);
typedef int32_t (*MFXSTM32L152_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*MFXSTM32L152_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  MFXSTM32L152_Init_Func          Init;
  MFXSTM32L152_DeInit_Func        DeInit;
  uint16_t                        Address;
  MFXSTM32L152_WriteReg_Func      WriteReg;
  MFXSTM32L152_ReadReg_Func       ReadReg;
  MFXSTM32L152_GetTick_Func       GetTick;
} MFXSTM32L152_IO_t;


typedef struct
{
  MFXSTM32L152_IO_t         IO;
  mfxstm32l152_ctx_t        Ctx;
  uint8_t                   IsInitialized;
} MFXSTM32L152_Object_t;

/* Touch screen driver structure initialization */
typedef struct
{
  int32_t ( *Init               )(MFXSTM32L152_Object_t *);
  int32_t ( *ReadID             )(MFXSTM32L152_Object_t *, uint32_t*);
  int32_t ( *Reset              )(MFXSTM32L152_Object_t *);
  int32_t ( *TS_Start           )(MFXSTM32L152_Object_t *);
  int32_t ( *TS_DetectTouch     )(MFXSTM32L152_Object_t *);
  int32_t ( *TS_GetXY           )(MFXSTM32L152_Object_t *, uint16_t*, uint16_t*);
  int32_t ( *TS_EnableIT        )(MFXSTM32L152_Object_t *);
  int32_t ( *TS_ClearIT         )(MFXSTM32L152_Object_t *);
  int32_t ( *TS_ITStatus        )(MFXSTM32L152_Object_t *);
  int32_t ( *TS_DisableIT       )(MFXSTM32L152_Object_t *);
}MFXSTM32L152_TS_Mode_t;

/* IO driver structure initialization */
typedef struct
{
  int32_t ( *Init               )(MFXSTM32L152_Object_t *, MFXSTM32L152_IO_Init_t *);
  int32_t ( *DeInit             )(MFXSTM32L152_Object_t *);
  int32_t ( *ReadID             )(MFXSTM32L152_Object_t *, uint32_t*);
  int32_t ( *Reset              )(MFXSTM32L152_Object_t *);
  int32_t ( *IO_Start           )(MFXSTM32L152_Object_t *, uint32_t);
  int32_t ( *IO_WritePin        )(MFXSTM32L152_Object_t *, uint32_t, uint8_t);
  int32_t ( *IO_ReadPin         )(MFXSTM32L152_Object_t *, uint32_t);
  int32_t ( *IO_EnableIT        )(MFXSTM32L152_Object_t *);
  int32_t ( *IO_DisableIT       )(MFXSTM32L152_Object_t *);
  int32_t ( *IO_ITStatus        )(MFXSTM32L152_Object_t *, uint32_t);
  int32_t ( *IO_ClearIT         )(MFXSTM32L152_Object_t *, uint32_t);
}MFXSTM32L152_IO_Mode_t;

/* IDD driver structure initialization */
typedef struct
{
  int32_t ( *Init               )(MFXSTM32L152_Object_t *);
  int32_t ( *DeInit             )(MFXSTM32L152_Object_t *);
  int32_t ( *ReadID             )(MFXSTM32L152_Object_t *, uint32_t*);
  int32_t ( *Reset              )(MFXSTM32L152_Object_t *);
  int32_t ( *LowPower           )(MFXSTM32L152_Object_t *);
  int32_t ( *WakeUp             )(MFXSTM32L152_Object_t *);
  int32_t ( *IDD_Start          )(MFXSTM32L152_Object_t *);
  int32_t ( *IDD_Config         )(MFXSTM32L152_Object_t *, MFXSTM32L152_IDD_Config_t*);
  int32_t ( *IDD_GetValue       )(MFXSTM32L152_Object_t *, uint32_t*);
  int32_t ( *IDD_EnableIT       )(MFXSTM32L152_Object_t *);
  int32_t ( *IDD_DisableIT      )(MFXSTM32L152_Object_t *);
  int32_t ( *IDD_GetITStatus    )(MFXSTM32L152_Object_t *);
  int32_t ( *IDD_ClearIT        )(MFXSTM32L152_Object_t *);
  int32_t ( *Error_EnableIT     )(MFXSTM32L152_Object_t *);
  int32_t ( *Error_ClearIT      )(MFXSTM32L152_Object_t *);
  int32_t ( *Error_GetITStatus  )(MFXSTM32L152_Object_t *);
  int32_t ( *Error_DisableIT    )(MFXSTM32L152_Object_t *);
  int32_t ( *Error_ReadSrc      )(MFXSTM32L152_Object_t *);
  int32_t ( *Error_ReadMsg      )(MFXSTM32L152_Object_t *);
}MFXSTM32L152_IDD_Mode_t;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup MFXSTM32L152_Exported_Constants MFXSTM32L152 Exported Constants
  * @{
  */
#define MFXSTM32L152_OK                      (0)
#define MFXSTM32L152_ERROR                   (-1)

/* MFXSTM32L152_REG_ADR_ID values */
#define MFXSTM32L152_ID                       0x7BU
#define MFXSTM32L152_ID_2                     0x79U

/* MFXSTM32L152_REG_ADR_SYS_CTRL values */
#define MFXSTM32L152_SWRST                    0x80U
#define MFXSTM32L152_STANDBY                  0x40U
#define MFXSTM32L152_ALTERNATE_GPIO_EN        0x08U /* If IDD and TS are enabled they take automatically the AF pins*/
#define MFXSTM32L152_IDD_EN                   0x04U
#define MFXSTM32L152_TS_EN                    0x02U
#define MFXSTM32L152_GPIO_EN                  0x01U

/* MFXSTM32L152_REG_ADR_ERROR_SRC values */
#define MFXSTM32L152_IDD_ERROR_SRC            0x04U  /* Error raised by Idd */
#define MFXSTM32L152_TS_ERROR_SRC             0x02U  /* Error raised by Touch Screen */
#define MFXSTM32L152_GPIO_ERROR_SRC           0x01U  /* Error raised by Gpio */

/* MFXSTM32L152_REG_ADR_MFX_IRQ_OUT values */
#define MFXSTM32L152_OUT_PIN_TYPE_OPENDRAIN   0x00U
#define MFXSTM32L152_OUT_PIN_TYPE_PUSHPULL    0x01U
#define MFXSTM32L152_OUT_PIN_POLARITY_LOW     0x00U
#define MFXSTM32L152_OUT_PIN_POLARITY_HIGH    0x02U

/* REG_ADR_IRQ_SRC_EN, REG_ADR_IRQ_PENDING & REG_ADR_IRQ_ACK values */
#define MFXSTM32L152_IRQ_TS_OVF               0x80U  /* TouchScreen FIFO Overflow irq*/
#define MFXSTM32L152_IRQ_TS_FULL              0x40U  /* TouchScreen FIFO Full irq*/
#define MFXSTM32L152_IRQ_TS_TH                0x20U  /* TouchScreen FIFO threshold triggered irq*/
#define MFXSTM32L152_IRQ_TS_NE                0x10U  /* TouchScreen FIFO Not Empty irq*/
#define MFXSTM32L152_IRQ_TS_DET               0x08U  /* TouchScreen Detect irq*/
#define MFXSTM32L152_IRQ_ERROR                0x04U  /* Error message from MFXSTM32L152 firmware irq */
#define MFXSTM32L152_IRQ_IDD                  0x02U  /* IDD function irq */
#define MFXSTM32L152_IRQ_GPIO                 0x01U  /* General GPIO irq (only for SRC_EN and PENDING) */
#define MFXSTM32L152_IRQ_ALL                  0xFFU  /* All global interrupts          */
#define MFXSTM32L152_IRQ_TS                   (MFXSTM32L152_IRQ_TS_DET | MFXSTM32L152_IRQ_TS_NE |  MFXSTM32L152_IRQ_TS_TH | MFXSTM32L152_IRQ_TS_FULL | MFXSTM32L152_IRQ_TS_OVF)

/* GPIO: IO Pins definition */
#define MFXSTM32L152_GPIO_PIN_0                0x0001U
#define MFXSTM32L152_GPIO_PIN_1                0x0002U
#define MFXSTM32L152_GPIO_PIN_2                0x0004U
#define MFXSTM32L152_GPIO_PIN_3                0x0008U
#define MFXSTM32L152_GPIO_PIN_4                0x0010U
#define MFXSTM32L152_GPIO_PIN_5                0x0020U
#define MFXSTM32L152_GPIO_PIN_6                0x0040U
#define MFXSTM32L152_GPIO_PIN_7                0x0080U

#define MFXSTM32L152_GPIO_PIN_8                0x0100U
#define MFXSTM32L152_GPIO_PIN_9                0x0200U
#define MFXSTM32L152_GPIO_PIN_10               0x0400U
#define MFXSTM32L152_GPIO_PIN_11               0x0800U
#define MFXSTM32L152_GPIO_PIN_12               0x1000U
#define MFXSTM32L152_GPIO_PIN_13               0x2000U
#define MFXSTM32L152_GPIO_PIN_14               0x4000U
#define MFXSTM32L152_GPIO_PIN_15               0x8000U

#define MFXSTM32L152_GPIO_PIN_16               0x010000U
#define MFXSTM32L152_GPIO_PIN_17               0x020000U
#define MFXSTM32L152_GPIO_PIN_18               0x040000U
#define MFXSTM32L152_GPIO_PIN_19               0x080000U
#define MFXSTM32L152_GPIO_PIN_20               0x100000U
#define MFXSTM32L152_GPIO_PIN_21               0x200000U
#define MFXSTM32L152_GPIO_PIN_22               0x400000U
#define MFXSTM32L152_GPIO_PIN_23               0x800000U

#define MFXSTM32L152_AGPIO_PIN_0               MFXSTM32L152_GPIO_PIN_16
#define MFXSTM32L152_AGPIO_PIN_1               MFXSTM32L152_GPIO_PIN_17
#define MFXSTM32L152_AGPIO_PIN_2               MFXSTM32L152_GPIO_PIN_18
#define MFXSTM32L152_AGPIO_PIN_3               MFXSTM32L152_GPIO_PIN_19
#define MFXSTM32L152_AGPIO_PIN_4               MFXSTM32L152_GPIO_PIN_20
#define MFXSTM32L152_AGPIO_PIN_5               MFXSTM32L152_GPIO_PIN_21
#define MFXSTM32L152_AGPIO_PIN_6               MFXSTM32L152_GPIO_PIN_22
#define MFXSTM32L152_AGPIO_PIN_7               MFXSTM32L152_GPIO_PIN_23

#define MFXSTM32L152_GPIO_PINS_ALL             0xFFFFFFU

/* GPIO: constant */
#define MFXSTM32L152_GPIO_DIR_IN                0x0U
#define MFXSTM32L152_GPIO_DIR_OUT               0x1U
#define MFXSTM32L152_IRQ_GPI_EVT_LEVEL          0x0U
#define MFXSTM32L152_IRQ_GPI_EVT_EDGE           0x1U
#define MFXSTM32L152_IRQ_GPI_TYPE_LLFE          0x0U  /* Low Level Falling Edge */
#define MFXSTM32L152_IRQ_GPI_TYPE_HLRE          0x1U  /*High Level Raising Edge */
#define MFXSTM32L152_GPI_WITHOUT_PULL_RESISTOR  0x0U
#define MFXSTM32L152_GPI_WITH_PULL_RESISTOR     0x1U
#define MFXSTM32L152_GPO_PUSH_PULL              0x0U
#define MFXSTM32L152_GPO_OPEN_DRAIN             0x1U
#define MFXSTM32L152_GPIO_PULL_DOWN             0x0U
#define MFXSTM32L152_GPIO_PULL_UP               0x1U

#define MFXSTM32L152_GPIO_NOPULL                0x0U   /*!< No Pull-up or Pull-down activation  */
#define MFXSTM32L152_GPIO_PULLUP                0x1U   /*!< Pull-up activation                  */
#define MFXSTM32L152_GPIO_PULLDOWN              0x2U   /*!< Pull-down activation                */

#define MFXSTM32L152_GPIO_MODE_OFF              0x0U  /* when pin isn't used*/
#define MFXSTM32L152_GPIO_MODE_ANALOG           0x1U  /* analog mode */
#define MFXSTM32L152_GPIO_MODE_INPUT            0x2U  /* input floating */
#define MFXSTM32L152_GPIO_MODE_OUTPUT_OD        0x3U  /* Open Drain output without internal resistor */
#define MFXSTM32L152_GPIO_MODE_OUTPUT_PP        0x4U  /* PushPull output without internal resistor */
#define MFXSTM32L152_GPIO_MODE_IT_RISING_EDGE   0x5U  /* float input - irq detect on rising edge */
#define MFXSTM32L152_GPIO_MODE_IT_FALLING_EDGE  0x6U  /* float input - irq detect on falling edge */
#define MFXSTM32L152_GPIO_MODE_IT_LOW_LEVEL     0x7U  /* float input - irq detect on low level */
#define MFXSTM32L152_GPIO_MODE_IT_HIGH_LEVEL    0x8U  /* float input - irq detect on high level */

/* TS registers masks */
#define MFXSTM32L152_TS_CTRL_STATUS             0x08U
#define MFXSTM32L152_TS_CLEAR_FIFO              0x80U

/** @defgroup IDD_Control_Register_Defines  IDD Control Register Defines
  * @{
  */
/* IDD control register masks */
#define MFXSTM32L152_IDD_CTRL_REQ                       0x01U
#define MFXSTM32L152_IDD_CTRL_SHUNT_NB                  0x0EU
#define MFXSTM32L152_IDD_CTRL_VREF_DIS                  0x40U
#define MFXSTM32L152_IDD_CTRL_CAL_DIS                   0x80U

/* IDD Shunt Number */
#define MFXSTM32L152_IDD_SHUNT_NB_1                      0x01U
#define MFXSTM32L152_IDD_SHUNT_NB_2                      0x02U
#define MFXSTM32L152_IDD_SHUNT_NB_3                      0x03U
#define MFXSTM32L152_IDD_SHUNT_NB_4                      0x04U
#define MFXSTM32L152_IDD_SHUNT_NB_5                      0x05U

/* Vref Measurement */
#define MFXSTM32L152_IDD_VREF_AUTO_MEASUREMENT_ENABLE    0x00U
#define MFXSTM32L152_IDD_VREF_AUTO_MEASUREMENT_DISABLE   0x70U

/* IDD Calibration */
#define MFXSTM32L152_IDD_AUTO_CALIBRATION_ENABLE         0x00U
#define MFXSTM32L152_IDD_AUTO_CALIBRATION_DISABLE        0x80U
/**
  * @}
  */

/** @defgroup IDD_PreDelay_Defines  IDD PreDelay Defines
  * @{
  */
/* IDD PreDelay masks */
#define MFXSTM32L152_IDD_PREDELAY_UNIT                   0x80U
#define MFXSTM32L152_IDD_PREDELAY_VALUE                  0x7FU


/* IDD PreDelay unit */
#define MFXSTM32L152_IDD_PREDELAY_0_5_MS                 0x00U
#define MFXSTM32L152_IDD_PREDELAY_20_MS                  0x80U
/**
  * @}
  */

/** @defgroup IDD_DeltaDelay_Defines  IDD Delta DElay Defines
  * @{
  */
/* IDD Delta Delay masks */
#define MFXSTM32L152_IDD_DELTADELAY_UNIT                 0x80U
#define MFXSTM32L152_IDD_DELTADELAY_VALUE                0x7FU

/* IDD Delta Delay unit */
#define MFXSTM32L152_IDD_DELTADELAY_0_5_MS               0x00U
#define MFXSTM32L152_IDD_DELTADELAY_20_MS                0x80U

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup MFXSTM32L152_Exported_Functions
  * @{
  */

/* MFXSTM32L152 Control functions */
int32_t MFXSTM32L152_RegisterBusIO (MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IO_t *pIO);
int32_t MFXSTM32L152_Init(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_DeInit(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_Reset(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_ReadID(MFXSTM32L152_Object_t *pObj, uint32_t *Id);
int32_t MFXSTM32L152_ReadFwVersion(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_LowPower(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_WakeUp(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_EnableITSource(MFXSTM32L152_Object_t *pObj, uint8_t Source);
int32_t MFXSTM32L152_DisableITSource(MFXSTM32L152_Object_t *pObj, uint8_t Source);
int32_t MFXSTM32L152_GlobalITStatus(MFXSTM32L152_Object_t *pObj, uint8_t Source);
int32_t MFXSTM32L152_ClearGlobalIT(MFXSTM32L152_Object_t *pObj, uint8_t Source);
int32_t MFXSTM32L152_SetIrqOutPinPolarity(MFXSTM32L152_Object_t *pObj, uint8_t Polarity);
int32_t MFXSTM32L152_SetIrqOutPinType(MFXSTM32L152_Object_t *pObj, uint8_t Type);


/* MFXSTM32L152 IO functionalities functions */
int32_t MFXSTM32L152_IO_Init(MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IO_Init_t *IoInit);
int32_t MFXSTM32L152_IO_Start(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin);
int32_t MFXSTM32L152_IO_WritePin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t PinState);
int32_t MFXSTM32L152_IO_ReadPin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin);
int32_t MFXSTM32L152_IO_EnableIT(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IO_DisableIT(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IO_ITStatus(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin);
int32_t MFXSTM32L152_IO_ClearIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin);
int32_t MFXSTM32L152_IO_InitPin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Direction);
int32_t MFXSTM32L152_IO_EnableAF(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IO_DisableAF(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IO_SetIrqTypeMode(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Type);
int32_t MFXSTM32L152_IO_SetIrqEvtMode(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Evt);
int32_t MFXSTM32L152_IO_EnablePinIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin);
int32_t MFXSTM32L152_IO_DisablePinIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin);

/* MFXSTM32L152 Touch screen functionalities functions */
int32_t MFXSTM32L152_TS_Start(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_TS_DetectTouch(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_TS_GetXY(MFXSTM32L152_Object_t *pObj, uint16_t *X, uint16_t *Y);
int32_t MFXSTM32L152_TS_EnableIT(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_TS_DisableIT(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_TS_ITStatus (MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_TS_ClearIT (MFXSTM32L152_Object_t *pObj);

/* MFXSTM32L152 IDD current measurement functionalities functions */
int32_t MFXSTM32L152_IDD_Start(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IDD_Config(MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IDD_Config_t * MfxIddConfig);
int32_t MFXSTM32L152_IDD_ConfigShuntNbLimit(MFXSTM32L152_Object_t *pObj, uint8_t ShuntNbLimit);
int32_t MFXSTM32L152_IDD_GetValue(MFXSTM32L152_Object_t *pObj, uint32_t *ReadValue);
int32_t MFXSTM32L152_IDD_GetShuntUsed(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IDD_EnableIT(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IDD_ClearIT(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IDD_GetITStatus(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_IDD_DisableIT(MFXSTM32L152_Object_t *pObj);

/* MFXSTM32L152 Error management functions */
int32_t MFXSTM32L152_Error_ReadSrc(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_Error_ReadMsg(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_Error_EnableIT(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_Error_ClearIT(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_Error_GetITStatus(MFXSTM32L152_Object_t *pObj);
int32_t MFXSTM32L152_Error_DisableIT(MFXSTM32L152_Object_t *pObj);

/**
  * @}
  */

/* Touch screen driver structure */
extern MFXSTM32L152_TS_Mode_t MFXSTM32L152_TS_Driver;

/* IO driver structure */
extern MFXSTM32L152_IO_Mode_t MFXSTM32L152_IO_Driver;

/* IDD driver structure */
extern MFXSTM32L152_IDD_Mode_t MFXSTM32L152_IDD_Driver;


#ifdef __cplusplus
}
#endif
#endif /* MFXSTM32L152_H */


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
