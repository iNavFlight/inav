/*
 * canard_stm32_driver.c
 *
 *  Created on: Jul 8, 2024
 *      Author: Roni Kant
 */

#include "common/log.h"
#include "common/time.h"
#include "drivers/io.h"
#include "canard.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_def.h"
#include "stm32h7xx_hal_fdcan.h"
// #include "stm32h7xx_hal_conf.h"
#include "drivers/nvic.h"
#include <string.h>
#include <stdint.h>

struct Timings {
        uint16_t prescaler;
        uint8_t sjw;
        uint8_t bs1;
        uint8_t bs2;
};

static bool canard_stm32ComputeTimings(const uint32_t target_bitrate, struct Timings*out_timings);
static void canard_stm32_GPIO_Init(void);
void Error_Handler(void);

/**
  * @brief  Process CAN message from RxLocation FIFO into rx_frame
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxLocation Location of the received message to be read.
  *         This parameter can be a value of @arg FDCAN_Rx_location.
  * @param  rx_frame pointer to a CanardCANFrame structure where the received CAN message will be
  * 		stored.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Recieve(FDCAN_HandleTypeDef *hfdcan, uint32_t RxLocation, CanardCANFrame *const rx_frame) {
	if (rx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];

	if (HAL_FDCAN_GetRxMessage(hfdcan, RxLocation, &RxHeader, RxData) == HAL_OK) {

		//LOG_DEBUG(SYSTEM, "Received message: ID=%lu, DLC=%lu", RxHeader.Identifier, RxHeader.DataLength);
		
		//LOG_BUF_DEBUG(SYSTEM, RxData, RxHeader.DataLength);
		//	for (int i = 0; i < RxHeader.DataLength; i++) {
		//		printf("%02x", RxData[i]);
		//	}
		//	printf("\n");

		// Process ID to canard format
		rx_frame->id = RxHeader.Identifier;

		if (RxHeader.IdType == FDCAN_EXTENDED_ID) { // canard will only process the message if it is extended ID
			rx_frame->id |= CANARD_CAN_FRAME_EFF;
		}

		if (RxHeader.RxFrameType == FDCAN_REMOTE_FRAME) { // canard won't process the message if it is a remote frame
			rx_frame->id |= CANARD_CAN_FRAME_RTR;
		}

		rx_frame->data_len = RxHeader.DataLength;
		memcpy(rx_frame->data, RxData, RxHeader.DataLength);

		// assume a single interface
		rx_frame->iface_id = 0;

		return 1;
	}

	// Either no CAN msg to be read, or an error that can be read from hfdcan->ErrorCode
	return 0;
}

/**
  * @brief  Process tx_frame CAN message into Tx FIFO/Queue and transmit it
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  tx_frame pointer to a CanardCANFrame structure that contains the CAN message to
  * 		transmit.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Transmit(FDCAN_HandleTypeDef *hfdcan, const CanardCANFrame* const tx_frame) {
	if (tx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	if (tx_frame->id & CANARD_CAN_FRAME_ERR) {
		return -CANARD_ERROR_INVALID_ARGUMENT; // unsupported frame format
	}

	FDCAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[8];

	// Process canard id to STM FDCAN header format
	if (tx_frame->id & CANARD_CAN_FRAME_EFF) {
		TxHeader.IdType = FDCAN_EXTENDED_ID;
		TxHeader.Identifier = tx_frame->id & CANARD_CAN_EXT_ID_MASK;
	} else {
		TxHeader.IdType = FDCAN_STANDARD_ID;
		TxHeader.Identifier = tx_frame->id & CANARD_CAN_STD_ID_MASK;
	}

	TxHeader.DataLength = tx_frame->data_len;

	if (tx_frame->id & CANARD_CAN_FRAME_RTR) {
		TxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	} else {
		TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	}

	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE; // unsure about this one
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF; // Disabling FDCAN (using CAN 2.0)
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN; // Disabling FDCAN (using CAN 2.0)
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // unsure about this one
	TxHeader.MessageMarker = 0; // unsure about this one
	memcpy(TxData, tx_frame->data, TxHeader.DataLength);

	if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, TxData) == HAL_OK) {
		// LOG_DEBUG(SYSTEM, "Successfully sent message with id: %lu", TxHeader.Identifier);
		return 1;
	}

	LOG_DEBUG(SYSTEM, "Failed at adding message with id: %lu to Tx Queue", TxHeader.Identifier);
    
	// This might be for many reasons including the Tx Fifo being full, the error can be read from hfdcan->ErrorCode
	return 0;
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
void canardSTM32_FDCAN1_Init(FDCAN_HandleTypeDef *hfdcan1, uint32_t bitrate)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    struct Timings out_timings;

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  FDCAN_FilterTypeDef sFilterConfig;
  sFilterConfig.IdType = FDCAN_EXTENDED_ID;
  sFilterConfig.FilterIndex = 0;
  sFilterConfig.FilterType = FDCAN_FILTER_DUAL;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1 = 0x0; //0x1401557F;
  sFilterConfig.FilterID2 = 0x1FFFFFFFU;
  // sFilterConfig.RxBufferIndex = 0;
  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1->Instance = FDCAN1;
  hfdcan1->Init.FrameFormat = FDCAN_FRAME_CLASSIC;  // Initialize in CAN2.0 mode not CAN_FD
  hfdcan1->Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1->Init.AutoRetransmission = DISABLE;
  hfdcan1->Init.TransmitPause = DISABLE;
  hfdcan1->Init.ProtocolException = DISABLE;

  canard_stm32ComputeTimings(bitrate, &out_timings);

  hfdcan1->Init.NominalPrescaler = out_timings.prescaler;
  hfdcan1->Init.NominalSyncJumpWidth = out_timings.sjw;
  hfdcan1->Init.NominalTimeSeg1 = out_timings.bs1;
  hfdcan1->Init.NominalTimeSeg2 = out_timings.bs2;
  LOG_DEBUG(SYSTEM, "Prescaler: %d, SJW: %d, BS1: %d, BS2: %d", out_timings.prescaler, out_timings.sjw, out_timings.bs1, out_timings.bs2);

  hfdcan1->Init.RxFifo0ElmtsNbr = 30;
  hfdcan1->Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1->Init.RxBuffersNbr = 1;
  hfdcan1->Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1->Init.StdFiltersNbr = 0;
  hfdcan1->Init.ExtFiltersNbr = 1;
  hfdcan1->Init.TxFifoQueueElmtsNbr = 32;
  hfdcan1->Init.TxEventsNbr = 0;
  hfdcan1->Init.TxBuffersNbr = 5;
  hfdcan1->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1->Init.TxElmtSize = FDCAN_DATA_BYTES_8;
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

    canard_stm32_GPIO_Init();  // Set up the pins for CAN and optional listen only mode
    
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, NVIC_PRIO_CAN, NVIC_PRIO_CAN);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);  // Enable FDCAN1 interrupt line 0
    
    // LOG_DEBUG(SYSTEM, "System Clock Speed: %lu", HAL_RCC_GetSysClockFreq());
    // LOG_DEBUG(SYSTEM, "PClk1 Clock Speed: %lu", HAL_RCC_GetPCLK1Freq());
    if (HAL_FDCAN_Init(hfdcan1) != HAL_OK)
    {
        LOG_ERROR(SYSTEM, "Failed CAN Init");
        Error_Handler();
    }
    /* USER CODE BEGIN FDCAN1_Init 2 */
    if (HAL_FDCAN_ConfigFilter(hfdcan1, &sFilterConfig) != HAL_OK) {
        LOG_ERROR(SYSTEM, "Failed Config Filter");
        Error_Handler();
    }
    if (HAL_FDCAN_ConfigGlobalFilter(hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK) {
        LOG_ERROR(SYSTEM, "Failed to config FDCAN filter");
        Error_Handler();
    }
    if (HAL_FDCAN_Start(hfdcan1) != HAL_OK) {
        LOG_ERROR(SYSTEM, "Failed to Start");
        Error_Handler();
    }
    // Activate notifications
    //  if (HAL_FDCAN_ActivateNotification(hfdcan1, 
    //     FDCAN_IT_RX_FIFO0_NEW_MESSAGE |
    // //     FDCAN_IT_ERROR_WARNING | 
    //  //    FDCAN_IT_ERROR_PASSIVE | 
    //      FDCAN_IT_BUS_OFF, 0) != HAL_OK)
    //  {
    //     LOG_ERROR(SYSTEM, "Failed to start interrupts");
    //     //     Error_Handler();
    //  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void canard_stm32_GPIO_Init(void)
{
   // Set up the Rx and Tx pins for CAN1 and if present, the standby or listen only pin.

    IOInit(IOGetByTag(IO_TAG(CAN1_TX)), OWNER_DRONECAN, RESOURCE_CAN_TX, 0);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(CAN1_TX)), IOCFG_AF_PP, GPIO_AF9_FDCAN1);  // How do I make the alternate function crossplatform?
    IOInit(IOGetByTag(IO_TAG(CAN1_RX)), OWNER_DRONECAN, RESOURCE_CAN_RX, 0);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(CAN1_RX)), IOCFG_AF_PP, GPIO_AF9_FDCAN1);  // How do I make the alternate function crossplatform?

 #ifdef CAN1_STANDBY
    // Initialize the standby or listen only pin.  Set default state to enable CAN.
    // TODO: Tie the pin state to a configuration option so we can turn CAN on and off.

    IOInit(IOGetByTag(IO_TAG(CAN1_STANDBY)), OWNER_DRONECAN, RESOURCE_CAN_STANDBY, 0);
    IOConfigGPIO(IOGetByTag(IO_TAG(CAN1_STANDBY)), IOCFG_OUT_PP);  // Do any boards use pullups, external/internal?
    IOLo(IOGetByTag(IO_TAG(CAN1_STANDBY)));
