#include "common/log.h"
#include "common/time.h"
#include <stdint.h>
#include "libcanard/canard_stm32_driver.h"
#include "libcanard/canard.h"
#include "dronecan.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "dronecan_msgs.h"
#include "nodesettings.h"

/* Private variables ---------------------------------------------------------*/

FDCAN_HandleTypeDef hfdcan1;

CanardInstance canard;
uint8_t memory_pool[1024];
static struct uavcan_protocol_NodeStatus node_status;
//static void MX_ICACHE_Init(void);
static void MX_FDCAN1_Init(void);
void Error_Handler(void);

/* void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
	// Receiving
	CanardCANFrame rx_frame;

	const uint64_t timestamp = HAL_GetTick() * 1000ULL;
	const int16_t rx_res = canardSTM32Recieve(hfdcan, FDCAN_RX_FIFO0, &rx_frame);

	if (rx_res < 0) {
		printf("Receive error %d\n", rx_res);
	}
	else if (rx_res > 0)        // Success - process the frame
	{
		canardHandleRxFrame(&canard, &rx_frame, timestamp);
	}
} */

// NOTE: All canard handlers and senders are based on this reference: https://dronecan.github.io/Specification/7._List_of_standard_data_types/
// Alternatively, you can look at the corresponding generated header file in the dsdlc_generated folder

// Canard Handlers ( Many have code copied from libcanard esc_node example: https://github.com/dronecan/libcanard/blob/master/examples/ESCNode/esc_node.c )

