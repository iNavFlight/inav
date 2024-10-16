/**
  ******************************************************************************
  * @file    mx_wifi_spi.c
  * @author  MCD Application Team
  * @brief   This file implements the IO operations to deal with the mx_wifi
  *          module. It mainly initializes and de-initializes the SPI interface.
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

#if defined(MX_WIFI_USE_SPI) && (MX_WIFI_USE_SPI == 1)

#if defined(MX_WIFI_IO_DEBUG)
#define DEBUG_LOG(...)       (void)printf(__VA_ARGS__) /*;*/
#define DEBUG_WARNING(...)   (void)printf(__VA_ARGS__) /*;*/
#else
#define DEBUG_LOG(...)
#define DEBUG_WARNING(...)
#endif /* MX_WIFI_IO_DEBUG */

#define DEBUG_ERROR(...)     (void)printf(__VA_ARGS__) /*;*/


#pragma pack(1)
typedef struct _spi_header
{
  uint8_t  type;
  uint16_t len;
  uint16_t lenx;
  uint8_t  dummy[3];
} spi_header_t;
#pragma pack()


#ifndef MX_WIFI_RESET_PIN

#define MX_WIFI_RESET_PIN        MXCHIP_RESET_Pin
#define MX_WIFI_RESET_PORT       MXCHIP_RESET_GPIO_Port

/* wifi spi cs */
#define MX_WIFI_SPI_CS_PIN       MXCHIP_NSS_Pin
#define MX_WIFI_SPI_CS_PORT      MXCHIP_NSS_GPIO_Port

/* wifi spi slave notify */
#define MX_WIFI_SPI_IRQ_PIN      MXCHIP_NOTIFY_Pin
#define MX_WIFI_SPI_IRQ_PORT     MXCHIP_NOTIFY_GPIO_Port
#define MX_WIFI_SPI_IRQ          MXCHIP_NOTIFY_EXTI_IRQn

#define MX_WIFI_SPI_FLOW_PIN     MXCHIP_FLOW_Pin
#define MX_WIFI_SPI_FLOW_PORT    MXCHIP_FLOW_GPIO_Port

#endif /* MX_WIFI_RESET_PIN */


#ifndef NET_PERF_TASK_TAG
#define NET_PERF_TASK_TAG(...)
#endif /* NET_PERF_TASK_TAG */

/* Private define ------------------------------------------------------------*/
/* SPI protocol */
#define SPI_WRITE         ((uint8_t)0x0A)
#define SPI_READ          ((uint8_t)0x0B)
#define SPI_DATA_SIZE     (MX_WIFI_HCI_DATA_SIZE)

/* HW RESET */

#define MX_WIFI_HW_RESET()                                                    \
  do {                                                                        \
    HAL_GPIO_WritePin(MX_WIFI_RESET_PORT, MX_WIFI_RESET_PIN, GPIO_PIN_RESET); \
    HAL_Delay(100);                                                           \
    HAL_GPIO_WritePin(MX_WIFI_RESET_PORT, MX_WIFI_RESET_PIN, GPIO_PIN_SET);   \
    HAL_Delay(1200);                                                          \
    DEBUG_LOG("\n[%" PRIu32 "] MX_WIFI_HW_RESET\n\n", HAL_GetTick());         \
  } while(0)

/* SPI CS */
#define MX_WIFI_SPI_CS_HIGH()                                                 \
  do {                                                                        \
    HAL_GPIO_WritePin(MX_WIFI_SPI_CS_PORT, MX_WIFI_SPI_CS_PIN, GPIO_PIN_SET); \
  } while(0)


#define MX_WIFI_SPI_CS_LOW()                                                   \
  do {                                                                         \
    HAL_GPIO_WritePin(MX_WIFI_SPI_CS_PORT, MX_WIFI_SPI_CS_PIN, GPIO_PIN_RESET);\
  } while(0)

/* SPI IRQ */
#define MX_WIFI_SPI_IRQ_IS_HIGH() \
  (GPIO_PIN_SET == HAL_GPIO_ReadPin(MX_WIFI_SPI_IRQ_PORT, MX_WIFI_SPI_IRQ_PIN))