#endif
}

static bool canard_stm32ComputeTimings(const uint32_t target_bitrate, struct Timings *out_timings)
{
    if (target_bitrate < 1) {
        return false;
    }

    /*
     * Hardware configuration
     */
    const uint32_t pclk = HAL_RCC_GetPCLK1Freq();

    static const int MaxBS1 = 16;
    static const int MaxBS2 = 8;

    /*
     * Ref. "Automatic Baudrate Detection in CANopen Networks", U. Koppe, MicroControl GmbH & Co. KG
     *      CAN in Automation, 2003
     *
     * According to the source, optimal quanta per bit are:
     *   Bitrate        Optimal Maximum
     *   1000 kbps      8       10
     *   500  kbps      16      17
     *   250  kbps      16      17
     *   125  kbps      16      17
     */
    const int max_quanta_per_bit = (target_bitrate >= 1000000) ? 10 : 17;
    LOG_DEBUG(SYSTEM, "Baudrate: %lu", target_bitrate);
    LOG_DEBUG(SYSTEM, "Max Quanta per bit: %i", max_quanta_per_bit);

    static const int MaxSamplePointLocation = 900;

    /*
     * Computing (prescaler * BS):
     *   BITRATE = 1 / (PRESCALER * (1 / PCLK) * (1 + BS1 + BS2))       -- See the Reference Manual
     *   BITRATE = PCLK / (PRESCALER * (1 + BS1 + BS2))                 -- Simplified
     * let:
     *   BS = 1 + BS1 + BS2                                             -- Number of time quanta per bit
     *   PRESCALER_BS = PRESCALER * BS
     * ==>
     *   PRESCALER_BS = PCLK / BITRATE
     */
    const uint32_t prescaler_bs = pclk / target_bitrate;
    LOG_DEBUG(SYSTEM, "Prescaler BS product: %lu", prescaler_bs);
     /*
     * Searching for such prescaler value so that the number of quanta per bit is highest.
     */
    uint8_t bs1_bs2_sum = (uint8_t)(max_quanta_per_bit - 1);

    while ((prescaler_bs % (1 + bs1_bs2_sum)) != 0) {
        if (bs1_bs2_sum <= 2) {
            return false;          // No solution
        }
        bs1_bs2_sum--;
    }

    const uint32_t prescaler = prescaler_bs / (1 + bs1_bs2_sum);
    if ((prescaler < 1U) || (prescaler > 1024U)) {
        return false;              // No solution
    }
    LOG_DEBUG(SYSTEM, "Prescaler: %lu", prescaler);

      /*
     * Now we have a constraint: (BS1 + BS2) == bs1_bs2_sum.
     * We need to find the values so that the sample point is as close as possible to the optimal value.
     *
     *   Solve[(1 + bs1)/(1 + bs1 + bs2) == 7/8, bs2]  (* Where 7/8 is 0.875, the recommended sample point location *)
     *   {{bs2 -> (1 + bs1)/7}}
     *
     * Hence:
     *   bs2 = (1 + bs1) / 7
     *   bs1 = (7 * bs1_bs2_sum - 1) / 8
     *
     * Sample point location can be computed as follows:
     *   Sample point location = (1 + bs1) / (1 + bs1 + bs2)
     *
     * Since the optimal solution is so close to the maximum, we prepare two solutions, and then pick the best one:
     *   - With rounding to nearest
     *   - With rounding to zero
     */
    struct BsPair {
        uint8_t bs1;
        uint8_t bs2;
        uint16_t sample_point_permill
    } solution;

    // First attempt with rounding to nearest
    solution.bs1 = (uint8_t)(((7 * bs1_bs2_sum - 1) + 4) / 8);
    solution.bs2 = (uint8_t)(bs1_bs2_sum - solution.bs1);
    solution.sample_point_permill = (uint16_t)(1000 * (1 + solution.bs1) / (1 + solution.bs1 + solution.bs2));

    if (solution.sample_point_permill > MaxSamplePointLocation) {
        // Second attempt with rounding to zero
        solution.bs1 = (uint8_t)((7 * bs1_bs2_sum - 1) / 8);
        solution.bs2 = (uint8_t)(bs1_bs2_sum - solution.bs1);
        solution.sample_point_permill = (uint16_t)(1000 * (1 + solution.bs1) / (1 + solution.bs1 + solution.bs2));
    }
     /*
     * Final validation
     * Helpful Python:
     * def sample_point_from_btr(x):
     *     assert 0b0011110010000000111111000000000 & x == 0
     *     ts2,ts1,brp = (x>>20)&7, (x>>16)&15, x&511
     *     return (1+ts1+1)/(1+ts1+1+ts2+1)
     *
     */
    if ((target_bitrate != (pclk / (prescaler * (1 + solution.bs1 + solution.bs2)))) || !((solution.bs1 >= 1) && (solution.bs1 <= MaxBS1) && (solution.bs2 >= 1) && (solution.bs2 <= MaxBS2)))
    {
        return false;
    }

    LOG_DEBUG(SYSTEM, "Timings: quanta/bit: %d, sample point location: %f%%",
          (int)(1 + solution.bs1 + solution.bs2), (double)(solution.sample_point_permill) / 10.F);

    out_timings->prescaler = (uint16_t)(prescaler);
    out_timings->sjw = 8;                        // Not happy with this value, but 1MBPs with unshielded cable?
    out_timings->bs1 = (uint8_t)(solution.bs1);  // The HAL takes care of the 1 bs offset in the register so don't remove it here like AP does.
    out_timings->bs2 = (uint8_t)(solution.bs2);  // The HAL takes care of the 1 bs offset in the register so don't remove it here like AP does.

    return true;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
//   __disable_irq();
//   while (1)
//   {
//   }
  /* USER CODE END Error_Handler_Debug */
}

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
    // HAL_FDCAN_DeactivateNotification(hfdcan, ErrorStatusITs);

    // if(ErrorStatusITs & FDCAN_IT_ERROR_WARNING)
    // {
    //     LOG_ERROR(SYSTEM, "CAN in Warning state");
    // }
    
    // if(ErrorStatusITs & FDCAN_IT_ERROR_PASSIVE)
    // {
    //     LOG_ERROR(SYSTEM, "CAN in ERROR PASSIVE state");
    // }
    
    /* UART Over-Run interrupt occurred -----------------------------------------*/
    //if ((__HAL_FDCAN_GET_IT(hfdcan, FDCAN_IT_ERROR_PASSIVE) != RESET)) {
        __HAL_FDCAN_CLEAR_IT(hfdcan, FDCAN_IT_ERROR_PASSIVE | FDCAN_IT_BUS_OFF);
    //}
    // if(ErrorStatusITs & FDCAN_IT_BUS_OFF)
    //{
    //     // Handle bus off state
    //      HAL_FDCAN_Stop(hfdcan); 
    //      LOG_ERROR(SYSTEM, "CAN in BUS OFF state");

    //      HAL_Delay(100);
    //      HAL_FDCAN_Start(hfdcan);
    // // }
}


