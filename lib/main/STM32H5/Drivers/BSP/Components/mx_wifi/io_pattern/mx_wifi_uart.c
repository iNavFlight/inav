/**
  ******************************************************************************
  * @file    mx_wifi_uart.c
  * @author  MCD Application Team
  * @brief   This file implements the IO operations to deal with the mx_wifi
  *          module. It mainly initializes and de-initializes the UART interface.
  *          Send and receive data over it.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "mx_wifi.h"
#include "mx_wifi_conf.h" /* Get some platform definitions. */
#include "mx_wifi_io.h"
#include "core/mx_wifi_hci.h"
#include "core/mx_wifi_slip.h"

#if defined(MX_WIFI_USE_SPI) && (MX_WIFI_USE_SPI == 0)

#if defined(MX_WIFI_IO_DEBUG)
#define DEBUG_LOG(...)       (void)printf(__VA_ARGS__) /*;*/
#define DEBUG_WARNING(...)   (void)printf(__VA_ARGS__) /*;*/

static void DEBUG_PRINT(char *prefix, uint8_t *data, uint16_t len)
{
  if (NULL != data)
  {
    DEBUG_LOG("%s[%d]:\n", prefix, len);
    for (int32_t i = 0; i < len; i++)
    {
      DEBUG_LOG(" %02x", data[i]);
    }
    DEBUG_LOG("\n");
  }
}
#else

#define DEBUG_LOG(...)
#define DEBUG_WARNING(...)
#define DEBUG_PRINT(...)
#endif /* MX_WIFI_IO_DEBUG */

#ifndef MX_WIFI_RESET_PIN

#define MX_WIFI_RESET_PIN        MXCHIP_RESET_Pin
#define MX_WIFI_RESET_PORT       MXCHIP_RESET_GPIO_Port

#endif /* MX_WIFI_RESET_PIN */

/* Private define ------------------------------------------------------------*/

/* HW RESET */

#define MX_WIFI_HW_RESET()                                                    \
  do {                                                                        \
    HAL_GPIO_WritePin(MX_WIFI_RESET_PORT, MX_WIFI_RESET_PIN, GPIO_PIN_RESET); \
    HAL_Delay(100);                                                           \
    HAL_GPIO_WritePin(MX_WIFI_RESET_PORT, MX_WIFI_RESET_PIN, GPIO_PIN_SET);   \
    HAL_Delay(1200);                                                          \
    DEBUG_LOG("\n[%" PRIu32 "] MX_WIFI_HW_RESET\n\n", HAL_GetTick());         \
  } while(0)

/* Global variables  ---------------------------------------------------------*/
extern UART_HandleTypeDef MXCHIP_UART;

/* Private variables ---------------------------------------------------------*/
static MX_WIFIObject_t MxWifiObj;
static UART_HandleTypeDef *const HUartMX = &MXCHIP_UART;

static SEM_DECLARE(UartRxSem);

static uint8_t RxChar;

/* Private functions ---------------------------------------------------------*/
static uint16_t MX_WIFI_UART_ReceiveData(uint8_t *pdata, uint16_t request_len);
static uint16_t MX_WIFI_UART_SendData(uint8_t *pdata, uint16_t len);

static void MX_WIFI_IO_DELAY(uint32_t ms);
static int8_t MX_WIFI_UART_Init(uint16_t mode);
static int8_t MX_WIFI_UART_DeInit(void);


#ifndef MX_WIFI_BARE_OS_H
static THREAD_DECLARE(MX_WIFI_RxThreadId);

static __IO bool UARTRxTaskQuitFlag = false;

static void mx_wifi_uart_rx_task(THREAD_CONTEXT_TYPE argument);
#endif /* MX_WIFI_BARE_OS_H */


static void MX_WIFI_IO_DELAY(uint32_t ms)
{
  DELAY_MS(ms);
}


/**
  * @brief  Initialize the UART
  * @param  mode
  * @retval status
  */
