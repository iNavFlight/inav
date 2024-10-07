/**
  ******************************************************************************
  * @file    net_conf.h
  * @author  MCD Application Team
  * @brief   This file provides the configuration for net
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef NET_CONF_H
#define NET_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#define MDM_SIM_SELECT_0_Pin GPIO_PIN_2
#define MDM_SIM_SELECT_0_GPIO_Port GPIOC

#define MDM_SIM_SELECT_1_Pin GPIO_PIN_3
#define MDM_SIM_SELECT_1_GPIO_Port GPIOI

#define MDM_SIM_CLK_Pin GPIO_PIN_4
#define MDM_SIM_CLK_GPIO_Port GPIOA

#define MDM_SIM_DATA_Pin GPIO_PIN_12
#define MDM_SIM_DATA_GPIO_Port GPIOB

#define MDM_SIM_RST_Pin GPIO_PIN_7
#define MDM_SIM_RST_GPIO_Port GPIOC

#define MDM_PWR_EN_Pin GPIO_PIN_3
#define MDM_PWR_EN_GPIO_Port GPIOD

#define MDM_DTR_Pin GPIO_PIN_0
#define MDM_DTR_GPIO_Port GPIOA

#define MDM_RST_Pin GPIO_PIN_2
#define MDM_RST_GPIO_Port GPIOB

#define USART1_TX_Pin GPIO_PIN_6
#define USART1_TX_GPIO_Port GPIOB

#define UART1_RX_Pin GPIO_PIN_10
#define UART1_RX_GPIO_Port GPIOG

#define UART1_CTS_Pin GPIO_PIN_11
#define UART1_CTS_GPIO_Port GPIOG

#define UART1_RTS_Pin GPIO_PIN_12
#define UART1_RTS_GPIO_Port GPIOG


#define NET_USE_RTOS

#ifndef GENERATOR_WAKAAMACLIENT_CLOUD
#define  NET_MBEDTLS_HOST_SUPPORT
#endif /* GENERATOR_WAKAAMACLIENT_CLOUD */

#include "net_conf_template.h"


#ifdef __cplusplus
}
#endif

#endif /* NET_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
