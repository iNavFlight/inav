#include <stdint.h>
#include "libcanard/canard_stm32_driver.h"
#include "libcanard/canard.h"
#include "dronecan.h"

#include <stdio.h>
#include <string.h>

//#include <dronecan_msgs.h>
//#include <node_settings.h>

/* Private variables ---------------------------------------------------------*/
// ADC_HandleTypeDef hadc1;

FDCAN_HandleTypeDef hfdcan1;

//UART_HandleTypeDef hlpuart1;

//RTC_HandleTypeDef hrtc;

//PCD_HandleTypeDef hpcd_USB_FS;

//CanardInstance canard;
uint8_t memory_pool[1024];
//static struct uavcan_protocol_NodeStatus node_status;

//void SystemClock_Config(void);
//static void MX_GPIO_Init(void);
//static void MX_ADC1_Init(void);
//static void MX_ICACHE_Init(void);
//static void MX_LPUART1_UART_Init(void);
//static void MX_RTC_Init(void);
//static void MX_UCPD1_Init(void);
//static void MX_USB_PCD_Init(void);
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
/*
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
/*
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
*/
// Canard Senders

/*
  send the 1Hz NodeStatus message. This is what allows a node to show
  up in the DroneCAN GUI tool and in the flight controller logs
 */
 /*
void send_NodeStatus(void) {
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];

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
*/
// Canard Util
/*
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
		case UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID: {
			*out_data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE;
			return true;
		}
		case UAVCAN_PROTOCOL_NODESTATUS_ID: {
			*out_data_type_signature = UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE;
			return true;
		}
		case ARDUPILOT_INDICATION_NOTIFYSTATE_ID: {
			*out_data_type_signature = ARDUPILOT_INDICATION_NOTIFYSTATE_SIGNATURE;
			return true;
		}
		}
	}
	// we don't want any other messages
	return false;
}
*/
/*
void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer) {
	// switch on data type ID to pass to the right handler function
//	printf("Transfer type: %du, Transfer ID: %du \n", transfer->transfer_type, transfer->data_type_id);
//	printf("0x");
//		for (int i = 0; i < transfer->payload_len; i++) {
//			printf("%02x", transfer->payload_head[i]);
//		}
//
//		printf("\n");
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
		case UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID: {
			handle_RawCommand(ins, transfer);
			break;
		}
		case UAVCAN_PROTOCOL_NODESTATUS_ID: {
			handle_NodeStatus(ins, transfer);
			break;
		}
		case ARDUPILOT_INDICATION_NOTIFYSTATE_ID: {
			handle_NotifyState(ins, transfer);
			break;
		}
		}
	}
}
*/
/*
void processCanardTxQueue(FDCAN_HandleTypeDef *hfdcan) {
	// Transmitting

	for (const CanardCANFrame *tx_frame ; (tx_frame = canardPeekTxQueue(&canard)) != NULL;) {
		const int16_t tx_res = canardSTM32Transmit(hfdcan, tx_frame);

		if (tx_res < 0) {
			printf("Transmit error %d\n", tx_res);
		} else if (tx_res > 0) {
			printf("Successfully transmitted message\n");
		}

		// Pop canardTxQueue either way
		canardPopTxQueue(&canard);
	}
}
*/
/*
  This function is called at 1 Hz rate from the main loop.
*/
//void process1HzTasks(uint64_t timestamp_usec) {
//   /*
//      Purge transfers that are no longer transmitted. This can free up some memory
//    */
//    canardCleanupStaleTransfers(&canard, timestamp_usec);

    /*
      Transmit the node status message
    */
//    send_NodeStatus();
//}

