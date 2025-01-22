/**
  ******************************************************************************
  * @file    mbox_ipcc.c
  * @author  MCD Application Team
  * @brief   This file provides code for the configuration
  *                      of the mailbox_ipcc_if.c MiddleWare.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics. 
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the 
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/*
 * Channel direction and usage:
 *
 *  ========   <-- new msg ---=============--------<------   =======
 * ||      ||                || CHANNEL 1 ||                ||     ||
 * ||  A7  ||  ------->-------=============--- buf free-->  || M4  ||
 * ||      ||                                               ||     ||
 * ||master||  <-- buf free---=============--------<------  ||slave||
 * ||      ||                || CHANNEL 2 ||                ||     ||
 *  ========   ------->-------=============----new msg -->   =======
 */

/* Includes ------------------------------------------------------------------*/
#include "openamp/open_amp.h"
#include "stm32mp1xx_hal.h"
#include "openamp_conf.h"

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

#define MASTER_CPU_ID       0
#define REMOTE_CPU_ID       1
#define IPCC_CPU_A7         MASTER_CPU_ID
#define IPCC_CPU_M4         REMOTE_CPU_ID

#define RX_NO_MSG           0
#define RX_NEW_MSG          1
#define RX_BUF_FREE         2

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

extern IPCC_HandleTypeDef hipcc;
int msg_received_ch1 = RX_NO_MSG;
int msg_received_ch2 = RX_NO_MSG;
uint32_t vring0_id = 0; /* used for channel 1 */
uint32_t vring1_id = 1; /* used for channel 2 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

void IPCC_channel1_callback(IPCC_HandleTypeDef * hipcc, uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir);
void IPCC_channel2_callback(IPCC_HandleTypeDef * hipcc, uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir);


/**
  * @brief  Initialize MAILBOX with IPCC peripheral
  * @param  None
  * @retval : Operation result
  */
int MAILBOX_Init(void)
{

   /* USER CODE BEGIN PRE_MAILBOX_INIT */

   /* USER CODE END  PRE_MAILBOX_INIT */

  if (HAL_IPCC_ActivateNotification(&hipcc, IPCC_CHANNEL_1, IPCC_CHANNEL_DIR_RX,
          IPCC_channel1_callback) != HAL_OK) {
	  OPENAMP_log_err("%s: ch_1 RX fail\n", __func__);
    return -1;
  }

  if (HAL_IPCC_ActivateNotification(&hipcc, IPCC_CHANNEL_2, IPCC_CHANNEL_DIR_RX,
          IPCC_channel2_callback) != HAL_OK) {
	  OPENAMP_log_err("%s: ch_2 RX fail\n", __func__);
    return -1;
  }


  /* USER CODE BEGIN POST_MAILBOX_INIT */

  /* USER CODE END  POST_MAILBOX_INIT */
  return 0;
}

/**
  * @brief  Initialize MAILBOX with IPCC peripheral
  * @param  virtio device
  * @retval : Operation result
  */
int MAILBOX_Poll(struct virtio_device *vdev)
{
  /* If we got an interrupt, ask for the corresponding virtqueue processing */
  int ret = -1;

   /* USER CODE BEGIN PRE_MAILBOX_POLL */

   /* USER CODE END  PRE_MAILBOX_POLL */

  if (msg_received_ch1 == RX_BUF_FREE) {

   /* USER CODE BEGIN MSG_CHANNEL1 */

   /* USER CODE END  MSG_CHANNEL1 */

    OPENAMP_log_dbg("Running virt0 (ch_1 buf free)\r\n");
    rproc_virtio_notified(vdev, VRING0_ID);
    msg_received_ch1 = RX_NO_MSG;
    ret = 0;
  }

  if (msg_received_ch2 == RX_NEW_MSG) {

   /* USER CODE BEGIN MSG_CHANNEL2 */

   /* USER CODE END  MSG_CHANNEL2 */

    OPENAMP_log_dbg("Running virt1 (ch_2 new msg)\r\n");
    rproc_virtio_notified(vdev, VRING1_ID);
    msg_received_ch2 = RX_NO_MSG;

    /* The OpenAMP framework does not notify for free buf: do it here */
      rproc_virtio_notified(NULL, VRING1_ID);
    ret = 0;
  }

  /* USER CODE BEGIN POST_MAILBOX_POLL */

  /* USER CODE END  POST_MAILBOX_POLL */

  return ret;
}


