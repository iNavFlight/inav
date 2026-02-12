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
#include "canard_stm32_driver.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_def.h"
#include "stm32f7xx_hal_can.h"

#include <string.h>
#include <stdint.h>

#define RX_BUFFER_SIZE 32

struct Timings {
        uint16_t prescaler;
        uint8_t sjw;
        uint8_t bs1;
        uint8_t bs2;
};

static struct RxBuffer_t {
    uint8_t writeIndex;
    uint8_t readIndex;
    CanRxMsgTypeDef rxMsg[RX_BUFFER_SIZE];
} RxBuffer;

static bool canardSTM32ComputeTimings(const uint32_t target_bitrate, struct Timings*out_timings);
static void canardSTM32GPIO_Init(void);

static CAN_HandleTypeDef hcan1;
CanRxMsgTypeDef rxMsg;

uint8_t rxBufferPushFrame(struct RxBuffer_t *rxBuf, CanRxMsgTypeDef *rxMsg) {
    uint8_t next;
    CanRxMsgTypeDef *pCurrentRxMsg;

    next = rxBuf->writeIndex + 1;
    if(next >= RX_BUFFER_SIZE){
        next = 0;
    }

    if(next == rxBuf->readIndex) {
        return -1;  // rxBuf is full
    }
    pCurrentRxMsg = &rxBuf->rxMsg[rxBuf->writeIndex];
    memcpy(pCurrentRxMsg, rxMsg, sizeof(CanRxMsgTypeDef));
    rxBuf->writeIndex = next;
    return 0;
}

uint8_t rxBufferPopFrame(struct RxBuffer_t *rxBuf, CanRxMsgTypeDef *rxMsg) {
    uint8_t next;
    CanRxMsgTypeDef *pCurrentRxMsg;

    if(rxBuf->writeIndex == rxBuf->readIndex){
        return -1;  // Nothing to read
    }

    next = rxBuf->readIndex + 1;
    if (next >= RX_BUFFER_SIZE){
        next = 0;
    }
    pCurrentRxMsg = &rxBuf->rxMsg[rxBuf->readIndex];
    memcpy(rxMsg, pCurrentRxMsg, sizeof(CanRxMsgTypeDef));
    rxBuf->readIndex = next;
    return 0;
}

uint8_t rxBufferNumMessages(struct RxBuffer_t *rxBuf) {
    if(rxBuf->writeIndex < rxBuf->readIndex)
        return((rxBuf->writeIndex + RX_BUFFER_SIZE) - rxBuf->readIndex);
    
    return (rxBuf->writeIndex - rxBuf->readIndex);
}

