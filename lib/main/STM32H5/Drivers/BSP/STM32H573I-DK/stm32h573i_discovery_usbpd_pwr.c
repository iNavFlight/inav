/**
  ******************************************************************************
  * @file    stm32h573i_discovery_usbpd_pwr.c
  * @author  MCD Application Team
  * @brief   This file provides a set of firmware functions to manage USB-PD Power
  *          available on STM32H573I-DK board(MB1677) from STMicroelectronics :
  *            - VBUS control
  *            - VBUS voltage/current measurement
  *            - VCONN control
  *            - VBUS presence detection
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

/* Includes ------------------------------------------------------------------*/
#include "stm32h573i_discovery_usbpd_pwr.h"
#include "stm32h573i_discovery_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H573I_DK
  * @{
  */

/** @defgroup STM32H573I_DK_USBPD_PWR USBPD_PWR
  * @{
  */

/** @defgroup STM32H573I_DK_USBPD_PWR_Exported_Variables USBPD_PWR Exported Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32H573I_DK_USBPD_PWR_Private_Typedef USBPD_PWR Private Typedef
  * @{
  */
/**
  * @brief  Port HW configuration description
  */
typedef struct
{
  uint8_t                    Type;              /*!< HW component associated to Port          */
  uint8_t                    Address;           /*!< I2C address (if any) associated to Port  */
} USBPD_PWR_PortConfig_t;

/**
  * @brief  Port BSP Power status
  */
typedef struct
{
  uint8_t                           IsInitialized;     /*!< Port Context Initialization status       */
  uint8_t                           IsItEnabled;       /*!< Port IT enabled status                   */
  USBPD_PWR_PowerRoleTypeDef        PwrRole;           /*!< Port current power role (SRC or SNK)     */
  USBPD_PWR_PowerModeTypeDef        PwrSavingMode;     /*!< Port current power saving mode           */
  uint32_t                          LastFaultTick;     /*!< Current Tick on last detected fault      */
  USBPD_PWR_VBUSDetectCallbackFunc *VBUSDetectCallback;/*!< Port callback for VBUS detection event   */
} USBPD_PWR_PortStatus_t;

/**
  * @}
  */

/** @defgroup STM32H573I_DK_USBPD_PWR_Private_Constants USBPD_PWR Private Constants
  * @{
  */

/* Maximum digital value of the ADC output (12 Bits resolution)
   To converting ADC measurement to an absolute voltage value:
   VCHANNELx = ADCx_DATA x (VDD/ADC_FULL_SCALE)
  */
#define ADC_FULL_SCALE       (0x0FFFU)

#define VDDA_APPLI            (3300U)

/* Minimum time between 2 detected faults allowing to consider that
   fault could be due to a false detection (could be recovered).
   => if 2 faults occurred in less than that duration, no recovery will be executed */
#define USBPD_PWR_FAULT_MIN_TIME_RECOVERY             (500U)             /* 500 ms */

/**
  * @}
  */

/** @defgroup STM32H573I_DK_USBPD_PWR_Private_Macros USBPD_PWR Private Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup STM32H573I_DK_USBPD_PWR_Private_Functions USBPD_PWR Private Functions
  * @{
  */
static void     MX_ADC_Init(void);
static void     PWR_TCPP0203_GPIOConfigInit(uint32_t PortNum);
static void     PWR_TCPP0203_ITConfigInit(uint32_t PortNum);
static int32_t  PWR_TCPP0203_BUSConfigInit(uint32_t PortNum, uint16_t Address);
static int32_t  PWR_TCPP0203_ConfigDeInit(uint32_t PortNum);
static void     PWR_TCPP0203_EventCallback(uint32_t PortNum);

static uint32_t PWR_TCPP0203_ConvertADCDataToVoltage(uint32_t ADCData, uint32_t Ra, uint32_t Rb);
static int32_t  PWR_TCPP0203_ConvertADCDataToCurrent(uint32_t ADCData);

/**
  * @}
  */

/** @defgroup STM32H573I_DK_USBPD_PWR_Private_Variables USBPD_PWR Private Variables
  * @{
  */
static USBPD_PWR_PortConfig_t USBPD_PWR_Port_Configs[USBPD_PWR_INSTANCES_NBR] =
{
  {
    USBPD_PWR_HW_CONFIG_TYPE_TCPP03,             /* Port 0 : TCPP03 Type                             */
    TCPP0203_I2C_ADDRESS_X68,                    /* TCPP03 on shield (Address 0x34)                  */
  },
};

static USBPD_PWR_PortStatus_t USBPD_PWR_Port_Status[USBPD_PWR_INSTANCES_NBR] =
{
  {0U, 0U, POWER_ROLE_SINK, USBPD_PWR_MODE_OFF, 0U, NULL},
};

static TCPP0203_Object_t      USBPD_PWR_PortCompObj[USBPD_PWR_INSTANCES_NBR] = { 0 };

/**
  * @}
  */

/** @addtogroup STM32H573I_DK_USBPD_PWR_Exported_Functions
  * @{
  */

/**
  * @brief  Global initialization of PWR resource used by USB-PD
  * @param  PortNum Type-C port identifier
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_Init(uint32_t PortNum)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_Init --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    if (USBPD_PWR_Port_Status[PortNum].IsInitialized == 0U)
    {
      /* Initialisation according to HW configuration type of port */
      switch (USBPD_PWR_Port_Configs[PortNum].Type)
      {
        case USBPD_PWR_HW_CONFIG_TYPE_TCPP02:
        case USBPD_PWR_HW_CONFIG_TYPE_TCPP03:
          /* Set default Role to SNK */
          USBPD_PWR_Port_Status[PortNum].PwrRole = POWER_ROLE_SINK;

          /* Set default Power Mode to Hibernate */
          USBPD_PWR_Port_Status[PortNum].PwrSavingMode = USBPD_PWR_MODE_HIBERNATE;

          /* Reset port callback for VBUS detection event */
          USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback = NULL;

          /* Reset last detected fault Tick */
          USBPD_PWR_Port_Status[PortNum].LastFaultTick = 0;

          /* Initialize required HW for GPIO, BUS communication, Interrupts */
          /* Initialize required GPIOs */
          PWR_TCPP0203_GPIOConfigInit(PortNum);

          /* Initialize EXTI for FLGn signal */
          PWR_TCPP0203_ITConfigInit(PortNum);

          /* Enable component */
          TCPP0203_PORT0_ENABLE_GPIO_SET();

          /* Initialize required BUS for communication */
          ret = PWR_TCPP0203_BUSConfigInit(PortNum, USBPD_PWR_Port_Configs[PortNum].Address);
          break;

        case USBPD_PWR_HW_CONFIG_TYPE_DEFAULT:
        case USBPD_PWR_HW_CONFIG_TYPE_TCPP01:
        default:
          ret = BSP_ERROR_WRONG_PARAM;
          break;
      }

      if (ret == BSP_ERROR_NONE)
      {
        /* Set Initialisation flag */
        USBPD_PWR_Port_Status[PortNum].IsInitialized = 1U;
      }
    }
  }
  return ret;
}

