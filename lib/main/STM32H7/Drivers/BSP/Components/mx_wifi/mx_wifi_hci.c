/**
  ******************************************************************************
  * @file    mx_wifi_hci.c
  * @author  MCD Application Team
  * @brief   Host driver HCI protocol of MXCHIP Wi-Fi component.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
/*cstat -MISRAC2012-* */
#include "stdlib.h"
/*cstat +MISRAC2012-* */

#include "mx_wifi_conf.h"
#include "mx_wifi_hci.h"
#include "mx_wifi_slip.h"
#include "CheckSumUtils.h"

#ifdef MX_WIFI_HCI_DEBUG
#define DEBUG_LOG(M, ...)  printf(M, ##__VA_ARGS__);
#else
#define DEBUG_LOG(M, ...)
#endif

/* Private defines -----------------------------------------------------------*/
#define HCI_RETRY_INTERVAL  100 // wait 100ms for ACK, if no ACK, resend hci frame
#ifdef MX_WIFI_HCI_ACK_ENABLE
#define HCI_RETRY_MAX       3   // HCI fram max retry 3 times, then drop and return error
#else
#define HCI_RETRY_MAX       1   // no retry if no ack
#endif
#define HCI_RECV_QUEUE_SIZE 3 // queue size 3


/*
  *  recv data queue
  */
typedef struct _hci_recv_data_t
{
  uint16_t len;
  uint8_t *data;
} hci_recv_data_t;

typedef struct _hci_recv_queue_t
{
  uint16_t num;
  uint16_t head;
  uint16_t tail;
  hci_recv_data_t buf[HCI_RECV_QUEUE_SIZE];
} hci_recv_queue_t;


/* Private function prototypes -----------------------------------------------*/
static phy_tx_func_t hci_phy_send_cb = NULL;
static phy_rx_func_t hci_phy_recv_cb = NULL;
static event_notify_func_t notify_event_cb = NULL;

#if MX_WIFI_USE_CMSIS_OS
static osMutexId hci_lock_id;
#define MX_WIFI_HCI_LOCK(a, to)         (osOK == osMutexWait(a, to))
#define MX_WIFI_HCI_UNLOCK(a)       (osMutexRelease(a))
#else
#define MX_WIFI_HCI_LOCK(a,to)         (true)
#define MX_WIFI_HCI_UNLOCK(a)
#endif

#if (MX_WIFI_USE_SPI == 1)
static uint8_t spi_recv_buf[MX_WIFI_DATA_SIZE + 600] = {0};
#endif

#if (MX_WIFI_USE_SPI == 0)
static uint8_t hci_seq_num = 0;
#endif

static hci_recv_queue_t hci_msg_queue;

/* Private functions ---------------------------------------------------------*/

static void mx_wifi_hci_yield(uint32_t timeout_ms);

//TODO add lock
static int8_t hci_recv_queue_init(hci_recv_queue_t *pqueue)
{
  int8_t rc = -1;

  if (NULL != pqueue)
  {
    pqueue->num  = 0;
    pqueue->head = 0;
    pqueue->tail = 0;
    for (int32_t i = 0; i < HCI_RECV_QUEUE_SIZE; i++)
    {
      pqueue->buf[i].data = NULL;
      pqueue->buf[i].len = 0;
    }
    rc = 0;
  }

  return rc;
}

static int8_t hci_recv_queue_deinit(hci_recv_queue_t *pqueue)
{
  int8_t rc = -1;

  if (NULL != pqueue)
  {
    for (int32_t i = 0; i < HCI_RECV_QUEUE_SIZE; i++)
    {
      if (NULL != pqueue->buf[i].data)
      {
        HCI_FREE(pqueue->buf[i].data);
        pqueue->buf[i].data = NULL;
      }
      pqueue->buf[i].len = 0;
    }
    pqueue->num  = 0;
    pqueue->head = 0;
    pqueue->tail = 0;

    rc = 0;
  }

  return rc;
}

static uint16_t hci_recv_queue_msg_num(hci_recv_queue_t *pqueue)
{
  return pqueue->num;
}

