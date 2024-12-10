/**
  ******************************************************************************
  * @file    mx_wifi_hci.c
  * @author  MCD Application Team
  * @brief   Host driver HCI protocol of MXCHIP Wi-Fi component.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
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
#include <inttypes.h>

#include "mx_wifi.h"
#include "mx_wifi_conf.h"
#include "core/mx_wifi_hci.h"
#include "io_pattern/mx_wifi_io.h"

#if (MX_WIFI_USE_SPI == 0)
#include "mx_wifi_slip.h"
#endif /* MX_WIFI_USE_SPI */

#ifdef MX_WIFI_HCI_DEBUG
#define DEBUG_LOG(...)       (void)printf(__VA_ARGS__) /*;*/
#else
#define DEBUG_LOG(...)
#endif /* MX_WIFI_HCI_DEBUG */

#define DEBUG_ERROR(...)     (void)printf(__VA_ARGS__) /*;*/

/* Private defines -----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* HCI low level function. */
static hci_send_func_t TclOutputFunc = NULL;

/* HCI receive data queue. */
static FIFO_DECLARE(HciPacketFifo);

static bool mx_wifi_hci_pkt_verify(const uint8_t *data, uint32_t len);


/* Private functions ---------------------------------------------------------*/
static bool mx_wifi_hci_pkt_verify(const uint8_t *data, uint32_t len)
{
  (void)data;
  (void)len;
  /* Here each UART slip frame or SPI frame as a HCI packet. */
  return true;
}


/* Global functions ----------------------------------------------------------*/
int32_t mx_wifi_hci_init(hci_send_func_t low_level_send)
{
  TclOutputFunc = low_level_send;
  FIFO_INIT(HciPacketFifo, MX_WIFI_MAX_RX_BUFFER_COUNT);

  return 0;
}


int32_t mx_wifi_hci_deinit(void)
{
  FIFO_DEINIT(HciPacketFifo);
  return 0;
}


int32_t mx_wifi_hci_send(uint8_t *payload, uint16_t len)
{
  int32_t ret = 0;
  uint16_t sent = 0;

#if (MX_WIFI_USE_SPI == 1)
  sent = TclOutputFunc(payload, len);
  if (len != sent)
  {
    DEBUG_ERROR("tcl_output(spi) error sent=%d !\n", sent);
    ret = -1;
  }
#else
  uint8_t *slip_frame = NULL;
  uint16_t slip_len = 0;

  slip_frame = slip_transfer(payload, len, &slip_len);
  if ((NULL != slip_frame) && (slip_len > 0))
  {
    sent = TclOutputFunc(slip_frame, slip_len);
    if (slip_len == sent)
    {
      ret = 0;
    }
    else
    {
      DEBUG_ERROR("tcl_output(uart) error sent=%d !\n", sent);
      ret = -1;
    }

    MX_WIFI_FREE(slip_frame);
    slip_frame = NULL;

    MX_STAT(free);
  }
  else
  {
    DEBUG_ERROR("Create slip frame error!\n");
    ret = -2;
  }
#endif /* (MX_WIFI_USE_SPI == 1) */

  return ret;
}


mx_buf_t *mx_wifi_hci_recv(uint32_t timeout)
{
  mx_buf_t *const nbuf = (mx_buf_t *)FIFO_POP(HciPacketFifo, timeout, process_txrx_poll);
  if (nbuf != NULL)
  {
#if 0
    const uint32_t len = MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf);
    DEBUG_LOG("\n%s(): %" PRIu32 "\n", __FUNCTION__, len);
#endif /* 0 */

    MX_STAT(out_fifo);
  }

  return nbuf;
}


void mx_wifi_hci_free(mx_buf_t *nbuf)
{
  if (NULL != nbuf)
  {
    (void) MX_NET_BUFFER_FREE(nbuf);

    MX_STAT(free);
  }
}


/**
  * @brief LOW LEVEL INTERFACE
  */
void mx_wifi_hci_input(mx_buf_t *netbuf)
{
  if (NULL != netbuf)
  {
    uint8_t *data = MX_NET_BUFFER_PAYLOAD(netbuf);
    uint32_t len  = MX_NET_BUFFER_GET_PAYLOAD_SIZE(netbuf);

    DEBUG_LOG("\n%s(): %" PRIu32 "\n", __FUNCTION__, len);
#if 0
    for (uint32_t i = 0; i < len; i++)
    {
      DEBUG_LOG("%02" PRIx32 " ", (uint32_t)data[i]);
    }
#endif /* 0 */

    if ((NULL != data) && (len > 0))
    {
      if (mx_wifi_hci_pkt_verify(data, len))
      {
        if (FIFO_OK != FIFO_PUSH(HciPacketFifo, netbuf, WAIT_FOREVER, NULL))
        {
          DEBUG_ERROR("push tcl input queue err!\n");
          MX_NET_BUFFER_FREE(netbuf);

          MX_STAT(free);
        }
        else
        {
          DEBUG_LOG("\nhci input len %" PRIu32 "\n", len);
          MX_STAT(in_fifo);
        }
      }
      else
      {
        DEBUG_LOG("input bad tcl pkt!\n");
        MX_NET_BUFFER_FREE(netbuf);

        MX_STAT(free);
      }
    }
  }
}
