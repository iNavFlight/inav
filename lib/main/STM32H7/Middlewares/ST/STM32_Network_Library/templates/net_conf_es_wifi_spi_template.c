/**
  ******************************************************************************
  * @file    es_wifi_io.c
  * @author  MCD Application Team
  * @brief   This file implements the IO operations to deal with the es-wifi
  *          module. It mainly Inits and Deinits the SPI interface. Send and
  *          receive data over it.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#if defined(TARGET_STM32F413H_DISCOVERY)
#include "stm32f4xx_hal.h"
#define DATA_READY_IRQ  EXTI15_10_IRQn
#endif /* TARGET_STM32F413H_DISCOVERY */

#if defined(TARGET_B_L475E_IOT01)
#include "stm32l4xx_hal.h"
#define DATA_READY_IRQ  EXTI1_IRQn
#endif /* TARGET_B_L475E_IOT01 */

#if defined(TARGET_STM32H7B3I_DISCOVERY)
#include "stm32h7xx_hal.h"
#define DATA_READY_IRQ  EXTI9_5_IRQn
#endif /* TARGET_STM32H7B3I_DISCOVERY */

#if defined(TARGET_STM32F413H_DISCOVERY) || defined(TARGET_B_L475E_IOT01)
#include <core_cm4.h>
#endif /* TARGET_STM32F413H_DISCOVERY || TARGET_B_L475E_IOT01 */
#if defined(TARGET_STM32H7B3I_DISCOVERY)
#include <core_cm7.h>
#endif /* TARGET_STM32H7B3I_DISCOVERY */
#include <string.h>
#include "es_wifi.h"
#include "es_wifi_conf.h"
#include "net_conf.h"
#ifdef GENERATOR_AWS_CLOUD
#include "mbedtls/x509_crt.h"
#endif /* GENERATOR_AWS_CLOUD */

/* Global variables  --------------------------------------------------------*/
SPI_HandleTypeDef hspi;

/* Function  definitions  --------------------------------------------------------*/
static void SPI_WIFI_MspInit(SPI_HandleTypeDef *hspi);

/* Private define ------------------------------------------------------------*/
#if defined(TARGET_STM32F413H_DISCOVERY)
#define WIFI_RESET_MODULE()                do{\
                                               HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_RESET);\
                                               HAL_Delay(10);\
                                               HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_SET);\
                                               HAL_Delay(500);\
                                             }while(0);


#define WIFI_ENABLE_NSS()                  do{ \
                                               HAL_GPIO_WritePin( GPIOG, GPIO_PIN_11, GPIO_PIN_RESET );\
                                             }while(0);

#define WIFI_DISABLE_NSS()                 do{ \
                                               HAL_GPIO_WritePin( GPIOG, GPIO_PIN_11, GPIO_PIN_SET );\
                                             }while(0);

#define WIFI_IS_CMDDATA_READY()            (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_12) == GPIO_PIN_SET)
#endif /* TARGET_STM32F413H_DISCOVERY */

#if defined(TARGET_B_L475E_IOT01)
#define WIFI_RESET_MODULE()                do{\
                                               HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_RESET);\
                                               HAL_Delay(10);\
                                               HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET);\
                                               HAL_Delay(500);\
                                             }while(0);


#define WIFI_ENABLE_NSS()                  do{ \
                                               HAL_GPIO_WritePin( GPIOE, GPIO_PIN_0, GPIO_PIN_RESET );\
                                             }while(0);

#define WIFI_DISABLE_NSS()                 do{ \
                                               HAL_GPIO_WritePin( GPIOE, GPIO_PIN_0, GPIO_PIN_SET );\
                                             }while(0);

#define WIFI_IS_CMDDATA_READY()            (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_1) == GPIO_PIN_SET)
#endif /* TARGET_B_L475E_IOT01 */

#if defined(TARGET_STM32H7B3I_DISCOVERY)
#define WIFI_RESET_MODULE()                do{\
                                               HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);\
                                               HAL_Delay(10);\
                                               HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET);\
                                               HAL_Delay(500);\
                                             }while(0);