/**
  * @brief  Global de-initialization of PWR resource used by USB-PD
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_Deinit(uint32_t PortNum)
{
  int32_t ret;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_DeInit --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    /* Deinitialize Power config for port */
    switch (USBPD_PWR_Port_Configs[PortNum].Type)
    {
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP02:
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP03:
        /* Perform any treatments required for de-initialization for GPIO, BUS communication, Interrupts */
        ret = PWR_TCPP0203_ConfigDeInit(PortNum);
        break;

      case USBPD_PWR_HW_CONFIG_TYPE_DEFAULT:
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP01:
      default:
        ret = BSP_ERROR_WRONG_PARAM;
        break;
    }

    /* Reset Initialisation flag */
    USBPD_PWR_Port_Status[PortNum].IsInitialized = 0U;
  }

  return ret;
}

/**
  * @brief  Assign Power role for current Port (Source or Sink)
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  Role Type-C port role
  *         This parameter can take one of the following values:
  *         @arg @ref POWER_ROLE_SOURCE
  *         @arg @ref POWER_ROLE_SINK
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_SetRole(uint32_t PortNum, USBPD_PWR_PowerRoleTypeDef Role)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    switch (Role)
    {
      case POWER_ROLE_SOURCE:
        /* Port Role is now SRC */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
        BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_SetRole : SRC --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
        /* Set port power Role */
        USBPD_PWR_Port_Status[PortNum].PwrRole = Role;

        /* Ensure that TCPP0203 mode is not set to Hibernate */
        if (USBPD_PWR_Port_Status[PortNum].PwrSavingMode == USBPD_PWR_MODE_HIBERNATE)
        {
          if (TCPP0203_SetPowerMode(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_POWER_MODE_LOWPOWER) != TCPP0203_OK)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
          else
          {
            USBPD_PWR_Port_Status[PortNum].PwrSavingMode = USBPD_PWR_MODE_LOWPOWER;
          }
        }
        break;

      case POWER_ROLE_SINK:
        /* Port Role is now SNK */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
        BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_SetRole : SNK --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
        /* Set port power Role */
        USBPD_PWR_Port_Status[PortNum].PwrRole = Role;
        break;

      default:
        ret = BSP_ERROR_WRONG_PARAM;
        break;
    }
  }

  return ret;
}


