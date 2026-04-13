/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    mx_wifi_io.c
  * @author  MCD Application Team
  * @brief   This file implements the IO operations to deal with the mx_wifi
  *          module. It mainly Inits and Deinits the SPI/UART interface. Send and
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <main.h>
#include <string.h>
#include "net_conf.h"
#include "mx_wifi.h"
#if (MX_WIFI_USE_SPI == 1)

/* Private define ------------------------------------------------------------*/
#if (osCMSIS < 0x20000U )
#define OSSEMAPHOREWAIT osSemaphoreWait
#else
#define OSSEMAPHOREWAIT osSemaphoreAcquire
#endif /* osCMSIS */

#define SPI_READ  (0xBE)
#define SPI_WRITE (0xEF)


#define MX_WIFI_RESET_MODULE()      do{\
                                            HAL_GPIO_WritePin(MX_WIFI_RESET_IO_PORT, MX_WIFI_RESET_IO_PIN, GPIO_PIN_RESET);\
                                            HAL_Delay(10);\
                                            HAL_GPIO_WritePin(MX_WIFI_RESET_IO_PORT, MX_WIFI_RESET_IO_PIN, GPIO_PIN_SET);\
                                            HAL_Delay(10);\
                                      }while(0);


#define MX_WIFI_SPI_CS_ENABLE()   do{ \
                                          HAL_GPIO_WritePin( MX_WIFI_SPI_SW_CS_PORT, MX_WIFI_SPI_SW_CS_PIN, GPIO_PIN_RESET );\
                                    }while(0);

#define MX_WIFI_SPI_CS_DISABLE()    do{ \
                                            HAL_GPIO_WritePin( MX_WIFI_SPI_SW_CS_PORT, MX_WIFI_SPI_SW_CS_PIN, GPIO_PIN_SET );\
                                      }while(0);

/* Private variables ---------------------------------------------------------*/
static MX_WIFIObject_t    MxWifiObj;
static __IO int32_t spi_slave_notify_event = 0;
static __IO int32_t spi_slave_flow_event = 0;

#if MX_WIFI_USE_CMSIS_OS
osSemaphoreId slave_notify_sem;
osSemaphoreDef(slave_notify_sem);

osSemaphoreId slave_flow_sem;
osSemaphoreDef(slave_flow_sem);
#endif /* WIFI_USE_CMSIS_OS */


/* Global variables  --------------------------------------------------------*/
SPI_HandleTypeDef hspi_mx;
DMA_HandleTypeDef hdma_spi_mxc_tx;
DMA_HandleTypeDef hdma_spi_mxc_rx;

MX_WIFIObject_t *wifi_obj_get(void);



static  void MX_WIFI_IO_DELAY(uint32_t ms)
{
#if MX_WIFI_USE_CMSIS_OS
  osDelay(ms);
#else
  HAL_Delay(ms);
#endif /* WIFI_USE_CMSIS_OS */
}

MX_WIFIObject_t *wifi_obj_get(void)
{
  return &MxWifiObj;
}

/**
  * @brief GPIO Initialization Function for WIFI reset IO
  * @param None
  * @retval None
  */
static void MX_WIFI_RESET_IO_Init(void)
{
  GPIO_InitTypeDef GPIO_Init;

  MX_WIFI_RESET_IO_CLK_ENABLE();

  /* configure Reset pin PA4 */
  HAL_GPIO_WritePin(MX_WIFI_RESET_IO_PORT, MX_WIFI_RESET_IO_PIN, GPIO_PIN_SET);
  GPIO_Init.Pin       = MX_WIFI_RESET_IO_PIN;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MX_WIFI_RESET_IO_PORT, &GPIO_Init);
}

/**
  * @brief  Initialize the flow, notify and software CS IO for SPI
  * @param  None
  * @retval None
  */