#define WIFI_ENABLE_NSS()                  do{ \
                                               HAL_GPIO_WritePin( GPIOA, GPIO_PIN_11, GPIO_PIN_RESET );\
                                             }while(0);

#define WIFI_DISABLE_NSS()                 do{ \
                                               HAL_GPIO_WritePin( GPIOA, GPIO_PIN_11, GPIO_PIN_SET );\
                                             }while(0);

#define WIFI_IS_CMDDATA_READY()            (HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_5) == GPIO_PIN_SET)

#define SPI_INTERFACE_PRIO                 0
#endif /* TARGET_STM32H7B3I_DISCOVERY */

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static  int32_t __IO spi_rx_event = 0;
static  int32_t __IO spi_tx_event = 0;
static  int32_t __IO cmddata_rdy_rising_event = 0;

#ifdef WIFI_USE_CMSIS_OS
osMutexId es_wifi_mutex;
osMutexDef(es_wifi_mutex);

static    osMutexId spi_mutex;
osMutexDef(spi_mutex);

static    osSemaphoreId spi_rx_sem;
osSemaphoreDef(spi_rx_sem);

static    osSemaphoreId spi_tx_sem;
osSemaphoreDef(spi_tx_sem);

static    osSemaphoreId cmddata_rdy_rising_sem;
osSemaphoreDef(cmddata_rdy_rising_sem);

#endif /* WIFI_USE_CMSIS_OS */

/* Private function prototypes -----------------------------------------------*/
static  int32_t wait_cmddata_rdy_high(int32_t timeout);
static  int32_t wait_cmddata_rdy_rising_event(int32_t timeout);
static  int32_t wait_spi_tx_event(int32_t timeout);
static  int32_t wait_spi_rx_event(int32_t timeout);
static  void SPI_WIFI_DelayUs(uint32_t);
static int8_t  SPI_WIFI_DeInit(void);
static int8_t  SPI_WIFI_Init(uint16_t mode);
static int8_t  SPI_WIFI_ResetModule(void);
static int16_t SPI_WIFI_ReceiveData(uint8_t *pData, uint16_t len, uint32_t timeout);
static int16_t SPI_WIFI_SendData(uint8_t *pData, uint16_t len, uint32_t timeout);
/* Private functions ---------------------------------------------------------*/

#ifdef GENERATOR_AWS_CLOUD
/*
 * Amazon Profile
 */
const   mbedtls_x509_crt_profile mbedtls_x509_crt_amazon_suite =
{
  /* Only SHA-256 and 384 */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) |
  MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384),
  /* Only ECDSA */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_RSA) | /* */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_ECKEY) | /* */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_ECDSA),
#if defined(MBEDTLS_ECP_C)
  /* Only NIST P-256 and P-384 */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP256R1) |
  MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP384R1),
#else
  0,
#endif /* MBEDTLS_ECP_C */
  2048
};
const int32_t net_tls_sizeof_suite_structure = sizeof(mbedtls_x509_crt_profile);
const void    *net_tls_user_suite0 = (void *) &mbedtls_x509_crt_amazon_suite;
#endif /* GENERATOR_AWS_CLOUD */


int32_t wifi_probe(void **ll_drv_context);

ES_WIFIObject_t    EsWifiObj;




/*******************************************************************************
                       COM Driver Interface (SPI)
  *******************************************************************************/
/**
  * @brief  Initialize SPI MSP
  * @param  hspi: SPI handle
  * @retval None
  */