/**
  * @brief  Set operating mode of Port regarding power saving constraints
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  PwrMode Type-C port power saving mode
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_MODE_OFF
  *         @arg @ref USBPD_PWR_MODE_HIBERNATE
  *         @arg @ref USBPD_PWR_MODE_LOWPOWER
  *         @arg @ref USBPD_PWR_MODE_NORMAL
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_SetPowerMode(uint32_t PortNum, USBPD_PWR_PowerModeTypeDef PwrMode)
{
  uint8_t flg_reg;
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_SetPowerMode --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    /* Evaluate requested Power config for port */
    switch (PwrMode)
    {
      case USBPD_PWR_MODE_OFF:
        /* Port should enter OFF mode : Reset TCPP0203 Reg0 */
        if (TCPP0203_Reset(&USBPD_PWR_PortCompObj[PortNum]) != TCPP0203_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        break;

      case USBPD_PWR_MODE_HIBERNATE:
        /* Port Mode is Hibernate : (Default state at startup) suitable for SNK role
             - Dead Batteries Enabled,
             - Gate Driver Consumer controlled by I2C, Gate Driver Provider OFF
             - waiting for VBUS_OK in FLGn (interrupt)
        */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
        BSP_USBPD_PWR_TRACE(PortNum, "-- Hibernate --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
        if (TCPP0203_SetPowerMode(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_POWER_MODE_HIBERNATE) != TCPP0203_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }

        /* Check if VBUS is present (in case of SNK VBUS powered device) */
        /* Read Flags register (FLGn) */
        if (TCPP0203_ReadFlagRegister(&USBPD_PWR_PortCompObj[PortNum], &flg_reg) == TCPP0203_OK)
        {
          /* If FLGn has been set to 0 in LOW POWER or HIBERNATE mode,
             it is expected that a VBUS_OK condition is detected.
             In this case, if any, vBus Detection callback could be called */
          if ((flg_reg & TCPP0203_FLAG_VBUS_OK_SET) == TCPP0203_FLAG_VBUS_OK_SET)
          {
            if (USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback != NULL)
            {
              /* In case of TCPP03 in Hibernate Mode (SNK config),
                 Mode is set to Low power to enable TCPP03 behavior on CC lines */
              /* Switch to Low Power mode */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
              BSP_USBPD_PWR_TRACE(PortNum, "-- Low Power --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
              if (TCPP0203_SetPowerMode(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_POWER_MODE_LOWPOWER) != TCPP0203_OK)
              {
                ret = BSP_ERROR_COMPONENT_FAILURE;
              }

              USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback(PortNum, VBUS_CONNECTED);
            }
          }
          else
          {
            /* VBUS indication not present in FLGn flags : nothing more to be done */
          }
        }
        break;

      case USBPD_PWR_MODE_LOWPOWER:
        /* Port Mode is Low Power : suitable while toggling SNK/SRC is case of DRP
             - Dead Batteries Disabled,
             - Gate Driver Consumer or Provider controlled by I2C
             - waiting for VBUS_OK in FLGn (interrupt)
        */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
        BSP_USBPD_PWR_TRACE(PortNum, "-- Low Power --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
        if (TCPP0203_SetPowerMode(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_POWER_MODE_LOWPOWER) != TCPP0203_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        break;

      case USBPD_PWR_MODE_NORMAL:
        /* Port Mode is Normal : No power saving measure (Explicit contract in place) */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
        BSP_USBPD_PWR_TRACE(PortNum, "-- Normal --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
        if (TCPP0203_SetPowerMode(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_POWER_MODE_NORMAL) != TCPP0203_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        break;

      default:
        ret = BSP_ERROR_WRONG_PARAM;
        break;
    }

    /* Set port low power operating mode */
    USBPD_PWR_Port_Status[PortNum].PwrSavingMode = PwrMode;
  }

  return ret;
}

/**
  * @brief  Get operating mode of Port regarding power saving constraints
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  PwrMode Pointer on current Type-C port power saving mode value
  *         Following values are available :
  *         @arg @ref USBPD_PWR_MODE_OFF
  *         @arg @ref USBPD_PWR_MODE_HIBERNATE
  *         @arg @ref USBPD_PWR_MODE_LOWPOWER
  *         @arg @ref USBPD_PWR_MODE_NORMAL
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_GetPowerMode(uint32_t PortNum, USBPD_PWR_PowerModeTypeDef *PwrMode)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_GetPowerMode --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    /* Get port low power operating mode */
    *PwrMode = USBPD_PWR_Port_Status[PortNum].PwrSavingMode;
  }

  return ret;
}

/**
  * @brief  Initialize the hardware resources used by the Type-C power delivery (PD)
  *         controller.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSInit(uint32_t PortNum)
{
  int32_t ret;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VBUSInit --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    /* Initialize required HW for VBUS management */
    switch (USBPD_PWR_Port_Configs[PortNum].Type)
    {
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP02:
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP03:
        /* Switch to Normal mode */
        ret = BSP_USBPD_PWR_SetPowerMode(PortNum, USBPD_PWR_MODE_NORMAL);

        MX_ADC_Init();

        /*  Start Conversion */
        LL_ADC_REG_StartConversion(USBPD_PWR_VSENSE_ADC_INSTANCE);
        break;

      case USBPD_PWR_HW_CONFIG_TYPE_DEFAULT:
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP01:
      default:
        ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
        break;
    }
  }
  return ret;
}

/**
  * @brief  Release the hardware resources used by the Type-C power delivery (PD)
  *         controller.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSDeInit(uint32_t PortNum)
{
  int32_t ret;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VBUSDeInit --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    /* De-Initialize required HW for VBUS management */
    switch (USBPD_PWR_Port_Configs[PortNum].Type)
    {
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP02:
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP03:
        /* Restore default gates configuration for Low power mode */
        /* => Close Gate Driver Consumer */
        if (TCPP0203_SetGateDriverConsumer(&USBPD_PWR_PortCompObj[PortNum],
                                           TCPP0203_GD_CONSUMER_SWITCH_CLOSED) != TCPP0203_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
          BSP_USBPD_PWR_TRACE(PortNum, "-- GDP/GDC setting : default --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

          if (USBPD_PWR_Port_Status[PortNum].PwrRole == POWER_ROLE_SOURCE)
          {
            /* Switch to Low Power mode */
            ret = BSP_USBPD_PWR_SetPowerMode(PortNum, USBPD_PWR_MODE_LOWPOWER);
          }
          else
          {
            /* Switch to Hibernate mode */
            ret = BSP_USBPD_PWR_SetPowerMode(PortNum, USBPD_PWR_MODE_HIBERNATE);
          }
        }
        break;

      case USBPD_PWR_HW_CONFIG_TYPE_DEFAULT:
      case USBPD_PWR_HW_CONFIG_TYPE_TCPP01:
      default:
        ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
        break;
    }
  }
  return ret;
}

/**
  * @brief  Enable power supply over VBUS.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSOn(uint32_t PortNum)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VBUSOn --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
    /* Port Role is now SRC : Close Gate Driver Provider */
    if (USBPD_PWR_Port_Status[PortNum].PwrRole == POWER_ROLE_SOURCE)
    {
      if (TCPP0203_SetGateDriverProvider(&USBPD_PWR_PortCompObj[PortNum],
                                         TCPP0203_GD_PROVIDER_SWITCH_CLOSED) != TCPP0203_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
#if (USE_BSP_USBPD_PWR_TRACE == 1)
      BSP_USBPD_PWR_TRACE(PortNum, "-- GDP/GDC setting : SRC --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
    }
    else
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Disable power supply over VBUS.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSOff(uint32_t PortNum)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t vbus;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Open Gate driver provider */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VBUSOff --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
    if (TCPP0203_SetGateDriverProvider(&USBPD_PWR_PortCompObj[PortNum],
                                       TCPP0203_GD_PROVIDER_SWITCH_OPEN) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Set Discharge On */
      if (TCPP0203_SetVBusDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VBUS_DISCHARGE_ON) != TCPP0203_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Wait VBUS level becomes lower than USBPD_PWR_LOW_VBUS_THRESHOLD */
        uint8_t counter = 0;

        (void)BSP_USBPD_PWR_VBUSGetVoltage(PortNum, &vbus);

        while ((vbus >= (USBPD_PWR_LOW_VBUS_THRESHOLD - 100U)) && (counter < 20U))
        {
          counter++;
          HAL_Delay(20);
          (void)BSP_USBPD_PWR_VBUSGetVoltage(PortNum, &vbus);
        }

        HAL_Delay(10);

        /* Set Discharge Off */
        if (TCPP0203_SetVBusDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VBUS_DISCHARGE_OFF) != TCPP0203_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
      }
    }
  }
  return ret;
}

