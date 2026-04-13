/**
  ******************************************************************************
  * @file    mx_wifi_slip.c
  * @author  MCD Application Team
  * @brief   Host driver SLIP protocol of MXCHIP Wi-Fi component.
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

#ifdef MX_WIFI_SLIP_DEBUG
#define DEBUG_LOG(M, ...)  printf(M, ##__VA_ARGS__);
#else
#define DEBUG_LOG(M, ...)
#endif

#define SLIP_BUF_NUM    (2)

enum
{
  SLIP_STATE_IDLE,     // NOT START
  SLIP_STATE_CONTINUE, // receive data to buffer
  SLIP_STATE_GOT_ESCAPE, // last byte is escape
};


typedef struct
{
  uint8_t *data;
  int32_t      len;
} slip_msg_t;

static uint8_t *slip_buffer = NULL, *slip_buf_header, buf_busy[SLIP_BUF_NUM];
static int32_t slip_index = 0;


static void slip_buf_get(void)
{
  int32_t i;

  slip_buffer = NULL;

  for (i = 0; i < SLIP_BUF_NUM; i++)
  {
    if (buf_busy[i] == (uint8_t)0)
    {
      slip_buffer = &(slip_buf_header[(SLIP_BUFFER_SIZE) * i]);
      buf_busy[i] = 1;
      break;
    }
  }
}

static void slip_buf_free(uint8_t *buf)
{
  int32_t index, addr_pbuf, addr_header;
  (void)memcpy(&addr_pbuf, &buf, sizeof(addr_pbuf));
  (void)memcpy(&addr_header, &slip_buf_header, sizeof(addr_header));
  index = (addr_pbuf - addr_header);

  index /= (SLIP_BUFFER_SIZE);
  if ((index < 0) || (index >= SLIP_BUF_NUM))
  {
    DEBUG_LOG("** ERROR: slip invlid buf %p, not in %p !\r\n", buf, slip_buf_header);
  }
  else
  {
    buf_busy[index] = 0;
  }
}

uint8_t *slip_transfer(uint8_t *data, int32_t len, int32_t *outlen)
{
  int32_t i, inc = 2, j;
  uint8_t *buff = NULL;
  void *pbuf;

  for (i = 0; i < len; i++)
  {
    if ((data[i] == (uint8_t)SLIP_START) || (data[i] == (uint8_t)SLIP_END)
        || (data[i] == (uint8_t)SLIP_ESCAPE))
    {
      inc++;
    }
  }

  pbuf = HCI_MALLOC((size_t)len + (size_t)inc);
  if (NULL != pbuf)
  {
    (void)memcpy(&buff, &pbuf, sizeof(buff));
    buff[0] = (uint8_t)SLIP_START;
    j = 1;
    for (i = 0; i < len; i++)
    {
      if (data[i] == (uint8_t)SLIP_START)
      {
        buff[j] = (uint8_t)SLIP_ESCAPE;
        j++;
        buff[j] = (uint8_t)SLIP_ESCAPE_START;
        j++;
      }
      else if (data[i] == (uint8_t)SLIP_END)
      {
        buff[j] = (uint8_t)SLIP_ESCAPE;
        j++;
        buff[j] = (uint8_t)SLIP_ESCAPE_END;
        j++;
      }
      else if (data[i] == (uint8_t)SLIP_ESCAPE)
      {
        buff[j] = (uint8_t)SLIP_ESCAPE;
        j++;
        buff[j] = (uint8_t)SLIP_ESCAPE_ES;
        j++;
      }
      else
      {
        buff[j] = data[i];
        j++;
      }
    }
    buff[j] = (uint8_t)SLIP_END;
    j++;
    *outlen = j;
  }

  return buff;
}

#ifdef MX_WIFI_HCI_ACK_ENABLE
static void slip_ack(uint8_t seq)
{
  uint8_t ack[8], *slipbuf;
  int32_t i = 0, sliplen;
  uint16_t crc16 = 0xFFFF;
#ifdef USE_STM32L_CRC  // STM32L4 CRC
  CRC_HandleTypeDef crc;
#else
  CRC16_Context crc; // software crc
#endif

  ack[i] = 1;
  i++;
  ack[i] = seq;
  i++;
#ifdef USE_STM32L_CRC
  HW_CRC16_Init(&crc);
  HW_CRC16_Update(&crc, ack, i, &crc16);
  crc16 = htons(crc16);
#else  // software crc
  CRC16_Init(&crc);
  CRC16_Update(&crc, ack, i);
  CRC16_Final(&crc, &crc16);
  crc16 = htons(crc16);
#endif

  memcpy(&ack[i], &crc16, 2);
  i += 2;
  slipbuf = slip_transfer(ack, i, &sliplen);
  if (slipbuf != NULL)
  {
    hci_phy_send(slipbuf, sliplen, 200);
    HCI_FREE(slipbuf);
  }
}
#endif

/* Create slip buffer, queue and receive thread */
int32_t slip_init(void)
{
  int32_t i, ret = 0;
  void *pbuf;

  slip_index = 0;
  slip_buffer = NULL;
  pbuf = HCI_MALLOC(SLIP_BUFFER_SIZE * SLIP_BUF_NUM);
  (void)memcpy(&slip_buf_header, &pbuf, sizeof(slip_buf_header));
  if (slip_buf_header == NULL)
  {
    DEBUG_LOG("** ERROR: slip init: No memory !\r\n");
    ret = -1;
  }
  else
  {
    for (i = 0; i < SLIP_BUF_NUM; i++)
    {
      buf_busy[i] = 0;
    }
  }
  return ret;
}