static void SPI_WIFI_MspInit(SPI_HandleTypeDef *hspi)
{

  GPIO_InitTypeDef GPIO_Init;
#if defined(TARGET_STM32F413H_DISCOVERY)
  __HAL_RCC_SPI3_CLK_ENABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /* configure Wake up pin */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
  GPIO_Init.Pin       = GPIO_PIN_15;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_Init);

  /* configure Data ready pin */
  GPIO_Init.Pin       = GPIO_PIN_12;
  GPIO_Init.Mode      = GPIO_MODE_IT_RISING;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_Init);

  /* configure Reset pin */
  GPIO_Init.Pin       = GPIO_PIN_1;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  GPIO_Init.Alternate = 0;
  HAL_GPIO_Init(GPIOH, &GPIO_Init);

  /* configure SPI NSS pin pin */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_11, GPIO_PIN_SET);
  GPIO_Init.Pin       =  GPIO_PIN_11;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOG, &GPIO_Init);

  /* configure SPI CLK pin */
  GPIO_Init.Pin       =  GPIO_PIN_12;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_Init.Alternate = GPIO_AF7_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_Init);

  /* configure SPI MOSI pin */
  GPIO_Init.Pin       = GPIO_PIN_5;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_Init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_Init);

  /* configure SPI MISO pin */
  GPIO_Init.Pin       = GPIO_PIN_4;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_PULLUP;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_Init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_Init);
#endif /* TARGET_STM32F413H_DISCOVERY */

#if defined(TARGET_B_L475E_IOT01)
  __HAL_RCC_SPI3_CLK_ENABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* configure Wake up pin */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
  GPIO_Init.Pin       = GPIO_PIN_13;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_Init);

  /* configure Data ready pin */
  GPIO_Init.Pin       = GPIO_PIN_1;
  GPIO_Init.Mode      = GPIO_MODE_IT_RISING;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_Init);

  /* configure Reset pin */
  GPIO_Init.Pin       = GPIO_PIN_8;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  GPIO_Init.Alternate = 0;
  HAL_GPIO_Init(GPIOE, &GPIO_Init);

  /* configure SPI NSS pin pin */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);
  GPIO_Init.Pin       =  GPIO_PIN_0;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOE, &GPIO_Init);

  /* configure SPI CLK pin */
  GPIO_Init.Pin       =  GPIO_PIN_10;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_Init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_Init);

  /* configure SPI MOSI pin */
  GPIO_Init.Pin       = GPIO_PIN_12;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_Init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_Init);

  /* configure SPI MISO pin */
  GPIO_Init.Pin       = GPIO_PIN_11;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_PULLUP;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_Init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_Init);
#endif /* TARGET_B_L475E_IOT01 */

#if defined(TARGET_STM32H7B3I_DISCOVERY)
  __HAL_RCC_SPI2_CLK_ENABLE();
  __HAL_RCC_SPI2_FORCE_RESET();
  __HAL_RCC_SPI2_RELEASE_RESET();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  /* configure Wake up pin */
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_2, GPIO_PIN_RESET);
  GPIO_Init.Pin       = GPIO_PIN_2;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOI, &GPIO_Init);

  /* configure Data ready pin */
  GPIO_Init.Pin       = GPIO_PIN_5;
  GPIO_Init.Mode      = GPIO_MODE_IT_RISING;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_Init);

  /* configure Reset pin */
  GPIO_Init.Pin       = GPIO_PIN_1;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  GPIO_Init.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOI, &GPIO_Init);

  /* configure SPI NSS pin pin */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
  GPIO_Init.Pin       =  GPIO_PIN_11;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_Init);

  /* configure SPI CLK pin */
  GPIO_Init.Pin       =  GPIO_PIN_12;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_Init.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOA, &GPIO_Init);

  /* configure SPI MOSI pin */
  GPIO_Init.Pin       = GPIO_PIN_3;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  GPIO_Init.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOC, &GPIO_Init);

  /* configure SPI MISO pin */
  GPIO_Init.Pin       = GPIO_PIN_2;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_PULLUP;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  GPIO_Init.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOC, &GPIO_Init);
#endif /* TARGET_STM32H7B3I_DISCOVERY */
}

#if defined(TARGET_STM32F413H_DISCOVERY) || defined(TARGET_B_L475E_IOT01)
/**
  * @brief  Initialize the SPI3
  * @param  None
  * @retval None
  */
#endif /* TARGET_STM32F413H_DISCOVERY || TARGET_B_L475E_IOT01 */
#if defined(TARGET_STM32H7B3I_DISCOVERY)
/**
  * @brief  Initialize the SPI2
  * @param  None
  * @retval None
  */