#define MX_WIFI_SPI_FLOW_IS_LOW() \
  (GPIO_PIN_RESET == HAL_GPIO_ReadPin(MX_WIFI_SPI_FLOW_PORT, MX_WIFI_SPI_FLOW_PIN))

/* Global variables  ---------------------------------------------------------*/
extern SPI_HandleTypeDef MXCHIP_SPI;

/* Private variables ---------------------------------------------------------*/
static MX_WIFIObject_t MxWifiObj = {0};
static SPI_HandleTypeDef *const HSpiMX = &MXCHIP_SPI;

static LOCK_DECLARE(SpiTxLock);

static SEM_DECLARE(SpiTxRxSem);
static SEM_DECLARE(SpiFlowRiseSem);
static SEM_DECLARE(SpiTransferDoneSem);

static uint8_t *SpiTxData = NULL;
static uint16_t SpiTxLen  = 0;

/* Private functions ---------------------------------------------------------*/
static uint16_t MX_WIFI_SPI_Read(uint8_t *buffer, uint16_t buff_size);
static HAL_StatusTypeDef TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint8_t *rxdata, uint16_t datalen,
                                         uint32_t timeout);
static HAL_StatusTypeDef Transmit(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint16_t datalen, uint32_t timeout);
static HAL_StatusTypeDef Receive(SPI_HandleTypeDef *hspi, uint8_t *rxdata, uint16_t datalen, uint32_t timeout);

static int8_t wait_flow_high(uint32_t timeout);
static uint16_t MX_WIFI_SPI_Write(uint8_t *data, uint16_t len);

static int8_t mx_wifi_spi_txrx_start(void);
static int8_t mx_wifi_spi_txrx_stop(void);

static void MX_WIFI_IO_DELAY(uint32_t ms);
static int8_t MX_WIFI_SPI_Init(uint16_t mode);
static int8_t MX_WIFI_SPI_DeInit(void);


#ifndef MX_WIFI_BARE_OS_H
static THREAD_DECLARE(MX_WIFI_TxRxThreadId);

static __IO bool SPITxRxTaskQuitFlag = false;

static void mx_wifi_spi_txrx_task(THREAD_CONTEXT_TYPE argument);
#endif /* MX_WIFI_BARE_OS_H */


static void MX_WIFI_IO_DELAY(uint32_t ms)
{
  DELAY_MS(ms);
}


/**
  * @brief  Initialize the SPI
  * @param  mode
  * @retval status
  */
static int8_t MX_WIFI_SPI_Init(uint16_t mode)
{
  int8_t ret = 0;

  if (MX_WIFI_RESET == mode)
  {
    MX_WIFI_HW_RESET();
  }
  else
  {
    ret = mx_wifi_spi_txrx_start();
  }

  return ret;
}


/**
  * @brief  De-Initialize the SPI
  * @param  None
  * @retval status
  */
static int8_t MX_WIFI_SPI_DeInit(void)
{
  mx_wifi_spi_txrx_stop();
  return 0;
}


void HAL_SPI_TransferCallback(void *hspi)
{
  (void)hspi;
  SEM_SIGNAL(SpiTransferDoneSem);
}


void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi == HSpiMX)
  {
    MX_ASSERT(false);
  }
}


/**
  * @brief  Interrupt handler for IRQ and FLOW pin
  * @param  isr_source
  * @retval None
  */
void mxchip_WIFI_ISR(uint16_t isr_source)
{
  /*DEBUG_LOG("\n[%"PRIu32"] %s()> %" PRIx32 "\n\n", HAL_GetTick(), __FUNCTION__, (uint32_t)isr_source);*/

  if (MX_WIFI_SPI_IRQ_PIN == isr_source)
  {
    SEM_SIGNAL(SpiTxRxSem);
  }
  if (MX_WIFI_SPI_FLOW_PIN == isr_source)
  {
    SEM_SIGNAL(SpiFlowRiseSem);
  }
}


static int8_t wait_flow_high(uint32_t timeout)
{
  int8_t ret = 0;
  if (SEM_WAIT(SpiFlowRiseSem, timeout, NULL) != SEM_OK)
  {
    ret = -1;
  }
  if (MX_WIFI_SPI_FLOW_IS_LOW())
  {
    DEBUG_ERROR("FLOW is low\n");
    ret = -1;
  }

  DEBUG_LOG("\n%s()< %" PRIi32 "\n\n", __FUNCTION__, (int32_t)ret);

  return ret;
}