int32_t slip_deinit(void)
{
  if (NULL != slip_buf_header)
  {
    HCI_FREE(slip_buf_header);
    slip_buf_header = NULL;
  }
  return 0;
}

/* send frame to slip thread. */
static int32_t slip_frame_input(uint8_t *data, int32_t len)
{
  static uint8_t slip_ack_num = 0;
  int32_t ret = 0;
#ifdef MX_WIFI_SLIP_DEBUG
  int32_t i = 0;
#endif

#ifdef MX_WIFI_HCI_ACK_ENABLE
  uint16_t crc16 = 0xFFFF;
#ifdef USE_STM32L_CRC
  CRC_HandleTypeDef crc;  // STM32L4 CRC
#else
  CRC16_Context crc;  // software crc
#endif
#endif

  if (len < 4)
  {
    DEBUG_LOG("** ERROR: slip frame too short !\r\n");
    slip_buf_free(data);
    ret = -1;
  }
  else
  {

#ifdef MX_WIFI_HCI_ACK_ENABLE
#ifdef USE_STM32L_CRC
    HW_CRC16_Init(&crc);
    HW_CRC16_Update(&crc, data, len - 2, &crc16);
    crc16 = htons(crc16);
#else  // software crc
    CRC16_Init(&crc);
    CRC16_Update(&crc, data, len - 2);
    CRC16_Final(&crc, &crc16);
    crc16 = htons(crc16);
#endif
    if (memcmp(&crc16, &data[len - 2], 2) != 0)
    {
      DEBUG_LOG("** ERROR: slip CRC mismatch %04x:%02x%02x !\r\n", crc16, data[len - 1], data[len - 2]);
      slip_buf_free(data);
      hci_event_notify(HCI_EVENT_ERROR_CRC_ERROR);
      ret = -1;
    }
    else
    {
#endif // MX_WIFI_HCI_ACK_ENABLE

#ifdef MX_WIFI_SLIP_DEBUG
      DEBUG_LOG("RX SLIP got %d bytes:\r\n", len);
      for (i = 0; i < len; i++)
      {
        DEBUG_LOG("%02x ", data[i]);
      }
      DEBUG_LOG("\r\n");
#endif

      if (data[0] == (uint8_t)SLIP_DATA)
      {
        if (slip_ack_num == (data[1] + (uint8_t)1))
        {
          hci_event_notify((uint8_t)HCI_EVENT_ERROR_REPEAT_FRAME);
          DEBUG_LOG("** ERROR: duplicate HCI frame, seq %d !\r\n", slip_ack_num);
        }
        else if (slip_ack_num != data[1])
        {
          DEBUG_LOG("** ERROR: slip out of sequence HCI frame, seq %d, ack %d !\r\n", data[1], slip_ack_num);
          hci_event_notify((uint8_t)HCI_EVENT_ERROR_OUT_OF_SEQ);
        }
        else
        {
          // correct ack seq
        }
#ifdef MX_WIFI_HCI_ACK_ENABLE
        slip_ack(data[1]); // always ack
#endif
        if (slip_ack_num != (data[1] + (uint8_t)1))  // ignore duplicate msg
        {
          (void)hci_data_process(&data[2], (uint16_t)len - (uint16_t)4);
        }
        slip_ack_num = (data[1] + (uint8_t)1);
      }
#ifdef MX_WIFI_HCI_ACK_ENABLE
      else if (data[0] == SLIP_ACK)
      {
        hci_ack_process(data[1]);
      }
#endif
      else
      {
        DEBUG_LOG("** ERROR: slip unsupport msg type %02x !\r\n", data[0]);
      }
      slip_buf_free(data);
#ifdef MX_WIFI_HCI_ACK_ENABLE
    }
#endif /* MX_WIFI_HCI_ACK_ENABLE */
  }

  return ret;
}