#endif /* TARGET_STM32H7B3I_DISCOVERY */
static int8_t SPI_WIFI_Init(uint16_t mode)
{
  int8_t  rc = 0;

  if (mode == ES_WIFI_INIT)
  {
#if defined(TARGET_STM32F413H_DISCOVERY) || defined(TARGET_B_L475E_IOT01)
    hspi.Instance               = SPI3;
#endif /* TARGET_STM32F413H_DISCOVERY || TARGET_B_L475E_IOT01 */
#if defined(TARGET_STM32H7B3I_DISCOVERY)
    hspi.Instance               = SPI2;
#endif /* TARGET_STM32H7B3I_DISCOVERY */
    SPI_WIFI_MspInit(&hspi);

    hspi.Init.Mode              = SPI_MODE_MASTER;
    hspi.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi.Init.DataSize          = SPI_DATASIZE_16BIT;
    hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi.Init.NSS               = SPI_NSS_SOFT;
    hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; /* 80/8= 10MHz (Inventek WIFI module supports up to 20MHz)*/
    hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi.Init.CRCPolynomial     = 0;

    if (HAL_SPI_Init(&hspi) != HAL_OK)
    {
      return -1;
    }

    /* Enable Interrupt for Data Ready pin , GPIO_PIN1 */
    HAL_NVIC_SetPriority((IRQn_Type)DATA_READY_IRQ, SPI_INTERFACE_PRIO, 0x00);
    HAL_NVIC_EnableIRQ((IRQn_Type)DATA_READY_IRQ);

    /* Enable Interrupt for SPI tx and rx */
#if defined(TARGET_STM32F413H_DISCOVERY) || defined(TARGET_B_L475E_IOT01)
    HAL_NVIC_SetPriority((IRQn_Type)SPI3_IRQn, SPI_INTERFACE_PRIO, 0);
    HAL_NVIC_EnableIRQ((IRQn_Type)SPI3_IRQn);
#endif /* TARGET_STM32F413H_DISCOVERY || TARGET_B_L475E_IOT01 */
#if defined(TARGET_STM32H7B3I_DISCOVERY)
    HAL_NVIC_SetPriority((IRQn_Type)SPI2_IRQn, SPI_INTERFACE_PRIO, 0);
    HAL_NVIC_EnableIRQ((IRQn_Type)SPI2_IRQn);
#endif /* TARGET_STM32H7B3I_DISCOVERY */


#ifdef WIFI_USE_CMSIS_OS
    cmddata_rdy_rising_event = 0;
    es_wifi_mutex = osMutexCreate(osMutex(es_wifi_mutex));
    spi_mutex = osMutexCreate(osMutex(spi_mutex));
    spi_rx_sem = osSemaphoreCreate(osSemaphore(spi_rx_sem), 1);
    spi_tx_sem = osSemaphoreCreate(osSemaphore(spi_tx_sem), 1);
    cmddata_rdy_rising_sem = osSemaphoreCreate(osSemaphore(cmddata_rdy_rising_sem), 1);
    /* take semaphore */
    SEM_WAIT(cmddata_rdy_rising_sem, 1);
    SEM_WAIT(spi_rx_sem, 1);
    SEM_WAIT(spi_tx_sem, 1);

#endif /* WIFI_USE_CMSIS_OS */
    /* first call used for calibration */
    SPI_WIFI_DelayUs(10);
  }

  rc = SPI_WIFI_ResetModule();

  return rc;
}