static int8_t hci_recv_queue_push(hci_recv_queue_t *pqueue, uint8_t *data, uint16_t len)
{
  int8_t rc = -1;
  uint8_t *pbuf;
  void *p;

  if ((NULL != pqueue) && (NULL != data) && (len > (uint16_t)0))
  {
    if ((uint16_t)HCI_RECV_QUEUE_SIZE == hci_recv_queue_msg_num(pqueue))  // full, delete head data
    {
      if (NULL != pqueue->buf[pqueue->head].data)
      {
        HCI_FREE(pqueue->buf[pqueue->head].data);
        pqueue->buf[pqueue->head].data = NULL;
      }
      pqueue->buf[pqueue->head].len = 0;
      pqueue->head = (pqueue->head + (uint16_t)1) % (uint16_t)HCI_RECV_QUEUE_SIZE;
    }
    /* add data after tail */
    p = HCI_MALLOC(len);
    if (NULL != p)
    {
      (void)memcpy(&pbuf, &p, sizeof(pbuf));
      (void)memcpy(pbuf, data, len);
      pqueue->buf[pqueue->tail].data = pbuf;
      pqueue->buf[pqueue->tail].len = len;
      pqueue->tail = (pqueue->tail + (uint16_t)1) % (uint16_t)HCI_RECV_QUEUE_SIZE;
      if (pqueue->num < (uint16_t)HCI_RECV_QUEUE_SIZE)
      {
        pqueue->num++;
      }

      rc = 0;
    }
  }

  return rc;
}

static int8_t hci_recv_queue_pop(hci_recv_queue_t *pqueue, uint8_t **pdata, uint16_t *plen)
{
  int8_t rc = -1;

  if ((NULL != pqueue) && (NULL != pdata) && (NULL != plen))
  {
    if (hci_recv_queue_msg_num(pqueue) > (uint16_t)0)
    {
      *pdata = pqueue->buf[pqueue->head].data;
      *plen = pqueue->buf[pqueue->head].len;

      pqueue->buf[pqueue->head].data = NULL;
      pqueue->buf[pqueue->head].len = 0;
      pqueue->head = (pqueue->head + (uint16_t)1) % (uint16_t)HCI_RECV_QUEUE_SIZE;
      pqueue->num--;
    }
    else
    {
      *plen = 0;
      *pdata = NULL;
    }

    rc = 0;
  }

  return rc;
}

int16_t hci_phy_send(uint8_t *data, uint16_t len, uint32_t timeout)
{
  int16_t rc;

  rc = hci_phy_send_cb(data, len, timeout);

  return rc;
}

int8_t hci_data_process(uint8_t *data, uint16_t len)
{
  int8_t rc;

  rc = hci_recv_queue_push(&hci_msg_queue, data, len);

  return rc;
}

int8_t hci_ack_process(uint8_t seq)
{
#ifdef MX_WIFI_HCI_DEBUG
  DEBUG_LOG("hci_ack_process: seq[%d]\r\n", seq);
#endif

#ifdef  MX_WIFI_HCI_ACK_ENABLE
  if (seq == hci_seq_num)
  {
    hci_seq_num++;
    hci_ack_flag = 1;
  }
  else
  {
    DEBUG_LOG("hci_ack_process: HCI_EVENT_ERROR_OUT_OF_SEQ\r\n");
    if (NULL != notify_event_cb)
    {
      notify_event_cb(HCI_EVENT_ERROR_OUT_OF_SEQ);
    }
  }
#endif
  (void)seq;

  return 0;
}

void hci_event_notify(uint8_t event)
{
#ifdef MX_WIFI_HCI_DEBUG
  DEBUG_LOG("hci_event_notify: event[%d]\r\n", event);
#endif
  if (NULL != notify_event_cb)
  {
    notify_event_cb(event);
  }
}
//#endif


/* Global functions ----------------------------------------------------------*/
/*
  * @param   timeout: in ms
  * @retval  0: success, <0: errcode,  >0: data send/recv len
  *
  */
int8_t mx_wifi_hci_init(phy_tx_func_t phy_send_func, phy_rx_func_t phy_recv_func, event_notify_func_t event_cb)
{
  int8_t rc;
#if MX_WIFI_USE_CMSIS_OS
  osMutexDef_t hci_lock;
#endif

  hci_phy_send_cb = phy_send_func;
  hci_phy_recv_cb = phy_recv_func;
#if (MX_WIFI_USE_SPI == 1)
  (void)memset(spi_recv_buf, 0, sizeof(spi_recv_buf));
#endif
  (void)hci_recv_queue_init(&hci_msg_queue);

#if MX_WIFI_USE_CMSIS_OS
#if (osCMSIS < 0x20000U )
  hci_lock_id = osMutexCreate(&hci_lock);
#else
  hci_lock_id = osMutexNew(&hci_lock);
#endif /* osCMSIS < 0x20000U */
#endif
#if (MX_WIFI_USE_SPI == 1)
  rc = 0;
  notify_event_cb = event_cb;
#else // UART
  notify_event_cb = event_cb;
  hci_seq_num = 0;
  rc = (int8_t)slip_init();
#endif
  return rc;
}