int32_t slip_input_byte(uint8_t data)
{
  static int32_t slip_state = SLIP_STATE_IDLE;
  int32_t ret = 0;

  /* get buffer */
  if (slip_buffer == NULL)
  {
    slip_buf_get();
  }

  /* buffer get failed */
  if (slip_buffer == NULL)
  {
    DEBUG_LOG("** ERROR: slip No buf !\r\n");
  }
  else
  {
    if (slip_index >= (SLIP_BUFFER_SIZE))
    {
      slip_index = 0;
      slip_state = (int32_t)SLIP_STATE_IDLE;
    }

    switch (slip_state)
    {
      case SLIP_STATE_GOT_ESCAPE:
        if (data == (uint8_t)SLIP_START)
        {
          slip_index = 0;
        }
        else if (data == (uint8_t)SLIP_ESCAPE_START)
        {
          slip_buffer[slip_index] = (uint8_t)SLIP_START;
          slip_index++;
        }
        else if (data == (uint8_t)SLIP_ESCAPE_ES)
        {
          slip_buffer[slip_index] = (uint8_t)SLIP_ESCAPE;
          slip_index++;
        }
        else if (data == (uint8_t)SLIP_ESCAPE_END)
        {
          slip_buffer[slip_index] = (uint8_t)SLIP_END;
          slip_index++;
        }
        else
        {
          ret = -1;
        }

        if ((int32_t)0 == ret)
        {
          slip_state = (int32_t)SLIP_STATE_CONTINUE;
        }
        break;

      case SLIP_STATE_IDLE:
        if (data == (uint8_t)SLIP_START)
        {
          slip_index = 0;
          slip_state = (int32_t)SLIP_STATE_CONTINUE;
        }
        break;

      case SLIP_STATE_CONTINUE: // continue
        if (data == (uint8_t)SLIP_START)
        {
          slip_index = 0;
          slip_state = (int32_t)SLIP_STATE_CONTINUE;
        }
        else if (data == (uint8_t)SLIP_END)
        {
          (void)slip_frame_input(slip_buffer, slip_index);
          ret = -1;
        }
        else if (data == (uint8_t)SLIP_ESCAPE)
        {
          slip_state = (int32_t)SLIP_STATE_GOT_ESCAPE;
        }
        else
        {
          slip_buffer[slip_index] = data;
          slip_index++;
        }
        break;

      default:
        break;
    }

    if ((int32_t)0 != ret)
    {
      slip_index = 0;
      slip_state = (int32_t)SLIP_STATE_IDLE;
    }
  }

  return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
