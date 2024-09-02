/**
  ******************************************************************************
  * @file    mx_wifi_ipc.c
  * @author  MCD Application Team
  * @brief   Host driver IPC protocol of MXCHIP Wi-Fi component.
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
#include <string.h>
#include <inttypes.h>

#include "mx_wifi.h"
#include "mx_wifi_ipc.h"
#include "mx_wifi_hci.h"
#include "io_pattern/mx_wifi_io.h"


#ifdef MX_WIFI_IPC_DEBUG
#define DEBUG_LOG(...)       (void)printf(__VA_ARGS__) /*;*/
#else
#define DEBUG_LOG(...)
#endif /* MX_WIFI_IPC_DEBUG */

#define DEBUG_ERROR(...)     (void)printf(__VA_ARGS__) /*;*/


/**
  * @brief IPC API event handlers
  */
typedef void (*event_callback_t)(mx_buf_t *mx_buff);

typedef struct _event_item_s
{
  uint16_t api_id;
  event_callback_t callback;
} event_item_t;


/**
  * @brief IPC API request list
  */
typedef struct _mipc_req_s
{
  uint32_t req_id;
  SEM_DECLARE(resp_flag);
  uint16_t *rbuffer_size; /* in/out */
  uint8_t *rbuffer;
} mipc_req_t;

#define MIPC_REQ_ID_RESET_VAL  ((uint32_t)(0xFFFFFFFF))


static mipc_req_t PendingRequest;

static uint8_t *byte_pointer_add_signed_offset(uint8_t *BytePointer, int32_t Offset);
static uint32_t get_new_req_id(void);
static uint32_t mpic_get_req_id(const uint8_t Buffer[]);
static uint16_t mpic_get_api_id(const uint8_t Buffer[]);
static void mipc_event(mx_buf_t *netbuf);


static uint8_t *byte_pointer_add_signed_offset(uint8_t *BytePointer, int32_t Offset)
{
  return BytePointer + Offset;
}


/* unique sequence number */
static uint32_t get_new_req_id(void)
{
  static uint32_t id = 1;
  return id++;
}


static uint32_t mpic_get_req_id(const uint8_t Buffer[])
{
  return *((const uint32_t *) &Buffer[MIPC_PKT_REQ_ID_OFFSET]);
}


static uint16_t mpic_get_api_id(const uint8_t Buffer[])
{
  return *((const uint16_t *) &Buffer[MIPC_PKT_API_ID_OFFSET]);
}