static int8_t MX_WIFI_UART_Init(uint16_t mode)
{
  int8_t ret = MX_WIFI_STATUS_OK;

  DEBUG_LOG("\n[%" PRIu32 "] %s()>\n", HAL_GetTick(), __FUNCTION__);

  if (MX_WIFI_RESET == mode)
  {
    MX_WIFI_HW_RESET();
  }
  else
  {
    SEM_INIT(UartRxSem, 1);

    if (THREAD_OK != THREAD_INIT(MX_WIFI_RxThreadId,
                                 mx_wifi_uart_rx_task, NULL,
                                 MX_WIFI_UART_THREAD_STACK_SIZE,
                                 MX_WIFI_UART_THREAD_PRIORITY))
    {
      ret = MX_WIFI_STATUS_ERROR;
    }

    HAL_UART_Receive_IT(HUartMX, &RxChar, 1);
  }

  DEBUG_LOG("\n[%" PRIu32 "] %s()<\n\n", HAL_GetTick(), __FUNCTION__);

  return ret;
}


/**
  * @brief  De-Initialize the UART
  * @param  None
  * @retval status
  */
static int8_t MX_WIFI_UART_DeInit(void)
{
  int8_t rc = 0;

#ifndef MX_WIFI_BARE_OS_H
  /* Set thread quit flag to TRUE. */
  UARTRxTaskQuitFlag = true;
#endif /* MX_WIFI_BARE_OS_H */

  /* Wake up the thread if it's sleeping. */
  SEM_SIGNAL(UartRxSem);

#ifndef MX_WIFI_BARE_OS_H
  /* Wait for the thread to terminate. */
  while (UARTRxTaskQuitFlag == true)
  {
    DELAY_MS(500);
  }
#endif /* MX_WIFI_BARE_OS_H */

  /* Delete the Thread (depends on implementation). */
  THREAD_DEINIT(MX_WIFI_RxThreadId);

  SEM_DEINIT(UartRxSem);

  /* Uart initialized in main(), so not de-init here, just stop IT. */
  if (HAL_UART_Abort_IT(HUartMX) != HAL_OK)
  {
    /* Error_Handler(); */
    MX_ASSERT(false);
  }

  return rc;
}


static uint8_t RxBuffer[MX_CIRCULAR_UART_RX_BUFFER_SIZE];
static __IO uint32_t RxBufferWritePos = 0;
static __IO uint32_t RxBufferReadPos = 0;

/* Sending data byte per byte to HCI would be very inefficient in term of       */
/* MCU cycles, buffering whole MTU is too costly, need to allocate a            */
/* while MTU buffer size and perform copy, so find a compromise                 */
/* using a circular buffer approach and sending data by segment of half         */
/* buffer                                                                       */

/**
  * @brief  Rx Callback when new data is received on the UART.
  * @param  huart: Uart handle receiving the data.
  * @retval None.
  */
void mxchip_WIFI_ISR_UART(void *huart)
{
  static uint32_t circular_uart_pending_data_len = 0;

  RxBuffer[RxBufferWritePos++] = RxChar;
  circular_uart_pending_data_len++;

  /* Re-wrap write index for the circular buffer. */
  if (MX_CIRCULAR_UART_RX_BUFFER_SIZE == RxBufferWritePos)
  {
    RxBufferWritePos = 0;
  }

  if (RxBufferWritePos == RxBufferReadPos)
  {
    /* This should not happen, or we run out of buffer and data are lost. */
    MX_ASSERT(false);
  }

  /* Circular buffer is half full, so it is time to signal data to HCI. */
  if ((MX_CIRCULAR_UART_RX_BUFFER_SIZE / 2) == circular_uart_pending_data_len)
  {
    SEM_SIGNAL(UartRxSem);
    circular_uart_pending_data_len = 0;
  }

  if (0 != circular_uart_pending_data_len)
  {
    /**
      * use the SLIP_END character to signal data to next stage,
      * to avoid waiting for other data that would never come
      * in fact it should be possible to use UART native HW support for such detection/management.
      */
    if (((uint8_t)SLIP_END) == RxChar)
    {
      SEM_SIGNAL(UartRxSem);
      circular_uart_pending_data_len = 0;
    }
  }

  /* Fire again request to get a new byte. */
  if (HAL_OK != HAL_UART_Receive_IT((UART_HandleTypeDef *)huart, &RxChar, 1))
  {
    MX_ASSERT(false);
  }
}