/**
  * @brief  Callback function called by OpenAMP MW to notify message processing
  * @param  VRING id
  * @retval Operation result
  */
int MAILBOX_Notify(void *priv, uint32_t id)
{
  uint32_t channel;
  (void)priv;

   /* USER CODE BEGIN PRE_MAILBOX_NOTIFY */

   /* USER CODE END  PRE_MAILBOX_NOTIFY */

  /* Called after virtqueue processing: time to inform the remote */
  if (id == VRING0_ID) {
    channel = IPCC_CHANNEL_1;
    OPENAMP_log_dbg("Send msg on ch_1\r\n");
  }
  else if (id == VRING1_ID) {
    /* Note: the OpenAMP framework never notifies this */
    channel = IPCC_CHANNEL_2;
    OPENAMP_log_dbg("Send 'buff free' on ch_2\r\n");
  }
  else {
    OPENAMP_log_err("invalid vring (%d)\r\n", (int)id);
    return -1;
  }

  /* Check that the channel is free (otherwise wait until it is) */
  if (HAL_IPCC_GetChannelStatus(&hipcc, channel, IPCC_CHANNEL_DIR_TX) == IPCC_CHANNEL_STATUS_OCCUPIED) {
    OPENAMP_log_dbg("Waiting for channel to be freed\r\n");
    while (HAL_IPCC_GetChannelStatus(&hipcc, channel, IPCC_CHANNEL_DIR_TX) == IPCC_CHANNEL_STATUS_OCCUPIED)
      ;
  }

  /* Inform A7 (either new message, or buf free) */
  HAL_IPCC_NotifyCPU(&hipcc, channel, IPCC_CHANNEL_DIR_TX);

 /* USER CODE BEGIN POST_MAILBOX_NOTIFY */

 /* USER CODE END  POST_MAILBOX_NOTIFY */

  return 0;
}

/* Private function  ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/* Callback from IPCC Interrupt Handler: Master Processor informs that there are some free buffers */
void IPCC_channel1_callback(IPCC_HandleTypeDef * hipcc,
         uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir)
{

  /* USER CODE BEGIN PRE_MAILBOX_CHANNEL1_CALLBACK */

  /* USER CODE END  PRE_MAILBOX_CHANNEL1_CALLBACK */

  if (msg_received_ch1 != RX_NO_MSG)
    OPENAMP_log_dbg("IPCC_channel1_callback: previous IRQ not treated (status = %d)\r\n", msg_received_ch1);

  msg_received_ch1 = RX_BUF_FREE;

  /* Inform A7 that we have received the 'buff free' msg */
  OPENAMP_log_dbg("Ack 'buff free' message on ch1\r\n");
  HAL_IPCC_NotifyCPU(hipcc, ChannelIndex, IPCC_CHANNEL_DIR_RX);

  /* USER CODE BEGIN POST_MAILBOX_CHANNEL1_CALLBACK */

  /* USER CODE END  POST_MAILBOX_CHANNEL1_CALLBACK */
}

/* Callback from IPCC Interrupt Handler: new message received from Master Processor */
void IPCC_channel2_callback(IPCC_HandleTypeDef * hipcc,
         uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir)
{

  /* USER CODE BEGIN PRE_MAILBOX_CHANNEL2_CALLBACK */

  /* USER CODE END  PRE_MAILBOX_CHANNEL2_CALLBACK */

  if (msg_received_ch2 != RX_NO_MSG)
    OPENAMP_log_dbg("IPCC_channel2_callback: previous IRQ not treated (status = %d)\r\n", msg_received_ch2);

  msg_received_ch2 = RX_NEW_MSG;

  /* Inform A7 that we have received the new msg */
  OPENAMP_log_dbg("Ack new message on ch2\r\n");
  HAL_IPCC_NotifyCPU(hipcc, ChannelIndex, IPCC_CHANNEL_DIR_RX);

  /* USER CODE BEGIN POST_MAILBOX_CHANNEL2_CALLBACK */

  /* USER CODE END  POST_MAILBOX_CHANNEL2_CALLBACK */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