static void mipc_event(mx_buf_t *netbuf)
{
  static const event_item_t event_table[] =
  {
    /* System */
    {MIPC_API_SYS_REBOOT_EVENT,         mapi_reboot_event_callback},
    {MIPC_API_SYS_FOTA_STATUS_EVENT,    mapi_fota_status_event_callback},

    /* WiFi */
    {MIPC_API_WIFI_STATUS_EVENT,        mapi_wifi_status_event_callback},
    {MIPC_API_WIFI_BYPASS_INPUT_EVENT,  mapi_wifi_netlink_input_callback}
  };

  if (NULL != netbuf)
  {
    uint8_t *const buffer_in = MX_NET_BUFFER_PAYLOAD(netbuf);
    const uint32_t buffer_in_size = MX_NET_BUFFER_GET_PAYLOAD_SIZE(netbuf);

    if ((NULL != buffer_in) && (buffer_in_size >= MIPC_PKT_MIN_SIZE))
    {
      const uint32_t req_id = mpic_get_req_id(buffer_in);
      const uint16_t api_id = mpic_get_api_id(buffer_in);

      DEBUG_LOG("%-15s(): req_id: 0x%08" PRIx32 ", api_id: 0x%04" PRIx32 "\n",
                __FUNCTION__, req_id, (uint32_t)api_id);

      if ((0 == (api_id & MIPC_API_EVENT_BASE)) && (MIPC_REQ_ID_NONE != req_id))
      {
        /* The command response must match pending req id. */
        if (PendingRequest.req_id == req_id)
        {
          /* return params */
          if ((PendingRequest.rbuffer_size != NULL) && (*PendingRequest.rbuffer_size > 0) &&
              (NULL != PendingRequest.rbuffer))
          {
            *(PendingRequest.rbuffer_size) = *PendingRequest.rbuffer_size < (buffer_in_size - MIPC_PKT_MIN_SIZE) ? \
                                             *PendingRequest.rbuffer_size : (uint16_t)(buffer_in_size - MIPC_PKT_MIN_SIZE);
            (void)memcpy(PendingRequest.rbuffer, byte_pointer_add_signed_offset(buffer_in, MIPC_PKT_PARAMS_OFFSET),
                         *PendingRequest.rbuffer_size);
          }
          /* printf("Signal for %d\n",pending_request.req_id); */
          PendingRequest.req_id = MIPC_REQ_ID_RESET_VAL;
          if (SEM_OK != SEM_SIGNAL(PendingRequest.resp_flag))
          {
            DEBUG_ERROR("Failed to signal command response\n");
            MX_ASSERT(false);
          }
          MX_STAT(cmd_get_answer);
        }
        else
        {
          DEBUG_LOG("response req_id: 0x%08"PRIx32" not match pending req_id: 0x%08" PRIx32 "!\n",
                    req_id, PendingRequest.req_id);
        }
        mx_wifi_hci_free(netbuf);
      }
      else /* event callback */
      {
        const uint32_t event_table_count = sizeof(event_table) / sizeof(event_table[0]);
        uint32_t i;

        for (i = 0; i < event_table_count; i++)
        {
          if (event_table[i].api_id == api_id)
          {
            const event_callback_t callback = event_table[i].callback;
            if (NULL != callback)
            {
              /* DEBUG_LOG("callback with %p\n", buffer_in); */
              callback(netbuf);
              break;
            }
          }
        }
        if (i == event_table_count)
        {
          DEBUG_ERROR("Unknown event: 0x%04" PRIx32 "!\n", (uint32_t)api_id);
          mx_wifi_hci_free(netbuf);
        }
      }
    }
    else
    {
      DEBUG_LOG("Unknown buffer content\n");
      mx_wifi_hci_free(netbuf);
    }
  }
}


/*******************************************************************************
  * IPC API implementations for mx_wifi over HCI
  ******************************************************************************/

int32_t mipc_init(mipc_send_func_t ipc_send)
{
  int32_t ret;

  PendingRequest.req_id = MIPC_REQ_ID_RESET_VAL;
  SEM_INIT(PendingRequest.resp_flag, 1);

  ret = mx_wifi_hci_init(ipc_send);

  return ret;
}


int32_t mipc_deinit(void)
{
  int32_t ret;

  SEM_DEINIT(PendingRequest.resp_flag);

  ret = mx_wifi_hci_deinit();

  return ret;
}