/**
  * @brief  Get actual VBUS status.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  pState VBUS status (1: On, 0: Off)
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSIsOn(uint32_t PortNum, uint8_t *pState)
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t val = 0;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (TCPP0203_GetGateDriverProviderAck(&USBPD_PWR_PortCompObj[PortNum], &val) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if (val == TCPP0203_GD_PROVIDER_SWITCH_ACK_OPEN)
      {
        *pState = 0;
      }
      else
      {
        *pState = 1;
      }
    }
  }
  return ret;
}

/**
  * @brief  Set a fixed PDO and manage the power control.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  VbusTargetInmv the vbus Target (in mV)
  * @param  OperatingCurrent the Operating Current (in mA)
  * @param  MaxOperatingCurrent the Max Operating Current (in mA)
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSSetVoltage_Fixed(uint32_t PortNum,
                                           uint32_t VbusTargetInmv,
                                           uint32_t OperatingCurrent,
                                           uint32_t MaxOperatingCurrent)
{
  int32_t ret = BSP_ERROR_NONE;
  UNUSED(MaxOperatingCurrent);
  UNUSED(OperatingCurrent);
  UNUSED(VbusTargetInmv);

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  return ret;
}

/**
  * @brief  Set a variable PDO and manage the power control.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  VbusTargetMinInmv the vbus Target min (in mV)
  * @param  VbusTargetMaxInmv the vbus Target max (in mV)
  * @param  OperatingCurrent the Operating Current (in mA)
  * @param  MaxOperatingCurrent the Max Operating Current (in mA)
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSSetVoltage_Variable(uint32_t PortNum,
                                              uint32_t VbusTargetMinInmv,
                                              uint32_t VbusTargetMaxInmv,
                                              uint32_t OperatingCurrent,
                                              uint32_t MaxOperatingCurrent)
{
  int32_t ret;
  UNUSED(MaxOperatingCurrent);
  UNUSED(OperatingCurrent);
  UNUSED(VbusTargetMaxInmv);
  UNUSED(VbusTargetMinInmv);

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  return ret;
}

/**
  * @brief  Set a Battery PDO and manage the power control.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  VbusTargetMin the vbus Target min (in mV)
  * @param  VbusTargetMax the vbus Target max (in mV)
  * @param  OperatingPower the Operating Power (in mW)
  * @param  MaxOperatingPower the Max Operating Power (in mW)
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSSetVoltage_Battery(uint32_t PortNum,
                                             uint32_t VbusTargetMin,
                                             uint32_t VbusTargetMax,
                                             uint32_t OperatingPower,
                                             uint32_t MaxOperatingPower)
{
  int32_t ret;
  UNUSED(OperatingPower);
  UNUSED(VbusTargetMax);
  UNUSED(VbusTargetMin);
  UNUSED(MaxOperatingPower);

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Set the power, the precision must be at 5% */
    /* Set the current limitation */
    /* not implemented */

    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  return ret;
}

/**
  * @brief  Set a APDO and manage the power control.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  VbusTargetInmv the vbus Target (in mV)
  * @param  OperatingCurrent the Operating current (in mA)
  * @param  Delta Delta between with previous APDO (in mV), 0 means APDO start
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSSetVoltage_APDO(uint32_t PortNum,
                                          uint32_t VbusTargetInmv,
                                          uint32_t OperatingCurrent,
                                          int32_t Delta)
{
  int32_t ret;
  UNUSED(Delta);
  UNUSED(OperatingCurrent);
  UNUSED(VbusTargetInmv);

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  return ret;
}

/**
  * @brief  Set the VBUS disconnection voltage threshold.
  * @note   Callback function invoked when VBUS falls below programmed threshold.
  * @note   By default VBUS disconnection threshold is set to 3.3V
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  VoltageThreshold VBUS disconnection voltage threshold (in mV)
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_SetVBUSDisconnectionThreshold(uint32_t PortNum,
                                                    uint32_t VoltageThreshold)
{
  UNUSED(VoltageThreshold);
  int32_t ret;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  return ret;
}

/**
  * @brief  Register USB Type-C Current callback function.
  * @note   Callback function invoked when VBUS rises above 4V (VBUS present) or
  *         when VBUS falls below programmed threshold (VBUS absent).
  * @note   Callback function is un-registered when callback function pointer
  *         argument is NULL.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  pfnVBUSDetectCallback callback function pointer
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_RegisterVBUSDetectCallback(uint32_t PortNum,
                                                 USBPD_PWR_VBUSDetectCallbackFunc *pfnVBUSDetectCallback)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if ((PortNum >= USBPD_PWR_INSTANCES_NBR) || (NULL == pfnVBUSDetectCallback))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Set port callback for VBUS detection event */
    USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback = pfnVBUSDetectCallback;
  }
  return ret;
}

/**
  * @brief  Get actual voltage level measured on the VBUS line.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  pVoltage Pointer on measured voltage level (in mV)
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSGetVoltage(uint32_t PortNum, uint32_t *pVoltage)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if ((PortNum >= USBPD_PWR_INSTANCES_NBR) || (NULL == pVoltage))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    uint32_t voltage;
    static __IO uint16_t adc_value;
    adc_value = (uint16_t) LL_ADC_REG_ReadConversionData12(USBPD_PWR_VSENSE_ADC_INSTANCE);
    voltage = PWR_TCPP0203_ConvertADCDataToVoltage(adc_value, USBPD_PWR_VSENSE_RA, USBPD_PWR_VSENSE_RB);

    *pVoltage = voltage;
  }
  return ret;
}

/**
  * @brief  Get actual current level measured on the VBUS line.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  pCurrent Pointer on measured current level (in mA)
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSGetCurrent(uint32_t PortNum, int32_t *pCurrent)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if ((PortNum >= USBPD_PWR_INSTANCES_NBR) || (NULL == pCurrent))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    int32_t current;
    uint16_t adc_value;
    adc_value = 0U;   /* Not yet implemented */

    current = PWR_TCPP0203_ConvertADCDataToCurrent(adc_value);

    *pCurrent = current;
  }
  return ret;
}