static void MX_WIFI_SPI_IO_Init(void)
{
  GPIO_InitTypeDef GPIO_Init;

  /* GPIO Ports Clock Enable */
  MX_WIFI_SPI_IO_FLOW_CLK_ENABLE();
  MX_WIFI_SPI_IO_NOTIFY_CLK_ENABLE();
  MX_WIFI_SPI_SW_CS_CLK_ENABLE();

  /* configure slave data notify pin */
  GPIO_Init.Pin       = MX_WIFI_SPI_IO_NOTIFY_PIN;
  GPIO_Init.Mode      = GPIO_MODE_IT_RISING_FALLING;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MX_WIFI_SPI_IO_NOTIFY_PORT, &GPIO_Init);

  /* configure slave flow control pin */
  GPIO_Init.Pin       = MX_WIFI_SPI_IO_FLOW_PIN;
  GPIO_Init.Mode      = GPIO_MODE_IT_RISING_FALLING;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MX_WIFI_SPI_IO_FLOW_PORT, &GPIO_Init);

  /* Enable Interrupt for slave flow control pin ,  PA3 */
  HAL_NVIC_SetPriority((IRQn_Type)MX_WIFI_SPI_IO_FLOW_IRQ, SPI_INTERFACE_IO_PRIO, 0x00);
  HAL_NVIC_EnableIRQ((IRQn_Type)MX_WIFI_SPI_IO_FLOW_IRQ);

  /* Enable Interrupt for slave Data Ready pin , PB0 */
  HAL_NVIC_SetPriority((IRQn_Type)MX_WIFI_SPI_IO_NOTIFY_IRQ, SPI_INTERFACE_IO_PRIO, 0x00);
  HAL_NVIC_EnableIRQ((IRQn_Type)MX_WIFI_SPI_IO_NOTIFY_IRQ);

  /* SPI soft-NSS */
  HAL_GPIO_WritePin(MX_WIFI_SPI_SW_CS_PORT, MX_WIFI_SPI_SW_CS_PIN, GPIO_PIN_RESET);
  GPIO_Init.Pin       = MX_WIFI_SPI_SW_CS_PIN;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MX_WIFI_SPI_SW_CS_PORT, &GPIO_Init);
}

/**
  * @brief  Initialize the SPI1 hardware
  * @param  None
  * @retval None
  */
static void MX_SPI_Init(void)
{
  /* SPI1 parameter configuration*/
  hspi_mx.Instance = SPI1;
  hspi_mx.Init.Mode = SPI_MODE_MASTER;
  hspi_mx.Init.Direction = SPI_DIRECTION_2LINES;
  hspi_mx.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi_mx.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi_mx.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi_mx.Init.NSS = SPI_NSS_SOFT;
  hspi_mx.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi_mx.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi_mx.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi_mx.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi_mx.Init.CRCPolynomial = 7;
  hspi_mx.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi_mx.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi_mx) != HAL_OK)
  {
    /*Error_Handler();*/
  }
}



/**
  * @brief  Initialize the SPI1
  * @param  None
  * @retval None
  */
static int8_t MX_WIFI_SPI_Init(uint16_t mode)
{
  int8_t  rc = 0;

  MX_WIFI_RESET_IO_Init();

  if (MX_WIFI_RESET == mode)
  {
    MX_WIFI_RESET_MODULE();
    rc = 0;
  }
  else
  {
    MX_WIFI_SPI_IO_Init();
    MX_SPI_Init();

#if MX_WIFI_USE_CMSIS_OS
#if (osCMSIS < 0x20000U )
    slave_notify_sem = osSemaphoreCreate(osSemaphore(slave_notify_sem), 1);
    slave_flow_sem = osSemaphoreCreate(osSemaphore(slave_flow_sem), 1);
#else
    slave_notify_sem = osSemaphoreNew(1, 1, NULL);
    slave_flow_sem   = osSemaphoreNew(1, 1, NULL);

#endif /* osCMSIS < 0x20000U*/
    /* take semaphore */
    OSSEMAPHOREWAIT(slave_notify_sem, 1);
    OSSEMAPHOREWAIT(slave_flow_sem, 1);
#endif /* WIFI_USE_CMSIS_OS */
    rc = 0;
  }

  return rc;
}