int32_t mipc_request(uint16_t api_id,
                     uint8_t *cparams, uint16_t cparams_size,
                     uint8_t *rbuffer, uint16_t *rbuffer_size,
                     uint32_t timeout_ms)
{
  int32_t ret = MIPC_CODE_ERROR;
  uint8_t *cbuf;
  bool copy_buffer = true;

  LOCK(wifi_obj_get()->lockcmd);

  /* DEBUG_LOG("\n%s()>  %" PRIu32 "\n", __FUNCTION__, (uint32_t)cparams_size); */

  if (cparams_size <= MX_WIFI_IPC_PAYLOAD_SIZE)
  {
    /* Create the command data. */
    const uint16_t cbuf_size = MIPC_PKT_REQ_ID_SIZE + MIPC_PKT_API_ID_SIZE + cparams_size;

#if MX_WIFI_TX_BUFFER_NO_COPY
    if (api_id == MIPC_API_WIFI_BYPASS_OUT_CMD)
    {
      cbuf = byte_pointer_add_signed_offset(cparams, - (MIPC_PKT_REQ_ID_SIZE + MIPC_PKT_API_ID_SIZE));
      copy_buffer = false;
    }
    else
#endif /* MX_WIFI_TX_BUFFER_NO_COPY */
    {
      DEBUG_LOG("\n%-15s(): Allocate %" PRIu32 " bytes\n", __FUNCTION__, (uint32_t)cbuf_size);

      cbuf = (uint8_t *)MX_WIFI_MALLOC(cbuf_size);

      MX_STAT(alloc);
    }

    if (NULL != cbuf)
    {
      /* Get an unique identifier. */
      const uint32_t req_id = get_new_req_id();

      /* Copy the protocol parameter to the head part of the buffer. */
      (void)memcpy(byte_pointer_add_signed_offset(cbuf, MIPC_PKT_REQ_ID_OFFSET), &req_id, sizeof(req_id));
      (void)memcpy(byte_pointer_add_signed_offset(cbuf, MIPC_PKT_API_ID_OFFSET), &api_id, sizeof(api_id));

      if ((true == copy_buffer) && (cparams_size > 0))
      {
        (void)memcpy(byte_pointer_add_signed_offset(cbuf, MIPC_PKT_PARAMS_OFFSET), cparams, cparams_size);
      }

      /* A single pending request due to LOCK usage on command. */
      if (PendingRequest.req_id != MIPC_REQ_ID_RESET_VAL)
      {
        DEBUG_LOG("Error req_id must be 0xffffffff here %" PRIu32 "\n", PendingRequest.req_id);
        MX_ASSERT(false);
      }

      PendingRequest.req_id = req_id;
      PendingRequest.rbuffer = rbuffer;
      PendingRequest.rbuffer_size = rbuffer_size;

      /* static int iter=0;                       */
      /* printf("%d push %d\n",iter++,cbuf_size); */

      /* Send the command. */
      DEBUG_LOG("%-15s(): req_id: 0x%08" PRIx32 " : %" PRIu32 "\n", __FUNCTION__, req_id, (uint32_t)cbuf_size);

      ret = mx_wifi_hci_send(cbuf, cbuf_size);
      if (ret == 0)
      {
        /* Wait for the command answer. */
        if (SEM_WAIT(PendingRequest.resp_flag, timeout_ms, mipc_poll) != SEM_OK)
        {
          DEBUG_ERROR("Error: command 0x%04" PRIx32 " timeout(%" PRIu32 " ms) waiting answer %" PRIu32 "\n",
                      (uint32_t)api_id, timeout_ms, PendingRequest.req_id);
          PendingRequest.req_id = MIPC_REQ_ID_RESET_VAL;
          ret = MIPC_CODE_ERROR;
        }
      }
      else
      {
        DEBUG_ERROR("Failed to send command to HCI\n");
        MX_ASSERT(false);
      }

      DEBUG_LOG("%-15s()< req_id: 0x%08" PRIx32 " done (%" PRId32 ")\n\n", __FUNCTION__, req_id, ret);

      if (true == copy_buffer)
      {
        MX_WIFI_FREE(cbuf);

        MX_STAT(free);
      }
    }
  }

  UNLOCK(wifi_obj_get()->lockcmd);

  return ret;
}


void mipc_poll(uint32_t timeout)
{
  mx_buf_t *nbuf;

  /* Process the received data inside the RX buffer. */
  nbuf = mx_wifi_hci_recv(timeout);

  if (NULL != nbuf)
  {
    const uint32_t len = MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf);

    DEBUG_LOG("%-15s(): %p HCI recv len %" PRIu32 "\n", __FUNCTION__, nbuf, len);

    if (len > 0U)
    {
      mipc_event(nbuf);
    }
    else
    {
      MX_NET_BUFFER_FREE(nbuf);

      MX_STAT(free);
    }
  }
}


int32_t mipc_echo(uint8_t *in, uint16_t in_len, uint8_t *out, uint16_t *out_len,
                  uint32_t timeout)
{
  int32_t ret = MIPC_CODE_ERROR;

  if ((NULL != in) && (NULL != out) && (NULL != out_len))
  {
    ret = mipc_request(MIPC_API_SYS_ECHO_CMD, in, in_len, out, out_len, timeout);
    if (MIPC_CODE_SUCCESS != ret)
    {
      *out_len = 0;
    }
  }
  return ret;
}