/**
  * @brief  Process CAN message from RxLocation FIFO into rx_frame
  * @param  rx_frame pointer to a CanardCANFrame structure where the received CAN message will be
  * 		stored.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Recieve(CanardCANFrame *const rx_frame) {
    CanRxMsgTypeDef canRxFrame;

    if (rx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	if (rxBufferPopFrame(&RxBuffer, &canRxFrame) == 0) {  // Wheres the data?
        rx_frame->id = canRxFrame.ExtId;

		// Process ID to canard format
		if (canRxFrame.IDE == CAN_ID_EXT) { // canard will only process the message if it is extended ID
            rx_frame->id |= CANARD_CAN_FRAME_EFF;
		}

		if (canRxFrame.RTR == CAN_RTR_REMOTE) { // canard won't process the message if it is a remote frame
			rx_frame->id |= CANARD_CAN_FRAME_RTR;
		}

		rx_frame->data_len = canRxFrame.DLC;
		memcpy(rx_frame->data, canRxFrame.Data, canRxFrame.DLC);

		// assume a single interface
		rx_frame->iface_id = 0;
		return 1;
	}
	// Either no CAN msg to be read, or an error that can be read from hfdcan->ErrorCode
	return 0;
}

/**
  * @brief  Process tx_frame CAN message into Tx FIFO/Queue and transmit it
  * @param  tx_frame pointer to a CanardCANFrame structure that contains the CAN message to
  * 		transmit.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Transmit(const CanardCANFrame* const tx_frame) {
	CanTxMsgTypeDef txMsg = {};
    uint32_t returnCode;

    if (tx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	if (tx_frame->id & CANARD_CAN_FRAME_ERR) {
		return -CANARD_ERROR_INVALID_ARGUMENT; // unsupported frame format
	}

	// Process canard id to STM FDCAN header format
	if (tx_frame->id & CANARD_CAN_FRAME_EFF) {
		txMsg.IDE = CAN_ID_EXT;
		txMsg.ExtId = tx_frame->id & CANARD_CAN_EXT_ID_MASK;
	} else {
		txMsg.IDE = CAN_ID_STD;
		txMsg.StdId = tx_frame->id & CANARD_CAN_STD_ID_MASK;
	}

	txMsg.DLC = tx_frame->data_len;

	if (tx_frame->id & CANARD_CAN_FRAME_RTR) {
		txMsg.RTR = CAN_RTR_REMOTE;
	} else {
		txMsg.RTR = CAN_RTR_DATA;
	}

	memcpy(txMsg.Data, tx_frame->data, tx_frame->data_len);

    hcan1.pTxMsg = &txMsg;

	returnCode = HAL_CAN_Transmit(&hcan1, 100);
    if( returnCode == HAL_OK) {
		// LOG_DEBUG(CAN, "Successfully sent message with id: %lu", tx_frame->id);
		return 1;
	}

	LOG_DEBUG(CAN, "Failed at adding message with id: %lu to Tx Queue.  Error: %lu", tx_frame->id, returnCode);

	// TX failed (FIFO full or other error) - return 0 to signal retry needed
	return 0;
}

/**
  * @brief FDCAN1 Initialization Function
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  bitrate desired bitrate to run the CAN network at.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32CAN1_Init(uint32_t bitrate)
{
//    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    struct Timings out_timings;

     /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();


    // /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    // /* USER CODE BEGIN CAN1_MspInit 1 */

    CAN_FilterConfTypeDef sFilterConfig;
    sFilterConfig.FilterIdHigh = 0;
    sFilterConfig.FilterIdLow  = 0;
    sFilterConfig.FilterMaskIdHigh = 0;
    sFilterConfig.FilterMaskIdLow = 0;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    sFilterConfig.FilterNumber = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.BankNumber = 0;
  
    hcan1.Instance = CAN1;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.TTCM = DISABLE;
    hcan1.Init.ABOM = ENABLE;
    hcan1.Init.AWUM = DISABLE;
    hcan1.Init.NART = DISABLE;
    hcan1.Init.RFLM = DISABLE;
    hcan1.Init.TXFP = DISABLE;
  
    canardSTM32ComputeTimings(bitrate, &out_timings);

    hcan1.Init.Prescaler = out_timings.prescaler;
    hcan1.Init.SJW = (out_timings.sjw << CAN_BTR_SJW_Pos);  // Shift the SJW value over to the correct position in the BTR
    hcan1.Init.BS1 = (out_timings.bs1 << CAN_BTR_TS1_Pos);  // Shift the bs1 value over to the correct position in the BTR
    hcan1.Init.BS2 = (out_timings.bs2 << CAN_BTR_TS2_Pos);  // Shift the bs2 value over to the correct position in the BTR
    LOG_DEBUG(CAN, "Prescaler: %d, SJW: %d, BS1: %d, BS2: %d", out_timings.prescaler, out_timings.sjw, out_timings.bs1, out_timings.bs2);

    // hcan1.Init.StdFiltersNbr = 0;
    // hcan1.Init.ExtFiltersNbr = 1;
    // hcan1.Init.TxFifoQueueElmtsNbr = 32;
    // LOG_DEBUG(CAN, "In CAN Init");

    /** Initializes the peripherals clock
    */
    // PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    // PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    // if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    // {
    //   LOG_DEBUG(CAN, "Unable to configure peripheral clock");
    // }

    canardSTM32GPIO_Init();  // Set up the pins for CAN and optional listen only mode
    
    // LOG_DEBUG(CAN, "System Clock Speed: %lu", HAL_RCC_GetSysClockFreq());
    // LOG_DEBUG(CAN, "PClk1 Clock Speed: %lu", HAL_RCC_GetPCLK1Freq());
    if (HAL_CAN_Init(&hcan1) != HAL_OK)
    {
        LOG_ERROR(CAN, "Failed CAN Init");
        return -CANARD_ERROR_INTERNAL;
    }
    hcan1.pRxMsg = &rxMsg;  // Set up a buffer to receive into

    /* USER CODE BEGIN FDCAN1_Init 2 */
    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
        LOG_ERROR(CAN, "Failed Config Filter");
        return -CANARD_ERROR_INTERNAL;
    }
    if (HAL_CAN_Receive_IT(&hcan1, CAN_FIFO0) != HAL_OK)
    {
        LOG_ERROR(CAN, "Failed to enable interrupts");
        return -CANARD_ERROR_INTERNAL;
    }
    //Don't need to explicitly start the CAN driver in v1.2.2
    // if (HAL_CAN_Start(hcan1) != HAL_OK) {
    //     LOG_ERROR(CAN, "Failed to Start");
    //     return -CANARD_ERROR_INTERNAL;
    // }
    return CANARD_OK;
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void canardSTM32GPIO_Init(void)
{

   // Set up the Rx and Tx pins for CAN1 and if present, the standby or listen only pin.
#if defined(CAN1_TX) && defined(CAN1_RX)
    IOInit(IOGetByTag(IO_TAG(CAN1_TX)), OWNER_DRONECAN, RESOURCE_CAN_TX, 0);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(CAN1_TX)), IOCFG_AF_PP, GPIO_AF9_CAN1);  // How do I make the alternate function crossplatform?
    IOInit(IOGetByTag(IO_TAG(CAN1_RX)), OWNER_DRONECAN, RESOURCE_CAN_RX, 0);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(CAN1_RX)), IOCFG_AF_PP, GPIO_AF9_CAN1);  // How do I make the alternate function crossplatform?