void handle_NodeStatus(CanardInstance *ins, CanardRxTransfer *transfer) {
	struct uavcan_protocol_NodeStatus nodeStatus;

	if (uavcan_protocol_NodeStatus_decode(transfer, &nodeStatus)) {
		return;
	}

	printf("Node health: %ud Node Mode: %ud\n", nodeStatus.health, nodeStatus.mode);

	printf("Node Health ");

	switch (nodeStatus.health) {
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK:
		printf("OK\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING:
		printf("WARNING\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR:
		printf("ERROR\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL:
		printf("CRITICAL\n");
		break;
	default:
		printf("UNKNOWN?\n");
		break;
	}

	printf("Node Mode ");

	switch(nodeStatus.mode) {
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL:
		printf("OPERATIONAL\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION:
		printf("INITIALIZATION\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_MAINTENANCE:
		printf("MAINTENANCE\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_SOFTWARE_UPDATE:
		printf("SOFTWARE UPDATE\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_OFFLINE:
		printf("OFFLINE\n");
		break;
	default:
		printf("UNKNOWN?\n");
		break;
	}
}
/*
void handle_NotifyState(CanardInstance *ins, CanardRxTransfer *transfer) {
	struct ardupilot_indication_NotifyState notifyState;

	if (ardupilot_indication_NotifyState_decode(transfer, &notifyState)) {
		return;
	}

	uint32_t nl = notifyState.vehicle_state & 0xFFFFFFFF;  // ignoring the last 32 bits for printing since the highest vehicle_state value right now is 23 even though they're allowed to be up to 64bit unsigned integer

	printf("Vehicle State: %lu ", nl);

	if (notifyState.aux_data.len > 0) {
		printf("Aux Data: 0x");

		for (int i = 0; i < notifyState.aux_data.len; i++) {
			printf("%02x", notifyState.aux_data.data[i]);
		}
	}

	printf("\n");

}
*/
/*
  handle a ESC RawCommand request
*/
/*
void handle_RawCommand(CanardInstance *ins, CanardRxTransfer *transfer)
{
    struct uavcan_equipment_esc_RawCommand rawCommand;
    if (uavcan_equipment_esc_RawCommand_decode(transfer, &rawCommand)) {
        return;
    }
    // see if it is for us
    if (rawCommand.cmd.len <= ESC_INDEX) {
        return;
    }
    // convert throttle to -1.0 to 1.0 range
//    printf("Throttle: %f \n", rawCommand.cmd.data[ESC_INDEX]/8192.0);
}
*/
/*
  get a 16 byte unique ID for this node, this should be based on the CPU unique ID or other unique ID
 */
void getUniqueID(uint8_t id[16]) {
    uint32_t HALUniqueIDs[3];
    // Make Unique ID out of the 96-bit STM32 UID and fill the rest with 0s
    memset(id, 0, 16);
    HALUniqueIDs[0] = HAL_GetUIDw0();
    HALUniqueIDs[1] = HAL_GetUIDw1();
    HALUniqueIDs[2] = HAL_GetUIDw2();
    memcpy(id, HALUniqueIDs, 12);
}

/*
  handle a GetNodeInfo request
*/

// TODO: All the data in here is temporary for testing. If actually need to send valid data, edit accordingly.
void handle_GetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer) {
	printf("GetNodeInfo request from %d\n", transfer->source_node_id);

	uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];
	struct uavcan_protocol_GetNodeInfoResponse pkt;

	memset(&pkt, 0, sizeof(pkt));

	node_status.uptime_sec = HAL_GetTick() / 1000ULL;
	pkt.status = node_status;

	// fill in your major and minor firmware version
	pkt.software_version.major = 1;
	pkt.software_version.minor = 0;
	pkt.software_version.optional_field_flags = 0;
	pkt.software_version.vcs_commit = 0; // should put git hash in here

	// should fill in hardware version
	pkt.hardware_version.major = 1;
	pkt.hardware_version.minor = 0;

	// just setting all 16 bytes to 1 for testing
	getUniqueID(pkt.hardware_version.unique_id);

	strncpy((char*)pkt.name.data, "ESCNode", sizeof(pkt.name.data));
	pkt.name.len = strnlen((char*)pkt.name.data, sizeof(pkt.name.data));

	uint16_t total_size = uavcan_protocol_GetNodeInfoResponse_encode(&pkt, buffer);

	canardRequestOrRespond(ins,
						   transfer->source_node_id,
						   UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,
						   UAVCAN_PROTOCOL_GETNODEINFO_ID,
						   &transfer->transfer_id,
						   transfer->priority,
						   CanardResponse,
						   &buffer[0],
						   total_size);
}

// Canard Senders

/*
  send the 1Hz NodeStatus message. This is what allows a node to show
  up in the DroneCAN GUI tool and in the flight controller logs
 */
void send_NodeStatus(void) {
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];

    LOG_DEBUG(SYSTEM, "Sending Node Status");
    node_status.uptime_sec = HAL_GetTick() / 1000UL;
    node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
    node_status.sub_mode = 0;

    // put whatever you like in here for display in GUI
    node_status.vendor_specific_status_code = 1234;

    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer);

    // we need a static variable for the transfer ID. This is
    // incremeneted on each transfer, allowing for detection of packet
    // loss
    static uint8_t transfer_id;

    canardBroadcast(&canard,
                    UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                    UAVCAN_PROTOCOL_NODESTATUS_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
}

// Canard Util

bool shouldAcceptTransfer(const CanardInstance *ins,
                                 uint64_t *out_data_type_signature,
                                 uint16_t data_type_id,
                                 CanardTransferType transfer_type,
                                 uint8_t source_node_id)
{
	if (transfer_type == CanardTransferTypeRequest) {
	// check if we want to handle a specific service request
		switch (data_type_id) {
		case UAVCAN_PROTOCOL_GETNODEINFO_ID: {
			*out_data_type_signature = UAVCAN_PROTOCOL_GETNODEINFO_REQUEST_SIGNATURE;
			return true;
		}
		}
	}
	if (transfer_type == CanardTransferTypeResponse) {
		// check if we want to handle a specific service request
		switch (data_type_id) {
		}
	}
	if (transfer_type == CanardTransferTypeBroadcast) {
		// see if we want to handle a specific broadcast packet
		switch (data_type_id) {
		
		case UAVCAN_PROTOCOL_NODESTATUS_ID: {
			*out_data_type_signature = UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE;
			return true;
		}
		
		}
	}
	// we don't want any other messages
	return false;
}


void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer) {
	// switch on data type ID to pass to the right handler function
    LOG_DEBUG(SYSTEM, "Transfer type: %du, Transfer ID: %du \n", transfer->transfer_type, transfer->data_type_id);
	LOG_DEBUG(SYSTEM, "0x");
    LOG_BUFFER_ERROR(SYSTEM, transfer->payload_head, transfer->payload_len);
	//	for (int i = 0; i < transfer->payload_len; i++) {
	//		LOG_DEBUG(SYSTEM,"%02x", transfer->payload_head[i]);
	//	}

	//	LOG_DEBUG(SYSTEM, "\n");
	if (transfer->transfer_type == CanardTransferTypeRequest) {
		// check if we want to handle a specific service request
		switch (transfer->data_type_id) {
		case UAVCAN_PROTOCOL_GETNODEINFO_ID: {
			handle_GetNodeInfo(ins, transfer);
			break;
		}
		}
	}
	if (transfer->transfer_type == CanardTransferTypeResponse) {
		switch (transfer->data_type_id) {
		}
	}
	if (transfer->transfer_type == CanardTransferTypeBroadcast) {
		// check if we want to handle a specific broadcast message
		switch (transfer->data_type_id) {
		
		case UAVCAN_PROTOCOL_NODESTATUS_ID: {
			handle_NodeStatus(ins, transfer);
			break;
		}
		
		}
	}
}


void processCanardTxQueue(FDCAN_HandleTypeDef *hfdcan) {
	// Transmitting
//    LOG_DEBUG(SYSTEM, "In transmit Queue");
	for (const CanardCANFrame *tx_frame ; (tx_frame = canardPeekTxQueue(&canard)) != NULL;) 
    {
        LOG_DEBUG(SYSTEM, "Found transmit frame");
		FDCAN_ProtocolStatusTypeDef protocolStatus = {};

        HAL_FDCAN_GetProtocolStatus(hfdcan, &protocolStatus);
        LOG_DEBUG(SYSTEM, "BusOff: %i", protocolStatus.BusOff);
        LOG_DEBUG(SYSTEM, "ErrorPassive: %i", protocolStatus.ErrorPassive);
        const int16_t tx_res = canardSTM32Transmit(hfdcan, tx_frame);

		if (tx_res < 0) {
			LOG_DEBUG(SYSTEM, "Transmit error %d\n", tx_res);
		} else if (tx_res > 0) {
			LOG_DEBUG(SYSTEM, "Successfully transmitted message\n");
		}
        else
        {
            LOG_DEBUG(SYSTEM, "hfderror %"PRIu32", TX Fifo Free: %d", hfdcan->ErrorCode, HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1));
        }
		// Pop canardTxQueue either way
		canardPopTxQueue(&canard);
	}
}

/*
  This function is called at 1 Hz rate from the main loop.
*/
void process1HzTasks(timeUs_t timestamp_usec) 
{
   /*
      Purge transfers that are no longer transmitted. This can free up some memory
    */
    canardCleanupStaleTransfers(&canard, timestamp_usec);

    /*
      Transmit the node status message
    */
    send_NodeStatus();
}

void dronecanInit(void)
{
  LOG_DEBUG(SYSTEM, "dronecan Init");
  MX_FDCAN1_Init();
 
  /*
   Initializing the Libcanard instance.
   */
  LOG_DEBUG(SYSTEM, "canardInit");
  canardInit(&canard,
			 memory_pool,
			 sizeof(memory_pool),
			 onTransferReceived,
			 shouldAcceptTransfer,
			 NULL);

//  uint64_t next_50hz_service_at = HAL_GetTick();

  // Could use DNA (Dynamic Node Allocation) by following example in esc_node.c but that requires a lot of setup and I'm not too sure of what advantage it brings
  // Instead, set a different NODE_ID for each device on the CAN bus by configuring node_settings
 if (NODE_ID > 0) {
	  canardSetLocalNodeID(&canard, NODE_ID);
 } else {
	  LOG_DEBUG(SYSTEM, "Node ID is 0, this node is anonymous and can't transmit most messaged. Please update this in node_settings.h\n");
 }

//  while (1)
//  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	processCanardTxQueue(&hfdcan1);

//	const uint64_t ts = HAL_GetTick();

//	if (ts >= next_1hz_service_at) {
//		next_1hz_service_at += 1000ULL;
//		process1HzTasks(ts);
//	}
//	if (ts >= next_50hz_service_at) {
//		next_50hz_service_at += 1000000ULL/50U;
//		send_ESCStatus();
//	}

//  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */
  FDCAN_FilterTypeDef sFilterConfig;
  sFilterConfig.IdType = FDCAN_EXTENDED_ID;
  sFilterConfig.FilterIndex = 0;
  sFilterConfig.FilterType = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1 = 0x01;
  sFilterConfig.FilterID2 = 0x0;
  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  //hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV22;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;  // Initialize in CAN2.0 mode not CAN_FD
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 100;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 2;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 5;
  hfdcan1.Init.TxEventsNbr = 5;
  hfdcan1.Init.TxBuffersNbr = 5;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_QUEUE_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  LOG_DEBUG(SYSTEM, "In CAN Init");

  GPIO_InitTypeDef gpio_init_structure;

  /* Enable FDCAN clock */
  __HAL_RCC_FDCAN_CLK_ENABLE();
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF9_FDCAN1;

  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1;

  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    LOG_DEBUG(SYSTEM, "Failed CAN Init");
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
	LOG_DEBUG(SYSTEM, "Failed Activate Notification");
	Error_Handler();
  }
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK) {
    LOG_DEBUG(SYSTEM, "Failed Config Filter");
    Error_Handler();
  }
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
    LOG_DEBUG(SYSTEM, "Failed to Start");
    Error_Handler();
  }

  LOG_DEBUG(SYSTEM, "hfderror %"PRIu32", TX Fifo Free: %d", hfdcan1.ErrorCode, HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1));
  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