/**
  * @brief  DeInitialize the SPI
  * @param  None
  * @retval None
  */
static int8_t MX_WIFI_SPI_DeInit(void)
{
  HAL_SPI_DeInit(&hspi_mx);

#ifdef  WIFI_USE_CMSIS_OS
  osSemaphoreDelete(slave_notify_sem);
  osSemaphoreDelete(slave_flow_sem);
#endif /* WIFI_USE_CMSIS_OS */

  return 0;
}

/**
  * @brief  SPI read/write cmd byte
  */
static int32_t wait_wifi_notify_event(int32_t timeout)
{
#ifdef  WIFI_USE_CMSIS_OS
  if (osOK != OSSEMAPHOREWAIT(slave_notify_sem, timeout))
  {
    return -1;
  }
#else
  int32_t tickstart = HAL_GetTick();
  while (0 == spi_slave_notify_event)
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      return -1;
    }
  }
  spi_slave_notify_event = 0;
#endif /* WIFI_USE_CMSIS_OS */
  return 0;
}

static int32_t wait_wifi_idle(int32_t timeout)
{
#ifdef  WIFI_USE_CMSIS_OS
  if (osOK != OSSEMAPHOREWAIT(slave_flow_sem, timeout))
  {
    return -1;
  }
#else
  int32_t tickstart = HAL_GetTick();
  while (0 == spi_slave_flow_event)
  {
    if ((HAL_GetTick() - tickstart) > timeout)
    {
      return -1;
    }
  }
  spi_slave_flow_event = 0;
#endif /* WIFI_USE_CMSIS_OS */
  return 0;
}

/**
  * @brief  Interrupt handler for  Data RDY signal
  * @param  None
  * @retval None
  */
void SPI_WIFI_ISR(uint16_t isr_source)
{
  if (MX_WIFI_SPI_IO_NOTIFY_PIN == isr_source)
  {
#ifdef  WIFI_USE_CMSIS_OS
    osSemaphoreRelease(slave_notify_sem);
#else
    spi_slave_notify_event = 1;
#endif /* WIFI_USE_CMSIS_OS */
  }
  else if (MX_WIFI_SPI_IO_FLOW_PIN == isr_source)
  {
#ifdef  WIFI_USE_CMSIS_OS
    osSemaphoreRelease(slave_flow_sem);
#else
    spi_slave_flow_event = 1;
#endif /* WIFI_USE_CMSIS_OS */
  }
  else
  {
  }
}

/**
  * @brief  Recv wifi Data thru SPI
  * @param  pdata : pointer to data
  * @param  len : Data length
  * @param  timeout : send timeout in mS
  * @retval Length of recved data
  */