#endif


 #ifdef CAN1_STANDBY
    // Initialize the standby or listen only pin.  Set default state to enable CAN.
    // TODO: Tie the pin state to a configuration option so we can turn CAN on and off.

    IOInit(IOGetByTag(IO_TAG(CAN1_STANDBY)), OWNER_DRONECAN, RESOURCE_CAN_STANDBY, 0);
    IOConfigGPIO(IOGetByTag(IO_TAG(CAN1_STANDBY)), IOCFG_OUT_PP);  // Do any boards use pullups, external/internal?
    IOLo(IOGetByTag(IO_TAG(CAN1_STANDBY)));
#endif
}
static bool canardSTM32ComputeTimings(const uint32_t target_bitrate, struct Timings *out_timings)
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
    const int max_quanta_per_bit = (target_bitrate >= 1000000) ? 10 : 18;
    LOG_DEBUG(CAN, "Baudrate: %lu", target_bitrate);
    LOG_DEBUG(CAN, "Max Quanta per bit: %i", max_quanta_per_bit);
    LOG_DEBUG(CAN, "Pclk1: %lu", pclk);
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
    LOG_DEBUG(CAN, "Prescaler BS product: %lu", prescaler_bs);
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
    LOG_DEBUG(CAN, "Prescaler: %lu", prescaler);

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
        uint16_t sample_point_permill;
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

    LOG_DEBUG(CAN, "Timings: quanta/bit: %d, sample point location: %f%%",
          (int)(1 + solution.bs1 + solution.bs2), (double)(solution.sample_point_permill) / (double)(10.0));

    out_timings->prescaler = (uint16_t)(prescaler);
    out_timings->sjw = 3;                        // Not happy with this value, but 1MBPs with unshielded cable?
    out_timings->bs1 = (uint8_t)(solution.bs1)-1;  // The HAL does not take care of the 1 bs offset in the register so remove it here like AP does.
    out_timings->bs2 = (uint8_t)(solution.bs2)-1;  // The HAL does not take care of the 1 bs offset in the register so remove it here like AP does.

    return true;
}

void canardSTM32GetProtocolStatus(canardProtocolStatus_t *pProtocolStat){

    pProtocolStat->BusOff = __HAL_CAN_GET_FLAG(&hcan1, CAN_FLAG_BOF);
    pProtocolStat->ErrorPassive = __HAL_CAN_GET_FLAG(&hcan1, CAN_FLAG_EPV);
    // LOG_DEBUG(CAN, "BusOff: %lu", pProtocolStat->BusOff);
    // LOG_DEBUG(CAN, "ErrorPassive: %lu", pProtocolStat->ErrorPassive);
}

int32_t canardSTM32GetRxFifoFillLevel(void){
    return rxBufferNumMessages(&RxBuffer);
}

void canardSTM32RecoverFromBusOff(void){
    // Auto recover from bus off is enabled
    // CLEAR_BIT(hcan1.Instance->CCCR, FDCAN_CCCR_INIT);  // Clear INIT bit to recover from Bus-Off
}

/*
  get a 16 byte unique ID for this node, this should be based on the CPU unique ID or other unique ID
 */
void canardSTM32GetUniqueID(uint8_t id[16]) {
    uint32_t HALUniqueIDs[3];
    // Make Unique ID out of the 96-bit STM32 UID and fill the rest with 0s
    memset(id, 0, 16);
    HALUniqueIDs[0] = *(uint32_t *)UID_BASE;
    HALUniqueIDs[1] = *(uint32_t *)(UID_BASE + 4);
    HALUniqueIDs[2] = *(uint32_t *)(UID_BASE + 8);
    memcpy(id, HALUniqueIDs, 12);
}

void CAN1_RX0_IRQHandler(void) {
      HAL_CAN_IRQHandler(&hcan1);
}

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef * hcan) {

    rxBufferPushFrame(&RxBuffer, hcan1.pRxMsg);
	
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_FMP0);

    //return HAL_OK;
}