// static void MX_ICACHE_Init(void)
// {

//   /* USER CODE BEGIN ICACHE_Init 0 */

//   /* USER CODE END ICACHE_Init 0 */

//   /* USER CODE BEGIN ICACHE_Init 1 */

//   /* USER CODE END ICACHE_Init 1 */

//   /** Enable instruction cache in 1-way (direct mapped cache)
//   */
//   if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   if (HAL_ICACHE_Enable() != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN ICACHE_Init 2 */

//   /* USER CODE END ICACHE_Init 2 */

// }

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
// static void MX_GPIO_Init(void)
// {
//   GPIO_InitTypeDef GPIO_InitStruct = {0};
// /* USER CODE BEGIN MX_GPIO_Init_1 */
// /* USER CODE END MX_GPIO_Init_1 */

//   /* GPIO Ports Clock Enable */
//   __HAL_RCC_GPIOC_CLK_ENABLE();
//   __HAL_RCC_GPIOB_CLK_ENABLE();
//   __HAL_RCC_GPIOG_CLK_ENABLE();
//   HAL_PWREx_EnableVddIO2();
//   __HAL_RCC_GPIOA_CLK_ENABLE();
//   __HAL_RCC_GPIOD_CLK_ENABLE();

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(GPIOB, UCPD_DBN_Pin|LED_BLUE_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin : UCPD_FLT_Pin */
//   GPIO_InitStruct.Pin = UCPD_FLT_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   HAL_GPIO_Init(UCPD_FLT_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pin : LED_GREEN_Pin */
//   GPIO_InitStruct.Pin = LED_GREEN_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pin : LED_RED_Pin */
//   GPIO_InitStruct.Pin = LED_RED_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(LED_RED_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pins : UCPD_DBN_Pin LED_BLUE_Pin */
//   GPIO_InitStruct.Pin = UCPD_DBN_Pin|LED_BLUE_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

// /* USER CODE BEGIN MX_GPIO_Init_2 */
// /* USER CODE END MX_GPIO_Init_2 */
// }

// /* USER CODE BEGIN 4 */
// int __io_putchar(int ch)
// {
// 	return ITM_SendChar(ch);
// }
// /* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
// void assert_failed(uint8_t *file, uint32_t line)
// {
//   /* USER CODE BEGIN 6 */
//   /* User can add his own implementation to report the file name and line number,
//      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
//   /* USER CODE END 6 */
// }
#endif /* USE_FULL_ASSERT */

void dronecanUpdate(timeUs_t currentTimeUs)
{
    static timeUs_t lastPrintTimeUs = 0;
    static timeUs_t next_1hz_service_at = 0;
    processCanardTxQueue(&hfdcan1);

    if (currentTimeUs >= next_1hz_service_at) 
    {
		next_1hz_service_at += 1000000ULL;
		process1HzTasks(currentTimeUs);
    }
    if ((currentTimeUs - lastPrintTimeUs) > 500000)
    {
        lastPrintTimeUs = currentTimeUs;
        LOG_DEBUG(SYSTEM, "In dronecanUpdate");
    }
}