void process_txrx_poll(uint32_t timeout)
{
  /* Waiting for having data received on UART. */
  if (SEM_OK == SEM_WAIT(UartRxSem, timeout, NULL))
  {
    /* This a volatile so copy it to a local one to avoid any issues. */
    const uint32_t write_pos = RxBufferWritePos;
    uint32_t read_pos = RxBufferReadPos;

    DEBUG_LOG("W:%" PRIu32 " R:%" PRIu32 "\n", write_pos, read_pos);

    if (write_pos != read_pos)
    {
      /* write_pos pointer has re-looped, so send the two segments. */
      do
      {
        mx_buf_t *nbuf = slip_input_byte(RxBuffer[read_pos]);
        if (NULL != nbuf)
        {
          DEBUG_PRINT("URX", MX_NET_BUFFER_PAYLOAD(nbuf), MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf));
          mx_wifi_hci_input(nbuf);
        }
        read_pos = read_pos + 1;
        if (MX_CIRCULAR_UART_RX_BUFFER_SIZE == read_pos)
        {
          read_pos = 0;
        }
      } while (read_pos != write_pos);
      RxBufferReadPos = write_pos;
    }
  }
}


#ifndef MX_WIFI_BARE_OS_H
static void mx_wifi_uart_rx_task(THREAD_CONTEXT_TYPE argument)
{
  (void)argument;

  UARTRxTaskQuitFlag = false;

  while (UARTRxTaskQuitFlag != true)
  {
    process_txrx_poll(WAIT_FOREVER);
  }

  UARTRxTaskQuitFlag = false;

  /* Prepare deletion (depends on implementation). */
  THREAD_TERMINATE();

  /* Delete the Thread. */
  THREAD_DEINIT(MX_WIFI_RxThreadId);
}
#endif /* MX_WIFI_BARE_OS_H */


/**
  * @brief  Send WiFi data through UART
  * @param  pdata : pointer to data
  * @param  len : data length
  * @retval Length of sent data
  */
static uint16_t MX_WIFI_UART_SendData(uint8_t *pdata, uint16_t len)
{
  uint16_t rc = len;

  DEBUG_PRINT("UTX", pdata, len);

  if (HAL_UART_Transmit(HUartMX, pdata, len, 200) != HAL_OK)
  {
    rc =  0;
  }

  return rc;
}


static uint16_t MX_WIFI_UART_ReceiveData(uint8_t *pdata, uint16_t request_len)
{
  uint16_t len = request_len;

  if (HAL_UART_Receive(HUartMX, pdata, len, 100) != HAL_OK)
  {
    len = 0;
  }

  return len;
}


int32_t mxwifi_probe(void **ll_drv_context)
{
  int32_t ret = -1;
  if (MX_WIFI_RegisterBusIO(&MxWifiObj,
                            MX_WIFI_UART_Init,
                            MX_WIFI_UART_DeInit,
                            MX_WIFI_IO_DELAY,
                            MX_WIFI_UART_SendData,
                            MX_WIFI_UART_ReceiveData) == MX_WIFI_STATUS_OK)
  {
    if (NULL != ll_drv_context)
    {
      *ll_drv_context = &MxWifiObj;
    }
    ret = 0;
  }

  return ret;
}


MX_WIFIObject_t *wifi_obj_get(void)
{
  return &MxWifiObj;
}

#endif /* (MX_WIFI_USE_SPI == 0) */
