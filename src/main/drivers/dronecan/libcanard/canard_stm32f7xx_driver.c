/*
 * canard_stm32_driver.c
 *
 *  Created on: Jul 8, 2024
 *      Author: Roni Kant
 */

#include "common/log.h"
#include "common/time.h"
#include "drivers/io.h"
#include "drivers/nvic.h"
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

typedef struct {
    CAN_RxHeaderTypeDef header;
    uint8_t data[8];
} RxFrame_t;

static struct RxBuffer_t {
    volatile uint8_t writeIndex;  // written in ISR, read in main loop
    volatile uint8_t readIndex;
    RxFrame_t rxMsg[RX_BUFFER_SIZE];
} RxBuffer;

#define TX_QUEUE_SIZE 32

static struct CanTxQueue_t {
    CanardCANFrame frames[TX_QUEUE_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} canTxQueue;

static bool canardSTM32ComputeTimings(const uint32_t target_bitrate, struct Timings*out_timings);
static void canardSTM32GPIO_Init(void);
static void canTxDrainQueue(CAN_HandleTypeDef *hcan);

static CAN_HandleTypeDef hcan1;

static volatile uint16_t canTxDropped = 0;
static volatile uint8_t  canTxQueueHWM = 0;
static volatile uint8_t  canRxBufferHWM = 0;

uint8_t rxBufferPushFrame(struct RxBuffer_t *rxBuf, RxFrame_t *rxMsg) {
    uint8_t next;
    RxFrame_t *pCurrentRxMsg;

    next = rxBuf->writeIndex + 1;
    if(next >= RX_BUFFER_SIZE){
        next = 0;
    }

    if(next == rxBuf->readIndex) {
        return -1;  // rxBuf is full
    }
    pCurrentRxMsg = &rxBuf->rxMsg[rxBuf->writeIndex];
    memcpy(pCurrentRxMsg, rxMsg, sizeof(RxFrame_t));
    rxBuf->writeIndex = next;
    uint8_t rxFill = (next >= rxBuf->readIndex) ? (next - rxBuf->readIndex) : (next + RX_BUFFER_SIZE - rxBuf->readIndex);
    if (rxFill > canRxBufferHWM) canRxBufferHWM = rxFill;
    return 0;
}

uint8_t rxBufferPopFrame(struct RxBuffer_t *rxBuf, RxFrame_t *rxMsg) {
    uint8_t next;
    RxFrame_t *pCurrentRxMsg;

    if(rxBuf->writeIndex == rxBuf->readIndex){
        return -1;  // Nothing to read
    }

    next = rxBuf->readIndex + 1;
    if (next >= RX_BUFFER_SIZE){
        next = 0;
    }
    pCurrentRxMsg = &rxBuf->rxMsg[rxBuf->readIndex];
    memcpy(rxMsg, pCurrentRxMsg, sizeof(RxFrame_t));
    rxBuf->readIndex = next;
    return 0;
}

uint8_t rxBufferNumMessages(struct RxBuffer_t *rxBuf) {
    if(rxBuf->writeIndex < rxBuf->readIndex)
        return((rxBuf->writeIndex + RX_BUFFER_SIZE) - rxBuf->readIndex);

    return (rxBuf->writeIndex - rxBuf->readIndex);
}

static bool canTxQueuePush(const CanardCANFrame *frame) {
    uint8_t next = (canTxQueue.head + 1) % TX_QUEUE_SIZE;
    uint8_t tail_snapshot = canTxQueue.tail;  // snapshot before ISR can advance it
    if (next == tail_snapshot) {
        canTxDropped++;
        return false;
    }
    canTxQueue.frames[canTxQueue.head] = *frame;
    __DMB();  // ensure frame data is visible before head advances
    canTxQueue.head = next;
    uint8_t fill = (next - tail_snapshot + TX_QUEUE_SIZE) % TX_QUEUE_SIZE;
    if (fill > canTxQueueHWM) canTxQueueHWM = fill;
    return true;
}

static bool canTxQueueIsEmpty(void) {
    return canTxQueue.head == canTxQueue.tail;
}

/**
  * @brief  Process CAN message from RxLocation FIFO into rx_frame
  * @param  rx_frame pointer to a CanardCANFrame structure where the received CAN message will be
  * 		stored.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Recieve(CanardCANFrame *const rx_frame) {
    RxFrame_t canRxFrame;

    if (rx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	if (rxBufferPopFrame(&RxBuffer, &canRxFrame) == 0) {  // Wheres the data?
        rx_frame->id = canRxFrame.header.ExtId;

		// Process ID to canard format
		if (canRxFrame.header.IDE == CAN_ID_EXT) { // canard will only process the message if it is extended ID
            rx_frame->id |= CANARD_CAN_FRAME_EFF;
		}

		if (canRxFrame.header.RTR == CAN_RTR_REMOTE) { // canard won't process the message if it is a remote frame
			rx_frame->id |= CANARD_CAN_FRAME_RTR;
		}

		rx_frame->data_len = canRxFrame.header.DLC;
		memcpy(rx_frame->data, canRxFrame.data, canRxFrame.header.DLC);

		// assume a single interface
		rx_frame->iface_id = 0;
		return 1;
	}
	// Either no CAN msg to be read, or an error that can be read from hfdcan->ErrorCode
	return 0;
}

static void buildTxHeader(const CanardCANFrame *frame, CAN_TxHeaderTypeDef *header, uint8_t *data) {
    if (frame->id & CANARD_CAN_FRAME_EFF) {
        header->IDE = CAN_ID_EXT;
        header->ExtId = frame->id & CANARD_CAN_EXT_ID_MASK;
    } else {
        header->IDE = CAN_ID_STD;
        header->StdId = frame->id & CANARD_CAN_STD_ID_MASK;
    }
    header->DLC = frame->data_len;
    header->RTR = (frame->id & CANARD_CAN_FRAME_RTR) ? CAN_RTR_REMOTE : CAN_RTR_DATA;
    header->TransmitGlobalTime = DISABLE;
    memcpy(data, frame->data, frame->data_len);
}

/**
  * @brief  Process tx_frame CAN message into Tx FIFO/Queue and transmit it
  * @param  tx_frame pointer to a CanardCANFrame structure that contains the CAN message to
  * 		transmit.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Transmit(const CanardCANFrame* const tx_frame) {
    if (tx_frame == NULL) {
        return -CANARD_ERROR_INVALID_ARGUMENT;
    }
    if (tx_frame->id & CANARD_CAN_FRAME_ERR) {
        return -CANARD_ERROR_INVALID_ARGUMENT;
    }

    if (!canTxQueuePush(tx_frame)) {
        return 0; // SW queue full - caller retries next cycle
    }

    // If all mailboxes are idle, RQCP will never fire to start the ISR chain.
    // Seed the HW via canTxDrainQueue with IRQs disabled to preserve the
    // SPSC contract — ISR is the sole consumer; we become the consumer only
    // while the ISR cannot run.
    __disable_irq();
    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 3) {
        canTxDrainQueue(&hcan1);
    }
    if (!canTxQueueIsEmpty()) {
        HAL_StatusTypeDef status = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);
        __enable_irq();
        return (status == HAL_OK) ? 1 : 0;  // 0 signals caller to retry
    }
    __enable_irq();
    return 1;
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

    // /* USER CODE BEGIN CAN1_MspInit 1 */

    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterIdHigh = 0;
    sFilterConfig.FilterIdLow  = 0;
    sFilterConfig.FilterMaskIdHigh = 0;
    sFilterConfig.FilterMaskIdLow = 0;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterActivation = ENABLE;