/*******************************************************************************
  * IPC API event callbacks
  ******************************************************************************/

/* System */

void mapi_reboot_event_callback(mx_buf_t *mxbuff)
{
  if (mxbuff != NULL)
  {
    mx_wifi_hci_free(mxbuff);
  }
  DEBUG_LOG("\nEVENT: reboot done.\n");
}


void mapi_fota_status_event_callback(mx_buf_t *nbuf)
{
  if (NULL != nbuf)
  {
    uint8_t *payload = MX_NET_BUFFER_PAYLOAD(nbuf);
    if (NULL != payload)
    {
      mx_wifi_fota_status_e status = *((mx_wifi_fota_status_e *)(byte_pointer_add_signed_offset(payload, MIPC_PKT_PARAMS_OFFSET)));

      DEBUG_LOG("\nEVENT: FOTA status: %02x\n", status);

      mx_wifi_hci_free(nbuf);

      {
        mx_wifi_fota_status_cb_t const status_cb = wifi_obj_get()->Runtime.fota_status_cb;
        const uint32_t cb_args = wifi_obj_get()->Runtime.fota_user_args;
        if (NULL != status_cb)
        {
          status_cb(status, cb_args);
        }
      }
    }
  }
}


/* WiFi */

void mapi_wifi_status_event_callback(mx_buf_t *netbuf)
{
  uint8_t cate;
  mx_wifi_status_callback_t status_cb = NULL;
  void *cb_args = NULL;

  if (NULL != netbuf)
  {
    uint8_t *payload = MX_NET_BUFFER_PAYLOAD(netbuf);
    mwifi_status_t status = *((mwifi_status_t *)(byte_pointer_add_signed_offset(payload, MIPC_PKT_PARAMS_OFFSET)));

    DEBUG_LOG("\nEVENT: wifi status: %02x\n", status);

    mx_wifi_hci_free(netbuf);

    switch (status)
    {
      case MWIFI_EVENT_STA_UP:
      case MWIFI_EVENT_STA_DOWN:
      case MWIFI_EVENT_STA_GOT_IP:
        cate = MC_STATION;
        status_cb = wifi_obj_get()->Runtime.status_cb[0];
        cb_args = wifi_obj_get()->Runtime.callback_arg[0];
        break;

      case MWIFI_EVENT_AP_UP:
      case MWIFI_EVENT_AP_DOWN:
        cate = MC_SOFTAP;
        status_cb = wifi_obj_get()->Runtime.status_cb[1];
        cb_args = wifi_obj_get()->Runtime.callback_arg[1];
        break;

      default:
        cate = MC_SOFTAP;
        MX_ASSERT(false);
        /* break; */
    }

    if (NULL != status_cb)
    {
      status_cb(cate, status, cb_args);
    }
  }
}


void mapi_wifi_netlink_input_callback(mx_buf_t *netbuf)
{
  if (NULL != netbuf)
  {
    uint8_t *const buffer_in = MX_NET_BUFFER_PAYLOAD(netbuf);
    wifi_bypass_in_rparams_t *const in_rprarams = (wifi_bypass_in_rparams_t *)(byte_pointer_add_signed_offset(buffer_in, MIPC_PKT_PARAMS_OFFSET));

    MX_STAT(callback);

    if ((NULL != wifi_obj_get()->Runtime.netlink_input_cb) && \
        (in_rprarams->tot_len > 0))
    {
      uint32_t low_level_netif_idx = (uint32_t)in_rprarams->idx;

      MX_NET_BUFFER_HIDE_HEADER(netbuf, MIPC_PKT_PARAMS_OFFSET + sizeof(wifi_bypass_in_rparams_t));
      wifi_obj_get()->Runtime.netlink_input_cb(netbuf, (void *)&low_level_netif_idx);
    }
    else
    {
      MX_NET_BUFFER_FREE(netbuf);

      MX_STAT(free);
    }
  }
}