void dronecanInit(void)
{

  MX_FDCAN1_Init();
 
  /*
   Initializing the Libcanard instance.
   */
//  canardInit(&canard,
//			 memory_pool,
//			 sizeof(memory_pool),
//			 onTransferReceived,
//			 shouldAcceptTransfer,
//			 NULL);

//  uint64_t next_1hz_service_at = HAL_GetTick();
//  uint64_t next_50hz_service_at = HAL_GetTick();

  // Could use DNA (Dynamic Node Allocation) by following example in esc_node.c but that requires a lot of setup and I'm not too sure of what advantage it brings
  // Instead, set a different NODE_ID for each device on the CAN bus by configuring node_settings
//  if (NODE_ID > 0) {
//	  canardSetLocalNodeID(&canard, NODE_ID);
//  } else {
//	  printf("Node ID is 0, this node is anonymous and can't transmit most messaged. Please update this in node_settings.h\n");
//  }

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
  * @brief System Clock Configuration
  * @retval None
  */ 
//void SystemClock_Config(void)
//{
//  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
//  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0) != HAL_OK)
//  {
//    Error_Handler();
//  }

  /** Configure LSE Drive Capability
  */
//  HAL_PWR_EnableBkUpAccess();
//  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
//                              |RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
//  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
// //   RCC_OscInitStruct.HSIState = RCC_HSI_ON;
// //   RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
// //   RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
// //   RCC_OscInitStruct.MSIState = RCC_MSI_ON;
// //   RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
// //   RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
// //   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
// //   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
// //   RCC_OscInitStruct.PLL.PLLM = 1;
// //   RCC_OscInitStruct.PLL.PLLN = 55;
// //   RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
// //   RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
// //   RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
// //   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
// //   {
// //     Error_Handler();
// //   }

//   /** Initializes the CPU, AHB and APB buses clocks
//   */
//   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
//                               |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
//   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
//   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
//   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

//   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
//   {
//     Error_Handler();
//   }
// }

// /**
//   * @brief ADC1 Initialization Function
//   * @param None
//   * @retval None
//   */
// static void MX_ADC1_Init(void)
// {

//   /* USER CODE BEGIN ADC1_Init 0 */

//   /* USER CODE END ADC1_Init 0 */

//   ADC_MultiModeTypeDef multimode = {0};
//   ADC_ChannelConfTypeDef sConfig = {0};

//   /* USER CODE BEGIN ADC1_Init 1 */

//   /* USER CODE END ADC1_Init 1 */

//   /** Common config
//   */
//   hadc1.Instance = ADC1;
//   hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
//   hadc1.Init.Resolution = ADC_RESOLUTION_12B;
//   hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
//   hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
//   hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
//   hadc1.Init.LowPowerAutoWait = DISABLE;
//   hadc1.Init.ContinuousConvMode = DISABLE;
//   hadc1.Init.NbrOfConversion = 1;
//   hadc1.Init.DiscontinuousConvMode = DISABLE;
//   hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
//   hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
//   hadc1.Init.DMAContinuousRequests = DISABLE;
//   hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
//   hadc1.Init.OversamplingMode = DISABLE;
//   if (HAL_ADC_Init(&hadc1) != HAL_OK)
//   {
//     Error_Handler();
//   }

//   /** Configure the ADC multi-mode
//   */
//   multimode.Mode = ADC_MODE_INDEPENDENT;
//   if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
//   {
//     Error_Handler();
//   }

//   /** Configure Regular Channel
//   */
//   sConfig.Channel = ADC_CHANNEL_3;
//   sConfig.Rank = ADC_REGULAR_RANK_1;
//   sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
//   sConfig.SingleDiff = ADC_SINGLE_ENDED;
//   sConfig.OffsetNumber = ADC_OFFSET_NONE;
//   sConfig.Offset = 0;
//   if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN ADC1_Init 2 */

//   /* USER CODE END ADC1_Init 2 */

// }

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
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
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
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
	printf("1");
	Error_Handler();
  }
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK) {
    printf("2");
    Error_Handler();
  }
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
    printf("3");
    Error_Handler();
  }

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
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
// static void MX_LPUART1_UART_Init(void)
// {

//   /* USER CODE BEGIN LPUART1_Init 0 */

//   /* USER CODE END LPUART1_Init 0 */

//   /* USER CODE BEGIN LPUART1_Init 1 */

//   /* USER CODE END LPUART1_Init 1 */
//   hlpuart1.Instance = LPUART1;
//   hlpuart1.Init.BaudRate = 209700;
//   hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
//   hlpuart1.Init.StopBits = UART_STOPBITS_1;
//   hlpuart1.Init.Parity = UART_PARITY_NONE;
//   hlpuart1.Init.Mode = UART_MODE_TX_RX;
//   hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//   hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
//   hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
//   hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
//   hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
//   if (HAL_UART_Init(&hlpuart1) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN LPUART1_Init 2 */

//   /* USER CODE END LPUART1_Init 2 */

// }

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
// static void MX_RTC_Init(void)
// {

//   /* USER CODE BEGIN RTC_Init 0 */

//   /* USER CODE END RTC_Init 0 */

//   RTC_PrivilegeStateTypeDef privilegeState = {0};

//   /* USER CODE BEGIN RTC_Init 1 */

//   /* USER CODE END RTC_Init 1 */

//   /** Initialize RTC Only
//   */
//   hrtc.Instance = RTC;
//   hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
//   hrtc.Init.AsynchPrediv = 127;
//   hrtc.Init.SynchPrediv = 255;
//   hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
//   hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
//   hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
//   hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
//   hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
//   if (HAL_RTC_Init(&hrtc) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
//   privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
//   privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
//   privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
//   if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN RTC_Init 2 */

//   /* USER CODE END RTC_Init 2 */

// }

/**
  * @brief UCPD1 Initialization Function
  * @param None
  * @retval None
  */
// static void MX_UCPD1_Init(void)
// {

//   /* USER CODE BEGIN UCPD1_Init 0 */

//   /* USER CODE END UCPD1_Init 0 */

//   LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

//   /* Peripheral clock enable */
//   LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_UCPD1);

//   LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
//   LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
//   /**UCPD1 GPIO Configuration
//   PB15   ------> UCPD1_CC2
//   PA15 (JTDI)   ------> UCPD1_CC1
//   */
//   GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
//   GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
//   GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//   LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//   GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
//   GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
//   GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//   LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//   /* USER CODE BEGIN UCPD1_Init 1 */

//   /* USER CODE END UCPD1_Init 1 */
//   /* USER CODE BEGIN UCPD1_Init 2 */

//   /* USER CODE END UCPD1_Init 2 */

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