int8_t mx_wifi_hci_deinit(void)
{
#if MX_WIFI_USE_CMSIS_OS
  (void)osMutexDelete(hci_lock_id);
#endif

#if (MX_WIFI_USE_SPI == 0)
  notify_event_cb = NULL;
  hci_seq_num = 0;
  (void)slip_deinit();
#endif

  return hci_recv_queue_deinit(&hci_msg_queue);
}

static void mx_wifi_hci_yield(uint32_t timeout_ms)
{
  int16_t rc;

#if (MX_WIFI_USE_SPI == 1)
  rc = hci_phy_recv_cb(spi_recv_buf, MX_WIFI_DATA_SIZE + 600, timeout_ms);
  if (rc > 0)
  {
#ifdef MX_WIFI_HCI_DEBUG
    DEBUG_LOG("RX HCI(SPI) got %d bytes:\r\n", rc);
    for (int32_t i = 0; i < rc; i++)
    {
      DEBUG_LOG("%02x ", spi_recv_buf[i]);
    }
    DEBUG_LOG("\r\n");
#endif
    (void)hci_recv_queue_push(&hci_msg_queue, spi_recv_buf, (uint16_t)rc);
  }
#else // UART
  uint8_t in_byte = 0;
  uint32_t tick_start = HAL_GetTick();

  while ((HAL_GetTick() - tick_start) < timeout_ms)
  {
    rc = hci_phy_recv_cb(&in_byte, 1, 10); // recv 1byte from phy
    if (1 == rc)
    {
      (void)slip_input_byte(in_byte); // process byte to detect new hci frame ,then push hci data to queue or set ack flag
    }
  }
#endif
}