static uint16_t MX_WIFI_SPI_Write(uint8_t *data, uint16_t len)
{
  uint16_t sent;

  DEBUG_LOG("\n%s()> %" PRIu32 "\n\n", __FUNCTION__, (uint32_t)len);

  LOCK(SpiTxLock);

  if ((NULL == data) || (0 == len) || (len > SPI_DATA_SIZE))
  {
    DEBUG_ERROR("Warning, SPI send null or size overflow! len=%" PRIu32 "\n", (uint32_t)len);
    SpiTxLen = 0;
    sent = 0;
  }
  else
  {
    SpiTxData = data;
    SpiTxLen  = len;

    if (SEM_SIGNAL(SpiTxRxSem) != SEM_OK)
    {
      /* Happen if received thread did not have a chance to run on time, need to increase priority */
      DEBUG_ERROR("Warning, SPI semaphore has been already notified\n");
    }
    sent = len;
  }

  UNLOCK(SpiTxLock);

  DEBUG_LOG("\n%s()< %" PRIi32 "\n\n", __FUNCTION__, (int32_t)sent);

  return sent;
}


static uint16_t MX_WIFI_SPI_Read(uint8_t *buffer, uint16_t buff_size)
{
  (void)buffer;
  (void)buff_size;
  return 0;
}


static HAL_StatusTypeDef TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint8_t *rxdata, uint16_t datalen,
                                         uint32_t timeout)
{
  HAL_StatusTypeDef ret;

  DEBUG_LOG(("\n%s()> %"PRIu32"\n"), __FUNCTION__, (uint32_t)datalen);

#if 0
  for (uint32_t i = 0; i < datalen; i++)
  {
    DEBUG_LOG("%02" PRIx32 " ", (uint32_t)txdata[i]);
  }
#endif /* 0 */

#if (defined(DMA_ON_USE) && (DMA_ON_USE == 1))
  ret = HAL_SPI_TransmitReceive_DMA(hspi, txdata, rxdata, datalen);
  SEM_WAIT(SpiTransferDoneSem, timeout, NULL);

#else
  ret = HAL_SPI_TransmitReceive(hspi, txdata, rxdata, datalen, timeout);
#endif /* (DMA_ON_USE == 1) */

  DEBUG_LOG("\n%s()< %" PRIi32 "\n\n", __FUNCTION__, (int32_t)ret);

  return ret;
}


static HAL_StatusTypeDef Transmit(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint16_t datalen, uint32_t timeout)
{
  HAL_StatusTypeDef ret;

  DEBUG_LOG("\n%s()> %" PRIu32 "\n", __FUNCTION__, (uint32_t)datalen);

#if 0
  for (uint32_t i = 0; i < datalen; i++)
  {
    DEBUG_LOG("%02" PRIx32 " ", (uint32_t)txdata[i]);
  }
#endif /* 0 */


#if (defined(DMA_ON_USE) && (DMA_ON_USE == 1))
  ret = HAL_SPI_Transmit_DMA(hspi, txdata, datalen);
  SEM_WAIT(SpiTransferDoneSem, timeout, NULL);

#else
  ret = HAL_SPI_Transmit(hspi, txdata, datalen, timeout);
#endif /* (DMA_ON_USE == 1) */

  DEBUG_LOG("\n%s() <%" PRIi32 "\n\n", __FUNCTION__, (int32_t)ret);

  return ret;
}


static HAL_StatusTypeDef Receive(SPI_HandleTypeDef *hspi, uint8_t *rxdata, uint16_t datalen, uint32_t timeout)
{
  HAL_StatusTypeDef ret;

  DEBUG_LOG("\n%s()> %" PRIu32 "\n", __FUNCTION__, (uint32_t)datalen);

#if (defined(DMA_ON_USE) && (DMA_ON_USE == 1))
  ret = HAL_SPI_Receive_DMA(hspi, rxdata, datalen);
  SEM_WAIT(SpiTransferDoneSem, timeout, NULL);

#else
  ret = HAL_SPI_Receive(hspi, rxdata, datalen, timeout);
#endif /* (DMA_ON_USE == 1) */

#if 0
  for (uint32_t i = 0; i < datalen; i++)
  {
    DEBUG_LOG("%02" PRIx32 " ", (uint32_t)rxdata[i]);
  }
#endif /* 0 */

  DEBUG_LOG("\n%s()< %" PRIi32 "\n\n", __FUNCTION__, (int32_t)ret);

  return ret;
}