    hcan1.Instance = CAN1;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = ENABLE;  // transmit in request order, not mailbox-number order

    canardSTM32ComputeTimings(bitrate, &out_timings);

    hcan1.Init.Prescaler = out_timings.prescaler;
    hcan1.Init.SyncJumpWidth = (uint32_t)out_timings.sjw << CAN_BTR_SJW_Pos;
    hcan1.Init.TimeSeg1     = (uint32_t)out_timings.bs1 << CAN_BTR_TS1_Pos;
    hcan1.Init.TimeSeg2     = (uint32_t)out_timings.bs2 << CAN_BTR_TS2_Pos;
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

    /* USER CODE BEGIN FDCAN1_Init 2 */
    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
        LOG_ERROR(CAN, "Failed Config Filter");
        return -CANARD_ERROR_INTERNAL;
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        LOG_ERROR(CAN, "Failed to Start");
        return -CANARD_ERROR_INTERNAL;
    }

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {  // persistent Rx notification
        LOG_ERROR(CAN, "Failed to activate interrupt");
        return -CANARD_ERROR_INTERNAL;
    }

    // Enable interrupt only after all initialization succeeds
    // (if any previous step failed, we return early without enabling IRQ)
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, NVIC_PRIO_CAN, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, NVIC_PRIO_CAN, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);

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
    uint32_t esr = hcan1.Instance->ESR;
    pProtocolStat->BusOff       = __HAL_CAN_GET_FLAG(&hcan1, CAN_FLAG_BOF);
    pProtocolStat->ErrorPassive = __HAL_CAN_GET_FLAG(&hcan1, CAN_FLAG_EPV);
    pProtocolStat->tec          = (uint8_t)((esr >> 16) & 0xFF);
    pProtocolStat->rec          = (uint8_t)((esr >> 24) & 0xFF);
    pProtocolStat->lec          = (uint8_t)((esr >> 4) & 0x07);
    pProtocolStat->tx_dropped    = canTxDropped;
    pProtocolStat->tx_queue_hwm  = canTxQueueHWM;
    pProtocolStat->rx_buffer_hwm = canRxBufferHWM;
}