int16_t mx_wifi_hci_send(uint8_t *data, uint16_t len, uint32_t timeout)
{
#ifdef  MX_WIFI_HCI_ACK_ENABLE
  static volatile uint8_t hci_ack_flag = 0;
#endif
  int16_t rc;

#if (MX_WIFI_USE_SPI == 1)

#ifdef MX_WIFI_HCI_DEBUG
  DEBUG_LOG("TX HCI(SPI) %d bytes:\r\n", len);
  for (int32_t i = 0; i < len; i++)
  {
    DEBUG_LOG("%02x ", data[i]);
  }
  DEBUG_LOG("\r\n");
#endif

#if MX_WIFI_USE_CMSIS_OS
  if (MX_WIFI_HCI_LOCK(hci_lock_id, timeout))
  {
    rc = hci_phy_send(data, len, timeout);
    (void)MX_WIFI_HCI_UNLOCK(hci_lock_id);
  }
  else
  {
    rc = -1;
  }
#else
  rc = hci_phy_send(data, len, timeout);
#endif

#else /* UART */

  rc = 0;
  void *pbuf;
  uint16_t hci_send_retry = 0;
#ifdef MX_WIFI_HCI_ACK_ENABLE
  uint32_t tick_start = 0;
#endif

  // prepare hci frame to send
  int32_t j = 0, sliplen;
  uint8_t *buff, *slip_buf;

#ifdef MX_WIFI_HCI_ACK_ENABLE

#ifdef USE_STM32L_CRC
  CRC_HandleTypeDef crc;  //STM32L CRC
#else
  CRC16_Context crc; // software crc
#endif

#endif

  uint16_t crc16 = 0xFFFF;

#ifdef MX_WIFI_HCI_DEBUG
  DEBUG_LOG("TX HCI(UART) %d bytes:\r\n", len);
  for (int32_t i = 0; i < len; i++)
  {
    DEBUG_LOG("%02x ", data[i]);
  }
  DEBUG_LOG("\r\n");
#endif

  pbuf = HCI_MALLOC((size_t)len + (size_t)4);
  if (pbuf == NULL)
  {
    DEBUG_LOG("** ERROR: mx_wifi_hci_send HCI_MALLOC failed !\r\n");
    rc = -1;
  }
  else
  {
    (void)memcpy(&buff, &pbuf, sizeof(buff));
    buff[j] = SLIP_DATA;
    j++;
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_HCI_LOCK(hci_lock_id, timeout))
    {
      HCI_FREE(buff);
      rc = -1;
    }
    else
#endif
    {
#ifdef MX_WIFI_HCI_ACK_ENABLE
      buff[j] = hci_seq_num;
#else
      buff[j] = hci_seq_num;
      hci_seq_num++;
#endif
      j++;
      (void)memcpy(&buff[j], data, len);
      j += (int32_t)len;
#ifdef MX_WIFI_HCI_ACK_ENABLE
#ifdef USE_STM32L_CRC  // STM32L4 CRC
      HW_CRC16_Init(&crc);
      HW_CRC16_Update(&crc, buff, j, &crc16);
      crc16 = htons(crc16);
#else
      CRC16_Init(&crc);
      CRC16_Update(&crc, buff, j);
      CRC16_Final(&crc, &crc16);
      crc16 = htons(crc16);
#endif
#endif // MX_WIFI_HCI_ACK_ENABLE
      (void)memcpy(&buff[j], &crc16, 2);
      j += 2;

      slip_buf = slip_transfer(buff, j, &sliplen);
      HCI_FREE(buff);
      if (slip_buf == NULL)
      {
        DEBUG_LOG("** ERROR: mx_wifi_hci_send slip_buf null !\r\n");
#if MX_WIFI_USE_CMSIS_OS
        (void)MX_WIFI_HCI_UNLOCK(hci_lock_id);
#endif
        rc = -1;
      }
      else
      {
        /* send hci frame with retry */
        while (hci_send_retry < (uint16_t)HCI_RETRY_MAX)
        {
          // timeout && retry check
#ifdef MX_WIFI_HCI_DEBUG
          if (hci_send_retry > 0)
          {
            DEBUG_LOG("hci_phy_send retry %d ...\r\n", hci_send_retry);
          }
#endif
          hci_send_retry++;

          // frame send
#ifdef MX_WIFI_HCI_ACK_ENABLE
          hci_ack_flag = 0;  // clear ack flag
#endif
          rc = hci_phy_send(slip_buf, (uint16_t)sliplen, timeout);
          if (rc <= 0)
          {
            DEBUG_LOG("** ERROR: hci_phy_send rc=%d !\r\n", rc);
            continue;
          }
#ifndef MX_WIFI_HCI_ACK_ENABLE
          else
          {
            rc = (int16_t)len; // success
            break;
          }
#else
          // wait ack
          tick_start = HAL_GetTick();
          while ((HAL_GetTick() - tick_start) < HCI_RETRY_INTERVAL)
          {
            if (1 == hci_ack_flag)
            {
              rc = len; // real user data len
              hci_send_retry = HCI_RETRY_MAX;  /* break outer retry while */
              break; /* break current */
            }
            else
            {
              mx_wifi_hci_yield(50);
            }
          }

          if (len != rc)
          {
            DEBUG_LOG("** ERROR: hci_phy_send ack timeout !\r\n");
            rc = -1;  // failed
          }
#endif
        }

#if MX_WIFI_USE_CMSIS_OS
        (void)MX_WIFI_HCI_UNLOCK(hci_lock_id);
#endif
        HCI_FREE(slip_buf);
      }
    }
  }
#endif // MX_WIFI_USE_SPI

  return rc;
}

int16_t mx_wifi_hci_recv(uint8_t **p_data, uint32_t timeout)
{
  int16_t rc = 0;
  uint8_t *data_ptr = NULL;
  uint16_t tmp_len;

  uint32_t tickStart = HAL_GetTick();

  if (NULL != p_data)
  {
#if MX_WIFI_USE_CMSIS_OS
    if (MX_WIFI_HCI_LOCK(hci_lock_id, timeout))
    {
#endif
      while ((HAL_GetTick() - tickStart) < timeout)
      {
        tmp_len = 0;
        rc = hci_recv_queue_pop(&hci_msg_queue, &data_ptr, &tmp_len);
        if ((0 == rc) && (tmp_len > (uint16_t)0))
        {
          *p_data = data_ptr;
          rc = (int16_t)tmp_len;
          break;
        }
        else
        {
          mx_wifi_hci_yield(10); // yield if no data
          rc = 0;  // set return data len 0
        }
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_HCI_UNLOCK(hci_lock_id);
    }
#endif
  }
  return rc;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