/**
  * @brief  Activate discharge on VBUS.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSDischargeOn(uint32_t PortNum)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VBUSDischargeOn --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    if (TCPP0203_SetGateDriverProvider(&USBPD_PWR_PortCompObj[PortNum],
                                       TCPP0203_GD_PROVIDER_SWITCH_OPEN) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if (TCPP0203_SetVBusDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VBUS_DISCHARGE_ON) != TCPP0203_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }
  return ret;
}

/**
  * @brief  Deactivate discharge on VBUS.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VBUSDischargeOff(uint32_t PortNum)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VBUSDischargeOff --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    if (TCPP0203_SetVBusDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VBUS_DISCHARGE_OFF) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Initialize VCONN sourcing.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  CCPinId Type-C CC pin identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_CC1
  *         @arg @ref USBPD_PWR_TYPE_C_CC2
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VCONNInit(uint32_t PortNum,
                                uint32_t CCPinId)
{
  UNUSED(CCPinId);
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* No specific treatment required for initialising VCONN management from TCPP03 */
    /* Initialise VCONN management from TCPP03 :
        Ensure VCONN switch to Open/Open
        Ensure VCONN Discharge OFF
    */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VCONNInit --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    if (TCPP0203_SetVConnSwitch(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_SWITCH_OPEN) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    if (TCPP0203_SetVConnDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_DISCHARGE_OFF) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Un-Initialize VCONN sourcing.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  CCPinId Type-C CC pin identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_CC1
  *         @arg @ref USBPD_PWR_TYPE_C_CC2
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VCONNDeInit(uint32_t PortNum,
                                  uint32_t CCPinId)
{
  UNUSED(CCPinId);
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* De-initialise VCONN management from TCPP03 :
        Force VCONN switch to Open/Open
        Force VCONN Discharge OFF
    */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VCONNDeInit --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    if (TCPP0203_SetVConnSwitch(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_SWITCH_OPEN) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if (TCPP0203_SetVConnDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_DISCHARGE_OFF) != TCPP0203_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }
  return ret;
}

/**
  * @brief  Enable VCONN sourcing.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  CCPinId Type-C CC pin identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_CC1
  *         @arg @ref USBPD_PWR_TYPE_C_CC2
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VCONNOn(uint32_t PortNum,
                              uint32_t CCPinId)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if parameters are valid */
  if ((PortNum >= USBPD_PWR_INSTANCES_NBR)
      || ((CCPinId != USBPD_PWR_TYPE_C_CC1) && (CCPinId != USBPD_PWR_TYPE_C_CC2))
     )
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Set TCPP0203 VCONN Switch according to CC pin used for VCONN */
    if (CCPinId == USBPD_PWR_TYPE_C_CC1)
    {
      if (TCPP0203_SetVConnSwitch(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_SWITCH_CC1) != TCPP0203_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
    else
    {
      if (TCPP0203_SetVConnSwitch(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_SWITCH_CC2) != TCPP0203_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }
  return ret;
}

/**
  * @brief  Disable VCONN sourcing.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  CCPinId CC pin identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_CC1
  *         @arg @ref USBPD_PWR_TYPE_C_CC2
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VCONNOff(uint32_t PortNum,
                               uint32_t CCPinId)
{
  UNUSED(CCPinId);
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Switch off VCONN : restore VCONN switch to Open/open position */
    if (TCPP0203_SetVConnSwitch(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_SWITCH_OPEN) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Discharge VCONN path */
      if (TCPP0203_SetVConnDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_DISCHARGE_ON) != TCPP0203_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Wait DISCHARGE is completed then disable VCONN Discharge */
        HAL_Delay(10U);
        if (TCPP0203_SetVConnDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_DISCHARGE_OFF) != TCPP0203_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
      }
    }
  }
  return ret;
}

/**
  * @brief  Get actual VCONN status.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @param  CCPinId Type-C CC pin identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_CC1
  *         @arg @ref USBPD_PWR_TYPE_C_CC2
  * @param  pState VCONN status (1: On, 0: Off)
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VCONNIsOn(uint32_t PortNum,
                                uint32_t CCPinId, uint8_t *pState)
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t vconnSwitch;

  *pState = 0U;

  /* Check if parameters are valid */
  if ((PortNum >= USBPD_PWR_INSTANCES_NBR) || (NULL == pState)
      || ((CCPinId != USBPD_PWR_TYPE_C_CC1) && (CCPinId != USBPD_PWR_TYPE_C_CC2))
     )
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Get TCPP0203 VCONN Switch position */
    if (TCPP0203_GetVConnSwitchAck(&USBPD_PWR_PortCompObj[PortNum], &vconnSwitch) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if (((CCPinId == USBPD_PWR_TYPE_C_CC1) && (vconnSwitch == TCPP0203_VCONN_SWITCH_CC1))
          || ((CCPinId == USBPD_PWR_TYPE_C_CC2) && (vconnSwitch == TCPP0203_VCONN_SWITCH_CC2))
         )
      {
        *pState = 1U;
      }
    }
  }
  return ret;
}

/**
  * @brief  Activate discharge on VCONN.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VCONNDischargeOn(uint32_t PortNum)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VCONNDischargeOn --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    if (TCPP0203_SetVConnSwitch(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_SWITCH_OPEN) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if (TCPP0203_SetVConnDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_DISCHARGE_ON) != TCPP0203_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }
  return ret;
}

/**
  * @brief  Deactivate discharge on VCONN.
  * @param  PortNum Type-C port identifier
  *         This parameter can take one of the following values:
  *         @arg @ref USBPD_PWR_TYPE_C_PORT_1
  * @retval BSP status
  */
