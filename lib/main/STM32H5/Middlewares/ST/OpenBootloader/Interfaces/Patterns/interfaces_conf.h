/**
  ******************************************************************************
  * @file    interfaces_conf.h
  * @author  MCD Application Team
  * @brief   Contains Interfaces configuration
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INTERFACES_CONF_H
#define INTERFACES_CONF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_ll_usart.h"
#include "stm32u5xx_ll_i2c.h"
#include "stm32u5xx_ll_spi.h"

#define MEMORIES_SUPPORTED                7U

/*-------------------------- Definitions for USART ---------------------------*/
#define USARTx                            USART3
#define USARTx_CLK_ENABLE()               __HAL_RCC_USART3_CLK_ENABLE()
#define USARTx_CLK_DISABLE()              __HAL_RCC_USART3_CLK_DISABLE()
#define USARTx_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_DEINIT()                   LL_USART_DeInit(USARTx)

#define USARTx_TX_PIN                     GPIO_PIN_8
#define USARTx_TX_GPIO_PORT               GPIOD
#define USARTx_RX_PIN                     GPIO_PIN_9
#define USARTx_RX_GPIO_PORT               GPIOD
#define USARTx_ALTERNATE                  GPIO_AF7_USART3

/*-------------------------- Definitions for I2C -----------------------------*/
#define I2Cx                              I2C2
#define I2Cx_CLK_ENABLE()                 __HAL_RCC_I2C2_CLK_ENABLE()
#define I2Cx_CLK_DISABLE()                __HAL_RCC_I2C2_CLK_DISABLE()
#define I2Cx_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOH_CLK_ENABLE()
#define I2Cx_DEINIT()                     LL_I2C_DeInit(I2Cx)

#define I2Cx_SCL_PIN                      GPIO_PIN_4
#define I2Cx_SCL_PIN_PORT                 GPIOH
#define I2Cx_SDA_PIN                      GPIO_PIN_5
#define I2Cx_SDA_PIN_PORT                 GPIOH
#define I2Cx_ALTERNATE                    GPIO_AF4_I2C2
#define I2C_ADDRESS                       0x000000B4U
#define OPENBL_I2C_TIMEOUT                0xFFFFF000U
#define I2C_TIMING                        0x00800000U

/*-------------------------- Definitions for FDCAN ---------------------------*/
#define FDCANx                            FDCAN1
#define FDCANx_CLK_ENABLE()               __HAL_RCC_FDCAN1_CLK_ENABLE()
#define FDCANx_CLK_DISABLE()              __HAL_RCC_FDCAN1_CLK_DISABLE()
#define FDCANx_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOB_CLK_ENABLE()

#define FDCANx_TX_PIN                     GPIO_PIN_8
#define FDCANx_TX_GPIO_PORT               GPIOB
#define FDCANx_TX_AF                      GPIO_AF9_FDCAN1
#define FDCANx_RX_PIN                     GPIO_PIN_9
#define FDCANx_RX_GPIO_PORT               GPIOB
#define FDCANx_RX_AF                      GPIO_AF9_FDCAN1

#define FDCANx_FORCE_RESET()              __HAL_RCC_FDCAN1_FORCE_RESET()
#define FDCANx_RELEASE_RESET()            __HAL_RCC_FDCAN1_RELEASE_RESET()

/*--------------------------- Definitions for SPI ----------------------------*/
#define SPIx                              SPI1
#define SPIx_CLK_ENABLE()                 __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIx_CLK_DISABLE()                __HAL_RCC_SPI1_CLK_DISABLE()
#define SPIx_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOE_CLK_ENABLE()
#define SPIx_DEINIT()                     LL_SPI_DeInit(SPIx)
#define SPIx_IRQ                          SPI1_IRQn

#define SPIx_MOSI_PIN                     GPIO_PIN_15
#define SPIx_MOSI_PIN_PORT                GPIOE
#define SPIx_MISO_PIN                     GPIO_PIN_14
#define SPIx_MISO_PIN_PORT                GPIOE
#define SPIx_SCK_PIN                      GPIO_PIN_13
#define SPIx_SCK_PIN_PORT                 GPIOE
#define SPIx_NSS_PIN                      GPIO_PIN_4
#define SPIx_NSS_PIN_PORT                 GPIOA
#define SPIx_ALTERNATE                    GPIO_AF5_SPI1

/*-------------------------- Definitions for I3C -----------------------------*/
#define I3Cx                              I3C1
#define I3Cx_CLK_ENABLE()                 __HAL_RCC_I3C1_CLK_ENABLE()
#define I3Cx_CLK_DISABLE()                __HAL_RCC_I3C1_CLK_DISABLE()
#define I3Cx_GPIO_CLK_SCL_ENABLE()        __HAL_RCC_GPIOH_CLK_ENABLE()
#define I3Cx_GPIO_CLK_SDA_ENABLE()        __HAL_RCC_GPIOH_CLK_ENABLE()
#define I3Cx_DEINIT()                     LL_I3C_DeInit(I3Cx)
#define I3Cx_EV_IRQ                       I3C1_EV_IRQn

#define I3Cx_SCL_PIN                      LL_GPIO_PIN_11
#define I3Cx_SCL_PORT                     GPIOH
#define I3Cx_SDA_PIN                      LL_GPIO_PIN_12
#define I3Cx_SDA_PORT                     GPIOH
#define I3Cx_ALTERNATE                    LL_GPIO_AF_5
#define OPENBL_I3C_TIMEOUT                0xFFFFF000U

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INTERFACES_CONF_H */