void process_txrx_poll(uint32_t timeout)
{
  static mx_buf_t *netb = NULL;
  bool first_miss = true;

  MX_WIFI_SPI_CS_HIGH();

  while (netb == NULL)
  {
    netb = MX_NET_BUFFER_ALLOC(MX_WIFI_BUFFER_SIZE);

    MX_STAT(alloc);

    if (netb == NULL)
    {
      DELAY_MS(1);
      if (true == first_miss)
      {
        first_miss = false;
        DEBUG_WARNING("Running Out of buffer for RX\n");
      }
    }
  }

  /* Waiting for data to be sent or to be received. */
  if (SEM_WAIT(SpiTxRxSem, timeout, NULL) == SEM_OK)
  {
    NET_PERF_TASK_TAG(0);

    LOCK(SpiTxLock);
    {
      spi_header_t mheader = {0};
      spi_header_t sheader = {0};
      uint8_t *txdata = NULL;
      bool is_continue = true;

      DEBUG_LOG("\n%s(): %p\n", __FUNCTION__, SpiTxData);

      if (SpiTxData == NULL)
      {
        if (!MX_WIFI_SPI_IRQ_IS_HIGH())
        {
          /* TX data null means no data to send, IRQ low means no data to be received. */
          is_continue = false;

          /* There nothing to do with the SPI. */
          /* Free allocated buffer, due to end of life being requested for the hosting thread. */
#ifndef MX_WIFI_BARE_OS_H
          if (SPITxRxTaskQuitFlag == true)
          {
            MX_NET_BUFFER_FREE(netb);
            netb = NULL;
          }
#endif /* MX_WIFI_BARE_OS_H */
        }
      }
      else
      {
        mheader.len = SpiTxLen;
        txdata = SpiTxData;
      }

      if (is_continue)
      {
        mheader.type = SPI_WRITE;
        mheader.lenx = ~mheader.len;

        MX_WIFI_SPI_CS_LOW();

        {
          /* Wait for the EMW to be ready. */
          if (wait_flow_high(timeout) != 0)
          {
            DEBUG_ERROR("Wait FLOW timeout 0\n");
          }
          else
          {
            /* Transmit only the header part. */
            if (HAL_OK != TransmitReceive(HSpiMX, (uint8_t *)&mheader, (uint8_t *)&sheader, sizeof(mheader), timeout))
            {
              DEBUG_ERROR("Send mheader error\n");
            }
            else
            {
              if (sheader.type != SPI_READ)
              {
                DEBUG_ERROR("Invalid SPI type %02x\n", sheader.type);
              }
              else
              {
                if ((sheader.len ^ sheader.lenx) != 0xFFFF)
                {
                  DEBUG_ERROR("Invalid length %04x-%04x\n", sheader.len, sheader.lenx);
                }
                else
                {
                  /* Send or received header must be not null */
                  if ((sheader.len == 0) && (mheader.len == 0))
                  {
                  }
                  else
                  {
                    if ((sheader.len > SPI_DATA_SIZE) || (mheader.len > SPI_DATA_SIZE))
                    {
                      DEBUG_ERROR("SPI length invalid: %d-%d\n", sheader.len, mheader.len);
                    }
                    else
                    {
                      uint16_t datalen;
                      uint8_t *rxdata = NULL;

                      /* Keep the max length between TX and RX. */
                      if (mheader.len > sheader.len)
                      {
                        datalen = mheader.len;
                      }
                      else
                      {
                        datalen = sheader.len;
                      }

                      /* Allocate a buffer for data to be received. */
                      if (sheader.len > 0)
                      {
                        /* Get start of the buffer payload. */
                        rxdata = MX_NET_BUFFER_PAYLOAD(netb);
                      }

                      /* FLOW must be high. */
                      if (wait_flow_high(timeout) != 0)
                      {
                        DEBUG_ERROR("Wait FLOW timeout 1\n");
                      }
                      else
                      {
                        HAL_StatusTypeDef ret;

                        /* TX with possible RX. */
                        if (NULL != txdata)
                        {
                          SpiTxData = NULL;
                          SpiTxLen = 0;
                          if (NULL != rxdata)
                          {
                            ret = TransmitReceive(HSpiMX, txdata, rxdata, datalen, timeout);
                          }
                          else
                          {
                            ret = Transmit(HSpiMX, txdata, datalen, timeout);
                          }
                        }
                        else
                        {
                          ret = Receive(HSpiMX, rxdata, datalen, timeout);
                        }

                        if (HAL_OK != ret)
                        {
                          DEBUG_ERROR("Transmit/Receive data timeout\n");
                        }
                        else
                        {
                          /* Resize the input buffer and send it back to the processing thread. */
                          if (sheader.len > 0)
                          {
                            NET_PERF_TASK_TAG(1);
                            MX_NET_BUFFER_SET_PAYLOAD_SIZE(netb, sheader.len);
                            mx_wifi_hci_input(netb);
                            netb = NULL;
                          }
                          else
                          {
                            NET_PERF_TASK_TAG(2);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          /* Notify transfer done. */
          MX_WIFI_SPI_CS_HIGH();
        }
      }
    }
    UNLOCK(SpiTxLock);
  }
}


#ifndef MX_WIFI_BARE_OS_H
static void mx_wifi_spi_txrx_task(THREAD_CONTEXT_TYPE argument)
{
  (void)argument;

  SPITxRxTaskQuitFlag = false;

  while (SPITxRxTaskQuitFlag != true)
  {
    process_txrx_poll(WAIT_FOREVER);
  }

  SPITxRxTaskQuitFlag = false;

  /* Prepare deletion (depends on implementation). */
  THREAD_TERMINATE();

  /* Delete the Thread. */
  THREAD_DEINIT(MX_WIFI_TxRxThreadId);
}
#endif /* MX_WIFI_BARE_OS_H */


static int8_t mx_wifi_spi_txrx_start(void)
{
  int8_t ret = 0;

  LOCK_INIT(SpiTxLock);
  SEM_INIT(SpiTxRxSem, 2);
  SEM_INIT(SpiFlowRiseSem, 1);
  SEM_INIT(SpiTransferDoneSem, 1);


  if (THREAD_OK != THREAD_INIT(MX_WIFI_TxRxThreadId, mx_wifi_spi_txrx_task, NULL,
                               MX_WIFI_SPI_THREAD_STACK_SIZE,
                               MX_WIFI_SPI_THREAD_PRIORITY))
  {
    ret = -1;
  }
  else
  {
    /* Notify SPI ready. */
    /* De-select the SPI slave. */
    MX_WIFI_SPI_CS_HIGH();
  }
  return ret;
}


static int8_t mx_wifi_spi_txrx_stop(void)
{
#ifndef MX_WIFI_BARE_OS_H
  /* Set thread quit flag to TRUE. */
  SPITxRxTaskQuitFlag = true;
#endif /* MX_WIFI_BARE_OS_H */

  /* Wake up the thread if it's sleeping. */
  SEM_SIGNAL(SpiTxRxSem);

#ifndef MX_WIFI_BARE_OS_H
  /* Wait for the thread to terminate. */
  while (SPITxRxTaskQuitFlag == true)
  {
    DELAY_MS(500);
  }
#endif /* MX_WIFI_BARE_OS_H */

  /* Delete the Thread (depends on implementation). */
  THREAD_DEINIT(MX_WIFI_TxRxThreadId);
  SEM_DEINIT(SpiTxRxSem);
  SEM_DEINIT(SpiFlowRiseSem);
  LOCK_DEINIT(SpiTxLock);

  return 0;
}


int32_t mxwifi_probe(void **ll_drv_context)
{
  int32_t ret = -1;

  if (MX_WIFI_RegisterBusIO(&MxWifiObj,
                            MX_WIFI_SPI_Init,
                            MX_WIFI_SPI_DeInit,
                            MX_WIFI_IO_DELAY,
                            MX_WIFI_SPI_Write,
                            MX_WIFI_SPI_Read) == MX_WIFI_STATUS_OK)
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

#endif /* (MX_WIFI_USE_SPI == 1) */