static int8_t SPI_WIFI_ResetModule(void)
{
  uint32_t tickstart = HAL_GetTick();
  uint8_t Prompt[6];
  uint8_t count = 0;
  HAL_StatusTypeDef  Status;

  WIFI_RESET_MODULE();
  WIFI_ENABLE_NSS();
  SPI_WIFI_DelayUs(15);

  while (WIFI_IS_CMDDATA_READY())
  {
    Status = HAL_SPI_Receive(&hspi, &Prompt[count], 1, 0xFFFF);
    count += 2;
    if (((HAL_GetTick() - tickstart) > 0xFFFF) || (Status != HAL_OK))
    {
      WIFI_DISABLE_NSS();
      return -1;
    }
  }

  WIFI_DISABLE_NSS();
  if ((Prompt[0] != 0x15) || (Prompt[1] != 0x15) || (Prompt[2] != '\r') ||
      (Prompt[3] != '\n') || (Prompt[4] != '>') || (Prompt[5] != ' '))
  {
    return -1;
  }
  return 0;
}

/**
  * @brief  DeInitialize the SPI
  * @param  None
  * @retval None
  */
static int8_t SPI_WIFI_DeInit(void)
{
  HAL_SPI_DeInit(&hspi);
#ifdef  WIFI_USE_CMSIS_OS
  osMutexDelete(spi_mutex);
  osMutexDelete(es_wifi_mutex);
  osSemaphoreDelete(spi_tx_sem);
  osSemaphoreDelete(spi_rx_sem);
  osSemaphoreDelete(cmddata_rdy_rising_sem);
#endif /* WIFI_USE_CMSIS_OS */
  return 0;
}



/**
  * @brief  Receive wifi Data from SPI
  * @param  pdata : pointer to data
  * @param  len : Data length
  * @param  timeout : send timeout in mS
  * @retval Length of received data (payload)
  */

int32_t wait_cmddata_rdy_high(int32_t timeout)
{
  int32_t tickstart = HAL_GetTick();
  while (WIFI_IS_CMDDATA_READY() == 0)
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      return -1;
    }
  }
  return 0;
}



int32_t wait_cmddata_rdy_rising_event(int32_t timeout)
{
#ifdef SEM_WAIT
  return SEM_WAIT(cmddata_rdy_rising_sem, timeout);
#else
  int32_t tickstart = HAL_GetTick();
  while (cmddata_rdy_rising_event == 1)
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      return -1;
    }
  }
  return 0;
#endif /* SEM_WAIT */
}

int32_t wait_spi_rx_event(int32_t timeout)
{
#ifdef SEM_WAIT
  return SEM_WAIT(spi_rx_sem, timeout);
#else
  int32_t tickstart = HAL_GetTick();
  while (spi_rx_event == 1)
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      return -1;
    }
  }
  return 0;
#endif /* SEM_WAIT */
}

int32_t wait_spi_tx_event(int32_t timeout)
{
#ifdef SEM_WAIT
  return SEM_WAIT(spi_tx_sem, timeout);
#else
  int32_t tickstart = HAL_GetTick();
  while (spi_tx_event == 1)
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      return -1;
    }
  }
  return 0;
#endif /* SEM_WAIT */
}



int16_t SPI_WIFI_ReceiveData(uint8_t *pData, uint16_t len, uint32_t timeout)
{
  int16_t length = 0;
  uint8_t tmp[2];

  WIFI_DISABLE_NSS();
  UNLOCK_SPI();
  SPI_WIFI_DelayUs(3);


  if (wait_cmddata_rdy_rising_event(timeout) < 0)
  {
    return ES_WIFI_ERROR_WAITING_DRDY_FALLING;
  }

  LOCK_SPI();
  WIFI_ENABLE_NSS();
  SPI_WIFI_DelayUs(15);
  while (WIFI_IS_CMDDATA_READY())
  {
    if ((length < len) || (!len))
    {
      spi_rx_event = 1;
      if (HAL_SPI_Receive_IT(&hspi, tmp, 1) != HAL_OK)
      {
        WIFI_DISABLE_NSS();
        UNLOCK_SPI();
        return ES_WIFI_ERROR_SPI_FAILED;
      }

      wait_spi_rx_event(timeout);

      pData[0] = tmp[0];
      pData[1] = tmp[1];
      length += 2;
      pData  += 2;

      if (length >= ES_WIFI_DATA_SIZE)
      {
        WIFI_DISABLE_NSS();
        SPI_WIFI_ResetModule();
        UNLOCK_SPI();
        return ES_WIFI_ERROR_STUFFING_FOREVER;
      }
    }
    else
    {
      break;
    }
  }
  WIFI_DISABLE_NSS();
  UNLOCK_SPI();
  return length;
}
/**
  * @brief  Send wifi Data thru SPI
  * @param  pdata : pointer to data
  * @param  len : Data length
  * @param  timeout : send timeout in mS
  * @retval Length of sent data
  */
