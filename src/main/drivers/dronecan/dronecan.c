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
static void MX_GPIO_Init(void);
void Error_Handler(void);
void PrintCanStatus(void);

void PrintCanStatus(void)
{
    uint32_t status = hfdcan1.Instance->PSR;
    FDCAN_ErrorCountersTypeDef errorCounters;
    HAL_FDCAN_GetErrorCounters (&hfdcan1, &errorCounters);
    
    LOG_DEBUG(SYSTEM, "CAN Status:\n");
    LOG_DEBUG(SYSTEM, "  Last Error Code: %lu\n", (status & FDCAN_PSR_LEC) >> FDCAN_PSR_LEC_Pos);
    LOG_DEBUG(SYSTEM, "  Activity: %s\n", (status & FDCAN_PSR_ACT) ? "Active" : "Inactive");
    LOG_DEBUG(SYSTEM, "  Error Passive: %s\n", (status & FDCAN_PSR_EP) ? "Yes" : "No");
    LOG_DEBUG(SYSTEM, "  Warning Status: %s\n", (status & FDCAN_PSR_EW) ? "Yes" : "No");
    LOG_DEBUG(SYSTEM, "  Bus Off: %s\n", (status & FDCAN_PSR_BO) ? "Yes" : "No");
    LOG_DEBUG(SYSTEM, "Tx Error Count: %lu", errorCounters.TxErrorCnt);
    LOG_DEBUG(SYSTEM, "Rx Error Count: %lu", errorCounters.RxErrorCnt);
}

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

	LOG_DEBUG(SYSTEM, "Node health: %ud Node Mode: %ud\n", nodeStatus.health, nodeStatus.mode);

	LOG_DEBUG(SYSTEM, "Node Health ");

	switch (nodeStatus.health) {
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK:
		LOG_DEBUG(SYSTEM, "OK\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING:
		LOG_DEBUG(SYSTEM, "WARNING\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR:
		LOG_DEBUG(SYSTEM, "ERROR\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL:
		LOG_DEBUG(SYSTEM, "CRITICAL\n");
		break;
	default:
		LOG_DEBUG(SYSTEM, "UNKNOWN?\n");
		break;
	}

	LOG_DEBUG(SYSTEM, "Node Mode ");

	switch(nodeStatus.mode) {
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL:
		LOG_DEBUG(SYSTEM, "OPERATIONAL\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION:
		LOG_DEBUG(SYSTEM, "INITIALIZATION\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_MAINTENANCE:
		LOG_DEBUG(SYSTEM, "MAINTENANCE\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_SOFTWARE_UPDATE:
		LOG_DEBUG(SYSTEM, "SOFTWARE UPDATE\n");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_OFFLINE:
		LOG_DEBUG(SYSTEM, "OFFLINE\n");
		break;
	default:
		LOG_DEBUG(SYSTEM, "UNKNOWN?\n");
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
	pkt.software_version.major = 9;
	pkt.software_version.minor = 0;
	pkt.software_version.optional_field_flags = 0;
	pkt.software_version.vcs_commit = 0; // should put git hash in here

	// should fill in hardware version
	pkt.hardware_version.major = 1;
	pkt.hardware_version.minor = 0;

	// just setting all 16 bytes to 1 for testing
	getUniqueID(pkt.hardware_version.unique_id);

	strncpy((char*)pkt.name.data, "Inav", sizeof(pkt.name.data));
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
    PrintCanStatus();
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
	for (const CanardCANFrame *tx_frame ; (tx_frame = canardPeekTxQueue(&canard)) != NULL;) 
    {
        LOG_DEBUG(SYSTEM, "Found transmit frame");
		FDCAN_ProtocolStatusTypeDef protocolStatus = {};

        HAL_FDCAN_GetProtocolStatus(hfdcan, &protocolStatus);
        LOG_DEBUG(SYSTEM, "BusOff: %lu", protocolStatus.BusOff);
        LOG_DEBUG(SYSTEM, "ErrorPassive: %lu", protocolStatus.ErrorPassive);
        const int16_t tx_res = canardSTM32Transmit(hfdcan, tx_frame);

		if (tx_res < 0) {
			LOG_DEBUG(SYSTEM, "Transmit error %d\n", tx_res);
		} else if (tx_res > 0) {
			LOG_DEBUG(SYSTEM, "Successfully transmitted message\n");
		}
        else
        {
            LOG_DEBUG(SYSTEM, "hfderror %"PRIu32"", hfdcan->ErrorCode);
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
 PrintCanStatus();

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
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
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;  // Initialize in CAN2.0 mode not CAN_FD
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  // TODO:: Calculate these dynamically based on clock speed and desired baudrate
  hfdcan1.Init.NominalPrescaler = 8;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 12;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;

  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 32;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 5;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_QUEUE_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  LOG_DEBUG(SYSTEM, "In CAN Init");
   
  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      LOG_DEBUG(SYSTEM, "Unable to configure peripheral clock");
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();
    /* Enable FDCAN clock */
    __HAL_RCC_FDCAN_CLK_ENABLE();
  
    MX_GPIO_Init();  // Set up the pins for CAN and optional listen only mode

    LOG_DEBUG(SYSTEM, "System Clock Speed: %lu", HAL_RCC_GetSysClockFreq());
    LOG_DEBUG(SYSTEM, "PClk1 Clock Speed: %lu", HAL_RCC_GetPCLK1Freq());
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
  
  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
  
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /**FDCAN1 GPIO Configuration
        PD0     ------> FDCAN1_RX
        PD1     ------> FDCAN1_TX
        PD3     ------> CANPhy Listen Only
        */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitTypeDef GPIO_InitStructCANSilent = {0};
  
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);

    /*Configure GPIO pin : PD3 */
    GPIO_InitStructCANSilent.Pin = GPIO_PIN_3;
    GPIO_InitStructCANSilent.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructCANSilent.Pull = GPIO_NOPULL;
    GPIO_InitStructCANSilent.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructCANSilent);
}

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