int32_t canardSTM32GetTxQueueFillLevel(void) {
    return (canTxQueue.head - canTxQueue.tail + TX_QUEUE_SIZE) % TX_QUEUE_SIZE;
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

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    RxFrame_t frame;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &frame.header, frame.data) == HAL_OK) {
        rxBufferPushFrame(&RxBuffer, &frame);
    }
}

static void canTxDrainQueue(CAN_HandleTypeDef *hcan) {
    // With TXFP=ENABLE the hardware transmits mailboxes in request order
    // (chronological), not mailbox-number order, so filling multiple mailboxes
    // per callback is safe and preserves frame sequence.
    while (!canTxQueueIsEmpty() && HAL_CAN_GetTxMailboxesFreeLevel(hcan) > 0) {
        __DMB();  // observe producer stores before reading frame data
        CAN_TxHeaderTypeDef txHeader = {};
        uint8_t txData[8];
        uint32_t txMailbox;
        buildTxHeader(&canTxQueue.frames[canTxQueue.tail], &txHeader, txData);
        if (HAL_CAN_AddTxMessage(hcan, &txHeader, txData, &txMailbox) != HAL_OK) {
            break;  // frame stays in queue (tail not advanced) and will retry next ISR
        }
        // Advance tail only after HAL confirms acceptance — frame is never lost
        canTxQueue.tail = (canTxQueue.tail + 1) % TX_QUEUE_SIZE;
    }
    if (canTxQueueIsEmpty()) {
        HAL_CAN_DeactivateNotification(hcan, CAN_IT_TX_MAILBOX_EMPTY);
    }
}

void CAN1_TX_IRQHandler(void) {
    HAL_CAN_IRQHandler(&hcan1);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) { canTxDrainQueue(hcan); }
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) { canTxDrainQueue(hcan); }
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) { canTxDrainQueue(hcan); }