int16_t SPI_WIFI_SendData(uint8_t *pdata,  uint16_t len, uint32_t timeout)
{
  uint8_t Padding[2];

  if (wait_cmddata_rdy_high(timeout) < 0)
  {
    return ES_WIFI_ERROR_SPI_FAILED;
  }

  /* arm to detect rising event */
  cmddata_rdy_rising_event = 1;
  LOCK_SPI();
  WIFI_ENABLE_NSS();
  SPI_WIFI_DelayUs(15);
  if (len > 1)
  {
    spi_tx_event = 1;
    if (HAL_SPI_Transmit_IT(&hspi, (uint8_t *)pdata, len / 2) != HAL_OK)
    {
      WIFI_DISABLE_NSS();
      UNLOCK_SPI();
      return ES_WIFI_ERROR_SPI_FAILED;
    }
    wait_spi_tx_event(timeout);
  }

  if (len & 1)
  {
    Padding[0] = pdata[len - 1];
    Padding[1] = '\n';

    spi_tx_event = 1;
    if (HAL_SPI_Transmit_IT(&hspi, Padding, 1) != HAL_OK)
    {
      WIFI_DISABLE_NSS();
      UNLOCK_SPI();
      return ES_WIFI_ERROR_SPI_FAILED;
    }
    wait_spi_tx_event(timeout);

  }
  return len;
}

/**
   * @brief  Delay
  * @param  Delay in us
  * @retval None
  */
void SPI_WIFI_DelayUs(uint32_t n)
{
  __IO        uint32_t ct = 0;
  uint32_t        loop_per_us = 0;
  static uint32_t cycle_per_loop = 0;

  /* calibration happen on first call for a duration of 1 ms * nbcycle per loop */
  /* 10 cycle for STM32L4 */
  if (cycle_per_loop == 0)
  {
    uint32_t cycle_per_ms = (SystemCoreClock / 1000UL);
    uint32_t tick = 0;
    ct = cycle_per_ms;
    tick = HAL_GetTick();
    while (ct)
    {
      ct--;
    }
    cycle_per_loop = HAL_GetTick() - tick;
    if (cycle_per_loop == 0)
    {
      cycle_per_loop = 1;
    }
  }

  loop_per_us = SystemCoreClock / 1000000UL / cycle_per_loop;
  ct = n * loop_per_us;
  while (ct)
  {
    ct--;
  }
  return;
}

/**
  * @brief Rx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (spi_rx_event)
  {
    SEM_SIGNAL(spi_rx_sem);
    spi_rx_event = 0;
  }
}

/**
  * @brief Tx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (spi_tx_event)
  {
    SEM_SIGNAL(spi_tx_sem);
    spi_tx_event = 0;
  }
}


/**
  * @brief  Interrupt handler for  Data RDY signal
  * @param  None
  * @retval None
  */
void    SPI_WIFI_ISR(void)
{
  if (cmddata_rdy_rising_event == 1)
  {
    SEM_SIGNAL(cmddata_rdy_rising_sem);
    cmddata_rdy_rising_event = 0;
  }
}

/**
  * @brief  probe function to register wifi to Network library framework
  * @param  None
  * @retval None
  */
int32_t wifi_probe(void **ll_drv_context)
{
  if (ES_WIFI_RegisterBusIO(&EsWifiObj,
                            SPI_WIFI_Init,
                            SPI_WIFI_DeInit,
                            HAL_Delay,
                            SPI_WIFI_SendData,
                            SPI_WIFI_ReceiveData) == 0)
  {
    *ll_drv_context = &EsWifiObj;
    return 0;
  }
  return -1;
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
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