int32_t BSP_USBPD_PWR_VCONNDischargeOff(uint32_t PortNum)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if instance is valid */
  if (PortNum >= USBPD_PWR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
    BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_VCONNDischargeOff --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

    if (TCPP0203_SetVConnDischarge(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_VCONN_DISCHARGE_OFF) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  USBPD PWR callback used to notify a asynchronous PWR event.
  *         (This callback caould be called fromp an IT routine, associated to
  *         any PWR related event detection mechanism)
  * @param  PortNum Type-C port identifier
  * @retval None
  */
void BSP_USBPD_PWR_EventCallback(uint32_t PortNum)
{
#if (USE_BSP_USBPD_PWR_TRACE == 1)
  BSP_USBPD_PWR_TRACE(PortNum, "-- BSP_USBPD_PWR_EventCallback --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

  /* Handle USBPD PWR event */
  switch (USBPD_PWR_Port_Configs[PortNum].Type)
  {
    case USBPD_PWR_HW_CONFIG_TYPE_TCPP02:
    case USBPD_PWR_HW_CONFIG_TYPE_TCPP03:
      /* In case of TCPP0203, this callback could be called when an event is detected
         on FLGn line */
      PWR_TCPP0203_EventCallback(PortNum);
      break;

    case USBPD_PWR_HW_CONFIG_TYPE_DEFAULT:
    case USBPD_PWR_HW_CONFIG_TYPE_TCPP01:
    default:
      break;
  }
}

/**
  * @}
  */

/** @addtogroup STM32H573I_DK_USBPD_PWR_Private_Functions
  * @{
  */


/**
  * @brief  Configure TCPP0203 used GPIO.
  * @note   GPIO used for TCPP0203 operation includes VBUS measurement, ENABLE pin driving
  * @param  PortNum   Port number
  * @retval None
  */
static void PWR_TCPP0203_GPIOConfigInit(uint32_t PortNum)
{
  UNUSED(PortNum);

  /* Configure ENABLE GPIO */
  TCPP0203_PORT0_ENABLE_GPIO_CLK_ENABLE();
  /* Configure IO in output push-pull mode to drive ENABLE */
  LL_GPIO_SetPinMode(TCPP0203_PORT0_ENABLE_GPIO_PORT, TCPP0203_PORT0_ENABLE_GPIO_PIN, TCPP0203_PORT0_ENABLE_GPIO_MODE);
  LL_GPIO_SetPinOutputType(TCPP0203_PORT0_ENABLE_GPIO_PORT, TCPP0203_PORT0_ENABLE_GPIO_PIN,
                           TCPP0203_PORT0_ENABLE_GPIO_OUTPUT);
  LL_GPIO_SetPinSpeed(TCPP0203_PORT0_ENABLE_GPIO_PORT, TCPP0203_PORT0_ENABLE_GPIO_PIN, LL_GPIO_SPEED_FREQ_LOW);
  LL_GPIO_SetPinPull(TCPP0203_PORT0_ENABLE_GPIO_PORT, TCPP0203_PORT0_ENABLE_GPIO_PIN, TCPP0203_PORT0_ENABLE_GPIO_PUPD);

  /* Set ENABLE to default state */
  TCPP0203_PORT0_ENABLE_GPIO_DEFVALUE();

  /* Configure IANA GPIO */
  TCPP0203_PORT0_IANA_GPIO_CLK_ENABLE();
  /* Configure GPIO in Analog mode */
  LL_GPIO_SetPinMode(TCPP0203_PORT0_IANA_GPIO_PORT, TCPP0203_PORT0_IANA_GPIO_PIN, TCPP0203_PORT0_IANA_GPIO_MODE);

  /* Configure VBUS Connector sensing GPIO */
  TCPP0203_PORT0_VBUSC_GPIO_CLK_ENABLE();
  /* Configure GPIO in Analog mode */
  LL_GPIO_SetPinMode(TCPP0203_PORT0_VBUSC_GPIO_PORT, TCPP0203_PORT0_VBUSC_GPIO_PIN, TCPP0203_PORT0_VBUSC_GPIO_MODE);
}

static void MX_ADC_Init(void)
{
  static ADC_HandleTypeDef hadc;

  /* USER CODE BEGIN ADC_Init 0 */
  USBPD_PWR_VSENSE_ADC_ENABLE_CLOCK();

  /* Configure the GPIO as analog */
  USBPD_PWR_VSENSE_GPIO_ENABLE_CLOCK();
  LL_GPIO_SetPinMode(USBPD_PWR_VSENSE_GPIO_PORT, USBPD_PWR_VSENSE_GPIO_PIN, LL_GPIO_MODE_ANALOG);

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /* Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) */
  hadc.Instance = USBPD_PWR_VSENSE_ADC_INSTANCE;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {

  }
  /* Configure Regular Channel */
  sConfig.Channel = USBPD_PWR_VSENSE_ADC_CHANNEL;
  sConfig.Rank = USBPD_PWR_VSENSE_ADC_RANK;
  sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
  sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
  sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */

  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {

  }
  /* USER CODE BEGIN ADC_Init 2 */
  if (HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED) != HAL_OK)
  {

  }
  if (HAL_ADC_Start(&hadc) != HAL_OK)
  {

  }

  /* USER CODE END ADC_Init 2 */
}


/**
  * @brief  Configure TCPP0203 low level interrupt.
  * @note   Corresponds to EXTI mapped onto FLGn pin of TCPP0203
  * @param  PortNum   Port number
  * @retval None
  */
static void PWR_TCPP0203_ITConfigInit(uint32_t PortNum)
{
  if (USBPD_PWR_Port_Status[PortNum].IsItEnabled == 0U)
  {
    USBPD_PWR_Port_Status[PortNum].IsItEnabled = 1U;

    /* Enable the GPIO EXTI clock */
    TCPP0203_PORT0_FLG_GPIO_CLK_ENABLE();

    /* Enable a falling trigger EXTI line x Interrupt */
    TCPP0203_PORT0_FLG_EXTI_ENABLE();
    TCPP0203_PORT0_FLG_TRIG_ENABLE();

    LL_GPIO_SetPinMode(TCPP0203_PORT0_FLG_GPIO_PORT, TCPP0203_PORT0_FLG_GPIO_PIN, TCPP0203_PORT0_FLG_GPIO_MODE);
    LL_GPIO_SetPinPull(TCPP0203_PORT0_FLG_GPIO_PORT, TCPP0203_PORT0_FLG_GPIO_PIN, TCPP0203_PORT0_FLG_GPIO_PUPD);

    /* Connect External Line to the GPIO*/
    TCPP0203_PORT0_FLG_SET_EXTI();

    /* Configure NVIC for EXTIx_IRQn */
    NVIC_SetPriority(TCPP0203_PORT0_FLG_EXTI_IRQN, TCPP0203_PORT0_FLG_IT_PRIORITY);
    NVIC_EnableIRQ(TCPP0203_PORT0_FLG_EXTI_IRQN);
  }
}

/**
  * @brief  I2C BUS registration for TCPP0203 communication
  * @param  PortNum   Port number
  * @param  Address   I2C Address
  * @retval BSP status
  */
static int32_t PWR_TCPP0203_BUSConfigInit(uint32_t PortNum, uint16_t Address)
{
  int32_t                  ret = BSP_ERROR_NONE;
  TCPP0203_IO_t            TCPP0203IOCtx;
  uint8_t                  tcpp_type;

  /* Configure the TCPP0203 I2C driver */
  TCPP0203IOCtx.Address     = Address;
  TCPP0203IOCtx.Init        = BSP_I2C4_Init;
  TCPP0203IOCtx.DeInit      = BSP_I2C4_DeInit;
  TCPP0203IOCtx.ReadReg     = BSP_I2C4_ReadReg;
  TCPP0203IOCtx.WriteReg    = BSP_I2C4_WriteReg;
  TCPP0203IOCtx.GetTick     = BSP_GetTick;

  /* Register the component on BUS IO */
  if (TCPP0203_RegisterBusIO(&USBPD_PWR_PortCompObj[PortNum], &TCPP0203IOCtx) != TCPP0203_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else
  {
    /* Initialisation step */
    if (TCPP0203_Init(&USBPD_PWR_PortCompObj[PortNum]) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Read the component type */
    if (TCPP0203_ReadTCPPType(&USBPD_PWR_PortCompObj[PortNum], &tcpp_type) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    /* Check the component type */
    else if ((tcpp_type != TCPP0203_DEVICE_TYPE_02)
             && (tcpp_type != TCPP0203_DEVICE_TYPE_03))
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    /* Check the component type is same as declared in config */
    else if (((USBPD_PWR_HW_CONFIG_TYPE_TCPP02 == USBPD_PWR_Port_Configs[PortNum].Type)
              && (tcpp_type != TCPP0203_DEVICE_TYPE_02))
             || ((USBPD_PWR_HW_CONFIG_TYPE_TCPP03 == USBPD_PWR_Port_Configs[PortNum].Type)
                 && (tcpp_type != TCPP0203_DEVICE_TYPE_03)))
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    /* Reset TCPP0203 Reg0 register */
    else if (TCPP0203_Reset(&USBPD_PWR_PortCompObj[PortNum]) != TCPP0203_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Registration completed */
    }
  }
  return ret;
}

/**
  * @brief  De-Initialize TCPP0203 HW resources associated to Type-C port
  * @param  PortNum   Port number
  * @retval BSP Status
  */
static int32_t PWR_TCPP0203_ConfigDeInit(uint32_t PortNum)
{
  UNUSED(PortNum);
  return (BSP_ERROR_NONE);
}

/**
  * @brief  Handle Event callback
  * @param  PortNum   Port number
  * @retval None
  */
static void PWR_TCPP0203_EventCallback(uint32_t PortNum)
{
  uint8_t flg_reg;

  /* In case of TCPP0203, this callback could be called when an event is detected
     on FLGn line */
  if (USBPD_PWR_Port_Status[PortNum].IsInitialized == 1U)
  {
    /* Read Flags register (FLGn) */
    if (TCPP0203_ReadFlagRegister(&USBPD_PWR_PortCompObj[PortNum], &flg_reg) == TCPP0203_OK)
    {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
#ifdef _TRACE
      char _str[13];
      (void)sprintf(_str, "Reg2_0x%02x", flg_reg);
      BSP_USBPD_PWR_TRACE(PortNum, _str);
#endif /* _TRACE */
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

      /* If FLGn has been set to 0 in LOW POWER or HIBERNATE mode,
         it is expected that a VBUS_OK condition is detected.
         In this case, if any, vBus Detection callback could be called */
      switch (USBPD_PWR_Port_Status[PortNum].PwrSavingMode)
      {
        case USBPD_PWR_MODE_HIBERNATE:
        {
          if ((flg_reg & TCPP0203_FLAG_VBUS_OK_SET) == TCPP0203_FLAG_VBUS_OK_SET)
          {
            if (USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback != NULL)
            {
              /* In case of TCPP03 in Hibernate Mode (SNK config),
                 Mode is set to Low power to enable TCPP03 behavior on CC lines */
              /* Switch to Low Power mode */
              (void)BSP_USBPD_PWR_SetPowerMode(PortNum, USBPD_PWR_MODE_LOWPOWER);

              USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback(PortNum, VBUS_CONNECTED);
            }
          }
          else
          {
            /* In all other cases (NORMAL mode), FLGn signals fault detection events. */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
            BSP_USBPD_PWR_TRACE(PortNum, "-- Wrong detection --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
          }
          break;
        }

        case USBPD_PWR_MODE_LOWPOWER:
        {
          if ((flg_reg & TCPP0203_FLAG_VBUS_OK_SET) == TCPP0203_FLAG_VBUS_OK_SET)
          {
            if (USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback != NULL)
            {
              USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback(PortNum, VBUS_CONNECTED);
            }
          }
          else
          {
            /* In all other cases (NORMAL mode), FLGn signals fault detection events. */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
            BSP_USBPD_PWR_TRACE(PortNum, "-- Wrong detection --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
          }
          break;
        }

        case USBPD_PWR_MODE_NORMAL:
        {
          uint32_t tickfault = HAL_GetTick();
          uint8_t recoveryword;

          /* In all other cases (NORMAL mode), FLGn signals fault detection events. */
          /* Check type of error raised in FLGn */
          if ((flg_reg & TCPP0203_FLAG_OCP_VCONN_SET) == TCPP0203_FLAG_OCP_VCONN_SET)
          {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
            BSP_USBPD_PWR_TRACE(PortNum, "-- OCP VCONN detected --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
          }
          if ((flg_reg & TCPP0203_FLAG_OCP_VBUS_SET) == TCPP0203_FLAG_OCP_VBUS_SET)
          {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
            BSP_USBPD_PWR_TRACE(PortNum, "-- OCP VBUS detected --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */

            /* Check if detected fault could be recovered */
            /* In case of OCP fault, allow to restart and check if a 2nd OCP fault occurs in a limited time */
            if ((tickfault > USBPD_PWR_Port_Status[PortNum].LastFaultTick)
                && ((tickfault - USBPD_PWR_Port_Status[PortNum].LastFaultTick) > USBPD_PWR_FAULT_MIN_TIME_RECOVERY))
            {
              /* Send Recovery word to TCPP0203 :
                 GDC and GDP open (TCPP0203_GD_PROVIDER_SWITCH_OPEN is 0) */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
              BSP_USBPD_PWR_TRACE(PortNum, "-- Send Recovery Word --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
              recoveryword = TCPP0203_GD_CONSUMER_SWITCH_OPEN | TCPP0203_POWER_MODE_NORMAL;
              (void)TCPP0203_WriteCtrlRegister(&USBPD_PWR_PortCompObj[PortNum], &recoveryword);

              /* In case PWR Role is SRC, try to restore VBUS as soon as possible */
              if (USBPD_PWR_Port_Status[PortNum].PwrRole == POWER_ROLE_SOURCE)
              {
                (void)TCPP0203_SetPowerMode(&USBPD_PWR_PortCompObj[PortNum], TCPP0203_POWER_MODE_NORMAL);
                (void)TCPP0203_SetGateDriverProvider(&USBPD_PWR_PortCompObj[PortNum],
                                                     TCPP0203_GD_PROVIDER_SWITCH_CLOSED);
#if (USE_BSP_USBPD_PWR_TRACE == 1)
                BSP_USBPD_PWR_TRACE(PortNum, "-- GDP/GDC setting : SRC (Restored) --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
              }
              else
              {
              if (USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback != NULL)
              {
                /* Notify Error thanks to callback */
                USBPD_PWR_Port_Status[PortNum].VBUSDetectCallback(PortNum, VBUS_NOT_CONNECTED);
              }
            }
          }
            USBPD_PWR_Port_Status[PortNum].LastFaultTick = tickfault;
          }
          if ((flg_reg & TCPP0203_FLAG_OVP_VBUS_SET) == TCPP0203_FLAG_OVP_VBUS_SET)
          {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
            BSP_USBPD_PWR_TRACE(PortNum, "-- OVP VBUS detected --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
          }
          if ((flg_reg & TCPP0203_FLAG_OTP_SET) == TCPP0203_FLAG_OTP_SET)
          {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
            BSP_USBPD_PWR_TRACE(PortNum, "-- Over Temp detected --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
          }
          if ((flg_reg & TCPP0203_FLAG_OVP_CC_SET) == TCPP0203_FLAG_OVP_CC_SET)
          {
#if (USE_BSP_USBPD_PWR_TRACE == 1)
            BSP_USBPD_PWR_TRACE(PortNum, "-- OVP CC detected --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
          }
          break;
        }

        default:
        {
          /* Unexpected port state */
#if (USE_BSP_USBPD_PWR_TRACE == 1)
          BSP_USBPD_PWR_TRACE(PortNum, "-- Unexpected State for FLGn event --");
#endif /* (USE_BSP_USBPD_PWR_TRACE == 1) */
          break;
        }
      }
    }
  }
}

/**
  * @brief  Calculate the VBUS voltage level corresponding to ADC raw converted data.
  * @note   Voltage level is measured though a voltage divider
  *  Example :
  *  Voltage -- Ra ----.-   ^
  *                    |    |
  *                    Rb   | vadc = VBUS * Rb /(Ra + Rb)
  *                    |    |
  *                   GND   |
  * vadc = raw_data * (ADC_FULL_SCALE / VDD)
  * @param  ADCData  ADC raw converted data (resolution 12 bits)
  * @param  Ra       value of Ra resistor
  * @param  Rb       value of Rb resistor
  * @retval analog voltage (unit: mV)
  */
static uint32_t PWR_TCPP0203_ConvertADCDataToVoltage(uint32_t ADCData, uint32_t Ra, uint32_t Rb)
{
  uint32_t voltage;
  uint32_t vadc;

  vadc = (ADCData * VDD_VALUE) / ADC_FULL_SCALE;

  voltage = vadc * (Ra + Rb) / Rb;

  return voltage;
}

/**
  * @brief  Calculate the VBUS current level corresponding to ADC raw converted data.
  * @note   ADC measurement provides measurement on IANA pin.
  * @param  ADCData  ADC raw converted data (resolution 12 bits)
  * @retval VBUS analog current (unit: mA)
  */
static int32_t PWR_TCPP0203_ConvertADCDataToCurrent(uint32_t ADCData)
{
  int32_t current;

  current = (int32_t)(ADCData); /* Not yet implemented */

  return current;
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

/**
  * @}
  */

/**
  * @}
  */