int16_t MX_WIFI_SPI_ReceiveData(uint8_t *pdata, uint16_t len, uint32_t timeout_ms)
{
  int16_t rc = -1;
  uint8_t type = SPI_READ;
  uint16_t read_len  = 0;

  MX_WIFI_SPI_CS_ENABLE();

  if (wait_wifi_notify_event(timeout_ms) < 0)
  {
    /*DEBUG_LOG("DEBUG: spi recv: wait wifi data timeout.\r\n");*/
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }

  if (HAL_SPI_Transmit(&hspi_mx, &type, 1, 5) != HAL_OK)
  {
    DEBUG_LOG("** ERROR: spi recv: send READ(1) error !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  if (wait_wifi_idle(timeout_ms + 10) < 0)
  {
    DEBUG_LOG("** ERROR: spi recv: wait READ ack timeout !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  DEBUG_LOG("spi READ cmd ok.\r\n");

  if (HAL_SPI_Receive(&hspi_mx, (uint8_t *)&read_len, 2, 10) != HAL_OK)
  {
    DEBUG_LOG("** ERROR: spi recv: recv READ_LEN(2) error !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  if (wait_wifi_idle(timeout_ms + 10) < 0)
  {
    DEBUG_LOG("** ERROR: spi recv: wait READ len ack timeout !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  read_len = (read_len > len) ? len : read_len;
  DEBUG_LOG("spi READ len(%d) ok.\r\n", read_len);

  if (HAL_SPI_Receive(&hspi_mx, pdata, read_len, 200) != HAL_OK)
  {
    DEBUG_LOG("** ERROR: spi recv: recv DATA(%d) error !\r\n", read_len);
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  if (wait_wifi_idle(timeout_ms + 100) < 0)
  {
    DEBUG_LOG("** ERROR: spi recv: wait DATA ack timeout !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  DEBUG_LOG("spi READ data(%d) ok.\r\n", read_len);
  rc = read_len;

error_exit:
  MX_WIFI_SPI_CS_DISABLE();
  return rc;
}

/**
  * @brief  Send wifi Data thru SPI
  * @param  pdata : pointer to data
  * @param  len : Data length
  * @param  timeout : send timeout in mS
  * @retval Length of sent data
  */
static int16_t MX_WIFI_SPI_SendData(uint8_t *pdata,  uint16_t len, uint32_t timeout_ms)
{
  int16_t rc = -1;
  uint8_t type = SPI_WRITE;
  uint16_t send_len  = len;

  MX_WIFI_SPI_CS_ENABLE();

#ifdef  WIFI_USE_CMSIS_OS
  OSSEMAPHOREWAIT(slave_flow_sem, 1);
  OSSEMAPHOREWAIT(slave_notify_sem, 1);
#else
  spi_slave_notify_event = 0;
  spi_slave_flow_event = 0;
#endif /* WIFI_USE_CMSIS_OS */

  if (HAL_SPI_Transmit(&hspi_mx, (uint8_t *)&type, 1, 5) != HAL_OK)
  {
    DEBUG_LOG("** ERROR: spi send: send WRITE(1) error !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  if (wait_wifi_idle(timeout_ms) < 0)
  {
    DEBUG_LOG("** ERROR: spi send: wait WRITE ACK timeout !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  DEBUG_LOG("spi WRITE cmd ok.\r\n");

  if (HAL_SPI_Transmit(&hspi_mx, (uint8_t *)&send_len, 2, 10) != HAL_OK)
  {
    DEBUG_LOG("** ERROR: spi send: send WRITE_LEN(2) error !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  if (wait_wifi_idle(timeout_ms) < 0)
  {
    DEBUG_LOG("** ERROR: spi send: wait WRITE_LEN ACK timeout !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  DEBUG_LOG("spi WRITE len(%d) ok.\r\n", send_len);

  if (HAL_SPI_Transmit(&hspi_mx, pdata, send_len, 200) != HAL_OK)
  {
    DEBUG_LOG("** ERROR: spi send: send DATA(%d) error !\r\n", send_len);
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  if (wait_wifi_idle(timeout_ms + 100) < 0)
  {
    DEBUG_LOG("** ERROR: spi send: wait DATA ACK timeout !\r\n");
    rc = MX_WIFI_STATUS_IO_ERROR;
    goto error_exit;
  }
  DEBUG_LOG("spi WRITE data(%d) ok.\r\n", send_len);
  rc = send_len;

error_exit:
  MX_WIFI_SPI_CS_DISABLE();
  return rc;
}

/**
  * @brief  probe function to register wifi to connectivity framwotk
  * @param  None
  * @retval None
  */
int32_t wifi_probe(void **ll_drv_context)
{
  if (MX_WIFI_RegisterBusIO(&MxWifiObj,
                            MX_WIFI_SPI_Init,
                            MX_WIFI_SPI_DeInit,
                            MX_WIFI_IO_DELAY,
                            MX_WIFI_SPI_SendData,
                            MX_WIFI_SPI_ReceiveData) == 0)
  {
    *ll_drv_context = &MxWifiObj;
    return 0;
  }

  return -1;
}

#endif /* (MX_WIFI_USE_SPI == 1) */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
